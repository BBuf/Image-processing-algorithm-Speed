#pragma once
//近似值
#include "Core.h"

union Approximation
{
	double Value;
	int X[2];
};

// 函数1: 将数据截断在Byte数据类型内。
// 参考: http://www.cnblogs.com/zyl910/archive/2012/03/12/noifopex1.html
// 简介: 用位掩码做饱和处理，用带符号右移生成掩码。
unsigned char ClampToByte(int Value) {
	return ((Value | ((signed int)(255 - Value) >> 31)) & ~((signed int)Value >> 31));
}

//函数2: 将数据截断在指定范围内
//参考: 无
//简介: 无
int ClampToInt(int Value, int Min, int Max) {
	if (Value < Min) return Min;
	else if (Value > Max) return Max;
	else return Value;
}

//函数3: 整数除以255
//参考: 无
//简介: 移位
int Div255(int Value) {
	return (((Value >> 8) + Value + 1) >> 8);
}

//函数4: 取绝对值
//参考: https://oi-wiki.org/math/bit/
//简介: 比n > 0 ? n : -n 快

int Abs(int n) {
	return (n ^ (n >> 31)) - (n >> 31);
	/* n>>31 取得 n 的符号，若 n 为正数，n>>31 等于 0，若 n 为负数，n>>31 等于 - 1
	若 n 为正数 n^0=0, 数不变，若 n 为负数有 n^-1
	需要计算 n 和 - 1 的补码，然后进行异或运算，
	结果 n 变号并且为 n 的绝对值减 1，再减去 - 1 就是绝对值 */
}

//函数5: 四舍五入
//参考: 无
//简介: 无
double Round(double V)
{
	return (V > 0.0) ? floor(V + 0.5) : Round(V - 0.5);
}

//函数6: 返回-1到1之间的随机数
//参考: 无
//简介: 无
double Rand()
{
	return (double)rand() / (RAND_MAX + 1.0);
}

//函数7: Pow函数的近似计算，针对double类型和float类型
//参考: http://www.cvchina.info/2010/03/19/log-pow-exp-approximation/
//参考: http://martin.ankerl.com/2007/10/04/optimized-pow-approximation-for-java-and-c-c/
//简介: 这个函数只是为了加速的近似计算，有5%-12%不等的误差
double Pow(double X, double Y)
{
	Approximation V = { X };
	V.X[1] = (int)(Y * (V.X[1] - 1072632447) + 1072632447);
	V.X[0] = 0;
	return V.Value;
}


float Pow(float X, float Y)
{
	Approximation V = { X };
	V.X[1] = (int)(Y * (V.X[1] - 1072632447) + 1072632447);
	V.X[0] = 0;
	return (float)V.Value;
}

//函数8: Exp函数的近似计算，针对double类型和float类型
double Exp(double Y)			//	用联合体的方式的速度要快些
{
	Approximation V;
	V.X[1] = (int)(Y * 1485963 + 1072632447);
	V.X[0] = 0;
	return V.Value;
}

float Exp(float Y)			//	用联合体的方式的速度要快些
{
	Approximation V;
	V.X[1] = (int)(Y * 1485963 + 1072632447);
	V.X[0] = 0;
	return (float)V.Value;
}

// 函数9: Pow函数更准一点的近似计算，但是速度会稍慢
// http://martin.ankerl.com/2012/01/25/optimized-approximative-pow-in-c-and-cpp/
// Besides that, I also have now a slower approximation that has much less error
// when the exponent is larger than 1. It makes use exponentiation by squaring,
// which is exact for the integer part of the exponent, and uses only the exponent’s fraction for the approximation:
// should be much more precise with large Y

double PrecisePow(double X, double Y) {
	// calculate approximation with fraction of the exponent
	int e = (int)Y;
	Approximation V = { X };
	V.X[1] = (int)((Y - e) * (V.X[1] - 1072632447) + 1072632447);
	V.X[0] = 0;
	// exponentiation by squaring with the exponent's integer part
	// double r = u.d makes everything much slower, not sure why
	double r = 1.0;
	while (e)
	{
		if (e & 1)	r *= X;
		X *= X;
		e >>= 1;
	}
	return r * V.Value;
}

//函数10: 返回Min到Max之间的随机数
//参考: 无
//简介: Min为随机数的最小值，Max为随机数的最大值
int Random(int Min, int Max) {
	return rand() % (Max + 1 - Min) + Min;
}

