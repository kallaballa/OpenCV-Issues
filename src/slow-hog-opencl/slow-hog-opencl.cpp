#define CL_TARGET_OPENCL_VERSION 120

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/objdetect/objdetect.hpp>

using std::cerr;
using std::endl;

int main(int argc, char **argv) {
    if (argc != 1) {
        cerr << "Usage: slow-hog-opencl" << endl;
        exit(1);
    }
    cv::UMat gray(360, 640, CV_8UC1, cv::Scalar(128));
    std::vector<cv::Rect> locations;
    cv::HOGDescriptor hog;
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());

    uint64_t cnt = 1;
    double lastFps = 5;
    cv::TickMeter tick;
    while (true) {
        tick.start();

        hog.detectMultiScale(gray, locations);

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
