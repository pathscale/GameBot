#include <fstream>
#include <vector>
#include <chrono>
#include "include/csvpp.h"


struct event {
    int x;
    int y;
    float start; // sec
    float duration; // sec
};

const event read_event(csvpp::RowReader &rd);
const std::vector<event> read_events(std::istream &i);
std::chrono::milliseconds float_to_dur(float dur);
