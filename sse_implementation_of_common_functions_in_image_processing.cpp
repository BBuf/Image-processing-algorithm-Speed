#include <stdio.h>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

// 函数1: 对数函数的SSE实现，高精度版
inline __m128 _mm_log_ps(__m128 x)
{
	static const __declspec(align(16)) int _ps_min_norm_pos[4] = { 0x00800000, 0x00800000, 0x00800000, 0x00800000 };
	static const __declspec(align(16)) int _ps_inv_mant_mask[4] = { ~0x7f800000, ~0x7f800000, ~0x7f800000, ~0x7f800000 };
	static const __declspec(align(16)) int _pi32_0x7f[4] = { 0x7f, 0x7f, 0x7f, 0x7f };
	static const __declspec(align(16)) float _ps_1[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static const __declspec(align(16)) float _ps_0p5[4] = { 0.5f, 0.5f, 0.5f, 0.5f };
	static const __declspec(align(16)) float _ps_sqrthf[4] = { 0.707106781186547524f, 0.707106781186547524f, 0.707106781186547524f, 0.707106781186547524f };
	static const __declspec(align(16)) float _ps_log_p0[4] = { 7.0376836292E-2f, 7.0376836292E-2f, 7.0376836292E-2f, 7.0376836292E-2f };
	static const __declspec(align(16)) float _ps_log_p1[4] = { -1.1514610310E-1f, -1.1514610310E-1f, -1.1514610310E-1f, -1.1514610310E-1f };
	static const __declspec(align(16)) float _ps_log_p2[4] = { 1.1676998740E-1f, 1.1676998740E-1f, 1.1676998740E-1f, 1.1676998740E-1f };
	static const __declspec(align(16)) float _ps_log_p3[4] = { -1.2420140846E-1f, -1.2420140846E-1f, -1.2420140846E-1f, -1.2420140846E-1f };
	static const __declspec(align(16)) float _ps_log_p4[4] = { 1.4249322787E-1f, 1.4249322787E-1f, 1.4249322787E-1f, 1.4249322787E-1f };
	static const __declspec(align(16)) float _ps_log_p5[4] = { -1.6668057665E-1f, -1.6668057665E-1f, -1.6668057665E-1f, -1.6668057665E-1f };
	static const __declspec(align(16)) float _ps_log_p6[4] = { 2.0000714765E-1f, 2.0000714765E-1f, 2.0000714765E-1f, 2.0000714765E-1f };
	static const __declspec(align(16)) float _ps_log_p7[4] = { -2.4999993993E-1f, -2.4999993993E-1f, -2.4999993993E-1f, -2.4999993993E-1f };
	static const __declspec(align(16)) float _ps_log_p8[4] = { 3.3333331174E-1f, 3.3333331174E-1f, 3.3333331174E-1f, 3.3333331174E-1f };
	static const __declspec(align(16)) float _ps_log_q1[4] = { -2.12194440e-4f, -2.12194440e-4f, -2.12194440e-4f, -2.12194440e-4f };
	static const __declspec(align(16)) float _ps_log_q2[4] = { 0.693359375f, 0.693359375f, 0.693359375f, 0.693359375f };

	__m128 one = *(__m128*)_ps_1;
	__m128 invalid_mask = _mm_cmple_ps(x, _mm_setzero_ps());
	/* cut off denormalized stuff */
	x = _mm_max_ps(x, *(__m128*)_ps_min_norm_pos);
	__m128i emm0 = _mm_srli_epi32(_mm_castps_si128(x), 23);

	/* keep only the fractional part */
	x = _mm_and_ps(x, *(__m128*)_ps_inv_mant_mask);
	x = _mm_or_ps(x, _mm_set1_ps(0.5f));

	emm0 = _mm_sub_epi32(emm0, *(__m128i *)_pi32_0x7f);
	__m128 e = _mm_cvtepi32_ps(emm0);
	e = _mm_add_ps(e, one);

	__m128 mask = _mm_cmplt_ps(x, *(__m128*)_ps_sqrthf);
	__m128 tmp = _mm_and_ps(x, mask);
	x = _mm_sub_ps(x, one);
	e = _mm_sub_ps(e, _mm_and_ps(one, mask));
	x = _mm_add_ps(x, tmp);

	__m128 z = _mm_mul_ps(x, x);
	__m128 y = *(__m128*)_ps_log_p0;
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_log_p1);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_log_p2);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_log_p3);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_log_p4);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_log_p5);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_log_p6);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_log_p7);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_log_p8);
	y = _mm_mul_ps(y, x);

	y = _mm_mul_ps(y, z);
	tmp = _mm_mul_ps(e, *(__m128*)_ps_log_q1);
	y = _mm_add_ps(y, tmp);
	tmp = _mm_mul_ps(z, *(__m128*)_ps_0p5);
	y = _mm_sub_ps(y, tmp);
	tmp = _mm_mul_ps(e, *(__m128*)_ps_log_q2);
	x = _mm_add_ps(x, y);
	x = _mm_add_ps(x, tmp);
	x = _mm_or_ps(x, invalid_mask); // negative arg will be NAN

	return x;
}

