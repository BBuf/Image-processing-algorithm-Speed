#include <stdio.h>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

int ComparisonFunction(const void *X, const void *Y) {
	unsigned char Dx = *(unsigned char *)X;
	unsigned char Dy = *(unsigned char *)Y;
	if (Dx < Dy) return -1;
	else if (Dx > Dy) return 1;
	else return 0;
}

void MedianBlur3X3_Ori(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride) {
	int Channel = Stride / Width;
	if (Channel == 1) {
		unsigned char Array[9];
		for (int Y = 1; Y < Height - 1; Y++) {
			unsigned char *LineP0 = Src + (Y - 1) * Stride + 1;
			unsigned char *LineP1 = LineP0 + Stride;
			unsigned char *LineP2 = LineP1 + Stride;
			unsigned char *LinePD = Dest + Y * Stride + 1;
			for (int X = 1; X < Width - 1; X++) {
				Array[0] = LineP0[X - 1];        Array[1] = LineP0[X];    Array[2] = LineP0[X + 1];
				Array[3] = LineP1[X - 1];        Array[4] = LineP1[X];    Array[5] = LineP2[X + 1];
				Array[6] = LineP2[X - 1];        Array[7] = LineP2[X];    Array[8] = LineP2[X + 1];
				qsort(Array, 9, sizeof(unsigned char), &ComparisonFunction);
				LinePD[X] = Array[4];
			}
		}
	}
	else {
		unsigned char ArrayB[9], ArrayG[9], ArrayR[9];
		for (int Y = 1; Y < Height - 1; Y++) {
			unsigned char *LineP0 = Src + (Y - 1) * Stride + 3;
			unsigned char *LineP1 = LineP0 + Stride;
			unsigned char *LineP2 = LineP1 + Stride;
			unsigned char *LinePD = Dest + Y * Stride + 3;
			for (int X = 1; X < Width - 1; X++){
				ArrayB[0] = LineP0[-3];       ArrayG[0] = LineP0[-2];       ArrayR[0] = LineP0[-1];
				ArrayB[1] = LineP0[0];        ArrayG[1] = LineP0[1];        ArrayR[1] = LineP0[2];
				ArrayB[2] = LineP0[3];        ArrayG[2] = LineP0[4];        ArrayR[2] = LineP0[5];

				ArrayB[3] = LineP1[-3];       ArrayG[3] = LineP1[-2];       ArrayR[3] = LineP1[-1];
				ArrayB[4] = LineP1[0];        ArrayG[4] = LineP1[1];        ArrayR[4] = LineP1[2];
				ArrayB[5] = LineP1[3];        ArrayG[5] = LineP1[4];        ArrayR[5] = LineP1[5];

				ArrayB[6] = LineP2[-3];       ArrayG[6] = LineP2[-2];       ArrayR[6] = LineP2[-1];
				ArrayB[7] = LineP2[0];        ArrayG[7] = LineP2[1];        ArrayR[7] = LineP2[2];
				ArrayB[8] = LineP2[3];        ArrayG[8] = LineP2[4];        ArrayR[8] = LineP2[5];

				qsort(ArrayB, 9, sizeof(unsigned char), &ComparisonFunction);
				qsort(ArrayG, 9, sizeof(unsigned char), &ComparisonFunction);
				qsort(ArrayR, 9, sizeof(unsigned char), &ComparisonFunction);

				LinePD[0] = ArrayB[4];
				LinePD[1] = ArrayG[4];
				LinePD[2] = ArrayR[4];

				LineP0 += 3;
				LineP1 += 3;
				LineP2 += 3;
				LinePD += 3;
			}
		}
	}
}

void Swap(int &X, int &Y) {
	X ^= Y;
	Y ^= X;
	X ^= Y;
}

