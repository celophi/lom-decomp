#include "common.h"

typedef struct FieldStateB177C
{
    u8 pad0[0x400];
    u32 flags;
} FieldStateB177C;

extern s32 D_8010AE78;
extern u8 *D_80122B74;
extern FieldStateB177C *D_80122B78;
extern void func_800C1D14(s32 arg0, s32 arg1);

/**
 * @brief Consume the pending actor-state mode for the first three field actor slots.
 */
void func_800B177C(void)
{
    s32 i;
    s32 mode;
    u8 *slot;

    i = 0;
    do
    {
        mode = (D_80122B78->flags >> 17) & 3;
        switch (mode)
        {
        case 1:
        case 3:
            slot = D_80122B74 + i * 0x250;
            if ((slot[0x608] >> 7) != 0)
            {
                func_80087FC0(i, 0);
                func_800C1D14(i, 0);
            }
            else
            {
                func_80087FC0(i, 1);
                func_800C1D14(i, 0);
            }
            break;
        case 2:
            slot = D_80122B74 + i * 0x250;
            if ((slot[0x608] >> 7) != 0)
            {
                func_80087FC0(i, 0);
                func_800C1D14(i, 0);
            }
            break;
        }
        i++;
    } while (i < 3);

    D_80122B78->flags &= 0xFFF9FFFF;
    D_8010AE78 = 0;
}
