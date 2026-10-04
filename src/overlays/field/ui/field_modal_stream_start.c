/** @file field_modal_stream_start.c
 * @brief Load and enter the CARDA, GOLEM, NIKI and ADDHERO sub-overlays.
 */

#include "main/cd_resources.h"
#include "main/cdrom.h"
#include "common.h"
#include "../internal/field_calls.h"
#include "../internal/field_modal_runtime.h"

/** @brief Load address of the modal sub-overlays. */
#define FIELD_SUBOVERLAY_ADDRESS ((void*)SECONDARY_OVERLAY_ADDRESS)

/** @brief Render buffers handed to the GOLEM entry point. */
#define FIELD_GOLEM_RENDER_BUFFERS ((void*)SECONDARY_OVERLAY_AT(0x10000))

/** @brief Work buffer handed to the CARDA, NIKI and ADDHERO entry points. */
#define FIELD_MODAL_WORK_BUFFER ((void*)SECONDARY_OVERLAY_AT(0x30000))

extern s32 g_field_modal_state;
extern s32 g_field_card_overlay_mode;
extern s32 g_field_niki_addhero_state;
extern s32 g_field_card_pet_slot;
extern s32 g_field_gosub_state;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];

void carda_init(void* work, s32 mode);
void golem_run(void* render_buffers, s32 restore_slot_on_cancel);
void niki_addhero_init(void* work, s32 mode);

/**
 * @brief Load CARDA and start it, unless another modal is already running.
 * @param mode Card-screen mode; stored plus one and passed to the CARDA entry point.
 */
void field_open_carda(s32 mode)
{
    if (g_field_modal_state == FIELD_MODAL_NONE)
    {
        field_reset_actor_resources();
        cdrom_stream(CD_RES_CARDA_BIN, FIELD_SUBOVERLAY_ADDRESS);
        cdrom_wait_queue_empty();
        g_field_card_overlay_mode = mode + 1;
        g_field_gosub_state = 1;
        g_gosub_result_count = 0;
        g_field_modal_state = FIELD_MODAL_CARDA;
        g_field_card_pet_slot = g_gosub_result_values[0];
        carda_init(FIELD_MODAL_WORK_BUFFER, mode);
    }
}

/**
 * @brief Load GOLEM, run its editor to completion, then rebuild the current land grid.
 */
void field_run_golem(void)
{
    field_reset_actor_resources();
    cdrom_stream(CD_RES_GOLEM_BIN, FIELD_SUBOVERLAY_ADDRESS);
    cdrom_wait_queue_empty();
    golem_run(FIELD_GOLEM_RENDER_BUFFERS, 0);
    field_golem_rebuild_current_grid();
    field_reset_actor_resources();
}

/**
 * @brief Empty per-frame hook, called with the render half after field_update_modal.
 * @param render_half Render half being drawn (unused).
 */
void field_modal_frame_stub(FieldRenderHalf* render_half)
{
}

/**
 * @brief Load NIKI and start it, unless another modal is already running.
 * @param mode Mode passed to the NIKI entry point.
 */
void field_open_niki(s32 mode)
{
    if (g_field_modal_state == FIELD_MODAL_NONE)
    {
        field_reset_actor_resources();
        cdrom_stream(CD_RES_NIKI_BIN, FIELD_SUBOVERLAY_ADDRESS);
        cdrom_wait_queue_empty();
        g_field_niki_addhero_state = 1;
        g_field_modal_state = FIELD_MODAL_NIKI;
        niki_addhero_init(FIELD_MODAL_WORK_BUFFER, mode);
    }
}

/**
 * @brief Load ADDHERO and start it, unless another modal is already running.
 * @param mode Mode passed to the ADDHERO entry point.
 */
void field_open_addhero(s32 mode)
{
    if (g_field_modal_state == FIELD_MODAL_NONE)
    {
        field_reset_actor_resources();
        cdrom_stream(CD_RES_ADDHERO_BIN, FIELD_SUBOVERLAY_ADDRESS);
        cdrom_wait_queue_empty();
        g_field_niki_addhero_state = 1;
        g_field_modal_state = FIELD_MODAL_ADDHERO;
        niki_addhero_init(FIELD_MODAL_WORK_BUFFER, mode);
    }
}
