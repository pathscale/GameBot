#include <thread>
#include <iostream>
#include <sstream>
#include <chrono>
#include "include/interact.h"
#include "include/reader.h"


#include <opencv2/core.hpp>
#include <sys/fcntl.h>
#include <iostream>
#include <linux/fb.h>
#include <sys/mman.h>
#include <opencv2/imgcodecs.hpp>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>

struct buffer_config {
    uint32_t size;
    uint32_t xres;
    uint32_t yres;
    uint32_t stride;
    uint32_t format;
};

class ImageSource {
protected:
    int fd;
    int frame_size;
public:
    struct buffer_config get_buf_conf() {
        struct buffer_config bconf;
        int r = read(fd, &bconf, sizeof(struct buffer_config));
        if (r != sizeof(struct buffer_config)) {
            std::cerr << "short header read" << std::endl;
            bconf.size = 0;
        }
        this->frame_size = bconf.size;
        return bconf;
    }
    
    void* get_frame() {
        void *data = malloc(this->frame_size);
        int remaining;
        for (remaining = this->frame_size; remaining > 0; ) {
            int readsize = read(fd, (void*)((char*)data + (this->frame_size - remaining)), remaining);
            if (readsize <= 0) {
                break;
            }
            remaining -= readsize;
        }
        if (remaining < 0) {
            std::cerr << "super error" << std::endl;
            return NULL;
        }
        if (remaining > 0) {
            std::cerr << "short frame read" << std::endl;
            free(data);
            return NULL;
        }
        return data;
    }
    
    ~ImageSource() {
        close(fd);
    }
};

class FileSource : public ImageSource {
public:
    FileSource(char *filename) {
        fd = open(filename, O_RDONLY);
    }
};

class SocketSource : public ImageSource {
public:
    SocketSource(int port) {
        fd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in serv_addr = {AF_INET, htons(port), 0};
        int res = inet_aton("127.0.0.1", &serv_addr.sin_addr);
        if (res == 0) {
            std::cerr << "Bad address" << std::endl;
            return;
        }
        res = connect(fd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr_in));
        if (res) {
            std::cerr << "couldn't connect" << std::endl;
        }
    }
};

class GameState {
    /* State transitions:
     * Menus (click:play) -> loading screen (anchor:moon) -> level loaded, no character (~139ms <33ms; 1200ms>) (anchor: white avatar) -> level (~6986ms) -> first jump
     */
    /* timings between load end and first jump (regardless ping)
     * 6.893 way too late (falls in, good after respawn)
     * 6.693 much too late (floats over (?), good after 2 crashes)
     * 6.393 a bit too late (falls into 2nd hole, good later)
     * 6.193 minimally too early (bug finishes it)
     * 6.193 minimally too early (bug finishes it)
     */
public:
    enum state {
        UNKNOWN = 0,
        NOTLOAD = 1,
        LOAD = 2,
    };
protected:
    enum state cur_state;
    const cv::Point2i pixel = cv::Point2i(250, 30); // a pixel from the moon
    const cv::Vec4b val = cv::Vec4b(252, 239, 222);
public:
    GameState()
        : cur_state(UNKNOWN)
    {};
    void update(const cv::Mat &frame) {
        const cv::Vec4b bgr = frame.at<cv::Vec4b>(pixel); 
        const int thr = 5;
        //std::cout << (int)bgr[0] << " " << (int)bgr[1] << " " << (int)bgr[2] << std::endl;
        if (std::abs(bgr[0] - val[0]) < thr &&
            std::abs(bgr[1] - val[1]) < thr &&
            std::abs(bgr[2] - val[2]) < thr) {
            if (cur_state != LOAD) {
                std::cout << "loading started" << std::endl;
            }
            cur_state = LOAD;
        } else {
            if (cur_state == LOAD) {
                std::cout << "loading ended, BEGIN===================" << std::endl;
            }
            cur_state = NOTLOAD;
        }
        std::cout << "state is " << cur_state << std::endl;
    }
    enum state get_state() {
        return cur_state;
    }
};

class Listener {
protected:
    SocketSource s;
    struct buffer_config bconf;
    class GameState gs;
public:
    Listener(int port=9999)
        : s(port)
    {}
    
