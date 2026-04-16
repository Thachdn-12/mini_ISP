#include "awb.h"

void awb(uint8_t *rgb, int w, int h)
{
    long sumR=0,sumG=0,sumB=0;
    int total = w*h;

    for (int i=0;i<total;i++){
        sumR += rgb[i*3];
        sumG += rgb[i*3+1];
        sumB += rgb[i*3+2];
    }

    float gainR = (float)sumG / sumR;
    float gainB = (float)sumG / sumB;

    for (int i=0;i<total;i++){
        int r = rgb[i*3] * gainR;
        int b = rgb[i*3+2] * gainB;

        rgb[i*3]   = r > 255 ? 255 : r;
        rgb[i*3+2] = b > 255 ? 255 : b;
    }
}