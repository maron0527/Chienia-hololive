#include <dlib/opencv.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <array>
#include <vector>
#include <cmath>  
#include <string>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include "picojson.h"

using namespace dlib;
using namespace std;
using namespace cv;


void drawObjects(cv::Mat& image, std::vector<cv::Rect>& objects, cv::Scalar color, CvPoint offset = cvPoint(0, 0), bool isfill = false) {
	float thickness = 1;
	if (isfill) thickness = -1;
	for (auto r : objects) {

		cv::rectangle(image,
			cvPoint(r.x + offset.x, r.y + offset.y),
			cvPoint(r.x + offset.x + r.width, r.y + offset.y + r.height),
			color, thickness);
	}
}

const std::vector<int> eyes_L = { 37,38,39,40,41,36 };
const std::vector<int> eyes_R = { 43,44,45,46,47,42 };

class face
{
public:
	bool is_close_L;
	bool is_close_R;
	float horizontal_pos;
	float vertical_pos;
	float horizontal_rot;
	float vertical_rot;
};

bool is_close(double upper_lid, double lower_lid) {
	if (abs(upper_lid - lower_lid) < 10) {
		return true;
	}
	else {
		return false;
	}
}
void color_categorize(cv::Mat& image, std::vector<cv::Rect>& objects, cv::Scalar color, const int* array, int array_size) {

	for (int i = 0; i < array_size; i++) {

		cv::rectangle(image,
			cvPoint(objects[array[i]].x, objects[array[i]].y),
			cvPoint(objects[array[i]].x + objects[array[i]].width, objects[array[i]].y + objects[array[i]].height),
			color);

	}
}

void showText(cv::Mat& image, int x, int y, string text) {
	cv::putText(image, text, cv::Point(200 * x + 50, 100 * y + 50), cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0, 0, 200), 2, CV_AA);
}

void face_points(cv::Mat& image, std::vector<cv::Rect>& objects) {
	showText(image, 0, 0, is_close(objects[37].y, objects[41].y, 0));
	showText(image, 0, 1, is_close(objects[44].y, objects[46].y, 1));
}


