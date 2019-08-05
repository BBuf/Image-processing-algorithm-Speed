#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "../../OpencvTest/OpencvTest/Core.h"
#include "../../OpencvTest/OpencvTest/MaxFilter.h"
#include "../../OpencvTest/OpencvTest/Utility.h"
#include "../../OpencvTest/OpencvTest/BoxFilter.h"
using namespace std;
using namespace cv;
#define __SSSE3__ 1

void BoxBlur_SSE(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride, int Channel, int Radius) {
	TMatrix a, b;
	TMatrix *p1 = &a, *p2 = &b;
	TMatrix **p3 = &p1, **p4 = &p2;
	IS_CreateMatrix(Width, Height, IS_DEPTH_8U, Channel, p3);
	IS_CreateMatrix(Width, Height, IS_DEPTH_8U, Channel, p4);
	(p1)->Data = Src;
	(p2)->Data = Dest;
	BoxBlur_SSE(p1, p2, Radius, EdgeMode::Smear);
}

int IM_Sign(int X) {
	return (X >> 31) | (unsigned(-X)) >> 31;
}

inline unsigned char IM_ClampToByte(int Value)
{
	if (Value < 0)
		return 0;
	else if (Value > 255)
		return 255;
	else
		return (unsigned char)Value;
	//return ((Value | ((signed int)(255 - Value) >> 31)) & ~((signed int)Value >> 31));
}


inline __m128i _mm_sgn_epi16(__m128i v) {
#ifdef __SSSE3__
	v = _mm_sign_epi16(_mm_set1_epi16(1), v); // use PSIGNW on SSSE3 and later
#else
	v = _mm_min_epi16(v, _mm_set1_epi16(1));  // use PMINSW/PMAXSW on SSE2/SSE3.
	v = _mm_max_epi16(v, _mm_set1_epi16(-1));
	//_mm_set1_epi16(1) = _mm_srli_epi16(_mm_cmpeq_epi16(v, v), 15);
	//_mm_set1_epi16(-1) = _mm_cmpeq_epi16(v, v);

#endif
	return v;
}

void MultiScaleSharpen(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride, int Radius) {
	int Channel = Stride / Width;
	unsigned char *B1 = (unsigned char *)malloc(Height * Stride * sizeof(unsigned char));
	unsigned char *B2 = (unsigned char *)malloc(Height * Stride * sizeof(unsigned char));
	unsigned char *B3 = (unsigned char *)malloc(Height * Stride * sizeof(unsigned char));
	BoxBlur_SSE(Src, B1, Width, Height, Channel, Stride, Radius);
	BoxBlur_SSE(Src, B2, Width, Height, Channel, Stride, Radius * 2);
	BoxBlur_SSE(Src, B3, Width, Height, Channel, Stride, Radius * 4);
	for (int Y = 0; Y < Height * Stride; Y++) {
		int DiffB1 = Src[Y] - B1[Y];
		int DiffB2 = B1[Y] - B2[Y];
		int DiffB3 = B2[Y] - B3[Y];
		Dest[Y] = IM_ClampToByte(((4 - 2 * IM_Sign(DiffB1)) * DiffB1 + 2 * DiffB2 + DiffB3) / 4 + Src[Y]);
	}
}

void MultiScaleSharpen_SSE(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride, int Radius) {
	int Channel = Stride / Width;
	unsigned char *B1 = (unsigned char *)malloc(Height * Stride * sizeof(unsigned char));
	unsigned char *B2 = (unsigned char *)malloc(Height * Stride * sizeof(unsigned char));
	unsigned char *B3 = (unsigned char *)malloc(Height * Stride * sizeof(unsigned char));
	BoxBlur_SSE(Src, B1, Width, Height, Channel, Stride, Radius);
	BoxBlur_SSE(Src, B2, Width, Height, Channel, Stride, Radius * 2);
	BoxBlur_SSE(Src, B3, Width, Height, Channel, Stride, Radius * 4);
	int BlockSize = 8, Block = (Height * Stride) / BlockSize;
	__m128i Zero = _mm_setzero_si128();
	__m128i Four = _mm_set1_epi16(4);
	for (int Y = 0; Y < Block * BlockSize; Y += BlockSize) {
		__m128i SrcV = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i *)(Src + Y)), Zero);
		__m128i SrcB1 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i *)(B1 + Y)), Zero);
		__m128i SrcB2 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i *)(B2 + Y)), Zero);
		__m128i SrcB3 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i *)(B3 + Y)), Zero);
		__m128i DiffB1 = _mm_sub_epi16(SrcV, SrcB1);
		__m128i DiffB2 = _mm_sub_epi16(SrcB1, SrcB2);
		__m128i DiffB3 = _mm_sub_epi16(SrcB2, SrcB3);
		//__m128i Offset = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_mullo_epi16(_mm_sub_epi16(Four, _mm_slli_epi16(_mm_sgn_epi16(DiffB1), 1)), DiffB1), _mm_slli_epi16(DiffB2, 1)), DiffB3), 2);
		__m128i Offset = _mm_add_epi16(_mm_srai_epi16(_mm_sub_epi16(_mm_slli_epi16(_mm_sub_epi16(SrcB1, _mm_sign_epi16(DiffB1, DiffB1)), 1), _mm_add_epi16(SrcB2, SrcB3)), 2), DiffB1);
		_mm_storel_epi64((__m128i *)(Dest + Y), _mm_packus_epi16(_mm_add_epi16(SrcV, Offset), Zero));
	}
	for (int Y = Block * BlockSize; Y < Height * Stride; Y++) {
		int DiffB1 = Src[Y] - B1[Y];
		int DiffB2 = B1[Y] - B2[Y];
		int DiffB3 = B2[Y] - B3[Y];
		Dest[Y] = IM_ClampToByte(((4 - 2 * IM_Sign(DiffB1)) * DiffB1 + 2 * DiffB2 + DiffB3) / 4 + Src[Y]);
	}
}

int main() {
	Mat src = imread("F:\\car.jpg");
	int Height = src.rows;
	int Width = src.cols;
	unsigned char *Src = src.data;
	unsigned char *Dest = new unsigned char[Height * Width * 3];
	int Stride = Width * 3;
	int Radius = 5;
	int64 st = cvGetTickCount();
	for (int i = 0; i <10; i++) {
		//Mat temp = MaxFilter(src, Radius);
		MultiScaleSharpen_SSE(Src, Dest, Width, Height, Stride, Radius);
	}
	double duration = (cv::getTickCount() - st) / cv::getTickFrequency() * 100;
	printf("%.5f\n", duration);
	MultiScaleSharpen(Src, Dest, Width, Height, Stride, Radius);
	Mat dst(Height, Width, CV_8UC3, Dest);
	imshow("origin", src);
	imshow("result", dst);
	imwrite("F:\\res.jpg", dst);
	waitKey(0);
	return 0;
}