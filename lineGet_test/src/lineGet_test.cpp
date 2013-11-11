#include <iostream>
#include <vector>
#include <cv.h>
#include <highgui.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

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

struct lineArea{
	char* filename;
	char* dirname;
	vector<struct areaData> data;
};

void outputLineImage(char* dirname, char* filename);
vector<struct areaData> borderFollowing(const char* filename, const char*dirname);


int main(int argc, char** argv) {
	// char argv[1][] = argv[1];		// 文字画像出力先ディレクトリ
	struct stat st;	// stat関数で得られる情報の保持
	struct dirent *dist;

	if(stat(argv[1], &st) != 0){	// 成功:0,失敗:-1
		// ディレクトリが存在しない場合
		cout << "ディレクトリ " << argv[1] << " は存在しません" << endl;		// Debug message
	}else{
		// ディレクトリが存在する場合
		DIR *dp = opendir(argv[1]);

		while((dist = readdir(dp)) != NULL){
			char filepath[64];	// ファイルパス（ディレクトリ名＋ファイル名）
			sprintf(filepath, "%s/%s", argv[1], dist->d_name);
			outputLineImage(argv[1], dist->d_name);	// 画素の膨張
		}

		closedir(dp);
	}

	return 0;
}

void outputLineImage(char *dirname, char *filename){
	cv::Mat image_Input;
	cv::Mat image_Input_bin;
	cv::Mat image_Result;
	char filepath[64];
	vector<struct lineArea> lineData;
	struct lineArea lineDataTmp;



	////////////////////////////////////////////////////////////
	//	画像の読み込み・黒画素の膨張							  //
	////////////////////////////////////////////////////////////
	sprintf(filepath, "%s/%s", dirname, filename);
	image_Input = cv::imread(filepath, 0);

	// エラー処理（Matに正しく画像が読み込まれていない場合のエラー処理）
	if(image_Input.empty()){	// cv::Mat.empty() 行列にデータが確保されてない場合true
		cout << "Error: No image data (" << filepath << ")" << endl;	// Debug message.
	}else{
		cv::threshold(image_Input, image_Input_bin, 0, 255, cv::THRESH_BINARY);	// 画像の二値化

		image_Result = cv::Mat(image_Input.size(), CV_8UC3);	// 黒画素を膨張させた出力結果の格納先

		// 黒画素を膨張（cv::erodeは白画素を縮小）
		cv::erode(image_Input_bin, image_Result, cv::Mat(cv::Size(5,1), CV_8U), cv::Point(-1,-1), 15);

		sprintf(filepath, "./OutputImage/%s", filename);
		if(cv::imwrite(filepath, image_Result)){	// 結果の出力
			cout << "Log:  Output Successful (" << filepath << ")" << endl;		// Debug message.
			lineDataTmp.filename = filename;
			lineDataTmp.dirname = dirname;
			lineDataTmp.data = borderFollowing(filename, "OutputImage");
			lineData.push_back(lineDataTmp);
		}else{
			cout << "Error: Output Failed (" << filepath << ")" << endl;	// Debug message.
		}
	}
}