//函数11: 符号函数
//参考: 无
//简介: 无
int sgn(int X) {
	if (X > 0) return 1;
	if (X < 0) return -1;
	return 0;
}

//函数12: 获取某个整形变量对应的颜色值
//参考: 无
//简介: 无
void GetRGB(int Color, int *R, int *G, int *B) {
	*R = Color & 255;
	*G = (Color & 65280) / 256;
	*B = (Color & 16711680) / 65536;
}

//函数13: 牛顿法近似获取指定数字的算法平方根
//参考: https://www.cnblogs.com/qlky/p/7735145.html
//简介: 仍然是近似算法，近似出了指定数字的平方根
float Sqrt(float X)
{
	float HalfX = 0.5f * X;             // 对double类型的数字无效
	int I = *(int*)&X;                  // get bits for floating VALUE 
	I = 0x5f375a86 - (I >> 1);          // gives initial guess y0
	X = *(float*)&I;                    // convert bits BACK to float
	X = X * (1.5f - HalfX * X * X);     // Newton step, repeating increases accuracy
	X = X * (1.5f - HalfX * X * X);     // Newton step, repeating increases accuracy
	X = X * (1.5f - HalfX * X * X);     // Newton step, repeating increases accuracy
	return 1 / X;
}

//函数14: 无符号短整形直方图数据相加，即是Y = X + Y
//参考: 无
//简介: SSE优化
void HistgramAddShort(unsigned short *X, unsigned short *Y)
{
	*(__m128i*)(Y + 0) = _mm_add_epi16(*(__m128i*)&Y[0], *(__m128i*)&X[0]);		//	不要想着用自己写的汇编超过他的速度了，已经试过了
	*(__m128i*)(Y + 8) = _mm_add_epi16(*(__m128i*)&Y[8], *(__m128i*)&X[8]);
	*(__m128i*)(Y + 16) = _mm_add_epi16(*(__m128i*)&Y[16], *(__m128i*)&X[16]);
	*(__m128i*)(Y + 24) = _mm_add_epi16(*(__m128i*)&Y[24], *(__m128i*)&X[24]);
	*(__m128i*)(Y + 32) = _mm_add_epi16(*(__m128i*)&Y[32], *(__m128i*)&X[32]);
	*(__m128i*)(Y + 40) = _mm_add_epi16(*(__m128i*)&Y[40], *(__m128i*)&X[40]);
	*(__m128i*)(Y + 48) = _mm_add_epi16(*(__m128i*)&Y[48], *(__m128i*)&X[48]);
	*(__m128i*)(Y + 56) = _mm_add_epi16(*(__m128i*)&Y[56], *(__m128i*)&X[56]);
	*(__m128i*)(Y + 64) = _mm_add_epi16(*(__m128i*)&Y[64], *(__m128i*)&X[64]);
	*(__m128i*)(Y + 72) = _mm_add_epi16(*(__m128i*)&Y[72], *(__m128i*)&X[72]);
	*(__m128i*)(Y + 80) = _mm_add_epi16(*(__m128i*)&Y[80], *(__m128i*)&X[80]);
	*(__m128i*)(Y + 88) = _mm_add_epi16(*(__m128i*)&Y[88], *(__m128i*)&X[88]);
	*(__m128i*)(Y + 96) = _mm_add_epi16(*(__m128i*)&Y[96], *(__m128i*)&X[96]);
	*(__m128i*)(Y + 104) = _mm_add_epi16(*(__m128i*)&Y[104], *(__m128i*)&X[104]);
	*(__m128i*)(Y + 112) = _mm_add_epi16(*(__m128i*)&Y[112], *(__m128i*)&X[112]);
	*(__m128i*)(Y + 120) = _mm_add_epi16(*(__m128i*)&Y[120], *(__m128i*)&X[120]);
	*(__m128i*)(Y + 128) = _mm_add_epi16(*(__m128i*)&Y[128], *(__m128i*)&X[128]);
	*(__m128i*)(Y + 136) = _mm_add_epi16(*(__m128i*)&Y[136], *(__m128i*)&X[136]);
	*(__m128i*)(Y + 144) = _mm_add_epi16(*(__m128i*)&Y[144], *(__m128i*)&X[144]);
	*(__m128i*)(Y + 152) = _mm_add_epi16(*(__m128i*)&Y[152], *(__m128i*)&X[152]);
	*(__m128i*)(Y + 160) = _mm_add_epi16(*(__m128i*)&Y[160], *(__m128i*)&X[160]);
	*(__m128i*)(Y + 168) = _mm_add_epi16(*(__m128i*)&Y[168], *(__m128i*)&X[168]);
	*(__m128i*)(Y + 176) = _mm_add_epi16(*(__m128i*)&Y[176], *(__m128i*)&X[176]);
	*(__m128i*)(Y + 184) = _mm_add_epi16(*(__m128i*)&Y[184], *(__m128i*)&X[184]);
	*(__m128i*)(Y + 192) = _mm_add_epi16(*(__m128i*)&Y[192], *(__m128i*)&X[192]);
	*(__m128i*)(Y + 200) = _mm_add_epi16(*(__m128i*)&Y[200], *(__m128i*)&X[200]);
	*(__m128i*)(Y + 208) = _mm_add_epi16(*(__m128i*)&Y[208], *(__m128i*)&X[208]);
	*(__m128i*)(Y + 216) = _mm_add_epi16(*(__m128i*)&Y[216], *(__m128i*)&X[216]);
	*(__m128i*)(Y + 224) = _mm_add_epi16(*(__m128i*)&Y[224], *(__m128i*)&X[224]);
	*(__m128i*)(Y + 232) = _mm_add_epi16(*(__m128i*)&Y[232], *(__m128i*)&X[232]);
	*(__m128i*)(Y + 240) = _mm_add_epi16(*(__m128i*)&Y[240], *(__m128i*)&X[240]);
	*(__m128i*)(Y + 248) = _mm_add_epi16(*(__m128i*)&Y[248], *(__m128i*)&X[248]);
}

