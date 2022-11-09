#define CL_TARGET_OPENCL_VERSION 120

constexpr long unsigned int WIDTH = 1920;
constexpr long unsigned int HEIGHT = 1080;
constexpr const int VA_HW_DEVICE_INDEX = 0;

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/ocl.hpp>


using std::cerr;
using std::endl;


int main(int argc, char **argv) {
    if(argc != 1) {
        cerr << "Usage: hls-hsv-opencl" << endl;
        exit(1);
    }
    cv::ocl::setUseOpenCL(true);

    cv::UMat red(1080,1920,CV_8UC3, cv::Scalar(0,0,255));
    cv::UMat hls;
    cv::UMat hsv;

    cv::cvtColor(red, hls, cv::COLOR_BGR2HLS);
    cv::cvtColor(hls, red, cv::COLOR_HLS2BGR);

    cv::imshow("result", red);
    cv::waitKey(0);
    return 0;
}
