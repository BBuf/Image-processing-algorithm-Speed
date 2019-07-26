#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "../../OpencvTest/OpencvTest/Core.h"
#include "../../OpencvTest/OpencvTest/MaxFilter.h"
#include "../../OpencvTest/OpencvTest/Utility.h"
using namespace std;
using namespace cv;

void MaxFilter_SSE(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride, int Channel, int Radius) {
	TMatrix a, b;
	TMatrix *p1 = &a, *p2 = &b;
	TMatrix **p3 = &p1, **p4 = &p2;
	IS_CreateMatrix(Width, Height, IS_DEPTH_8U, Channel, p3);
	IS_CreateMatrix(Width, Height, IS_DEPTH_8U, Channel, p4);
	(p1)->Data = Src;
	(p2)->Data = Dest;
	MaxFilter(p1, p2, Radius);
}

Mat MaxFilter(Mat src, int radius) {
	int row = src.rows;
	int col = src.cols;
	int border = (radius - 1) / 2;
	Mat dst(row, col, CV_8UC3);
	printf("success\n");
	for (int i = border; i + border < row; i++) {
		for (int j = border; j + border < col; j++) {
			for (int k = 0; k < 3; k++) {
				int val = src.at<Vec3b>(i, j)[k];
				for (int x = -border; x <= border; x++) {
					for (int y = -border; y <= border; y++) {
						val = max(val, (int)src.at<Vec3b>(i + x, j + y)[k]);
					}
				}
				dst.at<Vec3b>(i, j)[k] = val;
			}
		}
	}
	printf("success\n");
	return dst;
}

int main() {
	Mat src = imread("F:\\car.jpg");
	int Height = src.rows;
	int Width = src.cols;
	unsigned char *Src = src.data;
	unsigned char *Dest = new unsigned char[Height * Width * 3];
	int Stride = Width * 3;
	int Radius = 11;
	int64 st = cvGetTickCount();
	for (int i = 0; i <10; i++) {
		Mat temp = MaxFilter(src, Radius);
		//MaxFilter_SSE(Src, Dest, Width, Height, Stride, 3, Radius);
	}
	double duration = (cv::getTickCount() - st) / cv::getTickFrequency() * 100;
	printf("%.5f\n", duration);
	MaxFilter_SSE(Src, Dest, Width, Height, Stride, 3, Radius);
	Mat dst(Height, Width, CV_8UC3, Dest);
	imshow("origin", src);
	imshow("result", dst);
	imwrite("F:\\res.jpg", dst);
	waitKey(0);
	return 0;
}