#include "common.h"

/** @brief Halfword view of one 0x22-byte action animation map. */
typedef struct
{
    u16 half[17];
} Map;
/** @brief Packed halfword with a separately writable low byte. */
typedef union
{
    u16 half;
    u8 byte[2];
} Half;
/** @brief Command fields in one 0x190-byte animation record. */
typedef struct
{
    Half f0;
    Half f2;
    Half f4;
    Half f6;
    Half f8;
    Half fA;
    Half fC;
    Half fE;
    Half f10;
    Half f12;
    Half f14;
    Half f16;
    Half f18;
    Half f1A;
    Half f1C;
    Half f1E;
    Half f20;
    Half f22;
    Half f24;
    Half f26;
    Half f28;
    Half f2A;
    Half f2C;
    Half f2E;
    Half f30;
    Half f32;
    Half f34;
    Half f36;
    Half f38;
    Half f3A;
    Half f3C;
    Half f3E;
    Half f40;
    Half f42;
    Half f44;
    Half f46;
    Half f48;
    Half f4A;
    Half f4C;
    Half f4E;
    Half f50;
    Half f52;
    Half f54;
    Half f56;
    u8 pad58[0x190 - 0x58];
} Record;
extern Record D_8010A038[];
extern u8 g_field_action_animation_maps[];
/**
 * @brief Reset the selected action map and restore shared animation command defaults.
 * @param arg0 Action map index to clear before restoring defaults.
 */
void func_80091518(s32 arg0)
{
    Record *var_a0;
    Map *var_v1_2;
    Map *var_v1_3;
    u8 *temp_a0;
    s16 *var_v1;
    s32 var_a1;
    u8 *temp_v0;

    var_a1 = 0;
    var_v1 = (s16 *)g_field_action_animation_maps;
    var_v1 = (s16 *)(arg0 * 0x22 + (s32)var_v1);
    temp_a0 = (u8 *)var_v1;
    do
    {
        *var_v1 = 0;
        temp_v0 = temp_a0 + var_a1;
        var_a1 += 1;
        *(u8 *)(temp_v0 + 0x16) = 0;
        var_v1++;
    } while (var_a1 < 8);
    var_a1 = 0;
    var_v1_2 = (Map *)g_field_action_animation_maps;
    do
    {
        var_v1_2->half[2] = 0x285;
        var_v1_2->half[3] = 0x385;
        var_v1_2->half[1] = 0x185;
        var_v1_2->half[0] = 0x85;
        var_v1_2->half[8] = 0x885;
        var_v1_2->half[9] = 0x985;
        var_v1_2->half[10] = 0xA85;
        var_a1 += 1;
        var_v1_2++;
    } while (var_a1 < 2);
    var_a1 = 0;
    var_a0 = D_8010A038;
    do
    {
        var_a0->f10.half = 0x1F;
        var_a0->f18.half = 0x25;
        var_a0->f40.half = 0x27;
        var_a0->f12.half = (u16)(var_a0->f12.half & 0xFBFF);
        var_a0->f1A.half = (u16)(var_a0->f1A.half & 0xFBFF);
        var_a0->f12.byte[0] = 0xFF;
        var_a0->fA.half = (u16)(var_a0->fA.half & 0xFBFF);
        var_a0->f2.half = (u16)(var_a0->f2.half & 0xFBFF);
        var_a0->f1A.byte[0] = 0xFF;
        var_a0->fA.byte[0] = 0xFF;
        var_a0->f42.half = (u16)(var_a0->f42.half & 0xFBFF);
        var_a0->f12.half = (u16)(var_a0->f12.half & 0xFCFF);
        var_a0->f14.half = 0;
        var_a0->f16.half = 1;
        var_a0->f1C.half = 0;
        var_a0->f1E.half = 1;
        var_a0->f44.half = 0;
        var_a0->f46.half = 0x10;
        var_a0->f2.byte[0] = 0xFF;
        var_a0->f42.byte[0] = 0xFF;
        var_a0->f1A.half = (u16)(var_a0->f1A.half & 0xFCFF);
        var_a0->fA.half = (u16)(var_a0->fA.half & 0xFCFF);
        var_a0->f2.half = (u16)(var_a0->f2.half & 0xFCFF);
        var_a0->f42.half = (u16)(var_a0->f42.half & 0xFCFF);
        var_a0->f48.half = 0x28;
        var_a0->f50.half = 0x29;
        var_a1 += 1;
        var_a0->f4C.half = 0;
        var_a0->f4E.half = 0x10;
        var_a0->f54.half = 0;
        var_a0->f56.half = 0x10;
        var_a0->f4A.half = (u16)(var_a0->f4A.half & 0xFBFF);
        var_a0->f4A.byte[0] = 0xFF;
        var_a0->f52.half = (u16)(var_a0->f52.half & 0xFBFF);
        var_a0->f52.byte[0] = 0xFF;
        var_a0->f4A.half = (u16)(var_a0->f4A.half & 0xFCFF);
        var_a0->f52.half = (u16)(var_a0->f52.half & 0xFCFF);
        var_a0++;
    } while (var_a1 < 3);
    var_a1 = 0;
    var_v1_3 = (Map *)g_field_action_animation_maps;
    do
    {
        var_v1_3->half[5] = 0x585;
        var_v1_3->half[7] = 0x785;
        var_v1_3->half[4] = 0x485;
        var_v1_3->half[6] = 0x685;
        var_a1 += 1;
        var_v1_3++;
    } while (var_a1 < 2);
}
