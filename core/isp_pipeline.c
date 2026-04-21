#include "isp_context.h"
#include "demosaic.h"
#include "awb.h"
#include "ccm.h"
#include "gamma.h"

void isp_run(uint8_t *raw,
             uint8_t *rgb,
             int w, int h,
             isp_context_t *ctx)
{
    demosaic_rggb(raw, rgb, w, h);

    awb_apply(rgb, w, h, ctx);

    ccm_apply(rgb, w, h, ctx);

    gamma_apply(rgb, w, h, ctx);
}