// 函数2: 低精度的log函数，大概有小数点后2位的精度
// 算法来源: https://stackoverflow.com/questions/9411823/fast-log2float-x-implementation-c
inline float IM_Flog(float val)
{
	union
	{
		float val;
		int x;
	} u = { val };
	float log_2 = (float)(((u.x >> 23) & 255) - 128);
	u.x &= ~(255 << 23);
	u.x += (127 << 23);
	log_2 += ((-0.34484843f) * u.val + 2.02466578f) * u.val - 0.67487759f;
	return log_2 * 0.69314718f;
}

// 函数3: 函数2的SSE实现
inline __m128 _mm_flog_ps(__m128 x)
{
	__m128i I = _mm_castps_si128(x);
	__m128 log_2 = _mm_cvtepi32_ps(_mm_sub_epi32(_mm_and_si128(_mm_srli_epi32(I, 23), _mm_set1_epi32(255)), _mm_set1_epi32(128)));
	I = _mm_and_si128(I, _mm_set1_epi32(-2139095041));        //    255 << 23
	I = _mm_add_epi32(I, _mm_set1_epi32(1065353216));        //    127 << 23
	__m128 F = _mm_castsi128_ps(I);
	__m128 T = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(-0.34484843f), F), _mm_set1_ps(2.02466578f));
	T = _mm_sub_ps(_mm_mul_ps(T, F), _mm_set1_ps(0.67487759f));
	return _mm_mul_ps(_mm_add_ps(log_2, T), _mm_set1_ps(0.69314718f));
}

// 函数4: e^x的近似计算
inline float IM_Fexp(float Y)
{
	union
	{
		double Value;
		int X[2];
	} V;
	V.X[1] = (int)(Y * 1512775 + 1072632447 + 0.5F);
	V.X[0] = 0;
	return (float)V.Value;
}

// 函数5: 函数4的SSE实现
inline __m128 _mm_fexp_ps(__m128 Y)
{
	__m128i T = _mm_cvtps_epi32(_mm_add_ps(_mm_mul_ps(Y, _mm_set1_ps(1512775)), _mm_set1_ps(1072632447)));
	__m128i TL = _mm_unpacklo_epi32(_mm_setzero_si128(), T);
	__m128i TH = _mm_unpackhi_epi32(_mm_setzero_si128(), T);
	return _mm_movelh_ps(_mm_cvtpd_ps(_mm_castsi128_pd(TL)), _mm_cvtpd_ps(_mm_castsi128_pd(TH)));
}

//函数6: pow函数的近似实现
inline float IM_Fpow(float a, float b)
{
	union
	{
		double Value;
		int X[2];
	} V;
	V.X[1] = (int)(b * (V.X[1] - 1072632447) + 1072632447);
	V.X[0] = 0;
	return (float)V.Value;
}

// 函数7: 通过_mm_rcp_ps，_mm_rsqrt_ps（求导数的近似值，大概为小数点后12bit），结合牛顿迭代法，求精度更高的导数
__m128 _mm_prcp_ps(__m128 a) {
	__m128 rcp = _mm_rcp_ps(a); //此函数只有12bit的精度
	return _mm_sub_ps(_mm_add_ps(rcp, rcp), _mm_mul_ps(a, _mm_mul_ps(rcp, rcp))); //x1 = x0 * (2 - d * x0) = 2 * x0 - d * x0 * x0，使用牛顿 - 拉弗森方法这种方法可以提高精度到23bit
}

// 函数8: 直接用导数实现a / b
__m128 _mm_fdiv_ps(__m128 a, __m128 b)
{
	return _mm_mul_ps(a, _mm_rcp_ps(b));
}

// 函数9: 避免除数为0时无法获得效果
// 在SSE指令中，没有提供整数的除法指令，不知道这是为什么，所以整数除法一般只能借用浮点版本的指令。
// 同时，除法存在的一个问题就是如果除数为0，可能会触发异常，不过SSE在这种情况下不会抛出异常，但是我们应该避免。
// 避免的方式有很多，比如判断如果除数为0，就做特殊处理，或者如果除数为0就除以一个很小的数，不过大部分的需求是，
// 除数为0，则返回0，此时就可以使用下面的SSE指令代替_mm_div_ps
//四个浮点数的除法a/b，如果b中某个分量为0，则对应位置返回0值

inline __m128 _mm_divz_ps(__m128 a, __m128 b)
{
	__m128 Mask = _mm_cmpeq_ps(b, _mm_setzero_ps());
	return _mm_blendv_ps(_mm_div_ps(a, b), _mm_setzero_ps(), Mask);
}

// 函数10: 将4个32位整数转换为字节数并保存
// 将4个32位整形变量数据打包到4个字节数据中

inline void _mm_storesi128_4char(unsigned char *Dest, __m128i P)
{
	__m128i T = _mm_packs_epi32(P, P);
	*((int *)Dest) = _mm_cvtsi128_si32(_mm_packus_epi16(T, T));
}