vector<struct areaData> borderFollowing(const char* filename, const char* dirname){
	vector<struct areaData> result;	// 最終結果
	struct areaData tmpData;
	cv::Mat image;
	cv::Mat image_bin;
	char filepath[128];
	int minX, minY;	// 境界線追跡で得た座標の最小値
	int maxX, maxY;	// 境界線追跡で得た座標の最大値
	int sx=0, sy=0;	// 境界線追跡開始点
	int px, py;		// 境界線追跡中の現在地
	int op = 2;		// 境界線追跡中の次に探索する方向
	int flag = 0;		// ループ脱出フラグ
	int bflag = 1;	// 境界線追跡フラグ（0:非実行, 1:実行)
	int endFlag = 0;	// 終了フラグ
	int count=0;


	////////////////////////////////////////////////////////////
	//	画像の読み込み										  //
	////////////////////////////////////////////////////////////
	sprintf(filepath, "./%s/%s", dirname, filename);
	image = cv::imread(filepath, 0);
/*
	// エラー処理（Matに正しく画像が読み込まれていない場合のエラー処理）
	if(image.empty()){	// cv::Mat.empty() 行列にデータが確保されてない場合true
		printf( "No image data\n" );
		return -1;
	}
*/
	cv::threshold(image, image_bin, 0, 255, cv::THRESH_BINARY|cv::THRESH_OTSU);	// 画像の二値化


	////////////////////////////////////////////////////////////
	//	境界線追跡											  //
	////////////////////////////////////////////////////////////
	while(!endFlag && !(image.empty())){
		// 始点の取得（ラスタスキャン順）
		for(int i=sy; i<image.rows; i++){
			for(int j=sx; j<image.cols; j++){
				if(image_bin.at<unsigned char>(i, j) == 0){
					if((image_bin.at<unsigned char>(i-1, j) == 1) || (image_bin.at<unsigned char>(i, j-1) == 1) || (image_bin.at<unsigned char>(i+1, j) == 1) || (image_bin.at<unsigned char>(i, j+1) == 1)){
						image_bin.at<unsigned char>(i, j) = 1;
					}else{
						sx = px = maxX = minX = j;
						sy = py = maxY = minY = i;
						flag = 1;
						break;
					}
				}
				// 終了条件(探索箇所が対象の最も右下の位置ならば)
				if(i==image.rows-1 && j==image.cols-1){
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
			while(flag){
				switch(op){
					case 0:	// 左上
						if(image_bin.at<unsigned char>(py-1, px-1) != 255){
							px = px - 1;
							py = py - 1;
							op = 6;
							break;
						}
						/* no break */
					case 1:	// 左
						if(image_bin.at<unsigned char>(py, px-1) != 255){
							px = px - 1;
							op = 0;
							break;
						}
						/* no break */
					case 2:	// 左下
						if(image_bin.at<unsigned char>(py+1, px-1) != 255){
							px = px - 1;
							py = py + 1;
							op = 0;
							break;
						}
						/* no break */
					case 3:	// 下
						if(image_bin.at<unsigned char>(py+1, px) != 255){
							py = py + 1;
							op = 2;
							break;
						}
						/* no break */
					case 4:	// 右下
						if(image_bin.at<unsigned char>(py+1, px+1) != 255){
							px = px + 1;
							py = py + 1;
							op = 2;
							break;
						}
						/* no break */
					case 5:	// 右
						if(image_bin.at<unsigned char>(py, px+1) != 255){
							px = px + 1;
							op = 4;
							break;
						}
						/* no break */
					case 6:	// 右上
						if(image_bin.at<unsigned char>(py-1, px+1) != 255){
							px = px + 1;
							py = py - 1;
							op = 4;
							break;
						}
						/* no break */
					case 7:	// 上
						if(image_bin.at<unsigned char>(py-1, px) != 255){
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

				image_bin.at<unsigned char>(py, px) = 1;

				// 終了条件確認
				if(px==sx && py==sy) flag = 0;
			}

			// 最大値・最小値座標の出力
			tmpData.maxX = maxX;
			tmpData.maxY = maxY;
			tmpData.minX = minX;
			tmpData.minY = minY;
			tmpData.sx = sx;
			tmpData.sy = sy;
			tmpData.Flag = 1;
			cout << "Log: tmpData [" << minX << "]:[" << minY << "], [" << maxX << "]:[" << maxY << "]" << " (" << filepath << ")" << endl;
			result.push_back(tmpData);	// dataの末尾に探索結果を追加
			count++;
		}
		bflag = 1;
		flag = 0;
		op = 2;		// 境界線追跡中の次に探索する方向を初期化
	}

	printf("境界線追跡後：%d\n", count);	// Debug

	cv::Mat dst(image.size(), CV_8UC3);
	int fromto[] = {0,0, 0,1, 0,2};
	cv::mixChannels(&image_bin, 1, &dst, 1, fromto, 3);

	for(int i=0; i<count; i++){
		cv::rectangle(dst, cv::Point(result[i].minX, result[i].minY), cv::Point(result[i].maxX, result[i].maxY), cv::Scalar(0, 0, 200), 1, 4);
	}

	sprintf(filepath, "./OutputImage2/%s", filename);
	imwrite(filepath, dst);

	return result;
}
