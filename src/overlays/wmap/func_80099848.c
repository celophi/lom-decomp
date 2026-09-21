#include "common.h"

/** @brief World-map actor record reset when this step spawns it. */
typedef struct
{
    u8 pad_00[2];
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
    u8 pad_28[2];
} WmapActor;

/** @brief World-map slot record populated when spawning this actor. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    s32 field_04;
    s32 field_08;
    u8 pad_0C[2];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[2];
} WmapSlotB;

extern WmapActor D_800DBE3C;
extern u8 D_80182E40[];
extern u8 D_8018B240[];
extern void *D_8013A184;
extern s32 D_8011D538;
extern WmapSlotB D_801AFBD0[];
extern s32 D_801B2C4C;
extern s32 D_801B2C50;
extern void func_800651B4(void *arg0);
extern void func_800999D0(void);
extern void func_8006CBD8(void (*callback)(void));
extern void func_80099918(void);

/** @brief World-map step: reset the actor and its slot, register a callback, advance. */
void func_80099848(void)
{
    WmapActor *actor = &D_800DBE3C;

    cdrom_wait_queue_empty();
    func_800651B4(D_80182E40);
    func_800651B4(D_8018B240);
    D_8013A184 = &D_8011D538;
    actor->field_10 = -1;
    actor->field_22 = 0x80;
    actor->field_24 = 0x80;
    actor->field_02 = 0;
    actor->field_06 = 0;
    actor->field_0E = 0;
    actor->field_26 = 0;
    D_801AFBD0[0].field_00 = 1;
    D_801AFBD0[0].field_08 = 0xC8;
    D_801AFBD0[0].field_0E = 0xA;
    D_801AFBD0[0].field_02 = 0;
    D_801AFBD0[0].field_10 = 0x3C;
    D_801AFBD0[0].field_04 = 0;
    func_8006CBD8(func_800999D0);
    D_801B2C50 = 0x3C;
    D_801B2C4C += 1;
    func_80099918();
}
