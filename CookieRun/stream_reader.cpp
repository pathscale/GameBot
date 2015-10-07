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
    enum states {
        UNKNOWN,
        NOTLOAD,
        LOAD
    };
    int state;
    const cv::Point2i pixel = cv::Point2i(250, 30); // a pixel from the moon
    const cv::Vec4b val = cv::Vec4b(252, 239, 222);
public:
    GameState()
        : state(UNKNOWN)
    {};
    void update(const cv::Mat &frame) {
        const cv::Vec4b bgr = frame.at<cv::Vec4b>(pixel); 
        const int thr = 5;
        //std::cout << (int)bgr[0] << " " << (int)bgr[1] << " " << (int)bgr[2] << std::endl;
        if (std::abs(bgr[0] - val[0]) < thr &&
            std::abs(bgr[1] - val[1]) < thr &&
            std::abs(bgr[2] - val[2]) < thr) {
            if (state != LOAD) {
                std::cout << "loading started" << std::endl;
            }
            state = LOAD;
        } else {
            if (state == LOAD) {
                std::cout << "loading ended, BEGIN===================" << std::endl;
            }
            state = NOTLOAD;
        }
    }
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "try again" << std::endl;
        return -1;
    }    
    //FileSource s(argv[1]);
    int port;
    std::stringstream sstr(argv[1]);
    sstr >> port;
    if (!sstr) {
        std::cout << "bad port" << std::endl;
        return -1;
    }
    SocketSource s(port);
    struct buffer_config bconf = s.get_buf_conf();
    int bytes_px = 4; // fixme
    
    std::cout << "x " << bconf.xres << " y " << bconf.yres << " stride " << bconf.stride << std::endl;
    std::cout << "calc size " << bconf.yres * bconf.stride * bytes_px << std::endl;
    std::cout << "total size " << bconf.size << std::endl;
    
    class GameState gs;
    
    void *fb_buf;
    int i;
    for (i = 0; fb_buf = s.get_frame(); i++) {
        cv::Mat m(bconf.yres, bconf.stride, CV_8UC4, fb_buf);
        gs.update(m);
        /*cv::Mat o(bconf.yres, bconf.stride, CV_8UC3);
        int from_to[] = {0, 2,
                         1, 1,
                         2, 0};
        cv::mixChannels(&m, 1, &o, 1, from_to, 3);
        std::cout << "out " + std::to_string(i) << std::endl;*/
        /*cv::Mat o = m(cv::Rect(196 - 10, 117 - 10, 20, 20));
        cv::imwrite("out" + std::to_string(i) + ".png", o);*/
        free(fb_buf);
    }
    std::cout << "wrote " << i << " images" << std::endl;
    return 0;
}
