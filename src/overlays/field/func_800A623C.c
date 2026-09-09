#include "common.h"
/** @brief Text pointer and packed position/countdown state for one text slot. */
typedef struct Slot
{
    u8 *text;
    u32 flags;
} Slot;
/** @brief Fixed-point position at the start of a 0x54-byte actor entry. */
typedef struct Position
{
    s32 x, y, z;
    u8 rest[0x54 - 12];
} Position;
extern Slot D_801226A0[];
extern Position D_800FDF58[];
extern u8 D_800ED064[];
extern u8 *g_pad_ctx;
extern s32 D_800F22A0, D_800F22A4, D_800F22A8;
extern s32 func_800AE864(u8 *);
/**
 * @brief Start an inactive text slot near its actor and clamp its screen position.
 * @param arg0 Actor/text slot index; indices at least two are ignored.
 * @param arg1 Text table index, or a negative packed selector into the pad context.
 */
void func_800A623C(s32 arg0, s32 arg1)
{
    s16 point[2];
    Slot *slot;
    Slot *initial;
    u8 *initial_base;
    u8 *actor_base;
    Slot *output;
    Slot *base;
    Position *actor;
    s32 offset;
    s32 xoff, x, y, width;

    s32 first, second;
    if (arg0 < 2)
    {
        initial_base = (u8 *)D_801226A0;
        initial = (Slot *)(arg0 * 8 + initial_base);
        if (!((initial->flags >> 17) & 0x3F))
        {
            first = arg1 * 2;
            if (arg1 < 0)
            {
                first = ((u32)arg1 >> 16) & 0xFF;
                second = first * 0x250 + 0x5F0;
                first = (s32)g_pad_ctx + second;
                second = ((arg1 & 0xFF) << 6) + 0x150;
            }
            else
            {
                second = (u32)D_800ED064;
                first = *(u16 *)(first + second);
            }
            initial->text = (u8 *)(first + second);
            xoff = D_800F22A0;
            base = D_801226A0;
            offset = arg0 * 8;
            slot = (Slot *)(offset + (u8 *)base);
            slot->flags |= 0x7E0000;
            if (xoff < 0)
            {
                xoff += 255;
            }
            actor_base = (u8 *)D_800FDF58;
            actor = (Position *)(arg0 * 0x54 + actor_base);
            x = actor->x;
            if (x < 0)
            {
                x += 255;
            }
            arg0 = xoff >> 8;
            xoff = D_800F22A4;
            point[0] = arg0 + ((x >> 8) + 160);
            if (xoff < 0)
            {
                xoff += 255;
            }
            x = xoff >> 8;
            y = actor->y;
            if (y < 0)
            {
                y += 255;
            }
            xoff = actor->z;
            y = x + ((y >> 8) + 112);
            if (xoff < 0)
            {
                xoff += 511;
            }
            x = D_800F22A8;
            xoff = y - (xoff >> 9);
            if (x < 0)
            {
                x += 511;
            }
            point[1] = xoff - (x >> 9);
            width = func_800AE864(slot->text) * 6;
            if (point[0] + width >= 321)
            {
                point[0] = 320 - width;
            }
            if (point[0] - width - 8 < 0)
            {
                point[0] = width + 8;
            }
            if (point[1] >= 177)
            {
                point[1] = 176;
            }
            if (point[1] < 50)
            {
                point[1] = 50;
            }
            output = (Slot *)(offset + (u8 *)base);
            output->flags = (output->flags & ~0x1FF) | (point[0] & 0x1FF);
            output->flags = (output->flags & 0xFFFE01FF) | (((point[1] + 4) & 0xFF) << 9);
        }
    }
}
