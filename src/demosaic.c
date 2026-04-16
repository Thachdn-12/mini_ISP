#include "demosaic.h"

void demosaic_rggb(uint8_t *raw, uint8_t *rgb, int w, int h)
{
    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {

            int idx = y * w + x;
            int r, g, b;

            if ((y % 2 == 0) && (x % 2 == 0)) {
                r = raw[idx];
                g = (raw[idx-1] + raw[idx+1] +
                     raw[idx-w] + raw[idx+w]) >> 2;
                b = (raw[idx-w-1] + raw[idx-w+1] +
                     raw[idx+w-1] + raw[idx+w+1]) >> 2;

            } else if ((y % 2 == 0) && (x % 2 == 1)) {
                r = (raw[idx-1] + raw[idx+1]) >> 1;
                g = raw[idx];
                b = (raw[idx-w] + raw[idx+w]) >> 1;

            } else if ((y % 2 == 1) && (x % 2 == 0)) {
                r = (raw[idx-w] + raw[idx+w]) >> 1;
                g = raw[idx];
                b = (raw[idx-1] + raw[idx+1]) >> 1;

            } else {
                r = (raw[idx-w-1] + raw[idx-w+1] +
                     raw[idx+w-1] + raw[idx+w+1]) >> 2;
                g = (raw[idx-1] + raw[idx+1] +
                     raw[idx-w] + raw[idx+w]) >> 2;
                b = raw[idx];
            }

            int i = idx * 3;
            rgb[i]     = r > 255 ? 255 : r;
            rgb[i + 1] = g > 255 ? 255 : g;
            rgb[i + 2] = b > 255 ? 255 : b;
        }
    }
}