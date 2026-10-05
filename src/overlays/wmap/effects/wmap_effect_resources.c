#include "../internal/wmap_main.h"
#include "../internal/wmap_resource_support.h"
#include "../internal/wmap_effect_resources.h"
#include "../internal/wmap_effect_resource_ids.h"
#include "main/cdrom.h"

/**
 * @file wmap_effect_resources.c
 * @brief Queue per-effect CD reads into shared texture, animation and model storage.
 * @note Model buffer aliases are offsets inside g_wmap_load_buffer, not separate
 * allocations. Embedded bundle slots are shared or asset-specific views of it.
 */

extern u8 g_wmap_load_buffer[];
/** @brief Effect workspace view at byte offset 0x89C. */
extern u8 g_wmap_mnt_bundle_model_pack_1[];
/** @brief Effect workspace view at byte offset 0xF3C. */
extern u8 g_wmap_mnt_bundle_model_pack_2[];
/** @brief Effect workspace view at byte offset 0xFDC. */
extern u8 g_wmap_shared_bundle_model_pack_1[];
/** @brief Effect workspace view at byte offset 0x1000. */
extern u8 g_wmap_effect_model_buffer_1000[];
/** @brief Effect workspace view at byte offset 0x1484. */
extern u8 g_wmap_mnt_bundle_model_pack_3[];
/** @brief Effect workspace view at byte offset 0x1B70. */
extern u8 g_wmap_fig_bundle_model_pack_2[];
/** @brief Effect workspace view at byte offset 0x1FB8. */
extern u8 g_wmap_shared_bundle_model_pack_2[];
/** @brief Effect workspace view at byte offset 0x2000. */
extern u8 g_wmap_effect_model_buffer_2000[];
/** @brief Effect workspace view at byte offset 0x2460. */
extern u8 g_wmap_mnt_bundle_model_pack_4[];
/** @brief Effect workspace view at byte offset 0x2B4C. */
extern u8 g_wmap_fig_bundle_model_pack_3[];
/** @brief Effect workspace view at byte offset 0x2F94. */
extern u8 g_wmap_jul_bundle_model_pack_3[];
/** @brief Effect workspace view at byte offset 0x31FC. */
extern u8 g_wmap_fig_bundle_model_pack_4[];
/** @brief Effect workspace view at byte offset 0x343C. */
extern u8 g_wmap_mnt_bundle_empty_slot[];
/** @brief Effect workspace view at byte offset 0x3808. */
extern u8 g_wmap_jul_bundle_model_pack_4[];
/** @brief Effect workspace view at byte offset 0x39CC. */
extern u8 g_wmap_fig_bundle_model_pack_5[];
/** @brief Effect workspace view at byte offset 0x3F9C. */
extern u8 g_wmap_fig_bundle_empty_slot[];
/** @brief Effect workspace view at byte offset 0x4000. */
extern u8 g_wmap_effect_model_buffer_4000[];
/** @brief Effect workspace view at byte offset 0x407C. */
extern u8 g_wmap_jul_bundle_model_pack_5[];
/** @brief Effect workspace view at byte offset 0x5000. */
extern u8 g_wmap_effect_model_buffer_5000[];
/** @brief Effect workspace view at byte offset 0x5C04. */
extern u8 g_wmap_mgc_bundle_model_pack_3[];
/** @brief Effect workspace view at byte offset 0x5EC0. */
extern u8 g_wmap_mgc_bundle_model_pack_4[];
/** @brief Effect workspace view at byte offset 0x6494. */
extern u8 g_wmap_mgc_bundle_model_pack_5[];
/** @brief Effect workspace view at byte offset 0x6A68. */
extern u8 g_wmap_mgc_bundle_empty_slot[];
/** @brief Effect workspace view at byte offset 0x8000. */
extern u8 g_wmap_effect_model_buffer_8000[];
/** @brief Effect workspace view at byte offset 0xB04C. */
extern u8 g_wmap_man_bundle_model_pack_1[];
/** @brief Effect workspace view at byte offset 0xC2A8. */
extern u8 g_wmap_jul_bundle_model_pack_6[];
/** @brief Effect workspace view at byte offset 0xD648. */
extern u8 g_wmap_man_bundle_model_pack_2[];
/** @brief Effect workspace view at byte offset 0xDB3C. */
extern u8 g_wmap_man_bundle_model_pack_3[];
/** @brief Effect workspace view at byte offset 0xEFC0. */
extern u8 g_wmap_man_bundle_model_pack_4[];
/** @brief Effect workspace view at byte offset 0xFF9C. */
extern u8 g_wmap_man_bundle_model_pack_5[];
/** @brief Effect workspace view at byte offset 0x10000. */
extern u8 g_wmap_effect_model_buffer_10000[];
/** @brief Effect workspace view at byte offset 0x103F8. */
extern u8 g_wmap_man_bundle_model_pack_6[];
/** @brief Effect workspace view at byte offset 0x113D4. */
extern u8 g_wmap_man_bundle_model_pack_7[];
/** @brief Effect workspace view at byte offset 0x15320. */
extern u8 g_wmap_man_bundle_model_pack_8[];
/** @brief Effect workspace view at byte offset 0x162FC. */
extern u8 g_wmap_man_bundle_model_pack_9[];
/** @brief Effect workspace view at byte offset 0x172F4. */
extern u8 g_wmap_jul_bundle_empty_slot[];
/** @brief Effect workspace view at byte offset 0x1DF48. */
extern u8 g_wmap_man_bundle_empty_slot[];
extern u8 g_wmap_animation_bank_0[];
extern u8 g_wmap_animation_bank_1[];
extern u8 g_wmap_animation_bank_2[];
extern u8 g_wmap_animation_bank_3[];
extern u8 g_wmap_animation_bank_4[];
extern u8 g_wmap_animation_bank_5[];
extern u8 g_wmap_effect_texture_buffer_0[];
extern u8 g_wmap_effect_texture_buffer_1[];
extern u8 g_wmap_effect_texture_buffer_2[];
extern u8* g_wmap_effect_model_pack_1;
extern u8* g_wmap_effect_model_pack_2;
extern u8* g_wmap_effect_model_pack_3;
extern u8* g_wmap_effect_model_pack_4;
extern u8* g_wmap_effect_model_pack_5;
extern u8* g_wmap_effect_model_pack_6;
extern u8* g_wmap_effect_model_pack_7;
extern u8* g_wmap_effect_model_pack_8;
extern u8* g_wmap_effect_model_pack_9;
/** @brief Pointer to the zero-filled trailing slot in MANBTP.DAT; no known reader. */
extern u8* g_wmap_effect_empty_model_slot;
extern void (*g_wmap_effect_resource_loaders[WMAP_EFFECT_RESOURCE_SET_COUNT])(void);

