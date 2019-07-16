#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

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

void Sobel_FLOAT(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride) {
	int Channel = Stride / Width;
	unsigned char *RowCopy = (unsigned char*)malloc((Width + 2) * 3 * Channel);
	unsigned char *First = RowCopy;
	unsigned char *Second = RowCopy + (Width + 2) * Channel;
	unsigned char *Third = RowCopy + (Width + 2) * 2 * Channel;
	//拷贝第二行数据，边界值填充
	memcpy(Second, Src, Channel);
	memcpy(Second + Channel, Src, Width*Channel);
	memcpy(Second + (Width + 1)*Channel, Src + (Width - 1)*Channel, Channel);
	//第一行和第二行一样
	memcpy(First, Second, (Width + 2) * Channel);
	//拷贝第三行数据，边界值填充
	memcpy(Third, Src + Stride, Channel);
	memcpy(Third + Channel, Src + Stride, Width * Channel);
	memcpy(Third + (Width + 1) * Channel, Src + Stride + (Width - 1) * Channel, Channel);

	for (int Y = 0; Y < Height; Y++) {
		unsigned char *LinePS = Src + Y * Stride;
		unsigned char *LinePD = Dest + Y * Stride;
		if (Y != 0) {
			unsigned char *Temp = First;
			First = Second;
			Second = Third;
			Third = Temp;
		}
		if (Y == Height - 1) {
			memcpy(Third, Second, (Width + 2) * Channel);
		}
		else {
			memcpy(Third, Src + (Y + 1) * Stride, Channel);
			memcpy(Third + Channel, Src + (Y + 1) * Stride, Width * Channel);                            //    由于备份了前面一行的数据，这里即使Src和Dest相同也是没有问题的
			memcpy(Third + (Width + 1) * Channel, Src + (Y + 1) * Stride + (Width - 1) * Channel, Channel);
		}
		if (Channel == 1) {
			for (int X = 0; X < Width; X++)
			{
				int GX = First[X] - First[X + 2] + (Second[X] - Second[X + 2]) * 2 + Third[X] - Third[X + 2];
				int GY = First[X] + First[X + 2] + (First[X + 1] - Third[X + 1]) * 2 - Third[X] - Third[X + 2];
				LinePD[X] = IM_ClampToByte(sqrtf(GX * GX + GY * GY + 0.0F));
			}
		}
		else
		{
			for (int X = 0; X < Width * 3; X++)
			{
				int GX = First[X] - First[X + 6] + (Second[X] - Second[X + 6]) * 2 + Third[X] - Third[X + 6];
				int GY = First[X] + First[X + 6] + (First[X + 3] - Third[X + 3]) * 2 - Third[X] - Third[X + 6];
				LinePD[X] = IM_ClampToByte(sqrtf(GX * GX + GY * GY + 0.0F));
			}
		}
	}
	free(RowCopy);
}

