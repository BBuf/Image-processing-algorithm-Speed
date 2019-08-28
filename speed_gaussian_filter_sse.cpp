#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

void CalcGaussCof(float Radius, float &B0, float &B1, float &B2, float &B3)
{
	float Q, B;
	if (Radius >= 2.5)
		Q = (double)(0.98711 * Radius - 0.96330);                            //    对应论文公式11b
	else if ((Radius >= 0.5) && (Radius < 2.5))
		Q = (double)(3.97156 - 4.14554 * sqrt(1 - 0.26891 * Radius));
	else
		Q = (double)0.1147705018520355224609375;

	B = 1.57825 + 2.44413 * Q + 1.4281 * Q * Q + 0.422205 * Q * Q * Q;        //    对应论文公式8c
	B1 = 2.44413 * Q + 2.85619 * Q * Q + 1.26661 * Q * Q * Q;
	B2 = -1.4281 * Q * Q - 1.26661 * Q * Q * Q;
	B3 = 0.422205 * Q * Q * Q;

	B0 = 1.0 - (B1 + B2 + B3) / B;
	B1 = B1 / B;
	B2 = B2 / B;
	B3 = B3 / B;
}

void ConvertBGR8U2BGRAF(unsigned char *Src, float *Dest, int Width, int Height, int Stride)
{
	//#pragma omp parallel for
	for (int Y = 0; Y < Height; Y++)
	{
		unsigned char *LinePS = Src + Y * Stride;
		float *LinePD = Dest + Y * Width * 3;
		for (int X = 0; X < Width; X++, LinePS += 3, LinePD += 3)
		{
			LinePD[0] = LinePS[0];    LinePD[1] = LinePS[1];    LinePD[2] = LinePS[2];
		}
	}
}

void ConvertBGR8U2BGRAF_SSE(unsigned char *Src, float *Dest, int Width, int Height, int Stride) {
	const int BlockSize = 4;
	int Block = (Width - 2) / BlockSize;
	__m128i Mask = _mm_setr_epi8(0, 1, 2, -1, 3, 4, 5, -1, 6, 7, 8, -1, 9, 10, 11, -1);
	__m128i Zero = _mm_setzero_si128();
	for (int Y = 0; Y < Height; Y++) {
		unsigned char *LinePS = Src + Y * Stride;
		float *LinePD = Dest + Y * Width * 4;
		int X = 0;
		for (; X < Block * BlockSize; X += BlockSize, LinePS += BlockSize * 3, LinePD += BlockSize * 4) {
			__m128i SrcV = _mm_shuffle_epi8(_mm_loadu_si128((const __m128i*)LinePS), Mask);
			__m128i Src16L = _mm_unpacklo_epi8(SrcV, Zero);
			__m128i Src16H = _mm_unpackhi_epi8(SrcV, Zero);
			_mm_store_ps(LinePD + 0, _mm_cvtepi32_ps(_mm_unpacklo_epi16(Src16L, Zero)));
			_mm_store_ps(LinePD + 4, _mm_cvtepi32_ps(_mm_unpackhi_epi16(Src16L, Zero)));
			_mm_store_ps(LinePD + 8, _mm_cvtepi32_ps(_mm_unpacklo_epi16(Src16H, Zero)));
			_mm_store_ps(LinePD + 12, _mm_cvtepi32_ps(_mm_unpackhi_epi16(Src16H, Zero)));
		}
		for (; X < Width; X++, LinePS += 3, LinePD += 4) {
			LinePD[0] = LinePS[0];    LinePD[1] = LinePS[1];    LinePD[2] = LinePS[2];    LinePD[3] = 0;
		}
	}
}

void GaussBlurFromLeftToRight(float *Data, int Width, int Height, float B0, float B1, float B2, float B3)
{
	//#pragma omp parallel for
	for (int Y = 0; Y < Height; Y++)
	{
		float *LinePD = Data + Y * Width * 3;
		//w[n-1], w[n-2], w[n-3]
		float BS1 = LinePD[0], BS2 = LinePD[0], BS3 = LinePD[0]; //边缘处使用重复像素的方案
		float GS1 = LinePD[1], GS2 = LinePD[1], GS3 = LinePD[1];
		float RS1 = LinePD[2], RS2 = LinePD[2], RS3 = LinePD[2];
		for (int X = 0; X < Width; X++, LinePD += 3)
		{
			LinePD[0] = LinePD[0] * B0 + BS1 * B1 + BS2 * B2 + BS3 * B3;
			LinePD[1] = LinePD[1] * B0 + GS1 * B1 + GS2 * B2 + GS3 * B3;         // 进行顺向迭代
			LinePD[2] = LinePD[2] * B0 + RS1 * B1 + RS2 * B2 + RS3 * B3;
			BS3 = BS2, BS2 = BS1, BS1 = LinePD[0];
			GS3 = GS2, GS2 = GS1, GS1 = LinePD[1];
			RS3 = RS2, RS2 = RS1, RS1 = LinePD[2];
		}
	}
}

