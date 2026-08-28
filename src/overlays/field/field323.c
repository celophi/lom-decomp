#include "common.h"

extern s32 func_800A88A0(void *arg0, void *arg1, void *arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6);

typedef struct {
    u8 unk0;   /* 0x0 */
    u8 unk1;   /* 0x1 */
} StructEC400;

typedef struct {
    u8 pad0[0x40B8];
    s32 unk40B8; /* 0x40B8 */
} ArgA;

extern StructEC400 D_800EC400;
extern s32 D_801227E0;

/**
 * @brief Refresh a handle at arg0->unk40B8 unless the global gate is set.
 *
 * @note 100% match. Splitting the packed address calculation into separate
 *       low-byte and offset value webs reproduces the target operand order.
 */
void func_800AB690(ArgA *arg0)
{
    s32 handle;
    s32 low;
    s32 offset;
    u8 *base;

    handle = arg0->unk40B8;
    if (D_801227E0 == 0)
    {
        low = D_800EC400.unk0;
        offset = (D_800EC400.unk1 << 8) + (s32)(base = (u8 *)&D_800EC400 - 0x3C);
        handle = func_800A88A0(
            handle,
            arg0,
            (void *)(low + offset),
            4,
            0xA0,
            0x68,
            2);
    }
    arg0->unk40B8 = handle;
}
