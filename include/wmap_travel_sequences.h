#ifndef WMAP_TRAVEL_SEQUENCES_H
#define WMAP_TRAVEL_SEQUENCES_H

#include "common.h"
#include "wmap_sprite_render.h"

WmapSpriteActor* wmap_turn_vehicle(s32 advance);
s32 wmap_draw_vehicle(s32 initialize);
s32 wmap_finish_vehicle_turn(s32 initialize);
s32 wmap_run_special_travel(s32 initialize);
s32 wmap_run_special_return(s32 initialize);

#endif
