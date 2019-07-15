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
	#pragma parallel for num_threads(4)
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

//Adjustment如果为正值，会增加饱和度
//Adjustment如果为负值，会降低饱和度
void VibranceAlgorithm_FLOAT(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride, int Adjustment) {
	float VibranceAdjustment = -0.01 * Adjustment;
	for (int Y = 0; Y < Height; Y++) {
		unsigned char *LinePS = Src + Y * Stride;
		unsigned char *LinePD = Dest + Y * Stride;
		for (int X = 0; X < Width; X++) {
			int Blue = LinePS[0], Green = LinePS[1], Red = LinePS[2];
			int Avg = (Blue + Green + Green + Red) >> 2;
			int Max = max(max(Blue, Green), Red);
			float AmtVal = (abs(Max - Avg) / 127.0f) * VibranceAdjustment;
			if (Blue != Max) Blue += (Max - Blue) * AmtVal;
			if (Green != Max) Green += (Max - Green) * AmtVal;
			if (Red != Max) Red += (Max - Red) * AmtVal;
			if (Red < 0) Red = 0;
			else if (Red > 255) Red = 255;
			if (Green < 0) Green = 0;
			else if (Green > 255) Green = 255;
			if (Blue < 0) Blue = 0;
			else if (Blue > 255) Blue = 255;
			LinePD[0] = Blue;
			LinePD[1] = Green;
			LinePD[2] = Red;
			LinePS += 3;
			LinePD += 3;
		}
	}
}

void VibranceAlgorithm_INT(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride, int Adjustment) {
	int VibranceAdjustment = -1.28 * Adjustment;
	for (int Y = 0; Y < Height; Y++) {
		unsigned char *LinePS = Src + Y * Stride;
		unsigned char *LinePD = Dest + Y * Stride;
		for (int X = 0; X < Width; X++) {
			int Blue, Green, Red, Max;
			Blue = LinePS[0], Green = LinePS[1], Red = LinePS[2];
			int Avg = (Blue + Green + Green + Red) >> 2;
			if (Blue > Green)
				Max = Blue;
			else
				Max = Green;
			if (Red > Max)
				Max = Red;
			int AmtVal = (Max - Avg) * VibranceAdjustment;
			if (Blue != Max) Blue += (((Max - Blue) * AmtVal) >> 14);
			if (Green != Max) Green += (((Max - Green) * AmtVal) >> 14);
			if (Red != Max) Red += (((Max - Red) * AmtVal) >> 14);
			if (Red < 0) Red = 0;
			else if (Red > 255) Red = 255;
			if (Green < 0) Green = 0;
			else if (Green > 255) Green = 255;
			if (Blue < 0) Blue = 0;
			else if (Blue > 255) Blue = 255;
			LinePD[0] = Blue;
			LinePD[1] = Green;
			LinePD[2] = Red;
			LinePS += 3;
			LinePD += 3;
		}
	}
}

