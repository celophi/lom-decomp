#include "common.h"

typedef struct FieldStateB168C
{
    u8 pad0[0x400];
    u32 flags;
} FieldStateB168C;

extern s32 D_8010AE78;
extern u8 *D_80122B74;
extern FieldStateB168C *D_80122B78;

/**
 * @brief Apply an actor-state mode to the first three field actor slots.
 * @param mode Mode controlling which active actor slots receive the state update.
 */
void func_800B168C(s32 mode)
{
    s32 i;
    s32 offset;
    u8 *slot;

    i = 0;
    offset = 0;
    do
    {
        slot = D_80122B74 + offset;
        if (slot[0x5F0] != 0)
        {
            switch (mode)
            {
            case 1:
            case 3:
                func_80087FC0(i, 2);
                break;
            case 2:
                if ((slot[0x608] >> 7) != 0)
                {
                    func_80087FC0(i, 2);
                }
                break;
            }
        }
        offset += 0x250;
        i++;
    } while (i < 3);

    D_8010AE78 = 1;
    D_80122B78->flags = (D_80122B78->flags & 0xFFF9FFFF) | ((mode & 3) << 17);
}
