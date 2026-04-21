#ifndef AWB_H
#define AWB_H

#include <stdint.h>

void awb_stats(rgb, w, h, ctx);
void awb_update(ctx);
void awb_apply(rgb, w, h, ctx);

#endif