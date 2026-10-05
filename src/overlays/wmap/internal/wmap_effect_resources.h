#ifndef WMAP_EFFECT_RESOURCES_H
#define WMAP_EFFECT_RESOURCES_H

#include "common.h"

#define WMAP_EFFECT_RESOURCE_SET_COUNT 36
#define WMAP_EFFECT_SHARED_RESOURCE_SET 0

/**
 * @brief Wait for prior reads and queue a changed world-map effect resource set.
 * @param effect_index Requested effect index; invalid indices select the shared set.
 * @note Caches the requested index before fallback and does not wait for new reads.
 */
void wmap_queue_effect_resources(s32 effect_index);
/**
 * @brief Queue a TIM read into effect texture buffer 0.
 * @param resource_index CD resource index; only its low 16 bits are used.
 */
void wmap_queue_effect_texture_0(s32 resource_index);
/**
 * @brief Queue a TIM read into effect texture buffer 1.
 * @param resource_index CD resource index; only its low 16 bits are used.
 */
void wmap_queue_effect_texture_1(s32 resource_index);
/* Entry points in g_wmap_effect_resource_loaders; shared entries use one loader. */
void wmap_queue_shared_land_effect_resources(void);
void wmap_queue_effect_04_resources(void);
void wmap_queue_effect_18_resources(void);
void wmap_queue_effect_01_resources(void);
void wmap_queue_effect_13_resources(void);
void wmap_queue_effect_26_resources(void);
void wmap_queue_effect_32_resources(void);
void wmap_queue_effect_15_resources(void);
void wmap_queue_effect_02_resources(void);
void wmap_queue_effect_07_resources(void);
void wmap_queue_effect_30_resources(void);
void wmap_queue_effect_12_resources(void);
void wmap_queue_effect_11_resources(void);
void wmap_queue_effect_03_resources(void);
void wmap_queue_effect_21_resources(void);
void wmap_queue_effect_17_resources(void);
void wmap_queue_effect_05_resources(void);
void wmap_queue_effect_10_resources(void);
void wmap_queue_effect_08_resources(void);
void wmap_queue_effect_09_resources(void);
void wmap_queue_effect_16_resources(void);
void wmap_queue_effect_27_resources(void);
void wmap_queue_effect_23_resources(void);
void wmap_queue_effect_19_resources(void);
void wmap_queue_effect_22_resources(void);
void wmap_queue_effect_25_resources(void);
void wmap_queue_effect_31_resources(void);
void wmap_queue_effect_24_resources(void);
void wmap_queue_effect_33_resources(void);
void wmap_queue_effect_34_resources(void);
void wmap_queue_effect_35_resources(void);

#endif
