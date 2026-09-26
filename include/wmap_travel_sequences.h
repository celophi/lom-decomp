#ifndef WMAP_TRAVEL_SEQUENCES_H
#define WMAP_TRAVEL_SEQUENCES_H

#include "common.h"

/** @brief Sprite actor record read by the animator and the sprite renderer. */
typedef struct
{
    s16 unknown_00;
    s16 unknown_02;
    u8 unknown_04[2];
    s8 scale_index;
    u8 unknown_07[7];
    s16 sequence;
    s16 previous_sequence;
    u8 unknown_12[0x10];
    s16 target_shade;
    s16 shade;
    s16 shade_step;
    u8 unknown_28[4];
} WmapSpriteActor;

WmapSpriteActor* wmap_turn_vehicle(s32 advance);
s32 wmap_draw_vehicle(s32 initialize);
s32 wmap_finish_vehicle_turn(s32 initialize);
s32 wmap_run_special_travel(s32 initialize);
s32 wmap_run_special_return(s32 initialize);

#endif
