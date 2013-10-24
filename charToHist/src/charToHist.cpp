//============================================================================
// Name        : OpenCV_test4.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Width, Height  Histogram
//============================================================================

#include <iostream>
#include <cv.h>
#include <highgui.h>
using namespace std;

void hwHist(int img_min, int img_max, const char* filePath);

int main( int argc, char** argv ) {
	hwHist(0, 1577, "./img_out/");

	return 0;
}

void hwHist(int img_min, int img_max, const char* filePath){
	int image_min = img_min;		// 画像ファイルの開始番号
	int image_max = img_max;		// 画像ファイルの終了番号
	int hist_row[128] = {};	// 横幅のヒストグラム用データ変数
	int hist_col[128] = {};	// 縦幅のヒストグラム用データ変数

	////////////////////////////////////////////////////////////
	//	縦幅と横幅のヒストグラム								  //
	////////////////////////////////////////////////////////////
	for(int i=image_min; i<=image_max; i++){
		cv::Mat image;
		char filename[16];

		// 画像の読み込み
		sprintf(filename, "%s%d.png", filePath, i);
		image = cv::imread(filename);	// >0:強制3ch, =0:グレースケール, <0:そのまま
		if(image.empty()){
			cout << "No image." << endl;
			exit(-1);
		}

		hist_row[image.rows]++;
		hist_col[image.cols]++;
	}

	cv::Mat hist_img_row(cv::Size(128, 128), CV_8U, cv::Scalar::all(255));	// 縦幅のヒストグラム
	cv::Mat hist_img_col(cv::Size(128, 128), CV_8U, cv::Scalar::all(255));	// 横幅のヒストグラム

	for(int i=0; i<128; i++){
		for(int j=0; j<hist_row[i]; j++)	hist_img_row.at<unsigned char>(j, i) = 0;
		for(int j=0; j<hist_col[i]; j++)	hist_img_col.at<unsigned char>(j, i) = 0;
	}

	cv::flip(hist_img_row, hist_img_row, 0);	// 水平方向反転
	cv::flip(hist_img_col, hist_img_col, 0);	// 水平方向反転

	////////////////////////////////////////////////////////////
	//	画像の表示											  //
	////////////////////////////////////////////////////////////
	cv::namedWindow("hist_img_row", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
	cv::imshow("hist_img_row", hist_img_row);
	cv::namedWindow("hist_img_col", CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);
	cv::imshow("hist_img_col", hist_img_col);
	cv::waitKey(0);

	////////////////////////////////////////////////////////////
	//	画像の出力											  //
	////////////////////////////////////////////////////////////
	cv::imwrite("hist_img_row.bmp", hist_img_row);
	cv::imwrite("hist_img_col.bmp", hist_img_col);
}
