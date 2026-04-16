#ifndef ISP_H
#define ISP_H
#include <stdint.h>

void run_isp(uint8_t *raw, uint8_t *rgb, int w, int h);

#endif