#include "wmap_main.h"
#include "wmap_party_travel.h"
#include "wmap_pathfinding.h"
#include "wmap_map_events.h"
#include "wmap_resource_support.h"
#include "wmap_sequence_runtime.h"
#include "wmap_sprite_render.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "cdrom.h"
#include "wmap_effect_backdrop.h"
#include "wmap_effect_resources.h"
#include "wmap_effect_primitives.h"
#include "wmap_map_labels.h"
#include "akao_cmd.h"

typedef struct
{
    s32 tile;
    u8 pad_04[36];
} WmapTile;
extern WmapTile D_80139290[6][6];

static inline s32 tile_exists(s32 x, s32 y)
{
    if (x < 0 || y < 0 || x >= 6 || y >= 6)
    {
        return 0;
    }
    return D_80139290[x][y].tile != 255;
}

/** @brief Dispatch the first pending map event and consume one event tick. */
void func_800A5DFC(void)
{
    extern s32 D_800D9224;
    extern u8 D_800DCEF4[4];
    extern u8 g_wmap_vehicle_cell_x;
    extern u8 g_wmap_vehicle_cell_y;
    extern s32 D_8011CF20;
    extern s32 D_8011CF44;
    extern s32 D_8011D4F8;
    extern u8 D_8011D538;
    extern u8 D_80123538;
    extern u8 D_80129538;
    extern s32 D_80129540;
    extern s32 D_8012954C;
    extern s32 D_80139238;
    extern s32 D_80139248;
    extern s32 D_80139834;
    extern s32 D_80139900;
    extern s32 D_8013997C;
    extern s32 D_8013B288;
    extern s32 D_8018222C;
    extern s32 D_80182DD4;
    extern s32 D_801ADAF0;
    s32 wmap_run_special_travel(s32 initialize);
    s32 wmap_run_special_return(s32 initialize);
    s32 func_800AB850(s32 initialize);
    s32 func_800B2080(s32 initialize);
    s32 func_800B45B8(s32 initialize);
    s32 func_800C2274(s32 initialize);

    s32 event_index;
    u8 *event;

    if (D_8011CF44 == 0)
    {
        g_wmap_input_locked = 1;
        g_wmap_buttons_held = 0;
        g_wmap_buttons_repeat = 0;
        if (D_80182DD4 != 0)
        {
            D_80182DD4 = 0;
            func_8006CAC0(&func_800C2274);
        }
        else if (D_8013B288 != 0)
        {
            D_8013B288 = 0;
            func_8006CAC0(&wmap_run_special_travel);
        }
        else if (D_8013997C != 0)
        {
            cdrom_queue_read(0x1145, &D_8011D538);
            cdrom_queue_read(0x1146, (u8 *)&D_8011D538 + 0x2000);
            cdrom_wait_queue_empty();
            D_8013997C = 0;
            func_8006CAC0(&func_800A7370);
        }
        else if (D_8011CF20 != 0)
        {
            D_8011CF20 = 0;
            func_8006CAC0(&wmap_run_special_return);
        }
        else if (D_801ADAF0 != 0)
        {
            D_8012954C = 0;
            func_8006CAC0(&func_800B45B8);
        }
        else if (D_8012954C != 0)
        {
            D_8012954C = 0;
            func_8006CAC0(&func_800B2080);
        }
        else if (D_8011D4F8 != 0)
        {
            D_8011D4F8 = 0;
            func_8006CAC0(&func_800AB850);
        }
        else if (D_80139248 != 0)
        {
            D_80139248 = 0;
            func_8006CAC0(&func_800A7BE8);
        }
        else if (D_80129540 != 0)
        {
            D_80129540 = 0;
            func_8006CAC0(&func_800A7FD0);
        }
        else if (D_80139900 != 0)
        {
            D_80139900 = 0;
            func_8006CAC0(&func_800A83B8);
        }
        else if (D_80139238 != 0)
        {
            D_80139238 = 0;
            func_8006CAC0(&func_800A87D4);
        }
        else if (D_80139834 != 0)
        {
            D_80139834 = 0;
            func_8006CAC0(&func_800A88D8);
        }
        else
        {
            for (event_index = 0; event_index < 8; event_index++)
            {
                event = (u8 *)&D_80129538 + event_index;
                if (*event != 0)
                {
                    *event = 0;
                    switch (event_index)
                    {
                    case 0:
                        cdrom_queue_read(0x1149, &D_80123538);
                        cdrom_wait_queue_empty();
                        func_80064F64(0x114A);
                        wmap_find_land_cell(4, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
                        D_8018222C = 1;
                        D_800DCEF4[3] = 1;
                        D_800DCEF4[2] = 1;
                        D_800DCEF4[1] = 1;
                        D_800DCEF4[0] = 1;
                        break;
                    case 1:
                        cdrom_queue_read(0x10DE, &D_80123538);
                        cdrom_wait_queue_empty();
                        func_80064F64(0x10DF);
                        func_800A61BC(11);
                        break;
                    case 2:
                        cdrom_queue_read(0x10D8, &D_80123538);
                        cdrom_wait_queue_empty();
                        func_80064F64(0x10D9);
                        func_800A61BC(17);
                        break;
                    case 3:
                        cdrom_queue_read(0x10DC, &D_80123538);
                        cdrom_wait_queue_empty();
                        func_80064F64(0x10DD);
                        func_800A61BC(11);
                        break;
                    case 4:
                        cdrom_queue_read(0x10DA, &D_80123538);
                        cdrom_wait_queue_empty();
                        func_80064F64(0x10DB);
                        func_800A61BC(10);
                        break;
                    }
                    func_8006CAC0(func_800A7668);
                    break;
                }
            }
        }
        D_800D9224 -= 1;
    }
}

/** @brief Select the first occupied neighbor, or a random direction when isolated. */
void func_800A61BC(s32 tile)
{
extern u8 D_800DCEF4[4];
extern s8 D_800DCEF5;
extern s8 D_800DCEF6;
extern s8 D_800DCEF7;
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 D_8018222C;
extern s32 rand(void);

    wmap_find_land_cell(tile, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    D_8018222C = 0;
    D_800DCEF4[3] = 0;
    D_800DCEF4[2] = 0;
    D_800DCEF4[1] = 0;
    D_800DCEF4[0] = 0;
    if (tile_exists(g_wmap_vehicle_cell_x + 1, g_wmap_vehicle_cell_y))
    {
        D_800DCEF4[0] = 1;
        return;
    }
    if (tile_exists(g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y + 1))
    {
        D_800DCEF5 = 1;
        return;
    }
    if (tile_exists(g_wmap_vehicle_cell_x - 1, g_wmap_vehicle_cell_y))
    {
        D_800DCEF6 = 1;
        return;
    }
    if (tile_exists(g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y - 1))
    {
        D_800DCEF7 = 1;
        return;
    }
    D_800DCEF4[rand() & 3] = 1;
}

/** @brief Save the projection state and set the next effect's map-relative position. */
void func_800A643C(void)
{
/** @brief Four-word world-map transform state. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    s32 pad;
} WmapTransform;

extern WmapTransform g_wmap_saved_view;
extern WmapTransform g_wmap_view;
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 D_80139234;
extern s32 g_wmap_view_scroll_mode;
extern s32 D_8013B208;
extern s32 D_8013B288;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_801B2E40;
extern void func_800A76F8__for_func_800A643C(void) __asm__("func_800A76F8");

    D_8013B288 = 0;
    D_80139234 = 0;
    g_wmap_input_locked = 1;
    D_8013B208 = 1;
    g_wmap_saved_view = g_wmap_view;
    wmap_find_land_cell(4, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * 48) - g_wmap_view.x;
    g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * 48) - g_wmap_view.y;
    D_801B2E40++;
    func_800A76F8__for_func_800A643C();
}

/** @brief Initialize active directional actors and play the transition sound. */
void func_800A6540(void)
{
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

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    s32 field_10;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

extern WmapConfigA D_800D95D8[];
extern WmapMotion D_801AFBD0[];
extern WmapMotion D_801AFD60[];
extern WmapResource D_80139988[];
extern u8 D_800DCEF4[4];
extern u8 D_80123538[];
extern s32 D_801B2E40;
extern s32 D_801B2E44;
extern void akao_cmd_c2(s32, s32, s32, s32);
extern void func_800A7738__for_func_800A6540(void) __asm__("func_800A7738");

    s32 i;
    WmapConfigA *actor;

    for (i = 40; i < 120; i++)
    {
        D_801AFBD0[i].state = 0;
        D_80139988[i].resource = D_80123538;
    }
    for (i = 0; i < 4; i++)
    {
        actor = &D_800D95D8[i];
        if (D_800DCEF4[i] != 0)
        {
            D_80139988[i + 20].resource = D_80123538;
            actor->field_06 = 15;
            actor->field_0E = i + 1;
            actor->field_10 = -1;
            actor->field_22 = 128;
            actor->field_26 = 2;
            actor->field_02 = 0;
            actor->field_24 = 0;
            D_801AFD60[i].state = 1;
            D_801AFD60[i].angle = i << 10;
            D_801AFD60[i].z = 90000;
            D_801AFD60[i].x = 900;
            D_801AFD60[i].field_0E = 0;
        }
    }
    akao_cmd_c2(0, 30, 127, 48);
    if (D_800DCEF4[3] & (D_800DCEF4[2] & (D_800DCEF4[0] & D_800DCEF4[1])))
    {
        wmap_play_sound(49, 128);
    }
    else
    {
        wmap_play_sound(21, 128);
    }
    D_801B2E44 = 64;
    D_801B2E40++;
    func_800A7738__for_func_800A6540();
}

/** @brief Project four rotating effect actors and update their draw depths. */
void func_800A66C0(void)
{
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

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    s32 field_10;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

extern WmapConfigA D_800D95D8[];
extern WmapResource D_80139A28[];
extern WmapMotion D_801AFD60[];

    SVECTOR position;
    s32 depth;
    s32 draw_depth;
    s32 i;
    WmapConfigA *actor;
    WmapMotion *motion;

    for (i = 0; i < 4; i++)
    {
        motion = &D_801AFD60[i];
        actor = &D_800D95D8[i];
        position.vx = ((motion->z >> 3) * (ccos(motion->angle) >> 6)) >> 12;
        position.vy = ((motion->z >> 3) * (csin(motion->angle) >> 6)) >> 12;
        position.vz = motion->field_0E;
        gte_ldv0(&position);
        gte_rtps();
        wmap_step_actor_animation(actor, &D_80139A28[i]);
        gte_stsxy(&motion->field_10);
        gte_stszotz(&depth);
        draw_depth = (7057 - depth) / 4 + 42;
        wmap_draw_actor_sprite(actor, motion->field_10, 3, draw_depth, 0x400);
        motion->scale = draw_depth;
    }
}

/** @brief Draw active trail actors and periodically copy four new trail samples. */
void func_800A6800(void)
{
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
} __attribute__((aligned(4))) WmapConfigA;

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    s32 field_10;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

extern WmapConfigA D_800D9268[];
extern WmapMotion D_801AFBD0[];
extern WmapResource D_80139988[];
extern s32 D_8018222C;
extern s32 D_8011CF74;
extern s32 D_80139234;

    s32 i;
    s32 destination;
    WmapConfigA *actor;
    WmapMotion *motion;

    if (D_8018222C != 0)
    {
        for (i = 40; i < 104; i++)
        {
            motion = &D_801AFBD0[i];
            if (motion->state != 0)
            {
                actor = &D_800D9268[i];
                wmap_step_actor_animation(actor, &D_80139988[i]);
                wmap_draw_actor_sprite(actor, motion->field_10, 3, motion->scale + 1, 0x400);
            }
        }
        if (D_8011CF74 % 10 == 0)
        {
            if (D_80139234 != -1)
            {
                for (i = 0; i < 4; i++)
                {
                    destination = i + D_80139234 * 4 + 40;
                    D_801AFBD0[destination] = D_801AFBD0[i + 20];
                    D_800D9268[destination] = D_800D9268[i + 20];
                    D_800D9268[destination].field_22 = 0;
                    D_800D9268[destination].field_26 = 2;
                }
                D_80139234 = (D_80139234 + 1) & 15;
            }
        }
    }
}

/** @brief Load resources and set the effect's map-relative position. */
    void func_800A6A20(void)
    {
/** @brief Four-word world-map transform state. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    s32 pad;
    } WmapTransform;

    extern WmapTransform g_wmap_saved_view;
    extern WmapTransform g_wmap_view;
    extern s32 g_wmap_vehicle_cell_x;
    extern s32 g_wmap_vehicle_cell_y;
    extern s32 g_wmap_view_scroll_mode;
    extern s32 D_8013B208;
    extern s32 g_wmap_scroll_remaining_x;
    extern s32 g_wmap_scroll_remaining_y;
    extern s32 D_801B2E48;
    extern void func_800A7C78__for_func_800A6A20(void) __asm__("func_800A7C78");
    extern u8 D_800DCA98[];

        g_wmap_saved_view = g_wmap_view;
        D_8013B208 = 1;
        wmap_find_land_cell(12, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
        cdrom_queue_read(0x10E2, D_800DCA98);
        func_80064F64(0x10E3);
        wmap_set_traveler_position(3, g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y);
        g_wmap_view_scroll_mode = 2;
        g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * 48) - g_wmap_view.x;
        g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * 48) - g_wmap_view.y;
        D_801B2E48++;
        func_800A7C78__for_func_800A6A20();
    }

/**
 * @brief World-map step handler: seed a pathfinding move for the actor, populate its
 *        motion record, and advance the step counter.
 */
void func_800A6B34(void)
{
extern void func_800A7D40__for_func_800A6B34(void) __asm__("func_800A7D40");
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 D_801B2E48;

    u8* base;
    u16 a;
    u16 b;

    wmap_find_land_cell(0x10, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    base = (u8*)g_wmap_travelers;
    func_8005EB68(*(s32*)(base + 0x36C), *(s32*)(base + 0x370), g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y,
                  (s32*)(base + 0x390), (s32*)(base + 0x410));
    *(s32*)(base + 0x384) = g_wmap_vehicle_cell_x;
    *(s32*)(base + 0x388) = g_wmap_vehicle_cell_y;
    a = *(u16*)(base + 0x394);
    g_wmap_travelers[3].moving = 1;
    g_wmap_travelers[3].path_index = 1;
    g_wmap_scripted_travel_active = 1;
    *(u16*)(base + 0x374) = a;
    *(u16*)(base + 0x37C) = ((s16)a - 1) * 0xA0;
    b = *(u16*)(base + 0x414);
    *(u16*)(base + 0x376) = b;
    *(u16*)(base + 0x37E) = ((s16)b - 1) * 0xA0;
    D_801B2E48 += 1;
    func_800A7D40__for_func_800A6B34();
}

/**
 * @brief World-map step handler: kick off the streamed cell load and seed the scroll
 *        target from the current cell, then advance the step.
 */
void func_800A6C24(void)
{
typedef struct { s32 w[4]; } WmapBlk16;

extern void cdrom_queue_read__for_func_800A6C24(s32 sector, void* dst) __asm__("cdrom_queue_read");
extern void func_800A8060__for_func_800A6C24(void) __asm__("func_800A8060");
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 g_wmap_saved_view[];
extern s32 g_wmap_view[];
extern u8 D_800DCA98[];
extern s32 D_8013B208;
extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_801B2E50;

    *(WmapBlk16*)g_wmap_saved_view = *(WmapBlk16*)g_wmap_view;
    D_8013B208 = 1;
    wmap_find_land_cell(1, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    cdrom_queue_read__for_func_800A6C24(0x10E0, D_800DCA98);
    func_80064F64(0x10E1);
    wmap_set_traveler_position(3, g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = (g_wmap_vehicle_cell_x - 1) * 0x30 - g_wmap_view[0];
    g_wmap_scroll_remaining_y = (g_wmap_vehicle_cell_y - 1) * 0x30 - g_wmap_view[1];
    D_801B2E50 += 1;
    func_800A8060__for_func_800A6C24();
}

/**
 * @brief World-map step handler: seed a pathfinding move for the actor, populate its
 *        motion record, and advance the step counter.
 */
void func_800A6D38(void)
{
extern void func_800A8128__for_func_800A6D38(void) __asm__("func_800A8128");
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 D_801B2E50;

    u8* base;
    u16 a;
    u16 b;

    wmap_find_land_cell(0x2, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    base = (u8*)g_wmap_travelers;
    func_8005EB68(*(s32*)(base + 0x36C), *(s32*)(base + 0x370), g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y,
                  (s32*)(base + 0x390), (s32*)(base + 0x410));
    *(s32*)(base + 0x384) = g_wmap_vehicle_cell_x;
    *(s32*)(base + 0x388) = g_wmap_vehicle_cell_y;
    a = *(u16*)(base + 0x394);
    g_wmap_travelers[3].moving = 1;
    g_wmap_travelers[3].path_index = 1;
    g_wmap_scripted_travel_active = 1;
    *(u16*)(base + 0x374) = a;
    *(u16*)(base + 0x37C) = ((s16)a - 1) * 0xA0;
    b = *(u16*)(base + 0x414);
    *(u16*)(base + 0x376) = b;
    *(u16*)(base + 0x37E) = ((s16)b - 1) * 0xA0;
    D_801B2E50 += 1;
    func_800A8128__for_func_800A6D38();
}

/**
 * @brief World-map step handler: kick off the streamed cell load and seed the scroll
 *        target from the current cell, then advance the step.
 */
void func_800A6E28(void)
{
typedef struct { s32 w[4]; } WmapBlk16;

extern void cdrom_queue_read__for_func_800A6E28(s32 sector, void* dst) __asm__("cdrom_queue_read");
extern void func_800A8448__for_func_800A6E28(void) __asm__("func_800A8448");
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 g_wmap_saved_view[];
extern s32 g_wmap_view[];
extern u8 D_800DCA98[];
extern s32 D_8013B208;
extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_801B2E58;

    *(WmapBlk16*)g_wmap_saved_view = *(WmapBlk16*)g_wmap_view;
    D_8013B208 = 1;
    wmap_find_land_cell(1, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    cdrom_queue_read__for_func_800A6E28(0x1216, D_800DCA98);
    func_80064F64(0x1217);
    wmap_set_traveler_position(3, g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = (g_wmap_vehicle_cell_x - 1) * 0x30 - g_wmap_view[0];
    g_wmap_scroll_remaining_y = (g_wmap_vehicle_cell_y - 1) * 0x30 - g_wmap_view[1];
    D_801B2E58 += 1;
    func_800A8448__for_func_800A6E28();
}

/**
 * @brief Build the effect route, initialize its position, and advance the sequence.
 */
void func_800A6F3C(void)
{
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 D_801B2E58;
extern void func_800A8510__for_func_800A6F3C(void) __asm__("func_800A8510");

    u8* base;
    u16 cell_x;
    u16 cell_y;
    s32 state;
    s32 coordinate;

    wmap_find_land_cell(0x12, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    base = (u8*)g_wmap_travelers;
    func_8005EB68(*(s32*)(base + 0x36C), *(s32*)(base + 0x370), g_wmap_vehicle_cell_x, g_wmap_vehicle_cell_y,
                  (s32*)(base + 0x390), (s32*)(base + 0x410));
    *(s32*)(base + 0x384) = g_wmap_vehicle_cell_x;
    *(s32*)(base + 0x388) = g_wmap_vehicle_cell_y;
    state = 1;
    cell_x = *(u16*)(base + 0x394);
    g_wmap_scripted_travel_active = state;
    g_wmap_travelers[3].moving = state;
    g_wmap_travelers[3].path_index = state;
    *(u16*)(base + 0x374) = cell_x;
    coordinate = (s16)cell_x - 1;
    *(u16*)(base + 0x37C) = coordinate * 0xA0;
    cell_y = *(u16*)(base + 0x414);
    coordinate = cell_y;
    *(u16*)(base + 0x376) = cell_y;
    *(u16*)(base + 0x37E) = ((s16)coordinate - 1) * 0xA0;
    wmap_play_sound(50, 128);
    D_801B2E58 += 1;
    func_800A8510__for_func_800A6F3C();
}

/** @brief World-map step: seed the scroll target from the current cell, then advance. */
void func_800A703C(void)
{
extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_view[];
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_801B2E60;
extern void func_800A8864__for_func_800A703C(void) __asm__("func_800A8864");

    func_8006D8F0(1);
    func_8006D870(1);
    wmap_find_land_cell(0x10, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    D_8011D510 = g_wmap_vehicle_cell_x;
    D_8011D530 = g_wmap_vehicle_cell_y;
    g_wmap_scroll_remaining_x = (g_wmap_vehicle_cell_x - 1) * 0x30 - g_wmap_view[0];
    g_wmap_scroll_remaining_y = (g_wmap_vehicle_cell_y - 1) * 0x30 - g_wmap_view[1];
    D_801B2E60 += 1;
    func_800A8864__for_func_800A703C();
}

/** @brief World-map step: seed the scroll target from the current cell, then advance. */
void func_800A7108(void)
{
extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_view[];
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_801B2E68;
extern void func_800A8968__for_func_800A7108(void) __asm__("func_800A8968");

    func_8006D8F0(1);
    func_8006D870(1);
    wmap_find_land_cell(0x17, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    D_8011D510 = g_wmap_vehicle_cell_x;
    D_8011D530 = g_wmap_vehicle_cell_y;
    g_wmap_scroll_remaining_x = (g_wmap_vehicle_cell_x - 1) * 0x30 - g_wmap_view[0];
    g_wmap_scroll_remaining_y = (g_wmap_vehicle_cell_y - 1) * 0x30 - g_wmap_view[1];
    D_801B2E68 += 1;
    func_800A8968__for_func_800A7108();
}

/** @brief Set the map-relative effect position, load resources, and advance the sequence. */
void func_800A71D4(void)
{
extern void func_800A7400__for_func_800A71D4(void) __asm__("func_800A7400");
extern s32 D_800DBE70;
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern s32 D_80139224;
extern s32 g_wmap_view_scroll_mode;
/** @brief Four-word world-map transform state. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    s32 pad;
} WmapTransform;

extern WmapTransform g_wmap_view;
extern s32 D_80139978;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_801B2E70;

    D_80139224 = 0;
    D_80139978 = 0x18;
    D_800DBE70 = 0;
    wmap_find_land_cell(0x18, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    D_8011D510 = g_wmap_vehicle_cell_x;
    D_8011D530 = g_wmap_vehicle_cell_y;
    g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * 0x30) - g_wmap_view.x;
    g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * 0x30) - g_wmap_view.y;
    func_800A89DC(0x21);
    D_801B2E70 += 1;
    func_800A7400__for_func_800A71D4();
}

/**
 * @brief World-map actor tick: advance timers, bump a wave index, and expire the step.
 */
void func_800A72B8(void)
{
extern s8 *wmap_turn_vehicle(s32 arg0);
extern u8 D_801AFBD0[];
extern s32 D_8011CF74;
extern s32 D_801B2E70;
extern s32 D_801B2E74;

    s8 *obj;
    u8 *base;

    obj = wmap_turn_vehicle(1);
    base = D_801AFBD0;
    if (*(s16 *)(base + 0xE) < 100)
    {
        *(s16 *)(base + 0xE) += 1;
    }
    if (*(s32 *)(base + 0x8) < 0x3E8)
    {
        *(s32 *)(base + 0x8) += 0x1E;
    }
    if ((D_8011CF74 & 3) == 0)
    {
        if (obj[6] < 0xF)
        {
            obj[6] += 1;
        }
    }
    if (--D_801B2E74 == 0)
    {
        D_801B2E70 += 1;
    }
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A7370(s32 arg0)
{
extern u32 D_801B2E70;
extern s32 D_801B2E74;
extern void (*D_800D6D34[])(void);
extern s32 g_wmap_view_scroll_mode;
extern void func_800A7440__for_func_800A7370(void) __asm__("func_800A7440");

    s32 result;

    if (arg0 != 0)
    {
        D_801B2E70 = 1;
        D_801B2E74 = 1;
        return 1;
    }

    if (D_801B2E70 < 0xA)
    {
        D_800D6D34[D_801B2E70]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A73E8(void)
{
extern u32 D_801B2E70;
extern s32 D_801B2E74;
extern void (*D_800D6D34[])(void);
extern s32 g_wmap_view_scroll_mode;
extern void func_800A7440__for_func_800A73E8(void) __asm__("func_800A7440");

    D_801B2E70 = 1;
    D_801B2E74 = 1;
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800A7400(void)
{
extern u32 D_801B2E70;
extern s32 D_801B2E74;
extern void (*D_800D6D34[])(void);
extern s32 g_wmap_view_scroll_mode;
extern void func_800A7440__for_func_800A7400(void) __asm__("func_800A7440");

    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B2E70 += 1;
        func_800A7440__for_func_800A7400();
    }
}

/** @brief Initialize the actor, register its callback, and start the sequence delay. */
void func_800A7440(void)
{
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
extern void func_800A72B8__for_func_800A7440(void) __asm__("func_800A72B8");
extern WmapConfigA g_wmap_vehicle_actor;
extern u8 D_8011D538[];
extern u8 *D_8013A184;
extern WmapAuxState D_801AFBD0;
extern s32 D_801B2E70;
extern s32 D_801B2E74;
extern void wmap_draw_vehicle(void);

    D_8013A184 = D_8011D538;
    g_wmap_vehicle_actor.field_10 = -1;
    g_wmap_vehicle_actor.field_02 = 0;
    g_wmap_vehicle_actor.field_06 = 0;
    g_wmap_vehicle_actor.field_0E = 0;
    g_wmap_vehicle_actor.field_26 = 0;
    g_wmap_vehicle_actor.field_22 = 0x80;
    g_wmap_vehicle_actor.field_24 = 0x80;
    D_801AFBD0.field_00 = 1;
    D_801AFBD0.field_08 = 0xC8;
    D_801AFBD0.field_0E = 0xA;
    D_801AFBD0.field_10 = 0x3C;
    D_801AFBD0.field_02 = 0;
    D_801AFBD0.field_04 = 2;
    wmap_install_callback(&wmap_draw_vehicle);
    func_800591A8(0x21);
    D_801B2E74 = 0x16E;
    D_801B2E70 += 1;
    func_800A72B8__for_func_800A7440();
}

/** @brief World-map step handler: install a callback, advance the step counter, chain to the next step. */
void func_800A74FC(void)
{
extern void wmap_finish_vehicle_turn(void);
extern void func_800A7544__for_func_800A74FC(void) __asm__("func_800A7544");
extern s32 g_wmap_vehicle_phase;
extern s32 D_801B2E70;

    g_wmap_vehicle_phase = 1;
    func_8006CAC0(wmap_finish_vehicle_turn);
    D_801B2E70 += 1;
    func_800A7544__for_func_800A74FC();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800A7544(void)
{
extern s32 g_wmap_vehicle_phase;
extern s32 D_801B2E70;
extern void func_800A7580__for_func_800A7544(void) __asm__("func_800A7580");

    if (g_wmap_vehicle_phase == 0)
    {
        D_801B2E70 += 1;
        func_800A7580__for_func_800A7544();
    }
}

/**
 * @brief Reset a world-map animation flag and schedule the next step.
 */
void func_800A7580(void)
{
extern u16 D_801AFBD0;
extern s32 D_801B2E70;
extern s32 D_801B2E74;
extern void func_800A75C0__for_func_800A7580(void) __asm__("func_800A75C0");

    D_801AFBD0 = 0;
    D_801B2E74 = 0x78;
    D_801B2E70 += 1;
    func_800A75C0__for_func_800A7580();
}

/** @brief Draw the sprite, move its packed coordinate, and update the countdown. */
void func_800A75C0(void)
{
extern void wmap_turn_vehicle(s32);
extern u8 g_wmap_vehicle_actor[];
extern s32 g_wmap_vehicle_screen_position;
extern u8 g_wmap_vehicle_animation[];
extern s32 D_801B2E70;
extern s32 D_801B2E74;

    s32 remaining_ticks;
    u8 *sprite = g_wmap_vehicle_actor;

    wmap_turn_vehicle(0);
    wmap_step_actor_animation(sprite, &g_wmap_vehicle_animation);
    wmap_draw_actor_sprite(sprite, g_wmap_vehicle_screen_position, 0x28, 2, 2);
    remaining_ticks = D_801B2E74 - 1;
    *(s16 *)&g_wmap_vehicle_screen_position -= 4;
    D_801B2E74 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2E70 += 1;
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_800A7650(void)
{
extern s32 D_801B2E70;

    D_801B2E70 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A7668(s32 arg0)
{
extern u32 D_801B2E40;
extern s32 D_801B2E44;
extern void (*D_800D6C14[])(void);
extern s32 g_wmap_view_scroll_mode;
extern void func_800A6540__for_func_800A7668(void) __asm__("func_800A6540");

    s32 result;

    if (arg0 != 0)
    {
        D_801B2E40 = 1;
        D_801B2E44 = 1;
        return 1;
    }

    if (D_801B2E40 < 0x10)
    {
        D_800D6C14[D_801B2E40]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A76E0(void)
{
extern u32 D_801B2E40;
extern s32 D_801B2E44;
extern void (*D_800D6C14[])(void);
extern s32 g_wmap_view_scroll_mode;
extern void func_800A6540__for_func_800A76E0(void) __asm__("func_800A6540");

    D_801B2E40 = 1;
    D_801B2E44 = 1;
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800A76F8(void)
{
extern u32 D_801B2E40;
extern s32 D_801B2E44;
extern void (*D_800D6C14[])(void);
extern s32 g_wmap_view_scroll_mode;
extern void func_800A6540__for_func_800A76F8(void) __asm__("func_800A6540");

    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B2E40 += 1;
        func_800A6540__for_func_800A76F8();
    }
}

/** @brief World-map step: run the two sub-steps, then advance after the timer. */
void func_800A7738(void)
{
extern s32 D_801B2E40;
extern s32 D_801B2E44;
extern void func_800A66C0__for_func_800A7738(void) __asm__("func_800A66C0");

    func_8006AEE0();
    func_800A66C0__for_func_800A7738();
    if (--D_801B2E44 == 0)
    {
        D_801B2E40 += 1;
    }
}

/**
 * @brief Clear the sub-flag, set the sequence parameter, advance the counter, and run the handler.
 */
void func_800A778C(void)
{
extern void func_800A77CC__for_func_800A778C(void) __asm__("func_800A77CC");
extern s32 D_800DCEC0;
extern s32 D_801B2E44;
extern s32 D_801B2E40;

    D_800DCEC0 = 0;
    D_801B2E44 = 0x1E;
    D_801B2E40 += 1;
    func_800A77CC__for_func_800A778C();
}

/** @brief Step the world-map particle set, decaying each slot's velocity field. */
void func_800A77CC(void)
{
extern u8 D_801AFBD0[];
extern s32 D_801B2E40;
extern s32 D_801B2E44;
extern void func_800A66C0__for_func_800A77CC(void) __asm__("func_800A66C0");
extern void func_800A6800__for_func_800A77CC(void) __asm__("func_800A6800");

    s32 i;
    u8* p;

    func_8006AEE0();
    func_800A66C0__for_func_800A77CC();
    func_800A6800__for_func_800A77CC();
    for (i = 0; i < 4; i++)
    {
        p = D_801AFBD0 + (0x14 + i) * 0x14;
        *(s32*)(p + 0x8) -= *(s32*)(p + 0x4);
    }
    if (--D_801B2E44 == 0)
    {
        D_801B2E40 += 1;
    }
}

/** @brief Set up the effect, select its delay, and run the next sequence step. */
void func_800A785C(void)
{
extern void func_800A78B0__for_func_800A785C(void) __asm__("func_800A78B0");
extern s32 D_8018222C;
extern s32 D_801B2E40;
extern s32 D_801B2E44;

    s32 delay;

    func_8005FF88(-1);
    delay = 0x46;
    if (D_8018222C != 0)
    {
        delay = 0x32;
    }
    D_801B2E44 = delay;
    D_801B2E40 += 1;
    func_800A78B0__for_func_800A785C();
}

/** @brief Step the world-map particle set, decaying each slot's velocity field. */
void func_800A78B0(void)
{
extern u8 D_801AFBD0[];
extern s32 D_801B2E40;
extern s32 D_801B2E44;
extern void func_800A66C0__for_func_800A78B0(void) __asm__("func_800A66C0");
extern void func_800A6800__for_func_800A78B0(void) __asm__("func_800A6800");

    s32 i;
    u8* p;

    func_8006AEE0();
    func_800A66C0__for_func_800A78B0();
    func_800A6800__for_func_800A78B0();
    for (i = 0; i < 4; i++)
    {
        p = D_801AFBD0 + (0x14 + i) * 0x14;
        *(s32*)(p + 0x8) -= *(s32*)(p + 0x4);
    }
    if (--D_801B2E44 == 0)
    {
        D_801B2E40 += 1;
    }
}

/** @brief Clear four actor states and start the next timed sequence step. */
void func_800A7940(void)
{
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

extern u8 D_800DCEF4[4];
extern WmapConfigA D_800D9268[];
extern s32 D_80139234;
extern s32 D_801B2E40;
extern s32 D_801B2E44;
extern void func_800A79E4__for_func_800A7940(void) __asm__("func_800A79E4");

    s32 i;

    if (!(D_800DCEF4[3] & (D_800DCEF4[2] & (D_800DCEF4[0] & D_800DCEF4[1]))))
    {
        akao_cmd_f1();
    }
    for (i = 0; i < 4; i++)
    {
        D_800D9268[i + 20].field_0E = 0;
    }
    D_80139234 = -1;
    D_801B2E44 = 0x28;
    D_801B2E40++;
    func_800A79E4__for_func_800A7940();
}

/** @brief Run three drawing updates and advance when the countdown expires. */
void func_800A79E4(void)
{
extern void func_800A66C0__for_func_800A79E4(void) __asm__("func_800A66C0");
extern void func_800A6800__for_func_800A79E4(void) __asm__("func_800A6800");
extern s32 D_801B2E40;
extern s32 D_801B2E44;

    s32 remaining_ticks;

    func_8006AEE0();
    func_800A66C0__for_func_800A79E4();
    func_800A6800__for_func_800A79E4();
    remaining_ticks = D_801B2E44 - 1;
    D_801B2E44 = remaining_ticks;
    if (remaining_ticks == 0)
    {
        D_801B2E40 += 1;
    }
}

/** @brief Clear four resource fields and start a 64-tick sequence step. */
void func_800A7A40(void)
{
/** @brief Resource record with the field cleared by this sequence step. */
typedef struct
{
    u8 unknown_0[0x22];
    s16 value;
    u8 unknown_24[8];
} WmapResourceValue;

extern WmapResourceValue D_800D9268[];
extern s32 D_801B2E40;
extern s32 D_801B2E44;
extern void func_800A7AA0__for_func_800A7A40(void) __asm__("func_800A7AA0");

    s32 index;
    for (index = 0; index < 4; index++)
    {
        D_800D9268[index + 20].value = 0;
    }
    D_801B2E44 = 64;
    D_801B2E40 += 1;
    func_800A7AA0__for_func_800A7A40();
}

/** @brief World-map step: run the two sub-steps, then advance after the timer. */
void func_800A7AA0(void)
{
extern s32 D_801B2E40;
extern s32 D_801B2E44;
extern void func_800A66C0__for_func_800A7AA0(void) __asm__("func_800A66C0");

    func_8006AEE0();
    func_800A66C0__for_func_800A7AA0();
    if (--D_801B2E44 == 0)
    {
        D_801B2E40 += 1;
    }
}

/** @brief Start audio, compute the coordinate delta, and advance the sequence. */
void func_800A7AF4(void)
{
extern void akao_cmd_c2(s32, s32, s32, s32);
extern void func_800A7B78__for_func_800A7AF4(void) __asm__("func_800A7B78");
extern s32 g_wmap_saved_view[];
extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_view[];
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_801B2E40;

    akao_cmd_c2(0, 0x1E, 0x30, 0x7F);
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = g_wmap_saved_view[0] - g_wmap_view[0];
    g_wmap_scroll_remaining_y = g_wmap_saved_view[1] - g_wmap_view[1];
    D_801B2E40 += 1;
    func_800A7B78__for_func_800A7AF4();
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800A7B78(void)
{
extern s32 g_wmap_view_scroll_mode;
extern s32 D_801B2E40;
extern void func_800A7BB8__for_func_800A7B78(void) __asm__("func_800A7BB8");

    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B2E40 += 1;
        func_800A7BB8__for_func_800A7B78();
    }
}

/** @brief Reset two world-map values, set the enable flag, and advance the state. */
void func_800A7BB8(void)
{
extern s32 D_800DCEC0;
extern s32 D_8013B208;
extern s32 D_801B2E40;

    g_wmap_input_locked = 0;
    D_8013B208 = 0;
    D_800DCEC0 = 1;
    D_801B2E40 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A7BE8(s32 arg0)
{
extern u32 D_801B2E48;
extern s32 D_801B2E4C;
extern void (*D_800D6C54[])(void);
extern s32 g_wmap_view_scroll_mode;
extern void func_800A7CB8__for_func_800A7BE8(void) __asm__("func_800A7CB8");

    s32 result;

    if (arg0 != 0)
    {
        D_801B2E48 = 1;
        D_801B2E4C = 1;
        return 1;
    }

    if (D_801B2E48 < 0x10)
    {
        D_800D6C54[D_801B2E48]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A7C60(void)
{
extern u32 D_801B2E48;
extern s32 D_801B2E4C;
extern void (*D_800D6C54[])(void);
extern s32 g_wmap_view_scroll_mode;
extern void func_800A7CB8__for_func_800A7C60(void) __asm__("func_800A7CB8");

    D_801B2E48 = 1;
    D_801B2E4C = 1;
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800A7C78(void)
{
extern u32 D_801B2E48;
extern s32 D_801B2E4C;
extern void (*D_800D6C54[])(void);
extern s32 g_wmap_view_scroll_mode;
extern void func_800A7CB8__for_func_800A7C78(void) __asm__("func_800A7CB8");

    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B2E48 += 1;
        func_800A7CB8__for_func_800A7C78();
    }
}

/** @brief Reset a world-map HUD sprite record, then bump its shared refcount. */
void func_800A7CB8(void)
{
extern u8 D_800D92EC[];
extern s32 D_801B2E48;
extern s32 D_801B2E4C;

    D_800D92EC[0x6] = 0xF;
    *(s16*)&D_800D92EC[0x10] = -1;
    *(s16*)&D_800D92EC[0x22] = 0x80;
    *(s16*)&D_800D92EC[0x2] = 0;
    *(s16*)&D_800D92EC[0xE] = 0;
    *(s16*)&D_800D92EC[0x24] = 0;
    *(s16*)&D_800D92EC[0x26] = 8;
    D_801B2E4C = 0x10;
    D_801B2E48 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A7D0C(void)
{
extern s32 D_801B2E4C;
extern s32 D_801B2E48;
extern void func_800A7D7C__for_func_800A7D0C(void) __asm__("func_800A7D7C");

    if (--D_801B2E4C == 0)
    {
        D_801B2E48 += 1;
    }
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800A7D40(void)
{
extern s32 D_801B2E4C;
extern s32 D_801B2E48;
extern void func_800A7D7C__for_func_800A7D40(void) __asm__("func_800A7D7C");

    if (g_wmap_scripted_travel_active == 0)
    {
        D_801B2E48 += 1;
        func_800A7D7C__for_func_800A7D40();
    }
}

/**
 * @brief World-map step: recompute the scroll offsets from the camera position and
 *        advance to the next handler.
 */
void func_800A7D7C(void)
{
extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 g_wmap_view[];
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_801B2E48;
extern void func_800A7E0C__for_func_800A7D7C(void) __asm__("func_800A7E0C");

    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = (g_wmap_vehicle_cell_x - 1) * 0x30 - g_wmap_view[0];
    g_wmap_scroll_remaining_y = (g_wmap_vehicle_cell_y - 1) * 0x30 - g_wmap_view[1];
    D_801B2E48 += 1;
    func_800A7E0C__for_func_800A7D7C();
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800A7E0C(void)
{
extern s32 g_wmap_view_scroll_mode;
extern s32 D_801B2E48;
extern void func_800A7E4C__for_func_800A7E0C(void) __asm__("func_800A7E4C");

    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B2E48 += 1;
        func_800A7E4C__for_func_800A7E0C();
    }
}

/**
 * @brief Reset a world-map step slot: arm its wait and advance the step index.
 */
void func_800A7E4C(void)
{
extern s32 D_801B2E48;
extern s32 D_801B2E4C;

    D_801B2E4C = 0x3C;
    D_801B2E48 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A7E6C(void)
{
extern s32 D_801B2E4C;
extern s32 D_801B2E48;

    if (--D_801B2E4C == 0)
    {
        D_801B2E48 += 1;
    }
}

/** @brief World-map step handler: clear a flag and advance the step. */
void func_800A7EA0(void)
{
extern s16 D_800D930E;
extern s32 D_801B2E48;
extern s32 D_801B2E4C;

    D_800D930E = 0;
    D_801B2E4C = 0x1E;
    D_801B2E48 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A7EC8(void)
{
extern s32 D_801B2E4C;
extern s32 D_801B2E48;

    if (--D_801B2E4C == 0)
    {
        D_801B2E48 += 1;
    }
}

/**
 * @brief World-map step handler: cache the relative scroll delta, bump the frame
 *        counter, and run the sub-step handler.
 */
void func_800A7EFC(void)
{
extern void func_800A7F6C__for_func_800A7EFC(void) __asm__("func_800A7F6C");
extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_saved_view[];
extern s32 g_wmap_view[];
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_801B2E48;

    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = g_wmap_saved_view[0] - g_wmap_view[0];
    g_wmap_scroll_remaining_y = g_wmap_saved_view[1] - g_wmap_view[1];
    D_801B2E48 += 1;
    func_800A7F6C__for_func_800A7EFC();
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800A7F6C(void)
{
extern s32 g_wmap_view_scroll_mode;
extern s32 D_801B2E48;
extern void func_800A7FAC__for_func_800A7F6C(void) __asm__("func_800A7FAC");

    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B2E48 += 1;
        func_800A7FAC__for_func_800A7F6C();
    }
}

/** @brief World-map step handler: clear two flags and advance the step counter. */
void func_800A7FAC(void)
{
extern s32 D_8013B208;
extern s32 D_801B2E48;

    g_wmap_input_locked = 0;
    D_8013B208 = 0;
    D_801B2E48 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A7FD0(s32 arg0)
{
extern u32 D_801B2E50;
extern s32 D_801B2E54;
extern void (*D_800D6C94[])(void);
extern s32 g_wmap_view_scroll_mode;
extern void func_800A80A0__for_func_800A7FD0(void) __asm__("func_800A80A0");

    s32 result;

    if (arg0 != 0)
    {
        D_801B2E50 = 1;
        D_801B2E54 = 1;
        return 1;
    }

    if (D_801B2E50 < 0x10)
    {
        D_800D6C94[D_801B2E50]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A8048(void)
{
extern u32 D_801B2E50;
extern s32 D_801B2E54;
extern void (*D_800D6C94[])(void);
extern s32 g_wmap_view_scroll_mode;
extern void func_800A80A0__for_func_800A8048(void) __asm__("func_800A80A0");

    D_801B2E50 = 1;
    D_801B2E54 = 1;
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800A8060(void)
{
extern u32 D_801B2E50;
extern s32 D_801B2E54;
extern void (*D_800D6C94[])(void);
extern s32 g_wmap_view_scroll_mode;
extern void func_800A80A0__for_func_800A8060(void) __asm__("func_800A80A0");

    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B2E50 += 1;
        func_800A80A0__for_func_800A8060();
    }
}

/** @brief Reset a world-map HUD sprite record, then bump its shared refcount. */
void func_800A80A0(void)
{
extern u8 D_800D92EC[];
extern s32 D_801B2E50;
extern s32 D_801B2E54;

    D_800D92EC[0x6] = 0xF;
    *(s16*)&D_800D92EC[0x10] = -1;
    *(s16*)&D_800D92EC[0x22] = 0x80;
    *(s16*)&D_800D92EC[0x2] = 0;
    *(s16*)&D_800D92EC[0xE] = 0;
    *(s16*)&D_800D92EC[0x24] = 0;
    *(s16*)&D_800D92EC[0x26] = 8;
    D_801B2E54 = 0x10;
    D_801B2E50 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A80F4(void)
{
extern s32 D_801B2E54;
extern s32 D_801B2E50;
extern void func_800A8164__for_func_800A80F4(void) __asm__("func_800A8164");

    if (--D_801B2E54 == 0)
    {
        D_801B2E50 += 1;
    }
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800A8128(void)
{
extern s32 D_801B2E54;
extern s32 D_801B2E50;
extern void func_800A8164__for_func_800A8128(void) __asm__("func_800A8164");

    if (g_wmap_scripted_travel_active == 0)
    {
        D_801B2E50 += 1;
        func_800A8164__for_func_800A8128();
    }
}

/**
 * @brief World-map step: recompute the scroll offsets from the camera position and
 *        advance to the next handler.
 */
void func_800A8164(void)
{
extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 g_wmap_view[];
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_801B2E50;
extern void func_800A81F4__for_func_800A8164(void) __asm__("func_800A81F4");

    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = (g_wmap_vehicle_cell_x - 1) * 0x30 - g_wmap_view[0];
    g_wmap_scroll_remaining_y = (g_wmap_vehicle_cell_y - 1) * 0x30 - g_wmap_view[1];
    D_801B2E50 += 1;
    func_800A81F4__for_func_800A8164();
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800A81F4(void)
{
extern s32 g_wmap_view_scroll_mode;
extern s32 D_801B2E50;
extern void func_800A8234__for_func_800A81F4(void) __asm__("func_800A8234");

    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B2E50 += 1;
        func_800A8234__for_func_800A81F4();
    }
}

/**
 * @brief Reset a world-map step slot: arm its wait and advance the step index.
 */
void func_800A8234(void)
{
extern s32 D_801B2E50;
extern s32 D_801B2E54;

    D_801B2E54 = 0x3C;
    D_801B2E50 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A8254(void)
{
extern s32 D_801B2E54;
extern s32 D_801B2E50;

    if (--D_801B2E54 == 0)
    {
        D_801B2E50 += 1;
    }
}

/** @brief World-map step handler: clear a flag and advance the step. */
void func_800A8288(void)
{
extern s16 D_800D930E;
extern s32 D_801B2E50;
extern s32 D_801B2E54;

    D_800D930E = 0;
    D_801B2E54 = 0x1E;
    D_801B2E50 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A82B0(void)
{
extern s32 D_801B2E54;
extern s32 D_801B2E50;

    if (--D_801B2E54 == 0)
    {
        D_801B2E50 += 1;
    }
}

/**
 * @brief World-map step handler: cache the relative scroll delta, bump the frame
 *        counter, and run the sub-step handler.
 */
void func_800A82E4(void)
{
extern void func_800A8354__for_func_800A82E4(void) __asm__("func_800A8354");
extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_saved_view[];
extern s32 g_wmap_view[];
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_801B2E50;

    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = g_wmap_saved_view[0] - g_wmap_view[0];
    g_wmap_scroll_remaining_y = g_wmap_saved_view[1] - g_wmap_view[1];
    D_801B2E50 += 1;
    func_800A8354__for_func_800A82E4();
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800A8354(void)
{
extern s32 g_wmap_view_scroll_mode;
extern s32 D_801B2E50;
extern void func_800A8394__for_func_800A8354(void) __asm__("func_800A8394");

    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B2E50 += 1;
        func_800A8394__for_func_800A8354();
    }
}

/** @brief World-map step handler: clear two flags and advance the step counter. */
void func_800A8394(void)
{
extern s32 D_8013B208;
extern s32 D_801B2E50;

    g_wmap_input_locked = 0;
    D_8013B208 = 0;
    D_801B2E50 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A83B8(s32 arg0)
{
extern u32 D_801B2E58;
extern s32 D_801B2E5C;
extern void (*D_800D6CD4[])(void);
extern s32 g_wmap_view_scroll_mode;
extern void func_800A8488__for_func_800A83B8(void) __asm__("func_800A8488");

    s32 result;

    if (arg0 != 0)
    {
        D_801B2E58 = 1;
        D_801B2E5C = 1;
        return 1;
    }

    if (D_801B2E58 < 0x10)
    {
        D_800D6CD4[D_801B2E58]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A8430(void)
{
extern u32 D_801B2E58;
extern s32 D_801B2E5C;
extern void (*D_800D6CD4[])(void);
extern s32 g_wmap_view_scroll_mode;
extern void func_800A8488__for_func_800A8430(void) __asm__("func_800A8488");

    D_801B2E58 = 1;
    D_801B2E5C = 1;
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800A8448(void)
{
extern u32 D_801B2E58;
extern s32 D_801B2E5C;
extern void (*D_800D6CD4[])(void);
extern s32 g_wmap_view_scroll_mode;
extern void func_800A8488__for_func_800A8448(void) __asm__("func_800A8488");

    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B2E58 += 1;
        func_800A8488__for_func_800A8448();
    }
}

/** @brief Reset a world-map HUD sprite record, then bump its shared refcount. */
void func_800A8488(void)
{
extern u8 D_800D92EC[];
extern s32 D_801B2E58;
extern s32 D_801B2E5C;

    D_800D92EC[0x6] = 0xF;
    *(s16*)&D_800D92EC[0x10] = -1;
    *(s16*)&D_800D92EC[0x22] = 0x80;
    *(s16*)&D_800D92EC[0x2] = 0;
    *(s16*)&D_800D92EC[0xE] = 0;
    *(s16*)&D_800D92EC[0x24] = 0;
    *(s16*)&D_800D92EC[0x26] = 8;
    D_801B2E5C = 0x10;
    D_801B2E58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A84DC(void)
{
extern s32 D_801B2E5C;
extern s32 D_801B2E58;
extern void func_800A854C__for_func_800A84DC(void) __asm__("func_800A854C");

    if (--D_801B2E5C == 0)
    {
        D_801B2E58 += 1;
    }
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800A8510(void)
{
extern s32 D_801B2E5C;
extern s32 D_801B2E58;
extern void func_800A854C__for_func_800A8510(void) __asm__("func_800A854C");

    if (g_wmap_scripted_travel_active == 0)
    {
        D_801B2E58 += 1;
        func_800A854C__for_func_800A8510();
    }
}

/**
 * @brief World-map step: recompute the scroll offsets from the camera position and
 *        advance to the next handler.
 */
void func_800A854C(void)
{
extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 g_wmap_view[];
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_801B2E58;
extern void func_800A85DC__for_func_800A854C(void) __asm__("func_800A85DC");

    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = (g_wmap_vehicle_cell_x - 1) * 0x30 - g_wmap_view[0];
    g_wmap_scroll_remaining_y = (g_wmap_vehicle_cell_y - 1) * 0x30 - g_wmap_view[1];
    D_801B2E58 += 1;
    func_800A85DC__for_func_800A854C();
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800A85DC(void)
{
extern s32 g_wmap_view_scroll_mode;
extern s32 D_801B2E58;
extern void func_800A861C__for_func_800A85DC(void) __asm__("func_800A861C");

    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B2E58 += 1;
        func_800A861C__for_func_800A85DC();
    }
}

/** @brief Issue audio command F1, start a 60-tick delay, and advance the state. */
void func_800A861C(void)
{
extern void akao_cmd_f1__for_func_800A861C(void) __asm__("akao_cmd_f1");
extern s32 D_801B2E58;
extern s32 D_801B2E5C;

    akao_cmd_f1__for_func_800A861C();
    D_801B2E5C = 60;
    D_801B2E58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A8654(void)
{
extern s32 D_801B2E5C;
extern s32 D_801B2E58;

    if (--D_801B2E5C == 0)
    {
        D_801B2E58 += 1;
    }
}

/** @brief Play sound 57, clear its field, and start a 30-tick delay. */
void func_800A8688(void)
{
extern s16 D_800D930E;
extern s32 D_801B2E58;
extern s32 D_801B2E5C;

    wmap_play_sound(0x39, 0x80);
    D_800D930E = 0;
    D_801B2E5C = 0x1E;
    D_801B2E58 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_800A86CC(void)
{
extern s32 D_801B2E5C;
extern s32 D_801B2E58;

    if (--D_801B2E5C == 0)
    {
        D_801B2E58 += 1;
    }
}

/**
 * @brief World-map step handler: cache the relative scroll delta, bump the frame
 *        counter, and run the sub-step handler.
 */
void func_800A8700(void)
{
extern void func_800A8770__for_func_800A8700(void) __asm__("func_800A8770");
extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_saved_view[];
extern s32 g_wmap_view[];
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern s32 D_801B2E58;

    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = g_wmap_saved_view[0] - g_wmap_view[0];
    g_wmap_scroll_remaining_y = g_wmap_saved_view[1] - g_wmap_view[1];
    D_801B2E58 += 1;
    func_800A8770__for_func_800A8700();
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800A8770(void)
{
extern s32 g_wmap_view_scroll_mode;
extern s32 D_801B2E58;
extern void func_800A87B0__for_func_800A8770(void) __asm__("func_800A87B0");

    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B2E58 += 1;
        func_800A87B0__for_func_800A8770();
    }
}

/** @brief World-map step handler: clear two flags and advance the step counter. */
void func_800A87B0(void)
{
extern s32 D_8013B208;
extern s32 D_801B2E58;

    g_wmap_input_locked = 0;
    D_8013B208 = 0;
    D_801B2E58 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A87D4(s32 arg0)
{
extern u32 D_801B2E60;
extern s32 D_801B2E64;
extern void (*D_800D6D14[])(void);
extern s32 g_wmap_view_scroll_mode;
extern void func_800A88A4__for_func_800A87D4(void) __asm__("func_800A88A4");

    s32 result;

    if (arg0 != 0)
    {
        D_801B2E60 = 1;
        D_801B2E64 = 1;
        return 1;
    }

    if (D_801B2E60 < 0x4)
    {
        D_800D6D14[D_801B2E60]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A884C(void)
{
extern u32 D_801B2E60;
extern s32 D_801B2E64;
extern void (*D_800D6D14[])(void);
extern s32 g_wmap_view_scroll_mode;
extern void func_800A88A4__for_func_800A884C(void) __asm__("func_800A88A4");

    D_801B2E60 = 1;
    D_801B2E64 = 1;
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800A8864(void)
{
extern u32 D_801B2E60;
extern s32 D_801B2E64;
extern void (*D_800D6D14[])(void);
extern s32 g_wmap_view_scroll_mode;
extern void func_800A88A4__for_func_800A8864(void) __asm__("func_800A88A4");

    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B2E60 += 1;
        func_800A88A4__for_func_800A8864();
    }
}

/** @brief World-map step handler: kick two sub-tasks and expire the step counter. */
void func_800A88A4(void)
{
extern s32 D_801B2E60;
extern void func_800591A8(s32 arg0);

    func_800A89DC(0x17);
    func_800591A8(0x17);
    D_801B2E60 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_800A88D8(s32 arg0)
{
extern u32 D_801B2E68;
extern s32 D_801B2E6C;
extern void (*D_800D6D24[])(void);
extern s32 g_wmap_view_scroll_mode;
extern void func_800A89A8__for_func_800A88D8(void) __asm__("func_800A89A8");

    s32 result;

    if (arg0 != 0)
    {
        D_801B2E68 = 1;
        D_801B2E6C = 1;
        return 1;
    }

    if (D_801B2E68 < 0x4)
    {
        D_800D6D24[D_801B2E68]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_800A8950(void)
{
extern u32 D_801B2E68;
extern s32 D_801B2E6C;
extern void (*D_800D6D24[])(void);
extern s32 g_wmap_view_scroll_mode;
extern void func_800A89A8__for_func_800A8950(void) __asm__("func_800A89A8");

    D_801B2E68 = 1;
    D_801B2E6C = 1;
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800A8968(void)
{
extern u32 D_801B2E68;
extern s32 D_801B2E6C;
extern void (*D_800D6D24[])(void);
extern s32 g_wmap_view_scroll_mode;
extern void func_800A89A8__for_func_800A8968(void) __asm__("func_800A89A8");

    if (g_wmap_view_scroll_mode != 2)
    {
        D_801B2E68 += 1;
        func_800A89A8__for_func_800A8968();
    }
}

/** @brief World-map step handler: kick two sub-tasks and expire the step counter. */
void func_800A89A8(void)
{
extern s32 D_801B2E68;
extern void func_800591A8(s32 arg0);

    func_800A89DC(0x16);
    func_800591A8(0x16);
    D_801B2E68 += 1;
}