static void wmap_queue_effect_texture_2(s32 resource_index);

/**
 * @brief Wait for pending reads and dispatch a resource set when its index changes.
 * @param effect_index Requested effect index; invalid indices queue the shared set.
 * @note The requested index is cached before fallback. Reads queued here finish
 * at the next queue wait; selecting the cached index still waits for prior reads.
 */
void wmap_queue_effect_resources(s32 effect_index)
{
    cdrom_wait_queue_empty();
    if (effect_index == g_wmap_last_effect_resource_set)
    {
        return;
    }
    g_wmap_last_effect_resource_set = effect_index;
    if (effect_index >= WMAP_EFFECT_RESOURCE_SET_COUNT || effect_index < 0)
    {
        effect_index = WMAP_EFFECT_SHARED_RESOURCE_SET;
        g_wmap_exit_frame = 1;
    }
    g_wmap_effect_resource_loaders[effect_index]();
}

/**
 * @brief Queue a TIM read into effect texture buffer 0.
 * @param resource_index CD resource index; only its low 16 bits are used.
 */
void wmap_queue_effect_texture_0(s32 resource_index)
{
    cdrom_queue_read((u16)resource_index, g_wmap_effect_texture_buffer_0);
}

/**
 * @brief Queue a TIM read into effect texture buffer 1.
 * @param resource_index CD resource index; only its low 16 bits are used.
 */
void wmap_queue_effect_texture_1(s32 resource_index)
{
    cdrom_queue_read((u16)resource_index, g_wmap_effect_texture_buffer_1);
}

