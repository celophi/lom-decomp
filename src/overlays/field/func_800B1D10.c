#include "common.h"

/** @brief Four-word actor position returned by the position query. */
typedef struct
{
    s32 unk0, unk4, unk8, unkC;
} Position;
/** @brief Trigger bounds and command viewed relative to the table header. */
typedef struct
{
    u8 pad0[4];
    u16 unk4, unk6, unk8, unkA, unkC;
} Region;
/** @brief Field position cache and one-shot trigger table state. */
typedef struct
{
    u8 pad0[0x44];
    s32 positions[3];
    u8 pad50[4];
    s32 unk54, unk58;
    u8 pad5C[0xB8 - 0x5C];
    s32 unkB8;
    u8 padBC[0xF00 - 0xBC];
    u8 *volatile unkF00;
} Context;
/** @brief Unsigned map coordinates used for trigger bounds checks. */
typedef struct
{
    u16 x, z;
} Point;
extern Position *D_80122B70;
extern Context *D_80122B78;
extern Point D_80042FC8;
extern s32 func_80087F44(s32, Position *);
extern void func_800B22F0(s32, u16);
extern void func_800B4410(u16);
/**
 * @brief Pack the horizontal actor coordinates into two 16-bit fields.
 * @param position Fixed-point actor position.
 * @return Packed X and Z coordinates.
 */
static inline s32 pack_position(Position *position)
{
    return ((s32)(u16)(position->unk0 >> 8) << 16) | ((position->unk8 >> 8) & 0xFFFF);
}
/**
 * @brief Refresh actor map positions and dispatch the first newly entered trigger.
 */
void func_800B1D10(void)
{
    Position positions[3];
    s32 *packed_cursor;
    Position *position_cursor;
    s32 region_offset;
    s32 used;
    Point *point;
    s32 region_bit;
    s32 index;

    s32 packed;
    u8 *table;
    Region *region;
    Region *trigger;

    index = 0;
    position_cursor = positions;
    D_80122B78->unk54 = (s32)-D_80122B70->unk4;
    packed_cursor = D_80122B78->positions;
    D_80122B78->unk58 = (s32) - (D_80122B70->unk8 + D_80122B70->unkC);
    do
    {
        packed = func_80087F44(index, position_cursor);
        if (packed != -1)
        {
            packed = pack_position(position_cursor);
        }
        *packed_cursor = packed;
        position_cursor++;
        index += 1;
        packed_cursor++;
    } while (index < 3);
    point = &D_80042FC8;
    point->x = (u16)(positions[0].unk0 >> 8);
    point->z = (u16)(positions[0].unk8 >> 8);
    table = D_80122B78->unkF00;
    if ((table != 0) &&
        (region_bit = 1, used = D_80122B78->unkB8, index = 0, ((*(u16 *)(table + 2)) != 0)))
    {
        region_offset = index;
    loop_7:
        if (!(used & region_bit))
        {
            region = (Region *)(D_80122B78->unkF00 + region_offset);
            if (((u16)point->x >= (u16)region->unk4) && ((u16)region->unk8 >= (u16)point->x) &&
                ((u16)point->z >= (u16)region->unk6))
            {
                if ((u16)region->unkA >= (u16)point->z)
                {
                    trigger = (Region *)(D_80122B78->unkF00 + region_offset);
                    D_80122B78->unkB8 = (s32)(D_80122B78->unkB8 | region_bit);
                    if (trigger->unkC & 0x8000)
                    {
                        func_800B22F0(0, trigger->unkC);
                        return;
                    }
                    func_800B4410(trigger->unkC);
                    return;
                }
                goto block_16;
            }
        }
    block_16:
        region_bit *= 2;
        index += 1;
        region_offset += 0xC;
        if (index >= (s32)(*(u16 *)(D_80122B78->unkF00 + 2)))
        {
        }
        else
        {
            goto loop_7;
        }
    }
}
