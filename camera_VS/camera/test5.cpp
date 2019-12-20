#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <vector>

using namespace cv;
using namespace std;

int main()
{
	Mat frame; //USB�J�������瓾���P�t���[�����i�[����ꏊ
	CascadeClassifier cascade; //�J�X�P�[�h���ފ�i�[�ꏊ
	cascade.load("C:/opencv/sources/data/haarcascades/haarcascade_frontalface_alt.xml"); //���ʊ��񂪓����Ă���J�X�P�[�h
	vector<Rect> faces; //�֊s�����i�[�ꏊ

	VideoCapture cap(0); // USB�J�����̃I�[�v��
	if (!cap.isOpened()) //�J�������N���ł��Ȃ��������̃G���[����
	{
		return -1;
	}

	while (1)//�������[�v
	{
		cap >> frame; //USB�J��������������̂P�t���[�����i�[
		cascade.detectMultiScale(frame, faces, 1.1, 3, 0, Size(20, 20)); //�i�[���ꂽ�t���[���ɑ΂��ăJ�X�P�[�h�t�@�C���Ɋ�Â��Ċ�����m
		for (int i = 0; i < faces.size(); i++) //���o������̌�"faces.size()"�����[�v���s��
		{
			rectangle(frame, Point(faces[i].x, faces[i].y), Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height), Scalar(0, 0, 255), 3, CV_AA); //���o�������ԐF��`�ň͂�
		}

		imshow("window", frame);//�摜��\���D

		int key = waitKey(1);
		if (key == 113)//q�{�^���������ꂽ�Ƃ�
		{
			break;//while���[�v���甲����i�I���j
		}
	}
	destroyAllWindows();
	return 0;
}