void MedianBlur3X3_Faster(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride){
	int Channel = Stride / Width;
	if (Channel == 1){

		for (int Y = 1; Y < Height - 1; Y++){
			unsigned char *LineP0 = Src + (Y - 1) * Stride + 1;
			unsigned char *LineP1 = LineP0 + Stride;
			unsigned char *LineP2 = LineP1 + Stride;
			unsigned char *LinePD = Dest + Y * Stride + 1;
			for (int X = 1; X < Width - 1; X++){
				int Gray0, Gray1, Gray2, Gray3, Gray4, Gray5, Gray6, Gray7, Gray8;
				Gray0 = LineP0[X - 1];        Gray1 = LineP0[X];    Gray2 = LineP0[X + 1];
				Gray3 = LineP1[X - 1];        Gray4 = LineP1[X];    Gray5 = LineP2[X + 1];
				Gray6 = LineP2[X - 1];        Gray7 = LineP2[X];    Gray8 = LineP2[X + 1];

				if (Gray1 > Gray2) Swap(Gray1, Gray2);
				if (Gray4 > Gray5) Swap(Gray4, Gray5);
				if (Gray7 > Gray8) Swap(Gray7, Gray8);
				if (Gray0 > Gray1) Swap(Gray0, Gray1);
				if (Gray3 > Gray4) Swap(Gray3, Gray4);
				if (Gray6 > Gray7) Swap(Gray6, Gray7);
				if (Gray1 > Gray2) Swap(Gray1, Gray2);
				if (Gray4 > Gray5) Swap(Gray4, Gray5);
				if (Gray7 > Gray8) Swap(Gray7, Gray8);
				if (Gray0 > Gray3) Swap(Gray0, Gray3);
				if (Gray5 > Gray8) Swap(Gray5, Gray8);
				if (Gray4 > Gray7) Swap(Gray4, Gray7);
				if (Gray3 > Gray6) Swap(Gray3, Gray6);
				if (Gray1 > Gray4) Swap(Gray1, Gray4);
				if (Gray2 > Gray5) Swap(Gray2, Gray5);
				if (Gray4 > Gray7) Swap(Gray4, Gray7);
				if (Gray4 > Gray2) Swap(Gray4, Gray2);
				if (Gray6 > Gray4) Swap(Gray6, Gray4);
				if (Gray4 > Gray2) Swap(Gray4, Gray2);

				LinePD[X] = Gray4;
			}
		}

	}
	else{
		for (int Y = 1; Y < Height - 1; Y++){
			unsigned char *LineP0 = Src + (Y - 1) * Stride + 3;
			unsigned char *LineP1 = LineP0 + Stride;
			unsigned char *LineP2 = LineP1 + Stride;
			unsigned char *LinePD = Dest + Y * Stride + 3;
			for (int X = 1; X < Width - 1; X++){
				int Blue0, Blue1, Blue2, Blue3, Blue4, Blue5, Blue6, Blue7, Blue8;
				int Green0, Green1, Green2, Green3, Green4, Green5, Green6, Green7, Green8;
				int Red0, Red1, Red2, Red3, Red4, Red5, Red6, Red7, Red8;
				Blue0 = LineP0[-3];        Green0 = LineP0[-2];    Red0 = LineP0[-1];
				Blue1 = LineP0[0];        Green1 = LineP0[1];        Red1 = LineP0[2];
				Blue2 = LineP0[3];        Green2 = LineP0[4];        Red2 = LineP0[5];

				Blue3 = LineP1[-3];        Green3 = LineP1[-2];    Red3 = LineP1[-1];
				Blue4 = LineP1[0];        Green4 = LineP1[1];        Red4 = LineP1[2];
				Blue5 = LineP1[3];        Green5 = LineP1[4];        Red5 = LineP1[5];

				Blue6 = LineP2[-3];        Green6 = LineP2[-2];    Red6 = LineP2[-1];
				Blue7 = LineP2[0];        Green7 = LineP2[1];        Red7 = LineP2[2];
				Blue8 = LineP2[3];        Green8 = LineP2[4];        Red8 = LineP2[5];

				if (Blue1 > Blue2) Swap(Blue1, Blue2);
				if (Blue4 > Blue5) Swap(Blue4, Blue5);
				if (Blue7 > Blue8) Swap(Blue7, Blue8);
				if (Blue0 > Blue1) Swap(Blue0, Blue1);
				if (Blue3 > Blue4) Swap(Blue3, Blue4);
				if (Blue6 > Blue7) Swap(Blue6, Blue7);
				if (Blue1 > Blue2) Swap(Blue1, Blue2);
				if (Blue4 > Blue5) Swap(Blue4, Blue5);
				if (Blue7 > Blue8) Swap(Blue7, Blue8);
				if (Blue0 > Blue3) Swap(Blue0, Blue3);
				if (Blue5 > Blue8) Swap(Blue5, Blue8);
				if (Blue4 > Blue7) Swap(Blue4, Blue7);
				if (Blue3 > Blue6) Swap(Blue3, Blue6);
				if (Blue1 > Blue4) Swap(Blue1, Blue4);
				if (Blue2 > Blue5) Swap(Blue2, Blue5);
				if (Blue4 > Blue7) Swap(Blue4, Blue7);
				if (Blue4 > Blue2) Swap(Blue4, Blue2);
				if (Blue6 > Blue4) Swap(Blue6, Blue4);
				if (Blue4 > Blue2) Swap(Blue4, Blue2);

				if (Green1 > Green2) Swap(Green1, Green2);
				if (Green4 > Green5) Swap(Green4, Green5);
				if (Green7 > Green8) Swap(Green7, Green8);
				if (Green0 > Green1) Swap(Green0, Green1);
				if (Green3 > Green4) Swap(Green3, Green4);
				if (Green6 > Green7) Swap(Green6, Green7);
				if (Green1 > Green2) Swap(Green1, Green2);
				if (Green4 > Green5) Swap(Green4, Green5);
				if (Green7 > Green8) Swap(Green7, Green8);
				if (Green0 > Green3) Swap(Green0, Green3);
				if (Green5 > Green8) Swap(Green5, Green8);
				if (Green4 > Green7) Swap(Green4, Green7);
				if (Green3 > Green6) Swap(Green3, Green6);
				if (Green1 > Green4) Swap(Green1, Green4);
				if (Green2 > Green5) Swap(Green2, Green5);
				if (Green4 > Green7) Swap(Green4, Green7);
				if (Green4 > Green2) Swap(Green4, Green2);
				if (Green6 > Green4) Swap(Green6, Green4);
				if (Green4 > Green2) Swap(Green4, Green2);

				if (Red1 > Red2) Swap(Red1, Red2);
				if (Red4 > Red5) Swap(Red4, Red5);
				if (Red7 > Red8) Swap(Red7, Red8);
				if (Red0 > Red1) Swap(Red0, Red1);
				if (Red3 > Red4) Swap(Red3, Red4);
				if (Red6 > Red7) Swap(Red6, Red7);
				if (Red1 > Red2) Swap(Red1, Red2);
				if (Red4 > Red5) Swap(Red4, Red5);
				if (Red7 > Red8) Swap(Red7, Red8);
				if (Red0 > Red3) Swap(Red0, Red3);
				if (Red5 > Red8) Swap(Red5, Red8);
				if (Red4 > Red7) Swap(Red4, Red7);
				if (Red3 > Red6) Swap(Red3, Red6);
				if (Red1 > Red4) Swap(Red1, Red4);
				if (Red2 > Red5) Swap(Red2, Red5);
				if (Red4 > Red7) Swap(Red4, Red7);
				if (Red4 > Red2) Swap(Red4, Red2);
				if (Red6 > Red4) Swap(Red6, Red4);
				if (Red4 > Red2) Swap(Red4, Red2);

				LinePD[0] = Blue4;
				LinePD[1] = Green4;
				LinePD[2] = Red4;

				LineP0 += 3;
				LineP1 += 3;
				LineP2 += 3;
				LinePD += 3;
			}
		}
	}
}

