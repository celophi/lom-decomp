#include "wmap_main.h"
#include "wmap_party_travel.h"
#include "wmap_travel_sequences.h"
#include "wmap_resource_support.h"
#include "wmap_sequence_runtime.h"
#include "wmap_sprite_render.h"
#include "sdk/abs.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
#include "wmap_map_labels.h"
#include "wmap_effect_resources.h"
#include "cdrom.h"

/** @brief Land the special travel starts from and the land the return trip ends on. */
#define WMAP_TRAVEL_START_LAND 2
#define WMAP_RETURN_LAND 0

/** @brief Map distance of one land cell. */
#define WMAP_CELL_SPACING 48

/** @brief Vehicle heading: 12-bit angle, turn speed, and the headings the sequences wait for. */
#define WMAP_HEADING_MASK 0xFFF
#define WMAP_HEADING_TURN_SPEED 32
#define WMAP_HEADING_DEPARTURE 0x200
#define WMAP_HEADING_CRUISE 0x400
#define WMAP_HEADING_LANDING 0x3E0

/** @brief Vehicle sprite: texture, draw variant, ordering-table depths and flap sound. */
#define WMAP_VEHICLE_TEXTURE 40
#define WMAP_VEHICLE_VARIANT 2
#define WMAP_VEHICLE_OT_FRONT 50
#define WMAP_VEHICLE_OT_BEHIND 61
#define WMAP_VEHICLE_OT_START 60
#define WMAP_VEHICLE_OT_FLIGHT 59
#define WMAP_VEHICLE_SOUND_FLAP 51
#define WMAP_VEHICLE_SOUND_PAN 127
#define WMAP_VEHICLE_SCALE_MAX 15
#define WMAP_VEHICLE_SHADE_NEUTRAL 128

/** @brief Vehicle orbit: start radius and height, their limits, and the climb step. */
#define WMAP_VEHICLE_START_RADIUS 200
#define WMAP_VEHICLE_START_HEIGHT 10
#define WMAP_VEHICLE_MAX_RADIUS 480
#define WMAP_VEHICLE_MAX_HEIGHT 100
#define WMAP_VEHICLE_RADIUS_STEP 4
#define WMAP_VEHICLE_LOW_HEIGHT 40

/** @brief Size of one vehicle animation bank in D_8011D538 and the CD files that fill it. */
#define WMAP_VEHICLE_BANK_SIZE 0x2000
#define WMAP_VEHICLE_BANK_FILE 0x1145
#define WMAP_VEHICLE_BANK_FILE_2 0x1146
#define WMAP_VEHICLE_TEXTURE_FILE 0x1147
#define WMAP_VEHICLE_PALETTE_FILE 0x1148

/** @brief Sequence timers, in frames. */
#define WMAP_VEHICLE_CLIMB_FRAMES 60
#define WMAP_VEHICLE_TURN_FRAMES 300
#define WMAP_VEHICLE_WAIT_FRAMES 120

/** @brief Step counts of the two sequences. */
#define WMAP_SPECIAL_TRAVEL_STEPS 20
#define WMAP_SPECIAL_RETURN_STEPS 14

/** @brief Route directions (dx + 1) + (dy + 1) * 3; the center value 4 is unused. */
#define WMAP_ROUTE_NONE (-1)
#define WMAP_ROUTE_UP 1
#define WMAP_ROUTE_LEFT 3
#define WMAP_ROUTE_RIGHT 5
#define WMAP_ROUTE_DOWN 7

/** @brief Element 0 of the D_801AFBD0 motion table while it drives the travel vehicle. */
typedef struct
{
    s16 active;
    s16 heading;
    s32 ot_override;
    s32 radius;
    s16 unknown_0c;
    s16 height;
    s16 ot_index;
    s16 unknown_12;
} WmapVehicleMotion;

/** @brief Animation resource slot of a sprite actor. */
typedef struct
{
    s32 unknown_00;
    u8* data;
} WmapAnimationSlot;

/** @brief GTE screen coordinate, read as one packed word or as two halves. */
typedef union
{
    s32 packed;
    struct
    {
        s16 x;
        s16 y;
    } point;
} WmapScreenPosition;

/** @brief Map scroll position and projection scale (see wmap_view_effects.c). */
typedef struct
{
    s32 x;
    s32 y;
    s32 projection_scale;
    s32 unknown_0c;
} WmapView;

/** @brief First word of a map cell: the land placed there. */
typedef struct
{
    s32 land_id;
    u8 unknown_04[36];
} WmapLandCell;

