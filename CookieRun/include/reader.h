#include <fstream>
#include <vector>
#include "include/csvpp.h"
#include "include/common.h"

const event read_event(csvpp::RowReader &rd);
const std::vector<event> read_events(std::istream &i);