void GaussBlurFromLeftToRight_SSE(float *Data, int Width, int Height, float B0, float B1, float B2, float B3) {
	const __m128 CofB0 = _mm_set_ps(0, B0, B0, B0);
	const __m128 CofB1 = _mm_set_ps(0, B1, B1, B1);
	const __m128 CofB2 = _mm_set_ps(0, B2, B2, B2);
	const __m128 CofB3 = _mm_set_ps(0, B3, B3, B3);
	for (int Y = 0; Y < Height; Y++) {
		float *LinePD = Data + Y * Width * 4;
		__m128 V1 = _mm_set_ps(LinePD[3], LinePD[2], LinePD[1], LinePD[0]);
		__m128 V2 = V1, V3 = V1;
		for (int X = 0; X < Width; X++, LinePD += 4) {
			__m128 V0 = _mm_load_ps(LinePD);
			__m128 V01 = _mm_add_ps(_mm_mul_ps(CofB0, V0), _mm_mul_ps(CofB1, V1));
			__m128 V23 = _mm_add_ps(_mm_mul_ps(CofB2, V2), _mm_mul_ps(CofB3, V3));
			__m128 V = _mm_add_ps(V01, V23);
			V3 = V2; V2 = V1; V1 = V;
			_mm_store_ps(LinePD, V);
		}
	}
}

void GaussBlurFromRightToLeft(float *Data, int Width, int Height, float B0, float B1, float B2, float B3) {
	for (int Y = 0; Y < Height; Y++) {
		//w[n+1], w[n+2], w[n+3]
		float *LinePD = Data + Y * Width * 3 + (Width * 3);
		float BS1 = LinePD[0], BS2 = LinePD[0], BS3 = LinePD[0]; //边缘处使用重复像素的方案
		float GS1 = LinePD[1], GS2 = LinePD[1], GS3 = LinePD[1];
		float RS1 = LinePD[2], RS2 = LinePD[2], RS3 = LinePD[2];
		for (int X = Width - 1; X >= 0; X--, LinePD -= 3)
		{
			LinePD[0] = LinePD[0] * B0 + BS3 * B1 + BS2 * B2 + BS1 * B3;
			LinePD[1] = LinePD[1] * B0 + GS3 * B1 + GS2 * B2 + GS1 * B3;         // 进行反向迭代
			LinePD[2] = LinePD[2] * B0 + RS3 * B1 + RS2 * B2 + RS1 * B3;
			BS1 = BS2, BS2 = BS3, BS3 = LinePD[0];
			GS1 = GS2, GS2 = GS3, GS3 = LinePD[1];
			RS1 = RS2, RS2 = RS3, RS3 = LinePD[2];
		}
	}
}

void GaussBlurFromRightToLeft_SSE(float *Data, int Width, int Height, float B0, float B1, float B2, float B3) {
	const __m128 CofB0 = _mm_set_ps(0, B0, B0, B0);
	const __m128 CofB1 = _mm_set_ps(0, B1, B1, B1);
	const __m128 CofB2 = _mm_set_ps(0, B2, B2, B2);
	const __m128 CofB3 = _mm_set_ps(0, B3, B3, B3);
	for (int Y = 0; Y < Height; Y++) {
		float *LinePD = Data + Y * Width * 4 + (Width * 4);
		__m128 V1 = _mm_set_ps(LinePD[3], LinePD[2], LinePD[1], LinePD[0]);
		__m128 V2 = V1, V3 = V1;
		for (int X = Width - 1; X >= 0; X--, LinePD -= 4) {
			__m128 V0 = _mm_load_ps(LinePD);
			__m128 V03 = _mm_add_ps(_mm_mul_ps(CofB0, V0), _mm_mul_ps(CofB1, V3));
			__m128 V12 = _mm_add_ps(_mm_mul_ps(CofB2, V2), _mm_mul_ps(CofB3, V1));
			__m128 V = _mm_add_ps(V03, V12);
			V1 = V2; V2 = V3; V3 = V;
			_mm_store_ps(LinePD, V);
		}
	}
}


