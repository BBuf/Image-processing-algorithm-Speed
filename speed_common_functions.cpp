//近似值
union Approximation
{
	double Value;
	int X[2];
};

// 函数1: 将数据截断在Byte数据类型内。
// 参考: http://www.cnblogs.com/zyl910/archive/2012/03/12/noifopex1.html
// 简介: 用位掩码做饱和处理，用带符号右移生成掩码。
unsigned char ClampToByte(int Value){
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

double PrecisePow(double X, double Y){
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
int Random(int Min, int Max){
	return rand() % (Max + 1 - Min) + Min;
}

//函数11: 符号函数
//参考: 无
//简介: 无
int sgn(int X){
	if (X > 0) return 1;
	if (X < 0) return -1;
	return 0;
}

//函数12: 获取某个整形变量对应的颜色值
//参考: 无
//简介: 无
void GetRGB(int Color, int *R, int *G, int *B){
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
