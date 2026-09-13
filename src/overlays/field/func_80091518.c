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
    Record *record;
    Map *default_map;
    Map *secondary_map;
    u8 *selected_map;
    s16 *animation_id;
    s32 index;
    s32 map_base;
    u8 *disabled_entry;
    s32 action_2_animation_id;
    s32 action_3_animation_id;
    s32 action_1_animation_id;
    s32 action_0_animation_id;
    s32 action_8_animation_id;
    s32 action_9_animation_id;
    s32 action_10_animation_id;
    s32 reset_byte_value;
    s32 one_value;
    s32 sixteen_value;
    s32 action_5_animation_id;
    s32 action_7_animation_id;
    s32 action_4_animation_id;
    s32 action_6_animation_id;

    index = 0;
    map_base = (s32)g_field_action_animation_maps;
    animation_id = (s16 *)(arg0 * 0x22 + map_base);
    selected_map = (u8 *)animation_id;
    do
    {
        do { *animation_id = 0; } while (0);
        disabled_entry = selected_map + index;
        index += 1;
        *(u8 *)(disabled_entry + 0x16) = 0;
        animation_id++;
    } while (index < 8);
    index = 0;
    action_2_animation_id = 0x285;
    action_3_animation_id = 0x385;
    action_1_animation_id = 0x185;
    action_0_animation_id = 0x85;
    action_8_animation_id = 0x885;
    action_9_animation_id = 0x985;
    action_10_animation_id = 0xA85;
    default_map = (Map *)g_field_action_animation_maps;
    do
    {
        default_map->half[2] = action_2_animation_id;
        default_map->half[3] = action_3_animation_id;
        default_map->half[1] = action_1_animation_id;
        default_map->half[0] = action_0_animation_id;
        default_map->half[8] = action_8_animation_id;
        default_map->half[9] = action_9_animation_id;
        default_map->half[10] = action_10_animation_id;
        default_map++;
    } while (++index < 2);
    index = 0;
    reset_byte_value = 0xFF;
    one_value = 1;
    sixteen_value = 0x10;
    record = D_8010A038;
    do
    {
        record->f10.half = 0x1F;
        record->f18.half = 0x25;
        record->f40.half = 0x27;
        record->f12.half = (u16)(record->f12.half & 0xFBFF);
        record->f1A.half = (u16)(record->f1A.half & 0xFBFF);
        record->f12.byte[0] = reset_byte_value;
        record->fA.half = (u16)(record->fA.half & 0xFBFF);
        record->f2.half = (u16)(record->f2.half & 0xFBFF);
        record->f1A.byte[0] = reset_byte_value;
        record->fA.byte[0] = reset_byte_value;
        record->f42.half = (u16)(record->f42.half & 0xFBFF);
        record->f12.half = (u16)(record->f12.half & 0xFCFF);
        record->f14.half = 0;
        record->f16.half = one_value;
        record->f1C.half = 0;
        record->f1E.half = one_value;
        record->f44.half = 0;
        record->f46.half = sixteen_value;
        record->f2.byte[0] = reset_byte_value;
        record->f42.byte[0] = reset_byte_value;
        record->f1A.half = (u16)(record->f1A.half & 0xFCFF);
        record->fA.half = (u16)(record->fA.half & 0xFCFF);
        record->f2.half = (u16)(record->f2.half & 0xFCFF);
        record->f42.half = (u16)(record->f42.half & 0xFCFF);
        record->f48.half = 0x28;
        record->f50.half = 0x29;
        index += 1;
        record->f4C.half = 0;
        record->f4E.half = sixteen_value;
        record->f54.half = 0;
        record->f56.half = sixteen_value;
        record->f4A.half = (u16)(record->f4A.half & 0xFBFF);
        record->f4A.byte[0] = reset_byte_value;
        record->f52.half = (u16)(record->f52.half & 0xFBFF);
        record->f52.byte[0] = reset_byte_value;
        record->f4A.half = (u16)(record->f4A.half & 0xFCFF);
        record->f52.half = (u16)(record->f52.half & 0xFCFF);
        record++;
    } while (index < 3);
    index = 0;
    action_5_animation_id = 0x585;
    action_7_animation_id = 0x785;
    action_4_animation_id = 0x485;
    action_6_animation_id = 0x685;
    secondary_map = (Map *)g_field_action_animation_maps;
    do
    {
        secondary_map->half[5] = action_5_animation_id;
        secondary_map->half[7] = action_7_animation_id;
        secondary_map->half[4] = action_4_animation_id;
        secondary_map->half[6] = action_6_animation_id;
        secondary_map++;
    } while (++index < 2);
}
