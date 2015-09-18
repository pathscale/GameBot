#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include "include/interact.h"

#define TRACKING_ID 0x1000abcd

static std::string get_adb_command() {
    char *e = getenv("ADB_BIN");
    std::string exec = "adb";
    if (e) {
        exec = e;
    }
    return exec + " " + "shell";
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
        s << "sendevent " << device << " " << code.type << " " << code.code << " " << code.value << std::endl;
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
    return codes_to_cmd(device, {
        {0x0003, 0x0039, TRACKING_ID}, // tracking ID
{0x0003, 0x0030, 0x00000005}, // major axis
{0x0003, 0x0035, x},
{0x0003, 0x0036, y},
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
    return codes_to_cmd(device,
                        {{0x0003, 0x0039, -1}, // reset tracking ID
                         {0x0000, 0x0000, 0x00000000}}); // syn
                        /*
                        {{1, 0x14a, 0}, // BTN_TOUCH
                         {0, 0, 0}, // SYN_REPORT
                        });*/
}

AdbInstance::AdbInstance() {
}

int AdbInstance::set_device() {
    stream = popen(get_adb_command().c_str(), "w");
    if (stream) {
        return 0;
    }
    return 1;
}

int AdbInstance::send_commands(const std::string &cmd) {
    size_t wrote = fwrite(cmd.c_str(), 1, cmd.length(), stream);
    if (wrote != cmd.length()) {
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
        pclose(stream);
    }
}