//函数15: 无符号短整形直方图数据相减，即是Y = Y - X
//参考: 无
//简介: SSE优化
void HistgramSubShort(unsigned short *X, unsigned short *Y)
{
	*(__m128i*)(Y + 0) = _mm_sub_epi16(*(__m128i*)&Y[0], *(__m128i*)&X[0]);
	*(__m128i*)(Y + 8) = _mm_sub_epi16(*(__m128i*)&Y[8], *(__m128i*)&X[8]);
	*(__m128i*)(Y + 16) = _mm_sub_epi16(*(__m128i*)&Y[16], *(__m128i*)&X[16]);
	*(__m128i*)(Y + 24) = _mm_sub_epi16(*(__m128i*)&Y[24], *(__m128i*)&X[24]);
	*(__m128i*)(Y + 32) = _mm_sub_epi16(*(__m128i*)&Y[32], *(__m128i*)&X[32]);
	*(__m128i*)(Y + 40) = _mm_sub_epi16(*(__m128i*)&Y[40], *(__m128i*)&X[40]);
	*(__m128i*)(Y + 48) = _mm_sub_epi16(*(__m128i*)&Y[48], *(__m128i*)&X[48]);
	*(__m128i*)(Y + 56) = _mm_sub_epi16(*(__m128i*)&Y[56], *(__m128i*)&X[56]);
	*(__m128i*)(Y + 64) = _mm_sub_epi16(*(__m128i*)&Y[64], *(__m128i*)&X[64]);
	*(__m128i*)(Y + 72) = _mm_sub_epi16(*(__m128i*)&Y[72], *(__m128i*)&X[72]);
	*(__m128i*)(Y + 80) = _mm_sub_epi16(*(__m128i*)&Y[80], *(__m128i*)&X[80]);
	*(__m128i*)(Y + 88) = _mm_sub_epi16(*(__m128i*)&Y[88], *(__m128i*)&X[88]);
	*(__m128i*)(Y + 96) = _mm_sub_epi16(*(__m128i*)&Y[96], *(__m128i*)&X[96]);
	*(__m128i*)(Y + 104) = _mm_sub_epi16(*(__m128i*)&Y[104], *(__m128i*)&X[104]);
	*(__m128i*)(Y + 112) = _mm_sub_epi16(*(__m128i*)&Y[112], *(__m128i*)&X[112]);
	*(__m128i*)(Y + 120) = _mm_sub_epi16(*(__m128i*)&Y[120], *(__m128i*)&X[120]);
	*(__m128i*)(Y + 128) = _mm_sub_epi16(*(__m128i*)&Y[128], *(__m128i*)&X[128]);
	*(__m128i*)(Y + 136) = _mm_sub_epi16(*(__m128i*)&Y[136], *(__m128i*)&X[136]);
	*(__m128i*)(Y + 144) = _mm_sub_epi16(*(__m128i*)&Y[144], *(__m128i*)&X[144]);
	*(__m128i*)(Y + 152) = _mm_sub_epi16(*(__m128i*)&Y[152], *(__m128i*)&X[152]);
	*(__m128i*)(Y + 160) = _mm_sub_epi16(*(__m128i*)&Y[160], *(__m128i*)&X[160]);
	*(__m128i*)(Y + 168) = _mm_sub_epi16(*(__m128i*)&Y[168], *(__m128i*)&X[168]);
	*(__m128i*)(Y + 176) = _mm_sub_epi16(*(__m128i*)&Y[176], *(__m128i*)&X[176]);
	*(__m128i*)(Y + 184) = _mm_sub_epi16(*(__m128i*)&Y[184], *(__m128i*)&X[184]);
	*(__m128i*)(Y + 192) = _mm_sub_epi16(*(__m128i*)&Y[192], *(__m128i*)&X[192]);
	*(__m128i*)(Y + 200) = _mm_sub_epi16(*(__m128i*)&Y[200], *(__m128i*)&X[200]);
	*(__m128i*)(Y + 208) = _mm_sub_epi16(*(__m128i*)&Y[208], *(__m128i*)&X[208]);
	*(__m128i*)(Y + 216) = _mm_sub_epi16(*(__m128i*)&Y[216], *(__m128i*)&X[216]);
	*(__m128i*)(Y + 224) = _mm_sub_epi16(*(__m128i*)&Y[224], *(__m128i*)&X[224]);
	*(__m128i*)(Y + 232) = _mm_sub_epi16(*(__m128i*)&Y[232], *(__m128i*)&X[232]);
	*(__m128i*)(Y + 240) = _mm_sub_epi16(*(__m128i*)&Y[240], *(__m128i*)&X[240]);
	*(__m128i*)(Y + 248) = _mm_sub_epi16(*(__m128i*)&Y[248], *(__m128i*)&X[248]);
}

