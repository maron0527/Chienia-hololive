//�w��摜�̕\��(lena�Ƃ����������\�������)

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

int main(void) {
	cv::Mat img;

	img = cv::imread("lena.jpg", 1);

	cv::namedWindow("image");
	cv::imshow("image", img);
	cv::waitKey(0);

	cv::destroyAllWindows();

	return 0;
}