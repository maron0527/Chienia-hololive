#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <vector>

using namespace cv;
using namespace std;

int main()
{
	CascadeClassifier cascade;
	CascadeClassifier cascade2;
	cascade.load("C:/opencv/sources/data/haarcascades/haarcascade_frontalface_alt.xml"); //正面顔情報が入っているカスケードファイル読み込み
	cascade2.load("C:/opencv/sources/data/haarcascades/haarcascade_eye_tree_eyeglasses.xml");
	Mat img = imread("C:/Users/nwuser/Desktop/Chiania/camera/camera/lena.jpg", IMREAD_UNCHANGED); //入力画像読み込み
	vector<Rect> faces; //輪郭情報を格納場所
	vector<Rect> eyes; //輪郭情報を格納場所
	cascade.detectMultiScale(img, faces, 1.1, 3, 0, Size(20, 20)); //カスケードファイルに基づいて顔を検知する．検知した顔情報をベクトルfacesに格納
	cascade2.detectMultiScale(img, eyes, 1.1, 3, 0, Size(20, 20)); //カスケードファイルに基づいて顔を検知する．検知した顔情報をベクトルfacesに格納
	for (int i = 0; i < faces.size(); i++) //検出した顔の個数"faces.size()"分ループを行う
	{
		rectangle(img, Point(faces[i].x, faces[i].y), Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height), Scalar(0, 0, 255), 3, CV_AA); //検出した顔を赤色矩形で囲む
	}
	for (int i = 0; i < eyes.size(); i++) //検出した顔の個数"faces.size()"分ループを行う
	{
		rectangle(img, Point(eyes[i].x, eyes[i].y), Point(eyes[i].x + eyes[i].width, eyes[i].y + eyes[i].height), Scalar(255, 0, 0), 3, CV_AA); //検出した顔を赤色矩形で囲む
	}

	imshow("detect face", img);
	waitKey(0);
}