#include <iostream>
#include <fstream>
#include <vector>
#include <cv.h>
#include <highgui.h>

using namespace std;

struct areaData{
	int minX;	// 座標の最小値X
	int minY;	// 座標の最小値Y
	int maxX;	// 座標の最大値X
	int maxY;	// 座標の最大値Y
	int sx;	// 境界線追跡の始点
	int sy;	// 境界線追跡の終点
	int Flag;	// 要素の有効／無効の判断
};

int main( int argc, char** argv )
{
	cv::Mat image, image_bin;	// 元画像, 二値化画像
	int imgHeight, imgWidth;		// 画像の縦幅, 画像の横幅
	int minX, minY;	// 境界線追跡で得た座標の最小値
	int maxX, maxY;	// 境界線追跡で得た座標の最大値
	int sx, sy;		// 境界線追跡開始点
	int px, py;		// 境界線追跡中の現在地
	int op = 2;		// 境界線追跡中の次に探索する方向
	int flag = 0;		// ループ脱出フラグ
	int bflag = 1;	// 境界線追跡フラグ（0:非実行, 1:実行)
	int endFlag = 0;	// 終了フラグ

	// 解析後の結果保持用
	// struct areaData data[102400];	// データ保持変数（2048でも足りないことがあるかもしれない）
	vector<struct areaData> data;
	struct areaData data2[2048];//	データの保持変数（不要な要素を削った後の保持用）
	struct areaData tmpData;
	int count = 0;	// 取得回数
	int count2 = 0;	// 最終的な取得回数

	std::ofstream ofs("log.txt");

	// エラー処理（コマンドライン引数のエラー処理）
	if( argc != 2)	{
		printf( "Usage: OpenCV_test2 filename\n" );
		return -1;
	}

	// 画像の読み込み
	image = cv::imread(argv[1], 0);
	// エラー処理（Matに正しく画像が読み込まれていない場合のエラー処理）
	if(image.empty()){	// cv::Mat.empty() 行列にデータが確保されてない場合true
		printf( "No image data\n" );
		return -1;
	}

	// 画像の二値化
	cv::threshold(image, image_bin, 0, 255, cv::THRESH_BINARY|cv::THRESH_OTSU);

	IplImage iplImage = image;	// Mat型をIplImage型に変換
	CvSize ImgSize = cvGetSize(&iplImage);	// 画像サイズの取得

	ofs << "Image Size: " << ImgSize.width << " x " << ImgSize.height << std::endl;

	imgHeight = ImgSize.height;	// 画像の縦幅
	imgWidth = ImgSize.width;	// 画像の横幅

	sx = sy = 0;	// 境界線追跡開始点の初期化

	while(!endFlag){
		// 始点の取得（ラスタスキャン順）
		for(int i=sy; i<imgHeight; i++){
			for(int j=sx; j<imgWidth; j++){
				if(image_bin.at<unsigned char>(i, j) == 0){
					sx = px = maxX = minX = j;
					sy = py = maxY = minY = i;
					flag = 1;
					break;
				}
				// 終了条件(探索箇所が対象の最も右下の位置ならば)
				if(i==imgHeight-1 && j==imgWidth-1){
					endFlag = 1;
					flag = 1;
					bflag = 0;
					break;
				}
			}
			if(flag == 1) break;
			sx = 0;
		}

		if(bflag){
			ofs << "Start Point: " << "(" << sx << ", " << sy << ")" << std::endl;

			while(flag){
				switch(op){
					case 0:	// 左上
						if(image_bin.at<unsigned char>(py-1, px-1) == 0){
							px = px - 1;
							py = py - 1;
							op = 6;
							break;
						}
						/* no break */
					case 1:	// 左
						if(image_bin.at<unsigned char>(py, px-1) == 0){
							px = px - 1;
							op = 0;
							break;
						}
						/* no break */
					case 2:	// 左下
						if(image_bin.at<unsigned char>(py+1, px-1) == 0){
							px = px - 1;
							py = py + 1;
							op = 0;
							break;
						}
						/* no break */
					case 3:	// 下
						if(image_bin.at<unsigned char>(py+1, px) == 0){
							py = py + 1;
							op = 2;
							break;
						}
						/* no break */
					case 4:	// 右下
						if(image_bin.at<unsigned char>(py+1, px+1) == 0){
							px = px + 1;
							py = py + 1;
							op = 2;
							break;
						}
						/* no break */
					case 5:	// 右
						if(image_bin.at<unsigned char>(py, px+1) == 0){
							px = px + 1;
							op = 4;
							break;
						}
						/* no break */
					case 6:	// 右上
						if(image_bin.at<unsigned char>(py-1, px+1) == 0){
							px = px + 1;
							py = py - 1;
							op = 4;
							break;
						}
						/* no break */
					case 7:	// 上
						if(image_bin.at<unsigned char>(py-1, px) == 0){
							py = py - 1;
							op = 6;
							break;
						}
						op = 0;
				}

				// 最大座標の更新確認
				if(px >= maxX) maxX = px;
				if(py >= maxY) maxY = py;
				// 最小座標の更新確認
				if(px <= minX) minX = px;
				if(py <= minY) minY = py;

				// 終了条件確認
				if(px==sx && py==sy) flag = 0;
			}

			// 最大値・最小値座標の出力
			ofs << count << "-" << bflag << "Area: " << "Min(" << minX << ", " << minY << ") Max(" << maxX << ", " << maxY << ")" << std::endl;
			tmpData.maxX = maxX;
			tmpData.maxY = maxY;
			tmpData.minX = minX;
			tmpData.minY = minY;
			tmpData.sx = sx;
			tmpData.sy = sy;
			tmpData.Flag = 1;
			data.push_back(tmpData);
			count++;
		}
		bflag = 1;
		flag = 0;
		// 白画素になるまで、探索開始点の座標Xをインクリメント
		while(1){
			if(image_bin.at<unsigned char>(sy, sx) == 255) break;
			sx++;
		}
		op = 2;		// 境界線追跡中の次に探索する方向を初期化
	}

	printf("境界線追跡後：%d\n", count);	// Debug

	//ToDo 最頻値を求める
	int maxHeight = 0;	// 縦幅の最大値
	int maxWidth = 0;		// 横幅の最大値
	int modeHeight = 0;	// 縦幅の最頻値
	int modeWidth = 0;	// 横幅の最頻値
	for(int i=0; i<count; i++){

	}
	//ToDo	大き過ぎる候補の除外（最頻値のX倍以上？）

	////////////////////////////////////////////////////////////
	//	重複範囲の統合										  //
	////////////////////////////////////////////////////////////
	cout << "重複範囲の統合 開始" << endl;	// Debug message
	int mFlag;	// 統合した要素が一つでもあるならば、フラグを立ててループを抜けない
	while(1){
		mFlag = 0;
		for(int i=0; i<count; i++){
			if(data[i].Flag){
				for(int j=0; j<count; j++){
					if((j != i) && data[j].Flag){
						if((data[i].minX <= data[j].minX) && (data[j].minX <= data[i].maxX) && (data[i].minY <= data[j].minY) && (data[j].minY <= data[i].maxY)){
							if(data[i].minX >= data[j].minX){	// minXの比較
								data[i].minX = data[j].minX;
							}
							if(data[i].minY >= data[j].minY){	// minYの比較
								data[i].minY = data[j].minY;
							}
							if(data[i].maxX <= data[j].maxX){	// maxXの比較
								data[i].maxX = data[j].maxX;
							}
							if(data[i].maxY <= data[j].maxY){	// maxYの比較
								data[i].maxY = data[j].maxY;
							}
							mFlag = 1;
							data[j].Flag = 0;
							break;
						}else if((data[i].minX <= data[j].minX) && (data[j].minX <= data[i].maxX) && (data[i].minY <= data[j].maxY) && (data[j].maxY <= data[i].maxY)){
							if(data[i].minX >= data[j].minX){	// minXの比較
								data[i].minX = data[j].minX;
							}
							if(data[i].minY >= data[j].minY){	// minYの比較
								data[i].minY = data[j].minY;
							}
							if(data[i].maxX <= data[j].maxX){	// maxXの比較
								data[i].maxX = data[j].maxX;
							}
							if(data[i].maxY <= data[j].maxY){	// maxYの比較
								data[i].maxY = data[j].maxY;
							}
							mFlag = 1;
							data[j].Flag = 0;
							break;
						}else if((data[i].minX <= data[j].maxX) && (data[j].maxX <= data[i].maxX) && (data[i].minY <= data[j].minY) && (data[j].minY <= data[i].maxY)){
							if(data[i].minX >= data[j].minX){	// minXの比較
								data[i].minX = data[j].minX;
							}
							if(data[i].minY >= data[j].minY){	// minYの比較
								data[i].minY = data[j].minY;
							}
							if(data[i].maxX <= data[j].maxX){	// maxXの比較
								data[i].maxX = data[j].maxX;
							}
							if(data[i].maxY <= data[j].maxY){	// maxYの比較
								data[i].maxY = data[j].maxY;
							}
							mFlag = 1;
							data[j].Flag = 0;
							break;
						}else if((data[i].minX <= data[j].maxX) && (data[j].maxX <= data[i].maxX) && (data[i].minY <= data[j].maxY) && (data[j].maxY <= data[i].maxY)){
							if(data[i].minX >= data[j].minX){	// minXの比較
								data[i].minX = data[j].minX;
							}
							if(data[i].minY >= data[j].minY){	// minYの比較
								data[i].minY = data[j].minY;
							}
							if(data[i].maxX <= data[j].maxX){	// maxXの比較
								data[i].maxX = data[j].maxX;
							}
							if(data[i].maxY <= data[j].maxY){	// maxYの比較
								data[i].maxY = data[j].maxY;
							}
							mFlag = 1;
							data[j].Flag = 0;
							break;
						}
					}
				}
			}
		}
		if(!mFlag) break;
	}
	cout << "重複範囲の統合 終了" << endl;	// Debug message

	// 最終結果の格納と、数のカウント
	for(int i=0; i<count; i++){
		if(data[i].Flag){
			data2[count2] = data[i];
			count2++;
		}
	}

	printf("重複範囲を統合後：%d\n", count2);	// Debug

/*
	// 大きさによる除外
	for(int i=0; i<count; i++){
		if(data[i].Flag){
			if((data[i].maxX - data[i].minX) >= (ImgSize.width * 0.1)){
				data[i].Flag = 0;
			}
		}
	}
	count2 = 0;
	for(int i=0; i<count; i++){
		if(data[i].Flag){
			data2[count2] = data[i];
			count2++;
		}
	}

	printf("大きすぎるデータを除外：%d\n", count2);
*/

	// 1chのMatを3chのMatに変換
	cv::Mat dst(image.size(), CV_8UC3);
	int fromto[] = {0,0, 0,1, 0,2};
	cv::mixChannels(&image_bin, 1, &dst, 3, fromto, 3);

	////////////////////////////////////////////////////////////
	//	文字候補の切り出し・出力								  //
	////////////////////////////////////////////////////////////
	cout << "文字候補画像の切り出し 開始" << endl;	// Debug message
	char filename[128];	// 切り出しファイル名
	cv::Mat roi_img;		// 切り出し画像
	for(int i=0; i<count2; i++){
		sprintf(filename, "img_out/%d.png", i);
		cv::Mat roi_img(image_bin, cv::Rect(data2[i].minX, data2[i].minY, data2[i].maxX-data2[i].minX+1, data2[i].maxY-data2[i].minY+1));	// 画像の切り出し
		cv::imwrite(filename, roi_img);	// ファイルの書き出し
	}
	cout << "文字候補画像の切り出し 終了" << endl;	// Debug message

	////////////////////////////////////////////////////////////
	//	文字候補枠の描画										  //
	////////////////////////////////////////////////////////////
	for(int i=0; i<count2; i++){
		cv::rectangle(dst, cv::Point(data2[i].minX, data2[i].minY), cv::Point(data2[i].maxX, data2[i].maxY), cv::Scalar(0, 0, 200), 1, 4);
	}

	////////////////////////////////////////////////////////////
	//	最終結果の出力										  //
	////////////////////////////////////////////////////////////
	imwrite("Result_image.png", dst);

	return 0;
}