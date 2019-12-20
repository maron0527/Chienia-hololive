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

//�t�B���^�p�̌W��
#define LPF_VALUE_PRE  0.4
#define LPF_VALUE_CUR  (1 - LPF_VALUE_PRE)
#define FACIAL_POINTS   68

//�J�����̃p�����[�^(�Œ�l)
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

/* ���[�J���֐� */
static void DrawFaceBox(cv::Mat frame, std::vector<cv::Point2d> reprojectdst);
static void SetInitialPoints(std::vector<cv::Point3d> * in_BoxPoints, std::vector<cv::Point3d> * in_FaceLandmarkPoints);

int main(int argc, char** argv)
{
	/************ �J�����ԍ��擾 ************/
	int cam_num;
	face actor;
	//cout << "input camera num --> "; cin >> cam_num;
	//getchar();
	//�J�����I�[�v��
	cv::VideoCapture cap(0);
	if (!cap.isOpened())
	{
		std::cout << "Unable to connect to camera" << std::endl;
		return EXIT_FAILURE;
	}
	//------------------------------------//

	/************ �\�P�b�g������ ************/
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


	/************ ���o��̏�����************/
	//�猟�o��
	//(Dlib��API�Ɏ�������Ă�����̂����̂܂܎g�p����
	dlib::frontal_face_detector detector = dlib::get_frontal_face_detector();

	//��튯�_���o��
	//�O���t�@�C�����猟�o���ǂݍ���(EXE�R�[�����̑�1����)
	dlib::shape_predictor predictor;
	dlib::deserialize("sp.dat") >> predictor;
	//-------------------------------//


	//�J�����̑g�ݍ��݊֐��Ƙc�݌W����ݒ�
	cv::Mat cam_matrix = cv::Mat(3, 3, CV_64FC1, K);
	cv::Mat dist_coeffs = cv::Mat(5, 1, CV_64FC1, D);

	//��튯��3D���W�����l
	std::vector<cv::Point3d> object_pts;
	//�������������`��3D���W�����l
	std::vector<cv::Point3d> reprojectsrc;
	//�����l��ݒ�
	SetInitialPoints(&reprojectsrc, &object_pts);

	//result
	cv::Mat rotation_vec;                           //��V�x�N�g��
	cv::Mat rotation_mat;                           //��V�s��
	cv::Mat translation_vec;                        //�ϊ��x�N�g��
	cv::Mat pose_mat = cv::Mat(3, 4, CV_64FC1);     //�p��(3X4�̔z��)
	cv::Mat euler_angle = cv::Mat(3, 1, CV_64FC1);  //�I�C���[�p(3X1�̔z��)
	cv::Mat prev_euler_angle = cv::Mat(3, 1, CV_64FC1); //�O�t���[���ł̃I�C���[�p(3X1�̔z��)

														//2D���W�i�[�f�[�^
	std::vector<cv::Point2d> reprojectdst;
	reprojectdst.resize(8);

	//��F���ɂ͎g�p���Ȃ�
	//�g�p����API�̈����𖄂߂邽�߂Ɏg�p����
	cv::Mat out_intrinsics = cv::Mat(3, 3, CV_64FC1);
	cv::Mat out_rotation = cv::Mat(3, 3, CV_64FC1);
	cv::Mat out_translation = cv::Mat(3, 1, CV_64FC1);

	//��ʕ\���p�e�L�X�g
	std::ostringstream outtext;

	//���C�����[�v
	while (1)
	{
		//�J�������猻�݂̉摜�t���[�����擾
		cv::Mat temp;
		cap >> temp;
		//Dlib��API�Ŏg�p���邽�߂ɕϊ�
		dlib::cv_image<dlib::bgr_pixel> cimg(temp);

		//�摜�f�[�^���猟�o��ɂ�����
		std::vector<dlib::rectangle> faces = detector(cimg);

		// 1�ȏ�̊�����o���Ă���ꍇ
		if (faces.size() > 0)
		{
			//���̃t���[���Ō��o���ꂽ�ŏ��̊���A��튯�_���o��ɂ�����
			dlib::full_object_detection shape = predictor(cimg, faces[0]);
			//��튯�_���o�킩��A��̗̈���擾
			auto rect = shape.get_rect();

			//�튯�_��`��
			for (unsigned int i = 0; i < FACIAL_POINTS; ++i)
			{
				cv::circle(
					temp,                   //�`���̉摜�f�[�^
					cv::Point(              //��튯�_��2D�_�I�u�W�F�N�g
						shape.part(i).x(),
						shape.part(i).y()),
					2,                      //�_�̃T�C�Y
					cv::Scalar(0, 0, 255),  //�_�̐F
					-1);
			}

			//��ʏ��2D���W�̈ꕔ���i�[
			//��̎p�����o�̂��߁A�ꕔ�̊�튯�_��ʂ̏ꏊ�Ɋi�[����
			std::vector<cv::Point2d> image_pts;
			image_pts.push_back(cv::Point2d(shape.part(17).x(), shape.part(17).y())); //#17 �E���E�[
			image_pts.push_back(cv::Point2d(shape.part(21).x(), shape.part(21).y())); //#21 �E�����[
			image_pts.push_back(cv::Point2d(shape.part(22).x(), shape.part(22).y())); //#22 �����E�[
			image_pts.push_back(cv::Point2d(shape.part(26).x(), shape.part(26).y())); //#26 �������[

			image_pts.push_back(cv::Point2d(shape.part(36).x(), shape.part(36).y())); //#36 �E�ډE�[
			image_pts.push_back(cv::Point2d(shape.part(39).x(), shape.part(39).y())); //#39 �E�ڍ��[
			image_pts.push_back(cv::Point2d(shape.part(42).x(), shape.part(42).y())); //#42 ���ډE�[
			image_pts.push_back(cv::Point2d(shape.part(45).x(), shape.part(45).y())); //#45 ���ڍ��[
			image_pts.push_back(cv::Point2d(shape.part(31).x(), shape.part(31).y())); //#31 �@�E
			image_pts.push_back(cv::Point2d(shape.part(35).x(), shape.part(35).y())); //#35 �@��
			image_pts.push_back(cv::Point2d(shape.part(48).x(), shape.part(48).y())); //#48 ���E
			image_pts.push_back(cv::Point2d(shape.part(54).x(), shape.part(54).y())); //#54 ����
			image_pts.push_back(cv::Point2d(shape.part(57).x(), shape.part(57).y())); //#57 ��������
			image_pts.push_back(cv::Point2d(shape.part(8).x(), shape.part(8).y()));   //#8  �{

																					  //��튯�_��3D���W�����l�A�J�������A���o����2D���W����
																					  //��튯�_��3D���W���ݒl�Ǝp�������Z�o����
			cv::solvePnP(object_pts, image_pts, cam_matrix, dist_coeffs, rotation_vec, translation_vec);

			//�������`��3D���W�����l�ƒ��O�Ɏ擾�����p����񂩂�
			//�������`��3D���W���ݒl���Z�o����
			cv::projectPoints(reprojectsrc, rotation_vec, translation_vec, cam_matrix, dist_coeffs, reprojectdst);

			//�������`��`�悷��
			DrawFaceBox(temp, reprojectdst);

			//�������`�̑Ίp����_�̍��W���Z�o
			cv::Point2d P1(reprojectdst[8].x, reprojectdst[8].y);
			cv::Point2d P2(reprojectdst[9].x, reprojectdst[9].y);
			cv::Point2d P3(reprojectdst[10].x, reprojectdst[10].y);
			cv::Point2d P4(reprojectdst[11].x, reprojectdst[11].y);
			double S1 = ((P4.x - P2.x) * (P1.y - P2.y) - (P4.y - P2.y) * (P1.x - P2.x)) / 2;
			double S2 = ((P4.x - P2.x) * (P2.y - P3.y) - (P4.y - P2.y) * (P2.x - P3.x)) / 2;
			long Center_X = P1.x + (P3.x - P1.x) * S1 / (S1 + S2);
			long Center_Y = P1.y + (P3.y - P1.y) * S1 / (S1 + S2);
			//��̌��݈ʒu�Ƃ��Ďg�p����
			cv::Point2d CenterPoint(Center_X, Center_Y);
			actor.horizontal_pos = ((float)Center_X / 2) / 640;
			actor.vertical_pos = ((float)Center_Y / 2) / 360;

			//�Z�o�������݂̊�̎p�������s�񂩂�I�C���[�p�ɕϊ�����
			//�I�C���[�p��0�x�A30�x�A90�x�Ȃǂ̈�ʓI�ȁu�p�x�v�̕\�����@
			cv::Rodrigues(rotation_vec, rotation_mat);
			cv::hconcat(rotation_mat, translation_vec, pose_mat);
			cv::decomposeProjectionMatrix(pose_mat, out_intrinsics, out_rotation, out_translation, cv::noArray(), cv::noArray(), cv::noArray(), euler_angle);

			//���̎�̊�F���́A����J�����A�w�i�̏�ԂȂǂɂ���ď�Ƀm�C�Y�������Ă���
			//(�����b���A�炪��Ƀu���u���k������ԂɂȂ�)
			//�����ŁA���l�ȉ��̎p���ω��͖������A���ω��ʂ��X���[�Y�ɂȂ�悤
			//�t�B���^�����������A�p���̕ω������炩�ɂ���
			//���łɁA���E35�x�ȏ�ɂ͋Ȃ���Ȃ��悤�ɐ�������B
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

			//�擾������̈ʒu�A�p������ʏ�ɕ\������
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

			//Unity���ւ̑��M�p�f�[�^���쐬����
			picojson::object obj;
			//��ʒuX���W
			obj.emplace(std::make_pair("horizontal_pos", actor.horizontal_pos));
			//��ʒuY���W
			obj.emplace(std::make_pair("vertical_pos", actor.vertical_pos));
			//��p�� Y��
			obj.emplace(std::make_pair("yaw", actor.yaw));
			//��p�� X��
			obj.emplace(std::make_pair("pitch", actor.pitch));
			//��p�� Z��
			obj.emplace(std::make_pair("roll", actor.roll));
			//�S���̊�튯�_���i�[
			picojson::array mouse;
			for (unsigned int i = 0; i < FACIAL_POINTS; ++i)
			{
				picojson::array coord;
				coord.push_back(picojson::value((double)shape.part(i).x()));
				coord.push_back(picojson::value((double)shape.part(i).y()));
				mouse.push_back(picojson::value(coord));
			}
			obj.emplace(std::make_pair("obj", picojson::value(mouse)));
			//������ɕϊ�
			picojson::value val(obj);

			//�V���A���C�Y
			std::string msg = val.serialize();
			//���M
			sendto(sock, msg.c_str(), msg.size(), 0, (struct sockaddr *)&addr, sizeof(addr));

			//��튯�_���N���A
			image_pts.clear();
			//press esc to end
			//esc�ŏI��
			auto key = cvWaitKey(2);
			if (key == '\x1b')
			{
				break;
			}
		}
		Sleep(1 / 1000);
		//�J��������擾�����f�����o��
		cv::imshow("demo", temp);
		//�Ȃ���������Ɖf���o�͂�����
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

	//��튯�_��3D���W�����l,  ���̓_����ɁA��̎p�����Z�o���� 
	//�Q�ƌ� http://aifi.isr.uc.pt/Downloads/OpenGL/glAnthropometric3DModel.cpp
	object_pts.push_back(cv::Point3d(6.825897, 6.760612, 4.402142));     //#33 �E���E�[
	object_pts.push_back(cv::Point3d(1.330353, 7.122144, 6.903745));     //#29 �E�����[
	object_pts.push_back(cv::Point3d(-1.330353, 7.122144, 6.903745));    //#34 �����E�[
	object_pts.push_back(cv::Point3d(-6.825897, 6.760612, 4.402142));    //#38 �������[
	object_pts.push_back(cv::Point3d(5.311432, 5.485328, 3.987654));     //#13 �E�ډE�[
	object_pts.push_back(cv::Point3d(1.789930, 5.393625, 4.413414));     //#17 �E�ڍ��[
	object_pts.push_back(cv::Point3d(-1.789930, 5.393625, 4.413414));    //#25 ���ډE�[
	object_pts.push_back(cv::Point3d(-5.311432, 5.485328, 3.987654));    //#21 ���ڍ��[
	object_pts.push_back(cv::Point3d(2.005628, 1.409845, 6.165652));     //#55 �@�E
	object_pts.push_back(cv::Point3d(-2.005628, 1.409845, 6.165652));    //#49 �@��
	object_pts.push_back(cv::Point3d(2.774015, -2.080775, 5.048531));    //#43 ���E
	object_pts.push_back(cv::Point3d(-2.774015, -2.080775, 5.048531));   //#39 ����
	object_pts.push_back(cv::Point3d(0.000000, -3.116408, 6.097667));    //#45 ��������
	object_pts.push_back(cv::Point3d(0.000000, -7.415691, 4.070434));    //#6 �{
	*in_BoxPoints = reprojectsrc;
	*in_FaceLandmarkPoints = object_pts;
}