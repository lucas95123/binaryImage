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
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

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

/*Erosion of the image*/
BYTE* Dilation(BYTE *fullImage,unsigned int &height,unsigned int &width)
{
    BYTE *tmp_fullImage = new BYTE[4320*7680];
    for(unsigned int i=0;i<height;i++)
    {
        for(unsigned int j=0;j<width;j++)
        {
            if(fullImage[i*width+j]==0)
            {
		fullImage[i*width+j]=0;
		if(j!=width-1)
                tmp_fullImage[i*width+j+1]=0;
		if(j!=0)
                tmp_fullImage[i*width+j-1]=0;
		if(i!=height-1)
                tmp_fullImage[(i+1)*width+j]=0;
		if(i!=0)
                tmp_fullImage[(i-1)*width+j]=0;
            }
        else tmp_fullImage[i*width+j]=255;
        }
    }
    return tmp_fullImage;
}

/*Erosion of the image*/
BYTE* Erosion(BYTE *fullImage,unsigned int &height,unsigned int &width)
{
    BYTE *tmp_fullImage = new BYTE[4320*7680];
    for(unsigned int i=0;i<height;i++)
    {
        for(unsigned int j=0;j<width;j++)
        {
            if(fullImage[i*width+j]==0)
            {
                if(i==0||j==0||i==height-1||j==width-1)
		{	tmp_fullImage[i*width+j]=255;
			continue;
		}
                else
                {
                    if(fullImage[(i+1)*width+j]==0&&fullImage[(i-1)*width+j]==0&&fullImage[i*width+j+1]==0&&fullImage[i*width+j-1]==0)
                        tmp_fullImage[i*width+j]=0;
                    else
                        tmp_fullImage[i*width+j]=255;
                }
            }
        else tmp_fullImage[i*width+j]=255;
        }
    }
    return tmp_fullImage;
}

int binaryImage(string Src,string Dst)
{
    FILE* fp, *fp_dilation, *fp_erosion, *fp_opening, *fp_closing;
    BITMAPFILEHEADER bmfh, bmfh_gray;
    BITMAPINFOHEADER bmhi, bmhi_gray;

    /*Opem files*/
    fp = fopen((Src+".bmp").c_str(), "r");
    if (fp == NULL)
    {
        printf("Error opening source file\n");
        return 0;
    }
    fp_dilation = fopen((Dst+"_dilation.bmp").c_str(), "w");
    fp_erosion = fopen((Dst+"_erosion.bmp").c_str(), "w");
    fp_opening = fopen((Dst+"_opening.bmp").c_str(), "w");
    fp_closing = fopen((Dst+"_closing.bmp").c_str(), "w");
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
    fwrite(&bmfh_gray, sizeof(BITMAPFILEHEADER), 1, fp_dilation);
    fwrite(&bmhi_gray, sizeof(BITMAPINFOHEADER), 1, fp_dilation);
    fwrite(&aColors, sizeof(RGBQUAD)* 256, 1, fp_dilation);
    fwrite(&bmfh_gray, sizeof(BITMAPFILEHEADER), 1, fp_erosion);
    fwrite(&bmhi_gray, sizeof(BITMAPINFOHEADER), 1, fp_erosion);
    fwrite(&aColors, sizeof(RGBQUAD)* 256, 1, fp_erosion);
    fwrite(&bmfh_gray, sizeof(BITMAPFILEHEADER), 1, fp_opening);
    fwrite(&bmhi_gray, sizeof(BITMAPINFOHEADER), 1, fp_opening);
    fwrite(&aColors, sizeof(RGBQUAD)* 256, 1, fp_opening);
    fwrite(&bmfh_gray, sizeof(BITMAPFILEHEADER), 1, fp_closing);
    fwrite(&bmhi_gray, sizeof(BITMAPINFOHEADER), 1, fp_closing);
    fwrite(&aColors, sizeof(RGBQUAD)* 256, 1, fp_closing);

    /*tmp data of sliding algorithm*/
    double *fullImageY = new double[bmhi.biHeight*bmhi.biWidth];
    /*Read Data*/
    PIXEL tmp[4320];/*buffer to store data of a line*/
    BYTE *tmp_full_image = new BYTE[4320*7680];
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
            tmp_full_image[i*bmhi.biWidth+j]=BUFFER.Y>threshoulds[i/SLIDEHEIGHT*HORIZONTALSLIDE+j/SLIDEWIDTH] ? 255 : 0;
        }
    } 

    /*Dilate and return the generated image*/
    BYTE* dilation_image=Dilation(tmp_full_image,bmhi.biHeight,bmhi.biWidth);
    for(unsigned int i=0;i<bmhi.biHeight;i++)
    {
        fwrite(&dilation_image[i*bmhi.biWidth], 1, byteWidthGray, fp_dilation);
    }

    /*Erode and return the generated image*/
    BYTE* erosion_image=Erosion(tmp_full_image,bmhi.biHeight,bmhi.biWidth);
    for(unsigned int i=0;i<bmhi.biHeight;i++)
    {
        fwrite(&erosion_image[i*bmhi.biWidth], 1, byteWidthGray, fp_erosion);
    }

    /*Open and return the generated image*/
    BYTE* opening_image=Dilation(erosion_image,bmhi.biHeight,bmhi.biWidth);
    for(unsigned int i=0;i<bmhi.biHeight;i++)
    {
        fwrite(&opening_image[i*bmhi.biWidth], 1, byteWidthGray, fp_opening);
    }

    /*Close and return the generated image*/
    BYTE* closing_image=Erosion(dilation_image,bmhi.biHeight,bmhi.biWidth);
    for(unsigned int i=0;i<bmhi.biHeight;i++)
    {
        fwrite(&closing_image[i*bmhi.biWidth], 1, byteWidthGray, fp_closing);
    }
    delete dilation_image;
    delete erosion_image;
    delete opening_image;
    delete closing_image;
    delete tmp_full_image;
    fclose(fp);
    fclose(fp_dilation);
    fclose(fp_erosion);
    fclose(fp_opening);
    fclose(fp_closing);
    return 1;
}

int main()
{
    string srcFileName;
    string dstFileName;
    cout<<"Please enter the name of source file(without .bmp): ";
    cin>>srcFileName;
    cout<<"Please enter the number of destination file(without .bmp): ";
    cin>>dstFileName;
    cout<<"Please enter the number of horizontal slide: ";
    cin>>HORIZONTALSLIDE;
    cout<<"Please enter the number of vertical slide: ";
    cin>>VERTICALSLIDE;
    binaryImage(srcFileName,dstFileName);
}