/**
 * @brief Queue a TIM read into effect texture buffer 2.
 * @param resource_index CD resource index; only its low 16 bits are used.
 */
static void wmap_queue_effect_texture_2(s32 resource_index)
{
    cdrom_queue_read((u16)resource_index, g_wmap_effect_texture_buffer_2);
}

/** @brief Queue the shared resource set for effects 0, 6, 14, 20, 28, 29. */
void wmap_queue_shared_land_effect_resources(void)
{
    /* Shared entries use the MHM resource set, including its GAT_FRA model. */
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_5000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x2000;
    wmap_queue_effect_texture_0(WMAP_EFFECT_SHARED_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_SHARED_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_SHARED_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_SHARED_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_SHARED_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_SHARED_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_SHARED_ANIMATION_3_RESOURCE, g_wmap_animation_bank_3);
    cdrom_queue_read(WMAP_EFFECT_SHARED_MODEL_2_RESOURCE, g_wmap_effect_model_pack_2);
    cdrom_queue_read(WMAP_EFFECT_SHARED_MODEL_1_RESOURCE, g_wmap_effect_model_pack_1);
    cdrom_queue_read(WMAP_EFFECT_SHARED_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
}

/** @brief Queue texture, animation and model resources for effect 4. */
void wmap_queue_effect_04_resources(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_2000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x2000;
    wmap_queue_effect_texture_0(WMAP_EFFECT_04_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_04_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_04_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_04_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_04_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_04_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_04_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
    cdrom_queue_read(WMAP_EFFECT_04_MODEL_1_RESOURCE, g_wmap_effect_model_pack_1);
    cdrom_queue_read(WMAP_EFFECT_04_MODEL_2_RESOURCE, g_wmap_effect_model_pack_2);
}

/** @brief Queue texture, animation and model resources for effect 18. */
void wmap_queue_effect_18_resources(void)
{
    func_80064F64(WMAP_EFFECT_18_PALETTE_RESOURCE);
    wmap_queue_effect_texture_0(WMAP_EFFECT_18_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_18_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_18_TEXTURE_2_RESOURCE);
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_8000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0xC000;
    cdrom_queue_read(WMAP_EFFECT_18_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_18_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_18_MODEL_1_RESOURCE, g_wmap_effect_model_pack_1);
    cdrom_queue_read(WMAP_EFFECT_18_MODEL_2_RESOURCE, g_wmap_effect_model_pack_2);
    cdrom_queue_read(WMAP_EFFECT_18_MODEL_2_AT_4000_RESOURCE, g_wmap_effect_model_pack_2 + 0x4000);
    cdrom_queue_read(WMAP_EFFECT_18_MODEL_2_AT_5000_RESOURCE, g_wmap_effect_model_pack_2 + 0x5000);
    cdrom_queue_read(WMAP_EFFECT_18_MODEL_2_AT_6000_RESOURCE, g_wmap_effect_model_pack_2 + 0x6000);
}

/** @brief Queue texture, animation and model resources for effect 1. */
void wmap_queue_effect_01_resources(void)
{
    wmap_queue_effect_texture_0(WMAP_EFFECT_01_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_01_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_01_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_01_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_01_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_01_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_01_ANIMATION_3_RESOURCE, g_wmap_animation_bank_3);
    cdrom_queue_read(WMAP_EFFECT_01_ANIMATION_4_RESOURCE, g_wmap_animation_bank_4);
    cdrom_queue_read(WMAP_EFFECT_01_ANIMATION_5_RESOURCE, g_wmap_animation_bank_5);
    cdrom_queue_read(WMAP_EFFECT_01_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
}

/** @brief Queue texture, animation and model resources for effect 13. */
void wmap_queue_effect_13_resources(void)
{
    wmap_queue_effect_texture_0(WMAP_EFFECT_13_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_13_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_13_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_13_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_13_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_13_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_13_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
}

/** @brief Queue texture, animation and model resources for effect 26. */
void wmap_queue_effect_26_resources(void)
{
    wmap_queue_effect_texture_0(WMAP_EFFECT_26_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_26_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_26_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_26_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_26_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_26_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_26_MODEL_BUNDLE_RESOURCE, g_wmap_load_buffer);
    g_wmap_effect_model_pack_1 = g_wmap_shared_bundle_model_pack_1;
    g_wmap_effect_model_pack_2 = g_wmap_fig_bundle_model_pack_2;
    g_wmap_effect_model_pack_3 = g_wmap_fig_bundle_model_pack_3;
    g_wmap_effect_model_pack_4 = g_wmap_fig_bundle_model_pack_4;
    g_wmap_effect_model_pack_5 = g_wmap_fig_bundle_model_pack_5;
    /* This bundle also exposes a zero-filled trailing slot. */
    g_wmap_effect_model_pack_6 = g_wmap_fig_bundle_empty_slot;
}

/** @brief Queue texture, animation and model resources for effect 32. */
void wmap_queue_effect_32_resources(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_2000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x2000;
    g_wmap_effect_model_pack_3 = g_wmap_effect_model_pack_2 + 0x2000;
    g_wmap_effect_model_pack_4 = g_wmap_effect_model_pack_3 + 0x2000;
    g_wmap_effect_model_pack_5 = g_wmap_effect_model_pack_4 + 0x2000;
    g_wmap_effect_model_pack_6 = g_wmap_effect_model_pack_5 + 0x2000;
    wmap_queue_effect_texture_0(WMAP_EFFECT_32_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_32_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_32_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_32_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_32_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_32_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
    cdrom_queue_read(WMAP_EFFECT_32_MODEL_1_RESOURCE, g_wmap_effect_model_pack_1);
    cdrom_queue_read(WMAP_EFFECT_32_MODEL_3_RESOURCE, g_wmap_effect_model_pack_3);
    cdrom_queue_read(WMAP_EFFECT_32_MODEL_4_RESOURCE, g_wmap_effect_model_pack_4);
    cdrom_queue_read(WMAP_EFFECT_32_MODEL_5_RESOURCE, g_wmap_effect_model_pack_5);
}

/** @brief Queue texture, animation and model resources for effect 15. */
void wmap_queue_effect_15_resources(void)
{
    wmap_queue_effect_texture_0(WMAP_EFFECT_15_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_15_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_15_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_15_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_15_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_15_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    g_wmap_effect_model_pack_1 = g_wmap_mnt_bundle_model_pack_1;
    g_wmap_effect_model_pack_2 = g_wmap_mnt_bundle_model_pack_2;
    g_wmap_effect_model_pack_3 = g_wmap_mnt_bundle_model_pack_3;
    g_wmap_effect_model_pack_4 = g_wmap_mnt_bundle_model_pack_4;
    /* This bundle also exposes a zero-filled trailing slot. */
    g_wmap_effect_model_pack_5 = g_wmap_mnt_bundle_empty_slot;
    cdrom_queue_read(WMAP_EFFECT_15_MODEL_BUNDLE_RESOURCE, g_wmap_load_buffer);
}

/** @brief Queue texture, animation and model resources for effect 2. */
void wmap_queue_effect_02_resources(void)
{
    wmap_queue_effect_texture_0(WMAP_EFFECT_02_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_02_TEXTURE_1_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_02_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_02_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_02_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
}

/** @brief Queue texture, animation and model resources for effect 7. */
void wmap_queue_effect_07_resources(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_10000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x2000;
    wmap_queue_effect_texture_0(WMAP_EFFECT_07_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_07_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_07_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_07_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_07_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_07_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_07_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
    cdrom_queue_read(WMAP_EFFECT_07_MODEL_2_RESOURCE, g_wmap_effect_model_pack_2);
    cdrom_queue_read(WMAP_EFFECT_07_MODEL_1_RESOURCE, g_wmap_effect_model_pack_1);
}

/** @brief Queue texture, animation and model resources for effect 30. */
void wmap_queue_effect_30_resources(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_10000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x2000;
    wmap_queue_effect_texture_0(WMAP_EFFECT_30_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_30_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_30_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_30_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_30_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_30_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_30_ANIMATION_3_RESOURCE, g_wmap_animation_bank_3);
    cdrom_queue_read(WMAP_EFFECT_30_ANIMATION_4_RESOURCE, g_wmap_animation_bank_4);
    cdrom_queue_read(WMAP_EFFECT_30_ANIMATION_5_RESOURCE, g_wmap_animation_bank_5);
    cdrom_queue_read(WMAP_EFFECT_30_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
    cdrom_queue_read(WMAP_EFFECT_30_MODEL_1_RESOURCE, g_wmap_effect_model_pack_1);
}

/** @brief Queue texture, animation and model resources for effect 12. */
void wmap_queue_effect_12_resources(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_10000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x2000;
    wmap_queue_effect_texture_0(WMAP_EFFECT_12_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_12_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_12_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_12_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
    cdrom_queue_read(WMAP_EFFECT_12_MODEL_1_RESOURCE, g_wmap_effect_model_pack_1);
    cdrom_queue_read(WMAP_EFFECT_12_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_12_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_12_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
}

/** @brief Queue texture, animation and model resources for effect 11. */
void wmap_queue_effect_11_resources(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_10000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x2000;
    g_wmap_effect_model_pack_3 = g_wmap_effect_model_pack_2 + 0x2000;
    wmap_queue_effect_texture_0(WMAP_EFFECT_11_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_11_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_11_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_11_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_11_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_11_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
    cdrom_queue_read(WMAP_EFFECT_11_MODEL_1_RESOURCE, g_wmap_effect_model_pack_1);
    cdrom_queue_read(WMAP_EFFECT_11_MODEL_2_RESOURCE, g_wmap_effect_model_pack_2);
    cdrom_queue_read(WMAP_EFFECT_11_MODEL_3_RESOURCE, g_wmap_effect_model_pack_3);
}

/** @brief Queue texture, animation and model resources for effect 3. */
void wmap_queue_effect_03_resources(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_10000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x2000;
    wmap_queue_effect_texture_0(WMAP_EFFECT_03_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_03_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_03_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_03_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
    cdrom_queue_read(WMAP_EFFECT_03_MODEL_1_RESOURCE, g_wmap_effect_model_pack_1);
    cdrom_queue_read(WMAP_EFFECT_03_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_03_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_03_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
}

/** @brief Queue texture, animation and model resources for effect 21. */
void wmap_queue_effect_21_resources(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_2000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x2000;
    wmap_queue_effect_texture_0(WMAP_EFFECT_21_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_21_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_21_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_21_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_21_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_21_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_21_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
    cdrom_queue_read(WMAP_EFFECT_21_MODEL_1_RESOURCE, g_wmap_effect_model_pack_1);
}

/** @brief Queue texture, animation and model resources for effect 17. */
void wmap_queue_effect_17_resources(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_2000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x2000;
    wmap_queue_effect_texture_0(WMAP_EFFECT_17_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_17_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_17_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_17_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_17_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_17_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_17_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
    cdrom_queue_read(WMAP_EFFECT_17_MODEL_1_RESOURCE, g_wmap_effect_model_pack_1);
}

/** @brief Queue texture, animation and model resources for effect 5. */
void wmap_queue_effect_05_resources(void)
{
    wmap_queue_effect_texture_0(WMAP_EFFECT_05_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_05_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_05_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_05_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_05_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_05_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    g_wmap_effect_model_pack_1 = g_wmap_shared_bundle_model_pack_1;
    g_wmap_effect_model_pack_2 = g_wmap_shared_bundle_model_pack_2;
    g_wmap_effect_model_pack_3 = g_wmap_mgc_bundle_model_pack_3;
    g_wmap_effect_model_pack_4 = g_wmap_mgc_bundle_model_pack_4;
    g_wmap_effect_model_pack_5 = g_wmap_mgc_bundle_model_pack_5;
    /* This bundle also exposes a zero-filled trailing slot. */
    g_wmap_effect_model_pack_6 = g_wmap_mgc_bundle_empty_slot;
    cdrom_queue_read(WMAP_EFFECT_05_MODEL_BUNDLE_RESOURCE, g_wmap_load_buffer);
}

/** @brief Queue texture, animation and model resources for effect 10. */
void wmap_queue_effect_10_resources(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_4000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x4000;
    g_wmap_effect_model_pack_3 = g_wmap_effect_model_pack_2 + 0xC800;
    wmap_queue_effect_texture_0(WMAP_EFFECT_10_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_10_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_10_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_10_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_10_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_10_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_10_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
    cdrom_queue_read(WMAP_EFFECT_10_MODEL_1_RESOURCE, g_wmap_effect_model_pack_1);
    cdrom_queue_read(WMAP_EFFECT_10_MODEL_2_RESOURCE, g_wmap_effect_model_pack_2);
}

/** @brief Queue texture, animation and model resources for effect 8. */
void wmap_queue_effect_08_resources(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_2000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x2000;
    wmap_queue_effect_texture_0(WMAP_EFFECT_08_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_08_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_08_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_08_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_08_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_08_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_08_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
    cdrom_queue_read(WMAP_EFFECT_08_MODEL_1_RESOURCE, g_wmap_effect_model_pack_1);
    cdrom_queue_read(WMAP_EFFECT_08_MODEL_2_RESOURCE, g_wmap_effect_model_pack_2);
}

/** @brief Queue texture, animation and model resources for effect 9. */
void wmap_queue_effect_09_resources(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_2000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x2000;
    wmap_queue_effect_texture_0(WMAP_EFFECT_09_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_09_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_09_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_09_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_09_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_09_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_09_ANIMATION_3_RESOURCE, g_wmap_animation_bank_3);
    cdrom_queue_read(WMAP_EFFECT_09_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
    cdrom_queue_read(WMAP_EFFECT_09_MODEL_1_RESOURCE, g_wmap_effect_model_pack_1);
}

/** @brief Queue texture, animation and model resources for effect 16. */
void wmap_queue_effect_16_resources(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_2000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x2000;
    wmap_queue_effect_texture_0(WMAP_EFFECT_16_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_16_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_16_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_16_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_16_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_16_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_16_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
    cdrom_queue_read(WMAP_EFFECT_16_MODEL_1_RESOURCE, g_wmap_effect_model_pack_1);
    cdrom_queue_read(WMAP_EFFECT_16_MODEL_2_RESOURCE, g_wmap_effect_model_pack_2);
}

/** @brief Queue texture, animation and model resources for effect 27. */
void wmap_queue_effect_27_resources(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_4000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x4000;
    g_wmap_effect_model_pack_3 = g_wmap_effect_model_pack_2 + 0x4000;
    g_wmap_effect_model_pack_4 = g_wmap_effect_model_pack_3 + 0x4000;
    wmap_queue_effect_texture_0(WMAP_EFFECT_27_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_27_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_27_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_27_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_27_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_27_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_27_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
    cdrom_queue_read(WMAP_EFFECT_27_MODEL_1_RESOURCE, g_wmap_effect_model_pack_1);
    cdrom_queue_read(WMAP_EFFECT_27_MODEL_2_RESOURCE, g_wmap_effect_model_pack_2);
    cdrom_queue_read(WMAP_EFFECT_27_MODEL_3_RESOURCE, g_wmap_effect_model_pack_3);
    cdrom_queue_read(WMAP_EFFECT_27_MODEL_4_RESOURCE, g_wmap_effect_model_pack_4);
}

/** @brief Queue texture, animation and model resources for effect 23. */
void wmap_queue_effect_23_resources(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_2000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x2000;
    g_wmap_effect_model_pack_3 = g_wmap_effect_model_pack_2 + 0x2000;
    g_wmap_effect_model_pack_4 = g_wmap_effect_model_pack_3 + 0x2000;
    wmap_queue_effect_texture_0(WMAP_EFFECT_23_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_23_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_23_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_23_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
    cdrom_queue_read(WMAP_EFFECT_23_MODEL_1_RESOURCE, g_wmap_effect_model_pack_1);
    cdrom_queue_read(WMAP_EFFECT_23_MODEL_2_RESOURCE, g_wmap_effect_model_pack_2);
    cdrom_queue_read(WMAP_EFFECT_23_MODEL_3_RESOURCE, g_wmap_effect_model_pack_3);
    cdrom_queue_read(WMAP_EFFECT_23_MODEL_4_RESOURCE, g_wmap_effect_model_pack_4);
    cdrom_queue_read(WMAP_EFFECT_23_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_23_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_23_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_23_ANIMATION_3_RESOURCE, g_wmap_animation_bank_3);
    cdrom_queue_read(WMAP_EFFECT_23_ANIMATION_4_RESOURCE, g_wmap_animation_bank_4);
}

/** @brief Queue texture, animation and model resources for effect 19. */
void wmap_queue_effect_19_resources(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_2000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x2000;
    g_wmap_effect_model_pack_3 = g_wmap_effect_model_pack_2 + 0x2000;
    wmap_queue_effect_texture_0(WMAP_EFFECT_19_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_19_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_19_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_19_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_19_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_19_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_19_ANIMATION_3_RESOURCE, g_wmap_animation_bank_3);
    cdrom_queue_read(WMAP_EFFECT_19_ANIMATION_4_RESOURCE, g_wmap_animation_bank_4);
    cdrom_queue_read(WMAP_EFFECT_19_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
    cdrom_queue_read(WMAP_EFFECT_19_MODEL_1_RESOURCE, g_wmap_effect_model_pack_1);
    cdrom_queue_read(WMAP_EFFECT_19_MODEL_2_RESOURCE, g_wmap_effect_model_pack_2);
    cdrom_queue_read(WMAP_EFFECT_19_MODEL_3_RESOURCE, g_wmap_effect_model_pack_3);
}

/** @brief Queue texture, animation and model resources for effect 22. */
void wmap_queue_effect_22_resources(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_2000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x2000;
    g_wmap_effect_model_pack_3 = g_wmap_effect_model_pack_2 + 0x4000;
    g_wmap_effect_model_pack_4 = g_wmap_effect_model_pack_3 + 0x4000;
    wmap_queue_effect_texture_0(WMAP_EFFECT_22_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_22_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_22_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_22_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_22_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_22_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_22_ANIMATION_3_RESOURCE, g_wmap_animation_bank_3);
    cdrom_queue_read(WMAP_EFFECT_22_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
    cdrom_queue_read(WMAP_EFFECT_22_MODEL_1_RESOURCE, g_wmap_effect_model_pack_1);
    cdrom_queue_read(WMAP_EFFECT_22_MODEL_2_RESOURCE, g_wmap_effect_model_pack_2);
    cdrom_queue_read(WMAP_EFFECT_22_MODEL_3_RESOURCE, g_wmap_effect_model_pack_3);
    cdrom_queue_read(WMAP_EFFECT_22_MODEL_4_RESOURCE, g_wmap_effect_model_pack_4);
}

/** @brief Queue texture, animation and model resources for effect 25. */
void wmap_queue_effect_25_resources(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_shared_bundle_model_pack_1;
    g_wmap_effect_model_pack_2 = g_wmap_shared_bundle_model_pack_2;
    g_wmap_effect_model_pack_3 = g_wmap_jul_bundle_model_pack_3;
    g_wmap_effect_model_pack_4 = g_wmap_jul_bundle_model_pack_4;
    g_wmap_effect_model_pack_5 = g_wmap_jul_bundle_model_pack_5;
    g_wmap_effect_model_pack_6 = g_wmap_jul_bundle_model_pack_6;
    /* This bundle also exposes a zero-filled trailing slot. */
    g_wmap_effect_model_pack_7 = g_wmap_jul_bundle_empty_slot;
    wmap_queue_effect_texture_0(WMAP_EFFECT_25_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_25_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_25_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_25_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_25_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_25_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_25_ANIMATION_3_RESOURCE, g_wmap_animation_bank_3);
    cdrom_queue_read(WMAP_EFFECT_25_MODEL_BUNDLE_RESOURCE, g_wmap_load_buffer);
}

/** @brief Queue texture, animation and model resources for effect 31. */
void wmap_queue_effect_31_resources(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_man_bundle_model_pack_1;
    g_wmap_effect_model_pack_2 = g_wmap_man_bundle_model_pack_2;
    g_wmap_effect_model_pack_3 = g_wmap_man_bundle_model_pack_3;
    g_wmap_effect_model_pack_4 = g_wmap_man_bundle_model_pack_4;
    g_wmap_effect_model_pack_5 = g_wmap_man_bundle_model_pack_5;
    g_wmap_effect_model_pack_6 = g_wmap_man_bundle_model_pack_6;
    g_wmap_effect_model_pack_7 = g_wmap_man_bundle_model_pack_7;
    g_wmap_effect_model_pack_8 = g_wmap_man_bundle_model_pack_8;
    g_wmap_effect_model_pack_9 = g_wmap_man_bundle_model_pack_9;
    /* This bundle also exposes a zero-filled trailing slot. */
    g_wmap_effect_empty_model_slot = g_wmap_man_bundle_empty_slot;
    wmap_queue_effect_texture_0(WMAP_EFFECT_31_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_31_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_31_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_31_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_31_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_31_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_31_MODEL_BUNDLE_RESOURCE, g_wmap_load_buffer);
}

/** @brief Queue texture, animation and model resources for effect 24. */
void wmap_queue_effect_24_resources(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_1000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x1000;
    g_wmap_effect_model_pack_3 = g_wmap_effect_model_pack_2 + 0x1000;
    g_wmap_effect_model_pack_4 = g_wmap_effect_model_pack_3 + 0x10800;
    wmap_queue_effect_texture_0(WMAP_EFFECT_24_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_24_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_24_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_24_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_24_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_24_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_24_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
    cdrom_queue_read(WMAP_EFFECT_24_MODEL_1_RESOURCE, g_wmap_effect_model_pack_1);
    cdrom_queue_read(WMAP_EFFECT_24_MODEL_2_RESOURCE, g_wmap_effect_model_pack_2);
    cdrom_queue_read(WMAP_EFFECT_24_MODEL_3_RESOURCE, g_wmap_effect_model_pack_3);
    cdrom_queue_read(WMAP_EFFECT_24_MODEL_4_RESOURCE, g_wmap_effect_model_pack_4);
}

/** @brief Queue texture, animation and model resources for effect 33. */
void wmap_queue_effect_33_resources(void)
{
    wmap_queue_effect_texture_0(WMAP_EFFECT_33_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_33_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_33_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_33_ANIMATION_4_RESOURCE, g_wmap_animation_bank_4);
    cdrom_queue_read(WMAP_EFFECT_33_ANIMATION_5_RESOURCE, g_wmap_animation_bank_5);
    cdrom_queue_read(WMAP_EFFECT_33_ANIMATION_3_RESOURCE, g_wmap_animation_bank_3);
    cdrom_queue_read(WMAP_EFFECT_33_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_33_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
}

/** @brief Queue texture, animation and model resources for effect 34. */
void wmap_queue_effect_34_resources(void)
{
    wmap_queue_effect_texture_0(WMAP_EFFECT_34_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_34_TEXTURE_1_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_34_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_34_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_34_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
}

/** @brief Queue texture, animation and model resources for effect 35. */
void wmap_queue_effect_35_resources(void)
{
    g_wmap_effect_model_pack_1 = g_wmap_effect_model_buffer_1000;
    g_wmap_effect_model_pack_2 = g_wmap_effect_model_pack_1 + 0x1000;
    g_wmap_effect_model_pack_3 = g_wmap_effect_model_pack_2 + 0x6000;
    wmap_queue_effect_texture_0(WMAP_EFFECT_35_TEXTURE_0_RESOURCE);
    wmap_queue_effect_texture_1(WMAP_EFFECT_35_TEXTURE_1_RESOURCE);
    wmap_queue_effect_texture_2(WMAP_EFFECT_35_TEXTURE_2_RESOURCE);
    cdrom_queue_read(WMAP_EFFECT_35_ANIMATION_0_RESOURCE, g_wmap_animation_bank_0);
    cdrom_queue_read(WMAP_EFFECT_35_ANIMATION_1_RESOURCE, g_wmap_animation_bank_1);
    cdrom_queue_read(WMAP_EFFECT_35_ANIMATION_2_RESOURCE, g_wmap_animation_bank_2);
    cdrom_queue_read(WMAP_EFFECT_35_BASE_MODEL_RESOURCE, g_wmap_load_buffer);
    cdrom_queue_read(WMAP_EFFECT_35_MODEL_1_RESOURCE, g_wmap_effect_model_pack_1);
    cdrom_queue_read(WMAP_EFFECT_35_MODEL_2_RESOURCE, g_wmap_effect_model_pack_2);
    cdrom_queue_read(WMAP_EFFECT_35_MODEL_3_RESOURCE, g_wmap_effect_model_pack_3);
}
