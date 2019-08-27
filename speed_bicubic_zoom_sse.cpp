#include <stdio.h>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

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
			_mm_storeu_ps(LinePD + 0, _mm_cvtepi32_ps(_mm_unpacklo_epi16(Src16L, Zero)));
			_mm_storeu_ps(LinePD + 4, _mm_cvtepi32_ps(_mm_unpackhi_epi16(Src16L, Zero)));
			_mm_storeu_ps(LinePD + 8, _mm_cvtepi32_ps(_mm_unpacklo_epi16(Src16H, Zero)));
			_mm_storeu_ps(LinePD + 12, _mm_cvtepi32_ps(_mm_unpackhi_epi16(Src16H, Zero)));
		}
		for (; X < Width; X++, LinePS += 3, LinePD += 4) {
			LinePD[0] = LinePS[0];    LinePD[1] = LinePS[1];    LinePD[2] = LinePS[2];    LinePD[3] = 0;
		}
	}
}

void ConvertBGRAF2BGR8U_SSE(float *Src, unsigned char *Dest, int Width, int Height, int Stride) {
	const int BlockSize = 4;
	int Block = (Width - 2) / BlockSize;
	//__m128i Mask = _mm_setr_epi8(0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, 3, 7, 11, 15);
	__m128i Mask1 = _mm_setr_epi8(0, -1, 1, -1, 4, -1, 5, -1, 8, -1, 9, -1, 12, -1, 13, -1);
	__m128i Mask2 = _mm_setr_epi8(2, -1, 3, -1, 6, -1, 7, -1, 10, -1, 11, -1, 14, -1, 15, -1);
	__m128i Zero = _mm_setzero_si128();
	for (int Y = 0; Y < Height; Y++) {
		float *LinePS = Src + Y * Width * 4;
		unsigned char *LinePD = Dest + Y * Stride;
		int X = 0;
		for (; X < Block * BlockSize; X += BlockSize, LinePS += BlockSize * 4, LinePD += BlockSize * 3) {
			//__m128i SrcV = _mm_shuffle_epi8(_mm_loadu_si128((const __m128i*)LinePS), Mask);
			// B1 0 G1 0 B2 0 G2 0 B3 0 G3 0 B4 0 G4 0
			__m128i BG = _mm_shuffle_epi8(_mm_loadu_si128((const __m128i*)LinePS), Mask1);
			// R1 0 A1 0 R2 0 A2 0 R3 0 A3 0 R4 0 A4 0
			__m128i RA = _mm_shuffle_epi8(_mm_loadu_si128((const __m128i*)LinePS), Mask2);
			__m128i BG16L = _mm_unpacklo_epi8(BG, Zero); // B1 0 G1 0 B2 0 G2 0 B3 0 G3 0 B4 0 G4 0
			__m128i RA16L = _mm_unpacklo_epi8(RA, Zero); // R1 0 A1 0 R2 0 A2 0 R3 0 A3 0 R4 0 A4 0

			__m128i B = _mm_unpacklo_epi16(BG16L, Zero); // B1 0 B2 0 B3 0 B4 0
			__m128i G = _mm_unpackhi_epi16(BG16L, Zero); // G1 0 G2 0 G3 0 G4 0
			__m128i R = _mm_unpacklo_epi16(RA16L, Zero); // R1 0 R2 0 R3 0 R4 0
			
			__m128i Dest1 = _mm_shuffle_epi8(B, _mm_setr_epi16(0, -1, -1, -1, -1, -1, 1, -1));
			Dest1 = _mm_or_si128(Dest1, _mm_shuffle_epi8(G, _mm_setr_epi16(-1, -1, 0, -1, -1, -1, -1, -1)));
			Dest1 = _mm_or_si128(Dest1, _mm_shuffle_epi8(R, _mm_setr_epi16(-1, -1, -1, -1, 0, -1, -1, -1)));

			__m128i Dest2 = _mm_shuffle_epi8(B, _mm_setr_epi16(-1, -1, -1, -1, 2, -1, -1, -1));
			Dest2 = _mm_or_si128(Dest2, _mm_shuffle_epi8(G, _mm_setr_epi16(1, -1, -1, -1, -1, -1, 2, -1)));
			Dest2 = _mm_or_si128(Dest2, _mm_shuffle_epi8(R, _mm_setr_epi16(-1, -1, 1, -1, -1, -1, -1, -1)));

			__m128i Dest3 = _mm_shuffle_epi8(B, _mm_setr_epi16(-1, -1, 3, -1, -1, -1, -1, -1));
			Dest3 = _mm_or_si128(Dest3, _mm_shuffle_epi8(G, _mm_setr_epi16(-1, -1, -1, -1, 3, -1, -1, -1)));
			Dest3 = _mm_or_si128(Dest3, _mm_shuffle_epi8(R, _mm_setr_epi16(2, -1, -1, -1, -1, -1, 3, -1)));

			Dest1 = _mm_packus_epi32(Dest1, Zero);
			Dest2 = _mm_packus_epi32(Dest2, Zero);
			Dest3 = _mm_packus_epi32(Dest3, Zero);
			_mm_storeu_si128((__m128i*)(LinePD + 0), Dest1);
			_mm_storeu_si128((__m128i*)(LinePD + 4), Dest2);
			_mm_storeu_si128((__m128i*)(LinePD + 8), Dest3);
		}
		for (; X < Width; X++, LinePS += 4, LinePD += 3) {
			LinePD[0] = LinePS[0]; LinePD[1] = LinePS[1]; LinePD[2] = LinePS[2];
		}
	}
}

