#include<awb.h>
#include<isp_context.h>

void awb_stats(uint8_t *rgb, int w, int h, isp_context_t *ctx)
{
    long sumR=0,sumG=0,sumB=0;
    int total = w*h;

    for(int i=0;i<total;i++){
        sumR += rgb[i*3];
        sumG += rgb[i*3+1];
        sumB += rgb[i*3+2];
    }

    ctx->awb_gain_r = (float)sumG / sumR;
    ctx->awb_gain_b = (float)sumG / sumB;
    ctx->awb_gain_g = 1.0f;
}

void awb_update(isp_context_t *ctx)
{
    float alpha = 0.1f; // tốc độ hội tụ

    ctx->awb_gain_r =
        (1 - alpha) * ctx->awb_gain_r +
        alpha * ctx->target_r;

    ctx->awb_gain_b =
        (1 - alpha) * ctx->awb_gain_b +
        alpha * ctx->target_b;

    ctx->awb_gain_g = 1.0f;
}

void awb_apply(uint8_t *rgb, int w, int h, isp_context_t *ctx)
{
    int total = w*h;

    for(int i=0;i<total;i++){
        rgb[i*3]   *= ctx->awb_gain_r;
        rgb[i*3+1] *= ctx->awb_gain_g;
        rgb[i*3+2] *= ctx->awb_gain_b;
    }
}