inline void _mm_sort_ab(__m128i &a, __m128i &b){
	const __m128i min = _mm_min_epu8(a, b);
	const __m128i max = _mm_max_epu8(a, b);
	a = min;
	b = max;
}

void MedianBlur3X3_Fastest(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride) {
	int Channel = Stride / Width;
	int BlockSize = 16, Block = ((Width - 2)* Channel) / BlockSize;
	for (int Y = 1; Y < Height - 1; Y++){
		unsigned char *LineP0 = Src + (Y - 1) * Stride + Channel;
		unsigned char *LineP1 = LineP0 + Stride;
		unsigned char *LineP2 = LineP1 + Stride;
		unsigned char *LinePD = Dest + Y * Stride + Channel;
		for (int X = 0; X < Block * BlockSize; X += BlockSize, LineP0 += BlockSize, LineP1 += BlockSize, LineP2 += BlockSize, LinePD += BlockSize){
			__m128i P0 = _mm_loadu_si128((__m128i *)(LineP0 - Channel));
			__m128i P1 = _mm_loadu_si128((__m128i *)(LineP0 - 0));
			__m128i P2 = _mm_loadu_si128((__m128i *)(LineP0 + Channel));
			__m128i P3 = _mm_loadu_si128((__m128i *)(LineP1 - Channel));
			__m128i P4 = _mm_loadu_si128((__m128i *)(LineP1 - 0));
			__m128i P5 = _mm_loadu_si128((__m128i *)(LineP1 + Channel));
			__m128i P6 = _mm_loadu_si128((__m128i *)(LineP2 - Channel));
			__m128i P7 = _mm_loadu_si128((__m128i *)(LineP2 - 0));
			__m128i P8 = _mm_loadu_si128((__m128i *)(LineP2 + Channel));

			_mm_sort_ab(P1, P2);        _mm_sort_ab(P4, P5);        _mm_sort_ab(P7, P8);
			_mm_sort_ab(P0, P1);        _mm_sort_ab(P3, P4);        _mm_sort_ab(P6, P7);
			_mm_sort_ab(P1, P2);        _mm_sort_ab(P4, P5);        _mm_sort_ab(P7, P8);
			_mm_sort_ab(P0, P3);        _mm_sort_ab(P5, P8);        _mm_sort_ab(P4, P7);
			_mm_sort_ab(P3, P6);        _mm_sort_ab(P1, P4);        _mm_sort_ab(P2, P5);
			_mm_sort_ab(P4, P7);        _mm_sort_ab(P4, P2);        _mm_sort_ab(P6, P4);
			_mm_sort_ab(P4, P2);

			_mm_storeu_si128((__m128i *)LinePD, P4);
		}
		for (int X = Block * BlockSize; X < (Width - 2) * Channel; X++, LinePD++){
			int Gray0, Gray1, Gray2, Gray3, Gray4, Gray5, Gray6, Gray7, Gray8;
			Gray0 = LineP0[X - 1];        Gray1 = LineP0[X];    Gray2 = LineP0[X + 1];
			Gray3 = LineP1[X - 1];        Gray4 = LineP1[X];    Gray5 = LineP2[X + 1];
			Gray6 = LineP2[X - 1];        Gray7 = LineP2[X];    Gray8 = LineP2[X + 1];

			if (Gray1 > Gray2) Swap(Gray1, Gray2);
			if (Gray4 > Gray5) Swap(Gray4, Gray5);
			if (Gray7 > Gray8) Swap(Gray7, Gray8);
			if (Gray0 > Gray1) Swap(Gray0, Gray1);
			if (Gray3 > Gray4) Swap(Gray3, Gray4);
			if (Gray6 > Gray7) Swap(Gray6, Gray7);
			if (Gray1 > Gray2) Swap(Gray1, Gray2);
			if (Gray4 > Gray5) Swap(Gray4, Gray5);
			if (Gray7 > Gray8) Swap(Gray7, Gray8);
			if (Gray0 > Gray3) Swap(Gray0, Gray3);
			if (Gray5 > Gray8) Swap(Gray5, Gray8);
			if (Gray4 > Gray7) Swap(Gray4, Gray7);
			if (Gray3 > Gray6) Swap(Gray3, Gray6);
			if (Gray1 > Gray4) Swap(Gray1, Gray4);
			if (Gray2 > Gray5) Swap(Gray2, Gray5);
			if (Gray4 > Gray7) Swap(Gray4, Gray7);
			if (Gray4 > Gray2) Swap(Gray4, Gray2);
			if (Gray6 > Gray4) Swap(Gray6, Gray4);
			if (Gray4 > Gray2) Swap(Gray4, Gray2);

			LinePD[X] = Gray4;
		}
	}
}

int main() {
	Mat src = imread("F:\\car.jpg");
	int Height = src.rows;
	int Width = src.cols;
	unsigned char *Src = src.data;
	unsigned char *Dest = new unsigned char[Height * Width * 3];
	int Stride = Width * 3;
	int Radius = 7;
	int64 st = cvGetTickCount();
	for (int i = 0; i <10; i++) {
		//Mat temp = MaxFilter(src, Radius);
		MedianBlur3X3_Ori(Src, Dest, Width, Height, Stride);
	}
	double duration = (cv::getTickCount() - st) / cv::getTickFrequency() * 100;
	printf("%.5f\n", duration);
	MedianBlur3X3_Fastest(Src, Dest, Width, Height, Stride);
	Mat dst(Height, Width, CV_8UC3, Dest);
	imshow("origin", src);
	imshow("result", dst);
	imwrite("F:\\res.jpg", dst);
	waitKey(0);
	return 0;
}