// 将整形的Value值限定在Min和Max内，可取Min或者Max的值
inline int ClampI(int Value, int Min, int Max) {
	if (Value < Min) return Min;
	else if (Value > Max) return Max;
	else return Value;
}

// 将整数限制到字节数据类型
inline unsigned char ClampToByte(int Value) {
	if (Value < 0) return 0;
	else if (Value > 255) return 255;
	else return (unsigned char)Value;
}

// 获取PosX, PosY位置的像素
inline unsigned char *GetCheckedPixel(unsigned char *Src, int Width, int Height, int Stride, int Channel, int PosX, int PosY) {
	return Src + ClampI(PosY, 0, Height - 1) * Stride + ClampI(PosX, 0, Width - 1) * Channel;
}

// 该函数计算插值曲线sin(x * PI) / (x * PI)的值,下面是它的近似拟合表达式
float SinXDivX(float X) {
	const float a = -1; //a还可以取 a=-2,-1,-0.75,-0.5等等，起到调节锐化或模糊程度的作用
	X = abs(X);
	float X2 = X * X, X3 = X2 * X;
	if (X <= 1)
		return (a + 2) * X3 - (a + 3) * X2 + 1;
	else if (X <= 2)
		return a * X3 - (5 * a) * X2 + (8 * a) * X - (4 * a);
	else
		return 0;
}

// 精确计算插值曲线sin(x * PI) / (x * PI)
float SinXDivX_Standard(float X) {
	if (abs(X) < 0.000001f)
		return 1;
	else
		return sin(X * 3.1415926f) / (X * 3.1415926f);
}