extern WmapSpriteActor g_wmap_vehicle_actor;
extern WmapVehicleMotion D_801AFBD0;
extern WmapAnimationSlot g_wmap_vehicle_animation;
extern u8 D_8011D538[];
extern s32 g_wmap_vehicle_bank_flipped;
extern MATRIX D_8011D0E8;
extern s32 D_80139224;
extern WmapScreenPosition g_wmap_vehicle_screen_position;
extern s32 D_8011CF74;
extern u8 D_80182E40[];
extern u8 D_8018B240[];
extern WmapSpriteActor D_800D9268[];

extern s16 g_wmap_route_headings[];
extern s32 g_wmap_vehicle_route;
extern s32 g_wmap_vehicle_phase;
extern s32 g_wmap_vehicle_flying;
extern s32 g_wmap_vehicle_cell_x;
extern s32 g_wmap_vehicle_cell_y;
extern s32 g_wmap_vehicle_target_x;
extern s32 g_wmap_vehicle_target_y;
extern WmapLandCell D_80139290[][6];

extern s32 g_wmap_view_scroll_mode;
extern s32 g_wmap_scroll_remaining_x;
extern s32 g_wmap_scroll_remaining_y;
extern WmapView g_wmap_view;
extern WmapView g_wmap_saved_view;
extern s32 D_800DCEC0;
extern s32 D_8013B208;
extern s32 D_8013B288;
extern s32 D_8013B294;

extern u32 g_wmap_special_travel_step;
extern s32 g_wmap_special_travel_timer;
extern void (*g_wmap_special_travel_steps[])(void);
extern u32 g_wmap_special_return_step;
extern s32 g_wmap_special_return_timer;
extern void (*g_wmap_special_return_steps[])(void);

static s32 wmap_start_vehicle_flight(s32 initialize);
static s32 wmap_fly_vehicle(s32 initialize);
static s32 wmap_fly_vehicle_first_leg(s32 initialize);

/* Sequence steps: called through the step tables and by the step before them. */
void wmap_special_travel_spawn_vehicle(void);
void wmap_special_travel_climb_vehicle(void);
void wmap_special_return_prepare(void);
void wmap_special_return_spawn_vehicle(void);
void wmap_special_return_climb_vehicle(void);
void wmap_special_travel_reset(void);
void wmap_special_travel_prepare(void);
void wmap_special_travel_wait(void);
void wmap_special_travel_scroll_to_start(void);
void wmap_special_travel_wait_scroll(void);
void wmap_special_travel_fly_to_party(void);
void wmap_special_travel_wait_party(void);
void wmap_special_travel_start_turn(void);
void wmap_special_travel_turn(void);
void wmap_special_travel_fly_to_land(void);
void wmap_special_travel_wait_land(void);
void wmap_special_travel_start_cruise(void);
void wmap_special_travel_cruise(void);
void wmap_special_travel_start_landing(void);
void wmap_special_travel_wait_landing(void);
void wmap_special_travel_stop_vehicle(void);
void wmap_special_travel_fly_away(void);
void wmap_special_travel_finish(void);
void wmap_special_return_reset(void);
void wmap_special_return_wait_scroll(void);
void wmap_special_return_fly_home(void);
void wmap_special_return_wait_home(void);
void wmap_special_return_start_cruise(void);
void wmap_special_return_cruise(void);
void wmap_special_return_start_landing(void);
void wmap_special_return_wait_landing(void);
void wmap_special_return_stop_vehicle(void);
void wmap_special_return_fly_away(void);
void wmap_special_return_finish(void);

/** @brief Lock input, save the map view, and queue the vehicle animation banks and texture. */
static inline void wmap_load_vehicle(void)
{
    D_8013B288 = 0;
    g_wmap_input_locked = 1;
    g_wmap_vehicle_bank_flipped = 0;
    D_8013B208 = 1;
    func_8005FF88(-1);
    D_800DCEC0 = 0;
    g_wmap_saved_view = g_wmap_view;
    cdrom_queue_read(WMAP_VEHICLE_BANK_FILE, D_8011D538);
    cdrom_queue_read(WMAP_VEHICLE_BANK_FILE_2, D_8011D538 + WMAP_VEHICLE_BANK_SIZE);
    func_800A8AA8(WMAP_VEHICLE_TEXTURE_FILE);
    func_800A8AF0(WMAP_VEHICLE_PALETTE_FILE);
}

