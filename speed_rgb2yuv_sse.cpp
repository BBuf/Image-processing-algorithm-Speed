#include <stdio.h>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

void RGBToYUV(unsigned char *RGB, unsigned char *Y, unsigned char *U, unsigned char *V, int Width, int Height, int Stride)
{
	const int Shift = 15;
	const int HalfV = 1 << (Shift - 1);
	const int Y_B_WT = 0.114f * (1 << Shift), Y_G_WT = 0.587f * (1 << Shift), Y_R_WT = (1 << Shift) - Y_B_WT - Y_G_WT;
	const int U_B_WT = 0.436f * (1 << Shift), U_G_WT = -0.28886f * (1 << Shift), U_R_WT = -(U_B_WT + U_G_WT);
	const int V_B_WT = -0.10001 * (1 << Shift), V_G_WT = -0.51499f * (1 << Shift), V_R_WT = -(V_B_WT + V_G_WT);
	for (int YY = 0; YY < Height; YY++)
	{
		unsigned char *LinePS = RGB + YY * Stride;
		unsigned char *LinePY = Y + YY * Width;
		unsigned char *LinePU = U + YY * Width;
		unsigned char *LinePV = V + YY * Width;
		for (int XX = 0; XX < Width; XX++, LinePS += 3)
		{
			int Blue = LinePS[0], Green = LinePS[1], Red = LinePS[2];
			LinePY[XX] = (Y_B_WT * Blue + Y_G_WT * Green + Y_R_WT * Red + HalfV) >> Shift;
			LinePU[XX] = ((U_B_WT * Blue + U_G_WT * Green + U_R_WT * Red + HalfV) >> Shift) + 128;
			LinePV[XX] = ((V_B_WT * Blue + V_G_WT * Green + V_R_WT * Red + HalfV) >> Shift) + 128;
		}
	}
}

int main() {

}
