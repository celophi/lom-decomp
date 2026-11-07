#ifndef WMAP_TRAVEL_SEQUENCES_H
#define WMAP_TRAVEL_SEQUENCES_H

#include "common.h"

#include "wmap_sprite_render.h"

/** @brief Vehicle sprite: texture, draw variant, ordering-table depths and flap sound. */
#define WMAP_VEHICLE_TEXTURE 40
#define WMAP_VEHICLE_VARIANT 2
#define WMAP_VEHICLE_OT_FRONT 50
#define WMAP_VEHICLE_OT_BEHIND 61
#define WMAP_VEHICLE_OT_START 60
#define WMAP_VEHICLE_OT_FLIGHT 59
#define WMAP_VEHICLE_SOUND_FLAP 51
#define WMAP_VEHICLE_SOUND_PAN 127

/** @brief Vehicle orbit: start radius and height, their limits, and the climb step. */
#define WMAP_VEHICLE_START_RADIUS 200
#define WMAP_VEHICLE_START_HEIGHT 10
#define WMAP_VEHICLE_MAX_RADIUS 480
#define WMAP_VEHICLE_MAX_HEIGHT 100
#define WMAP_VEHICLE_RADIUS_STEP 4
#define WMAP_VEHICLE_LOW_HEIGHT 40

/** @brief CD files of the vehicle's two animation banks and its texture. */
#define WMAP_VEHICLE_BANK_FILE 0x1145
#define WMAP_VEHICLE_BANK_FILE_2 0x1146
#define WMAP_VEHICLE_TEXTURE_FILE 0x1147
#define WMAP_VEHICLE_PALETTE_FILE 0x1148

/** @brief Element 0 of the g_wmap_actor_motions motion table while it drives the travel vehicle. */
typedef struct
{
    s16 active;
    s16 heading;
    s32 ot_override;
    s32 radius;
    s16 unused_0c; /**< Motion-slot lifetime halfword; vehicle code never reads or writes it. */
    s16 height;
    s16 ot_index;
    s16 unused_12; /**< High halfword of the slot's last word; vehicle code never reads or writes it. */
} WmapVehicleMotion;

WmapSpriteActor* wmap_turn_vehicle(s32 advance);
s32 wmap_draw_vehicle(s32 initialize);
s32 wmap_finish_vehicle_turn(s32 initialize);
s32 wmap_run_special_travel(s32 initialize);
s32 wmap_run_special_return(s32 initialize);

#endif
