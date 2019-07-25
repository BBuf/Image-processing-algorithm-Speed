#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "../../OpencvTest/OpencvTest/Core.h"
#include "../../OpencvTest/OpencvTest/MaxFilter.h"
#include "../../OpencvTest/OpencvTest/Utility.h"
using namespace std;
using namespace cv;

void MaxFilter(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride, int Channel, int Radius) {
	TMatrix a, b;
	TMatrix *p1 = &a, *p2 = &b;
	TMatrix **p3 = &p1, **p4 = &p2;
	IS_CreateMatrix(Width, Height, IS_DEPTH_8U, Channel, p3);
	IS_CreateMatrix(Width, Height, IS_DEPTH_8U, Channel, p4);
	(p1)->Data = Src;
	(p2)->Data = Dest;
	MaxFilter(p1, p2, Radius);
}

int main() {
	Mat src = imread("F:\\car.jpg");
	int Height = src.rows;
	int Width = src.cols;
	unsigned char *Src = src.data;
	unsigned char *Dest = new unsigned char[Height * Width * 3];
	int Stride = Width * 3;
	int Radius = 0;
	int64 st = cvGetTickCount();
	for (int i = 0; i <50; i++) {
		MaxFilter(Src, Dest, Width, Height, Stride, 3, Radius);
	}
	double duration = (cv::getTickCount() - st) / cv::getTickFrequency() * 20;
	printf("%.5f\n", duration);
	MaxFilter(Src, Dest, Width, Height, Stride, 3, Radius);
	Mat dst(Height, Width, CV_8UC3, Dest);
	imshow("origin", src);
	imshow("result", dst);
	imwrite("F:\\res.jpg", dst);
	waitKey(0);
	return 0;
}