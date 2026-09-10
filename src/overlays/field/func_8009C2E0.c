#include "common.h"

/** @brief Position prefix of the actor record passed to the mover. */
typedef struct
{
    s32 unk0; /* 0x00 */
    s32 unk4; /* 0x04 */
    s32 unk8; /* 0x08 */
} MoverPosition;

/** @brief Map bounds stored in the field scratch area. */
typedef struct
{
    s16 unk0; /* 0x00 */
    s16 unk2; /* 0x02 */
    s16 unk4; /* 0x04 */
    u8 pad6[0xC - 6];
    s16 unkC; /* 0x0C */
} MoverBounds;

/** @brief Scratchpad position, motion, and collision resolver state. */
typedef struct
{
    s32 position[3];
    s32 motion[3];
    s32 unk18;
    s32 unk1c;
    s32 unk20;
    s16 unk24;
    s16 unk26;
    union
    {
        s32 word;
        struct
        {
            s16 status;
            s16 flags;
        } halves;
    } mode;
} ScratchMover;

extern u8 D_800FE3CE;

extern s32 func_8005B6AC(ScratchMover *mover);

/**
 * @brief Stage entry/parameter fields into the collision-mover scratch block
 *        at 0x1F800000, run the mover resolver, and copy the resolved
 *        position/height back into @p entry.
 * @param entry Struct whose unk0/unk4/unk8 (position fields) are staged into
 *              the scratch mover block and then updated from its result.
 * @param args  Three s32 parameter words staged into the mover's
 *              move_x/move_height/move_z fields (offsets 0xC/0x10/0x14).
 * @note Only runs when entry->unk0 and entry->unk8 both pass range checks
 *       against the field bounds block at 0x801ED400.
 */
void func_8009C2E0(MoverPosition * entry, s32 *args)
{
    MoverBounds * hw = (MoverBounds *)0x801ED400;
    ScratchMover * mover = (ScratchMover *)0x1F800000;
    s32 position_z;
    s32 position_x;
    s32 mode_flags;

    position_x = entry->unk0;
    if (position_x < 0)
    {
        return;
    }
    if (position_x >= (hw->unk0 << 8))
    {
        return;
    }

    position_z = entry->unk8;
    if (position_z < 0)
    {
        return;
    }
    if (position_z >= ((s32)(hw->unk2 << 16) >> 7))
    {
        return;
    }

    mover->position[0] = position_x;
    mover->position[1] = entry->unk4;
    mover->position[2] = entry->unk8;
    mover->motion[0] = args[0];
    mover->motion[1] = args[1];
    mover->motion[2] = args[2];

    if ((u8)D_800FE3CE >= 0x40)
    {
        mover->unk24 = 0xC;
        mover->mode.halves.status = 8;
    }
    else
    {
        mover->unk24 = 9;
        mover->mode.halves.status = 6;
    }

    /* Keep the scratchpad setup store ahead of the overlapping flags load. */
    *(volatile s16 *)((u8 *)mover + 0x26) = 0x10;
    mode_flags = *(volatile s32 *)((u8 *)mover + 0x28);
    mover->unk1c = -1;
    mover->unk20 = 0;
    mode_flags &= 0xFFFDFFFF;
    mode_flags &= 0xFFFEFFFF;
    mover->mode.word = mode_flags;

    func_8005B6AC(mover);

    entry->unk0 = mover->position[0];
    entry->unk8 = mover->position[2];
    entry->unk4 = mover->position[1];
}
