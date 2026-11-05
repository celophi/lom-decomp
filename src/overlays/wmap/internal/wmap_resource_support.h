#ifndef WMAP_RESOURCE_SUPPORT_H
#define WMAP_RESOURCE_SUPPORT_H

#include "common.h"
#include "main/audio/akao.h"

/** @brief One-based world-map sound numbers (wmap_play_sound; entry n - 1 of g_wmap_sfx_buffers). */
enum WmapSound
{
    WMAP_SOUND_ZOOM_OUT = 1,
    WMAP_SOUND_ZOOM_IN = 2,
    /** @brief Map cursor or menu selection moved. */
    WMAP_SOUND_CURSOR = 3,
    WMAP_SOUND_OPEN_ARTIFACTS = 4,
    WMAP_SOUND_TURN_RIGHT = 8,
    WMAP_SOUND_TURN_LEFT = 9,
    /** @brief Menu closed or a menu page left. */
    WMAP_SOUND_BACK = 11,
    WMAP_SOUND_OPEN_MENU = 17,
    /** @brief Played as the map's exit effect fades the song out. */
    WMAP_SOUND_EXIT = 20,
    WMAP_SOUND_PLACE_ARTIFACT = 22,
    /** @brief Menu page opened. */
    WMAP_SOUND_CONFIRM = 23,
    WMAP_SOUND_CLOSE_ARTIFACTS = 24
};

/** @brief Sound-effect buffer of each world-map sound; entry n - 1 plays sound n. */
extern AkaoHeader* g_wmap_sfx_buffers[];

void func_80064F14();
void wmap_copy_words(s32* source, s32* destination, s32 byte_count);
void func_80064F5C(void);
void wmap_load_tim_file(s32 resource_index);
void wmap_stream_tim_file(s32 resource_index);
void wmap_queue_load_buffer_read(s32 resource_index);
void wmap_upload_tim(u8* data);
void wmap_play_sound(s32 sound_index, s32 pan);
void wmap_clear_framebuffers(void);
void wmap_queue_texture_page(s32 texture_page, s32 depth);
void wmap_init_actor_display_states(void);
s32 wmap_get_controller_repeat_buttons(void);

/** @brief Load targets for effect animations, textures and models. */
extern u8 g_wmap_animation_bank_0[];
extern u8 g_wmap_animation_bank_1[];
extern u8 g_wmap_animation_bank_2[];
extern u8 g_wmap_animation_bank_3[];
extern u8 g_wmap_animation_bank_4[];
extern u8 g_wmap_animation_bank_5[];
extern u8 g_wmap_effect_texture_buffer_0[];
extern u8 g_wmap_effect_texture_buffer_1[];
extern u8 g_wmap_effect_texture_buffer_2[];
extern u8 g_wmap_effect_model_buffer_2000[];

/** @brief Model packs of the running effect, each pointing into the effect model buffers. */
extern u8* g_wmap_effect_model_pack_1;
extern u8* g_wmap_effect_model_pack_2;
extern u8* g_wmap_effect_model_pack_3;
extern u8* g_wmap_effect_model_pack_4;
extern u8* g_wmap_effect_model_pack_5;
extern u8* g_wmap_effect_model_pack_6;
extern u8* g_wmap_effect_model_pack_7;

#endif
