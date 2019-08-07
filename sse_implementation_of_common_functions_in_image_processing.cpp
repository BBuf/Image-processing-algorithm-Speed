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



int main() {
	
}