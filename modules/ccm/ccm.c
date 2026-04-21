void ccm_apply(uint8_t *rgb, int w, int h, isp_context_t *ctx)
{
    int total = w*h;

    for(int i=0;i<total;i++){
        float r = rgb[i*3];
        float g = rgb[i*3+1];
        float b = rgb[i*3+2];

        float r2 = ctx->ccm[0][0]*r + ctx->ccm[0][1]*g + ctx->ccm[0][2]*b;
        float g2 = ctx->ccm[1][0]*r + ctx->ccm[1][1]*g + ctx->ccm[1][2]*b;
        float b2 = ctx->ccm[2][0]*r + ctx->ccm[2][1]*g + ctx->ccm[2][2]*b;

        rgb[i*3]   = r2 > 255 ? 255 : r2;
        rgb[i*3+1] = g2 > 255 ? 255 : g2;
        rgb[i*3+2] = b2 > 255 ? 255 : b2;
    }
}