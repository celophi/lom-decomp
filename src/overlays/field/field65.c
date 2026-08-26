#include "common.h"

typedef struct
{
    u8 data[0x20];
} UnkTable800EE6E8Entry;

extern UnkTable800EE6E8Entry D_800EE6E8[];

extern void *bcopy(const unsigned char *, unsigned char *, int);

void func_800A5638(void *dest, s32 index)
{
    bcopy((u8 *)&D_800EE6E8[index], dest, 0x20);
}
