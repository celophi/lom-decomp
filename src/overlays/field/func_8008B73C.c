#include "common.h"

extern s32 D_8010A020[];
extern u8 D_80105880[];
extern u8 g_field_actor_slots[];
extern u8 D_80105AE0[];

s32 field_start_actor_animation(s32, s32, s32);

/**
 * @brief Restart animations for three pending actor slots and clear their update flags.
 * @see decomp.me (100%)
 */
void func_8008B73C(void)
{
    s32 i;
    s32 *flag;
    u8 *rec;
    s32 scaled_index;
    s32 index;
    u8 *slot;
    s32 slot_offset;
    u8 *actor_base;
    u8 *ae0_base;

    i = 0;
    actor_base = g_field_actor_slots;
    ae0_base = D_80105AE0;
    flag = D_8010A020;
    rec = D_80105880;
restart_slots:
    {
        if (*flag != 0 && *(s32 *)(rec + 0x0) == 2)
        {
            slot_offset = *(s32 *)(rec + 0x18) * 0x244;
            *(s32 *)(slot_offset + (u32)actor_base + 0xC) = *(s32 *)(slot_offset + (u32)actor_base + 0x10);
            field_start_actor_animation(*(s32 *)(rec + 0x18), 0, 0);
            index = *(s32 *)(rec + 0xC);
            if (index >= 3)
            {
                index = 2;
            }
            scaled_index = index * 8;
            (ae0_base + (((scaled_index + index) * 0x10) - index) * 4)[0x179] = *(u8 *)(rec + 0x18);
            *flag = 0;
            slot = (u8 *)(*(s32 *)(rec + 0x18) * 0x244 + (u32)actor_base);
            slot[0x2A] = 1;
        }
        flag += 1;
        i += 1;
        rec += 0x1C;
    }
    if (i < 3)
    {
        goto restart_slots;
    }
}
