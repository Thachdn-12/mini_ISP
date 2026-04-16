#include <stdio.h>

#include "isp.h"
#include "demosaic.h"
#include "awb.h"

void run_isp(uint8_t *raw, uint8_t *rgb, int w, int h)
{
    demosaic_rggb(raw, rgb, w, h);
    awb(rgb, w, h);
}