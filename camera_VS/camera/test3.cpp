//図形描画

#include <opencv2/opencv.hpp>

using namespace std;

int main(void)
{
	// 画像を格納するオブジェクトを宣言する
	// # CV_8UC3 : 8bit 3channel フルカラー画像
	cv::Mat	image(400, 400, CV_8UC3);

	// 画像を白色で塗りつぶす
	image = cv::Scalar(255, 255, 255);

	// 多角形の頂点を設定する
	vector<cv::Point> p;
	int n = 5;
	for (int i = 0; i < n; i++) {
		p.push_back(cv::Point(cos(CV_PI * (2.0*i / n - 0.5)) * 180 + 200, sin(CV_PI * (2.0*i / n - 0.5)) * 180 + 200));
	}

	// 多角形を描く
	// # polylines(画像, 頂点集合, 多角形を閉じるか, 色, 線幅, 連結)
	cv::polylines(image, p, true, cv::Scalar(255, 0, 0), 2, cv::LINE_AA);

	p.clear();
	for (int i = 0; i < n; i++) {
		p.push_back(cv::Point(cos(CV_PI * (2.0*i / n - 0.5)) * 140 + 200, sin(CV_PI * (2.0*i / n - 0.5)) * 140 + 200));
	}

	// 中を塗りつぶした多角形を描く
	// # fillConvexPoly(画像, 頂点集合, 色, 連結)
	cv::fillConvexPoly(image, p, cv::Scalar(0, 0, 255), cv::LINE_AA);

	// fillPolyを使う場合は次のようにする
	//  vector<vector<cv::Point>> pp;
	//  pp.push_back(p);
	//  cv::fillPoly(image, pp, cv::Scalar(0, 0, 255), cv::LINE_AA);

	cv::imshow("多角形描画", image);
	cv::waitKey();

	// 複数の多角形を一度に描く

	// 多角形の頂点を設定する
	vector<vector<cv::Point>> ps(16);
	for (int n = 0; n < 16; n++) {
		cv::Point p0((n % 4) * 100 + 50, (n / 4) * 100 + 50);
		for (int i = 0; i < n + 3; i++) {
			cv::Point pa(cos(CV_PI * (2.0*i / (n + 3) - 0.5)) * 40, sin(CV_PI * (2.0*i / (n + 3) - 0.5)) * 40);
			ps[n].push_back(p0 + pa);
		}
	}

	image = cv::Scalar(255, 255, 255);

	// 多角形を描く
	cv::fillPoly(image, ps, cv::Scalar(0, 255, 255), cv::LINE_AA);		// 黄色で塗りつぶした多角形
	cv::polylines(image, ps, true, cv::Scalar(0, 0, 0), 3, cv::LINE_AA);// 黒色の線の多角形

	cv::imshow("多角形描画", image);
	cv::waitKey();

	return 0;
}