#include <opencv2/opencv.hpp>

int main(void)
{
	// 映像を取り込むためのオブジェクトを宣言する
	cv::VideoCapture cap;

	// カメラに接続する
	// # open の引数にカメラ番号を指定する（通常は0）
	cap.open(1);
	if (cap.isOpened() == false) {
		// カメラの接続ができなかったときは終了する
		printf("接続失敗");
		cv::waitKey();
		return 0;
	}

	// 画像情報を取得
	int    width = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH);		// フレーム横幅を取得
	int    height = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT);	// フレーム縦幅を取得

	std::cout << "画像サイズ " << width << "x" << height << std::endl;

	// 画像を格納するオブジェクトを宣言する
	cv::Mat frame;

	for (;;) {
		// 1フレームを取り込む
		cap >> frame;

		// 画像から空のとき、無限ループを抜ける
		if (frame.empty() == true) {
			printf("画像が空");
			cv::waitKey();
			break;
		}

		// ウィンドウに画像を表示する
		cv::imshow("入力中", frame);

		// 30ミリ秒待つ
		int key = cv::waitKey(0);

		// 何かキーが押されたら終了する
		if (key > 0) {
			break;
		}
	}

	return 0;
}