/** @brief Upload the vehicle resources, reset its sprite and orbit, and start drawing it. */
static inline void wmap_place_vehicle(void)
{
    WmapSpriteActor* actor = &g_wmap_vehicle_actor;

    cdrom_wait_queue_empty();
    func_800651B4(D_80182E40);
    func_800651B4(D_8018B240);
    g_wmap_vehicle_animation.data = D_8011D538;
    actor->previous_sequence = -1;
    actor->target_shade = WMAP_VEHICLE_SHADE_NEUTRAL;
    actor->shade = WMAP_VEHICLE_SHADE_NEUTRAL;
    actor->unknown_02 = 0;
    actor->scale_index = 0;
    actor->sequence = 0;
    actor->shade_step = 0;
    D_801AFBD0.active = 1;
    D_801AFBD0.radius = WMAP_VEHICLE_START_RADIUS;
    D_801AFBD0.height = WMAP_VEHICLE_START_HEIGHT;
    D_801AFBD0.heading = 0;
    D_801AFBD0.ot_index = WMAP_VEHICLE_OT_START;
    D_801AFBD0.ot_override = 0;
    wmap_install_callback(wmap_draw_vehicle);
}

/** @brief Turn the vehicle while it climbs away from the center and grows. */
static inline void wmap_climb_vehicle(void)
{
    WmapSpriteActor* actor;

    actor = wmap_turn_vehicle(1);
    if (D_801AFBD0.height < WMAP_VEHICLE_MAX_HEIGHT)
    {
        D_801AFBD0.height += 1;
    }
    if (D_801AFBD0.radius < WMAP_VEHICLE_MAX_RADIUS)
    {
        D_801AFBD0.radius += WMAP_VEHICLE_RADIUS_STEP;
    }
    if ((D_8011CF74 & 3) == 0)
    {
        if (actor->scale_index < WMAP_VEHICLE_SCALE_MAX)
        {
            actor->scale_index += 1;
        }
    }
}

/**
 * @brief Update the vehicle heading and pick its facing, animation bank and depth.
 * @param advance Nonzero turns the vehicle by one step first.
 * @return The vehicle sprite actor.
 * @note Headings from 0x800 use the mirrored second animation bank.
 */
WmapSpriteActor* wmap_turn_vehicle(s32 advance)
{
    s32 facing_angle;
    s32 heading;
    WmapSpriteActor* actor;

    actor = &g_wmap_vehicle_actor;
    /* The heading is read unsigned here (lhu); the other users read it signed. */
    if (advance != 0)
    {
        WmapVehicleMotion* motion = &D_801AFBD0;
        s32 angle = (u16)motion->heading + WMAP_HEADING_TURN_SPEED;

        angle &= WMAP_HEADING_MASK;
        heading = motion->heading = angle;
    }
    else
    {
        WmapVehicleMotion* motion = &D_801AFBD0;
        s32 angle = (u16)motion->heading;

        angle &= WMAP_HEADING_MASK;
        heading = motion->heading = angle;
    }
    facing_angle = heading + 0x200;
    actor->sequence = (facing_angle >> 9) & 3;
    if (facing_angle & 0x800)
    {
        if (g_wmap_vehicle_bank_flipped == 0)
        {
            actor->previous_sequence = -1;
        }
        g_wmap_vehicle_animation.data = &D_8011D538[WMAP_VEHICLE_BANK_SIZE];
        g_wmap_vehicle_bank_flipped = 1;
    }
    else
    {
        if (g_wmap_vehicle_bank_flipped != 0)
        {
            actor->previous_sequence = -1;
        }
        g_wmap_vehicle_animation.data = D_8011D538;
        g_wmap_vehicle_bank_flipped = 0;
    }
    if ((u32)(heading - 0x401) < 0x7FFU && D_801AFBD0.height < WMAP_VEHICLE_LOW_HEIGHT)
    {
        heading = WMAP_VEHICLE_OT_BEHIND;
    }
    else
    {
        heading = WMAP_VEHICLE_OT_FRONT;
    }
    D_801AFBD0.ot_index = heading;
    return actor;
}

/** @brief Special travel step 5: load the vehicle, place it on its orbit and start drawing it. */
void wmap_special_travel_spawn_vehicle(void)
{
    wmap_place_vehicle();
    g_wmap_special_travel_timer = WMAP_VEHICLE_CLIMB_FRAMES;
    g_wmap_special_travel_step += 1;
    wmap_special_travel_climb_vehicle();
}

/** @brief Special travel step 6: turn the vehicle while it climbs and grows, until the timer ends. */
void wmap_special_travel_climb_vehicle(void)
{
    wmap_climb_vehicle();
    if (--g_wmap_special_travel_timer == 0)
    {
        g_wmap_special_travel_step += 1;
    }
}

