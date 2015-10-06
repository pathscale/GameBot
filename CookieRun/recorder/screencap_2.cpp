#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>
#include <ui/PixelFormat.h>
#include <binder/IBinder.h>

#include <binder/MemoryHeapBase.h> // not implicitly pulled by any other header, lack results in compile error

#include "sockserv.hpp"
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <chrono> // benchmarking only

/*
 build command monster
 $ ~/arm-21-toolchain/bin/arm-linux-androideabi-g++ -pie -fPIE -std=c++11 -I /home/gamer/sources/native/include/ -I /home/gamer/sources/core/include/ -I /home/gamer/sources/libhardware/include -I /home/gamer/sources/OpenCV-native/jni/include/ screencap_2.cpp sockserv.o lib/libgui.so lib/libutils.so lib/libbinder.so lib/libcutils.so ../sources/OpenCV-native/libs/armeabi-v7a/libopencv_imgproc.a ../sources/OpenCV-native/libs/armeabi-v7a/libopencv_core.a ../sources/OpenCV-native/libs/armeabi-v7a/libopencv_hal.a ../sources/OpenCV-native/3rdparty/libs/armeabi-v7a/* lib/libz.so -o arm.out
 */

struct buffer_config {
    uint32_t size;
    uint32_t xres;
    uint32_t yres;
    uint32_t stride;
    uint32_t format; // FIXME: actually use this
};

int send_header(sserv::Socket &clientsock, struct buffer_config &bconf) {
    int wsize = sizeof(struct buffer_config);
    int w = clientsock.write_((void*)&bconf, wsize);
    return w != wsize;
}

int handle(sserv::Socket &clientsock) {
    std::cerr << "connected" << std::endl;
    android::ScreenshotClient screenshot;
    uint32_t displayId = android::ISurfaceComposer::eDisplayIdMain;
    android::sp<android::IBinder> display = android::SurfaceComposerClient::getBuiltInDisplay(displayId);
    if (display == NULL) {
        std::cerr << "display not detected" << std::endl;
        return 1;
    }
    
    const float scale_factor = 4; // ~1.2s/full frame / 16 = (1.2s/16)/scaled frame = 75ms/scaled frame
    
    // init
    if (screenshot.update(display, android::Rect(), false) != android::NO_ERROR) {
        std::cerr << "screencap failed" << std::endl;
        return 1;
    }
    // preallocate everything
    struct buffer_config down_conf;
    down_conf.xres = screenshot.getWidth() / scale_factor;
    down_conf.yres = screenshot.getHeight() / scale_factor;
    down_conf.stride = screenshot.getStride() / scale_factor;
    down_conf.format = screenshot.getFormat();
    
    cv::Mat downscaled(down_conf.yres, down_conf.stride, CV_8UC4);
    down_conf.size = downscaled.cols * downscaled.rows * downscaled.elemSize();
    std::cout << "images size " << downscaled.cols * downscaled.rows * downscaled.elemSize() << std::endl;
    
    // start network activity
    if (send_header(clientsock, down_conf)) {
        std::cerr << "couldn't write header" << std::endl;
        return 1;
    }
    
    //int iters = 200;
    // actual sending loop
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::milliseconds total(0);
    std::chrono::milliseconds send(0);
    //for (int i = iters; i > 0; i--) {
    while (1) {
        if (screenshot.update(display, android::Rect(), false) != android::NO_ERROR) {
            std::cerr << "screencap failed" << std::endl;
            return 1;
        }
        //std::cout << "snap!" << i << std::endl;
        
        const void *base = screenshot.getPixels();
        const cv::Mat m(screenshot.getHeight(), screenshot.getStride(), CV_8UC4, (void*)base); // hack warning, apparently cv::Mat doesn't like to take const void*
        // hack warning 2: channel data not decoded
        /*
        cv::Mat noalpha(m.size(), CV_8UC3);
        int from_to[] = {0, 0,
                         1, 1,
                         2, 2};
        cv::mixChannels(&m, 1, &noalpha, 1, from_to, 3);*/
        // scale
        cv::resize(m, downscaled, downscaled.size());
        
        std::chrono::steady_clock::time_point send_begin = std::chrono::steady_clock::now();
        int w = clientsock.write_(downscaled.data, down_conf.size);
        send += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - send_begin);
        if (w != down_conf.size) {
            std::cerr << "didn't send image" << std::endl;
            return 1;
        }
    }
    /*
    total += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin);
    std::cout << "finished " << iters << " iters in " << total.count() / 1000. << "s (" << iters * 1000. / total.count() << "fps)" << std::endl;
    std::cout << "sent " << iters * down_conf.size << " bytes in " << send.count() / 1000. << "s, giving the rate of " << iters * down_conf.size / (1024. * 1024) / (send.count() / 1000.) <<" MiB/s" << std::endl;*/
    return 0;
}


int main(void) {
    sserv::SocketServer ss = sserv::get_bound_sock(5645);
    if (ss.error()) {
        std::cerr << "can't bind" << std::endl;
        return 1;
    }
    std::cerr << "bound" << std::endl;
    while (1) {
        std::cout << "serving" << std::endl;
        sserv::Socket sock = sserv::accept(ss);
        if (sock.error()) {
            std::cerr << "can't accept" << std::endl;
            return 1;
        }
        int res = handle(sock);
        if (res) {
            std::cout << "handle ended with" << res << std::endl;
        }
        res = sserv::close_(sock);
        if (res) {
            return res;
        }
        std::cout << "ended" << std::endl;
    }
    res = sserv::close_(ss);
    std::cout << "exiting" << std::endl;
    return res;
}
