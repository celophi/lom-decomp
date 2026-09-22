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


/** @brief World-map 8-byte slot: only the +4 pointer field is written here. */
typedef struct
{
    s32 field_00;
    void *field_04;
} WmapSlot8;

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

extern WmapConfigA D_800D9268[];
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern u8 D_8011F538[];
extern s32 D_801B25D8;
extern s32 D_800DCEAC;
extern s32 D_800D9154;
extern s32 D_801B2E20;
extern s32 D_801B2E24;
extern s32 rand(void);
extern void func_800A5318(void);

/** @brief Reset the effect actors with randomized animation variants. */
void func_800A3A2C(void)
{
    s32 i;

    D_801B25D8 = 1;
    D_800DCEAC = 1;
    for (i = 80; i < 140; i++)
    {
        D_801AFBD0[i].field_00 = 0;
        D_80139988[i].field_04 = D_8011F538;
        D_800D9268[i].field_02 = 0;
        D_800D9268[i].field_06 = 15;
        D_800D9268[i].field_0E = rand() % 3 + 2;
        D_800D9268[i].field_10 = -1;
    }
    D_800D9154 = 2;
    D_801B2E24 = 16;
    D_801B2E20++;
    func_800A5318();
}
