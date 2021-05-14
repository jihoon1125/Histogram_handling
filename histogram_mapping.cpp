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

void MakeHistogram(UCHAR** imgbuf, float histogram[256], int width, int height)//histogram 만드는 함수, histogram_equalization.cpp의 함수와 동일하다.
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

	int width = 512;//512 x 512 이미지파일을 기준으로 한다.
	int height = 512;//

	/////////입력이미지들의 histogram, cdf array 선언 및 0으로 초기화//////////
	float Image1_Histogram[256] = { 0, };
	float Image1_Histogram_CDF[256] = { 0, };
	float Image2_Histogram[256] = { 0, };
	float Image2_Histogram_CDF[256] = { 0, };
	float Image_equal_Histogram[256] = { 0, };


	FILE* fp_InputImg1 = fopen("gray\\barbara(512x512).raw", "rb");//gray\\barbara(512x512).raw", "rb"); 히스토그램 비교대상
	if (!fp_InputImg1) {
		printf("Can not open file.");
	}

	FILE* fp_InputImg2 = fopen("gray\\Couple(512x512).raw", "rb");//gray\\Couple(512x512).raw", "rb"); 히스토그램 분포를 바꿀대상
	if (!fp_InputImg2) {
		printf("Can not open file.");
	}
	UCHAR** Input_imgBuf1 = memory_alloc2D(width, height); // input data 메모리 동적할당
	UCHAR** Input_imgBuf2 = memory_alloc2D(width, height);
	UCHAR** Output_imgBuf = memory_alloc2D(width, height);// output data 메모리 할당
	fread(&Input_imgBuf1[0][0], sizeof(UCHAR), width * height, fp_InputImg1); // 2차원 배열에 비교대상 영상 이미지 읽어오기
	fread(&Input_imgBuf2[0][0], sizeof(UCHAR), width * height, fp_InputImg2);
	MakeHistogram(Input_imgBuf1, Image1_Histogram, width, height);// 히스토그램 형성
	MakeHistogram(Input_imgBuf2, Image2_Histogram, width, height);

	for (int i = 1; i < 256; i++)//histogram cdf 값 설정
	{
		Image1_Histogram_CDF[i] = Image1_Histogram_CDF[i - 1] + Image1_Histogram[i];
		Image2_Histogram_CDF[i] = Image2_Histogram_CDF[i - 1] + Image2_Histogram[i];
	}

	DrawHistogram(Image1_Histogram, 10, 400); // 히스토그램 그래프
	DrawCDF(Image1_Histogram_CDF, 10, 400);//히스토그램 cdf 그래프

	DrawHistogram(Image2_Histogram, 30, 200); // 히스토그램 그래프
	DrawCDF(Image2_Histogram_CDF, 30, 200);


	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
			for (int k = 0; k < 256; k++)
			{
				bool factor;//최대한 근사한지 판별하기 위한 boolean variable
				factor = (Image2_Histogram_CDF[k] - Image1_Histogram_CDF[Input_imgBuf1[i][j]])>=0 ? true : false;//Image2의 cdf배열의 0 index부터 시작하여 Image1의 픽셀에 대한 cdf를 계속뺀다.  
				if (!factor) continue;																			 //처음엔 무조건 Image2 cdf값이 작으므로 음수이다가, 양수로 넘어가는 타이밍이 생긴다.
				Output_imgBuf[i][j] = k;																		 //그때 factor를 반전시켜주고, 결과 이미지버퍼에 Image2 cdf의 index값을 저장한다.
				break;
				}
			}
	


	MakeHistogram(Output_imgBuf, Image_equal_Histogram, width, height);//결과이미지 버퍼의 hisgogram을 만든다.

	DrawHistogram(Image_equal_Histogram, 400, 400); // Histogram 출력

	FILE* fp_outputImg = fopen("output_matching.raw", "wb"); // 결과 저장하기
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			fwrite(&Output_imgBuf[i][j], sizeof(UCHAR), 1, fp_outputImg);
		}
	}
	///////////메모리 해제 및 파일 닫기////////////////
	MemoryClear(Input_imgBuf1);
	MemoryClear(Input_imgBuf2);
	MemoryClear(Output_imgBuf);
	fclose(fp_outputImg);
	fclose(fp_InputImg1);
	fclose(fp_InputImg2);
	return 0;
}