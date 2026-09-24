/** @file field_modal_stream_start.c
 * @brief Load and enter the CARDA, GOLEM, NIKI and ADDHERO sub-overlays.
 */

#include "cd_resources.h"
#include "cdrom.h"
#include "common.h"
#include "field_modal_runtime.h"

/** @brief Load address of the modal sub-overlays. */
#define FIELD_SUBOVERLAY_ADDRESS ((void*)0x80140000)

/** @brief Work buffer handed to the sub-overlay entry points. */
#define FIELD_MODAL_WORK_BUFFER 0x80170000

extern s32 g_field_modal_state;
extern s32 g_field_card_overlay_mode;
extern s32 g_field_niki_addhero_state;
extern s32 D_801227C4;
extern s32 D_801227F0;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];

void func_80084240(void);
void func_8014024C(void* work, s32 mode);
void func_80140024(void* work, s32 mode);
void func_8014011C(s32 work, s32 mode);
void func_800C3BB0(void);

/**
 * @brief Load CARDA and start it, unless another modal is already running.
 * @param mode Card-screen mode; stored plus one and passed to the CARDA entry point.
 */
void func_800AD030(s32 mode)
{
    if (g_field_modal_state == FIELD_MODAL_NONE)
    {
        func_80084240();
        cdrom_stream(CD_RES_CARDA_BIN, FIELD_SUBOVERLAY_ADDRESS);
        cdrom_wait_queue_empty();
        g_field_card_overlay_mode = mode + 1;
        D_801227F0 = 1;
        g_gosub_result_count = 0;
        g_field_modal_state = FIELD_MODAL_CARDA;
        D_801227C4 = g_gosub_result_values[0];
        func_8014024C((void*)FIELD_MODAL_WORK_BUFFER, mode);
    }
}

/**
 * @brief Load GOLEM, run it to completion, then run func_800C3BB0.
 */
void func_800AD0C8(void)
{
    func_80084240();
    cdrom_stream(CD_RES_GOLEM_BIN, FIELD_SUBOVERLAY_ADDRESS);
    cdrom_wait_queue_empty();
    func_80140024((void*)0x80150000, 0);
    func_800C3BB0();
    func_80084240();
}

/** @brief Empty per-frame hook, called with the render half each frame. */
void func_800AD118(void)
{
}

/**
 * @brief Load NIKI and start it, unless another modal is already running.
 * @param mode Mode passed to the NIKI entry point.
 */
void func_800AD120(s32 mode)
{
    if (g_field_modal_state == FIELD_MODAL_NONE)
    {
        func_80084240();
        cdrom_stream(CD_RES_NIKI_BIN, FIELD_SUBOVERLAY_ADDRESS);
        cdrom_wait_queue_empty();
        g_field_niki_addhero_state = 1;
        g_field_modal_state = FIELD_MODAL_NIKI;
        func_8014011C(FIELD_MODAL_WORK_BUFFER, mode);
    }
}

/**
 * @brief Load ADDHERO and start it, unless another modal is already running.
 * @param mode Mode passed to the ADDHERO entry point.
 */
void func_800AD194(s32 mode)
{
    if (g_field_modal_state == FIELD_MODAL_NONE)
    {
        func_80084240();
        cdrom_stream(CD_RES_ADDHERO_BIN, FIELD_SUBOVERLAY_ADDRESS);
        cdrom_wait_queue_empty();
        g_field_niki_addhero_state = 1;
        g_field_modal_state = FIELD_MODAL_ADDHERO;
        func_8014011C(FIELD_MODAL_WORK_BUFFER, mode);
    }
}