void Bicubic_Original(unsigned char *Src, int Width, int Height, int Stride, unsigned char *Pixel, float X, float Y) {
	int Channel = Stride / Width;
	int PosX = floor(X), PosY = floor(Y);
	float PartXX = X - PosX, PartYY = Y - PosY;
	unsigned char *Pixel00 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX - 1, PosY - 1);
	unsigned char *Pixel01 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 0, PosY - 1);
	unsigned char *Pixel02 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 1, PosY - 1);
	unsigned char *Pixel03 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 2, PosY - 1);
	unsigned char *Pixel10 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX - 1, PosY + 0);
	unsigned char *Pixel11 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 0, PosY + 0);
	unsigned char *Pixel12 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 1, PosY + 0);
	unsigned char *Pixel13 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 2, PosY + 0);
	unsigned char *Pixel20 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX - 1, PosY + 1);
	unsigned char *Pixel21 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 0, PosY + 1);
	unsigned char *Pixel22 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 1, PosY + 1);
	unsigned char *Pixel23 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 2, PosY + 1);
	unsigned char *Pixel30 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX - 1, PosY + 2);
	unsigned char *Pixel31 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 0, PosY + 2);
	unsigned char *Pixel32 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 1, PosY + 2);
	unsigned char *Pixel33 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 2, PosY + 2);

	float U0 = SinXDivX(1 + PartXX), U1 = SinXDivX(PartXX);
	float U2 = SinXDivX(1 - PartXX), U3 = SinXDivX(2 - PartXX);
	float V0 = SinXDivX(1 + PartYY), V1 = SinXDivX(PartYY);
	float V2 = SinXDivX(1 - PartYY), V3 = SinXDivX(2 - PartYY);

	for (int I = 0; I < Channel; I++) {
		float Sum1 = (Pixel00[I] * U0 + Pixel01[I] * U1 + Pixel02[I] * U2 + Pixel03[I] * U3) * V0;
		float Sum2 = (Pixel10[I] * U0 + Pixel11[I] * U1 + Pixel12[I] * U2 + Pixel13[I] * U3) * V1;
		float Sum3 = (Pixel20[I] * U0 + Pixel21[I] * U1 + Pixel22[I] * U2 + Pixel23[I] * U3) * V2;
		float Sum4 = (Pixel30[I] * U0 + Pixel31[I] * U1 + Pixel22[I] * U2 + Pixel33[I] * U3) * V3;
		Pixel[I] = ClampToByte(Sum1 + Sum2 + Sum3 + Sum4 + 0.5f);
	}
}