//函数16: 无符号短整形直方图数据相加减，即是Z = Z + Y - X
//参考: 无
//简介: SSE优化
void HistgramSubAddShort(unsigned short *X, unsigned short *Y, unsigned short *Z)
{
	*(__m128i*)(Z + 0) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[0], *(__m128i*)&Z[0]), *(__m128i*)&X[0]);						//	不要想着用自己写的汇编超过他的速度了，已经试过了
	*(__m128i*)(Z + 8) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[8], *(__m128i*)&Z[8]), *(__m128i*)&X[8]);
	*(__m128i*)(Z + 16) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[16], *(__m128i*)&Z[16]), *(__m128i*)&X[16]);
	*(__m128i*)(Z + 24) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[24], *(__m128i*)&Z[24]), *(__m128i*)&X[24]);
	*(__m128i*)(Z + 32) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[32], *(__m128i*)&Z[32]), *(__m128i*)&X[32]);
	*(__m128i*)(Z + 40) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[40], *(__m128i*)&Z[40]), *(__m128i*)&X[40]);
	*(__m128i*)(Z + 48) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[48], *(__m128i*)&Z[48]), *(__m128i*)&X[48]);
	*(__m128i*)(Z + 56) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[56], *(__m128i*)&Z[56]), *(__m128i*)&X[56]);
	*(__m128i*)(Z + 64) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[64], *(__m128i*)&Z[64]), *(__m128i*)&X[64]);
	*(__m128i*)(Z + 72) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[72], *(__m128i*)&Z[72]), *(__m128i*)&X[72]);
	*(__m128i*)(Z + 80) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[80], *(__m128i*)&Z[80]), *(__m128i*)&X[80]);
	*(__m128i*)(Z + 88) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[88], *(__m128i*)&Z[88]), *(__m128i*)&X[88]);
	*(__m128i*)(Z + 96) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[96], *(__m128i*)&Z[96]), *(__m128i*)&X[96]);
	*(__m128i*)(Z + 104) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[104], *(__m128i*)&Z[104]), *(__m128i*)&X[104]);
	*(__m128i*)(Z + 112) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[112], *(__m128i*)&Z[112]), *(__m128i*)&X[112]);
	*(__m128i*)(Z + 120) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[120], *(__m128i*)&Z[120]), *(__m128i*)&X[120]);
	*(__m128i*)(Z + 128) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[128], *(__m128i*)&Z[128]), *(__m128i*)&X[128]);
	*(__m128i*)(Z + 136) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[136], *(__m128i*)&Z[136]), *(__m128i*)&X[136]);
	*(__m128i*)(Z + 144) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[144], *(__m128i*)&Z[144]), *(__m128i*)&X[144]);
	*(__m128i*)(Z + 152) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[152], *(__m128i*)&Z[152]), *(__m128i*)&X[152]);
	*(__m128i*)(Z + 160) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[160], *(__m128i*)&Z[160]), *(__m128i*)&X[160]);
	*(__m128i*)(Z + 168) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[168], *(__m128i*)&Z[168]), *(__m128i*)&X[168]);
	*(__m128i*)(Z + 176) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[176], *(__m128i*)&Z[176]), *(__m128i*)&X[176]);
	*(__m128i*)(Z + 184) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[184], *(__m128i*)&Z[184]), *(__m128i*)&X[184]);
	*(__m128i*)(Z + 192) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[192], *(__m128i*)&Z[192]), *(__m128i*)&X[192]);
	*(__m128i*)(Z + 200) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[200], *(__m128i*)&Z[200]), *(__m128i*)&X[200]);
	*(__m128i*)(Z + 208) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[208], *(__m128i*)&Z[208]), *(__m128i*)&X[208]);
	*(__m128i*)(Z + 216) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[216], *(__m128i*)&Z[216]), *(__m128i*)&X[216]);
	*(__m128i*)(Z + 224) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[224], *(__m128i*)&Z[224]), *(__m128i*)&X[224]);
	*(__m128i*)(Z + 232) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[232], *(__m128i*)&Z[232]), *(__m128i*)&X[232]);
	*(__m128i*)(Z + 240) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[240], *(__m128i*)&Z[240]), *(__m128i*)&X[240]);
	*(__m128i*)(Z + 248) = _mm_sub_epi16(_mm_add_epi16(*(__m128i*)&Y[248], *(__m128i*)&Z[248]), *(__m128i*)&X[248]);
}

