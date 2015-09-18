#include <iostream>
#include <sstream>
#include <fstream>
#include <csvpp.h>
#include <vector>

const char filename[] = "sample.log";

struct event {
    int x;
    int y;
    float start; // sec
    float duration; // sec
};


const event read_event(csvpp::RowReader &rd) {
    event e;
    for (const std::pair<std::string, std::string> entry : rd) {
        const std::string &key = entry.first;
        const std::string &strval = entry.second;
        //std::cout << "k " << key << " " << strval << std::endl;
        if (key.compare("x") == 0) {
            if (!(std::stringstream(strval) >> e.x)) {
                std::cerr << "x is not int:" << strval << std::endl;
            }
        } else if (key.compare("y") == 0) {
            if (!(std::stringstream(strval) >> e.y)) {
                std::cerr << "y is not int:" << strval << std::endl;
            }
        } else if (key.compare("start") == 0) {
            if (!(std::stringstream(strval) >> e.start)) {
                std::cerr << "start is not float:" << strval << std::endl;
            };
        } else if (key.compare("duration") == 0) {
            if (!(std::stringstream(strval) >> e.duration)) {
                std::cerr << "duration is not float:" << strval << std::endl;
            };
        } else {
            std::cerr << "unexpected key:" << key << std::endl;
        }
    }
    return e;
}

const std::vector<event> read_events(std::istream &i) {
    csvpp::RowReader rd;
    std::stringstream("x,y,start,duration\n") >> rd;
    std::vector<event> ret;
    while (i >> rd) {
        ret.push_back(read_event(rd));
    }
    return ret;
}

int main(void) {
    std::ifstream f(filename);
    for (const event e : read_events(f)) {
        std::cout << "x " << e.x << " y " << e.y << " s " << e.start << " d " << e.duration << std::endl;
    }
    return 0;
}
