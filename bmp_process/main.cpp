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
#define BYTEPTR BYTE*

int HORIZONTALSLIDE;
int VERTICALSLIDE;
int GetOSTUThreshold(int* HistGram)
{
    int Y, Amount = 0;
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

int * calculateThreshoulds(double *src,int height, int width)
{
    int verticalBlock=VERTICALSLIDE;
    int horizontalBlock=HORIZONTALSLIDE;
    int slideHeight=height/verticalBlock;
    int slideWidth=width/horizontalBlock;
    int slideEdgeHeight=height-slideHeight*verticalBlock;
    slideEdgeHeight=slideEdgeHeight==0?slideHeight:slideEdgeHeight;
    int slideEdgeWidth=width-slideWidth*horizontalBlock;
    slideEdgeWidth=slideEdgeWidth==0?slideWidth:slideEdgeWidth;
    int *threshoulds = new int[verticalBlock*horizontalBlock];
    for(int i=0;i<verticalBlock;i++)/*for every vertical block*/
        for(int j=0;j<horizontalBlock;j++)/*for every horizontal block*/
        {
            int h=0;
            int w=0;
            if(j==horizontalBlock-1&&i!=verticalBlock-1)
            {
                h=slideHeight;
                w=slideEdgeWidth;
            }
            else if(j!=horizontalBlock-1&&i==verticalBlock-1)
            {
                h=slideEdgeHeight;
                w=slideWidth;
            }
            else if(j==horizontalBlock-1&&i==verticalBlock-1)
            {
                h=slideEdgeHeight;
                w=slideEdgeWidth;
            }
            else
            {
                h=slideHeight;
                w=slideWidth;
            }
            int * histGram = new int[256];
            for (int m = 0; m < 256; m++)
                histGram[m] = 0;
            for(int k=0;k<h;k++)
                for(int l=0;l<w;l++)
                    histGram[(int)(src[(i*h+k)*width+j*w+l])]+=1;
            threshoulds[i*horizontalBlock+j]=GetOSTUThreshold(histGram);
        }
    return threshoulds;
}

void Dilation(BYTEPTR *&fullImage,unsigned int &height,unsigned int &width)
{
    for(int i=0;i<height;i++)
    {
        for(int j=0;j<width;j++)
        {
            if(fullImage[i][j]==1)
            {
                fullImage[i][j+1]=1;
                fullImage[i][j-1]=1;
                fullImage[i-1][j]=1;
                fullImage[i+1][j]=1;
            }
        }
    }
}

int binaryImage(char *Src)
{
    FILE* fp, *fp_gray;
    BITMAPFILEHEADER bmfh, bmfh_gray;
    BITMAPINFOHEADER bmhi, bmhi_gray;

    /*Opem files*/
    fp = fopen(Src, "r");
    fp_gray = fopen("binary_algo_2.bmp", "w");
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

    unsigned int i;
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

    /*tmp data of sliding algorithm*/
    double *fullImageY = new double[bmhi.biHeight*bmhi.biWidth];
    /*Read Data*/
    PIXEL tmp[4320];/*buffer to store data of a line*/
     BYTE tmp_gray[4320];/*buffer to store the grayscale data of a line*/
    PIXEL_YUV BUFFER;
    for (i = 0; i < bmhi.biHeight; i++)
    {
        fread(&tmp, byteWidth, 1, fp);
        unsigned int j;
        for (j = 0; j < bmhi.biWidth; j++)
        {
            BUFFER.Y = tmp[j].RED*0.299 + tmp[j].GREEN*0.587 + tmp[j].BLUE*0.114;/*RGB->YUV*/
            fullImageY[i*bmhi.biWidth+j]=BUFFER.Y;
        }
    }
    int SLIDEHEIGHT=bmhi.biHeight/VERTICALSLIDE;
    int SLIDEWIDTH=bmhi.biWidth/HORIZONTALSLIDE;
    int * threshoulds = calculateThreshoulds(fullImageY,bmhi.biHeight,bmhi.biWidth);
    fseek(fp, bmfh.bfOffBits, SEEK_SET);
    for (i = 0; i < bmhi.biHeight; i++)
    {
        fread(&tmp, byteWidth, 1, fp);
        unsigned int j;
       
        for (j = 0; j < bmhi.biWidth; j++)
        {
            BUFFER.Y = tmp[j].RED*0.299 + tmp[j].GREEN*0.587 + tmp[j].BLUE*0.114;/*RGB->YUV*/
            tmp_gray[j] = BUFFER.Y>threshoulds[i/SLIDEHEIGHT*HORIZONTALSLIDE+j/SLIDEWIDTH] ? 255 : 0;
        }
        //tmp_full_gray[i]=tmp_gray;
        fwrite(&tmp_gray, 1, byteWidthGray, fp_gray);
    }
    fclose(fp);
    fclose(fp_gray);
    return 1;
}

int main()
{
    char SrcFileName[20];
    printf("Please enter the name of source file:");
    scanf("%s", SrcFileName);
    printf("Please enter the number of horizontal slide: ");
    scanf("%d", &HORIZONTALSLIDE);
    printf("Please enter the number of vertical slide: ");
    scanf("%d", &VERTICALSLIDE);
    binaryImage(SrcFileName);
}
