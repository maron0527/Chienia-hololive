#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include "picojson.h"
#include <windows.h>

//フィルタ用の係数
#define LPF_VALUE_PRE  0.4
#define LPF_VALUE_CUR  (1 - LPF_VALUE_PRE)
#define FACIAL_POINTS   68

//カメラのパラメータ(固定値)
double K[9] = { 6.5308391993466671e+002, 0.0, 3.1950000000000000e+002, 0.0, 6.5308391993466671e+002, 2.3950000000000000e+002, 0.0, 0.0, 1.0 };
double D[5] = { 7.0834633684407095e-002, 6.9140193737175351e-002, 0.0, 0.0, -1.3073460323689292e+000 };

class face
{
public:
	float horizontal_pos;
	float vertical_pos;
	float yaw;
	float pitch;
	float roll;
	float eye_L_X;
	float eye_L_Y;
};

using namespace std;

/* ローカル関数 */
static void DrawFaceBox(cv::Mat frame, std::vector<cv::Point2d> reprojectdst);
static void SetInitialPoints(std::vector<cv::Point3d> * in_BoxPoints, std::vector<cv::Point3d> * in_FaceLandmarkPoints);

int main(int argc, char** argv)
{
	/************ カメラ番号取得 ************/
	int cam_num;
	face actor;
	//cout << "input camera num --> "; cin >> cam_num;
	//getchar();
	//カメラオープン
	cv::VideoCapture cap(0);
	if (!cap.isOpened())
	{
		std::cout << "Unable to connect to camera" << std::endl;
		return EXIT_FAILURE;
	}
	//------------------------------------//

	/************ ソケット初期化 ************/
	int sock;
	WSAData wsaData;
	struct sockaddr_in addr;
	WSAStartup(MAKEWORD(2, 0), &wsaData);
	sock = socket(AF_INET, SOCK_DGRAM, 0);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(22222);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	sendto(sock, "HELLO", 5, 0, (struct sockaddr *)&addr, sizeof(addr));
	//-------------------------------//


	/************ 検出器の初期化************/
	//顔検出器
	//(DlibのAPIに実装されているものをそのまま使用する
	dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();

	//顔器官点検出器
	//外部ファイルから検出器を読み込み(EXEコール時の第1引数)
	dlib::shape_predictor predictor;
	dlib::deserialize("sp.dat") >> predictor;
	//-------------------------------//


	//カメラの組み込み関数と歪み係数を設定
	cv::Mat cam_matrix = cv::Mat(3, 3, CV_64FC1, K);
	cv::Mat dist_coeffs = cv::Mat(5, 1, CV_64FC1, D);

	//顔器官の3D座標初期値
	std::vector<cv::Point3d> object_pts;
	//顔向きを示す矩形の3D座標初期値
	std::vector<cv::Point3d> reprojectsrc;
	//初期値を設定
	SetInitialPoints(&reprojectsrc, &object_pts);

	//result
	cv::Mat rotation_vec;                           //回天ベクトル
	cv::Mat rotation_mat;                           //回天行列
	cv::Mat translation_vec;                        //変換ベクトル
	cv::Mat pose_mat = cv::Mat(3, 4, CV_64FC1);     //姿勢(3X4の配列)
	cv::Mat euler_angle = cv::Mat(3, 1, CV_64FC1);  //オイラー角(3X1の配列)
	cv::Mat prev_euler_angle = cv::Mat(3, 1, CV_64FC1); //前フレームでのオイラー角(3X1の配列)

														//2D座標格納データ
	std::vector<cv::Point2d> reprojectdst;
	reprojectdst.resize(8);

	//顔認識には使用しない
	//使用するAPIの引数を埋めるために使用する
	cv::Mat out_intrinsics = cv::Mat(3, 3, CV_64FC1);
	cv::Mat out_rotation = cv::Mat(3, 3, CV_64FC1);
	cv::Mat out_translation = cv::Mat(3, 1, CV_64FC1);

	//画面表示用テキスト
	std::ostringstream outtext;

	//メインループ
	while (1)
	{
		//カメラから現在の画像フレームを取得
		cv::Mat temp;
		cap >> temp;
		//DlibのAPIで使用するために変換
		dlib::cv_image<dlib::bgr_pixel> cimg(temp);

		//画像データを顔検出器にかける
		std::vector<dlib::rectangle> faces = detector(cimg);

		// 1個以上の顔を検出している場合
		if (faces.size() > 0)
		{
			//そのフレームで検出された最初の顔を、顔器官点検出器にかける
			dlib::full_object_detection shape = predictor(cimg, faces[0]);
			//顔器官点検出器から、顔の領域を取得
			auto rect = shape.get_rect();

			//器官点を描画
			for (unsigned int i = 0; i < FACIAL_POINTS; ++i)
			{
				cv::circle(
					temp,                   //描画先の画像データ
					cv::Point(              //顔器官点の2D点オブジェクト
						shape.part(i).x(),
						shape.part(i).y()),
					2,                      //点のサイズ
					cv::Scalar(0, 0, 255),  //点の色
					-1);
			}

			//画面上の2D座標の一部を格納
			//顔の姿勢検出のため、一部の顔器官点を別の場所に格納する
			std::vector<cv::Point2d> image_pts;
			image_pts.push_back(cv::Point2d(shape.part(17).x(), shape.part(17).y())); //#17 右眉右端
			image_pts.push_back(cv::Point2d(shape.part(21).x(), shape.part(21).y())); //#21 右眉左端
			image_pts.push_back(cv::Point2d(shape.part(22).x(), shape.part(22).y())); //#22 左眉右端
			image_pts.push_back(cv::Point2d(shape.part(26).x(), shape.part(26).y())); //#26 左眉左端

			image_pts.push_back(cv::Point2d(shape.part(36).x(), shape.part(36).y())); //#36 右目右端
			image_pts.push_back(cv::Point2d(shape.part(39).x(), shape.part(39).y())); //#39 右目左端
			image_pts.push_back(cv::Point2d(shape.part(42).x(), shape.part(42).y())); //#42 左目右端
			image_pts.push_back(cv::Point2d(shape.part(45).x(), shape.part(45).y())); //#45 左目左端
			image_pts.push_back(cv::Point2d(shape.part(31).x(), shape.part(31).y())); //#31 鼻右
			image_pts.push_back(cv::Point2d(shape.part(35).x(), shape.part(35).y())); //#35 鼻左
			image_pts.push_back(cv::Point2d(shape.part(48).x(), shape.part(48).y())); //#48 口右
			image_pts.push_back(cv::Point2d(shape.part(54).x(), shape.part(54).y())); //#54 口左
			image_pts.push_back(cv::Point2d(shape.part(57).x(), shape.part(57).y())); //#57 口中央下
			image_pts.push_back(cv::Point2d(shape.part(8).x(), shape.part(8).y()));   //#8  顎

																					  //顔器官点の3D座標初期値、カメラ情報、検出した2D座標から
																					  //顔器官点の3D座標現在値と姿勢情報を算出する
			cv::solvePnP(object_pts, image_pts, cam_matrix, dist_coeffs, rotation_vec, translation_vec);

			//顔向き矩形の3D座標初期値と直前に取得した姿勢情報から
			//顔向き矩形の3D座標現在値を算出する
			cv::projectPoints(reprojectsrc, rotation_vec, translation_vec, cam_matrix, dist_coeffs, reprojectdst);

			//顔向き矩形を描画する
			DrawFaceBox(temp, reprojectdst);

			//顔向き矩形の対角線交点の座標を算出
			cv::Point2d P1(reprojectdst[8].x, reprojectdst[8].y);
			cv::Point2d P2(reprojectdst[9].x, reprojectdst[9].y);
			cv::Point2d P3(reprojectdst[10].x, reprojectdst[10].y);
			cv::Point2d P4(reprojectdst[11].x, reprojectdst[11].y);
			double S1 = ((P4.x - P2.x) * (P1.y - P2.y) - (P4.y - P2.y) * (P1.x - P2.x)) / 2;
			double S2 = ((P4.x - P2.x) * (P2.y - P3.y) - (P4.y - P2.y) * (P2.x - P3.x)) / 2;
			long Center_X = P1.x + (P3.x - P1.x) * S1 / (S1 + S2);
			long Center_Y = P1.y + (P3.y - P1.y) * S1 / (S1 + S2);
			//顔の現在位置として使用する
			cv::Point2d CenterPoint(Center_X, Center_Y);
			actor.horizontal_pos = ((float)Center_X / 2) / 640;
			actor.vertical_pos = ((float)Center_Y / 2) / 360;

			//算出した現在の顔の姿勢情報を行列からオイラー角に変換する
			//オイラー角→0度、30度、90度などの一般的な「角度」の表現方法
			cv::Rodrigues(rotation_vec, rotation_mat);
			cv::hconcat(rotation_mat, translation_vec, pose_mat);
			cv::decomposeProjectionMatrix(pose_mat, out_intrinsics, out_rotation, out_translation, cv::noArray(), cv::noArray(), cv::noArray(), euler_angle);

			//この手の顔認識は、光やカメラ、背景の状態などによって常にノイズが入っている
			//(早い話が、顔が常にブルブル震えた状態になる)
			//そこで、一定値以下の姿勢変化は無視し、かつ変化量がスムーズになるよう
			//フィルタ処理をかけ、姿勢の変化を滑らかにする
			//ついでに、左右35度以上には曲がらないように制限する。
			for (int i = 0; i < 3; i++)
			{
				if (euler_angle.at<double>(i) > 35) euler_angle.at<double>(i) = 35;
				if (euler_angle.at<double>(i) < -35) euler_angle.at<double>(i) = -35;
				if ((std::abs(prev_euler_angle.at<double>(i) - euler_angle.at<double>(i)) > 1))
				{
					if (prev_euler_angle.at<double>(i) > euler_angle.at<double>(i))
					{
						euler_angle.at<double>(i) -= 0.005;
					}
					else
					{
						euler_angle.at<double>(i) += 0.005;
					}
					euler_angle.at<double>(i) = (LPF_VALUE_PRE * prev_euler_angle.at<double>(i))
						+ (LPF_VALUE_CUR * euler_angle.at<double>(i));
				}
				else
				{
					euler_angle.at<double>(i) = prev_euler_angle.at<double>(i);
				}
			}
			prev_euler_angle.at<double>(0) = euler_angle.at<double>(0);
			prev_euler_angle.at<double>(1) = euler_angle.at<double>(1);
			prev_euler_angle.at<double>(2) = euler_angle.at<double>(2);

			//取得した顔の位置、姿勢を画面上に表示する
			outtext << "X: " << std::setprecision(3) << euler_angle.at<double>(0);
			cv::putText(temp, outtext.str(), cv::Point(50, 40), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0, 0, 0));
			outtext.str("");
			outtext << "Y: " << std::setprecision(3) << euler_angle.at<double>(1);
			cv::putText(temp, outtext.str(), cv::Point(50, 60), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0, 0, 0));
			outtext.str("");
			outtext << "Z: " << std::setprecision(3) << euler_angle.at<double>(2);
			cv::putText(temp, outtext.str(), cv::Point(50, 80), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0, 0, 0));
			outtext.str("");

			outtext << "Pos_X: " << std::setprecision(3) << actor.horizontal_pos;
			cv::putText(temp, outtext.str(), cv::Point(50, 100), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0, 0, 0));
			outtext.str("");

			outtext << "Pos_Y: " << std::setprecision(3) << actor.vertical_pos;
			cv::putText(temp, outtext.str(), cv::Point(50, 120), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0, 0, 0));
			outtext.str("");
			actor.yaw = euler_angle.at<double>(1);
			actor.pitch = euler_angle.at<double>(0);
			actor.roll = euler_angle.at<double>(2);

			//Unity側への送信用データを作成する
			picojson::object obj;
			//顔位置X座標
			obj.emplace(std::make_pair("horizontal_pos", actor.horizontal_pos));
			//顔位置Y座標
			obj.emplace(std::make_pair("vertical_pos", actor.vertical_pos));
			//顔姿勢 Y軸
			obj.emplace(std::make_pair("yaw", actor.yaw));
			//顔姿勢 X軸
			obj.emplace(std::make_pair("pitch", actor.pitch));
			//顔姿勢 Z軸
			obj.emplace(std::make_pair("roll", actor.roll));
			//全部の顔器官点を格納
			picojson::array mouse;
			for (unsigned int i = 0; i < FACIAL_POINTS; ++i)
			{
				picojson::array coord;
				coord.push_back(picojson::value((double)shape.part(i).x()));
				coord.push_back(picojson::value((double)shape.part(i).y()));
				mouse.push_back(picojson::value(coord));
			}
			obj.emplace(std::make_pair("obj", picojson::value(mouse)));
			//文字列に変換
			picojson::value val(obj);

			//シリアライズ
			std::string msg = val.serialize();
			//送信
			sendto(sock, msg.c_str(), msg.size(), 0, (struct sockaddr *)&addr, sizeof(addr));

			//顔器官点をクリア
			image_pts.clear();
			//press esc to end
			//escで終了
			auto key = cvWaitKey(2);
			if (key == '\x1b')
			{
				break;
			}
		}
		Sleep(1 / 1000);
		//カメラから取得した映像を出力
		cv::imshow("demo", temp);
		//なぜかこれやると映像出力が動く
		cvWaitKey(1);
	}

	return 0;
}