//函数17: 拷贝Alpha通道的数据
//参考: 无
//简介: 直接用原始的代码，速度很好
void CopyAlphaChannel(TMatrix *Src, TMatrix *Dest) {
	if (Src->Channel != 4 || Dest->Channel != 4) return;
	if (Src->Data == Dest->Data) return;
	unsigned char *SrcP = Src->Data, *DestP = Dest->Data;
	int Y, Index = 3;
	for (Y = 0; Y < Src->Width * Src->Height; Y++, Index += 4) {
		SrcP[Index] = DestP[Index];
	}
}

// 函数18: 按照指定的边缘模式计算扩展后各坐标的有理值
// 参数列表: 
// Width: 矩阵的宽度
// Height: 矩阵的高度
// Left: 左侧需要扩展的坐标
// Right: 右侧需要扩展的坐标
// Top: 顶部需要扩展的坐标
// Bottom: 底部需要扩展的坐标
// Edge: 处理边缘的方式
// RawPos: 保存行方向的有理坐标值
// ColPos: 保存列方向的有理坐标值
// 返回函数是执行成功
IS_RET GetValidCoordinate(int Width, int Height, int Left, int Right, int Top, int Bottom, EdgeMode Edge, TMatrix **Row, TMatrix **Col) {
	if((Left < 0) || (Right < 0) || (Top < 0) || (Bottom < 0)) return IS_RET_ERR_ARGUMENTOUTOFRANGE;
	IS_RET Ret = IS_CreateMatrix(Width + Left + Right, 1, IS_DEPTH_32S, 1, Row);
	if (Ret != IS_RET_OK) return Ret;
	Ret = IS_CreateMatrix(1, Height + Top + Bottom, IS_DEPTH_32S, 1, Col);
	if (Ret != IS_RET_OK) return Ret;
	int X, Y, XX, YY, *RowPos = (int *)(*Row)->Data, *ColPos = (int*)(*Col)->Data;
	for (X = -Left; X < Width + Right; X++) {
		if (X < 0) {
			if (Edge == EdgeMode::Tile) { //重复边缘像素
				RowPos[X + Left] = 0;
			}
			else {
				XX = -X;
				while (XX >= Width) XX -= Width;
				RowPos[X + Left] = XX;
			}
		}
		else if (X >= Width) {
			if (Edge == EdgeMode::Tile) {
				RowPos[X + Left] = 0;
			}
			else {
				XX = -X;
				while (XX >= Width) XX -= Width; //做镜像数据
				RowPos[X + Left] = XX;
			}
		}
		else {
			RowPos[X + Left] = X;
		}
	}
	for (Y = -Top; Y < Height + Bottom; Y++) {
		if (Y < 0) {
			if (Edge == EdgeMode::Tile)
				ColPos[Y + Top] = 0;
			else {
				YY = -Y;
				while (YY >= Height) YY -= Height;
				ColPos[Y + Top] = YY;
			}
		}
		else if (Y >= Height) {
			if (Edge == EdgeMode::Tile)
				ColPos[Y + Top] = Height - 1;
			else {
				YY = Height - (Y - Height + 2);
				while (YY < 0) YY += Height;
				ColPos[Y + Top] = YY;
			}
		}
		else {
			ColPos[Y + Top] = Y;
		}
	}
	return IS_RET_OK;
}

