#ifndef _PAD_H
#define _PAD_H

#include "common.h"

typedef struct
{
    u8 deviceState; // 0x00 - status / mode flag
    u8 _pad1;
    u16 buttonData; // 0x02 - raw 16-bit input (pre-remap)

    u8 _pad2[0x28]; // 0x04–0x2B - unused here

    s16 axisX; // 0x2C - signed axis (negative/positive thresholded)
    s16 axisY; // 0x2E - signed axis (negative/positive thresholded)
} SCDRegs;

#endif