void DrawFaceBox(cv::Mat frame, std::vector<cv::Point2d> reprojectdst)
{
	cv::line(frame, reprojectdst[8], reprojectdst[9], cv::Scalar(0, 0, 255));
	cv::line(frame, reprojectdst[9], reprojectdst[10], cv::Scalar(0, 0, 255));
	cv::line(frame, reprojectdst[10], reprojectdst[11], cv::Scalar(0, 0, 255));
	cv::line(frame, reprojectdst[11], reprojectdst[8], cv::Scalar(0, 0, 255));

	cv::line(frame, reprojectdst[8], reprojectdst[10], cv::Scalar(0, 0, 255));
	cv::line(frame, reprojectdst[9], reprojectdst[11], cv::Scalar(0, 0, 255));
}

void SetInitialPoints(std::vector<cv::Point3d> * in_BoxPoints, std::vector<cv::Point3d> * in_FaceLandmarkPoints)
{
	std::vector<cv::Point3d> reprojectsrc = (std::vector<cv::Point3d>) *in_BoxPoints;
	std::vector<cv::Point3d> object_pts = (std::vector<cv::Point3d>) *in_FaceLandmarkPoints;

	reprojectsrc.push_back(cv::Point3d(10.0, 10.0, 10.0));
	reprojectsrc.push_back(cv::Point3d(10.0, 10.0, -10.0));
	reprojectsrc.push_back(cv::Point3d(10.0, -10.0, -10.0));
	reprojectsrc.push_back(cv::Point3d(10.0, -10.0, 10.0));
	reprojectsrc.push_back(cv::Point3d(-10.0, 10.0, 10.0));
	reprojectsrc.push_back(cv::Point3d(-10.0, 10.0, -10.0));
	reprojectsrc.push_back(cv::Point3d(-10.0, -10.0, -10.0));
	reprojectsrc.push_back(cv::Point3d(-10.0, -10.0, 10.0));

	reprojectsrc.push_back(cv::Point3d(-10.0, -10.0, 0));
	reprojectsrc.push_back(cv::Point3d(10.0, -10.0, 0));
	reprojectsrc.push_back(cv::Point3d(10.0, 10.0, 0));
	reprojectsrc.push_back(cv::Point3d(-10.0, 10.0, 0));

	//顔器官点の3D座標初期値,  この点を基準に、顔の姿勢を算出する 
	//参照元 http://aifi.isr.uc.pt/Downloads/OpenGL/glAnthropometric3DModel.cpp
	object_pts.push_back(cv::Point3d(6.825897, 6.760612, 4.402142));     //#33 右眉右端
	object_pts.push_back(cv::Point3d(1.330353, 7.122144, 6.903745));     //#29 右眉左端
	object_pts.push_back(cv::Point3d(-1.330353, 7.122144, 6.903745));    //#34 左眉右端
	object_pts.push_back(cv::Point3d(-6.825897, 6.760612, 4.402142));    //#38 左眉左端
	object_pts.push_back(cv::Point3d(5.311432, 5.485328, 3.987654));     //#13 右目右端
	object_pts.push_back(cv::Point3d(1.789930, 5.393625, 4.413414));     //#17 右目左端
	object_pts.push_back(cv::Point3d(-1.789930, 5.393625, 4.413414));    //#25 左目右端
	object_pts.push_back(cv::Point3d(-5.311432, 5.485328, 3.987654));    //#21 左目左端
	object_pts.push_back(cv::Point3d(2.005628, 1.409845, 6.165652));     //#55 鼻右
	object_pts.push_back(cv::Point3d(-2.005628, 1.409845, 6.165652));    //#49 鼻左
	object_pts.push_back(cv::Point3d(2.774015, -2.080775, 5.048531));    //#43 口右
	object_pts.push_back(cv::Point3d(-2.774015, -2.080775, 5.048531));   //#39 口左
	object_pts.push_back(cv::Point3d(0.000000, -3.116408, 6.097667));    //#45 口中央下
	object_pts.push_back(cv::Point3d(0.000000, -7.415691, 4.070434));    //#6 顎
	*in_BoxPoints = reprojectsrc;
	*in_FaceLandmarkPoints = object_pts;
}