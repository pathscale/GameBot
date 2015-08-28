cmake_minimum_required (VERSION 2.8.11)
project (LevelMe_CoC)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(OpenCV)
find_package(Qt5Qml)
find_package(Qt5Quick)
find_package(Qt5Widgets)

qt5_add_resources(LEVELME_COC_RESOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/qml/qml.qrc
	${CMAKE_CURRENT_SOURCE_DIR}/cutouts/cutouts.qrc)

add_executable(levelme_coc WIN32
	src/botprogram.cpp
	src/cocbattlefield.cpp
	src/main.cpp
	src/resourcemanager.cpp
	src/cutouts.cpp
	${LEVELME_COC_RESOURCES})

set(my_opencv_libs
	opencv_core
	opencv_highgui
        opencv_imgproc
        opencv_imgcodecs)

target_link_libraries(levelme_coc
	${my_opencv_libs}
	Qt5::Qml
	Qt5::Quick
	Qt5::Widgets)