/**
 * @brief Project and draw the vehicle on its orbit, playing the flap sound on wing frames.
 * @param initialize Sequence callback flag; unused.
 * @return Nonzero while the vehicle is active, so the callback stays installed.
 */
s32 wmap_draw_vehicle(s32 initialize)
{
    SVECTOR position;
    s32 angle;
    WmapSpriteActor* actor = &g_wmap_vehicle_actor;
    s32 animation_frame;

    angle = D_801AFBD0.heading;
    PushMatrix();
    SetRotMatrix(&D_8011D0E8);
    SetTransMatrix(&D_8011D0E8);
    position.vx = ((D_801AFBD0.radius >> 4) * (ccos(angle) >> 4)) >> 6;
    position.vy = ((D_801AFBD0.radius >> 4) * (csin(angle) >> 4)) >> 6;
    position.vz = D_801AFBD0.height;
    gte_ldv0(&position);
    gte_rtps();
    animation_frame = wmap_step_actor_animation(actor, &g_wmap_vehicle_animation);
    if (D_80139224 != 0)
    {
        if (animation_frame == 12 || animation_frame == 22 || animation_frame == 0)
        {
            wmap_play_sound(WMAP_VEHICLE_SOUND_FLAP, WMAP_VEHICLE_SOUND_PAN);
        }
    }
    gte_stsxy(&g_wmap_vehicle_screen_position);
    if (D_801AFBD0.ot_override != 0)
    {
        wmap_draw_actor_sprite(actor, g_wmap_vehicle_screen_position.packed, WMAP_VEHICLE_TEXTURE, D_801AFBD0.ot_override, WMAP_VEHICLE_VARIANT);
    }
    else
    {
        wmap_draw_actor_sprite(actor, g_wmap_vehicle_screen_position.packed, WMAP_VEHICLE_TEXTURE, D_801AFBD0.ot_index, WMAP_VEHICLE_VARIANT);
    }
    PopMatrix();
    return D_801AFBD0.active;
}

/**
 * @brief Plan the flight from the vehicle cell to the target cell and install its callback.
 * @param initialize Sequence callback flag; unused.
 * @return Always 0: the callback only runs once.
 * @note A straight or exactly diagonal route flies directly; any other route first flies
 *       the straight part that makes the rest diagonal.
 */
static s32 wmap_start_vehicle_flight(s32 initialize)
{
    s32 dx;
    s32 dy;
    s32 delta;
    s32 difference;
    s32 sign;
    s32 leg_dy;
    s32 magnitude_x;
    s32 magnitude_y;

    g_wmap_vehicle_route = WMAP_ROUTE_NONE;
    g_wmap_vehicle_phase = 0;
    if ((g_wmap_vehicle_cell_x == g_wmap_vehicle_target_x && g_wmap_vehicle_cell_y != g_wmap_vehicle_target_y) ||
        (g_wmap_vehicle_cell_x != g_wmap_vehicle_target_x && g_wmap_vehicle_cell_y == g_wmap_vehicle_target_y))
    {
        dx = g_wmap_vehicle_target_x - g_wmap_vehicle_cell_x;
        dy = g_wmap_vehicle_target_y - g_wmap_vehicle_cell_y;
        if (dx != 0)
        {
            dx = dx / abs(dx) + 1;
        }
        else
        {
            dx = 1;
        }
        if (dy != 0)
        {
            dy = dy / abs(dy) + 1;
        }
        else
        {
            dy = 1;
        }
        g_wmap_vehicle_route = dx + dy * 3;
    }
    else
    {
        delta = g_wmap_vehicle_target_x - g_wmap_vehicle_cell_x;
        dy = g_wmap_vehicle_target_y - g_wmap_vehicle_cell_y;
        if (abs(delta) == abs(dy))
        {
            dx = delta;
            if (dx != 0)
            {
                dx = dx / abs(dx) + 1;
            }
            else
            {
                dx = 1;
            }
            if (dy != 0)
            {
                dy = dy / abs(dy) + 1;
            }
            else
            {
                dy = 1;
            }
            g_wmap_vehicle_route = dx + dy * 3;
        }
    }
    if (g_wmap_vehicle_route != WMAP_ROUTE_NONE)
    {
        g_wmap_view_scroll_mode = 2;
        wmap_install_callback(wmap_fly_vehicle);
    }
    else
    {
        dx = g_wmap_vehicle_target_x - g_wmap_vehicle_cell_x;
        leg_dy = g_wmap_vehicle_target_y - g_wmap_vehicle_cell_y;
        magnitude_x = abs(dx);
        magnitude_y = abs(leg_dy);
        difference = magnitude_x - magnitude_y;
        if (difference > 0)
        {
            sign = 0;
            if (dx != 0)
            {
                sign = -1;
                if (dx > 0)
                {
                    sign = 1;
                }
            }
            difference = difference * sign;
            g_wmap_scroll_remaining_y = 0;
            g_wmap_scroll_remaining_x = difference * WMAP_CELL_SPACING;
            g_wmap_vehicle_cell_x += difference;
            if (dx > 0)
            {
                g_wmap_vehicle_route = WMAP_ROUTE_RIGHT;
            }
            else
            {
                g_wmap_vehicle_route = WMAP_ROUTE_LEFT;
            }
        }
        else
        {
            delta = g_wmap_vehicle_cell_y - g_wmap_vehicle_target_y;
            sign = 0;
            if (delta != 0)
            {
                sign = -1;
                /* dx is reused for the test; a separate temporary changes the register allocation. */
                dx = delta > 0;
                if (dx)
                {
                    sign = 1;
                }
            }
            difference = difference * sign;
            g_wmap_scroll_remaining_x = 0;
            g_wmap_scroll_remaining_y = difference * WMAP_CELL_SPACING;
            g_wmap_vehicle_cell_y += difference;
            if (leg_dy > 0)
            {
                g_wmap_vehicle_route = WMAP_ROUTE_DOWN;
            }
            else
            {
                g_wmap_vehicle_route = WMAP_ROUTE_UP;
            }
        }
        wmap_install_callback(wmap_fly_vehicle_first_leg);
    }
    return 0;
}