// ImageShop说如果把Channel改为固定的值，速度能提高很多，待测试
void Bicubic_Border(unsigned char *Src, int Width, int Height, int Stride, unsigned char *Pixel, short *SinXDivX_Table, int SrcX, int SrcY) {
	int Channel = Stride / Width;
	int U = (unsigned char)(SrcX >> 8), V = (unsigned char)(SrcY >> 8);

	int U0 = SinXDivX_Table[256 + U], U1 = SinXDivX_Table[U];
	int U2 = SinXDivX_Table[256 - U], U3 = SinXDivX_Table[512 - U];
	int V0 = SinXDivX_Table[256 + V], V1 = SinXDivX_Table[V];
	int V2 = SinXDivX_Table[256 - V], V3 = SinXDivX_Table[512 - V];
	int PosX = SrcX >> 16, PosY = SrcY >> 16;

	unsigned char *Pixel00 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX - 1, PosY - 1);
	unsigned char *Pixel01 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 0, PosY - 1);
	unsigned char *Pixel02 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 1, PosY - 1);
	unsigned char *Pixel03 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 2, PosY - 1);
	unsigned char *Pixel10 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX - 1, PosY + 0);
	unsigned char *Pixel11 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 0, PosY + 0);
	unsigned char *Pixel12 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 1, PosY + 0);
	unsigned char *Pixel13 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 2, PosY + 0);
	unsigned char *Pixel20 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX - 1, PosY + 1);
	unsigned char *Pixel21 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 0, PosY + 1);
	unsigned char *Pixel22 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 1, PosY + 1);
	unsigned char *Pixel23 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 2, PosY + 1);
	unsigned char *Pixel30 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX - 1, PosY + 2);
	unsigned char *Pixel31 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 0, PosY + 2);
	unsigned char *Pixel32 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 1, PosY + 2);
	unsigned char *Pixel33 = GetCheckedPixel(Src, Width, Height, Stride, Channel, PosX + 2, PosY + 2);

	for (int I = 0; I < Channel; I++)
	{
		int Sum1 = (Pixel00[I] * U0 + Pixel01[I] * U1 + Pixel02[I] * U2 + Pixel03[I] * U3) * V0;
		int Sum2 = (Pixel10[I] * U0 + Pixel11[I] * U1 + Pixel12[I] * U2 + Pixel13[I] * U3) * V1;
		int Sum3 = (Pixel20[I] * U0 + Pixel21[I] * U1 + Pixel22[I] * U2 + Pixel23[I] * U3) * V2;
		int Sum4 = (Pixel30[I] * U0 + Pixel31[I] * U1 + Pixel22[I] * U2 + Pixel33[I] * U3) * V3;
		Pixel[I] = ClampToByte((Sum1 + Sum2 + Sum3 + Sum4) >> 16);
	}
}
void Bicubic_Center(unsigned char *Src, int Width, int Height, int Stride, unsigned char *Pixel, short *SinXDivX_Table, int SrcX, int SrcY)
{
	int Channel = Stride / Width;
	int U = (unsigned char)(SrcX >> 8), V = (unsigned char)(SrcY >> 8);

	int U0 = SinXDivX_Table[256 + U], U1 = SinXDivX_Table[U];
	int U2 = SinXDivX_Table[256 - U], U3 = SinXDivX_Table[512 - U];
	int V0 = SinXDivX_Table[256 + V], V1 = SinXDivX_Table[V];
	int V2 = SinXDivX_Table[256 - V], V3 = SinXDivX_Table[512 - V];
	int PosX = SrcX >> 16, PosY = SrcY >> 16;

	unsigned char *Pixel00 = Src + (PosY - 1) * Stride + (PosX - 1) * Channel;
	unsigned char *Pixel01 = Pixel00 + Channel;
	unsigned char *Pixel02 = Pixel01 + Channel;
	unsigned char *Pixel03 = Pixel02 + Channel;
	unsigned char *Pixel10 = Pixel00 + Stride;
	unsigned char *Pixel11 = Pixel10 + Channel;
	unsigned char *Pixel12 = Pixel11 + Channel;
	unsigned char *Pixel13 = Pixel12 + Channel;
	unsigned char *Pixel20 = Pixel10 + Stride;
	unsigned char *Pixel21 = Pixel20 + Channel;
	unsigned char *Pixel22 = Pixel21 + Channel;
	unsigned char *Pixel23 = Pixel22 + Channel;
	unsigned char *Pixel30 = Pixel20 + Stride;
	unsigned char *Pixel31 = Pixel30 + Channel;
	unsigned char *Pixel32 = Pixel31 + Channel;
	unsigned char *Pixel33 = Pixel32 + Channel;
	for (int I = 0; I < Channel; I++)
	{
		int Sum1 = (Pixel00[I] * U0 + Pixel01[I] * U1 + Pixel02[I] * U2 + Pixel03[I] * U3) * V0;
		int Sum2 = (Pixel10[I] * U0 + Pixel11[I] * U1 + Pixel12[I] * U2 + Pixel13[I] * U3) * V1;
		int Sum3 = (Pixel20[I] * U0 + Pixel21[I] * U1 + Pixel22[I] * U2 + Pixel23[I] * U3) * V2;
		int Sum4 = (Pixel30[I] * U0 + Pixel31[I] * U1 + Pixel22[I] * U2 + Pixel33[I] * U3) * V3;
		Pixel[I] = ClampToByte((Sum1 + Sum2 + Sum3 + Sum4) >> 16);
	}
}

// 原始的插值算法
void IM_Resize_Cubic_Origin(unsigned char *Src, unsigned char *Dest, int SrcW, int SrcH, int StrideS, int DstW, int DstH, int StrideD) {
	int Channel = StrideS / SrcW;
	if ((SrcW == DstW) && (SrcH == DstH)) {
		memcpy(Dest, Src, SrcW * SrcH * Channel * sizeof(unsigned char));
		return;
	}
	for (int Y = 0; Y < DstH; Y++) {
		unsigned char *LinePD = Dest + Y * StrideD;
		float SrcY = (Y + 0.49999999f) * SrcH / DstH - 0.5f;
		for (int X = 0; X < DstW; X++) {
			float SrcX = (X + 0.4999999f) * SrcW / DstW - 0.5f;
			Bicubic_Original(Src, SrcW, SrcH, StrideS, LinePD, SrcX, SrcY);
			LinePD += Channel;
		}
	}
}

