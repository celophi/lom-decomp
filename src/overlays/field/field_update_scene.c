#include "common.h"
#include "sdk/libgpu.h"
#include "sdk/libetc.h"
#include "sdk/libgte.h"
/** @brief Default scene spawn position and packed facing selector. */
typedef union
{
    /** @brief Signed map coordinates and packed selector halfword. */
    struct
    {
        s16 x, y, z;
        u16 flags;
    } h;
    /** @brief Word-aligned facing bitfield matching the original stack update. */
    struct
    {
        u32 xy;
        u32 z : 16;
        u32 direction : 4;
        u32 unused : 12;
    } bits;
} Spawn;

/** @brief Controller actuator bytes reset when changing scenes. */
typedef struct
{
    u8 pad0[0x91];
    u8 port1_a;
    u8 port1_b;
    u8 pad93[0x13F - 0x93];
    u8 port2_a;
    u8 port2_b;
} Controller;

/** @brief Actor position prefix with the original 0x54-byte record stride. */
typedef struct
{
    s32 x, y, z;
    u8 pad[0x54 - 12];
} Position;

#define U8_AT(p, o) (*(u8 *)((u8 *)(p) + (o)))
#define U16_AT(p, o) (*(u16 *)((u8 *)(p) + (o)))
#define S16_AT(p, o) (*(s16 *)((u8 *)(p) + (o)))
#define S32_AT(p, o) (*(s32 *)((u8 *)(p) + (o)))
#define U32_AT(p, o) (*(u32 *)((u8 *)(p) + (o)))
void akao_cmd_c1();
void akao_cmd_f1();
void akao_song_cmd_12c();
void cdrom_queue_read();
void cdrom_wait_queue_empty();
void field_clear_actor_slots();
void field_init_ctx();
void field_initialize_actor_slots();
void field_initialize_actor_system();
void field_load_map();
void field_stop_song();
void field_text_reset_windows();
u8 *func_800630BC(u16);
void func_80067AA4();
void func_8006A958();
void func_8006C3FC();
void func_80084240();
void func_80084630();
void func_80086F20();
void func_8008C730();
void func_8008C7A8();
void func_80091BC8();
void func_80092394();
void func_8009BCAC();
void func_8009BCF8();
void func_8009BDD4();
void func_8009BE1C();
void func_8009C12C();
void func_8009C2E0();
void func_8009C434();
void func_8009C4B4();
void func_8009C56C();
void func_800A255C();
void func_800A2DFC();
void func_800A35F4();
void func_800A3654();
void func_800A368C();
void func_800A3728();
void func_800A380C();
void func_800A3BE8();
void func_800A54D0();
void func_800A6204();
void func_800A6EEC();
void func_800A8D10();
void func_800AF8C4();
void func_800B0094();
void func_800B01FC();
void func_800B34D0();
extern s32 D_800473F0;
extern u8 D_800EB0A4[];
extern u8 D_800EB254[];
extern s32 D_800F22B0;
extern u8 D_800FDF58[];
extern u8 D_800FE054[];
extern s32 D_800FE754;
extern u8 D_800FF610[];
extern s32 D_800FF650;
extern s32 D_8010A018;
extern s32 D_8010A02C;
extern s32 D_8010A030;
extern u8 D_8010A038[];
extern s32 D_8010AE48;
extern s32 D_8010AE4C;
extern s32 D_8010AE50;
extern s32 D_8010D010;
extern s32 D_8010D014;
extern s32 D_8010D020;
extern s32 D_8010D028;
extern u8 D_8010D088[];
extern s32 D_80115888;
extern s32 D_8011588C;
extern s32 D_80115890;
extern s32 D_80115894;
extern s32 D_80115898;
extern s32 D_8011589C;
extern s32 D_801158A0;
extern s32 D_801158A8;
extern s32 D_801178B0;
extern s32 D_801178B4;
extern s32 D_801178BC;
extern s32 D_801178C0;
extern u32 D_801178CC;
extern s32 D_801178D0;
extern u8 *D_801178D4;
extern s32 D_80122908;

extern s32 g_field_audio_timer;
extern s32 g_field_resource_cursor;
extern u8 g_field_resource_entries[];
extern s32 g_field_scene_request_pending;
extern s32 g_layout_flag;
extern s32 g_layout_option;
extern s32 g_layout_sub_mode;
extern s32 g_scene_mode;

