//
//  main.cpp
//  FaceTracker
//
//  Created by ryohey on 2015/12/11.
//


#include <iostream>
//#include <cv.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
//#include <highgui.h>
#include <ctype.h>


#include "dlib/opencv.h"
#include "dlib/image_processing/frontal_face_detector.h"
#include "dlib/image_processing.h"

using namespace cv;
using namespace std;

void drawObjects(cv::Mat& image, std::vector<cv::Rect>& objects, cv::Scalar color, CvPoint offset = cvPoint(0, 0)) {

	for (auto r : objects) {
		cv::rectangle(image,
			cvPoint(r.x + offset.x, r.y + offset.y),
			cvPoint(r.x + offset.x + r.width, r.y + offset.y + r.height),
			color);
	}
}

int main(int argc, char **argv) {
	std::string path(argv[0]);
	const std::string RESOURCE_DIR = path.substr(0, path.find_last_of("\\/") + 1);
	const std::string FACE_LANDMARK_MODEL = "shape_predictor_68_face_landmarks.dat-1";

	dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
	dlib::shape_predictor pose_model;
	dlib::deserialize(RESOURCE_DIR + FACE_LANDMARK_MODEL) >> pose_model;

	const double w = 720, h = 480;

	cv::VideoCapture capture(0);
	capture.set(CV_CAP_PROP_FRAME_WIDTH, w);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, h);

	cvNamedWindow("Capture", CV_WINDOW_AUTOSIZE);

	while (1)
	{
		cv::Mat frame;
		capture.read(frame);

		// dlib
		{
			dlib::cv_image<dlib::bgr_pixel> cimg(frame);

			// Detect faces
			std::vector<dlib::rectangle> faces = detector(cimg);

			// Find the pose of each face.
			std::vector<dlib::full_object_detection> shapes;

			for (auto face : faces) {

				shapes.push_back(pose_model(cimg, face));
			}

			std::vector<cv::Rect> objects;

			// 点のサイズ
			const int s = 2;

			/*for (auto shape : shapes) {
				auto rect = shape.get_rect();
				objects.push_back((cv::Rect) {
					(int)rect.left(), (int)rect.top(),
						(int)rect.width(), (int)rect.height()
				});

				for (auto i = 0; i < shape.num_parts(); i++) {
					auto part = shape.part(i);

					objects.push_back((cv::Rect) {
						(int)part.x(), (int)part.y(),
							s, s
					
					};
				}
			}*/

			/*if (objects.size() > 0) {
				drawObjects(frame, objects, cv::Scalar(255, 255, 0));
			}*/
		}

		IplImage frame_ = frame;
		cvShowImage("Capture", &frame_);

		auto c = cvWaitKey(2);
		if (c == '\x1b') {
			break;
		}
	}

	cvDestroyWindow("Capture");

	return 0;
}