// C语言实现的查表+插值算法
void IM_Resize_Cubic_Table(unsigned char *Src, unsigned char *Dest, int SrcW, int SrcH, int StrideS, int DstW, int DstH, int StrideD) {
	int Channel = StrideS / SrcW;
	if ((SrcW == DstW) && (SrcH == DstH)) {
		memcpy(Dest, Src, SrcW * SrcH * Channel * sizeof(unsigned char));
		return;
	}
	short *SinXDivX_Table = (short *)malloc(513 * sizeof(short));
	for (int I = 0; I < 513; I++)
		SinXDivX_Table[I] = int(0.5 + 256 * SinXDivX(I / 256.0f)); // 建立查找表，定点化
	int AddX = (SrcW << 16) / DstW, AddY = (SrcH << 16) / DstH;
	int ErrorX = -(1 << 15) + (AddX >> 1), ErrorY = -(1 << 15) + (AddY >> 1);

	int StartX = ((1 << 16) - ErrorX) / AddX + 1;			//	计算出需要特殊处理的边界
	int StartY = ((1 << 16) - ErrorY) / AddY + 1;			//	y0+y*yr>=1; y0=ErrorY => y>=(1-ErrorY)/yr
	int EndX = (((SrcW - 3) << 16) - ErrorX) / AddX + 1;
	int EndY = (((SrcH - 3) << 16) - ErrorY) / AddY + 1;	//	y0+y*yr<=(height-3) => y<=(height-3-ErrorY)/yr
	if (StartY >= DstH)			StartY = DstH;
	if (StartX >= DstW)			StartX = DstW;
	if (EndX < StartX)			EndX = StartX;
	if (EndY < StartY)			EndY = StartY;
	// 输出边界
	//printf("%d %d %d %d\n", StartX, StartY, EndX, EndY);
	int SrcY = ErrorY;
	for (int Y = 0; Y < StartY; Y++, SrcY += AddY)			//	前面的不是都有效的取样部分数据
	{
		unsigned char *LinePD = Dest + Y * StrideD;
		for (int X = 0, SrcX = ErrorX; X < DstW; X++, SrcX += AddX, LinePD += Channel)
		{
			Bicubic_Border(Src, SrcW, SrcH, StrideS, LinePD, SinXDivX_Table, SrcX, SrcY);
		}
	}
	for (int Y = StartY; Y < EndY; Y++, SrcY += AddY)
	{
		int SrcX = ErrorX;
		unsigned char *LinePD = Dest + Y * StrideD;
		for (int X = 0; X < StartX; X++, SrcX += AddX, LinePD += Channel)
		{
			Bicubic_Border(Src, SrcW, SrcH, StrideS, LinePD, SinXDivX_Table, SrcX, SrcY);
		}
		for (int X = StartX; X < EndX; X++, SrcX += AddX, LinePD += Channel)
		{
			Bicubic_Center(Src, SrcW, SrcH, StrideS, LinePD, SinXDivX_Table, SrcX, SrcY);
		}
		for (int X = EndX; X < DstW; X++, SrcX += AddX, LinePD += Channel)
		{
			Bicubic_Border(Src, SrcW, SrcH, StrideS, LinePD, SinXDivX_Table, SrcX, SrcY);
		}
	}
	for (int Y = EndY; Y < DstH; Y++, SrcY += AddY)
	{
		unsigned char *LinePD = Dest + Y * StrideD;
		for (int X = 0, SrcX = ErrorX; X < DstW; X++, SrcX += AddX, LinePD += Channel)
		{
			Bicubic_Border(Src, SrcW, SrcH, StrideS, LinePD, SinXDivX_Table, SrcX, SrcY);
		}
	}
	free(SinXDivX_Table);
}

