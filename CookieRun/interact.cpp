#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <stdlib.h>
#include "include/interact.h"

#define TRACKING_ID 0x1000abcd

std::chrono::milliseconds float_to_dur(float dur) {
    return std::chrono::milliseconds((int)(dur * 1000));
}

static std::string get_adb_command() {
    return "adb shell";
    //return "cat > pajp";
}

struct evcode {
    uint16_t type;
    uint16_t code;
    int32_t value;
};

std::string codes_to_cmd(const std::string &device, const std::vector<evcode> &codes) {
    std::stringstream s;
    for (const evcode &code : codes) {
        s << "sendevent " << device << " " << code.type << " " << code.code << " " << code.value << ";";
    }
    return s.str();
}

/*
 * adb shell getevent
 * on a device which supports multitouch and identifiable inputs (type B)
 * https://www.kernel.org/doc/Documentation/input/multi-touch-protocol.txt
// not shown: set slot 0
/dev/input/event2: 0003 0039 000010e6 // tracking ID
/dev/input/event2: 0003 0030 00000004 // width
/dev/input/event2: 0003 0035 000001be // x
/dev/input/event2: 0003 0036 000001a0 // y
/dev/input/event2: 0003 003a 00000034 // pressure
/dev/input/event2: 0000 0000 00000000 // syn
/dev/input/event2: 0003 0030 00000003 // width
/dev/input/event2: 0003 003a 00000032 // pressure
/dev/input/event2: 0000 0000 00000000 // syn
/dev/input/event2: 0003 003a 00000027 // pressure
/dev/input/event2: 0000 0000 00000000 // syn
/dev/input/event2: 0003 0039 ffffffff // invalidate tracking id (lift)
/dev/input/event2: 0000 0000 00000000 // syn
*/

// FIXME: specify slot in each packet
// NOTE: this will blow up slightly if somethine else is pressed
// NOTE: this may blow up a lot if disconnected in the middle of input
// WARNING: some devices may not understand these commands at all

std::string AdbInstance::get_start_touch(unsigned x, unsigned y) {
    return codes_to_cmd(evdev_path, {
        {0x0003, 0x0039, TRACKING_ID}, // tracking ID
        {0x0003, 0x0030, 0x00000005}, // major axis
        {0x0003, 0x0035, (int)x},
        {0x0003, 0x0036, (int)y},
        {0x0003, 0x003a, 0x00000021}, // pressure
        {0x0000, 0x0000, 0x00000000}}); // syn
    /*return codes_to_cmd(device,
                        {{3, 0x35, x}, // ABS_MT_POSITION_X
                         {3, 0x36, y}, // ABS_MT_POSITION_Y
                         {1, 0x14a, 1}, // BTN_TOUCH
                         {0, 0, 0}, // SYN_REPORT
                        });*/
}

std::string AdbInstance::get_end_touch(int desc) {
    return codes_to_cmd(evdev_path,
                        {{0x0003, 0x0039, -1}, // reset tracking ID
                         {0x0000, 0x0000, 0x00000000}}); // syn
                        /*
                        {{1, 0x14a, 0}, // BTN_TOUCH
                         {0, 0, 0}, // SYN_REPORT
                        });*/
}

AdbInstance::AdbInstance(const std::string &evdev_path)
    : evdev_path(evdev_path)
{}

int AdbInstance::set_device() {
    stream = popen(get_adb_command().c_str(), "w");
    if (!stream) {
        return 1;
    }
    send_commands("echo working");
    return 0;
}

int AdbInstance::send_commands(const std::string &cmd) {
    std::string out(cmd + "\n");
    size_t wrote = fwrite(out.c_str(), 1, out.length(), stream);
    if (wrote != out.length()) {
        std::cerr << "write size invalid" << std::endl;
        return 1;
    }
    fflush(stream);
    return 0;
}

int AdbInstance::start_touch(unsigned x, unsigned y) {
    return send_commands(get_start_touch(x, y));
}

int AdbInstance::end_touch(int desc) {
    return send_commands(get_end_touch(desc));
}

AdbInstance::~AdbInstance() {
    if (stream) {
        send_commands("exit");
        pclose(stream);
    }
}

int AdbInstance::playback_events(const std::vector<event> &events, float delay_sec) {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now() + float_to_dur(delay_sec);
    for (const event e : events) {
        std::chrono::steady_clock::time_point event_time = begin + float_to_dur(e.start);
        std::this_thread::sleep_until(event_time);
        int desc = start_touch(e.x, e.y);
        //std::cout << "ON: x " << e.x << " y " << e.y << " s " << e.start << " d " << e.duration << std::endl;
        if (e.duration) {
            event_time = event_time + float_to_dur(e.duration);
            std::this_thread::sleep_until(event_time);
        }
        int res = end_touch(desc);
        if (res) {
            std::cerr << "invalid touch descriptor" << std::endl;
        }
        //std::cout << "OFF" << std::endl;
    }
    return 0;
}
