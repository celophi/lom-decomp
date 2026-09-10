#include "common.h"

/** @brief Three fixed-point coordinates at the head of the field actor record. */
typedef struct
{
    s32 x, y, z;
} FieldSelectionPosition;
/** @brief Two halfword state fields cleared when selection begins. */
typedef struct
{
    s16 first, second;
} FieldSelectionState;
u32 field_load_vram_resource(s32, s16 *, s32); /* extern */
void func_800AA02C(void);                      /* extern */
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern FieldSelectionPosition D_800FDF58;
extern FieldSelectionState D_801077FC;
extern s16 D_8011F330;
extern s32 D_8011F334;
extern s32 D_8011F338;
extern s32 D_8011F33C;
extern s32 D_8011F340;
extern s32 D_8011F344;
extern s32 D_8011F348;
extern s32 D_8011F34C;
extern s32 D_8011F350;
extern s32 D_8011F354;
extern u8 D_8011F358[];
extern s32 D_8011F378;
extern s32 D_8011F37C;
extern s32 D_8011F380;
extern s8 D_8011F388[];
extern s32 D_8011F3A8;
extern s32 D_8011F3AC;
extern s32 D_8011F3B4;
extern s32 D_8011F3B8;
extern s32 D_8011F3BC;
extern s32 D_8011F3C0;
extern s32 D_8011F3C4;

/**
 * @brief Initialize a resource-backed selection display and its rotation state.
 * @param position_mode Zero centers the display; one follows the first actor.
 * @param resource_index Resource offset and saved-selection index.
 * @param excluded_mask Bit mask of resource entries omitted from selection.
 * @param cancel_index Selection used when cancelling, or minus one to disable it.
 * @note The six-halfword load buffer also holds the signed screen coordinates.
 * @note Subtracting the negated offset preserves the original addition operand order.
 * @note GCC 2.7.2 CDK matches all 215 instructions (860 bytes).
 */
void func_800A43E8(s32 position_mode, s32 resource_index, u16 excluded_mask, s32 cancel_index)
{
    s16 load_params[6];
    s16 screen_x;
    s16 screen_y;
    s32 rotation;
    s32 unused_count;
    s32 unused_actor_x;
    s32 unused_actor_z;
    s32 bit;
    s32 unused_actor_y;
    s32 camera_x;
    s32 camera_y;
    s32 unused_camera_z;
    s32 index;
    u32 resource_info;
    u8 *saved_index;
    u8 selection;

    if (D_8011F3AC == 0)
    {
        func_800AA02C();
        D_8011F37C = cancel_index;
        D_8011F330 = excluded_mask;
        load_params[0] = 0x140;
        load_params[1] = 0;
        load_params[2] = 0;
        load_params[3] = 0x1F2;
        resource_info = field_load_vram_resource(resource_index + 0xBE8, &load_params[0], 1);
        bit = 1;
        index = 0;
        D_8011F348 = (resource_info & 0x1F) * 8;
        D_8011F34C = (resource_info >> 2) & 0xF8;
        D_8011F3C4 = (resource_info >> 0xA) & 0x1F;
        D_8011F3B8 = 0;
        if (D_8011F3C4 != 0)
        {
            do
            {
                if (!(excluded_mask & 0xFFFF & bit))
                {
                    D_8011F388[D_8011F3B8] = index;
                    D_8011F3B8 += 1;
                }
                index += 1;
                bit *= 2;
            } while (index < D_8011F3C4);
        }
        D_8011F350 = 0x100;
        D_8011F354 = (resource_info >> 0xD) & 0xF8;
        D_8011F338 = 0x20;
        saved_index = resource_index + D_8011F358;
        selection = *saved_index;
        if ((s32)selection >= D_8011F3B8)
        {
            *saved_index = 0;
            D_8011F378 = 0;
        }
        else
        {
            D_8011F378 = (s32)selection % (s32)D_8011F3B8;
        }
        rotation = -(0x1000 / (s32)D_8011F3B8) * D_8011F378;
        D_8011F334 = resource_index;
        D_8011F380 = 0;
        D_8011F3C0 = rotation;
        D_8011F33C = rotation;
        switch (position_mode)
        { /* irregular */
        case 0:
            D_8011F340 = 0xA0;
            D_8011F344 = 0x70;
            break;
        case 1:
            camera_x = D_800F22A0 / 256;
            screen_x = D_800FDF58.x / 256 + 160;
            screen_x = camera_x - (-screen_x);
            load_params[4] = screen_x;
            camera_y = D_800F22A4 / 256;
            screen_y = D_800FDF58.y / 256 + 112;
            screen_y = camera_y - (-screen_y);
            screen_y -= D_800FDF58.z / 512;
            screen_y -= D_800F22A8 / 512;
            load_params[5] = screen_y;
            D_8011F340 = load_params[4];
            D_8011F344 = load_params[5];
            break;
        }
        D_8011F3AC = 2;
        D_8011F3BC = 0;
        D_8011F3B4 = 1;
        D_8011F3A8 = 0;
        D_801077FC.second = 0;
        D_801077FC.first = 0;
    }
}
