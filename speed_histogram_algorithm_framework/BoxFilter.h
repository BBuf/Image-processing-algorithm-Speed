#pragma once
#include "Core.h"
#include "Utility.h"

// 函数功能: 实现图像方框模糊效果
// 参数列表:
// Src: 需要处理的源图像的数据结构
// Dest: 保存处理后的图像的数据结构
// Radius: 方框模糊的半径，有效范围[1, 1000]
// EdgeBehavior: 边缘处数据的处理方法，0表示重复边缘像素，1使用镜像的方式对边缘像素求均值
// 补充:
// 1. 能处理8位灰度和24位图像
// 2. Src和Dest可以相同，在相同时速度会稍慢
// 3. SSE优化版本在初始化时和半径是有关的，所以在半径增大时，耗时会略微增加

IS_RET BoxBlur(TMatrix *Src, TMatrix *Dest, int Radius, EdgeMode Edge) {
	if (Src == NULL || Dest == NULL) return IS_RET_ERR_NULLREFERENCE;
	if (Src->Data == NULL || Dest->Data == NULL) return IS_RET_ERR_NULLREFERENCE;
	if (Src->Width != Dest->Width || Src->Height != Dest->Height || Src->Channel != Dest->Channel || Src->Depth != Dest->Depth || Src->WidthStep != Dest->WidthStep) return IS_RET_ERR_PARAMISMATCH;
	if (Src->Depth != IS_DEPTH_8U || Dest->Depth != IS_DEPTH_8U) return IS_RET_ERR_NOTSUPPORTED;
	IS_RET Ret = IS_RET_OK;
	TMatrix *Row = NULL, *Col = NULL;
	int *RowPos, *ColPos, *ColSum, *Diff;
	int X, Y, Z, Width, Height, Channel, Index;
	int Value, ValueB, ValueG, ValueR;
	int Size = 2 * Radius + 1, Amount = Size * Size, HalfAmount = Amount / 2;
	Width = Src->Width;
	Height = Src->Height;
	Channel = Src->Channel;
	Ret = GetValidCoordinate(Width, Height, Radius, Radius, Radius, Radius, EdgeMode::Smear, &Row, &Col);		//	获取坐标偏移量
	RowPos = ((int *)Row->Data);
	ColPos = ((int *)Col->Data);		   
	ColSum = (int *)IS_AllocMemory(Width * Channel * sizeof(int), true);
	Diff = (int *)IS_AllocMemory((Width - 1) * Channel * sizeof(int), true);
	unsigned char *RowData = (unsigned char *)IS_AllocMemory((Width + 2 * Radius) * Channel, true);
	TMatrix Sum;
	TMatrix *p = &Sum;
	TMatrix **q = &p;
	IS_CreateMatrix(Width, Height, IS_DEPTH_32S, Channel, q);
	for (Y = 0; Y < Height; Y++) {
		unsigned char *LinePS = Src->Data + Y * Src->WidthStep;
		int *LinePD = (int *)(p->Data + Y * p->WidthStep);
		//	拷贝一行数据及边缘部分部分到临时的缓冲区中
		if (Channel == 1)
		{
			for (X = 0; X < Radius; X++)
				RowData[X] = LinePS[RowPos[X]];
			memcpy(RowData + Radius, LinePS, Width);
			for (X = Radius + Width; X < Radius + Width + Radius; X++)
				RowData[X] = LinePS[RowPos[X]];
		}
		else if (Channel == 3)
		{
			for (X = 0; X < Radius; X++)
			{
				Index = RowPos[X] * 3;
				RowData[X * 3] = LinePS[Index];
				RowData[X * 3 + 1] = LinePS[Index + 1];
				RowData[X * 3 + 2] = LinePS[Index + 2];
			}
			memcpy(RowData + Radius * 3, LinePS, Width * 3);
			for (X = Radius + Width; X < Radius + Width + Radius; X++)
			{
				Index = RowPos[X] * 3;
				RowData[X * 3 + 0] = LinePS[Index + 0];
				RowData[X * 3 + 1] = LinePS[Index + 1];
				RowData[X * 3 + 2] = LinePS[Index + 2];
			}
		}
		unsigned char *AddPos = RowData + Size * Channel;
		unsigned char *SubPos = RowData;
		for (X = 0; X < (Width - 1) * Channel; X++)
			Diff[X] = AddPos[X] - SubPos[X];
		//	第一个点要特殊处理
		if (Channel == 1)
		{
			for (Z = 0, Value = 0; Z < Size; Z++)	Value += RowData[Z];
			LinePD[0] = Value;

			for (X = 1; X < Width; X++)
			{
				Value += Diff[X - 1];	LinePD[X] = Value;				//	分四路并行速度又能提高很多
			}
		}
		else if (Channel == 3)
		{
			for (Z = 0, ValueB = ValueG = ValueR = 0; Z < Size; Z++)
			{
				ValueB += RowData[Z * 3 + 0];
				ValueG += RowData[Z * 3 + 1];
				ValueR += RowData[Z * 3 + 2];
			}
			LinePD[0] = ValueB;	LinePD[1] = ValueG;	LinePD[2] = ValueR;

			for (X = 1; X < Width; X++)
			{
				Index = X * 3;
				ValueB += Diff[Index - 3];		LinePD[Index + 0] = ValueB;
				ValueG += Diff[Index - 2];		LinePD[Index + 1] = ValueG;
				ValueR += Diff[Index - 1];		LinePD[Index + 2] = ValueR;
			}
		}
	}
	for (Y = 0; Y < Size - 1; Y++)			//	注意没有最后一项哦						
	{
		int *LinePS = (int *)(p->Data + ColPos[Y] * p->WidthStep);
		for (X = 0; X < Width * Channel; X++)	ColSum[X] += LinePS[X];
	}

	for (Y = 0; Y < Height; Y++)
	{
		unsigned char* LinePD = Dest->Data + Y * Dest->WidthStep;
		int *AddPos = (int*)(p->Data + ColPos[Y + Size - 1] * p->WidthStep);
		int *SubPos = (int*)(p->Data + ColPos[Y] * p->WidthStep);

		for (X = 0; X < Width * Channel; X++)
		{
			Value = ColSum[X] + AddPos[X];
			LinePD[X] = (Value + HalfAmount) / Amount;					//		+  HalfAmount 主要是为了四舍五入
			ColSum[X] = Value - SubPos[X];
		}
	}
	IS_FreeMemory(RowPos);
	IS_FreeMemory(ColPos);
	IS_FreeMemory(Diff);
	IS_FreeMemory(ColSum);
	IS_FreeMemory(RowData);
	return Ret;
}