int main(int argc, char **argv) {
	face actor;
	int sock;
	struct sockaddr_in addr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(22222);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	sendto(sock, "HELLO", 5, 0, (struct sockaddr *)&addr, sizeof(addr));

	std::string path(argv[0]);
	const std::string RESOURCE_DIR = path.substr(0, path.find_last_of("\\/") + 1);
	const std::string FACE_LANDMARK_MODEL = "shape_predictor_68_face_landmarks.dat";

	dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();
	dlib::shape_predictor pose_model;
	dlib::deserialize(RESOURCE_DIR + FACE_LANDMARK_MODEL) >> pose_model;

	const double w = 480, h = 320;
	int cam_num;
	cout << "input camera num-->"; cin >> cam_num;
	getchar();
	cv::VideoCapture capture(cam_num);
	capture.set(CV_CAP_PROP_FRAME_WIDTH, w);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, h);

	cvNamedWindow("Capture", CV_WINDOW_NORMAL);

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
			std::vector<cv::Rect> face_rect;
			// 点のサイズ
			const int s = 2;

			for (auto shape : shapes) {
				auto rect = shape.get_rect();

				face_rect.push_back((cv::Rect) {
					(int)rect.left(), (int)rect.top(),
						(int)rect.width(), (int)rect.height()
				});

				for (auto i = 0; i < shape.num_parts(); i++) {
					auto part = shape.part(i);

					objects.push_back((cv::Rect) {
						(int)part.x(), (int)part.y(),
							s, s
					});
				}
			}

			if (objects.size() > 0) {
				std::vector<cv::Point2d> image_points;
				std::vector<cv::Point3d> model_points;
				drawObjects(frame, face_rect, cv::Scalar(255, 255, 255), false);
				drawObjects(frame, objects, cv::Scalar(255, 255, 0), false);
				face_points(frame, objects);
				color_categorize(frame, objects, cv::Scalar(0, 0, 255), eyes_L.data(), eyes_L.size());
				color_categorize(frame, objects, cv::Scalar(0, 255, 0), eyes_R.data(), eyes_R.size());
				actor.is_close_L = is_close(objects[37].y, objects[41].y);
				actor.is_close_R = is_close(objects[44].y, objects[46].y);
				actor.horizontal_pos = (float)(face_rect[0].x + face_rect[0].width / 2) / 1280;
				actor.vertical_pos = (float)(face_rect[0].y + face_rect[0].height / 2) / 720;
				showText(frame, 0, 2, to_string((float)(face_rect[0].x + face_rect[0].width / 2) / 1280));
				showText(frame, 0, 3, to_string((float)(face_rect[0].y + face_rect[0].height / 2) / 720));

				double focal_length = frame.cols;
				cv::Point2d center = cv::Point2d(frame.cols / 2, frame.rows / 2);
				cv::Mat camera_matrix = (cv::Mat_<double>(3, 3) << focal_length, 0, center.x, 0, focal_length, center.y, 0, 0, 1);
				cv::Mat dist_coeffs = cv::Mat::zeros(4, 1, cv::DataType<double>::type); // 歪なし
				// Output rotation and translation
				cv::Mat rotation_vector; // Rotation in axis-angle form
				cv::Mat translation_vector;
				model_points.push_back(cv::Point3d(0.0f, 0.0f, 0.0f));               // 鼻先
				model_points.push_back(cv::Point3d(0.0f, -330.0f, -65.0f));          // 顎
				model_points.push_back(cv::Point3d(-225.0f, 170.0f, -135.0f));       // 左目端点
				model_points.push_back(cv::Point3d(225.0f, 170.0f, -135.0f));        // 右目端点
				model_points.push_back(cv::Point3d(-150.0f, -150.0f, -125.0f));      // 口左端
				model_points.push_back(cv::Point3d(150.0f, -150.0f, -125.0f));       // 口右端

				image_points.push_back(cv::Point2d(objects[30].x, objects[30].y));    // Nose tip
				image_points.push_back(cv::Point2d(objects[8].x, objects[8].y));    // Chin
				image_points.push_back(cv::Point2d(objects[45].x, objects[45].y));     // Left eye left corner
				image_points.push_back(cv::Point2d(objects[36].x, objects[36].y));    // Right eye right corner
				image_points.push_back(cv::Point2d(objects[54].x, objects[54].y));    // Left Mouth corner
				image_points.push_back(cv::Point2d(objects[48].x, objects[48].y));    // Right mouth corner

				cv::solvePnP(model_points, image_points, camera_matrix, dist_coeffs, rotation_vector, translation_vector);
				std::vector<Point3d> nose_end_point3D;
				std::vector<Point2d> nose_end_point2D;
				nose_end_point3D.push_back(Point3d(0, 0, 1000.0));

				projectPoints(nose_end_point3D, rotation_vector, translation_vector, camera_matrix, dist_coeffs, nose_end_point2D);
				for (int i = 0; i < image_points.size(); i++)
				{
					circle(frame, image_points[i], 3, Scalar(0, 0, 255), -1);
				}
				//cout << "Rotation Vector " << endl << rotation_vector << endl;
				//actor.horizontal_rot = rotation_vector.at<int>(0, 0);
				//actor.vertical_rot = rotation_vector.at<int>(0, 1);
				actor.horizontal_rot = nose_end_point2D[0].x / 1280;
				actor.vertical_rot = 1 - nose_end_point2D[0].y / 720;
				cv::line(frame, image_points[0], nose_end_point2D[0], cv::Scalar(255, 0, 0), 2);

				showText(frame, 1, 0, to_string(actor.horizontal_rot));
				showText(frame, 1, 1, to_string(actor.vertical_rot));

				picojson::object obj;
				obj.emplace(std::make_pair("is_close_L", actor.is_close_L));
				obj.emplace(std::make_pair("is_close_R", actor.is_close_R));
				obj.emplace(std::make_pair("horizontal_rot", actor.horizontal_rot));
				obj.emplace(std::make_pair("vertical_rot", actor.vertical_rot));
				obj.emplace(std::make_pair("horizontal_pos", actor.horizontal_pos));
				obj.emplace(std::make_pair("vertical_pos", actor.vertical_pos));
				// 文字列にするためにvalueを使用
				picojson::value val(obj);

				// return std::string
				std::string msg = val.serialize();

				//std::cout << val.serialize(); 

				sendto(sock, msg.c_str(), msg.size(), 0, (struct sockaddr *)&addr, sizeof(addr));

			}
		}
		cv::Mat view;
		cv::resize(frame, view, Size(), 0.5, 0.5);
		cv::imshow("Capture", view);
		auto c = cvWaitKey(2);
		if (c == '\x1b') {
			break;
		}
	}
	close(sock);
	cvDestroyWindow("Capture");

	return 0;
}