#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

void GetGrayIntegralImage(unsigned char *Src, int *Integral, int Width, int Height, int Stride)
{
	memset(Integral, 0, (Width + 1) * sizeof(int));                    //    第一行都为0
	for (int Y = 0; Y < Height; Y++)
	{
		unsigned char *LinePS = Src + Y * Stride;
		int *LinePL = Integral + Y * (Width + 1) + 1;                //上一行的位置
		int *LinePD = Integral + (Y + 1) * (Width + 1) + 1;           //    当前位置，注意每行的第一列的值都为0
		LinePD[-1] = 0;                                               //    第一列的值为0
		for (int X = 0, Sum = 0; X < Width; X++)
		{
			Sum += LinePS[X];                                          //    行方向累加
			LinePD[X] = LinePL[X] + Sum;                               //    更新积分图
		}
	}
}

void GetGrayIntegralImage_SSE(unsigned char *Src, int *Integral, int Width, int Height, int Stride) {
	memset(Integral, 0, (Width + 1) * sizeof(int)); //第一行都为0
	int BlockSize = 8, Block = Width / BlockSize;
	for (int Y = 0; Y < Height; Y++) {
		unsigned char *LinePS = Src + Y * Stride;
		int *LinePL = Integral + Y * (Width + 1) + 1; //上一行位置
		int *LinePD = Integral + (Y + 1) * (Width + 1) + 1; //当前位置，注意每行的第一列都为0
		LinePD[-1] = 0;
		__m128i PreV = _mm_setzero_si128();
		__m128i Zero = _mm_setzero_si128();
		for (int X = 0; X < Block * BlockSize; X += BlockSize) {
			__m128i Src_Shift0 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*)(LinePS + X)), Zero); //A7 A6 A5 A 4 A3 A2 A1 A0
			__m128i Src_Shift1 = _mm_slli_si128(Src_Shift0, 2); //A6 A5 A4 A3 A2 A1 A0 0
			__m128i Src_Shift2 = _mm_slli_si128(Src_Shift1, 2); //A5 A4 A3 A2 A1 A0 0  0
			__m128i Src_Shift3 = _mm_slli_si128(Src_Shift2, 2); //A4 A3 A2 A1 A0 0  0  0
			__m128i Shift_Add12 = _mm_add_epi16(Src_Shift1, Src_Shift2); //A6+A5 A5+A4 A4+A3 A3+A2 A2+A1 A1+A0 A0+0  0+0
			__m128i Shift_Add03 = _mm_add_epi16(Src_Shift0, Src_Shift3); //A7+A4 A6+A3 A5+A2 A4+A1 A3+A0 A2+0  A1+0  A0+0 
			__m128i Low = _mm_add_epi16(Shift_Add12, Shift_Add03); //A7+A6+A5+A4 A6+A5+A4+A3 A5+A4+A3+A2 A4+A3+A2+A1 A3+A2+A1+A0 A2+A1+A0+0 A1+A0+0+0 A0+0+0+0
			__m128i High = _mm_add_epi32(_mm_unpackhi_epi16(Low, Zero), _mm_unpacklo_epi16(Low, Zero)); //A7+A6+A5+A4+A3+A2+A1+A0  A6+A5+A4+A3+A2+A1+A0  A5+A4+A3+A2+A1+A0  A4+A3+A2+A1+A0
			__m128i SumL = _mm_loadu_si128((__m128i *)(LinePL + X + 0));
			__m128i SumH = _mm_loadu_si128((__m128i *)(LinePL + X + 4));
			SumL = _mm_add_epi32(SumL, PreV);
			SumL = _mm_add_epi32(SumL, _mm_unpacklo_epi16(Low, Zero));
			SumH = _mm_add_epi32(SumH, PreV);
			SumH = _mm_add_epi32(SumH, High);
			PreV = _mm_add_epi32(PreV, _mm_shuffle_epi32(High, _MM_SHUFFLE(3, 3, 3, 3)));
			_mm_storeu_si128((__m128i *)(LinePD + X + 0), SumL);
			_mm_storeu_si128((__m128i *)(LinePD + X + 4), SumH);
		}
		for (int X = Block * BlockSize, V = LinePD[X - 1] - LinePL[X - 1]; X < Width; X++)
		{
			V += LinePS[X];
			LinePD[X] = V + LinePL[X];
		}
	}
}

void BoxBlur(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride, int Radius) {
	int *Integral = (int *)malloc((Width + 1) * (Height + 1) * sizeof(int));
	GetGrayIntegralImage(Src, Integral, Width, Height, Stride);
//#pragma parallel for num_threads(4)
	for (int Y = 0; Y < Height; Y++) {
		int Y1 = max(Y - Radius, 0);
		int Y2 = min(Y + Radius + 1, Height - 1);
		int *LineP1 = Integral + Y1 * (Width + 1);
		int *LineP2 = Integral + Y2 * (Width + 1);
		unsigned char *LinePD = Dest + Y * Stride;
		for (int X = 0; X < Height; X++) {
			int X1 = max(X - Radius, 0);
			int X2 = min(X + Radius + 1, Width);
			int Sum = LineP2[X2] - LineP1[X2] - LineP2[X1] + LineP1[X1];
			int PixelCount = (X2 - X1) * (Y2 - Y1);
			LinePD[X] = (Sum + (PixelCount >> 1)) / PixelCount;
		}
	}
	free(Integral);
}

int main() {
	Mat src = imread("F:\\car.jpg", 0);
	int Height = src.rows;
	int Width = src.cols;
	unsigned char *Src = src.data;
	unsigned char *Dest = new unsigned char[Height * Width];
	int Stride = Width;
	int Radius = 11;
	int64 st = cvGetTickCount();
	for (int i = 0; i < 10; i++) {
		BoxBlur(Src, Dest, Width, Height, Stride, Radius);
	}
	double duration = (cv::getTickCount() - st) / cv::getTickFrequency() * 100;
	printf("%.5f\n", duration);
	BoxBlur(Src, Dest, Width, Height, Stride, Radius);
	Mat dst(Height, Width, CV_8UC1, Dest);
	imshow("origin", src);
	imshow("result", dst);
	imwrite("F:\\res.jpg", dst);
	waitKey(0);
	waitKey(0);
}