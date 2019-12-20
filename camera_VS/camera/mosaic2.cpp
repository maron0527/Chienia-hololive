//���A���^�C���Ń��U�C�N����������
#include <vector>
#include <string>
#include <iostream>
#include <opencv2/opencv.hpp>


cv::CascadeClassifier classifier;
const char * const file = "C:/opencv/sources/data/haarcascades/haarcascade_frontalface_alt2.xml";
cv::Size min_face;

// ����������
void init(int w, int h)
{
	std::cout << "frame=" << w << 'x' << h << std::endl;

	if (!classifier.load(file)) {
		std::cout << "fail to load: " << file << std::endl;
		std::exit(1);
	}
	const double scale = 0.10;
	min_face = cv::Size(w * scale, h * scale);
	std::cout << "min_face=" << min_face << std::endl;
}

//�v���Z�X�摜�t���[��
cv::Mat process(cv::Mat& frame)
{
	std::vector<cv::Rect> faces;
	classifier.detectMultiScale(frame, faces, 1.1, 3, 0, min_face);
	for (const auto& r : faces) {
		// ���U�C�N�t�B���^�[
		const double f = 0.1; //���U�C�N�̑e��
		cv::Mat tmp;
		cv::resize(frame(r), tmp, {}, f, f);//�k��
		cv::resize(tmp, frame(r), r.size(), 0, 0, cv::INTER_NEAREST);//�k���������̂��g��
		// �G���A���E����`��
		cv::rectangle(frame, r.tl(), r.br(), { 0,0,0 });
	}
	return frame;
}

int main()
{
	//�E�C���h�E�̃T�C�Y
	const char * const window = "Capture (Press ESC to exit)";
	const double scale = 0.75;
	const int delay = 32;   // [msec]

	cv::VideoCapture cap(0);//�J����
	if (!cap.isOpened()) {
		std::cerr << "fail to open cv::VideoCapture" << std::endl;
		return 2;
	}

	const double width = cap.get(cv::CAP_PROP_FRAME_WIDTH) * scale;
	const double height = cap.get(cv::CAP_PROP_FRAME_HEIGHT) * scale;
	cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
	init(width, height);

	cv::namedWindow(window);
	cv::Mat frame;
	do {
		cap >> frame;
		cv::imshow(window, process(frame));
	} while (cv::waitKey(delay) != '\x1b');

	return 0;
}