/**
 * @brief Turn the vehicle to its route, then scroll the map to the target cell.
 * @param initialize Sequence callback flag; unused.
 * @return 1 while the flight is running, 0 once the vehicle has arrived.
 */
static s32 wmap_fly_vehicle(s32 initialize)
{
    if (g_wmap_vehicle_phase == 0)
    {
        wmap_turn_vehicle(1);
        if (D_801AFBD0.heading == g_wmap_route_headings[g_wmap_vehicle_route])
        {
            g_wmap_view_scroll_mode = 2;
            g_wmap_scroll_remaining_x = (g_wmap_vehicle_target_x - g_wmap_vehicle_cell_x) * WMAP_CELL_SPACING;
            g_wmap_scroll_remaining_y = (g_wmap_vehicle_target_y - g_wmap_vehicle_cell_y) * WMAP_CELL_SPACING;
            g_wmap_vehicle_phase += 1;
        }
        return 1;
    }
    if (g_wmap_view_scroll_mode == 2)
    {
        return 1;
    }
    g_wmap_vehicle_flying = 0;
    g_wmap_vehicle_cell_x = g_wmap_vehicle_target_x;
    g_wmap_vehicle_cell_y = g_wmap_vehicle_target_y;
    return 0;
}

/**
 * @brief Fly the straight first leg of a two-leg flight, then plan the diagonal rest.
 * @param initialize Sequence callback flag; unused.
 * @return 1 while the leg is running, 0 once the diagonal leg is installed.
 */
static s32 wmap_fly_vehicle_first_leg(s32 initialize)
{
    s32 delta_y;
    s32 delta_x;

    if (g_wmap_vehicle_phase == 0)
    {
        wmap_turn_vehicle(1);
        if (D_801AFBD0.heading == g_wmap_route_headings[g_wmap_vehicle_route])
        {
            g_wmap_view_scroll_mode = 2;
            g_wmap_vehicle_phase++;
        }
        return 1;
    }
    if (g_wmap_view_scroll_mode == 2)
    {
        return 1;
    }
    g_wmap_vehicle_phase = 0;
    delta_x = g_wmap_vehicle_target_x - g_wmap_vehicle_cell_x;
    delta_y = g_wmap_vehicle_target_y - g_wmap_vehicle_cell_y;
    if (delta_x != 0)
    {
        delta_x = (delta_x / abs(delta_x)) + 1;
    }
    else
    {
        delta_x = 1;
    }
    if (delta_y != 0)
    {
        delta_y = (delta_y / abs(delta_y)) + 1;
    }
    else
    {
        delta_y = 1;
    }
    g_wmap_vehicle_route = delta_x + (delta_y * 3);
    wmap_install_callback(wmap_fly_vehicle);
    return 0;
}

/** @brief Special return step 1: lock input, load the vehicle and scroll to the special land. */
void wmap_special_return_prepare(void)
{
    wmap_load_vehicle();
    wmap_find_land_cell(WMAP_SPECIAL_TRAVEL_LAND, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * WMAP_CELL_SPACING) - g_wmap_view.x;
    g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * WMAP_CELL_SPACING) - g_wmap_view.y;
    g_wmap_special_return_step++;
    wmap_special_return_wait_scroll();
}