//w[n] w[n-1], w[n-2], w[n-3]
void GaussBlurFromTopToBottom(float *Data, int Width, int Height, float B0, float B1, float B2, float B3)
{
	for (int Y = 0; Y < Height; Y++)
	{
		float *LinePD3 = Data + (Y + 0) * Width * 3;
		float *LinePD2 = Data + (Y + 1) * Width * 3;
		float *LinePD1 = Data + (Y + 2) * Width * 3;
		float *LinePD0 = Data + (Y + 3) * Width * 3;
		for (int X = 0; X < Width; X++, LinePD0 += 3, LinePD1 += 3, LinePD2 += 3, LinePD3 += 3)
		{
			LinePD0[0] = LinePD0[0] * B0 + LinePD1[0] * B1 + LinePD2[0] * B2 + LinePD3[0] * B3;
			LinePD0[1] = LinePD0[1] * B0 + LinePD1[1] * B1 + LinePD2[1] * B2 + LinePD3[1] * B3;
			LinePD0[2] = LinePD0[2] * B0 + LinePD1[2] * B1 + LinePD2[2] * B2 + LinePD3[2] * B3;
		}
	}
}

void GaussBlurFromTopToBottom_SSE(float *Data, int Width, int Height, float B0, float B1, float B2, float B3){
	const  __m128 CofB0 = _mm_set_ps(0, B0, B0, B0);
	const  __m128 CofB1 = _mm_set_ps(0, B1, B1, B1);
	const  __m128 CofB2 = _mm_set_ps(0, B2, B2, B2);
	const  __m128 CofB3 = _mm_set_ps(0, B3, B3, B3);
	for (int Y = 0; Y < Height; Y++)
	{
		float *LinePS3 = Data + (Y + 0) * Width * 4;
		float *LinePS2 = Data + (Y + 1) * Width * 4;
		float *LinePS1 = Data + (Y + 2) * Width * 4;
		float *LinePS0 = Data + (Y + 3) * Width * 4;
		for (int X = 0; X < Width * 4; X += 4)
		{
			__m128 V3 = _mm_load_ps(LinePS3 + X);
			__m128 V2 = _mm_load_ps(LinePS2 + X);
			__m128 V1 = _mm_load_ps(LinePS1 + X);
			__m128 V0 = _mm_load_ps(LinePS0 + X);
			__m128 V01 = _mm_add_ps(_mm_mul_ps(CofB0, V0), _mm_mul_ps(CofB1, V1));
			__m128 V23 = _mm_add_ps(_mm_mul_ps(CofB2, V2), _mm_mul_ps(CofB3, V3));
			_mm_store_ps(LinePS0 + X, _mm_add_ps(V01, V23));
		}
	}
}
//w[n] w[n+1], w[n+2], w[n+3]
void GaussBlurFromBottomToTop(float *Data, int Width, int Height, float B0, float B1, float B2, float B3) {
	for (int Y = Height - 1; Y >= 0; Y--) {
		float *LinePD3 = Data + (Y + 3) * Width * 3;
		float *LinePD2 = Data + (Y + 2) * Width * 3;
		float *LinePD1 = Data + (Y + 1) * Width * 3;
		float *LinePD0 = Data + (Y + 0) * Width * 3;
		for (int X = 0; X < Width; X++, LinePD0 += 3, LinePD1 += 3, LinePD2 += 3, LinePD3 += 3) {
			LinePD0[0] = LinePD0[0] * B0 + LinePD1[0] * B1 + LinePD2[0] * B2 + LinePD3[0] * B3;
			LinePD0[1] = LinePD0[1] * B0 + LinePD1[1] * B1 + LinePD2[1] * B2 + LinePD3[1] * B3;
			LinePD0[2] = LinePD0[2] * B0 + LinePD1[2] * B1 + LinePD2[2] * B2 + LinePD3[2] * B3;
		}
	}
}

