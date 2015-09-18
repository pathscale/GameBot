#include <string>
#include <sstream>
#include <iostream>
#include "include/interact.h"


static std::string get_adb_command() {
    return std::string("./adb") + " " + "shell";
    //return "cat > pajp";
}

std::string AdbInstance::get_start_touch(unsigned x, unsigned y) {
    std::stringstream s;
    s << "sendevent " << device << " " << x << " " << y << " code" << std::endl;
    // TODO
    return s.str();
}

std::string AdbInstance::get_end_touch(int desc) {
    std::stringstream s;
    s << "sendevent " << device << " " << 0 << " " << 0 << " code" << std::endl;
    // TODO
    return s.str();
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
