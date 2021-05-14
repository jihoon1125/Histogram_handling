#include <iostream>
#include <math.h>
#include <windows.h>

#pragma warning(disable : 4996)

using namespace std;

void DrawCDF(float cdf[256], int x_origin, int y_origin);


HWND hwnd;
HDC hdc;

void MemoryClear(UCHAR** buf) {
	if (buf) {
		free(buf[0]);
		free(buf);
		buf = NULL;
	}
}

UCHAR** memory_alloc2D(int width, int height)
{
	UCHAR** ppMem2D = 0;
	int	i;

	//arrary of pointer
	ppMem2D = (UCHAR**)calloc(sizeof(UCHAR*), height);
	if (ppMem2D == 0) {
		return 0;
	}

	*ppMem2D = (UCHAR*)calloc(sizeof(UCHAR), height * width);
	if ((*ppMem2D) == 0) {//free the memory of array of pointer        
		free(ppMem2D);
		return 0;
	}

	for (i = 1; i < height; i++) {
		ppMem2D[i] = ppMem2D[i - 1] + width;
	}

	return ppMem2D;
}

void MakeHistogram(UCHAR** imgbuf, float histogram[256], int width, int height)// histogram making func
{
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
			histogram[imgbuf[i][j]]++;//imgbuf[i][j]�� ��°��� pixel�� ��, histogram�� �� ���� �󵵼��� ��Ÿ����
	}

	for (int i = 0; i < 256; i++)
		histogram[i] *= (1.0 / (width * height));//��ü Ȯ���� 1�� �ΰ� �̿� ���� pdf�� normalize�Ѵ�.

}

UCHAR** MakeHistogramEqualization(UCHAR** imgbuf, float histogram[256], float Equal_histogram[256], int width, int height)//histogram ��Ȱȭ �� output image �ȼ��� update
{
	UCHAR** Output_imgbuf = memory_alloc2D(width, height);//output image memory allocate

	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
			Output_imgbuf[i][j] = (int)(256 * histogram[imgbuf[i][j]]);//��� �̹��� pixel�� ����
	}

	MakeHistogram(Output_imgbuf, Equal_histogram, width, height);//equal histogram make

	return Output_imgbuf;//return output image buffer

}


void DrawCDF(float cdf[256], int x_origin, int y_origin) {//CDF drawing func
	for (int CurX = 0; CurX < 256; CurX++) {
		for (int CurY = 0; CurY < cdf[CurX]; CurY++) {
			MoveToEx(hdc, x_origin + CurX, y_origin, 0);
			SetPixel(hdc, x_origin + CurX, y_origin - cdf[CurX] * 100, RGB(0, 0, 255));
		}
	}
}

void DrawHistogram(float histogram[256], int x_origin, int y_origin) {//histogram drawing func
	MoveToEx(hdc, x_origin, y_origin, 0);
	LineTo(hdc, x_origin + 255, y_origin);

	MoveToEx(hdc, x_origin, 100, 0);
	LineTo(hdc, x_origin, y_origin);

	for (int CurX = 0; CurX < 256; CurX++) {
		for (int CurY = 0; CurY < histogram[CurX]; CurY++) {
			MoveToEx(hdc, x_origin + CurX, y_origin, 0);
			LineTo(hdc, x_origin + CurX, y_origin - histogram[CurX] * 5000);
		}
	}
}

int main(void)
{
	system("color F0");
	hwnd = GetForegroundWindow();
	hdc = GetWindowDC(hwnd);

	int width = 512;     ////512 x 512 ����
	int height = 512;	 ////

	float Image_Histogram[256] = { 0, };//�̹��� ������׷� pdf �迭
	float Image_Histogram_CDF[256] = { 0, };//�̹��� ������׷��� cdf �迭
	float Image_equal_Histogram[256] = { 0, };// equalization���� �迭


	FILE* fp_InputImg = fopen("gray\\gBarbara512_512.raw", "rb");//RAW\\gray\\barbara(512x512).raw", "rb");
	if (!fp_InputImg) {
		printf("Can not open file.");
	}

	UCHAR** Input_imgBuf = memory_alloc2D(width, height); // input data �޸� �����Ҵ�
	UCHAR** Output_imgBuf;
	fread(&Input_imgBuf[0][0], sizeof(UCHAR), width * height, fp_InputImg); // 2���� �迭�� �̹��� �о����

	MakeHistogram(Input_imgBuf, Image_Histogram, width, height);// ������׷� ����

	for (int i = 1; i < 256; i++)
	{
		Image_Histogram_CDF[i] = Image_Histogram_CDF[i - 1] + Image_Histogram[i];//cdf formula
	}

	DrawHistogram(Image_Histogram, 30, 400); // ������׷� �׷���
	DrawCDF(Image_Histogram_CDF, 30, 400);//������׷� cdf �׷���

	Output_imgBuf = MakeHistogramEqualization(Input_imgBuf, Image_Histogram_CDF, Image_equal_Histogram, width, height);
	
	DrawHistogram(Image_equal_Histogram, 400, 400); // ������׷� ��Ȱȭ �׷���

	FILE* fp_outputImg = fopen("output.raw", "wb"); // ��� �����ϱ�
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			fwrite(&Output_imgBuf[i][j], sizeof(UCHAR), 1, fp_outputImg);// Output_imgbuf�� �� fp_outputimg�� write
		}
	}

	MemoryClear(Input_imgBuf);//memory clear
	MemoryClear(Output_imgBuf);//memory clear
	fclose(fp_outputImg);
	fclose(fp_InputImg);
	return 0;
}