void VibranceAlgorithm_SSE(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride, int Adjustment) {
	int VibranceAdjustment = (int)(-1.28 * Adjustment);
	__m128i Adjustment128 = _mm_setr_epi16(VibranceAdjustment, VibranceAdjustment, VibranceAdjustment, VibranceAdjustment,
		VibranceAdjustment, VibranceAdjustment, VibranceAdjustment, VibranceAdjustment);
	int X;
	for (int Y = 0; Y < Height; Y++) {
		unsigned char *LinePS = Src + Y * Stride;
		unsigned char *LinePD = Dest + Y * Stride;
		X = 0;
		__m128i Src1, Src2, Src3, Dest1, Dest2, Dest3, Blue8, Green8, Red8, Max8;
		__m128i BL16, BH16, GL16, GH16, RL16, RH16, MaxL16, MaxH16, AvgL16, AvgH16, AmtVal;
		__m128i Zero = _mm_setzero_si128();
		for (; X < Width - 16; X += 16, LinePS += 48, LinePD += 48) {
			Src1 = _mm_loadu_si128((__m128i *)(LinePS + 0)); //B1,G1,R1,B2,G2,R2,B3,G3,R3,B4,G4,R4,B5,G5,R5,B6
			Src2 = _mm_loadu_si128((__m128i *)(LinePS + 16));//G6,R6,B7,G7,R7,B8,G8,R8,B9,G9,R9,B10,G10,R10,B11,G11
			Src3 = _mm_loadu_si128((__m128i *)(LinePS + 32));//R11,B12,G12,R12,B13,G13,R13,B14,G14,R14,B15,G15,R15,B16,G16,R16

			Blue8 = _mm_shuffle_epi8(Src1, _mm_setr_epi8(0, 3, 6, 9, 12, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1));
			Blue8 = _mm_or_si128(Blue8, _mm_shuffle_epi8(Src2, _mm_setr_epi8(-1, -1, -1, -1, -1, -1, 2, 5, 8, 11, 14, -1, -1, -1, -1, -1)));
			Blue8 = _mm_or_si128(Blue8, _mm_shuffle_epi8(Src3, _mm_setr_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, 4, 7, 10, 13)));

			Green8 = _mm_shuffle_epi8(Src1, _mm_setr_epi8(1, 4, 7, 10, 13, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1));
			Green8 = _mm_or_si128(Green8, _mm_shuffle_epi8(Src2, _mm_setr_epi8(-1, -1, -1, -1, -1, 0, 3, 6, 9, 12, 15, -1, -1, -1, -1, -1)));
			Green8 = _mm_or_si128(Green8, _mm_shuffle_epi8(Src3, _mm_setr_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 2, 5, 8, 11, 14)));

			Red8 = _mm_shuffle_epi8(Src1, _mm_setr_epi8(2, 5, 8, 11, 14, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1));
			Red8 = _mm_or_si128(Red8, _mm_shuffle_epi8(Src2, _mm_setr_epi8(-1, -1, -1, -1, -1, 1, 4, 7, 10, 13, -1, -1, -1, -1, -1, -1)));
			Red8 = _mm_or_si128(Red8, _mm_shuffle_epi8(Src3, _mm_setr_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 3, 6, 9, 12, 15)));

			Max8 = _mm_max_epu8(_mm_max_epu8(Blue8, Green8), Red8);
			
			BL16 = _mm_unpacklo_epi8(Blue8, Zero);
			BH16 = _mm_unpackhi_epi8(Blue8, Zero);
			GL16 = _mm_unpacklo_epi8(Green8, Zero);
			GH16 = _mm_unpackhi_epi8(Green8, Zero);
			RL16 = _mm_unpacklo_epi8(Red8, Zero);
			RH16 = _mm_unpackhi_epi8(Red8, Zero);
			MaxL16 = _mm_unpacklo_epi8(Max8, Zero);
			MaxH16 = _mm_unpackhi_epi8(Max8, Zero);

			AvgL16 = _mm_srli_epi16(_mm_add_epi16(_mm_add_epi16(BL16, RL16), _mm_slli_epi16(GL16, 1)), 2);
			AvgH16 = _mm_srli_epi16(_mm_add_epi16(_mm_add_epi16(BH16, RH16), _mm_slli_epi16(GH16, 1)), 2);

			AmtVal = _mm_mullo_epi16(_mm_sub_epi16(MaxL16, AvgL16), Adjustment128);
			BL16 = _mm_adds_epi16(BL16, _mm_mulhi_epi16(_mm_slli_epi16(_mm_sub_epi16(MaxL16, BL16), 2), AmtVal));
			GL16 = _mm_adds_epi16(GL16, _mm_mulhi_epi16(_mm_slli_epi16(_mm_sub_epi16(MaxL16, GL16), 2), AmtVal));
			RL16 = _mm_adds_epi16(RL16, _mm_mulhi_epi16(_mm_slli_epi16(_mm_sub_epi16(MaxL16, RL16), 2), AmtVal));

			AmtVal = _mm_mullo_epi16(_mm_sub_epi16(MaxH16, AvgH16), Adjustment128);
			BH16 = _mm_adds_epi16(BH16, _mm_mulhi_epi16(_mm_slli_epi16(_mm_sub_epi16(MaxH16, BH16), 2), AmtVal));
			GH16 = _mm_adds_epi16(GH16, _mm_mulhi_epi16(_mm_slli_epi16(_mm_sub_epi16(MaxH16, GH16), 2), AmtVal));
			RH16 = _mm_adds_epi16(RH16, _mm_mulhi_epi16(_mm_slli_epi16(_mm_sub_epi16(MaxH16, RH16), 2), AmtVal));
			
			Blue8 = _mm_packus_epi16(BL16, BH16);
			Green8 = _mm_packus_epi16(GL16, GH16);
			Red8 = _mm_packus_epi16(RL16, RH16);

			Dest1 = _mm_shuffle_epi8(Blue8, _mm_setr_epi8(0, -1, -1, 1, -1, -1, 2, -1, -1, 3, -1, -1, 4, -1, -1, 5));
			Dest1 = _mm_or_si128(Dest1, _mm_shuffle_epi8(Green8, _mm_setr_epi8(-1, 0, -1, -1, 1, -1, -1, 2, -1, -1, 3, -1, -1, 4, -1, -1)));
			Dest1 = _mm_or_si128(Dest1, _mm_shuffle_epi8(Red8, _mm_setr_epi8(-1, -1, 0, -1, -1, 1, -1, -1, 2, -1, -1, 3, -1, -1, 4, -1)));

			Dest2 = _mm_shuffle_epi8(Blue8, _mm_setr_epi8(-1, -1, 6, -1, -1, 7, -1, -1, 8, -1, -1, 9, -1, -1, 10, -1));
			Dest2 = _mm_or_si128(Dest2, _mm_shuffle_epi8(Green8, _mm_setr_epi8(5, -1, -1, 6, -1, -1, 7, -1, -1, 8, -1, -1, 9, -1, -1, 10)));
			Dest2 = _mm_or_si128(Dest2, _mm_shuffle_epi8(Red8, _mm_setr_epi8(-1, 5, -1, -1, 6, -1, -1, 7, -1, -1, 8, -1, -1, 9, -1, -1)));

			Dest3 = _mm_shuffle_epi8(Blue8, _mm_setr_epi8(-1, 11, -1, -1, 12, -1, -1, 13, -1, -1, 14, -1, -1, 15, -1, -1));
			Dest3 = _mm_or_si128(Dest3, _mm_shuffle_epi8(Green8, _mm_setr_epi8(-1, -1, 11, -1, -1, 12, -1, -1, 13, -1, -1, 14, -1, -1, 15, -1)));
			Dest3 = _mm_or_si128(Dest3, _mm_shuffle_epi8(Red8, _mm_setr_epi8(10, -1, -1, 11, -1, -1, 12, -1, -1, 13, -1, -1, 14, -1, -1, 15)));
			
			_mm_store_si128((__m128i *)(LinePD + 0), Dest1);
			_mm_store_si128((__m128i *)(LinePD + 16), Dest2);
			_mm_store_si128((__m128i *)(LinePD + 32), Dest3);
		}
		for (; X < Width; X ++) {
			int Blue, Green, Red, Max;
			Blue = LinePS[0], Green = LinePS[1], Red = LinePS[2];
			int Avg = (Blue + Green + Green + Red) >> 2;
			if (Blue > Green)
				Max = Blue;
			else
				Max = Green;
			if (Red > Max)
				Max = Red;
			int AmtVal = (Max - Avg) * VibranceAdjustment;
			if (Blue != Max) Blue += (((Max - Blue) * AmtVal) >> 14);
			if (Green != Max) Green += (((Max - Green) * AmtVal) >> 14);
			if (Red != Max) Red += (((Max - Red) * AmtVal) >> 14);
			if (Red < 0) Red = 0;
			else if (Red > 255) Red = 255;
			if (Green < 0) Green = 0;
			else if (Green > 255) Green = 255;
			if (Blue < 0) Blue = 0;
			else if (Blue > 255) Blue = 255;
			LinePD[0] = Blue;
			LinePD[1] = Green;
			LinePD[2] = Red;
			LinePS += 3;
			LinePD += 3;
		}
	}
}

int main() {
	Mat src = imread("F:\\1.jpg");
	int Height = src.rows;
	int Width = src.cols;
	unsigned char *Src = src.data;
	unsigned char *Dest = new unsigned char[Height * Width * 3];
	int Stride = Width * 3;
	int Radius = 11;
	int Adjustment = 50;
	/*int64 st = cvGetTickCount();
	for (int i = 0; i <50; i++) {
		VibranceAlgorithm_SSE(Src, Dest, Width, Height, Stride, Adjustment);
	}
	double duration = (cv::getTickCount() - st) / cv::getTickFrequency() * 20;
	printf("%.5f\n", duration);*/
	VibranceAlgorithm_SSE(Src, Dest, Width, Height, Stride, Adjustment);
	Mat dst(Height, Width, CV_8UC3, Dest);
	imshow("origin", src);
	imshow("result", dst);
	imwrite("F:\\res.jpg", dst);
	waitKey(0);
	waitKey(0);
}