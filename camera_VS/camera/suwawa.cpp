#include <opencv2/opencv.hpp>
#include <iostream>

int main()
{
	cv::Mat img, dst;

	img = cv::imread("vol5-1.jpg");
	cv::cvtColor(img, dst, cv::COLOR_BGR2GRAY);

	std::cout << dst(cv::Rect(95, 115, 30, 40)) << std::endl;

	return 0;
}