// 函数11: 读取12个字节数到一个XMM寄存器中
// XMM寄存器是16个字节大小的，而且SSE的很多计算是以4的整数倍字节位单位进行的，
// 但是在图像处理中，70%情况下处理的是彩色的24位图像，即一个像素占用3个字节，
// 如果直接使用load指令载入数据，一次性可载入5加1 / 3个像素，这对算法的处理是很不方便的，
// 一般状况下都是加载4个像素，即12个字节，然后扩展成16个字节（给每个像素增加一个Alpha值），
// 我们当然可以直接使用load加载16个字节，然后每次跳过12个字节在进行load加载，但是其实也可以
// 使用下面的加载12个字节的函数：
// 从指针p处加载12个字节数据到XMM寄存器中，寄存器最高32位清0

inline __m128i _mm_loadu_epi96(const __m128i * p)
{
	return _mm_unpacklo_epi64(_mm_loadl_epi64(p), _mm_cvtsi32_si128(((int *)p)[2]));
}

// 函数12: 保存XMM的高12位
// 将寄存器Q的低位12个字节数据写入到指针P中。
inline void _mm_storeu_epi96(__m128i *P, __m128i Q)
{
	_mm_storel_epi64(P, Q);
	((int *)P)[2] = _mm_cvtsi128_si32(_mm_srli_si128(Q, 8));
}

// 函数13: 计算整数整除255的四舍五入结果。
inline int IM_Div255(int V)
{
	return (((V >> 8) + V + 1) >> 8);        //    似乎V可以是负数
}
 
// 函数14: 函数13的SSE实现
// 返回16位无符号整形数据整除255后四舍五入的结果： x = ((x + 1) + (x >> 8)) >> 8

inline __m128i _mm_div255_epu16(__m128i x)
{
	return _mm_srli_epi16(_mm_adds_epu16(_mm_adds_epu16(x, _mm_set1_epi16(1)), _mm_srli_epi16(x, 8)), 8);
}

// 函数15: 求XMM寄存器内所有元素的累加值
// 这也是个常见的需求，我们可能把某个结果重复的结果保存在寄存器中，最后结束时在把寄存器中的每个元素想加，
// 你当然可以通过访问__m128i变量的内部的元素实现，但是据说这样会降低循环内的优化，一种方式是直接用SSE指令实现，
// 比如对8个有符号的short类型的相加代码如下所示：
//    8个有符号的16位的数据相加的和。
//    https://stackoverflow.com/questions/31382209/computing-the-inner-product-of-vectors-with-allowed-scalar-values-0-1-and-2-usi/31382878#31382878

inline int _mm_hsum_epi16(__m128i V)                            //    V7 V6 V5 V4 V3 V2 V1 V0
{
	//    V = _mm_unpacklo_epi16(_mm_hadd_epi16(V, _mm_setzero_si128()), _mm_setzero_si128());    也可以用这句，_mm_hadd_epi16似乎对计算结果超出32768能获得正确结果
	__m128i T = _mm_madd_epi16(V, _mm_set1_epi16(1));   //    V7+V6                        V5+V4            V3+V2    V1+V0
	T = _mm_add_epi32(T, _mm_srli_si128(T, 8));            //    V7+V6+V3+V2                    V5+V4+V1+V0        0        0        
	T = _mm_add_epi32(T, _mm_srli_si128(T, 4));            //    V7+V6+V3+V2+V5+V4+V1+V0        V5+V4+V1+V0        0        0    
	return _mm_cvtsi128_si32(T);                        //    提取低位    
}

// 函数16: 求16个字节的最小值
// 比如我们要求一个字节序列的最小值，我们肯定会使用_mm_min_epi8这样的函数保存每隔16个字节的最小值，
// 这样最终我们得到16个字节的一个XMM寄存器，整个序列的最小值肯定在这个16个字节里面，
// 这个时候我们可以巧妙的借用下面的SSE语句得到这16个字节的最小值：
// 求16个字节数据的最小值, 只能针对字节数据。

inline int _mm_hmin_epu8(__m128i a)
{
	__m128i L = _mm_unpacklo_epi8(a, _mm_setzero_si128());
	__m128i H = _mm_unpackhi_epi8(a, _mm_setzero_si128());
	return _mm_extract_epi16(_mm_min_epu16(_mm_minpos_epu16(L), _mm_minpos_epu16(H)), 0);
}

// 函数17: 求16个字节的最大值
// 求16个字节数据的最大值, 只能针对字节数据。
inline int _mm_hmax_epu8(__m128i a)
{
	__m128i b = _mm_subs_epu8(_mm_set1_epi8(255), a);
	__m128i L = _mm_unpacklo_epi8(b, _mm_setzero_si128());
	__m128i H = _mm_unpackhi_epi8(b, _mm_setzero_si128());
	return 255 - _mm_extract_epi16(_mm_min_epu16(_mm_minpos_epu16(L), _mm_minpos_epu16(H)), 0);
}

int main() {

}
