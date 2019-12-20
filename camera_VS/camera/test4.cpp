//�t�@�C���p�X���w�肵�Ċ�F�؂�����(�Ԃ����Ŋ炪�͂܂��)

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <vector>

using namespace cv;
using namespace std;

int main()
{
	CascadeClassifier cascade;
	cascade.load("C:/opencv/sources/data/haarcascades/haarcascade_frontalface_alt.xml"); //���ʊ��񂪓����Ă���J�X�P�[�h�t�@�C���ǂݍ���
	Mat img = imread("C:/Users/nwuser/Desktop/Chiania/camera/camera/lena.jpg", IMREAD_UNCHANGED); //���͉摜�ǂݍ���
	vector<Rect> faces; //�֊s�����i�[�ꏊ
	cascade.detectMultiScale(img, faces, 1.1, 3, 0, Size(20, 20)); //�J�X�P�[�h�t�@�C���Ɋ�Â��Ċ�����m����D���m����������x�N�g��faces�Ɋi�[

	for (int i = 0; i < faces.size(); i++) //���o������̌�"faces.size()"�����[�v���s��
	{
		rectangle(img, Point(faces[i].x, faces[i].y), Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height), Scalar(0, 0, 255), 3, CV_AA); //���o�������ԐF��`�ň͂�
	}

	imshow("detect face", img);
	waitKey(0);
}