void GaussBlurFromBottomToTop_SSE(float *Data, int Width, int Height, float B0, float B1, float B2, float B3) {
	const  __m128 CofB0 = _mm_set_ps(0, B0, B0, B0);
	const  __m128 CofB1 = _mm_set_ps(0, B1, B1, B1);
	const  __m128 CofB2 = _mm_set_ps(0, B2, B2, B2);
	const  __m128 CofB3 = _mm_set_ps(0, B3, B3, B3);
	for (int Y = Height - 1; Y >= 0; Y--) {
		float *LinePS3 = Data + (Y + 3) * Width * 4;
		float *LinePS2 = Data + (Y + 2) * Width * 4;
		float *LinePS1 = Data + (Y + 1) * Width * 4;
		float *LinePS0 = Data + (Y + 0) * Width * 4;
		for (int X = 0; X < Width * 4; X += 4) {
			__m128 V3 = _mm_load_ps(LinePS3 + X);
			__m128 V2 = _mm_load_ps(LinePS2 + X);
			__m128 V1 = _mm_load_ps(LinePS1 + X);
			__m128 V0 = _mm_load_ps(LinePS0 + X);
			__m128 V01 = _mm_add_ps(_mm_mul_ps(CofB0, V0), _mm_mul_ps(CofB1, V1));
			__m128 V23 = _mm_add_ps(_mm_mul_ps(CofB2, V2), _mm_mul_ps(CofB3, V3));
			_mm_store_ps(LinePS0 + X, _mm_add_ps(V01, V23));
		}
	}
}

void ConvertBGRAF2BGR8U(float *Src, unsigned char *Dest, int Width, int Height, int Stride)
{
	//#pragma omp parallel for
	for (int Y = 0; Y < Height; Y++)
	{
		float *LinePS = Src + Y * Width * 3;
		unsigned char *LinePD = Dest + Y * Stride;
		for (int X = 0; X < Width; X++, LinePS += 3, LinePD += 3)
		{
			LinePD[0] = LinePS[0];    LinePD[1] = LinePS[1];    LinePD[2] = LinePS[2];
		}
	}
}


