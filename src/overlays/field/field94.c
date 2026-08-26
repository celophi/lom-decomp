#include "common.h"

typedef struct
{
    s8 pad[0xC];
    s32 flags;
} InnerStruct80123FB0;

typedef struct
{
    s8 pad[0x20];
    InnerStruct80123FB0* inner;
} OuterStruct80123FB0;

extern OuterStruct80123FB0* D_80123FB0;

s32 func_800B788C(s32 arg0)
{
    s32 result;

    result = arg0;
    if (D_80123FB0->inner->flags & 1)
    {
        result *= 2;
    }
    return result;
}
