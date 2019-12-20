#include <opencv2/opencv.hpp>

int main(void)
{
	// �f������荞�ނ��߂̃I�u�W�F�N�g��錾����
	cv::VideoCapture cap;

	// �J�����ɐڑ�����
	// # open �̈����ɃJ�����ԍ����w�肷��i�ʏ��0�j
	cap.open(1);
	if (cap.isOpened() == false) {
		// �J�����̐ڑ����ł��Ȃ������Ƃ��͏I������
		printf("�ڑ����s");
		cv::waitKey();
		return 0;
	}

	// �摜�����擾
	int    width = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH);		// �t���[���������擾
	int    height = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT);	// �t���[���c�����擾

	std::cout << "�摜�T�C�Y " << width << "x" << height << std::endl;

	// �摜���i�[����I�u�W�F�N�g��錾����
	cv::Mat frame;

	for (;;) {
		// 1�t���[������荞��
		cap >> frame;

		// �摜�����̂Ƃ��A�������[�v�𔲂���
		if (frame.empty() == true) {
			printf("�摜����");
			cv::waitKey();
			break;
		}

		// �E�B���h�E�ɉ摜��\������
		cv::imshow("���͒�", frame);

		// 30�~���b�҂�
		int key = cv::waitKey(0);

		// �����L�[�������ꂽ��I������
		if (key > 0) {
			break;
		}
	}

	return 0;
}