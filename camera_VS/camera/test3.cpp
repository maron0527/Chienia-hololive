//�}�`�`��

#include <opencv2/opencv.hpp>

using namespace std;

int main(void)
{
	// �摜���i�[����I�u�W�F�N�g��錾����
	// # CV_8UC3 : 8bit 3channel �t���J���[�摜
	cv::Mat	image(400, 400, CV_8UC3);

	// �摜�𔒐F�œh��Ԃ�
	image = cv::Scalar(255, 255, 255);

	// ���p�`�̒��_��ݒ肷��
	vector<cv::Point> p;
	int n = 5;
	for (int i = 0; i < n; i++) {
		p.push_back(cv::Point(cos(CV_PI * (2.0*i / n - 0.5)) * 180 + 200, sin(CV_PI * (2.0*i / n - 0.5)) * 180 + 200));
	}

	// ���p�`��`��
	// # polylines(�摜, ���_�W��, ���p�`����邩, �F, ����, �A��)
	cv::polylines(image, p, true, cv::Scalar(255, 0, 0), 2, cv::LINE_AA);

	p.clear();
	for (int i = 0; i < n; i++) {
		p.push_back(cv::Point(cos(CV_PI * (2.0*i / n - 0.5)) * 140 + 200, sin(CV_PI * (2.0*i / n - 0.5)) * 140 + 200));
	}

	// ����h��Ԃ������p�`��`��
	// # fillConvexPoly(�摜, ���_�W��, �F, �A��)
	cv::fillConvexPoly(image, p, cv::Scalar(0, 0, 255), cv::LINE_AA);

	// fillPoly���g���ꍇ�͎��̂悤�ɂ���
	//  vector<vector<cv::Point>> pp;
	//  pp.push_back(p);
	//  cv::fillPoly(image, pp, cv::Scalar(0, 0, 255), cv::LINE_AA);

	cv::imshow("���p�`�`��", image);
	cv::waitKey();

	// �����̑��p�`����x�ɕ`��

	// ���p�`�̒��_��ݒ肷��
	vector<vector<cv::Point>> ps(16);
	for (int n = 0; n < 16; n++) {
		cv::Point p0((n % 4) * 100 + 50, (n / 4) * 100 + 50);
		for (int i = 0; i < n + 3; i++) {
			cv::Point pa(cos(CV_PI * (2.0*i / (n + 3) - 0.5)) * 40, sin(CV_PI * (2.0*i / (n + 3) - 0.5)) * 40);
			ps[n].push_back(p0 + pa);
		}
	}

	image = cv::Scalar(255, 255, 255);

	// ���p�`��`��
	cv::fillPoly(image, ps, cv::Scalar(0, 255, 255), cv::LINE_AA);		// ���F�œh��Ԃ������p�`
	cv::polylines(image, ps, true, cv::Scalar(0, 0, 0), 3, cv::LINE_AA);// ���F�̐��̑��p�`

	cv::imshow("���p�`�`��", image);
	cv::waitKey();

	return 0;
}