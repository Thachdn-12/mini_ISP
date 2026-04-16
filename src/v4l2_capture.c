#include <stdio.h>

#include "v4l2_capture.h"

int v4l2_init(const char *dev, int w, int h)
{
    printf("v4l2_init stub\n");
    return 0;
}

void v4l2_start(int fd)
{
    printf("v4l2_start stub\n");
}

uint8_t* v4l2_get_frame(int fd, int *index)
{
    static uint8_t dummy[640*480];
    *index = 0;
    return dummy;
}

void v4l2_queue(int fd, int index)
{
}