void CalcGaussCof(float Radius, float &B0, float &B1, float &B2, float &B3)
{
	float Q, B;
	if (Radius >= 2.5)
		Q = (double)(0.98711 * Radius - 0.96330);                            //    对应论文公式11b
	else if ((Radius >= 0.5) && (Radius < 2.5))
		Q = (double)(3.97156 - 4.14554 * sqrt(1 - 0.26891 * Radius));
	else
		Q = (double)0.1147705018520355224609375;

	B = 1.57825 + 2.44413 * Q + 1.4281 * Q * Q + 0.422205 * Q * Q * Q;        //    对应论文公式8c
	B1 = 2.44413 * Q + 2.85619 * Q * Q + 1.26661 * Q * Q * Q;
	B2 = -1.4281 * Q * Q - 1.26661 * Q * Q * Q;
	B3 = 0.422205 * Q * Q * Q;

	B0 = 1.0 - (B1 + B2 + B3) / B;
	B1 = B1 / B;
	B2 = B2 / B;
	B3 = B3 / B;
}

void ConvertBGR8U2BGRAF(unsigned char *Src, float *Dest, int Width, int Height, int Stride)
{
	//#pragma omp parallel for
	for (int Y = 0; Y < Height; Y++)
	{
		unsigned char *LinePS = Src + Y * Stride;
		float *LinePD = Dest + Y * Width * 3;
		for (int X = 0; X < Width; X++, LinePS += 3, LinePD += 3)
		{
			LinePD[0] = LinePS[0];    LinePD[1] = LinePS[1];    LinePD[2] = LinePS[2];
		}
	}
}

void GaussBlurFromLeftToRight(float *Data, int Width, int Height, float B0, float B1, float B2, float B3)
{
	//#pragma omp parallel for
	for (int Y = 0; Y < Height; Y++)
	{
		float *LinePD = Data + Y * Width * 3;
		//w[n-1], w[n-2], w[n-3]
		float BS1 = LinePD[0], BS2 = LinePD[0], BS3 = LinePD[0]; //边缘处使用重复像素的方案
		float GS1 = LinePD[1], GS2 = LinePD[1], GS3 = LinePD[1];
		float RS1 = LinePD[2], RS2 = LinePD[2], RS3 = LinePD[2];
		for (int X = 0; X < Width; X++, LinePD += 3)
		{
			LinePD[0] = LinePD[0] * B0 + BS1 * B1 + BS2 * B2 + BS3 * B3;
			LinePD[1] = LinePD[1] * B0 + GS1 * B1 + GS2 * B2 + GS3 * B3;         // 进行顺向迭代
			LinePD[2] = LinePD[2] * B0 + RS1 * B1 + RS2 * B2 + RS3 * B3;
			BS3 = BS2, BS2 = BS1, BS1 = LinePD[0];
			GS3 = GS2, GS2 = GS1, GS1 = LinePD[1];
			RS3 = RS2, RS2 = RS1, RS1 = LinePD[2];
		}
	}
}

void GaussBlurFromRightToLeft(float *Data, int Width, int Height, float B0, float B1, float B2, float B3) {
	for (int Y = Height - 1; Y >= 0; Y--) {
		//w[n+1], w[n+2], w[n+3]
		float *LinePD = Data + ((Y + 1) * (Width - 1) * 3);
		float BS1 = LinePD[0], BS2 = LinePD[0], BS3 = LinePD[0];
		float GS1 = LinePD[1], GS2 = LinePD[1], GS3 = LinePD[1];
		float RS1 = LinePD[2], RS2 = LinePD[2], RS3 = LinePD[2];
		for (int X = Width - 1; X >= 0; X--, LinePD -= 3) {
			LinePD[0] = LinePD[0] * B0 + BS1 * B1 + BS2 * B2 + BS3 * B3;
			LinePD[1] = LinePD[1] * B0 + GS1 * B1 + GS2 * B2 + GS3 * B3;         // 进行反向迭代
			LinePD[2] = LinePD[2] * B0 + RS1 * B1 + RS2 * B2 + RS3 * B3;
			BS1 = BS2, BS2 = BS3, BS3 = LinePD[0];
			GS1 = GS2, GS2 = GS3, GS3 = LinePD[1];
			RS1 = RS2, RS2 = RS2, RS3 = LinePD[2];
		}
 	}
}

