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
	for (Y = 0; Y < Size - 1; Y++)			//	注意没有最后一项哦						//	这里的耗时只占整个的15%左右
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

