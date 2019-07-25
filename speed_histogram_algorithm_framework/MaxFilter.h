#pragma once
#include "Core.h"
#include "Utility.h"

// 函数供能: 在指定半径内，最大值”滤镜用周围像素的最高亮度值替换当前像素的亮度值。
// 参数列表:
// Src: 需要处理的源图像的数据结构
// Dest: 保存处理后的图像的数据结构
// Radius: 半径，有效范围
// 说明：
// 1、程序的执行时间和半径基本无关，但和图像内容有关
// 2、Src和Dest可以相同，不同时执行速度很快
// 3、对于各向异性的图像来说，执行速度很快，对于有大面积相同像素的图像，速度会慢一点

IS_RET  MaxFilter(TMatrix *Src, TMatrix *Dest, int Radius)
{
	if (Src == NULL || Dest == NULL) return IS_RET_ERR_NULLREFERENCE;
	if (Src->Data == NULL || Dest->Data == NULL) return IS_RET_ERR_NULLREFERENCE;
	if (Src->Width != Dest->Width || Src->Height != Dest->Height || Src->Channel != Dest->Channel || Src->Depth != Dest->Depth || Src->WidthStep != Dest->WidthStep) return IS_RET_ERR_PARAMISMATCH;
	if (Src->Depth != IS_DEPTH_8U || Dest->Depth != IS_DEPTH_8U) return IS_RET_ERR_NOTSUPPORTED;
	if (Radius < 0 || Radius >= 127) return IS_RET_ERR_ARGUMENTOUTOFRANGE;

	IS_RET Ret = IS_RET_OK;

	if (Src->Data == Dest->Data)
	{
		TMatrix *Clone = NULL;
		Ret = IS_CloneMatrix(Src, &Clone);
		if (Ret != IS_RET_OK) return Ret;
		Ret = MaxFilter(Clone, Dest, Radius);
		IS_FreeMatrix(&Clone);
		return Ret;
	}
	if (Src->Channel == 1)
	{
		TMatrix *Row = NULL, *Col = NULL;
		unsigned char *LinePS, *LinePD;
		int X, Y, K, Width = Src->Width, Height = Src->Height;
		int *RowOffset, *ColOffSet;

		unsigned short *ColHist = (unsigned short *)IS_AllocMemory(256 * (Width + 2 * Radius) * sizeof(unsigned short), true);
		if (ColHist == NULL) { Ret = IS_RET_ERR_OUTOFMEMORY; goto Done8; }
		unsigned short *Hist = (unsigned short *)IS_AllocMemory(256 * sizeof(unsigned short), true);
		if (Hist == NULL) { Ret = IS_RET_ERR_OUTOFMEMORY; goto Done8; }
		Ret = GetValidCoordinate(Width, Height, Radius, Radius, Radius, Radius, EdgeMode::Smear, &Row, &Col);		//	获取坐标偏移量
		if (Ret != IS_RET_OK) goto Done8;

		ColHist += Radius * 256;		RowOffset = ((int *)Row->Data) + Radius;
		ColOffSet = ((int *)Col->Data) + Radius;		    	//	进行偏移以便操作

		for (Y = 0; Y < Height; Y++)
		{
			if (Y == 0)											//	第一行的列直方图,要重头计算
			{
				for (K = -Radius; K <= Radius; K++)
				{
					LinePS = Src->Data + ColOffSet[K] * Src->WidthStep;
					for (X = -Radius; X < Width + Radius; X++)
					{
						ColHist[X * 256 + LinePS[RowOffset[X]]]++;
					}
				}
			}
			else												//	其他行的列直方图，更新就可以了
			{
				LinePS = Src->Data + ColOffSet[Y - Radius - 1] * Src->WidthStep;
				for (X = -Radius; X < Width + Radius; X++)		// 删除移出范围内的那一行的直方图数据
				{
					ColHist[X * 256 + LinePS[RowOffset[X]]]--;
				}

				LinePS = Src->Data + ColOffSet[Y + Radius] * Src->WidthStep;
				for (X = -Radius; X < Width + Radius; X++)		// 增加进入范围内的那一行的直方图数据
				{
					ColHist[X * 256 + LinePS[RowOffset[X]]]++;
				}
			}

			memset(Hist, 0, 256 * sizeof(unsigned short));		//	每一行直方图数据清零先

			LinePD = Dest->Data + Y * Dest->WidthStep;

			for (X = 0; X < Width; X++)
			{
				if (X == 0)
				{
					for (K = -Radius; K <= Radius; K++)			//	行第一个像素，需要重新计算	
						HistgramAddShort(ColHist + K * 256, Hist);
				}
				else
				{
					/*	HistgramAddShort(ColHist + RowOffset[X + Radius] * 256, Hist);
					HistgramSubShort(ColHist + RowOffset[X - Radius - 1] * 256, Hist);
					*/
					HistgramSubAddShort(ColHist + RowOffset[X - Radius - 1] * 256, ColHist + RowOffset[X + Radius] * 256, Hist);  //	行内其他像素，依次删除和增加就可以了
				}
				for (K = 255; K >= 0; K--)
				{
					if (Hist[K] != 0)
					{
						LinePD[X] = K;
						break;
					}
				}
			}
		}
		ColHist -= Radius * 256;		//	恢复偏移操作
	Done8:
		IS_FreeMatrix(&Row);
		IS_FreeMatrix(&Col);
		IS_FreeMemory(ColHist);
		IS_FreeMemory(Hist);
		return Ret;
	}
	else
	{
		TMatrix *Blue = NULL, *Green = NULL, *Red = NULL, *Alpha = NULL;			//	由于C变量如果不初始化，其值是随机值，可能会导致释放时的错误。
		IS_RET Ret = SplitRGBA(Src, &Blue, &Green, &Red, &Alpha);
		if (Ret != IS_RET_OK) goto Done24;
		Ret = MaxFilter(Blue, Blue, Radius);
		if (Ret != IS_RET_OK) goto Done24;
		Ret = MaxFilter(Green, Green, Radius);
		if (Ret != IS_RET_OK) goto Done24;
		Ret = MaxFilter(Red, Red, Radius);
		if (Ret != IS_RET_OK) goto Done24;											//	32位的Alpha不做任何处理，实际上32位的相关算法基本上是不能分通道处理的
		CopyAlphaChannel(Src, Dest);
		Ret = CombineRGBA(Dest, Blue, Green, Red, Alpha);
	Done24:
		IS_FreeMatrix(&Blue);
		IS_FreeMatrix(&Green);
		IS_FreeMatrix(&Red);
		IS_FreeMatrix(&Alpha);
		return Ret;
	}
}