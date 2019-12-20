#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <vector>

using namespace cv;
using namespace std;

int main()
{
	Mat frame; //USBカメラから得た１フレームを格納する場所
	CascadeClassifier cascade; //カスケード分類器格納場所
	cascade.load("C:/opencv/sources/data/haarcascades/haarcascade_frontalface_alt.xml"); //正面顔情報が入っているカスケード
	vector<Rect> faces; //輪郭情報を格納場所

	VideoCapture cap(0); // USBカメラのオープン
	if (!cap.isOpened()) //カメラが起動できなかった時のエラー処理
	{
		return -1;
	}

	while (1)//無限ループ
	{
		cap >> frame; //USBカメラが得た動画の１フレームを格納
		cascade.detectMultiScale(frame, faces, 1.1, 3, 0, Size(20, 20)); //格納されたフレームに対してカスケードファイルに基づいて顔を検知
		for (int i = 0; i < faces.size(); i++) //検出した顔の個数"faces.size()"分ループを行う
		{
			rectangle(frame, Point(faces[i].x, faces[i].y), Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height), Scalar(0, 0, 255), 3, CV_AA); //検出した顔を赤色矩形で囲む
		}

		imshow("window", frame);//画像を表示．

		int key = waitKey(1);
		if (key == 113)//qボタンが押されたとき
		{
			break;//whileループから抜ける（終了）
		}
	}
	destroyAllWindows();
	return 0;
}