//�t�@�C���p�X���w�肵�ĉ摜�̕\��(�É����o��)

#include <opencv2/opencv.hpp>

int main(void)
{
	// �摜���i�[����I�u�W�F�N�g��錾����
	cv::Mat	image;

	// �摜�t�@�C������摜�f�[�^��ǂݍ���
	image = cv::imread("C:/opencv/sources/samples/data/heika.png",0);

	if (image.empty() == true) {
		// �摜�f�[�^���ǂݍ��߂Ȃ������Ƃ��͏I������
		return 0;
	}

	// �E�B���h�E�ɉ摜��\������
	// # �����ł̓E�B���h�E�Ɂu�摜�v�Ƃ������ʖ���t���Ă���
	cv::imshow("�摜", image);

	// �����L�[���������܂ő҂�
	cv::waitKey();

	return 0;
}