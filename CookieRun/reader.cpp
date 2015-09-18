#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>
#include "include/csvpp.h"

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
        ///std::cout << "k " << key << " " << strval << std::endl;
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

std::chrono::duration<int, std::milli> float_to_dur(float dur) {
    return std::chrono::duration<int, std::milli>((int)(dur * 1000));
}

int main(void) {
    std::ifstream f(filename);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    //std::cout << begin.time_since_epoch().count() / 1000 << std::endl;
    for (const event e : read_events(f)) {
        std::chrono::steady_clock::time_point event_time = begin + float_to_dur(e.start);
        //std::cout << event_time.time_since_epoch().count() / 1000 << std::endl;
        std::this_thread::sleep_until(event_time);
        std::cout << "ON: x " << e.x << " y " << e.y << " s " << e.start << " d " << e.duration << std::endl;
        if (e.duration) {
            event_time = event_time + float_to_dur(e.duration);
            std::this_thread::sleep_until(event_time);
        }
        std::cout << "OFF" << std::endl;
    }
    return 0;
}
