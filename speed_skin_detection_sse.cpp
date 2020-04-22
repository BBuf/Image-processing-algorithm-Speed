#include "stdafx.h"
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <future>
using namespace std;
using namespace cv;

#define IM_Max(a, b) (((a) >= (b)) ? (a): (b))
#define IM_Min(a, b) (((a) >= (b)) ? (b): (a))
#define _mm_cmpge_epu8(a, b) _mm_cmpeq_epi8(_mm_max_epu8(a, b), a)

void IM_GetRoughSkinRegion(unsigned char *Src, unsigned char *Skin, int Width, int Height, int Stride) {
	for (int Y = 0; Y < Height; Y++)
	{
		unsigned char *LinePS = Src + Y * Stride;
		unsigned char *LinePD = Skin + Y * Width;
		for (int X = 0; X < Width; X++)
		{
			int Blue = LinePS[0], Green = LinePS[1], Red = LinePS[2];
			if (Red >= 60 && Green >= 40 && Blue >= 20 && Red >= Blue && (Red - Green) >= 10 && IM_Max(IM_Max(Red, Green), Blue) - IM_Min(IM_Min(Red, Green), Blue) >= 10)
				LinePD[X] = 255;
			else
				LinePD[X] = 16;
			LinePS += 3;
		}
	}
}

void IM_GetRoughSkinRegion_OpenMP(unsigned char *Src, unsigned char *Skin, int Width, int Height, int Stride) {
	for (int Y = 0; Y < Height; Y++)
	{
		unsigned char *LinePS = Src + Y * Stride;
		unsigned char *LinePD = Skin + Y * Width;
#pragma omp parallel for num_threads(4)
		for (int X = 0; X < Width; X++)
		{
			int Blue = LinePS[X*3 + 0], Green = LinePS[X*3 + 1], Red = LinePS[X*3 + 2];
			if (Red >= 60 && Green >= 40 && Blue >= 20 && Red >= Blue && (Red - Green) >= 10 && IM_Max(IM_Max(Red, Green), Blue) - IM_Min(IM_Min(Red, Green), Blue) >= 10)
				LinePD[X] = 255;
			else
				LinePD[X] = 16;
		}
	}
}


void IM_GetRoughSkinRegion_SSE(unsigned char *Src, unsigned char *Skin, int Width, int Height, int Stride) {
	const int NonSkinLevel = 10; //非肤色部分的处理程序，本例取16，最大值取100，那样就是所有区域都为肤色，毫无意义
	const int BlockSize = 16;
	int Block = Width / BlockSize;
	for (int Y = 0; Y < Height; Y++) {
		unsigned char *LinePS = Src + Y * Stride;
		unsigned char *LinePD = Skin + Y * Width;
		for (int X = 0; X < Block * BlockSize; X += BlockSize, LinePS += BlockSize * 3, LinePD += BlockSize) {
			__m128i Src1, Src2, Src3, Blue, Green, Red, Result, Max, Min, AbsDiff;
			Src1 = _mm_loadu_si128((__m128i *)(LinePS + 0));
			Src2 = _mm_loadu_si128((__m128i *)(LinePS + 16));
			Src3 = _mm_loadu_si128((__m128i *)(LinePS + 32));

			Blue = _mm_shuffle_epi8(Src1, _mm_setr_epi8(0, 3, 6, 9, 12, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1));
			Blue = _mm_or_si128(Blue, _mm_shuffle_epi8(Src2, _mm_setr_epi8(-1, -1, -1, -1, -1, -1, 2, 5, 8, 11, 14, -1, -1, -1, -1, -1)));
			Blue = _mm_or_si128(Blue, _mm_shuffle_epi8(Src3, _mm_setr_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, 4, 7, 10, 13)));

			Green = _mm_shuffle_epi8(Src1, _mm_setr_epi8(1, 4, 7, 10, 13, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1));
			Green = _mm_or_si128(Green, _mm_shuffle_epi8(Src2, _mm_setr_epi8(-1, -1, -1, -1, -1, 0, 3, 6, 9, 12, 15, -1, -1, -1, -1, -1)));
			Green = _mm_or_si128(Green, _mm_shuffle_epi8(Src3, _mm_setr_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 2, 5, 8, 11, 14)));

			Red = _mm_shuffle_epi8(Src1, _mm_setr_epi8(2, 5, 8, 11, 14, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1));
			Red = _mm_or_si128(Red, _mm_shuffle_epi8(Src2, _mm_setr_epi8(-1, -1, -1, -1, -1, 1, 4, 7, 10, 13, -1, -1, -1, -1, -1, -1)));
			Red = _mm_or_si128(Red, _mm_shuffle_epi8(Src3, _mm_setr_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 3, 6, 9, 12, 15)));

			Max = _mm_max_epu8(_mm_max_epu8(Blue, Green), Red); //IM_Max(IM_Max(Red, Green), Blue)
			Min = _mm_min_epu8(_mm_min_epu8(Blue, Green), Red); //IM_Min(IM_Min(Red, Green), Blue)
			Result = _mm_cmpge_epu8(Blue, _mm_set1_epi8(20)); //Blue >= 20
			Result = _mm_and_si128(Result, _mm_cmpge_epu8(Green, _mm_set1_epi8(40))); //Green >= 40
			Result = _mm_and_si128(Result, _mm_cmpge_epu8(Red, _mm_set1_epi8(60))); //Red >= 60
			Result = _mm_and_si128(Result, _mm_cmpge_epu8(Red, Blue)); //Red >= Blue
			Result = _mm_and_si128(Result, _mm_cmpge_epu8(_mm_subs_epu8(Red, Green), _mm_set1_epi8(10))); //(Red - Green) >= 10 
			Result = _mm_and_si128(Result, _mm_cmpge_epu8(_mm_subs_epu8(Max, Min), _mm_set1_epi8(10))); //IM_Max(IM_Max(Red, Green), Blue) - IM_Min(IM_Min(Red, Green), Blue) >= 10
			Result = _mm_or_si128(Result, _mm_set1_epi8(16));
			_mm_storeu_si128((__m128i*)(LinePD + 0), Result);
		}
		for (int X = Block * BlockSize; X < Width; X++, LinePS += 3, LinePD++)
		{
			int Blue = LinePS[0], Green = LinePS[1], Red = LinePS[2];
			if (Red >= 60 && Green >= 40 && Blue >= 20 && Red >= Blue && (Red - Green) >= 10 && IM_Max(IM_Max(Red, Green), Blue) - IM_Min(IM_Min(Red, Green), Blue) >= 10)
				LinePD[0] = 255;									//	全为肤色部分																			
			else
				LinePD[0] = 16;
		}
	}
}