// 函数功能: 实现图像方框模糊效果（SSE优化）

IS_RET BoxBlur_SSE(TMatrix *Src, TMatrix *Dest, int Radius, EdgeMode Edge) {
	if (Src == NULL || Dest == NULL) return IS_RET_ERR_NULLREFERENCE;
	if (Src->Data == NULL || Dest->Data == NULL) return IS_RET_ERR_NULLREFERENCE;
	if (Src->Width != Dest->Width || Src->Height != Dest->Height || Src->Channel != Dest->Channel || Src->Depth != Dest->Depth || Src->WidthStep != Dest->WidthStep) return IS_RET_ERR_PARAMISMATCH;
	if (Src->Depth != IS_DEPTH_8U || Dest->Depth != IS_DEPTH_8U) return IS_RET_ERR_NOTSUPPORTED;
	IS_RET Ret = IS_RET_OK;
	TMatrix *Row = NULL, *Col = NULL;
	int *RowPos, *ColPos, *ColSum, *Diff;
	int X, Y, Z, Width, Height, Channel, Index;
	int Value, ValueB, ValueG, ValueR;
	int Size = 2 * Radius + 1, Amount = Size * Size, HalfAmount = Amount / 2;
	float Scale = 1.0 / (Size * Size);
	Width = Src->Width;
	Height = Src->Height;
	Channel = Src->Channel;
	Ret = GetValidCoordinate(Width, Height, Radius, Radius, Radius, Radius, EdgeMode::Smear, &Row, &Col);		//	获取坐标偏移量
	RowPos = ((int *)Row->Data);
	ColPos = ((int *)Col->Data);
	ColSum = (int *)IS_AllocMemory(Width * Channel * sizeof(int), true);
	Diff = (int *)IS_AllocMemory((Width - 1) * Channel * sizeof(int), true);
	unsigned char *RowData = (unsigned char *)IS_AllocMemory((Width + 2 * Radius) * Channel, true);
	TMatrix Sum;
	TMatrix *p = &Sum;
	TMatrix **q = &p;
	IS_CreateMatrix(Width, Height, IS_DEPTH_32S, Channel, q);
	for (Y = 0; Y < Height; Y++) {
		unsigned char *LinePS = Src->Data + Y * Src->WidthStep;
		int *LinePD = (int *)(p->Data + Y * p->WidthStep);
		//	拷贝一行数据及边缘部分部分到临时的缓冲区中
		if (Channel == 1)
		{
			for (X = 0; X < Radius; X++)
				RowData[X] = LinePS[RowPos[X]];
			memcpy(RowData + Radius, LinePS, Width);
			for (X = Radius + Width; X < Radius + Width + Radius; X++)
				RowData[X] = LinePS[RowPos[X]];
		}
		else if (Channel == 3)
		{
			for (X = 0; X < Radius; X++)
			{
				Index = RowPos[X] * 3;
				RowData[X * 3] = LinePS[Index];
				RowData[X * 3 + 1] = LinePS[Index + 1];
				RowData[X * 3 + 2] = LinePS[Index + 2];
			}
			memcpy(RowData + Radius * 3, LinePS, Width * 3);
			for (X = Radius + Width; X < Radius + Width + Radius; X++)
			{
				Index = RowPos[X] * 3;
				RowData[X * 3 + 0] = LinePS[Index + 0];
				RowData[X * 3 + 1] = LinePS[Index + 1];
				RowData[X * 3 + 2] = LinePS[Index + 2];
			}
		}
		unsigned char *AddPos = RowData + Size * Channel;
		unsigned char *SubPos = RowData;
		X = 0;
		__m128i Zero = _mm_setzero_si128();
		for (; X <= (Width - 1) * Channel - 8; X += 8) {
			__m128i Add = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i const *)(AddPos + X)), Zero);
			__m128i Sub = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i const *)(SubPos + X)), Zero);
			_mm_store_si128((__m128i *)(Diff + X + 0), _mm_sub_epi32(_mm_unpacklo_epi16(Add, Zero), _mm_unpacklo_epi16(Sub, Zero)));
			_mm_store_si128((__m128i *)(Diff + X + 4), _mm_sub_epi32(_mm_unpackhi_epi16(Add, Zero), _mm_unpackhi_epi16(Sub, Zero)));
		}
		for (; X < (Width - 1) * Channel; X++)
			Diff[X] = AddPos[X] - SubPos[X];
		// 第一个点要特殊处理
		//	第一个点要特殊处理
		if (Channel == 1)
		{
			for (Z = 0, Value = 0; Z < Size; Z++)	Value += RowData[Z];
			LinePD[0] = Value;

			for (X = 1; X < Width; X++)
			{
				Value += Diff[X - 1];
				LinePD[X] = Value;
			}
		}
		else if (Channel == 3)
		{
			for (Z = 0, ValueB = ValueG = ValueR = 0; Z < Size; Z++)
			{
				ValueB += RowData[Z * 3 + 0];
				ValueG += RowData[Z * 3 + 1];
				ValueR += RowData[Z * 3 + 2];
			}
			LinePD[0] = ValueB;	LinePD[1] = ValueG;	LinePD[2] = ValueR;

			for (X = 1; X < Width; X++)
			{
				Index = X * 3;
				ValueB += Diff[Index - 3];		LinePD[Index + 0] = ValueB;
				ValueG += Diff[Index - 2];		LinePD[Index + 1] = ValueG;
				ValueR += Diff[Index - 1];		LinePD[Index + 2] = ValueR;
			}
		}
	}

	for (Y = 0; Y < Size - 1; Y++) {
		X = 0;
		int *LinePS = (int *)(p->Data + ColPos[Y] * p->WidthStep);
		for (; X <= Width * Channel - 4; X += 4) {
			__m128i SumP = _mm_load_si128((const __m128i*)(ColSum + X));
			__m128i SrcP = _mm_load_si128((const __m128i*)(LinePS + X));
			_mm_store_si128((__m128i *)(ColSum + X), _mm_add_epi32(SumP, SrcP));
		}
		for (; X < Width * Channel; X++) ColSum[X] += LinePS[X];
	}

	for (Y = 0; Y < Height; Y++) {
		unsigned char *LinePD = Dest->Data + Y * Dest->WidthStep;
		int *AddPos = (int*)(p->Data + ColPos[Y + Size - 1] * p->WidthStep);
		int *SubPos = (int*)(p->Data + ColPos[Y] * p->WidthStep);
		X = 0;
		const __m128 Inv = _mm_set1_ps(Scale);
		for (; X <= Width * Channel - 8; X += 8) {
			__m128i Sub1 = _mm_loadu_si128((const __m128i*)(SubPos + X + 0));
			__m128i Sub2 = _mm_loadu_si128((const __m128i*)(SubPos + X + 4));
			__m128i Add1 = _mm_loadu_si128((const __m128i*)(AddPos + X + 0));
			__m128i Add2 = _mm_loadu_si128((const __m128i*)(AddPos + X + 4));
			__m128i Col1 = _mm_load_si128((const __m128i*)(ColSum + X + 0));
			__m128i Col2 = _mm_load_si128((const __m128i*)(ColSum + X + 4));

			__m128i Sum1 = _mm_add_epi32(Col1, Add1);
			__m128i Sum2 = _mm_add_epi32(Col2, Add2);

			__m128i Dest1 = _mm_cvtps_epi32(_mm_mul_ps(Inv, _mm_cvtepi32_ps(Sum1)));
			__m128i Dest2 = _mm_cvtps_epi32(_mm_mul_ps(Inv, _mm_cvtepi32_ps(Sum2)));

			Dest1 = _mm_packs_epi32(Dest1, Dest2);
			_mm_storel_epi64((__m128i *)(LinePD + X), _mm_packus_epi16(Dest1, Dest1));

			_mm_store_si128((__m128i *)(ColSum + X + 0), _mm_sub_epi32(Sum1, Sub1));
			_mm_store_si128((__m128i *)(ColSum + X + 4), _mm_sub_epi32(Sum2, Sub2));
		}
		for (; X < Width * Channel; X++){
			Value = ColSum[X] + AddPos[X];
			LinePD[X] = Value * Scale;
			ColSum[X] = Value - SubPos[X];
		}
	}
	IS_FreeMemory(RowPos);
	IS_FreeMemory(ColPos);
	IS_FreeMemory(Diff);
	IS_FreeMemory(ColSum);
	IS_FreeMemory(RowData);
	return Ret;
}