/* @file binary_image.c
* @brief Read a color bmp;
RGB->YUV;
Calculate threshold of binary image
Write a binary image
* @author lucas95123@outlook.com
* @version 1.0
* @date 2015/10/20
*/
#include "bmp.h"
#include <stdio.h>
#include <stdlib.h>
#define THRESHOLD 100

int GetOSTUThreshold(int* HistGram)
{
    int X, Y, Amount = 0;
    int PixelBack = 0, PixelFore = 0, PixelIntegralBack = 0, PixelIntegralFore = 0, PixelIntegral = 0;
    double OmegaBack, OmegaFore, MicroBack, MicroFore, SigmaB, Sigma;              // ÀàŒä·œ²î;
    int MinValue, MaxValue;
    int Threshold = 0;

    for (MinValue = 0; MinValue < 256 && HistGram[MinValue] == 0; MinValue++);
    for (MaxValue = 255; MaxValue > MinValue && HistGram[MinValue] == 0; MaxValue--);
    if (MaxValue == MinValue) return MaxValue;          // ÍŒÏñÖÐÖ»ÓÐÒ»žöÑÕÉ«
    if (MinValue + 1 == MaxValue) return MinValue;      // ÍŒÏñÖÐÖ»ÓÐ¶þžöÑÕÉ«

    for (Y = MinValue; Y <= MaxValue; Y++) Amount += HistGram[Y];        //  ÏñËØ×ÜÊý

    PixelIntegral = 0;
    for (Y = MinValue; Y <= MaxValue; Y++) PixelIntegral += HistGram[Y] * Y;
    SigmaB = -1;
    for (Y = MinValue; Y < MaxValue; Y++)
    {
        PixelBack = PixelBack + HistGram[Y];
        PixelFore = Amount - PixelBack;
        OmegaBack = (double)PixelBack / Amount;
        OmegaFore = (double)PixelFore / Amount;
        PixelIntegralBack += HistGram[Y] * Y;
        PixelIntegralFore = PixelIntegral - PixelIntegralBack;
        MicroBack = (double)PixelIntegralBack / PixelBack;
        MicroFore = (double)PixelIntegralFore / PixelFore;
        Sigma = OmegaBack * OmegaFore * (MicroBack - MicroFore) * (MicroBack - MicroFore);
        if (Sigma > SigmaB)
        {
            SigmaB = Sigma;
            Threshold = Y;
        }
    }
    return Threshold;
}

int binaryImage(char *Src)
{
    FILE* fp, *fp_gray;
    BITMAPFILEHEADER bmfh, bmfh_gray;
    BITMAPINFOHEADER bmhi, bmhi_gray;

    /*Opem files*/
    fp = fopen(Src, "r");
    fp_gray = fopen("binary_algo_1.bmp", "w");
    if (fp == NULL || fp_gray == NULL)
    {
        printf("Error opening source file\n");
        return 0;
    }
    /*Read header*/
    fread(&bmfh, sizeof(BITMAPFILEHEADER), 1, fp);
    fread(&bmhi, sizeof(BITMAPINFOHEADER), 1, fp);
    int byteWidth = ((bmhi.biWidth * 24 + 31)&~31) / 8;
    int byteWidthGray = ((bmhi.biWidth * 8 + 31)&~31) / 8;

    /*Write binary bmp header*/
    RGBQUAD aColors[256];

    int i;
    for (i = 0; i <= 255; i++)
    {
        aColors[i].rgbRed = i;
        aColors[i].rgbGreen = i;
        aColors[i].rgbBlue = i;
        aColors[i].rgbReserved = 0;
    }
    bmfh_gray = bmfh;
    bmhi_gray = bmhi;
    bmfh_gray.bfSize = bmhi.biHeight*((bmhi.biWidth * 8 + 31)&~31) / 8 + sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)* 256;
    bmfh_gray.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)* 256;
    bmhi_gray.biBitCount = 8;
    fwrite(&bmfh_gray, sizeof(BITMAPFILEHEADER), 1, fp_gray);
    fwrite(&bmhi_gray, sizeof(BITMAPINFOHEADER), 1, fp_gray);
    fwrite(&aColors, sizeof(RGBQUAD)* 256, 1, fp_gray);

    /*Scan and calculate threshold*/


    /*Read Data*/
    PIXEL tmp[2160];
    BYTE tmp_gray[2160];
    int histGram[256];
    PIXEL_YUV BUFFER;
    for (i = 0; i < 256; i++)
        histGram[i] = 0;
    printf("%d\n",bmhi.biWidth);

    printf("%d\n",((bmhi.biWidth * 8 + 31)&~31) / 8);
    for (i = 0; i < bmhi.biHeight; i++)
    {
        fread(&tmp, byteWidth, 1, fp);
        int j;
        for (j = 0; j < bmhi.biWidth; j++)
        {
            BUFFER.Y = tmp[j].RED*0.299 + tmp[j].GREEN*0.587 + tmp[j].BLUE*0.114;/*RGB->YUV*/
            histGram[(int)BUFFER.Y] += 1;
        }
    }
    int threshould = GetOSTUThreshold(histGram);
    fseek(fp, bmfh.bfOffBits, SEEK_SET);
    for (i = 0; i < bmhi.biHeight; i++)
    {
        fread(&tmp, byteWidth, 1, fp);
        int j;
        for (j = 0; j < bmhi.biWidth; j++)
        {
            BUFFER.Y = tmp[j].RED*0.299 + tmp[j].GREEN*0.587 + tmp[j].BLUE*0.114;/*RGB->YUV*/
            tmp_gray[j] = BUFFER.Y>threshould ? 255 : 0;
        }
        fwrite(&tmp_gray, 1, byteWidthGray, fp_gray);
    }
    printf("%d", threshould);
    fclose(fp);
    fclose(fp_gray);
    return 1;
}

int main()
{
    char SrcFileName[20];
    char DstFileName[20];
    printf("Please enter the name of source file:");
    scanf("%s", SrcFileName);
    binaryImage(SrcFileName);
}
