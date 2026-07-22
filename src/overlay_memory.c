#include "overlay_memory.h"

extern s32 g_overlay_load_base;
extern u32 g_field_render_buffers;
extern u32 g_world_map_overlay_end;
extern u32 g_title_menu_buffers;

/**
 * @brief Return the common base address used for loading overlays.
 * @return Overlay load base at 0x8004FC70.
 */
s32* get_overlay_load_base(void)
{
    return &g_overlay_load_base;
}

/**
 * @brief Return the render-buffer base immediately after the FIELD overlay image.
 * @return Base of the two field render buffers at 0x80123FD8.
 * @see decomp.me (100%) https://decomp.me/scratch/rgamP
 */
u32* get_field_render_buffers(void)
{
    return &g_field_render_buffers;
}

/**
 * @brief Return the address immediately after the decompressed WMAP overlay image.
 * @return WMAP overlay end at 0x801B32D8.
 * @see decomp.me (100%) https://decomp.me/scratch/B5ptQ
 */
u32* get_world_map_overlay_end(void)
{
    return &g_world_map_overlay_end;
}

/**
 * @brief Return the menu-buffer base immediately after the TITLE overlay image.
 * @return Base of the title menu buffers at 0x801026D0.
 * @see decomp.me (100%) https://decomp.me/scratch/fl1lB
 */
u32* get_title_menu_buffers(void)
{
    return &g_title_menu_buffers;
}
