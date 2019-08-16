#include <stdio.h>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

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

void Bicubic_Original(unsigned char *Src, int Width, int Height, int Stride, unsigned char *Pixel, float X, float Y){
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

// C语言实现的Resize算法

void IM_Resize_Cubic_PureC(unsigned char *Src, unsigned char *Dest, int SrcW, int SrcH, int StrideS, int DstW, int DstH, int StrideD) {
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

// 使用SSE优化Resize算法
// 最大支持图像大小为: 32767*32767
void IM_Resize_SSE(unsigned char *Src, unsigned char *Dest, int SrcW, int SrcH, int StrideS, int DstW, int DstH, int StrideD) {
	int Channel = StrideS / SrcW;
	if ((SrcW == DstW) && (SrcH == DstH)) {
		memcpy(Dest, Src, SrcW * SrcH * Channel * sizeof(unsigned char));
		return;
	}
	short *SinXDivX_Table = (short *)malloc(513 * sizeof(short));
	short *Table = (short *)malloc(DstW * 4 * sizeof(short));

}