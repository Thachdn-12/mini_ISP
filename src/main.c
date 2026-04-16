#include <stdio.h>
#include <stdlib.h>

#include "v4l2_capture.h"
#include "isp.h"

int main()
{
    int w = 640, h = 480;

    int fd = v4l2_init("/dev/video0", w, h);
    v4l2_start(fd);

    uint8_t *rgb = malloc(w * h * 3);

    while (1) {
        int index;
        uint8_t *raw = v4l2_get_frame(fd, &index);

        run_isp(raw, rgb, w, h);

        FILE *f = fopen("data/output/out.rgb", "wb");
        fwrite(rgb, 1, w*h*3, f);
        fclose(f);

        v4l2_queue(fd, index);
    }
}