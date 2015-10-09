#include <iostream>
#include <sstream>
#include <chrono>
#include "include/interact.h"
#include "include/reader.h"

int main(int argc, char **argv) {
    if (argc < 4) {
        std::cout << "usage: provide evdev path for the touch screen on your device together with log file to play back" << std::endl;
        std::cout << "./player /dev/input/event0 recording.log 1.23" << std::endl;
        std::cout << "./player EVDEV CSV_SOURCE DELAY_SEC" << std::endl;
        std::cout << "hint: adb shell getevent" << std::endl;
        return 1;
    }
    
    float delay_sec = 0;
    std::stringstream delay_conv(argv[3]);
    if (!(delay_conv >> delay_sec)) {
        std::cerr << "Offset must be float " << argv[3] << std::endl;
        return 1;
    }
    
    std::ifstream f(argv[2]);
    if (f.fail()) {
        std::cerr << "Couldn't open " << argv[2] << std::endl;
        return 1;
    }
    AdbInstance adb(argv[1]);
    if (adb.set_device()) {
        std::cerr << "ADB no device" << std::endl;
        return 1;
    }
    
    return adb.playback_events(read_events(f));
}
