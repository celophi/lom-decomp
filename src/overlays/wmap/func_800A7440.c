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

/** @brief Auxiliary callback state. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    s16 unknown_0c;
    s16 field_0E;
    s16 field_10;
} WmapAuxState;

extern void func_800591A8(s32);
extern void func_8006CBD8(void (*callback)(void));
extern void func_800A72B8(void);
extern WmapConfigA D_800DBE3C;
extern u8 D_8011D538[];
extern u8 *D_8013A184;
extern WmapAuxState D_801AFBD0;
extern s32 D_801B2E70;
extern s32 D_801B2E74;
extern void func_800999D0(void);

/** @brief Initialize the actor, register its callback, and start the sequence delay. */
void func_800A7440(void)
{
    D_8013A184 = D_8011D538;
    D_800DBE3C.field_10 = -1;
    D_800DBE3C.field_02 = 0;
    D_800DBE3C.field_06 = 0;
    D_800DBE3C.field_0E = 0;
    D_800DBE3C.field_26 = 0;
    D_800DBE3C.field_22 = 0x80;
    D_800DBE3C.field_24 = 0x80;
    D_801AFBD0.field_00 = 1;
    D_801AFBD0.field_08 = 0xC8;
    D_801AFBD0.field_0E = 0xA;
    D_801AFBD0.field_10 = 0x3C;
    D_801AFBD0.field_02 = 0;
    D_801AFBD0.field_04 = 2;
    func_8006CBD8(&func_800999D0);
    func_800591A8(0x21);
    D_801B2E74 = 0x16E;
    D_801B2E70 += 1;
    func_800A72B8();
}
