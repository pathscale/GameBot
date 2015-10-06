This is the device component, used to record live screen and cast it over USB.

Build requirements:

- Android NDK, standalone https://developer.android.com/ndk/guides/standalone_toolchain.html [~/arm-21-toolchain/bin/]
- OpenCV for Android [../sources/OpenCV-native/]
- .so binaries for device
    ./adb pull /system/lib/ [./lib/]
- Android headers appropriate to .so files (platform/core, libhardware, frameworks/base, frameworks/native) [/home/gamer/sources/]

Build commands:

$ ~/arm-21-toolchain/bin/arm-linux-androideabi-g++ -std=c++11 -pie -fPIE sockserv.cpp -c -o sockserv.o
$ ~/arm-21-toolchain/bin/arm-linux-androideabi-g++ -pie -fPIE -std=c++11 -I /home/gamer/sources/native/include/ -I /home/gamer/sources/core/include/ -I /home/gamer/sources/libhardware/include -I /home/gamer/sources/OpenCV-native/jni/include/ screencap_2.cpp sockserv.o lib/libgui.so lib/libutils.so lib/libbinder.so lib/libcutils.so ../sources/OpenCV-native/libs/armeabi-v7a/libopencv_imgproc.a ../sources/OpenCV-native/libs/armeabi-v7a/libopencv_core.a ../sources/OpenCV-native/libs/armeabi-v7a/libopencv_hal.a ../sources/OpenCV-native/3rdparty/libs/armeabi-v7a/* lib/libz.so -o arm.out

Server runs on port 5645

Run commands:

$ adb forward tcp:9999 tcp:5645
$ adb push ./arm.out /data/local/tmp/
$ adb shell /data/local/tmp/arm.out
