- _mm_cvtps_epi32 把四个float变量强转为四个int变量。其中需要注意的是他的截断规则：四舍五入，在进位后末位是偶数的进，否则不进位。

- _mm_cvttps_epi32 把四个float变量强转为四个int变量。直接截断，和c/c++中的r = (int)a一样。

- _mm_cvtpd_ps 将两个双精度， a 的浮点值设置为单精度的，浮点值。返回值:

  ```c++
  r0 := (float) a0
  r1 := (float) a1
  r2 := 0.0 ; r3 := 0.0
  ```

- _mm_movelh_ps 移动更低两个单精度， b 的浮点值到上面两个单精度，结果的浮点值。

  ```c++
  r3 := b1
  r2 := b0
  r1 := a1
  r0 := a0
  ```

- _mm_cmpneq_ps 比较两个单精度，如果对应位置的数相等返回0，不相等则返回1。

- _mm_blendv_ps 混和打包函数：

  ```c++
  __m128 _mm_blendv_ps( 
     __m128 a,
     __m128 b,
     __m128 mask 
  );
  
  r0 := (mask0 & 0x80000000) ? b0 : a0
  r1 := (mask1 & 0x80000000) ? b1 : a1
  r2 := (mask2 & 0x80000000) ? b2 : a2
  r3 := (mask3 & 0x80000000) ? b3 : a3
  ```

- _mm_packs_epi32 将a和b的8位有符号和32位整数转化位16位整型数据。

- _mm_cvtsi128_si32 移动最低有效位的32位a到32位整数。

- _mm_packus_epi16 将a和b的16位整数转化位8位无符号整型数据。

- _mm_cvtsi32_si128 将a的低32位赋值给一个32bits的整数，返回值为r=a0

- _mm_loadu_si128表示：Loads 128-bit value；即加载128位值。

- _mm_max_epu8 (a,b)表示：比较a和b中对应的无符号的8bits的整数，取其较大值，重复这个过程16次。即：r0=max(a0,b0),...,r15=max(a15,b15)。

- _mm_min_epi8(a,b)表示：大体意思同上，不同的是这次比较的是有符号的8bits的整数。

- _mm_setzero_si128表示：将128bits的值都赋值为0。

- _mm_subs_epu8(a,b)表示：a和b中对应的8bits数相减，r0= UnsignedSaturate(a0-b0)，...，r15= UnsignedSaturate(a15 - b15)。

- _mm_adds_epi8(a,b)表示：a和b中对应的8bits数相加，r0=SingedSaturate(a0+b0),...,r15=SingedSaturate(a15+b15)。

- _mm_unpackhi_epi64(a,b)表示：a和b的高64位交错，低64位舍去。

- _mm_srli_si128(a,imm)表示：将a进行逻辑右移imm位，高位填充0。

- _mm_cvtsi128_si32(a)表示：将a的低32位赋值给一个32bits的整数，返回值为r=a0。

- _mm_xor_si128(a,b)表示：将a和b进行按位异或，即r=a^b。

- _mm_or_si128(a,b)表示：将a和b进行或运算，即r=a|b。

- _mm_and_si128(a,b)表示：将a和b进行与运算，即r=a&b。

- _mm_cmpgt_epi8(a,b)表示：分别比较a的每个8bits整数是否大于b的对应位置的8bits整数，若大于，则返回0xffff，否则返回0x0。即r0=(a0>b0)?0xff:0x0  r1=(a1>b1)?0xff:0x0...r15=(a15>b15)?0xff:0x0

- _mm_unpacklo_epi64表示:  a和b的高64位交错，高64位舍去。

- _mm_madd_epi16 表示：返回一个__m128i的寄存器，它含有4个有符号的32位整数。

  ```c++
  r0 := (a0 * b0) + (a1 * b1)
  r1 := (a2 * b2) + (a3 * b3)
  r2 := (a4 * b4) + (a5 * b5)
  r3 := (a6 * b6) + (a7 * b7)
  ```

- _mm_extract_epi16(a, imm) 表示: 返回imm位置上的16位数。
- _mm_min_epu16 表示：两个数的最小者。
- _mm_minpos_epu16 表示：返回128 位值， 最低序的 16 位是参数找到的最小值a，第二个低的顺序 16 位是参数找到的最小值的索引a。