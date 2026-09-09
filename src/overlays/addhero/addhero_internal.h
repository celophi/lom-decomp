#ifndef ADDHERO_INTERNAL_H
#define ADDHERO_INTERNAL_H

#include "common.h"
#include "pad.h"
#include "vector.h"
#include "display.h"
#include "gpu_packet.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/kernel.h"

/* Declarations shared by ADDHERO implementation files. */

#define ADDHERO_DIRECTORY_ENTRY_COUNT 20
#define ADDHERO_DIRECTORY_ENTRY_BYTES sizeof(struct DIRENTRY)
#define ADDHERO_CARD_DIRECTORY_BYTES (ADDHERO_DIRECTORY_ENTRY_COUNT * ADDHERO_DIRECTORY_ENTRY_BYTES)
#define ADDHERO_CARD_BLOCK_BYTES 8192
#define ADDHERO_USED_BLOCK_LIMIT 14
#define ADDHERO_SAVE_FILENAME_PREFIX_LENGTH 12
#define ADDHERO_NEW_SAVE_FILENAME_PREFIX_LENGTH 8
#define ADDHERO_LOAD_RESULT_ABORT 2
#define ADDHERO_LOAD_RESULT_CONTINUE 3
#define ADDHERO_LOAD_RESULT_COMPLETE 4
#define ADDHERO_LOAD_RESULT_CARD_ERROR 5

/*
 * g_addhero_entry_state is dual-purpose:
 *
 *   0x00-0x0F  Number of save entries loaded from the current card. A PSX
 *              memory card holds 15 directory blocks, so the count never
 *              reaches 0x10; the value is used directly as a count/index
 *              while scanning and browsing the entry list.
 *   0xF3-0xFF  Modal state-machine sentinel. The `state >= 0x10` test tells a
 *              sentinel apart from a live entry count.
 *
 * Only the sentinels whose role is unambiguous from the control flow are named
 * below. The remaining sentinels are message/status screens whose exact meaning
 * depends on the (binary-resident) message resource each one draws, so they are
 * left as raw values until identified:
 *   0xF3  companion prompt page to SAVE_CONFIRM (navigates to it or cancels)
 *   0xF7  drawn when the selected entry is not found while advancing the list
 *   0xF8/0xF9  mode-dependent card read/scan failure notice
 *   0xFA/0xFB/0xFC/0xFD  status/result notices
 *   0xFE  no-op (draws nothing)
 */
#define ADDHERO_ENTRY_STATE_IDLE 0xFF /* browsing / reset; no operation active */

/** @brief Memory-card device prefix, such as "bu00", stored with word alignment. */
typedef union
{
    u32 word;
    struct
    {
        u8 name[2];
        u8 slot;
        u8 port;
    } characters;
} AddheroCardDevice;

/** @brief Eight-byte card-path template, including its suffix and terminator. */
typedef struct
{
    AddheroCardDevice device;
    u8 suffix[4];
} AddheroCardPathTemplate;

/** @brief Card path used to remove a placeholder save file. */
typedef struct
{
    AddheroCardDevice device;
    u8 suffix[28];
} AddheroProbeFilePath;

extern struct DIRENTRY g_addhero_entries[][ADDHERO_DIRECTORY_ENTRY_COUNT];
extern AddheroCardPathTemplate g_addhero_file_template;
extern u8* g_addhero_load_step;
extern s32 g_addhero_scroll_y;
extern s32 g_addhero_progress_active;
extern s32 g_addhero_scroll_target_y;
extern s32 g_addhero_mode;
extern s32 g_addhero_entry_state;
extern s32 g_addhero_card_slot;
extern s32 g_addhero_selected_row;
extern s32 g_addhero_selection_status;
extern s32 g_addhero_scroll_frames;
extern s32 g_addhero_io_busy;
extern s32 g_addhero_progress_bar_active;
extern s32 g_addhero_progress_start_tick;
extern s32 g_addhero_entry_scan_active;
extern s32 g_addhero_write_in_progress;
extern s32 g_addhero_rank_count;
extern s32 g_addhero_entry_suffix_values[];
extern s32 g_addhero_entry_ranks[];
extern s32 g_addhero_selected_entry_extended;
extern s32 g_addhero_entry_value_limit;
extern s32 g_addhero_has_free_entry_space;
extern u8 g_addhero_loadseq_start;
extern u8 g_addhero_loadseq_card[];
extern u8 g_addhero_save_blob[];
extern u8 g_addhero_entry_read_buffer;
extern u8 g_addhero_save_file_path[];
extern char g_lom_save_filename_prefix[];
extern char g_lom_alt_save_filename_prefix[];
extern char g_new_save_entry_prefix[];
extern char g_lom_save_dummy_filename[];
extern char g_lom_alt_save_dummy_filename[];

void addhero_scroll_to_selection(void);
void addhero_open_status_dialog(s32 message_id);
void addhero_open_exit_dialog(s32 message_id);
s32 addhero_rank_entries(s32 unused0, s32 unused1, s32 unused2);
s32 addhero_has_known_entry_type(void);
s32 addhero_begin_entry_scan(s32 page);
s32 addhero_scan_next_entry(s32 page);
void addhero_clear_software_card_events(void);
void addhero_clear_hardware_card_events(void);
s32 addhero_poll_software_card_events(void);
s32 addhero_poll_hardware_card_events(void);
void addhero_sort_entries_by_type(void);
s32 strncmp(void* a, void* b, s32 n);
void func_800AA02C(void);
void func_80019788(s32 arg0);
void func_80019A34(RECT* rect, void* str);
s32 VSync(s32 arg0);
void bcopy(void* dst, void* src, s32 len);
void addhero_shutdown_card_events(void);
void addhero_begin_glyph_cache_frame(void);
void addhero_evict_unused_glyphs(void);
void addhero_reset_glyph_cache(void);
void addhero_init_card_events(void);
void addhero_restart_load_sequence(void);
s32 addhero_poll_and_retry_card_info(void);
void addhero_commit_selected_entry(void);
s32 addhero_draw_cached_text(s32 result, s32* ot, u8* name, s32 x, s32 y, s32 a5, s32 a6);
s32 addhero_advance_load_sequence();
s32 strcat(void* a, void* b);
s32 read(s32 a, void* b, s32 c);
s32 write(s32 a, void* b, s32 c);
s32 close(s32 a);
s32 erase(void* a);
s32 strcpy(void* a, void* b, ...);
s32 _card_info(s32 a);
s32 _card_wait(s32 a);

#endif
