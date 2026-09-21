#include "common.h"

/** @brief World-map actor configuration with its original field layout. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad_04[2];
    u8 field_06;
    u8 pad_07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad_28[4];
} WmapConfigA;

extern WmapConfigA D_800D9268[];
extern s32 D_80139988[];
extern s32 D_801B0FD8[];
extern s32 D_801B1058[];

/** @brief Initialize actor indices and clear the callback state tables. */
void func_8006CD18(void)
{
    s32 index;
    s32 *callback_state;
    s32 *active;

    for (index = 0; index < 256; index++)
    {
        D_800D9268[index].field_00 = index;
        D_80139988[index * 2] = index;
        D_800D9268[index].field_02 = -1;
    }
    index = 0xD;
    callback_state = D_801B0FD8;
    callback_state += index;
    do
    {
        *callback_state = 0;
        index -= 1;
        callback_state--;
    } while (index >= 0);
    index = 7;
    active = D_801B1058;
    active += index;
    do
    {
        *active = 0;
        index -= 1;
        active--;
    } while (index >= 0);
}
