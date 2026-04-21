#ifndef ISP_CONTEXT_H
#define ISP_CONTEXT_H

typedef struct {
    float awb_gain_r;
    float awb_gain_g;
    float awb_gain_b;

    float ccm[3][3];

    float exposure;
    float gain;

} isp_context_t;

#endif