#define CL_TARGET_OPENCL_VERSION 120

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/objdetect/objdetect.hpp>

using std::cerr;
using std::endl;

int main(int argc, char **argv) {
    if (argc != 1) {
        cerr << "Usage: hls-hsv-opencl" << endl;
        exit(1);
    }
    cv::UMat gray(360, 640, CV_8UC1, cv::Scalar(128));
    std::vector<cv::Rect> locations;
    cv::HOGDescriptor hog;
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());

    uint64_t cnt = 1;
    int64 start = cv::getTickCount();
    double tickFreq = cv::getTickFrequency();
    double lastFps = 5;

    while (true) {
        hog.detectMultiScale(gray, locations);

        //Measure FPS
        if (cnt % uint64(ceil(lastFps)) == 0) {
            int64 tick = cv::getTickCount();
            lastFps = tickFreq / ((tick - start + 1) / cnt);
            cerr << "FPS : " << lastFps << '\r';
            start = tick;
            cnt = 1;
        }

        ++cnt;
    }
    return 0;
}
