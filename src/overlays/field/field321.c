#include "common.h"

typedef struct {
    u32 unk0;   /* 0x0 */
    u32 unk4;   /* 0x4 */
} ArgB;

typedef struct {
    u8 pad0[0x40];
    u8 unk40;   /* 0x40 */
    u8 pad41[0x40B8 - 0x41];
    s32 unk40B8; /* 0x40B8 */
} ArgA;

/**
 * @see decomp.me (100%) TODO
 */
void func_800A6634(ArgA *arg0, ArgB *arg1)
{
    s32 count;
    s32 val;
    u32 field4;
    s32 handle;
    void *p40;

    count = 0x100;
    handle = arg0->unk40B8;
    field4 = arg1->unk4;
    val = (field4 >> 0x11) & 0x3F;
    p40 = &arg0->unk40;
    if (val < 0x20)
    {
        count = val * 4;
    }
    arg0->unk40B8 = func_800A66B4(
        handle,
        p40,
        arg1->unk0,
        4,
        field4 & 0x1FF,
        (arg1->unk4 >> 9) & 0xFF,
        2,
        count);
}
