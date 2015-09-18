cmake_minimum_required (VERSION 2.8.11)
project (InputPlayer)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(player WIN32
	reader.cpp
	csvpp/csvpp.cpp
	csvpp/stringhelper.cpp)
