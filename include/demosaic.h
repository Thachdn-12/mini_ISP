#ifndef DEMOSAIC_H
#define DEMOSAIC_H

#include <stdint.h>

void demosaic_rggb(uint8_t *raw, uint8_t *rgb, int w, int h);

#endif