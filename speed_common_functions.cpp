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