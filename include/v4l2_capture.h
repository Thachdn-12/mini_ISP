#ifndef V4L2_CAPTURE_H
#define V4L2_CAPTURE_H

#include <stdint.h>

int v4l2_init(const char *dev, int w, int h);
void v4l2_start(int fd);
uint8_t* v4l2_get_frame(int fd, int *index);
void v4l2_queue(int fd, int index);

#endif