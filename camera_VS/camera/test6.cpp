#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

int main() {
  // カメラの起動
	cv::Mat img;

	img = cv::imread("lena.jpg", 1);

  VideoCapture cap(0);
  CascadeClassifier cascade;
  cascade.load("haarcascade_eye_tree_eyeglasses.xml");
  vector<Rect> faces;
  Mat frame;
  while (1) {
    // カメラから取得した画像をフレームに落とし込む
    cap >> frame;  
    // 指定した識別器を用いて画像から対象を検出
    cascade.detectMultiScale(frame, faces, 1.1,3,0,Size(20,20));

    // 検出した対象を囲む矩形を元画像に描画
    /*for (int i = 0; i < faces.size(); i++){
      rectangle(frame, Point(faces[i].x,faces[i].y),Point(faces[i].x + faces[i].width,faces[i].y + faces[i].height),Scalar(0,200,0),3,CV_AA);
    }*/
    imshow("Detect Faces", frame);
    int key = waitKey(10);
    if (key == 27) {//escが押されたら終了
      break;
    }
  }
  return 0;
}