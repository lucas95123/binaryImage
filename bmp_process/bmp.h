#ifndef _BMP_H_
#define _BMP_H_

#include <stdio.h>
typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned int	DWRD;

#pragma pack(2)
typedef struct
{
    WORD	bfType;
    DWRD	bfSize;
    WORD	bfReserved1;
    WORD	bfReserved2;
    DWRD	bfOffBits;
}
BITMAPFILEHEADER;

typedef struct
{
    DWRD biSize;
    DWRD biWidth;
    DWRD biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWRD bmCompression;
    DWRD biSizeImage;
    DWRD biXPelsPerMeter;
    DWRD biYPelsPerMeter;
    DWRD biClrUsed;
    DWRD beClrImportant;
}
BITMAPINFOHEADER;

typedef struct
{
    BYTE	rgbBlue;
    BYTE	rgbGreen;
    BYTE	rgbRed;
    BYTE	rgbReserved;
}
RGBQUAD;

typedef struct{
	BYTE BLUE;
	BYTE GREEN;
	BYTE RED;
}PIXEL;

typedef struct{
	double Y;
	double U;
	double V;
}PIXEL_YUV;

#endif 