//w[n] w[n-1], w[n-2], w[n-3]
void GaussBlurFromTopToBottom(float *Data, int Width, int Height, float B0, float B1, float B2, float B3)
{
	for (int Y = 0; Y < Height; Y++)
	{
		float *LinePD3 = Data + (Y + 0) * Width * 3;
		float *LinePD2 = Data + (Y + 1) * Width * 3;
		float *LinePD1 = Data + (Y + 2) * Width * 3;
		float *LinePD0 = Data + (Y + 3) * Width * 3;
		for (int X = 0; X < Width; X++, LinePD0 += 3, LinePD1 += 3, LinePD2 += 3, LinePD3 += 3)
		{
			LinePD0[0] = LinePD0[0] * B0 + LinePD1[0] * B1 + LinePD2[0] * B2 + LinePD3[0] * B3;
			LinePD0[1] = LinePD0[1] * B0 + LinePD1[1] * B1 + LinePD2[1] * B2 + LinePD3[1] * B3;
			LinePD0[2] = LinePD0[2] * B0 + LinePD1[2] * B1 + LinePD2[2] * B2 + LinePD3[2] * B3;
		}
	}
}

//w[n] w[n+1], w[n+2], w[n+3]
void GaussBlurFromBottomToTop(float *Data, int Width, int Height, float B0, float B1, float B2, float B3) {
	for (int Y = Height - 1; Y >= 0; Y--) {
		float *LinePD3 = Data + (Y + 3) * Width * 3;
		float *LinePD2 = Data + (Y + 2) * Width * 3;
		float *LinePD1 = Data + (Y + 1) * Width * 3;
		float *LinePD0 = Data + (Y + 0) * Width * 3;
		for (int X = 0; X < Width; X++, LinePD0 += 3, LinePD1 += 3, LinePD2 += 3, LinePD3 += 3) {
			LinePD0[0] = LinePD0[0] * B0 + LinePD1[0] * B1 + LinePD2[0] * B2 + LinePD3[0] * B3;
			LinePD0[1] = LinePD0[1] * B0 + LinePD1[1] * B1 + LinePD2[1] * B2 + LinePD3[1] * B3;
			LinePD0[2] = LinePD0[2] * B0 + LinePD1[2] * B1 + LinePD2[2] * B2 + LinePD3[2] * B3;
		}
	}
}

void ConvertBGRAF2BGR8U(float *Src, unsigned char *Dest, int Width, int Height, int Stride)
{
	//#pragma omp parallel for
	for (int Y = 0; Y < Height; Y++)
	{
		float *LinePS = Src + Y * Width * 3;
		unsigned char *LinePD = Dest + Y * Stride;
		for (int X = 0; X < Width; X++, LinePS += 3, LinePD += 3)
		{
			LinePD[0] = LinePS[0];    LinePD[1] = LinePS[1];    LinePD[2] = LinePS[2];
		}
	}
}

void GaussBlur(unsigned char *Src, unsigned char *Dest, int Width, int Height, int Stride, float Radius)
{
	float B0, B1, B2, B3;
	float *Buffer = (float *)malloc(Width * (Height + 6) * sizeof(float) * 3);

	CalcGaussCof(Radius, B0, B1, B2, B3);
	ConvertBGR8U2BGRAF(Src, Buffer + 3 * Width * 3, Width, Height, Stride);

	GaussBlurFromLeftToRight(Buffer + 3 * Width * 3, Width, Height, B0, B1, B2, B3);
	GaussBlurFromRightToLeft(Buffer + 3 * Width * 3, Width, Height, B0, B1, B2, B3);        //    如果启用多线程，建议把这个函数写到GaussBlurFromLeftToRight的for X循环里，因为这样就可以减少线程并发时的阻力

	memcpy(Buffer + 0 * Width * 3, Buffer + 3 * Width * 3, Width * 3 * sizeof(float));
	memcpy(Buffer + 1 * Width * 3, Buffer + 3 * Width * 3, Width * 3 * sizeof(float));
	memcpy(Buffer + 2 * Width * 3, Buffer + 3 * Width * 3, Width * 3 * sizeof(float));

	GaussBlurFromTopToBottom(Buffer, Width, Height, B0, B1, B2, B3);

	memcpy(Buffer + (Height + 3) * Width * 3, Buffer + (Height + 2) * Width * 3, Width * 3 * sizeof(float));
	memcpy(Buffer + (Height + 4) * Width * 3, Buffer + (Height + 2) * Width * 3, Width * 3 * sizeof(float));
	memcpy(Buffer + (Height + 5) * Width * 3, Buffer + (Height + 2) * Width * 3, Width * 3 * sizeof(float));

	GaussBlurFromBottomToTop(Buffer, Width, Height, B0, B1, B2, B3);

	ConvertBGRAF2BGR8U(Buffer + 3 * Width * 3, Dest, Width, Height, Stride);

	free(Buffer);
}