void Sobel_INT(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride) {
	int Channel = Stride / Width;
	unsigned char *RowCopy = (unsigned char*)malloc((Width + 2) * 3 * Channel);
	unsigned char *First = RowCopy;
	unsigned char *Second = RowCopy + (Width + 2) * Channel;
	unsigned char *Third = RowCopy + (Width + 2) * 2 * Channel;
	//拷贝第二行数据，边界值填充
	memcpy(Second, Src, Channel);
	memcpy(Second + Channel, Src, Width*Channel);
	memcpy(Second + (Width + 1)*Channel, Src + (Width - 1)*Channel, Channel);
	//第一行和第二行一样
	memcpy(First, Second, (Width + 2) * Channel);
	//拷贝第三行数据，边界值填充
	memcpy(Third, Src + Stride, Channel);
	memcpy(Third + Channel, Src + Stride, Width * Channel);
	memcpy(Third + (Width + 1) * Channel, Src + Stride + (Width - 1) * Channel, Channel);

	unsigned char Table[65026];
	for (int Y = 0; Y < 65026; Y++) Table[Y] = (sqrtf(Y + 0.0f) + 0.5f);
	for (int Y = 0; Y < Height; Y++) {
		unsigned char *LinePS = Src + Y * Stride;
		unsigned char *LinePD = Dest + Y * Stride;
		if (Y != 0) {
			unsigned char *Temp = First;
			First = Second;
			Second = Third;
			Third = Temp;
		}
		if (Y == Height - 1) {
			memcpy(Third, Second, (Width + 2) * Channel);
		}
		else {
			memcpy(Third, Src + (Y + 1) * Stride, Channel);
			memcpy(Third + Channel, Src + (Y + 1) * Stride, Width * Channel);                            //    由于备份了前面一行的数据，这里即使Src和Dest相同也是没有问题的
			memcpy(Third + (Width + 1) * Channel, Src + (Y + 1) * Stride + (Width - 1) * Channel, Channel);
		}
		if (Channel == 1) {
			for (int X = 0; X < Width; X++)
			{
				int GX = First[X] - First[X + 2] + (Second[X] - Second[X + 2]) * 2 + Third[X] - Third[X + 2];
				int GY = First[X] + First[X + 2] + (First[X + 1] - Third[X + 1]) * 2 - Third[X] - Third[X + 2];
				LinePD[X] = Table[min(GX * GX + GY * GY, 65025)];
			}
		}
		else
		{
			for (int X = 0; X < Width * 3; X++)
			{
				int GX = First[X] - First[X + 6] + (Second[X] - Second[X + 6]) * 2 + Third[X] - Third[X + 6];
				int GY = First[X] + First[X + 6] + (First[X + 3] - Third[X + 3]) * 2 - Third[X] - Third[X + 6];
				LinePD[X] = Table[min(GX * GX + GY * GY, 65025)];
			}
		}
	}
	free(RowCopy);
}

