#include "common/card_menu.h"

/**
 * @brief Run one frame: tear down and exit if requested, else update and render.
 * @param render FIELD render buffer being built this frame.
 * @return 1 when the overlay has finished, otherwise 0.
 */
s32 card_menu_update_frame(FieldRenderHalf* render)
{
    if (g_card_menu_exit_requested != 0)
    {
        shutdown_card_events();
        field_text_reset_windows();
        DrawSync(0);
        return 1;
    }
    field_text_reset_scratch();
    begin_glyph_cache_frame();
    card_menu_update_state(render);
    evict_unused_glyphs();
    field_text_upload_immediate_cache();
    g_card_menu_frame_parity ^= 1;
    return 0;
}