// 4个有符号的32位的数据相加的和
inline int _mm_hsum_epi32(__m128i V) { //V3 V2 V1 V0
	__m128i T = _mm_add_epi32(V, _mm_srli_si128(V, 8)); //V3+V1	 V2+V0	V1	V0
	T = _mm_add_epi32(T, _mm_srli_si128(T, 4)); //V3+V1+V2+V0		V2+V0+V1	V1+V0	V0
	return _mm_cvtsi128_si32(T); //提取低位
}

// 使用SSE优化立方插值算法
// 最大支持图像大小为: 32767*32767
void IM_Resize_SSE(unsigned char *Src, unsigned char *Dest, int SrcW, int SrcH, int StrideS, int DstW, int DstH, int StrideD) {
	int Channel = StrideS / SrcW;
	if ((SrcW == DstW) && (SrcH == DstH)) {
		memcpy(Dest, Src, SrcW * SrcH * Channel * sizeof(unsigned char));
		return;
	}
	short *SinXDivX_Table = (short *)malloc(513 * sizeof(short));
	short *Table = (short *)malloc(DstW * 4 * sizeof(short));
	for (int I = 0; I < 513; I++)
		SinXDivX_Table[I] = int(0.5 + 256 * SinXDivX(I / 256.0f)); //	建立查找表，定点化
	int AddX = (SrcW << 16) / DstW, AddY = (SrcH << 16) / DstH;
	int ErrorX = -(1 << 15) + (AddX >> 1), ErrorY = -(1 << 15) + (AddY >> 1);

	int StartX = ((1 << 16) - ErrorX) / AddX + 1;			//	计算出需要特殊处理的边界
	int StartY = ((1 << 16) - ErrorY) / AddY + 1;			//	y0+y*yr>=1; y0=ErrorY => y>=(1-ErrorY)/yr
	int EndX = (((SrcW - 3) << 16) - ErrorX) / AddX + 1;
	int EndY = (((SrcH - 3) << 16) - ErrorY) / AddY + 1;	//	y0+y*yr<=(height-3) => y<=(height-3-ErrorY)/yr
	if (StartY >= DstH)			StartY = DstH;
	if (StartX >= DstW)			StartX = DstW;
	if (EndX < StartX)			EndX = StartX;
	if (EndY < StartY)			EndY = StartY;
	for(int X = StartX, SrcX = ErrorX + StartX * AddX; X < EndY; X++, SrcX += AddX){
		int U = (unsigned char)(SrcX >> 8);
		Table[X * 4 + 0] = SinXDivX_Table[256 + U]; //建立一个新表便于SSE操作
		Table[X * 4 + 1] = SinXDivX_Table[U];
		Table[X * 4 + 2] = SinXDivX_Table[256 - U];
		Table[X * 4 + 3] = SinXDivX_Table[512 - U];
	}
	int SrcY = ErrorY;
	for (int Y = 0; Y < StartY; Y++, SrcY += AddY){ // 同IM_Resize_Cubic_Table函数
		unsigned char *LinePD = Dest + Y * StrideD;
		for (int X = 0, SrcX = ErrorX; X < DstW; X++, SrcX += AddX, LinePD += Channel){
			Bicubic_Border(Src, SrcW, SrcH, StrideS, LinePD, SinXDivX_Table, SrcX, SrcY);
		}
	}
	for (int Y = StartY; Y < EndY; Y++, SrcY += AddY) {
		int SrcX = ErrorX;
		unsigned char *LinePD = Dest + Y * StrideD;
		for (int X = 0; X < StartX; X++, SrcX += AddX, LinePD += Channel){
			Bicubic_Border(Src, SrcW, SrcH, StrideS, LinePD, SinXDivX_Table, SrcX, SrcY);
		}
		int V = (unsigned char)(SrcY >> 8);
		unsigned char *LineY = Src + ((SrcY >> 16) - 1) * StrideS;
		__m128i PartY = _mm_setr_epi32(SinXDivX_Table[256 + V], SinXDivX_Table[V], SinXDivX_Table[256 - V], SinXDivX_Table[512 - V]);
		for (int X = StartX; X < EndX; X++, SrcX += AddX, LinePD += Channel) {
			__m128i PartX = _mm_loadl_epi64((__m128i *)(Table + X * 4));
			//PartX: U0 U1 U2 U3 U0 U1 U2 U3 
			PartX = _mm_unpacklo_epi64(PartX, PartX); 
			unsigned char *Pixel0 = LineY + ((SrcX >> 16) - 1) * Channel;
			unsigned char *Pixel1 = Pixel0 + StrideS;
			unsigned char *Pixel2 = Pixel1 + StrideS;
			unsigned char *Pixel3 = Pixel2 + StrideS;
			if (Channel == 1) {
				__m128i P01 = _mm_cvtepu8_epi16(_mm_unpacklo_epi32(_mm_cvtsi32_si128(*((int *)Pixel0)), _mm_cvtsi32_si128(*((int *)Pixel1)))); //	P00 P01 P02 P03 P10 P11 P12 P13
				__m128i P23 = _mm_cvtepu8_epi16(_mm_unpacklo_epi32(_mm_cvtsi32_si128(*((int *)Pixel2)), _mm_cvtsi32_si128(*((int *)Pixel3)))); //	P20 P21 P22 P23 P30 P31 P32 P33
				__m128i Sum01 = _mm_madd_epi16(P01, PartX); // P00 * U0 + P01 * U1		P02 * U2 + P03 * U3		 P10 * U0 + P11 * U1		P12 * U2 + P13 * U3
				__m128i Sum23 = _mm_madd_epi16(P23, PartX); // P20 * U0 + P21 * U1		P22 * U2 + P23 * U3		 P30 * U0 + P31 * U1		P32 * U2 + P33 * U3
				__m128i Sum = _mm_hadd_epi32(Sum01, Sum23); // P00 * U0 + P01 * U1 + P02 * U2 + P03 * U3	 P10 * U0 + P11 * U1 + P12 * U2 + P13 * U3	P20 * U0 + P21 * U1	+ P22 * U2 + P23 * U3	P30 * U0 + P31 * U1 + P32 * U2 + P33 * U3
				LinePD[0] = ClampToByte(_mm_hsum_epi32(_mm_mullo_epi32(Sum, PartY)) >> 16);
			}
			else if (Channel == 4) {
				__m128i P0 = _mm_loadu_si128((__m128i *)Pixel0), P1 = _mm_loadu_si128((__m128i *)Pixel1);
				__m128i P2 = _mm_loadu_si128((__m128i *)Pixel2), P3 = _mm_loadu_si128((__m128i *)Pixel3);
				P0 = _mm_shuffle_epi8(P0, _mm_setr_epi8(0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15));	 // B0 G0 R0 A0
				P1 = _mm_shuffle_epi8(P1, _mm_setr_epi8(0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15));	 //	B1 G1 R1 A1
				P2 = _mm_shuffle_epi8(P2, _mm_setr_epi8(0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15));	 // B2 G2 R2 A2
				P3 = _mm_shuffle_epi8(P3, _mm_setr_epi8(0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15));	 //	B3 G3 R3 A3

				__m128i BG01 = _mm_unpacklo_epi32(P0, P1);		//	B0 B1 G0 G1
				__m128i RA01 = _mm_unpackhi_epi32(P0, P1);		//	R0 R1 A0 A1
				__m128i BG23 = _mm_unpacklo_epi32(P2, P3);		//	B2 B3 G2 G3
				__m128i RA23 = _mm_unpackhi_epi32(P2, P3);		//	R2 R3 A2 A3

				__m128i B01 = _mm_unpacklo_epi8(BG01, _mm_setzero_si128());
				__m128i B23 = _mm_unpacklo_epi8(BG23, _mm_setzero_si128());
				__m128i SumB = _mm_hadd_epi32(_mm_madd_epi16(B01, PartX), _mm_madd_epi16(B23, PartX));

				__m128i G01 = _mm_unpackhi_epi8(BG01, _mm_setzero_si128());
				__m128i G23 = _mm_unpackhi_epi8(BG23, _mm_setzero_si128());
				__m128i SumG = _mm_hadd_epi32(_mm_madd_epi16(G01, PartX), _mm_madd_epi16(G23, PartX));

				__m128i R01 = _mm_unpacklo_epi8(RA01, _mm_setzero_si128());
				__m128i R23 = _mm_unpacklo_epi8(RA23, _mm_setzero_si128());
				__m128i SumR = _mm_hadd_epi32(_mm_madd_epi16(R01, PartX), _mm_madd_epi16(R23, PartX));

				__m128i A01 = _mm_unpackhi_epi8(RA01, _mm_setzero_si128());
				__m128i A23 = _mm_unpackhi_epi8(RA23, _mm_setzero_si128());
				__m128i SumA = _mm_hadd_epi32(_mm_madd_epi16(A01, PartX), _mm_madd_epi16(A23, PartX));

				__m128i Result = _mm_setr_epi32(_mm_hsum_epi32(_mm_mullo_epi32(SumB, PartY)), _mm_hsum_epi32(_mm_mullo_epi32(SumG, PartY)), _mm_hsum_epi32(_mm_mullo_epi32(SumR, PartY)), _mm_hsum_epi32(_mm_mullo_epi32(SumA, PartY)));
				Result = _mm_srai_epi32(Result, 16);
				//	*((int *)LinePD) = _mm_cvtsi128_si32(_mm_packus_epi16(_mm_packus_epi32(Result, Result), Result));
				_mm_stream_si32((int *)LinePD, _mm_cvtsi128_si32(_mm_packus_epi16(_mm_packus_epi32(Result, Result), Result)));

				//LinePD[0] = IM_ClampToByte(_mm_hsum_epi32(_mm_mullo_epi32(SumB, PartY)) >> 16);	//	确实有部分存在超出unsigned char范围的，因为定点化的缘故
				//LinePD[1] = IM_ClampToByte(_mm_hsum_epi32(_mm_mullo_epi32(SumG, PartY)) >> 16);
				//LinePD[2] = IM_ClampToByte(_mm_hsum_epi32(_mm_mullo_epi32(SumR, PartY)) >> 16);
				//LinePD[3] = IM_ClampToByte(_mm_hsum_epi32(_mm_mullo_epi32(SumA, PartY)) >> 16);
			}
		}
		for (int X = EndX; X < DstW; X++, SrcX += AddX, LinePD += Channel)
		{
			Bicubic_Border(Src, SrcW, SrcH, StrideS, LinePD, SinXDivX_Table, SrcX, SrcY);
		}
	}
	for (int Y = EndY; Y < DstH; Y++, SrcY += AddY)
	{
		unsigned char *LinePD = Dest + Y * StrideD;
		for (int X = 0, SrcX = ErrorX; X < DstW; X++, SrcX += AddX, LinePD += Channel)
		{
			Bicubic_Border(Src, SrcW, SrcH, StrideS, LinePD, SinXDivX_Table, SrcX, SrcY);
		}
	}
	free(Table);
	free(SinXDivX_Table);
}

int main() {
	Mat src = imread("F:\\car.jpg");
	int Height = src.rows;
	int Width = src.cols;
	int Stride = Width * 3;
	unsigned char *Src = src.data;
	unsigned char *Dest = new unsigned char[Height * Width * 3];
	float *Buffer = (float *)malloc(Width * Height * sizeof(float) * 4);
	ConvertBGR8U2BGRAF_SSE(Src, Buffer, Width, Height, Stride);
	ConvertBGRAF2BGR8U_SSE(Buffer, Dest, Width, Height, Stride);
	Mat dst(Height, Width, CV_8UC3, Dest);
	imshow("origin", src);
	imshow("result", dst);
	imwrite("F:\\res.jpg", dst);
	waitKey(0);
}