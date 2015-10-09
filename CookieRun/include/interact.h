#include <string>
#include <vector>
#include "include/common.h"

class AdbInstance {
protected:
    FILE *stream = 0;
    std::string evdev_path;
  
    int send_commands(const std::string &cmd);
    std::string get_start_touch(unsigned x, unsigned y);
    std::string get_end_touch(int desc);
public:
    AdbInstance(const std::string &evdev_path);
    int set_device();
    int playback_events(const std::vector<event> &events, float delay_sec=0);
    int start_touch(unsigned x, unsigned y);
    int end_touch(int desc);
    ~AdbInstance();
};
