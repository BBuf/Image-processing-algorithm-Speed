#pragma once
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <opencv2/opencv.hpp>
using namespace std;

#define WIDTHBYTES(bytes) (((bytes * 8) + 31) / 32 * 4)
const float Inv255 = 1.0 / 255;
const double Eps = 2.220446049250313E-16;


//边缘填充的方式
enum EdgeMode {
	Tile = 0, //重复边缘元素
	Smear = 1 //镜像边缘元素
};

enum IS_RET {
	IS_RET_OK,									//	正常
	IS_RET_ERR_OUTOFMEMORY,						//	内存溢出
	IS_RET_ERR_STACKOVERFLOW,					//	堆栈溢出
	IS_RET_ERR_NULLREFERENCE,					//	空引用
	IS_RET_ERR_ARGUMENTOUTOFRANGE,				//	参数不在正常范围内
	IS_RET_ERR_PARAMISMATCH,					//	参数不匹配
	IS_RET_ERR_DIVIDEBYZERO,
	IS_RET_ERR_INDEXOUTOFRANGE,
	IS_RET_ERR_NOTSUPPORTED,
	IS_RET_ERR_OVERFLOW,
	IS_RET_ERR_FILENOTFOUND,
	IS_RET_ERR_UNKNOWN
};

enum IS_DEPTH
{
	IS_DEPTH_8U = 0,			//	unsigned char
	IS_DEPTH_8S = 1,			//	char
	IS_DEPTH_16S = 2,			//	short
	IS_DEPTH_32S = 3,			//  int
	IS_DEPTH_32F = 4,			//	float
	IS_DEPTH_64F = 5,			//	double
};

struct TMatrix
{
	int Width;					//	矩阵的宽度
	int Height;					//	矩阵的高度
	int WidthStep;				//	矩阵一行元素的占用的字节数
	int Channel;				//	矩阵通道数
	int Depth;					//	矩阵元素的类型
	unsigned char *Data;		//	矩阵的数据
	int Reserved;				//	保留使用
};

// 内存申请
void *IS_AllocMemory(unsigned int Size, bool ZeroMemory = true) {
	void *Ptr = _mm_malloc(Size, 32);
	if (Ptr != NULL)
		if (ZeroMemory == true)
			memset(Ptr, 0, Size);
	return Ptr;
}

// 内存释放
void IS_FreeMemory(void *Ptr) {
	if (Ptr != NULL) _mm_free(Ptr);
}

// 根据矩阵元素的类型来获取一个元素实际占用的字节数
int IS_ELEMENT_SIZE(int Depth) {
	int Size;
	switch (Depth)
	{
	case IS_DEPTH_8U:
		Size = sizeof(unsigned char);
		break;
	case IS_DEPTH_8S:
		Size = sizeof(char);
		break;
	case IS_DEPTH_16S:
		Size = sizeof(short);
		break;
	case IS_DEPTH_32S:
		Size = sizeof(int);
		break;
	case IS_DEPTH_32F:
		Size = sizeof(float);
		break;
	case IS_DEPTH_64F:
		Size = sizeof(double);
		break;
	default:
		Size = 0;
		break;
	}
	return Size;
}

//创建新的矩阵数据
IS_RET IS_CreateMatrix(int Width, int Height, int Depth, int Channel, TMatrix **Matrix) {
	if (Width < 1 || Height < 1) return IS_RET_ERR_ARGUMENTOUTOFRANGE; //参数不在正常范围内
	if (Depth != IS_DEPTH_8U && Depth != IS_DEPTH_8S && Depth != IS_DEPTH_16S && Depth != IS_DEPTH_32S &&
		Depth != IS_DEPTH_32F && Depth != IS_DEPTH_64F) return IS_RET_ERR_ARGUMENTOUTOFRANGE; //参数不在正常范围内
	if (Channel != 1 && Channel != 2 && Channel != 3 && Channel != 4) return IS_RET_ERR_ARGUMENTOUTOFRANGE;
	*Matrix = (TMatrix *)IS_AllocMemory(sizeof(TMatrix));
	(*Matrix)->Width = Width;
	(*Matrix)->Height = Height;
	(*Matrix)->Depth = Depth;
	(*Matrix)->Channel = Channel;
	(*Matrix)->WidthStep = WIDTHBYTES(Width * Channel * IS_ELEMENT_SIZE(Depth));
	(*Matrix)->Data = (unsigned char*)IS_AllocMemory((*Matrix)->Height * (*Matrix)->WidthStep, true);
	if ((*Matrix)->Data == NULL) {
		IS_FreeMemory(*Matrix);
		return IS_RET_ERR_OUTOFMEMORY; //内存溢出
	}
	(*Matrix)->Reserved = 0;
	return IS_RET_OK;
}

//释放创建的矩阵
IS_RET IS_FreeMatrix(TMatrix **Matrix) {
	if ((*Matrix) == NULL) return IS_RET_ERR_NULLREFERENCE; //空引用
	if ((*Matrix)->Data == NULL) {
		IS_FreeMemory((*Matrix));
		return IS_RET_ERR_OUTOFMEMORY;
	}
	else {
		IS_FreeMemory((*Matrix)->Data);
		IS_FreeMemory((*Matrix));
		return IS_RET_OK;
	}
}

//克隆现有的矩阵
IS_RET IS_CloneMatrix(TMatrix *Src, TMatrix **Dest) {
	if (Src == NULL) return IS_RET_ERR_NULLREFERENCE;
	if (Src->Data == NULL) return IS_RET_ERR_NULLREFERENCE;
	IS_RET Ret = IS_CreateMatrix(Src->Width, Src->Height, Src->Depth, Src->Channel, Dest);
	if (Ret == IS_RET_OK) memcpy((*Dest)->Data, Src->Data, (*Dest)->Height * (*Dest)->WidthStep);
	return Ret;
}