void _IM_GetRoughSkinRegion(unsigned char* Src, const int32_t Width, const int32_t start_row, const int32_t thread_stride, const int32_t Stride, unsigned char* Dest) {
	const int NonSkinLevel = 10; //非肤色部分的处理程序，本例取16，最大值取100，那样就是所有区域都为肤色，毫无意义
	const int BlockSize = 16;
	int Block = Width / BlockSize;
	for (int Y = start_row; Y < start_row + thread_stride; Y++) {
		unsigned char *LinePS = Src + Y * Stride;
		unsigned char *LinePD = Dest + Y * Width;
		for (int X = 0; X < Block * BlockSize; X += BlockSize, LinePS += BlockSize * 3, LinePD += BlockSize) {
			__m128i Src1, Src2, Src3, Blue, Green, Red, Result, Max, Min, AbsDiff;
			Src1 = _mm_loadu_si128((__m128i *)(LinePS + 0));
			Src2 = _mm_loadu_si128((__m128i *)(LinePS + 16));
			Src3 = _mm_loadu_si128((__m128i *)(LinePS + 32));

			Blue = _mm_shuffle_epi8(Src1, _mm_setr_epi8(0, 3, 6, 9, 12, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1));
			Blue = _mm_or_si128(Blue, _mm_shuffle_epi8(Src2, _mm_setr_epi8(-1, -1, -1, -1, -1, -1, 2, 5, 8, 11, 14, -1, -1, -1, -1, -1)));
			Blue = _mm_or_si128(Blue, _mm_shuffle_epi8(Src3, _mm_setr_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, 4, 7, 10, 13)));

			Green = _mm_shuffle_epi8(Src1, _mm_setr_epi8(1, 4, 7, 10, 13, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1));
			Green = _mm_or_si128(Green, _mm_shuffle_epi8(Src2, _mm_setr_epi8(-1, -1, -1, -1, -1, 0, 3, 6, 9, 12, 15, -1, -1, -1, -1, -1)));
			Green = _mm_or_si128(Green, _mm_shuffle_epi8(Src3, _mm_setr_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 2, 5, 8, 11, 14)));

			Red = _mm_shuffle_epi8(Src1, _mm_setr_epi8(2, 5, 8, 11, 14, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1));
			Red = _mm_or_si128(Red, _mm_shuffle_epi8(Src2, _mm_setr_epi8(-1, -1, -1, -1, -1, 1, 4, 7, 10, 13, -1, -1, -1, -1, -1, -1)));
			Red = _mm_or_si128(Red, _mm_shuffle_epi8(Src3, _mm_setr_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 3, 6, 9, 12, 15)));

			Max = _mm_max_epu8(_mm_max_epu8(Blue, Green), Red); //IM_Max(IM_Max(Red, Green), Blue)
			Min = _mm_min_epu8(_mm_min_epu8(Blue, Green), Red); //IM_Min(IM_Min(Red, Green), Blue)
			Result = _mm_cmpge_epu8(Blue, _mm_set1_epi8(20)); //Blue >= 20
			Result = _mm_and_si128(Result, _mm_cmpge_epu8(Green, _mm_set1_epi8(40))); //Green >= 40
			Result = _mm_and_si128(Result, _mm_cmpge_epu8(Red, _mm_set1_epi8(60))); //Red >= 60
			Result = _mm_and_si128(Result, _mm_cmpge_epu8(Red, Blue)); //Red >= Blue
			Result = _mm_and_si128(Result, _mm_cmpge_epu8(_mm_subs_epu8(Red, Green), _mm_set1_epi8(10))); //(Red - Green) >= 10 
			Result = _mm_and_si128(Result, _mm_cmpge_epu8(_mm_subs_epu8(Max, Min), _mm_set1_epi8(10))); //IM_Max(IM_Max(Red, Green), Blue) - IM_Min(IM_Min(Red, Green), Blue) >= 10
			Result = _mm_or_si128(Result, _mm_set1_epi8(16));
			_mm_storeu_si128((__m128i*)(LinePD + 0), Result);
		}
		for (int X = Block * BlockSize; X < Width; X++, LinePS += 3, LinePD++)
		{
			int Blue = LinePS[0], Green = LinePS[1], Red = LinePS[2];
			if (Red >= 60 && Green >= 40 && Blue >= 20 && Red >= Blue && (Red - Green) >= 10 && IM_Max(IM_Max(Red, Green), Blue) - IM_Min(IM_Min(Red, Green), Blue) >= 10)
				LinePD[0] = 255;									//	全为肤色部分																			
			else
				LinePD[0] = 16;
		}
	}
}

