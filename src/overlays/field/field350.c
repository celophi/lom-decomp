#include "common.h"

/** @brief {min, span} threshold pair from the D_800FF610 table (stride 4). */
typedef struct
{
    u16 min;  /* 0x00 */
    u16 span; /* 0x02 */
} FieldThreshold;

/** @brief D_80105AE0 actor slot; only the 0x10 flags word is read here. */
typedef struct
{
    u8 pad0[0x10];
    s32 flags; /* 0x10 */
} FieldActorSlot;

/** @brief Caller struct holding the actor slot index at 0x3A. */
typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A; /* 0x3A */
} FieldActor;

extern s32 D_800FE754;
extern FieldThreshold D_800FF610[];
extern u8 D_80105AE0[];

/**
 * @brief Classify a packed value against the active D_800FF610 threshold band.
 *
 * Returns 0 when the actor slot selected by @p arg0 has no low nibble set in its
 * flags word. Otherwise reads the threshold entry for the current
 * @c D_800FE754 - 1 index and compares the high 24 bits of @c *arg1 against it:
 * 1 when below the band minimum, 1 when above (min + span), else 0.
 *
 * @param arg0 Actor whose slot flags gate the test.
 * @param arg1 Pointer to the packed value; the top 24 bits are the sample.
 * @return 0 inside the band or when the slot is inactive; 1 when outside it.
 *
 * @see decomp.me (100%) TODO
 */
s32 func_80098748(FieldActor *arg0, s32 *arg1)
{
    FieldThreshold *rec;
    FieldThreshold *b;
    s32 idx;
    u16 lo;
    s32 new_var;
    s32 val;

    if ((((FieldActorSlot *)(D_80105AE0 + arg0->unk3A * 0x23C))->flags & 0xF) == 0)
    {
        return 0;
    }
    b = D_800FF610;
    idx = D_800FE754 - 1;
    rec = &b[idx];
    lo = rec->min;
    val = *arg1 >> 8;
    if (val < (new_var = (s32)lo))
    {
        return 1;
    }
    return (s32)(lo + rec->span) < val;
}
