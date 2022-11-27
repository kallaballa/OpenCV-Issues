#define CL_TARGET_OPENCL_VERSION 120

constexpr const int VA_HW_DEVICE_INDEX = 0;

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <string>

using std::cerr;
using std::endl;
using std::string;

int main(int argc, char **argv) {
    if(argc != 2) {
        cerr << "Usage: v4l2-vaapi <video-file/device-file>" << endl;
        exit(1);
    }

    //Initialize HW decoding using VAAPI
    cv::VideoCapture capture(argv[1], cv::CAP_FFMPEG, {
            cv::CAP_PROP_HW_DEVICE, VA_HW_DEVICE_INDEX,
            cv::CAP_PROP_HW_ACCELERATION, cv::VIDEO_ACCELERATION_VAAPI,
            cv::CAP_PROP_HW_ACCELERATION_USE_OPENCL, 1
    });
    // check if we succeeded
    if (!capture.isOpened()) {
        cerr << "ERROR! Unable to open video-input" << endl;
        return -1;
    }

    double fps = capture.get(cv::CAP_PROP_FPS);
    cerr << "FPS from stream: " << fps << endl;

    uint64_t cnt = 1;
    double lastFps = fps;
    cv::TickMeter tick;

    cv::UMat videoFrame;
    while (true) {
        tick.start();
        //Decode a frame on the GPU using VAAPI
        capture >> videoFrame;
        if (videoFrame.empty()) {
            cerr << endl << "End of stream. Exiting" << endl;
            break;
        }
        tick.stop();
        //Print FPS
        if (cnt % uint64(ceil(lastFps)) == 0) {
            lastFps = tick.getFPS();
            cerr << "FPS : " << lastFps << '\r';
            cnt = 1;
        }

        ++cnt;
    }

    return 0;
}