void IM_GetRoughSkinRegion_SSE2(unsigned char *Src, unsigned char *Skin, int width, int height, int stride) {
	const int32_t hw_concur = std::min(height >> 4, static_cast<int32_t>(std::thread::hardware_concurrency()));
	std::vector<std::future<void>> fut(hw_concur);
	const int thread_stride = (height - 1) / hw_concur + 1;
	int i = 0, start = 0;
	for (; i < std::min(height, hw_concur); i++, start += thread_stride)
	{
		fut[i] = std::async(std::launch::async, _IM_GetRoughSkinRegion, Src, width, start, thread_stride, stride, Skin);
	}
	for (int j = 0; j < i; ++j)
		fut[j].wait();
}

void IM_GrayToRGB(unsigned char *Gray, unsigned char *RGB, int Width, int Height, int Stride)
{
	for (int Y = 0; Y < Height; Y++)
	{
		unsigned char *LinePS = Gray + Y * Width;					//	源图的第Y行像素的首地址
		unsigned char *LinePD = RGB + Y * Stride;					//	Skin区域的第Y行像素的首地址	
		int X = 0;
		for (int X = 0; X < Width; X++)
		{
			LinePD[0] = LinePD[1] = LinePD[2] = LinePS[X];
			LinePD += 3;
		}
	}
}

int main() {
	Mat src = imread("F:\\face.jpg");
	int Height = src.rows;
	int Width = src.cols;
	unsigned char *Src = src.data;
	unsigned char *Skin = new unsigned char[Height * Width];
	unsigned char *Dest = new unsigned char[Height * Width * 3];
	int Stride = Width * 3;
	int Radius = 11;
	int Adjustment = 50;
	int64 st = cvGetTickCount();
	for (int i = 0; i <1000; i++) {
		IM_GetRoughSkinRegion_SSE2(Src, Skin, Width, Height, Stride);
		//IM_GrayToRGB(Skin, Dest, Width, Height, Stride);
	}
	double duration = (cv::getTickCount() - st) / cv::getTickFrequency();
	printf("%.5f\n", duration);
	IM_GetRoughSkinRegion_SSE2(Src, Skin, Width, Height, Stride);
	IM_GrayToRGB(Skin, Dest, Width, Height, Stride);
	Mat dst(Height, Width, CV_8UC3, Dest);
	imshow("origin", src);
	imshow("result", dst);
	imwrite("F:\\res.jpg", dst);
	waitKey(0);
}