    int connect() {    
        bconf = s.get_buf_conf();
        if (bconf.size == 0) {
            return 1;
        }
        int bytes_px = 4; // fixme
        
        std::cout << "x " << bconf.xres << " y " << bconf.yres << " stride " << bconf.stride << std::endl;
        std::cout << "calc size " << bconf.yres * bconf.stride * bytes_px << std::endl;
        std::cout << "total size " << bconf.size << std::endl;
        return 0;
    }
    
    int read_state() {
        void *fb_buf = s.get_frame();
        if (fb_buf == 0) {
            return 1;
        }
        cv::Mat m(bconf.yres, bconf.stride, CV_8UC4, fb_buf);
        gs.update(m);
        free(fb_buf);
        return 0;
    }
    
    int wait_until_level() {
        void *fb_buf;
        int i;
        GameState::state prev_state = gs.get_state();
        while (1) {
            if (read_state()) {
                return 1;
            }
            GameState::state cur_state = gs.get_state();
            if (prev_state == GameState::LOAD && cur_state == GameState::NOTLOAD) {
                break;
            }
            prev_state = cur_state;
        }
        return 0;
    }
};

int playback_events(AdbInstance &adb, const std::vector<event> &events, float delay_sec=0) {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now() + float_to_dur(delay_sec);
    for (const event e : events) {
        std::chrono::steady_clock::time_point event_time = begin + float_to_dur(e.start);
        std::this_thread::sleep_until(event_time);
        int desc = adb.start_touch(e.x, e.y);
        std::cout << "ON: x " << e.x << " y " << e.y << " s " << e.start << " d " << e.duration << std::endl;
        if (e.duration) {
            event_time = event_time + float_to_dur(e.duration);
            std::this_thread::sleep_until(event_time);
        }
        if (adb.end_touch(desc)) {
            std::cerr << "invalid touch descriptor" << std::endl;
        }
        std::cout << "OFF" << std::endl;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "usage: upload and start recorder server" << std::endl;
        std::cout << "browse to screen with \"play\" button" << std::endl;
        std::cout << "provide evdev path for the touch screen on your device" << std::endl;
        std::cout << "hint: adb shell getevent" << std::endl;
        std::cout << "DETECTION WORKS WITH NVIDIA SHIELD ONLY!!!" << std::endl;
        std::cout << "./player /dev/input/event0" << std::endl;
        std::cout << "./player EVDEV" << std::endl;
        return 1;
    }
    
    AdbInstance adb(argv[1]);
    if (adb.set_device()) {
        std::cerr << "ADB no device" << std::endl;
        return 1;
    }
    std::ifstream ns("nav_start.log");
    if (!ns) {
        std::cerr << "Failed to open nav_start.log" << std::endl;
        return 1;
    }
    const std::vector<event> navigate_to_start = read_events(ns);
    std::ifstream cp("click_play.log");
    if (!cp) {
        std::cerr << "Failed to open click_play.log" << std::endl;
        return 1;
    }
    const std::vector<event> click_play = read_events(cp);
    std::ifstream pl("play_level.log");
    if (!pl) {
        std::cerr << "Failed to open play_level.log" << std::endl;
        return 1;
    }
    const std::vector<event> play_level = read_events(pl);
    std::cout << navigate_to_start.size() << " " << click_play.size() << " " << play_level.size() << std::endl;
    /*
    TODO: autostart server
    AdbCommand::execute("forward tcp:9999 tcp:5645");
    AdbCommand::execute("push ./recorder.arm /data/local/tmp/");
    AdbCommand server("shell /data/local/tmp/recorder.arm");
    server.start();
    */
    
    std::cout << "going to start screen" << std::endl;
    playback_events(adb, navigate_to_start);
    std::cout << "at start screen, connecting video" << std::endl;
    Listener listener;
    if (listener.connect()) {
        std::cout << "failed to connect" << std::endl;
        return 1;
    }
    std::cout << "connected, starting level" << std::endl;
    playback_events(adb, click_play);
    std::cout << "waiting until level starts" << std::endl;
    if (listener.wait_until_level()) {
        return 1;
    }
    std::cout << "GO GO GO" << std::endl;
    playback_events(adb, play_level);
    std::cout << "done!" << std::endl;
    return 0;
}