void Sobel_SSE(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride) {
	int Channel = Stride / Width;
	unsigned char *RowCopy = (unsigned char*)malloc((Width + 2) * 3 * Channel);
	unsigned char *First = RowCopy;
	unsigned char *Second = RowCopy + (Width + 2) * Channel;
	unsigned char *Third = RowCopy + (Width + 2) * 2 * Channel;
	//拷贝第二行数据，边界值填充
	memcpy(Second, Src, Channel);
	memcpy(Second + Channel, Src, Width*Channel);
	memcpy(Second + (Width + 1)*Channel, Src + (Width - 1)*Channel, Channel);
	//第一行和第二行一样
	memcpy(First, Second, (Width + 2) * Channel);
	//拷贝第三行数据，边界值填充
	memcpy(Third, Src + Stride, Channel);
	memcpy(Third + Channel, Src + Stride, Width * Channel);
	memcpy(Third + (Width + 1) * Channel, Src + Stride + (Width - 1) * Channel, Channel);

	int BlockSize = 8, Block = (Width * Channel) / BlockSize;

	unsigned char Table[65026];
	for (int Y = 0; Y < 65026; Y++) Table[Y] = (sqrtf(Y + 0.0f) + 0.5f);
	for (int Y = 0; Y < Height; Y++) {
		unsigned char *LinePS = Src + Y * Stride;
		unsigned char *LinePD = Dest + Y * Stride;
		if (Y != 0) {
			unsigned char *Temp = First;
			First = Second;
			Second = Third;
			Third = Temp;
		}
		if (Y == Height - 1) {
			memcpy(Third, Second, (Width + 2) * Channel);
		}
		else {
			memcpy(Third, Src + (Y + 1) * Stride, Channel);
			memcpy(Third + Channel, Src + (Y + 1) * Stride, Width * Channel);                            //    由于备份了前面一行的数据，这里即使Src和Dest相同也是没有问题的
			memcpy(Third + (Width + 1) * Channel, Src + (Y + 1) * Stride + (Width - 1) * Channel, Channel);
		}
		if (Channel == 1) {
			for (int X = 0; X < Width; X++)
			{
				int GX = First[X] - First[X + 2] + (Second[X] - Second[X + 2]) * 2 + Third[X] - Third[X + 2];
				int GY = First[X] + First[X + 2] + (First[X + 1] - Third[X + 1]) * 2 - Third[X] - Third[X + 2];
				//LinePD[X] = Table[min(GX * GX + GY * GY, 65025)];
			}
		}
		else
		{
			__m128i Zero = _mm_setzero_si128();
			for (int X = 0; X < Block * BlockSize; X += BlockSize)
			{
				__m128i FirstP0 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i *)(First + X)), Zero);
				__m128i FirstP1 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i *)(First + X + 3)), Zero);
				__m128i FirstP2 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i *)(First + X + 6)), Zero);

				__m128i SecondP0 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i *)(Second + X)), Zero);
				__m128i SecondP2 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i *)(Second + X + 6)), Zero);

				__m128i ThirdP0 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i *)(Third + X)), Zero);
				__m128i ThirdP1 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i *)(Third + X + 3)), Zero);
				__m128i ThirdP2 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i *)(Third + X + 6)), Zero);

				__m128i GX16 = _mm_abs_epi16(_mm_add_epi16(_mm_add_epi16(_mm_sub_epi16(FirstP0, FirstP2), _mm_slli_epi16(_mm_sub_epi16(SecondP0, SecondP2), 1)), _mm_sub_epi16(ThirdP0, ThirdP2)));
				__m128i GY16 = _mm_abs_epi16(_mm_sub_epi16(_mm_add_epi16(_mm_add_epi16(FirstP0, FirstP2), _mm_slli_epi16(_mm_sub_epi16(FirstP1, ThirdP1), 1)), _mm_add_epi16(ThirdP0, ThirdP2)));

				__m128i GXYL = _mm_unpacklo_epi16(GX16, GY16);
				__m128i GXYH = _mm_unpackhi_epi16(GX16, GY16);

				__m128i ResultL = _mm_cvtps_epi32(_mm_sqrt_ps(_mm_cvtepi32_ps(_mm_madd_epi16(GXYL, GXYL))));
				__m128i ResultH = _mm_cvtps_epi32(_mm_sqrt_ps(_mm_cvtepi32_ps(_mm_madd_epi16(GXYH, GXYH))));

				/*__m128i GX32L = _mm_unpacklo_epi16(GX16, Zero);
				__m128i GX32H = _mm_unpackhi_epi16(GX16, Zero);
				__m128i GY32L = _mm_unpacklo_epi16(GY16, Zero);
				__m128i GY32H = _mm_unpackhi_epi16(GY16, Zero);

				__m128i ResultL = _mm_cvtps_epi32(_mm_sqrt_ps(_mm_cvtepi32_ps(_mm_add_epi32(_mm_mullo_epi32(GX32L, GX32L), _mm_mullo_epi32(GY32L, GY32L)))));
				__m128i ResultH = _mm_cvtps_epi32(_mm_sqrt_ps(_mm_cvtepi32_ps(_mm_add_epi32(_mm_mullo_epi32(GX32H, GX32H), _mm_mullo_epi32(GY32H, GY32H)))));

				_mm_storel_epi64((__m128i *)(LinePD + X), _mm_packus_epi16(_mm_packus_epi32(ResultL, ResultH), Zero));*/
				_mm_storel_epi64((__m128i *)(LinePD + X), _mm_packus_epi16(_mm_packus_epi32(ResultL, ResultH), Zero));
			}

			for (int X = Block * BlockSize; X < Width * 3; X++)
			{
				int GX = First[X] - First[X + 6] + (Second[X] - Second[X + 6]) * 2 + Third[X] - Third[X + 6];
				int GY = First[X] + First[X + 6] + (First[X + 3] - Third[X + 3]) * 2 - Third[X] - Third[X + 6];
				LinePD[X] = IM_ClampToByte(sqrtf(GX * GX + GY * GY + 0.0F));
			}
		}
	}
	free(RowCopy);
}

int main() {
	Mat src = imread("F:\\car.jpg");
	int Height = src.rows;
	int Width = src.cols;
	unsigned char *Src = src.data;
	unsigned char *Dest = new unsigned char[Height * Width * 3];
	int Stride = Width * 3;
	int Radius = 11;
	int Adjustment = 50;
	int64 st = cvGetTickCount();
	for (int i = 0; i <50; i++) {
		Sobel_SSE(Src, Dest, Width, Height, Stride);
	}
	double duration = (cv::getTickCount() - st) / cv::getTickFrequency() * 20;
	printf("%.5f\n", duration);
	Sobel_SSE(Src, Dest, Width, Height, Stride);
	Mat dst(Height, Width, CV_8UC3, Dest);
	imshow("origin", src);
	imshow("result", dst);
	imwrite("F:\\res.jpg", dst);
	waitKey(0);
	waitKey(0);
}