void ConvertBGRAF2BGR8U_SSE(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride) {
	const int BlockSize = 4;
	int Block = (Width - 2) / BlockSize;
	//__m128i Mask = _mm_setr_epi8(0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, 3, 7, 11, 15);
	__m128i MaskB = _mm_setr_epi8(0, 4, 8, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
	__m128i MaskG = _mm_setr_epi8(1, 5, 9, 13, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
	__m128i MaskR = _mm_setr_epi8(2, 6, 10, 14, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
	__m128i Zero = _mm_setzero_si128();
	for (int Y = 0; Y < Height; Y++) {
		unsigned char *LinePS = Src + Y * Width * 4;
		unsigned char *LinePD = Dest + Y * Stride;
		int X = 0;
		for (; X < Block * BlockSize; X += BlockSize, LinePS += BlockSize * 4, LinePD += BlockSize * 3) {
			__m128i SrcV = _mm_loadu_si128((const __m128i*)LinePS);
			__m128i B = _mm_shuffle_epi8(SrcV, MaskB);
			__m128i G = _mm_shuffle_epi8(SrcV, MaskG);
			__m128i R = _mm_shuffle_epi8(SrcV, MaskR);
			__m128i Ans1 = Zero, Ans2 = Zero, Ans3 = Zero;
			Ans1 = _mm_or_si128(Ans1, _mm_shuffle_epi8(B, _mm_setr_epi8(0, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1))); 
			Ans1 = _mm_or_si128(Ans1, _mm_shuffle_epi8(G, _mm_setr_epi8(-1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)));
			Ans1 = _mm_or_si128(Ans1, _mm_shuffle_epi8(R, _mm_setr_epi8(-1, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)));

			Ans2 = _mm_or_si128(Ans2, _mm_shuffle_epi8(B, _mm_setr_epi8(-1, -1, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)));
			Ans2 = _mm_or_si128(Ans2, _mm_shuffle_epi8(G, _mm_setr_epi8(1, -1, -1, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)));
			Ans2 = _mm_or_si128(Ans2, _mm_shuffle_epi8(R, _mm_setr_epi8(-1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)));

			Ans3 = _mm_or_si128(Ans3, _mm_shuffle_epi8(B, _mm_setr_epi8(-1, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)));
			Ans3 = _mm_or_si128(Ans3, _mm_shuffle_epi8(G, _mm_setr_epi8(-1, -1, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)));
			Ans3 = _mm_or_si128(Ans3, _mm_shuffle_epi8(R, _mm_setr_epi8(2, -1, -1, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1)));

			_mm_storeu_si128((__m128i*)(LinePD + 0), Ans1);
			_mm_storeu_si128((__m128i*)(LinePD + 4), Ans2);
			_mm_storeu_si128((__m128i*)(LinePD + 8), Ans3);
		}
		for (; X < Width; X++, LinePS += 4, LinePD += 3) {
			LinePD[0] = LinePS[0]; LinePD[1] = LinePS[1]; LinePD[2] = LinePS[2];
		}
	}
}

void GaussBlur(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride, float Radius)
{
	float B0, B1, B2, B3;
	float *Buffer = (float *)malloc(Width * (Height + 6) * sizeof(float) * 3);
	CalcGaussCof(Radius, B0, B1, B2, B3);
	ConvertBGR8U2BGRAF(Src, Buffer + 3 * Width * 3, Width, Height, Stride);
	GaussBlurFromLeftToRight(Buffer + 3 * Width * 3, Width, Height, B0, B1, B2, B3);
	GaussBlurFromRightToLeft(Buffer + 3 * Width * 3, Width, Height, B0, B1, B2, B3);        //    如果启用多线程，建议把这个函数写到GaussBlurFromLeftToRight的for X循环里，因为这样就可以减少线程并发时的阻力

	memcpy(Buffer + 0 * Width * 3, Buffer + 3 * Width * 3, Width * 3 * sizeof(float));
	memcpy(Buffer + 1 * Width * 3, Buffer + 3 * Width * 3, Width * 3 * sizeof(float));
	memcpy(Buffer + 2 * Width * 3, Buffer + 3 * Width * 3, Width * 3 * sizeof(float));

	GaussBlurFromTopToBottom(Buffer, Width, Height, B0, B1, B2, B3);

	memcpy(Buffer + (Height + 3) * Width * 3, Buffer + (Height + 2) * Width * 3, Width * 3 * sizeof(float));
	memcpy(Buffer + (Height + 4) * Width * 3, Buffer + (Height + 2) * Width * 3, Width * 3 * sizeof(float));
	memcpy(Buffer + (Height + 5) * Width * 3, Buffer + (Height + 2) * Width * 3, Width * 3 * sizeof(float));

	GaussBlurFromBottomToTop(Buffer, Width, Height, B0, B1, B2, B3);

	ConvertBGRAF2BGR8U(Buffer + 3 * Width * 3, Dest, Width, Height, Stride);

	free(Buffer);
}

void GaussBlur_SSE(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride, float Radius)
{
	float B0, B1, B2, B3;
	float *Buffer = (float *)_mm_malloc(Width * (Height + 6) * sizeof(float) * 4, 16);
	CalcGaussCof(Radius, B0, B1, B2, B3);
	ConvertBGR8U2BGRAF_SSE(Src, Buffer + 3 * Width * 4, Width, Height, Stride);
	GaussBlurFromLeftToRight_SSE(Buffer + 3 * Width * 4, Width, Height, B0, B1, B2, B3);        //    在SSE版本中，这两个函数占用的时间比下面两个要多,不过C语言版本也是一样的
	GaussBlurFromRightToLeft_SSE(Buffer + 3 * Width * 4, Width, Height, B0, B1, B2, B3);        //    如果启用多线程，建议把这个函数写到GaussBlurFromLeftToRight的for X循环里，因为这样就可以减少线程并发时的阻力

	memcpy(Buffer + 0 * Width * 4, Buffer + 3 * Width * 4, Width * 4 * sizeof(float));
	memcpy(Buffer + 1 * Width * 4, Buffer + 3 * Width * 4, Width * 4 * sizeof(float));
	memcpy(Buffer + 2 * Width * 4, Buffer + 3 * Width * 4, Width * 4 * sizeof(float));

	GaussBlurFromTopToBottom_SSE(Buffer, Width, Height, B0, B1, B2, B3);

	memcpy(Buffer + (Height + 3) * Width * 4, Buffer + (Height + 2) * Width * 4, Width * 4 * sizeof(float));
	memcpy(Buffer + (Height + 4) * Width * 4, Buffer + (Height + 2) * Width * 4, Width * 4 * sizeof(float));
	memcpy(Buffer + (Height + 5) * Width * 4, Buffer + (Height + 2) * Width * 4, Width * 4 * sizeof(float));

	GaussBlurFromBottomToTop_SSE(Buffer, Width, Height, B0, B1, B2, B3);

	ConvertBGRAF2BGR8U_SSE(Buffer + 3 * Width * 4, Dest, Width, Height, Stride);

	_mm_free(Buffer);
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
	for (int i = 0; i < 20; i++) {
		GaussBlur_SSE(Src, Dest, Width, Height, Stride, Radius);
	}
	double duration = (cv::getTickCount() - st) / cv::getTickFrequency() *  50;
	printf("%.5f\n", duration);
	GaussBlur_SSE(Src, Dest, Width, Height, Stride, Radius);
	Mat dst(Height, Width, CV_8UC3, Dest);
	imshow("origin", src);
	imshow("result", dst);
	imwrite("F:\\res.jpg", dst);
	waitKey(0);
}