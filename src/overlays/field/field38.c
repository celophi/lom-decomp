#include "common.h"

typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A; /* 0x3A */
} Struct_D800FDF58;

typedef struct
{
    u8 pad0[0x4A];
    u16 unk4A; /* 0x4A */
    u8 pad4C[0x23C - 0x4C];
} Struct_D80105AE0;

extern Struct_D80105AE0 D_80105AE0[];

s32 func_80090F50(Struct_D800FDF58 *rec)
{
    return D_80105AE0[rec->unk3A].unk4A < 0xFF;
}