/** @brief Special return step 3: as wmap_special_travel_spawn_vehicle, on the return sequence. */
void wmap_special_return_spawn_vehicle(void)
{
    wmap_place_vehicle();
    g_wmap_special_return_timer = WMAP_VEHICLE_CLIMB_FRAMES;
    g_wmap_special_return_step += 1;
    wmap_special_return_climb_vehicle();
}

/** @brief Special return step 4: as wmap_special_travel_climb_vehicle, on the return sequence. */
void wmap_special_return_climb_vehicle(void)
{
    wmap_climb_vehicle();
    if (--g_wmap_special_return_timer == 0)
    {
        g_wmap_special_return_step += 1;
    }
}

/**
 * @brief Turn the vehicle until it faces the landing heading.
 * @param initialize Sequence callback flag; unused.
 * @return 1 while turning, 0 once the heading is reached.
 */
s32 wmap_finish_vehicle_turn(s32 initialize)
{
    wmap_turn_vehicle(1);
    if (D_801AFBD0.heading == WMAP_HEADING_LANDING)
    {
        g_wmap_vehicle_phase = 0;
        return 0;
    }
    return 1;
}

/**
 * @brief Run the current step of the special travel sequence (flight to the special land).
 * @param initialize Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once every step has run.
 */
s32 wmap_run_special_travel(s32 initialize)
{
    s32 result;

    if (initialize != 0)
    {
        g_wmap_special_travel_step = 1;
        g_wmap_special_travel_timer = 1;
        return 1;
    }

    if (g_wmap_special_travel_step < WMAP_SPECIAL_TRAVEL_STEPS)
    {
        g_wmap_special_travel_steps[g_wmap_special_travel_step]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/** @brief Special travel step 0: restart the sequence. */
void wmap_special_travel_reset(void)
{
    g_wmap_special_travel_step = 1;
    g_wmap_special_travel_timer = 1;
}

/** @brief Special travel step 1: lock input, save the view and load the vehicle. */
void wmap_special_travel_prepare(void)
{
    D_80139224 = 1;
    wmap_load_vehicle();
    g_wmap_special_travel_timer = 1;
    g_wmap_special_travel_step += 1;
}

/** @brief Special travel step 2: wait for the step timer. */
void wmap_special_travel_wait(void)
{
    if (--g_wmap_special_travel_timer == 0)
    {
        g_wmap_special_travel_step += 1;
    }
}

/** @brief Special travel step 3: scroll the map to the vehicle's start land. */
void wmap_special_travel_scroll_to_start(void)
{
    wmap_find_land_cell(WMAP_TRAVEL_START_LAND, &g_wmap_vehicle_cell_x, &g_wmap_vehicle_cell_y);
    g_wmap_view_scroll_mode = 2;
    g_wmap_scroll_remaining_x = ((g_wmap_vehicle_cell_x - 1) * WMAP_CELL_SPACING) - g_wmap_view.x;
    g_wmap_scroll_remaining_y = ((g_wmap_vehicle_cell_y - 1) * WMAP_CELL_SPACING) - g_wmap_view.y;
    g_wmap_special_travel_step += 1;
    wmap_special_travel_wait_scroll();
}

/** @brief Special travel step 4: wait for the map scroll, then spawn the vehicle. */
void wmap_special_travel_wait_scroll(void)
{
    if (g_wmap_view_scroll_mode != 2)
    {
        g_wmap_special_travel_step += 1;
        wmap_special_travel_spawn_vehicle();
    }
}

/** @brief Special travel step 7: fly the vehicle to the party unless the party is at the start land. */
void wmap_special_travel_fly_to_party(void)
{
    D_801AFBD0.ot_index = WMAP_VEHICLE_OT_FLIGHT;
    if (D_80139290[g_wmap_travelers[0].cell_x][g_wmap_travelers[0].cell_y].land_id == WMAP_TRAVEL_START_LAND)
    {
        g_wmap_vehicle_flying = 0;
    }
    else
    {
        g_wmap_vehicle_target_x = g_wmap_travelers[0].cell_x;
        g_wmap_vehicle_target_y = g_wmap_travelers[0].cell_y;
        g_wmap_vehicle_flying = 1;
        wmap_install_callback(wmap_start_vehicle_flight);
    }
    g_wmap_special_travel_step++;
    wmap_special_travel_wait_party();
}

/** @brief Special travel step 8: wait for the flight to the party. */
void wmap_special_travel_wait_party(void)
{
    if (g_wmap_vehicle_flying == 0)
    {
        g_wmap_special_travel_step += 1;
        wmap_special_travel_start_turn();
    }
}

/** @brief Special travel step 9: start the turn to the departure heading. */
void wmap_special_travel_start_turn(void)
{
    g_wmap_special_travel_timer = WMAP_VEHICLE_TURN_FRAMES;
    g_wmap_special_travel_step += 1;
    wmap_special_travel_turn();
}

/** @brief Special travel step 10: turn to the departure heading (sets actor 0 unknown_02 to -1 there). */
void wmap_special_travel_turn(void)
{
    s32 remaining_ticks;

    wmap_turn_vehicle(1);
    if (D_801AFBD0.heading == WMAP_HEADING_DEPARTURE)
    {
        D_800D9268[0].unknown_02 = -1;
        g_wmap_special_travel_timer = 0;
        g_wmap_special_travel_step += 1;
    }
    remaining_ticks = g_wmap_special_travel_timer - 1;
    g_wmap_special_travel_timer = remaining_ticks;
    if (remaining_ticks == 0)
    {
        g_wmap_special_travel_step += 1;
    }
}

/** @brief Special travel step 11: move the party to the special land and fly the vehicle there. */
void wmap_special_travel_fly_to_land(void)
{
    wmap_find_land_cell(WMAP_SPECIAL_TRAVEL_LAND, &g_wmap_vehicle_target_x, &g_wmap_vehicle_target_y);
    wmap_set_traveler_position(0, g_wmap_vehicle_target_x, g_wmap_vehicle_target_y);
    g_wmap_vehicle_flying = 1;
    wmap_install_callback(wmap_start_vehicle_flight);
    g_wmap_special_travel_step += 1;
    wmap_special_travel_wait_land();
}

/** @brief Special travel step 12: wait for the flight to the special land. */
void wmap_special_travel_wait_land(void)
{
    if (g_wmap_vehicle_flying == 0)
    {
        g_wmap_special_travel_step += 1;
        wmap_special_travel_start_cruise();
    }
}

/** @brief Special travel step 13: start circling over the special land. */
void wmap_special_travel_start_cruise(void)
{
    g_wmap_special_travel_timer = WMAP_VEHICLE_WAIT_FRAMES;
    g_wmap_special_travel_step += 1;
    wmap_special_travel_cruise();
}

/** @brief Special travel step 14: circle over the land (clears actor 0 unknown_02 at the cruise heading). */
void wmap_special_travel_cruise(void)
{
    wmap_turn_vehicle(1);
    if (D_801AFBD0.heading == WMAP_HEADING_CRUISE)
    {
        D_800D9268[0].unknown_02 = 0;
    }
    if (--g_wmap_special_travel_timer == 0)
    {
        g_wmap_special_travel_step += 1;
    }
}

/** @brief Special travel step 15: turn to the landing heading. */
void wmap_special_travel_start_landing(void)
{
    g_wmap_vehicle_phase = 1;
    wmap_start_sequence(wmap_finish_vehicle_turn);
    g_wmap_special_travel_step += 1;
    wmap_special_travel_wait_landing();
}

/** @brief Special travel step 16: wait for the landing turn. */
void wmap_special_travel_wait_landing(void)
{
    if (g_wmap_vehicle_phase == 0)
    {
        g_wmap_special_travel_step += 1;
        wmap_special_travel_stop_vehicle();
    }
}

/** @brief Special travel step 17: stop drawing the vehicle on its orbit. */
void wmap_special_travel_stop_vehicle(void)
{
    D_801AFBD0.active = 0;
    g_wmap_special_travel_timer = WMAP_VEHICLE_WAIT_FRAMES;
    g_wmap_special_travel_step += 1;
    wmap_special_travel_fly_away();
}

/** @brief Special travel step 18: draw the vehicle flying off to the left. */
void wmap_special_travel_fly_away(void)
{
    WmapSpriteActor* actor = &g_wmap_vehicle_actor;

    wmap_turn_vehicle(0);
    wmap_step_actor_animation(actor, &g_wmap_vehicle_animation);
    wmap_draw_actor_sprite(actor, g_wmap_vehicle_screen_position.packed, WMAP_VEHICLE_TEXTURE, D_801AFBD0.ot_index, WMAP_VEHICLE_VARIANT);
    g_wmap_vehicle_screen_position.point.x -= 6;
    if (--g_wmap_special_travel_timer == 0)
    {
        g_wmap_special_travel_step += 1;
    }
}

/** @brief Special travel step 19: flag the arrival. */
void wmap_special_travel_finish(void)
{
    D_8013B294 = 1;
    g_wmap_special_travel_step += 1;
}

/**
 * @brief Run the current step of the special return sequence (flight back from the special land).
 * @param initialize Nonzero restarts the sequence instead of running a step.
 * @return 1 while the sequence runs, 0 once every step has run.
 */
s32 wmap_run_special_return(s32 initialize)
{
    s32 result;

    if (initialize != 0)
    {
        g_wmap_special_return_step = 1;
        g_wmap_special_return_timer = 1;
        return 1;
    }

    if (g_wmap_special_return_step < WMAP_SPECIAL_RETURN_STEPS)
    {
        g_wmap_special_return_steps[g_wmap_special_return_step]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/** @brief Special return step 0: restart the sequence. */
void wmap_special_return_reset(void)
{
    g_wmap_special_return_step = 1;
    g_wmap_special_return_timer = 1;
}

/** @brief Special return step 2: wait for the map scroll, then spawn the vehicle. */
void wmap_special_return_wait_scroll(void)
{
    if (g_wmap_view_scroll_mode != 2)
    {
        g_wmap_special_return_step += 1;
        wmap_special_return_spawn_vehicle();
    }
}

/** @brief Special return step 5: move the party to the return land and fly the vehicle there. */
void wmap_special_return_fly_home(void)
{
    wmap_find_land_cell(WMAP_RETURN_LAND, &g_wmap_vehicle_target_x, &g_wmap_vehicle_target_y);
    wmap_set_traveler_position(0, g_wmap_vehicle_target_x, g_wmap_vehicle_target_y);
    g_wmap_vehicle_flying = 1;
    wmap_install_callback(wmap_start_vehicle_flight);
    g_wmap_special_return_step += 1;
    wmap_special_return_wait_home();
}

/** @brief Special return step 6: wait for the flight to the return land. */
void wmap_special_return_wait_home(void)
{
    if (g_wmap_vehicle_flying == 0)
    {
        g_wmap_special_return_step += 1;
        wmap_special_return_start_cruise();
    }
}

/** @brief Special return step 7: start circling over the return land. */
void wmap_special_return_start_cruise(void)
{
    g_wmap_special_return_timer = WMAP_VEHICLE_WAIT_FRAMES;
    g_wmap_special_return_step += 1;
    wmap_special_return_cruise();
}

/** @brief Special return step 8: circle over the land (clears actor 0 unknown_02 at the cruise heading). */
void wmap_special_return_cruise(void)
{
    wmap_turn_vehicle(1);
    if (D_801AFBD0.heading == WMAP_HEADING_CRUISE)
    {
        D_800D9268[0].unknown_02 = 0;
    }
    if (--g_wmap_special_return_timer == 0)
    {
        g_wmap_special_return_step += 1;
    }
}

/** @brief Special return step 9: turn to the landing heading. */
void wmap_special_return_start_landing(void)
{
    g_wmap_vehicle_phase = 1;
    wmap_start_sequence(wmap_finish_vehicle_turn);
    g_wmap_special_return_step += 1;
    wmap_special_return_wait_landing();
}

/** @brief Special return step 10: wait for the landing turn. */
void wmap_special_return_wait_landing(void)
{
    if (g_wmap_vehicle_phase == 0)
    {
        g_wmap_special_return_step += 1;
        wmap_special_return_stop_vehicle();
    }
}

/** @brief Special return step 11: stop drawing the vehicle on its orbit. */
void wmap_special_return_stop_vehicle(void)
{
    D_801AFBD0.active = 0;
    g_wmap_special_return_timer = WMAP_VEHICLE_WAIT_FRAMES;
    g_wmap_special_return_step += 1;
    wmap_special_return_fly_away();
}

/** @brief Special return step 12: draw the vehicle flying off to the left. */
void wmap_special_return_fly_away(void)
{
    WmapSpriteActor* actor = &g_wmap_vehicle_actor;

    wmap_turn_vehicle(0);
    wmap_step_actor_animation(actor, &g_wmap_vehicle_animation);
    wmap_draw_actor_sprite(actor, g_wmap_vehicle_screen_position.packed, WMAP_VEHICLE_TEXTURE, D_801AFBD0.ot_index, WMAP_VEHICLE_VARIANT);
    g_wmap_vehicle_screen_position.point.x -= 4;
    if (--g_wmap_special_return_timer == 0)
    {
        g_wmap_special_return_step += 1;
    }
}

/** @brief Special return step 13: flag the arrival. */
void wmap_special_return_finish(void)
{
    D_8013B294 = 1;
    g_wmap_special_return_step += 1;
}