/**
 * @brief Complete a pending scene change and initialize its runtime resources.
 * @note Coordinates disc reads, audio transitions, resource installation, map state, and actor placement.
 * @note The scene-file header at 0x80190000 stores byte offsets relative to that buffer.
 */
void field_update_scene(void)
{
    Spawn spawn;
    RECT rect;
    VECTOR offset;
    u8 *actor_description_base;
    u8 *image_base;
    u8 *geometry_base;
    u8 *layout_base;
    u8 *persistent_map;
    s32 actor_end;
    s32 context_id;
    s32 spawn_id;
    s32 layout_option;
    s32 tile_offset;
    u8 *scene_buffer;
    volatile Controller *controller;
    u8 *unused_actors;
    u8 *spawn_record;
    u8 *list_header;
    u8 *direction_entry;
    u8 *list_source;
    u8 *list_output;
    u8 *actor_flags;
    u8 *actor_position;
    u8 *scene_cursor;
    Position *initial_position;
    s32 direction_flag;
    s32 first_image;
    s32 scene_id;
    s32 tile_count;
    s32 inherited_tile_count;
    s32 previous_mode;
    s32 temp_v0_2;
    s32 initial_x;
    s32 saved_scene_id;
    s32 image_palette;
    s32 tile_index;
    s32 inherited_tile_index;
    s32 resource_offset;
    s32 image_index;
    s32 copy_index;
    s32 image_count;
    s32 actor_index;
    s32 tile_destination_offset;
    u8 *resource_base;
    u8 *tile_base;
    u8 *spacing_base;
    u8 *direction_base;
    s32 inherited_tile_destination_offset;
    s8 actor_mode;
    s32 palette_index;
    u8 *tile_ids;
    u8 *inherited_tile_ids;
    u8 *section_20;
    u8 *section_04;
    u32 animation_count;
    u8 *section_08;
    u8 *section_0c;
    u8 *section_10;
    u8 *section_source;
    u8 *second_section_source;
    u8 *third_section_source;
    u8 *fourth_section_source;
    u8 *animation_source;
    u8 *resource_output;
    u8 *image_scan;
    u8 *image_cursor;
    u8 temp_v0_3;
    u8 temp_v0_4;
    u8 temp_v0_5;
    u8 temp_v0_6;
    u8 temp_v0_7;
    u8 image_id;
    u8 *resource_fields;
    u8 *tile;
    u8 *inherited_tile;
    u8 *resource;
    u8 *inherited_resource;
    u8 *description;
    u8 *temp_s6_2;
    u8 *previous_resource;
    u8 *tile_fields;
    u8 *inherited_tile_fields;
    u8 *geometry_offsets;

    controller = (volatile Controller *)0x801ED600;
    persistent_map = (u8 *)0x801ED480;
    if (g_field_scene_request_pending != 0)
    {
        controller->port2_a = 0;
        controller->port1_a = 0;
        controller->port2_b = 0;
        controller->port1_b = 0;
        if (g_field_audio_timer != 0)
        {
            akao_song_cmd_12c();
            if (D_80115898 == -1)
            {
                func_800A380C();
                akao_cmd_c1(0, 1, D_8011588C);
            }
            g_field_audio_timer = 0;
        }
        func_8009C56C();
        func_8009C4B4();
        scene_id = D_801178B0;
        g_field_scene_request_pending = 0;
        context_id = D_801178BC;
        spawn_id = D_80115888;
        layout_option = D_801178C0;
        DrawSync(0);
        previous_mode = D_801158A0;
        D_801158A0 = scene_id & 0x8000;
        scene_id = scene_id & 0x7FFF;
        if (D_80115898 != -1)
        {
            if (D_80115894 == 0)
            {
                akao_cmd_c1(0, 0x3C, 0);
            }
        }
        scene_buffer = (u8 *)0x80190000;
        cdrom_queue_read((scene_id + 0x60C) & 0xFFFF, scene_buffer);
        actor_index = 0;
        cdrom_wait_queue_empty();
        func_8009C4B4();
        g_scene_mode = scene_id;
        D_800FE754 = 0;
        D_8010D020 = 0;
        func_800B01FC();
        func_800A6204();
        akao_cmd_f1();
        field_initialize_actor_system();
        func_8009C4B4();
        initial_position = (Position *)D_800FDF58;
        initial_x = 0xA000;
        do
        {
            initial_position->x = initial_x;
            initial_position->y = 0;
            initial_position->z = 0xE000;
            initial_position++;
            actor_index += 1;
            initial_x += 0x2800;
        } while (actor_index < 3);
        if (previous_mode != D_801158A0)
        {
            func_800A35F4(D_801158A0 == 0);
        }
        func_8009C4B4();
        list_output = D_800FF610;
        D_80122908 = 0;
        scene_cursor = scene_buffer + S32_AT(scene_buffer, 0x14);
        list_header = scene_buffer + S32_AT(scene_buffer, 0x24);
        list_source = list_header + 4;
        actor_index = S32_AT(list_header, 0);
        actor_description_base = scene_cursor;
        D_800FF650 = actor_index;
        geometry_base = scene_buffer + S32_AT(scene_buffer, 0x18);
        image_base = scene_buffer + S32_AT(scene_buffer, 0x1C);
        layout_base = scene_buffer + S32_AT(scene_buffer, 0x00);
        section_08 = scene_buffer + S32_AT(scene_buffer, 0x08);
        section_04 = scene_buffer + S32_AT(scene_buffer, 0x04);
        section_0c = scene_buffer + S32_AT(scene_buffer, 0x0C);
        section_10 = scene_buffer + S32_AT(scene_buffer, 0x10);
        section_20 = scene_buffer + S32_AT(scene_buffer, 0x20);
        if (actor_index != 0)
        {
            do
            {
                temp_v0_2 = S32_AT(list_source, 0);
                list_source += 4;
                actor_index -= 1;
                S32_AT(list_output, 0) = temp_v0_2;
                list_output += 4;
            } while (actor_index != 0);
        }
        resource_output = D_8010D088;
        section_source = section_08;
        D_801178D4 = D_8010D088;
        if (section_source != section_0c)
        {
            do
            {
                temp_v0_3 = *section_source;
                section_source += 1;
                *resource_output = temp_v0_3;
                resource_output += 1;
            } while (section_source != section_0c);
        }
        second_section_source = section_04;
        D_8010A018 = (s32)(resource_output + 3) & ~3;
        if (second_section_source != section_08)
        {
            do
            {
                temp_v0_4 = *second_section_source;
                second_section_source += 1;
                *resource_output = temp_v0_4;
                resource_output += 1;
            } while (second_section_source != section_08);
        }
        third_section_source = section_0c;
        D_8010A02C = (s32)(resource_output + 3) & ~3;
        if (third_section_source != section_10)
        {
            do
            {
                temp_v0_5 = *third_section_source;
                third_section_source += 1;
                *resource_output = temp_v0_5;
                resource_output += 1;
            } while (third_section_source != section_10);
        }
        fourth_section_source = section_10;
        D_8010A030 = (s32)(resource_output + 3) & ~3;
        if (fourth_section_source != scene_cursor)
        {
            do
            {
                temp_v0_6 = *fourth_section_source;
                fourth_section_source += 1;
                *resource_output = temp_v0_6;
                resource_output += 1;
            } while (fourth_section_source != scene_cursor);
        }
        animation_source = section_20 + 4;
        actor_index = 0;
        animation_count = U32_AT(section_20, 0);
        D_801178D0 = (s32)(resource_output + 3) & ~3;
        D_801178CC = animation_count;
        if (animation_count != 0)
        {
            do
            {
                copy_index = 0;
            copy_animation_record:
                copy_index += 1;
                temp_v0_7 = *animation_source;
                animation_source += 1;
                *resource_output = temp_v0_7;
                resource_output += 1;
                if (copy_index < 0x4A0)
                {
                    goto copy_animation_record;
                }
                actor_index += 1;
            } while ((u32)actor_index < (u32)D_801178CC);
        }
        D_800473F0 = resource_output - D_8010D088;
        D_801158A8 = D_8011588C;
        D_8011588C = (s32)U16_AT(scene_cursor, 0x0);
        D_801178B4 = (s32)U16_AT(scene_cursor, 0x2);
        scene_cursor += 4;
        func_8009C434();
        actor_index = 3;
        func_8006A958((u16)U16_AT(scene_cursor, 0) >> 0xF);
        D_8010AE48 = 0;
        func_8009C4B4();
        unused_actors = D_800FE054;
        U16_AT(persistent_map, 0) = U16_AT(scene_cursor, 0) & 0x7FFF;
        scene_cursor += 2;
        actor_end = U16_AT(scene_cursor, 0) + 3;
        scene_cursor = scene_cursor + 2;
        do
        {
            U8_AT(unused_actors, 0x25) = 0xFF;
            actor_index += 1;
            unused_actors += 0x54;
        } while (actor_index < 0xD);
        palette_index = 2;
        D_80115890 = 0;
        func_8009BCAC();
        actor_index = 0;
        if ((actor_end - 3) > 0)
        {
            geometry_offsets = geometry_base;
            resource_offset = 0x3C;
            tile_offset = 0x4B0;
            do
            {
                image_count = 0;
                description = actor_description_base + S32_AT(scene_cursor, 0);
                image_scan = description + 2;
                if (U8_AT(description, 0x2) != 0xFF)
                {
                    image_count = 1;
                scan_image_list:
                    image_scan += 1;
                    if (image_count != 6)
                    {
                        image_count += 1;
                        if (*image_scan == 0xFF)
                        {
                            image_count -= 1;
                        }
                        else
                        {
                            goto scan_image_list;
                        }
                    }
                }
                palette_index += image_count;
                if ((u32)(palette_index - 6) < 4U)
                {
                    palette_index += 4;
                }
                if (image_count != 0)
                {
                    resource_base = g_field_resource_entries;
                    resource_fields = resource_offset + resource_base;
                    U8_AT(resource_fields, 0x9) = palette_index;
                    U16_AT(resource_fields, 0xA) = (u16)U16_AT(description, 0x8);
                    if (D_80115890 != 0)
                    {
                        U8_AT(resource_fields, 0x8) = 0;
                    }
                    else
                    {
                        U8_AT(resource_fields, 0x8) = (s8)(image_count != 1);
                    }
                    D_80115890 = image_count != 1;
                    resource_base = g_field_resource_entries;
                    resource = resource_offset + resource_base;
                    S32_AT(resource, 0x0) = (s32)g_field_resource_cursor;
                    S32_AT(resource, 0x10) =
                        (s32)((S32_AT(resource, 0x10) & ~1) | ((u8)U8_AT(description, 0x1) >> 7));
                    func_8009BDD4(geometry_base + S32_AT(geometry_offsets, 0x0),
                                  geometry_base + S32_AT(geometry_offsets, 0x4));
                    S32_AT(resource, 0x4) = (s32)g_field_resource_cursor;
                    S32_AT(resource, 0x10) = (s32)(S32_AT(resource, 0x10) | 2);
                    tile_ids = description + 0x10;
                    U16_AT(resource, 0xE) = (u16)U16_AT(description, 0xA);
                    tile_count = U16_AT(description, 0xC);
                    tile_index = 0;
                    if (tile_count > 0)
                    {
                        tile_fields = description + 0x12;
                        tile_destination_offset = tile_offset;
                        do
                        {
                            tile_base = D_8010A038;
                            tile = tile_destination_offset + tile_base;
                            U16_AT(tile, 0x2) =
                                (u16)((U16_AT(tile, 0x2) & 0xFBFF) | (U16_AT(tile_fields, 0x0) & 0x400));
                            U16_AT(tile, 0x0) = (u16)U16_AT(tile_ids, 0);
                            tile_destination_offset += 8;
                            U8_AT(tile, 0x2) = (u8)U16_AT(tile_fields, 0x0);
                            tile_index += 1;
                            U16_AT(tile, 0x4) = (u16)U16_AT(tile_fields, 0x2);
                            tile_ids += 8;
                            U16_AT(tile, 0x6) = (u16)U16_AT(tile_fields, 0x4);
                            U16_AT(tile, 0x2) =
                                (u16)((U16_AT(tile, 0x2) & 0xFCFF) | (U16_AT(tile_fields, 0x0) & 0x300));
                            tile_fields += 8;
                        } while (tile_index < tile_count);
                        image_index = 0;
                    }
                    else
                    {
                        goto load_images;
                    }
                }
                else
                {
                    rect.y = actor_index + 0x1F6;
                    rect.w = 0x100;
                    rect.x = 0;
                    rect.h = 1;
                    MoveImage(&rect, 0, actor_index + 0x1F7);
                    previous_resource = ((actor_index + 2) * 0x14) + g_field_resource_entries;
                    resource_base = g_field_resource_entries;
                    inherited_resource = resource_offset + resource_base;
                    U16_AT(inherited_resource, 0xA) = (u16)U16_AT(previous_resource, 0xA);
                    U8_AT(inherited_resource, 0x8) = 0;
                    U8_AT(inherited_resource, 0x9) = (u8)U8_AT(previous_resource, 0x9);
                    S32_AT(inherited_resource, 0x0) = (s32)g_field_resource_cursor;
                    S32_AT(inherited_resource, 0x10) =
                        (s32)((S32_AT(inherited_resource, 0x10) & ~1) | ((u8)U8_AT(description, 0x1) >> 7));
                    func_8009BDD4(geometry_base + S32_AT(geometry_offsets, 0x0),
                                  geometry_base + S32_AT(geometry_offsets, 0x4));
                    S32_AT(inherited_resource, 0x4) = (s32)g_field_resource_cursor;
                    S32_AT(inherited_resource, 0x10) = (s32)(S32_AT(inherited_resource, 0x10) | 2);
                    inherited_tile_ids = description + 0x10;
                    U16_AT(inherited_resource, 0xE) = (u16)U16_AT(description, 0xA);
                    inherited_tile_count = U16_AT(description, 0xC);
                    inherited_tile_index = 0;
                    if (image_count < inherited_tile_count)
                    {
                        inherited_tile_fields = description + 0x12;
                        inherited_tile_destination_offset = tile_offset;
                        do
                        {
                            tile_base = D_8010A038;
                            inherited_tile = inherited_tile_destination_offset + tile_base;
                            U16_AT(inherited_tile, 0x2) = (u16)((U16_AT(inherited_tile, 0x2) & 0xFBFF) |
                                                                (U16_AT(inherited_tile_fields, 0x0) & 0x400));
                            U16_AT(inherited_tile, 0x0) = (u16)U16_AT(inherited_tile_ids, 0);
                            inherited_tile_destination_offset += 8;
                            U8_AT(inherited_tile, 0x2) = (u8)U16_AT(inherited_tile_fields, 0x0);
                            inherited_tile_index += 1;
                            U16_AT(inherited_tile, 0x4) = (u16)U16_AT(inherited_tile_fields, 0x2);
                            inherited_tile_ids += 8;
                            U16_AT(inherited_tile, 0x6) = (u16)U16_AT(inherited_tile_fields, 0x4);
                            U16_AT(inherited_tile, 0x2) = (u16)((U16_AT(inherited_tile, 0x2) & 0xFCFF) |
                                                                (U16_AT(inherited_tile_fields, 0x0) & 0x300));
                            inherited_tile_fields += 8;
                        } while (inherited_tile_index < inherited_tile_count);
                    }
                    DrawSync(0);
                load_images:
                    image_index = 0;
                }
                image_cursor = description + 2;
                if (image_count != 0)
                {
                    image_palette = palette_index - 0;
                    do
                    {
                        first_image = image_index == 0;
                        image_id = *image_cursor;
                        image_cursor += 1;
                        image_count -= 1;
                        image_index += 1;
                        func_8009BCF8(image_base + S32_AT(image_base, image_id * 4), image_palette,
                                      actor_index + 3, first_image);
                        image_palette = palette_index - image_index;
                    } while (image_count != 0);
                }
                geometry_offsets += 4;
                resource_offset += 0x14;
                actor_index += 1;
                tile_offset += 0x190;
                scene_cursor += 4;
            } while (actor_index < (actor_end - 3));
        }
        func_8009C4B4();
        func_80084630();
        func_8009BE1C(layout_base);
        if (D_80115898 != -1)
        {
            func_800B0094(0);
            if (D_80115894 == 0)
            {
                field_stop_song();
            }
            if (D_80115898 != -2)
            {
                func_800A368C(D_80115898, 0);
                if ((D_80115894 == 0) && (D_80115898 != -2))
                {
                    func_800A380C();
                }
            }
            else
            {
                func_800A3728(D_80115898);
            }
            g_layout_flag = D_80115898;
        }
        else
        {
            akao_cmd_c1(0, 0x3C, D_8011588C);
        }
        if (D_8011589C != -1)
        {
            func_800A368C(D_8011589C, 1);
            g_layout_sub_mode = D_8011589C;
        }
        func_8009C4B4();
        if (D_80115894 != 0)
        {
            g_field_audio_timer = 0xF;
        }
        else
        {
            g_field_audio_timer = 0;
        }
        D_80115894 = 0;
        if (D_801158A0 != 0)
        {
            func_800A3654();
        }
        func_8009C4B4();
        DrawSync(0);
        VSync(0);
        func_8009C4B4();
        field_load_map(U16_AT(persistent_map, 0));
        func_8009C4B4();
        field_init_ctx(D_800F22B0, (u16)context_id);
        func_8009C4B4();
        field_text_reset_windows();
        func_80092394();
        if (D_801158A0 != 0)
        {
            func_800B34D0(0);
        }
        spawn_record = func_800630BC((u16)spawn_id);
        if (spawn_record == NULL)
        {
            spawn_record = (u8 *)&spawn;
            spawn.h.z = 0xE0;
            spawn.h.x = 0xA0;
            spawn.h.y = 0;
            spawn.bits.direction = 0;
        }
        spacing_base = D_800EB254;
        direction_base = D_800EB0A4;
        actor_position = D_800FDF58;
        actor_index = 0;
        do
        {
            actor_flags = actor_position + 0x24;
            if (U8_AT(actor_flags, 0x1) != 0xFF)
            {
                S32_AT(actor_position, 0) = S16_AT(spawn_record, 0x0) << 8;
                S32_AT(actor_flags, -0x20) = (s32)(S16_AT(spawn_record, 0x2) << 8);
                S32_AT(actor_flags, -0x1C) = (s32)(S16_AT(spawn_record, 0x4) << 8);
                offset.vy = 0;
                offset.vx = S16_AT(spacing_base, (U8_AT(spawn_record, 0x6) & 0xF) * 4) * actor_index;
                offset.vz = S16_AT(spacing_base, (U8_AT(spawn_record, 0x6) & 0xF) * 4 + 2) * actor_index;
                func_8009C2E0(actor_position, &offset.vx);
                U8_AT(actor_flags, -0x9) = (s8)((U8_AT(spawn_record, 0x6) & 0xF) << 5);
                direction_entry = ((U8_AT(spawn_record, 0x6) & 0xF) * 4) + direction_base;
                U8_AT(actor_flags, 0x3) = 0;
                U8_AT(actor_flags, 0x0) = 1;
                direction_flag = U8_AT(direction_entry, 0) & 0x80;
                actor_mode = ((S32_AT(direction_entry, 0) & ~0x80) % 5) | direction_flag;
                U8_AT(actor_flags, -0x3) = actor_mode;
                func_8006C3FC(actor_position);
            }
            actor_index += 1;
            actor_position += 0x54;
        } while (actor_index < 3);
        func_8009C12C();
        func_8008C730();
        func_80091BC8();
        D_8010D010 = D_8010AE4C;
        D_8010D014 = D_8010AE50;
        func_800A3BE8(layout_option);
        g_layout_option = layout_option;
        func_8009C4B4();
        D_8010D028 = 0;
        field_initialize_actor_slots();
        field_clear_actor_slots();
        func_80067AA4();
        func_80084240();
        func_800A255C();
        func_80086F20();
        saved_scene_id = g_scene_mode;
        if (D_801158A0 != 0)
        {
            saved_scene_id += 0x8000;
        }
        func_800A8D10(saved_scene_id, context_id, g_layout_flag, spawn_id, g_layout_option,
                      g_layout_sub_mode);
        func_800A54D0();
        func_8008C7A8();
        func_800A2DFC();
        func_800AF8C4();
        func_8009C4B4();
        func_800A6EEC();
    }
}
