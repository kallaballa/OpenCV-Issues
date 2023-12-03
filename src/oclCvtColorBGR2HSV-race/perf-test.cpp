#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>

#include <iostream>

int main() {
	int maxWidth = 1920;
	int maxHeight = 1080;
	int checkersSize = 60;
	int iterations = 100000;

	CV_Assert(maxWidth % checkersSize == 0 && maxHeight % checkersSize == 0);
	int shorterSide = std::min(maxWidth, maxHeight);
	int numImages = shorterSide / checkersSize - 1;

	std::cerr << "Generating test images" << std::endl;
	std::vector<cv::UMat> test_images(numImages);
	for (size_t i = 0; i < numImages; ++i) {
		int rows = (checkersSize * (i + 1));
		int cols = (checkersSize * (i + 1));

		cv::UMat &image = test_images[i];
		image.create(rows, cols, CV_8UC3);
		cv::Mat mat = image.getMat(cv::ACCESS_WRITE);
		cv::Vec3b color;
		for (int i = 0; i < mat.rows; i++) {
			for (int j = 0; j < mat.cols; j++) {
				if ((i / checkersSize) % 3 == (j / checkersSize) % 3) {
					color = cv::Vec3b(0, 0, 255);
				} else if ((i / checkersSize) % 2 != (j / checkersSize) % 2) {
					color = cv::Vec3b(0, 255, 0);
				} else {
					color = cv::Vec3b(255, 0, 0);
				}

				mat.at<cv::Vec3b>(cv::Point(j, i)) = color;
			}
		}
	}
	std::cerr << "Starting performance test" << std::endl;
	int64 before = cv::getTickCount();
	std::vector<cv::UMat> result_images(numImages);
	for (size_t i = 0; i < iterations; ++i) {
		for (size_t j = 0; j < numImages; ++j) {
			cv::cvtColor(test_images[j], result_images[j], cv::COLOR_BGR2HSV);
			//save them for inspection
//			cv::imwrite(std::to_string(i) + "result.png", result_images[j]);
		}
	}
	int64 after = cv::getTickCount();
	std::cerr << "Duration: " << ((after - before) / cv::getTickFrequency())
			<< "s" << std::endl;
	return 0;
}

