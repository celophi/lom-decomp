#include "common.h"

/** @brief World-map actor configuration. */
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

/** @brief Auxiliary sequence state and counter fields. */
typedef struct
{
    s16 state;
    s16 counter;
    u8 unknown_04[4];
    s32 value;
    s16 unknown_0c;
    s16 flags;
} WmapAuxState;

extern void func_80088B38(void);
extern WmapConfigA D_800D9CB8;
extern u8 D_80121538[];
extern s32 D_80139234;
extern s32 D_8013923C;
extern u8 *D_80139B6C;
extern WmapAuxState D_801B0080;
extern s32 D_801B2988;
extern s32 D_801B298C;

/** @brief Initialize the actor and auxiliary state for the next timed step. */
void func_80089FF4(void)
{
    D_80139234 = 0;
    D_8013923C = 2;
    D_80139B6C = D_80121538;
    D_800D9CB8.field_06 = 0xF;
    D_800D9CB8.field_10 = -1;
    D_800D9CB8.field_26 = 8;
    D_800D9CB8.field_02 = 0;
    D_800D9CB8.field_0E = 0;
    D_800D9CB8.field_22 = 0x81;
    D_800D9CB8.field_24 = 0x81;
    D_801B0080.state = 1;
    D_801B0080.value = 0;
    D_801B0080.counter = 0;
    D_801B0080.flags = 0;
    D_801B298C = 0x70;
    D_801B2988 += 1;
    func_80088B38();
}
