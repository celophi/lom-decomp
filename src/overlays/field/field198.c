#include "common.h"

typedef struct
{
    u8 unk0;        /* 0x00 */
    u8 pad1[0x1F];
    u8 unk20;       /* 0x20 */
    u8 pad21[0x13];
    u32 unk34;      /* 0x34 */
    u8 pad38[8];
} Rec800C9684;

extern u8 D_80122C02;
extern Rec800C9684 D_80122A08[];

void func_800C9684(void)
{
    s32 index;
    Rec800C9684 *rec;
    u8 tmp;
    u8 tmp2;

    index = D_80122C02;
    tmp = D_80122A08[index].unk20;
    rec = &D_80122A08[index];
    (&D_80122C02)[index + 0x17] = tmp;
    tmp2 = rec->unk0;
    rec->unk0 = 0;
    rec->unk20 = tmp2;
    rec->unk34 = *(u32 *) ((u8 *) &D_80122C02 + 6);
}
