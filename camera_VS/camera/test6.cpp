#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

int main() {
  // �J�����̋N��
	cv::Mat img;

	img = cv::imread("lena.jpg", 1);

  VideoCapture cap(0);
  CascadeClassifier cascade;
  cascade.load("haarcascade_eye_tree_eyeglasses.xml");
  vector<Rect> faces;
  Mat frame;
  while (1) {
    // �J��������擾�����摜���t���[���ɗ��Ƃ�����
    cap >> frame;  
    // �w�肵�����ʊ��p���ĉ摜����Ώۂ����o
    cascade.detectMultiScale(frame, faces, 1.1,3,0,Size(20,20));

    // ���o�����Ώۂ��͂ދ�`�����摜�ɕ`��
    /*for (int i = 0; i < faces.size(); i++){
      rectangle(frame, Point(faces[i].x,faces[i].y),Point(faces[i].x + faces[i].width,faces[i].y + faces[i].height),Scalar(0,200,0),3,CV_AA);
    }*/
    imshow("Detect Faces", frame);
    int key = waitKey(10);
    if (key == 27) {//esc�������ꂽ��I��
      break;
    }
  }
  return 0;
}