// 函数19: 将彩色图像分解为R、G、B、A单通道的图像
// 参数列表:
// Src: 需要处理的源图像的数据结构
// Blue: 保存Blue通道图像的数据结构
// Green: 保存Green通道图像的数据结构
// Red: 保存Red通道图像的数据结构
// Alpha: 保存Alpha通道图像的数据结构
// 采用了8位并行处理速度大概提升20%
// 返回函数是否执行成功
IS_RET SplitRGBA(TMatrix *Src, TMatrix **Blue, TMatrix **Green, TMatrix **Red, TMatrix **Alpha) {
	if (Src == NULL) return IS_RET_ERR_NULLREFERENCE;
	if (Src->Data == NULL) return IS_RET_ERR_NULLREFERENCE;
	if (Src->Depth != IS_DEPTH_8U) return IS_RET_ERR_NOTSUPPORTED;
	IS_RET Ret = IS_CreateMatrix(Src->Width, Src->Height, Src->Depth, 1, Blue);
	if (Ret != IS_RET_OK) goto Done;
	Ret = IS_CreateMatrix(Src->Width, Src->Height, Src->Depth, 1, Green);
	if (Ret != IS_RET_OK) goto Done;
	Ret = IS_CreateMatrix(Src->Width, Src->Height, Src->Depth, 1, Red);
	if (Ret != IS_RET_OK) goto Done;
	if (Src->Channel == 4) {
		Ret = IS_CreateMatrix(Src->Width, Src->Height, Src->Depth, 1, Alpha);
		if (Ret != IS_RET_OK) goto Done;
	}
	int X, Y, Block, Width = Src->Width, Height = Src->Height;
	unsigned char *LinePS, *LinePB, *LinePG, *LinePR, *LinePA;
	const int BlockSize = 8;
	Block = Width / BlockSize;						//	8路并行,再多开几路，速度并没有增加了
	if (Src->Channel == 3)
	{
		for (Y = 0; Y < Height; Y++)
		{
			LinePS = Src->Data + Y * Src->WidthStep;
			LinePB = (*Blue)->Data + Y * (*Blue)->WidthStep;
			LinePG = (*Green)->Data + Y * (*Green)->WidthStep;
			LinePR = (*Red)->Data + Y * (*Red)->WidthStep;
			for (X = 0; X < Block * BlockSize; X += BlockSize)			//	如果把LinePB全写在一起，速度反而慢一些
			{
				LinePB[0] = LinePS[0];		LinePG[0] = LinePS[1];		LinePR[0] = LinePS[2];
				LinePB[1] = LinePS[3];		LinePG[1] = LinePS[4];		LinePR[1] = LinePS[5];
				LinePB[2] = LinePS[6];		LinePG[2] = LinePS[7];		LinePR[2] = LinePS[8];
				LinePB[3] = LinePS[9];		LinePG[3] = LinePS[10];		LinePR[3] = LinePS[11];
				LinePB[4] = LinePS[12];		LinePG[4] = LinePS[13];		LinePR[4] = LinePS[14];
				LinePB[5] = LinePS[15];		LinePG[5] = LinePS[16];		LinePR[5] = LinePS[17];
				LinePB[6] = LinePS[18];		LinePG[6] = LinePS[19];		LinePR[6] = LinePS[20];
				LinePB[7] = LinePS[21];		LinePG[7] = LinePS[22];		LinePR[7] = LinePS[23];
				LinePB += 8;				LinePG += 8;				LinePR += 8;				LinePS += 24;
			}
			while (X < Width)
			{
				LinePB[0] = LinePS[0];		LinePG[0] = LinePS[1];		LinePR[0] = LinePS[2];
				LinePB++;					LinePG++;					LinePR++;					LinePS += 3;
				X++;
			}
		}
	}
	else if (Src->Channel == 4)
	{
		for (Y = 0; Y < Height; Y++)
		{
			LinePS = Src->Data + Y * Src->WidthStep;
			LinePB = (*Blue)->Data + Y * (*Blue)->WidthStep;
			LinePG = (*Green)->Data + Y * (*Green)->WidthStep;
			LinePR = (*Red)->Data + Y * (*Red)->WidthStep;
			LinePA = (*Alpha)->Data + Y * (*Alpha)->WidthStep;
			for (X = 0; X < Block * BlockSize; X += BlockSize)
			{
				LinePB[0] = LinePS[0];		LinePG[0] = LinePS[1];		LinePR[0] = LinePS[2];		LinePA[0] = LinePS[3];
				LinePB[1] = LinePS[4];		LinePG[1] = LinePS[5];		LinePR[1] = LinePS[6];		LinePA[1] = LinePS[7];
				LinePB[2] = LinePS[8];		LinePG[2] = LinePS[9];		LinePR[2] = LinePS[10];		LinePA[2] = LinePS[11];
				LinePB[3] = LinePS[12];		LinePG[3] = LinePS[13];		LinePR[3] = LinePS[14];		LinePA[3] = LinePS[15];
				LinePB[4] = LinePS[16];		LinePG[4] = LinePS[17];		LinePR[4] = LinePS[18];		LinePA[4] = LinePS[19];
				LinePB[5] = LinePS[20];		LinePG[5] = LinePS[21];		LinePR[5] = LinePS[22];		LinePA[5] = LinePS[23];
				LinePB[6] = LinePS[24];		LinePG[6] = LinePS[25];		LinePR[6] = LinePS[26];		LinePA[6] = LinePS[27];
				LinePB[7] = LinePS[28];		LinePG[7] = LinePS[29];		LinePR[7] = LinePS[30];		LinePA[7] = LinePS[31];
				LinePB += 8;				LinePG += 8;				LinePR += 8;				LinePA += 8;				LinePS += 32;
			}
			while (X < Width)
			{
				LinePB[0] = LinePS[0];		LinePG[0] = LinePS[1];		LinePR[0] = LinePS[2];		LinePA[0] = LinePS[3];
				LinePB++;					LinePG++;					LinePR++;					LinePA++;					LinePS += 4;
				X++;
			}
		}
	}
	return IS_RET_OK;
Done:
	if (*Blue != NULL) IS_FreeMatrix(Blue);
	if (*Green != NULL) IS_FreeMatrix(Green);
	if (*Red != NULL) IS_FreeMatrix(Red);
	if (*Alpha != NULL) IS_FreeMatrix(Alpha);
	return Ret;
}

