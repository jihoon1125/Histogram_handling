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

void MakeHistogram(UCHAR** imgbuf, float histogram[256], int width, int height)//histogram ����� �Լ�, histogram_equalization.cpp�� �Լ��� �����ϴ�.
{
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
			histogram[imgbuf[i][j]]++;
	}

	for (int i = 0; i < 256; i++)
		histogram[i] *= ((double)1 / (width * height));

}

void DrawCDF(float cdf[256], int x_origin, int y_origin) {
	for (int CurX = 0; CurX < 256; CurX++) {
		for (int CurY = 0; CurY < cdf[CurX]; CurY++) {
			MoveToEx(hdc, x_origin + CurX, y_origin, 0);
			SetPixel(hdc, x_origin + CurX, y_origin - cdf[CurX] * 100, RGB(0, 0, 255));
		}
	}
}

void DrawHistogram(float histogram[256], int x_origin, int y_origin) {
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

	int width = 512;//512 x 512 �̹��������� �������� �Ѵ�.
	int height = 512;//

	/////////�Է��̹������� histogram, cdf array ���� �� 0���� �ʱ�ȭ//////////
	float Image1_Histogram[256] = { 0, };
	float Image1_Histogram_CDF[256] = { 0, };
	float Image2_Histogram[256] = { 0, };
	float Image2_Histogram_CDF[256] = { 0, };
	float Image_equal_Histogram[256] = { 0, };


	FILE* fp_InputImg1 = fopen("gray\\barbara(512x512).raw", "rb");//gray\\barbara(512x512).raw", "rb"); ������׷� �񱳴��
	if (!fp_InputImg1) {
		printf("Can not open file.");
	}

	FILE* fp_InputImg2 = fopen("gray\\Couple(512x512).raw", "rb");//gray\\Couple(512x512).raw", "rb"); ������׷� ������ �ٲܴ��
	if (!fp_InputImg2) {
		printf("Can not open file.");
	}
	UCHAR** Input_imgBuf1 = memory_alloc2D(width, height); // input data �޸� �����Ҵ�
	UCHAR** Input_imgBuf2 = memory_alloc2D(width, height);
	UCHAR** Output_imgBuf = memory_alloc2D(width, height);// output data �޸� �Ҵ�
	fread(&Input_imgBuf1[0][0], sizeof(UCHAR), width * height, fp_InputImg1); // 2���� �迭�� �񱳴�� ���� �̹��� �о����
	fread(&Input_imgBuf2[0][0], sizeof(UCHAR), width * height, fp_InputImg2);
	MakeHistogram(Input_imgBuf1, Image1_Histogram, width, height);// ������׷� ����
	MakeHistogram(Input_imgBuf2, Image2_Histogram, width, height);

	for (int i = 1; i < 256; i++)//histogram cdf �� ����
	{
		Image1_Histogram_CDF[i] = Image1_Histogram_CDF[i - 1] + Image1_Histogram[i];
		Image2_Histogram_CDF[i] = Image2_Histogram_CDF[i - 1] + Image2_Histogram[i];
	}

	DrawHistogram(Image1_Histogram, 10, 400); // ������׷� �׷���
	DrawCDF(Image1_Histogram_CDF, 10, 400);//������׷� cdf �׷���

	DrawHistogram(Image2_Histogram, 30, 200); // ������׷� �׷���
	DrawCDF(Image2_Histogram_CDF, 30, 200);


	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
			for (int k = 0; k < 256; k++)
			{
				bool factor;//�ִ��� �ٻ����� �Ǻ��ϱ� ���� boolean variable
				factor = (Image2_Histogram_CDF[k] - Image1_Histogram_CDF[Input_imgBuf1[i][j]])>=0 ? true : false;//Image2�� cdf�迭�� 0 index���� �����Ͽ� Image1�� �ȼ��� ���� cdf�� ��ӻ���.  
				if (!factor) continue;																			 //ó���� ������ Image2 cdf���� �����Ƿ� �����̴ٰ�, ����� �Ѿ�� Ÿ�̹��� �����.
				Output_imgBuf[i][j] = k;																		 //�׶� factor�� ���������ְ�, ��� �̹������ۿ� Image2 cdf�� index���� �����Ѵ�.
				break;
				}
			}
	


	MakeHistogram(Output_imgBuf, Image_equal_Histogram, width, height);//����̹��� ������ hisgogram�� �����.

	DrawHistogram(Image_equal_Histogram, 400, 400); // Histogram ���

	FILE* fp_outputImg = fopen("output_matching.raw", "wb"); // ��� �����ϱ�
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			fwrite(&Output_imgBuf[i][j], sizeof(UCHAR), 1, fp_outputImg);
		}
	}
	///////////�޸� ���� �� ���� �ݱ�////////////////
	MemoryClear(Input_imgBuf1);
	MemoryClear(Input_imgBuf2);
	MemoryClear(Output_imgBuf);
	fclose(fp_outputImg);
	fclose(fp_InputImg1);
	fclose(fp_InputImg2);
	return 0;
}