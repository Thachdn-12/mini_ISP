#include "isp.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main()
{
    int w = 640, h = 480;

    FILE *f = fopen("data/input/frame.raw", "rb");
    if (!f) {
        perror("open raw");
        return -1;
    }

    uint8_t *raw = malloc(w * h);
    fread(raw, 1, w*h, f);
    fclose(f);

    uint8_t *rgb = malloc(w * h * 3);

    printf("Running ISP...\n");

    run_isp(raw, rgb, w, h);

    FILE *out = fopen("data/output/out.rgb", "wb");
    fwrite(rgb, 1, w*h*3, out);
    fclose(out);

    printf("Done. Output: data/output/out.rgb\n");

    free(raw);
    free(rgb);

    return 0;
}