// 函数20: 将R,G,B,A单通道的图像合并为彩色的图像
// 参数列表:
// Dest: 合并处理后的图像的数据结构
// Blue: Blue通道图像的数据结构
// Green: Green通道图像的数据结构
// Red: Red通道图像的数据结构
// Alpha: Alpha通道图像的数据结构
IS_RET CombineRGBA(TMatrix *Dest, TMatrix *Blue, TMatrix *Green, TMatrix *Red, TMatrix *Alpha)
{
	if (Dest == NULL || Blue == NULL || Green == NULL || Red == NULL) return IS_RET_ERR_NULLREFERENCE;
	if (Dest->Data == NULL || Blue->Data == NULL || Green->Data == NULL || Red->Data == NULL) return IS_RET_ERR_NULLREFERENCE;
	if ((Dest->Channel != 3 && Dest->Channel != 4) || Blue->Channel != 1 || Green->Channel != 1 || Red->Channel != 1) return IS_RET_ERR_PARAMISMATCH;
	if (Dest->Width != Blue->Width || Dest->Width != Green->Width || Dest->Width != Red->Width || Dest->Width != Blue->Width)  return IS_RET_ERR_PARAMISMATCH;
	if (Dest->Height != Blue->Height || Dest->Height != Green->Height || Dest->Height != Red->Height || Dest->Height != Blue->Height)  return IS_RET_ERR_PARAMISMATCH;

	if (Dest->Channel == 4)
	{
		if (Alpha == NULL) return IS_RET_ERR_NULLREFERENCE;
		if (Alpha->Data == NULL) return IS_RET_ERR_NULLREFERENCE;
		if (Alpha->Channel != 1) return IS_RET_ERR_PARAMISMATCH;
		if (Dest->Width != Alpha->Width || Dest->Height != Alpha->Height) return IS_RET_ERR_PARAMISMATCH;
	}

	int X, Y, Block, Width = Dest->Width, Height = Dest->Height;
	unsigned char *LinePD, *LinePB, *LinePG, *LinePR, *LinePA;
	const int BlockSize = 8;
	Block = Width / BlockSize;						//	8路并行,再多开几路，速度并没有增加了

	if (Dest->Channel == 3)
	{
		for (Y = 0; Y < Height; Y++)
		{
			LinePD = Dest->Data + Y * Dest->WidthStep;
			LinePB = Blue->Data + Y * Blue->WidthStep;
			LinePG = Green->Data + Y * Green->WidthStep;
			LinePR = Red->Data + Y * Red->WidthStep;
			for (X = 0; X < Block * BlockSize; X += BlockSize)				//	如果把LinePB全写在一起，速度区别不大
			{
				LinePD[0] = LinePB[0];		LinePD[1] = LinePG[0];		LinePD[2] = LinePR[0];
				LinePD[3] = LinePB[1];		LinePD[4] = LinePG[1];		LinePD[5] = LinePR[1];
				LinePD[6] = LinePB[2];		LinePD[7] = LinePG[2];		LinePD[8] = LinePR[2];
				LinePD[9] = LinePB[3];		LinePD[10] = LinePG[3];		LinePD[11] = LinePR[3];
				LinePD[12] = LinePB[4];		LinePD[13] = LinePG[4];		LinePD[14] = LinePR[4];
				LinePD[15] = LinePB[5];		LinePD[16] = LinePG[5];		LinePD[17] = LinePR[5];
				LinePD[18] = LinePB[6];		LinePD[19] = LinePG[6];		LinePD[20] = LinePR[6];
				LinePD[21] = LinePB[7];		LinePD[22] = LinePG[7];		LinePD[23] = LinePR[7];
				LinePB += 8;				LinePG += 8;				LinePR += 8;				LinePD += 24;
			}
			while (X < Width)
			{
				LinePD[0] = LinePB[0];		LinePD[1] = LinePG[0];		LinePD[2] = LinePR[0];
				LinePB++;					LinePG++;					LinePR++;					LinePD += 3;
				X++;
			}
		}
	}
	else if (Dest->Channel == 4)
	{
		for (Y = 0; Y < Height; Y++)
		{
			LinePD = Dest->Data + Y * Dest->WidthStep;
			LinePB = Blue->Data + Y * Blue->WidthStep;
			LinePG = Green->Data + Y * Green->WidthStep;
			LinePR = Red->Data + Y * Red->WidthStep;
			LinePA = Alpha->Data + Y * Alpha->WidthStep;
			for (X = 0; X < Block * BlockSize; X += BlockSize)
			{
				LinePD[0] = LinePB[0];		LinePD[1] = LinePG[0];		LinePD[2] = LinePR[0];		LinePD[3] = LinePA[0];
				LinePD[4] = LinePB[1];		LinePD[5] = LinePG[1];		LinePD[6] = LinePR[1];		LinePD[7] = LinePA[1];
				LinePD[8] = LinePB[2];		LinePD[9] = LinePG[2];		LinePD[10] = LinePR[2];		LinePD[11] = LinePA[2];
				LinePD[12] = LinePB[3];		LinePD[13] = LinePG[3];		LinePD[14] = LinePR[3];		LinePD[15] = LinePA[3];
				LinePD[16] = LinePB[4];		LinePD[17] = LinePG[4];		LinePD[18] = LinePR[4];		LinePD[19] = LinePA[4];
				LinePD[20] = LinePB[5];		LinePD[21] = LinePG[5];		LinePD[22] = LinePR[5];		LinePD[23] = LinePA[5];
				LinePD[24] = LinePB[6];		LinePD[25] = LinePG[6];		LinePD[26] = LinePR[6];		LinePD[27] = LinePA[6];
				LinePD[28] = LinePB[7];		LinePD[29] = LinePG[7];		LinePD[30] = LinePR[7];		LinePD[31] = LinePA[7];
				LinePB += 8;				LinePG += 8;				LinePR += 8;				LinePA += 8;				LinePD += 32;
			}
			while (X < Width)
			{
				LinePD[0] = LinePB[0];		LinePD[1] = LinePG[0];		LinePD[2] = LinePR[0];		LinePD[3] = LinePA[0];
				LinePB++;					LinePG++;					LinePD++;					LinePA++;					LinePD += 4;
				X++;
			}
		}
	}
	return IS_RET_OK;
}