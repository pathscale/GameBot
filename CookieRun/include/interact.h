#include <string>

class AdbInstance {
protected:
    FILE *stream = 0;
    std::string device = "/dev/input/event2"; // FIXME: unhardcode
  
    int send_commands(const std::string &cmd);
    std::string get_start_touch(unsigned x, unsigned y);
    std::string get_end_touch(int desc);
public:
    AdbInstance();
    int set_device();
    int start_touch(unsigned x, unsigned y);
    int end_touch(int desc);
    ~AdbInstance();
};
