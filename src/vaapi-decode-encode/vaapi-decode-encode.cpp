#define CL_TARGET_OPENCL_VERSION 120

constexpr const int VA_HW_DEVICE_INDEX = 0;
constexpr const char* OUTPUT_FILENAME = "vaapi-decode-encode.mkv";

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <string>

using std::cerr;
using std::endl;
using std::string;

int main(int argc, char **argv) {
    if(argc != 2) {
        cerr << "Usage: vaapi-decode-encode <video-file/device-file>" << endl;
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
        cerr << "ERROR! Unable to open camera" << endl;
        return -1;
    }

    double fps = capture.get(cv::CAP_PROP_FPS);
    double width = capture.get(cv::CAP_PROP_FRAME_WIDTH);
    double height = capture.get(cv::CAP_PROP_FRAME_HEIGHT);

    //Initialize VP9 HW encoding using VAAPI
    cv::VideoWriter writer(OUTPUT_FILENAME, cv::CAP_FFMPEG, cv::VideoWriter::fourcc('V', 'P', '9', '0'), fps, cv::Size(width, height), {
            cv::VIDEOWRITER_PROP_HW_ACCELERATION, cv::VIDEO_ACCELERATION_VAAPI,
            cv::VIDEOWRITER_PROP_HW_ACCELERATION_USE_OPENCL, 1
    });

    cerr << "FPS from stream: " << fps << endl;

    uint64_t cnt = 1;
    int64 start = cv::getTickCount();
    double tickFreq = cv::getTickFrequency();
    double lastFps = fps;

    cv::UMat videoFrame;
    while (true) {
        //Decode a frame on the GPU using VAAPI
        capture >> videoFrame;
        if (videoFrame.empty()) {
            cerr << endl << "End of stream. Exiting" << endl;
            break;
        }

        writer << videoFrame;

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
