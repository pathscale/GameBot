#include <string>
#include <vector>
#include <chrono>
#include "include/common.h"

class AdbInstance {
protected:
    FILE *stream = 0;
    std::string evdev_path;
    std::string syn;
    
    int send_commands(const std::string &cmd);
    std::string get_start_touch(unsigned x, unsigned y, bool syn=true);
    std::string get_end_touch(int desc, bool syn=true);
public:
    AdbInstance(const std::string &evdev_path);
    int set_device();
    int playback_events(const std::vector<event> &events, float delay_sec=0);
    int start_touch(unsigned x, unsigned y);
    int end_touch(int desc);
    int start_touch(unsigned x, unsigned y, std::chrono::steady_clock::time_point time);
    int end_touch(int desc, std::chrono::steady_clock::time_point time);
    ~AdbInstance();
};
