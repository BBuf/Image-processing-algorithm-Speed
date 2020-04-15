#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <future>
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

void Sobel_SSE1(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride) {
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

				__m128i GX32L = _mm_unpacklo_epi16(GX16, Zero);
				__m128i GX32H = _mm_unpackhi_epi16(GX16, Zero);
				__m128i GY32L = _mm_unpacklo_epi16(GY16, Zero);
				__m128i GY32H = _mm_unpackhi_epi16(GY16, Zero);
				__m128i ResultL = _mm_cvtps_epi32(_mm_sqrt_ps(_mm_cvtepi32_ps(_mm_add_epi32(_mm_mullo_epi32(GX32L, GX32L), _mm_mullo_epi32(GY32L, GY32L)))));
				__m128i ResultH = _mm_cvtps_epi32(_mm_sqrt_ps(_mm_cvtepi32_ps(_mm_add_epi32(_mm_mullo_epi32(GX32H, GX32H), _mm_mullo_epi32(GY32H, GY32H)))));
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

void Sobel_SSE2(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride) {
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

unsigned char *RowCopy;
unsigned char *First;
unsigned char *Second;
unsigned char *Third;
int Channel, Block, BlockSize;
void _Sobel(unsigned char* Src, const int32_t Width, const int32_t Height, const int32_t start_row, const int32_t thread_stride, const int32_t Stride, unsigned char* Dest) {
	for (int Y = start_row; Y < start_row + thread_stride; Y++) {
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
			__m256i Zero = _mm256_setzero_si256();
			for (int X = 0; X < Block * BlockSize; X += BlockSize)
			{
				__m256i FirstP0 = _mm256_cvtepu8_epi16(_mm_loadu_si128((const __m128i*)(First + X)));
				__m256i FirstP1 = _mm256_cvtepu8_epi16(_mm_loadu_si128((const __m128i*)(First + X + 3)));
				__m256i FirstP2 = _mm256_cvtepu8_epi16(_mm_loadu_si128((const __m128i*)(First + X + 6)));

				__m256i SecondP0 = _mm256_cvtepu8_epi16(_mm_loadu_si128((const __m128i*)(Second + X)));
				__m256i SecondP2 = _mm256_cvtepu8_epi16(_mm_loadu_si128((const __m128i*)(Second + X + 6)));

				__m256i ThirdP0 = _mm256_cvtepu8_epi16(_mm_loadu_si128((const __m128i*)(Third + X)));
				__m256i ThirdP1 = _mm256_cvtepu8_epi16(_mm_loadu_si128((const __m128i*)(Third + X + 3)));
				__m256i ThirdP2 = _mm256_cvtepu8_epi16(_mm_loadu_si128((const __m128i*)(Third + X + 6)));

				//GX0	GX1	    GX2    GX3    GX4    GX5    GX6    GX7     GX8	 GX9	 GX10    GX11    GX12    GX13    GX14    GX15
				__m256i GX16 = _mm256_abs_epi16(_mm256_adds_epi16(_mm256_adds_epi16(_mm256_subs_epi16(FirstP0, FirstP2), _mm256_slli_epi16(_mm256_subs_epi16(SecondP0, SecondP2), 1)), _mm256_subs_epi16(ThirdP0, ThirdP2)));
				//GY0   GY1     GY2    GY3    GY4    GY5    GY6    GY7     GY8   GY9     GY10    GY11    GY12    GY13    GY14    GY15
				__m256i GY16 = _mm256_abs_epi16(_mm256_subs_epi16(_mm256_adds_epi16(_mm256_adds_epi16(FirstP0, FirstP2), _mm256_slli_epi16(_mm256_subs_epi16(FirstP1, ThirdP1), 1)), _mm256_adds_epi16(ThirdP0, ThirdP2)));
				//GX0　　GY0　　GX1　　GY1　　GX2　　GY2　　GX3　　GY3    GX4    GY4     GX5     GY5      GX6     GY6     GX7     GY7
				__m256i GXYL = _mm256_unpacklo_epi16(GX16, GY16);
				//GX8　　GY8　　GX9　　GY9　　GX10　GY10　　GX11　GY11    GX12   GY12    GX13    GY13     GX14    GY14    GX15    GY15     
				__m256i GXYH = _mm256_unpackhi_epi16(GX16, GY16);


				__m256i ResultL = _mm256_cvtps_epi32(_mm256_sqrt_ps(_mm256_cvtepi32_ps(_mm256_madd_epi16(GXYL, GXYL))));
				__m256i ResultH = _mm256_cvtps_epi32(_mm256_sqrt_ps(_mm256_cvtepi32_ps(_mm256_madd_epi16(GXYH, GXYH))));

				__m256i Result = _mm256_packus_epi16(_mm256_packus_epi32(ResultL, ResultH), Zero);

				__m128i Ans = _mm256_castsi256_si128(Result);
				_mm_storeu_si128((__m128i *)(LinePD + X), Ans);
			}

			for (int X = Block * BlockSize; X < Width * 3; X++)
			{
				int GX = First[X] - First[X + 6] + (Second[X] - Second[X + 6]) * 2 + Third[X] - Third[X + 6];
				int GY = First[X] + First[X + 6] + (First[X + 3] - Third[X + 3]) * 2 - Third[X] - Third[X + 6];
				LinePD[X] = IM_ClampToByte(sqrtf(GX * GX + GY * GY + 0.0F));
			}
		}
	}
}

void Sobel_AVX1(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride) {
	Channel = Stride / Width;
	RowCopy = (unsigned char*)malloc((Width + 2) * 3 * Channel);
	First = RowCopy;
	Second = RowCopy + (Width + 2) * Channel;
	Third = RowCopy + (Width + 2) * 2 * Channel;
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

	BlockSize = 16, Block = (Width * Channel) / BlockSize;

	_Sobel(Src, Width, Height, 0, Height, Stride, Dest);
	
	free(RowCopy);
	free(First);
	free(Second);
	free(Third);
}

void Sobel_AVX2(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride) {
	//INIT
	Channel = Stride / Width;
	RowCopy = (unsigned char*)malloc((Width + 2) * 3 * Channel);
	First = RowCopy;
	Second = RowCopy + (Width + 2) * Channel;
	Third = RowCopy + (Width + 2) * 2 * Channel;
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

	BlockSize = 16, Block = (Width * Channel) / BlockSize;

	//Run
	const int32_t hw_concur = std::min(Height >> 4, static_cast<int32_t>(std::thread::hardware_concurrency()));
	std::vector<std::future<void>> fut(hw_concur);
	const int thread_stride = (Height - 1) / hw_concur + 1;
	int i = 0, start = 0;
	for (; i < std::min(Height, hw_concur); i++, start += thread_stride)
	{
		fut[i] = std::async(std::launch::async, _Sobel, Src, Width, Height, start, thread_stride, Stride, Dest);
	}
	for (int j = 0; j < i; ++j)
		fut[j].wait();

	free(RowCopy);
	free(First);
	free(Second);
	free(Third);
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
	for (int i = 0; i <1000; i++) {
		Sobel_SSE3(Src, Dest, Width, Height, Stride);
	}
	double duration = (cv::getTickCount() - st) / cv::getTickFrequency();
	printf("%.5f\n", duration);
	Sobel_AVX1(Src, Dest, Width, Height, Stride);
	Mat dst(Height, Width, CV_8UC3, Dest);
	imshow("origin", src);
	imshow("result", dst);
	imwrite("F:\\res.jpg", dst);
	waitKey(0);
}