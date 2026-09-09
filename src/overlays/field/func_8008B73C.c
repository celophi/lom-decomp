#include "common.h"

extern s32 D_8010A020[];
extern u8 D_80105880[];
extern u8 g_field_actor_slots[];
extern u8 D_80105AE0[];

s32 field_start_actor_animation(s32, s32, s32);

/**
 * @brief Restart animations for three pending actor slots and clear their update flags.
 * @note WIP: an extra saved register enlarges the frame and changes scheduling.
 */
void func_8008B73C(void)
{
    s32 i;
    s32 *flag;
    u8 *rec;
    s32 v0;
    s32 v1;
    u8 *slot;
    u8 *ae0;

    i = 0;
    flag = D_8010A020;
    rec = D_80105880;
    do
    {
        if (*flag != 0 && *(s32 *)(rec + 0x0) == 2)
        {
            slot = g_field_actor_slots + *(s32 *)(rec + 0x18) * 0x244;
            *(s32 *)(slot + 0xC) = *(s32 *)(slot + 0x10);
            field_start_actor_animation(*(s32 *)(rec + 0x18), 0, 0);
            v1 = *(s32 *)(rec + 0xC);
            if (v1 >= 3)
            {
                v1 = 2;
            }
            v0 = v1 * 8;
            ae0 = D_80105AE0 + (((v0 + v1) * 0x10) - v1) * 4;
            ae0[0x179] = *(u8 *)(rec + 0x18);
            *flag = 0;
            slot = g_field_actor_slots + *(s32 *)(rec + 0x18) * 0x244;
            slot[0x2A] = 1;
        }
        flag += 1;
        i += 1;
        rec += 0x1C;
    } while (i < 3);
}
