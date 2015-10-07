cmake_minimum_required (VERSION 2.8.11)
project (InputPlayer)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(OpenCV)

add_executable(simple_player WIN32
	simple_player.cpp
	reader.cpp
	interact.cpp
	csvpp/csvpp.cpp
	csvpp/stringhelper.cpp)

add_executable(cookie_player
	cookie_player.cpp
	reader.cpp
	interact.cpp
	csvpp/csvpp.cpp
	csvpp/stringhelper.cpp)

add_executable(stream_reader
	stream_reader.cpp)

target_link_libraries(cookie_player
	opencv_core
    opencv_imgcodecs)

target_link_libraries(stream_reader
	opencv_core
    opencv_imgcodecs)
