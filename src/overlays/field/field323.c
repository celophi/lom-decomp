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
 * @brief Refresh a handle at arg0->unk40B8 unless the global gate D_801227E0 is set.
 *
 * @details When D_801227E0 is zero, re-acquires the handle via func_800A88A0,
 * passing a pointer formed from the two leading bytes of D_800EC400 as an index
 * into the region 0x3C bytes before it.
 *
 * @note NOT YET MATCHED (99.69%). The single remaining defect is one commutative
 *       operand swap on the final @c addu of the three-term index sum
 *       (@c "addu a2,v0,a2" vs @c "addu a2,a2,v0"); per idioms.md [ALLOC-15] this
 *       swap is not reachable by source operand order or re-association (all were
 *       measured inert), and the permuter converged to the same state. Everything
 *       else - insn count, frame, and all other rows - is exact.
 *
 * @see decomp.me (99.69%) TODO
 */
void func_800AB690(ArgA *arg0)
{
    s32 handle;
    u8 *base;

    handle = arg0->unk40B8;
    if (D_801227E0 == 0)
    {
        handle = func_800A88A0(
            handle,
            arg0,
            D_800EC400.unk0 + ((D_800EC400.unk1 << 8) + (base = (u8 *)&D_800EC400 - 0x3C)),
            4,
            0xA0,
            0x68,
            2);
    }
    arg0->unk40B8 = handle;
}
