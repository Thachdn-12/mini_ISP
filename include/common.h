#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define CLAMP(x) ((x) > 255 ? 255 : ((x) < 0 ? 0 : (x)))

#endif