#include "common.h"

typedef struct
{
    u8 unk0;
    u8 pad1[0xAE - 1];
    u8 unkAE;
} Struct_801ED600_2;

typedef struct
{
    u8 unk0;
    u8 unk1;
} D_801227B8_t;

extern s32 D_801227C8;
extern s32 D_8012291C;
extern D_801227B8_t D_801227B8;

void func_800AA824(void)
{
    Struct_801ED600_2 *ptr = (Struct_801ED600_2 *)0x801ED600;

    D_801227C8 = 0;
    D_8012291C = 0;
    D_801227B8.unk0 = ptr->unk0;
    D_801227B8.unk1 = ptr->unkAE;
}
