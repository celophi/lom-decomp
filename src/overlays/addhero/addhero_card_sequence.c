#include "addhero_internal.h"

#define ADDHERO_LOAD_RESULT_PENDING 1

/** @brief Card-path workspace retained while advancing a load/save sequence. */
typedef struct
{
    AddheroCardDevice device;
    u8 suffix[100];
} AddheroSequenceFilePath;

/** @brief Commands in the card load/save sequence bytecode. */
typedef enum
{
    ADDHERO_STEP_IDLE = 0,
    ADDHERO_STEP_CARD_INFO = 1,
    ADDHERO_STEP_POLL_CARD_INFO = 2,
    ADDHERO_STEP_CLEAR_SOFTWARE_EVENTS = 3,
    ADDHERO_STEP_POLL_HARDWARE_EVENTS = 4,
    ADDHERO_STEP_CLEAR_HARDWARE_EVENTS = 5,
    ADDHERO_STEP_SCAN_ENTRIES = 6,
    ADDHERO_STEP_CLEAR_CARD = 8,
    ADDHERO_STEP_LOAD_CARD = 9,
    ADDHERO_STEP_ERASE_ENTRY = 10,
    ADDHERO_STEP_POLL_CARD_LOAD = 15,
    ADDHERO_STEP_WAIT_HARDWARE_EVENTS = 16,
    ADDHERO_STEP_READ_ENTRY = 17,
    ADDHERO_STEP_POLL_ENTRY_READ = 18,
    ADDHERO_STEP_READ_SAVE = 19,
    ADDHERO_STEP_POLL_SAVE_READ = 20,
    ADDHERO_STEP_CHECK_CARD_TYPE = 24,
    ADDHERO_STEP_WRITE_SAVE = 25,
    ADDHERO_STEP_POLL_SAVE_WRITE = 26,
    ADDHERO_STEP_READ_BEFORE_WRITE = 27,
    ADDHERO_STEP_POLL_PREWRITE_READ = 28,
    ADDHERO_STEP_INIT_RETRIES = 30
} AddheroCardStep;

extern s32 g_addhero_retry_count;
extern s32 g_addhero_primary_poll_countdown;
extern s32 g_addhero_secondary_poll_countdown;
extern s32 g_addhero_file_handle;
extern u8 g_addhero_target_file_path[];

s32 open(void* a, s32 b);
s32 rename(void* a, void* b);
s32 _card_load(s32 a);
s32 _card_clear(s32 a);
s32 func_80032174(s32 a, void* b, s32* c);
s32 McxCardType(s32 a);

/**
 * @brief Remove the two placeholder files while advancing the card sequence.
 * @see decomp.me (100%)
 */
static inline void addhero_erase_placeholder_files_inline(void)
{
    AddheroProbeFilePath p;

    memcpy(&p, &g_addhero_file_template, 6);
    p.device.characters.slot += *(u8*)&g_addhero_card_slot;
    strcat(&p, &g_lom_save_dummy_filename);
    erase(&p);

    memcpy(&p, &g_addhero_file_template, 6);
    p.device.characters.slot += *(u8*)&g_addhero_card_slot;
    strcat(&p, &g_lom_alt_save_dummy_filename);
    erase(&p);
}

/**
 * @brief Advance the active memory-card load/save sequence by one step.
 * @return One of the ADDHERO_LOAD_RESULT_* values describing how the caller
 *         should continue the sequence.
 */
s32 addhero_advance_load_sequence(void)
{
    AddheroSequenceFilePath card_path;
    s32 card_status0;
    s32 card_status1;
    s32 result;
    s32 attempts;
    s32 poll_status;
    s32 io_status;
    s32 entry_index;
    s32 empty_rank;
    s32 load_step;

    memcpy(&card_path, &g_addhero_file_template, 6);
    result = ADDHERO_LOAD_RESULT_PENDING;
    card_path.device.characters.slot += *(u8*)&g_addhero_card_slot;

    if (g_addhero_load_step == NULL)
    {
        return result;
    }

    load_step = *g_addhero_load_step;
    switch (load_step)
    {

    case ADDHERO_STEP_CARD_INFO:
        result = ADDHERO_LOAD_RESULT_CONTINUE;
        _card_wait(g_addhero_card_slot);
        _card_info(g_addhero_card_slot * 0x10);
        g_addhero_load_step++;
        break;

    case ADDHERO_STEP_POLL_CARD_INFO:
        poll_status = addhero_poll_software_card_events();
        switch (poll_status)
        {
        case ADDHERO_CARD_EVENT_COMPLETE:
            g_addhero_load_step++;
            break;
        case ADDHERO_CARD_EVENT_ERROR:
        case ADDHERO_CARD_EVENT_TIMEOUT:
            result = ADDHERO_LOAD_RESULT_COMPLETE;
            g_addhero_selection_status = 0;
            g_addhero_entry_state = ADDHERO_ENTRY_STATE_CARD_IO_ERROR;
            g_addhero_load_step++;
            break;
        case ADDHERO_CARD_EVENT_NEW_CARD:
            g_addhero_rank_count = 0x28;
            empty_rank = -1;
            for (entry_index = 14; entry_index >= 0; entry_index--)
            {
                g_addhero_entry_ranks[entry_index] = empty_rank;
            }
            g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
            g_addhero_load_step = &g_addhero_loadseq_start;
            break;
        }
        break;

    case ADDHERO_STEP_CLEAR_SOFTWARE_EVENTS:
        addhero_clear_software_card_events();
        g_addhero_load_step++;
        break;

    case ADDHERO_STEP_POLL_HARDWARE_EVENTS:
        do
        {
            poll_status = addhero_poll_hardware_card_events();
        } while (poll_status == ADDHERO_CARD_EVENT_NONE);
        if (poll_status == ADDHERO_CARD_EVENT_COMPLETE)
        {
            g_addhero_load_step++;
            break;
        }
        if (poll_status < ADDHERO_CARD_EVENT_COMPLETE)
        {
            break;
        }
        if (poll_status >= ADDHERO_CARD_EVENT_COUNT)
        {
            break;
        }
        result = ADDHERO_LOAD_RESULT_COMPLETE;
        g_addhero_selection_status = 0;
        g_addhero_entry_state = ADDHERO_ENTRY_STATE_CARD_IO_ERROR;
        break;

    case ADDHERO_STEP_CLEAR_HARDWARE_EVENTS:
        addhero_clear_hardware_card_events();
        g_addhero_load_step++;
        break;

    case ADDHERO_STEP_SCAN_ENTRIES:
        addhero_erase_placeholder_files_inline();
        g_addhero_entry_scan_active = 1;
        if (addhero_begin_entry_scan(g_addhero_card_slot) == 0)
        {
            result = ADDHERO_LOAD_RESULT_ABORT;
            g_addhero_load_step = NULL;
            g_addhero_entry_state = 0xF8;
            g_addhero_entry_scan_active = 0;
            break;
        }
        attempts = 0;
        g_addhero_load_step++;
        do
        {
            if (addhero_scan_next_entry(g_addhero_card_slot) == 0)
            {
                if (g_addhero_mode != 0)
                {
                    g_addhero_selected_row = 0;
                }
                g_addhero_entry_scan_active = 0;
                if (g_addhero_entry_state == 0xF8)
                {
                    break;
                }
                if (g_addhero_entry_state == ADDHERO_ENTRY_STATE_CARD_FULL)
                {
                    break;
                }
                addhero_commit_selected_entry();
                break;
            }
            attempts++;
        } while (attempts < 0x14);
        break;

    case ADDHERO_STEP_CLEAR_CARD:
        result = ADDHERO_LOAD_RESULT_CONTINUE;
        _card_wait(g_addhero_card_slot);
        _card_clear(g_addhero_card_slot * 0x10);
        g_addhero_load_step++;
        break;

    case ADDHERO_STEP_LOAD_CARD:
        result = ADDHERO_LOAD_RESULT_CONTINUE;
        _card_wait(g_addhero_card_slot);
        _card_load(g_addhero_card_slot * 0x10);
        g_addhero_primary_poll_countdown = 0x10;
        g_addhero_secondary_poll_countdown = 0x10;
        g_addhero_load_step++;
        break;

    case ADDHERO_STEP_IDLE:
        result = ADDHERO_LOAD_RESULT_ABORT;
        g_addhero_write_in_progress = 0;
        break;

    case ADDHERO_STEP_ERASE_ENTRY:
        strcat(&card_path, g_addhero_entries[g_addhero_card_slot][g_addhero_selected_row].name);
        attempts = 0;
        do
        {
            if (erase(&card_path) != 0)
            {
                break;
            }
            attempts++;
        } while (attempts < 0x14);
        g_addhero_load_step++;
        break;

    case ADDHERO_STEP_POLL_CARD_LOAD:
        poll_status = addhero_poll_software_card_events();
        switch (poll_status)
        {
        case ADDHERO_CARD_EVENT_COMPLETE:
            g_addhero_load_step++;
            break;
        case ADDHERO_CARD_EVENT_ERROR:
        case ADDHERO_CARD_EVENT_TIMEOUT:
            g_addhero_secondary_poll_countdown--;
            if (g_addhero_secondary_poll_countdown != 0)
            {
                goto reissue_card_load;
            }
            result = ADDHERO_LOAD_RESULT_COMPLETE;
            g_addhero_selection_status = 0;
            g_addhero_entry_state = ADDHERO_ENTRY_STATE_CARD_IO_ERROR;
            break;
        case ADDHERO_CARD_EVENT_NEW_CARD:
            g_addhero_primary_poll_countdown--;
            if (g_addhero_primary_poll_countdown != 0)
            {
            reissue_card_load:
                _card_wait(g_addhero_card_slot);
                _card_clear(g_addhero_card_slot * 0x10);
                _card_wait(g_addhero_card_slot);
                _card_load(g_addhero_card_slot * 0x10);
            }
            else
            {
                result = ADDHERO_LOAD_RESULT_CARD_ERROR;
                g_addhero_entry_state = 0xFC;
                g_addhero_load_step = g_addhero_loadseq_card;
            }
            break;
        }
        break;

    case ADDHERO_STEP_WAIT_HARDWARE_EVENTS:
        do
        {
            poll_status = addhero_poll_hardware_card_events();
        } while (poll_status == ADDHERO_CARD_EVENT_NONE);
        g_addhero_load_step++;
        break;

    case ADDHERO_STEP_READ_ENTRY:
        g_addhero_io_busy = 1;
        g_addhero_selection_status = 0;
        _card_wait(g_addhero_card_slot);
        g_addhero_file_handle = open(g_addhero_save_file_path, 0x8001);
        if (g_addhero_file_handle == -1)
        {
            break;
        }
        addhero_clear_software_card_events();
        _card_wait(g_addhero_card_slot);
        if (read(g_addhero_file_handle, &g_addhero_entry_read_buffer, g_addhero_selected_entry_extended != 0 ? 0x280 : 0x80) == -1)
        {
            close(g_addhero_file_handle);
            break;
        }
        g_addhero_load_step++;
        break;

    case ADDHERO_STEP_POLL_ENTRY_READ:
        if (g_addhero_io_busy != 0)
        {
            poll_status = addhero_poll_software_card_events();
            if (poll_status == ADDHERO_CARD_EVENT_COMPLETE)
            {
                g_addhero_io_busy = 0;
                g_addhero_selection_status = 1;
                close(g_addhero_file_handle);
                break;
            }
            if (poll_status == ADDHERO_CARD_EVENT_NONE)
            {
                break;
            }
            close(g_addhero_file_handle);
            g_addhero_entry_state = ADDHERO_ENTRY_STATE_IDLE;
            g_addhero_load_step = &g_addhero_loadseq_start;
        }
        else
        {
            g_addhero_load_step++;
        }
        break;

    case ADDHERO_STEP_READ_SAVE:
        g_addhero_progress_active = 1;
        g_addhero_progress_start_tick = VSync(-1);
        g_addhero_progress_bar_active = 1;
        _card_wait(g_addhero_card_slot);
        g_addhero_file_handle = open(g_addhero_save_file_path, 0x8001);
        addhero_clear_software_card_events();
        _card_wait(g_addhero_card_slot);
        if (read(g_addhero_file_handle, g_addhero_save_blob, 0x4000) == -1)
        {
            close(g_addhero_file_handle);
            g_addhero_retry_count--;
            if (g_addhero_retry_count == 0)
            {
            show_read_error:
                addhero_open_status_dialog(1);
                break;
            }
            break;
        }
        g_addhero_load_step++;
        break;

    case ADDHERO_STEP_POLL_SAVE_READ:
        io_status = addhero_poll_software_card_events();
        if (io_status == ADDHERO_CARD_EVENT_COMPLETE)
        {
            g_addhero_progress_active = 0;
            g_addhero_load_step++;
            close(g_addhero_file_handle);
            break;
        }
        if (io_status < ADDHERO_CARD_EVENT_COMPLETE)
        {
            break;
        }
        if (io_status >= ADDHERO_CARD_EVENT_COUNT)
        {
            break;
        }
        close(g_addhero_file_handle);
        g_addhero_retry_count--;
        if (g_addhero_retry_count == 0)
        {
            g_addhero_progress_bar_active = 0;
            goto show_read_error;
        }
        g_addhero_load_step--;
        break;

    case ADDHERO_STEP_CHECK_CARD_TYPE:
        attempts = 0;
        do
        {
            if (McxCardType(g_addhero_card_slot * 0x10) == 1)
            {
                break;
            }
            VSync(0);
            attempts++;
        } while (attempts < 0x14);
        if (attempts != 0x14)
        {
            func_80032174(0, &card_status0, &card_status1);
            if (card_status1 == 0)
            {
                g_addhero_load_step++;
                break;
            }
        }
        addhero_open_status_dialog(3);
        break;

    case ADDHERO_STEP_INIT_RETRIES:
        g_addhero_retry_count = 5;
        g_addhero_load_step++;
        break;

    case ADDHERO_STEP_READ_BEFORE_WRITE:
        g_addhero_progress_active = 1;
        g_addhero_progress_start_tick = VSync(-1);
        g_addhero_progress_bar_active = 1;
        _card_wait(g_addhero_card_slot);
        g_addhero_file_handle = open(g_addhero_save_file_path, 0x8001);
        addhero_clear_software_card_events();
        _card_wait(g_addhero_card_slot);
        if (read(g_addhero_file_handle, g_addhero_save_blob, 0x4000) == -1)
        {
            close(g_addhero_file_handle);
            g_addhero_retry_count--;
            if (g_addhero_retry_count == 0)
            {
            show_prewrite_read_error:
                addhero_open_exit_dialog(1);
                break;
            }
            break;
        }
        g_addhero_load_step++;
        break;

    case ADDHERO_STEP_POLL_PREWRITE_READ:
        io_status = addhero_poll_software_card_events();
        if (io_status == ADDHERO_CARD_EVENT_COMPLETE)
        {
            g_addhero_progress_active = 0;
            g_addhero_load_step++;
            close(g_addhero_file_handle);
            break;
        }
        if (io_status < ADDHERO_CARD_EVENT_COMPLETE)
        {
            break;
        }
        if (io_status >= ADDHERO_CARD_EVENT_COUNT)
        {
            break;
        }
        g_addhero_retry_count--;
        if (g_addhero_retry_count == 0)
        {
            g_addhero_progress_bar_active = 0;
            goto show_prewrite_read_error;
        }
        goto retry_previous_step;

    case ADDHERO_STEP_WRITE_SAVE:
        if (g_addhero_has_free_entry_space == 0)
        {
            _card_wait(g_addhero_card_slot);
            attempts = 0;
            do
            {
                if (erase(g_addhero_save_file_path) != 0)
                {
                    break;
                }
                attempts++;
            } while (attempts < 0x14);
        }
        strcat(&card_path, g_lom_save_dummy_filename);
        _card_wait(g_addhero_card_slot);
        g_addhero_file_handle = open(&card_path, 0x20200);
        if (g_addhero_file_handle == -1)
        {
            close(-1);
            attempts = 0;
            do
            {
                if (erase(&card_path) != 0)
                {
                    break;
                }
                attempts++;
            } while (attempts < 0x14);
        retry_save_write:
            g_addhero_retry_count--;
            if (g_addhero_retry_count == 0)
            {
            show_write_error:
                addhero_open_exit_dialog(0);
                break;
            }
            break;
        }
        close(g_addhero_file_handle);
        strcpy(g_addhero_target_file_path, &card_path);
        _card_wait(g_addhero_card_slot);
        g_addhero_file_handle = open(g_addhero_target_file_path, 0x8002);
        addhero_clear_software_card_events();
        g_addhero_progress_start_tick = VSync(-1);
        g_addhero_progress_bar_active = 1;
        _card_wait(g_addhero_card_slot);
        if (write(g_addhero_file_handle, g_addhero_save_blob, 0x4000) == -1)
        {
            close(g_addhero_file_handle);
            attempts = 0;
            do
            {
                if (erase(g_addhero_target_file_path) != 0)
                {
                    break;
                }
                attempts++;
            } while (attempts < 0x14);
            goto retry_save_write;
        }
        g_addhero_load_step++;
        break;

    case ADDHERO_STEP_POLL_SAVE_WRITE:
        io_status = addhero_poll_software_card_events();
        if (io_status != ADDHERO_CARD_EVENT_COMPLETE)
        {
            if (io_status < ADDHERO_CARD_EVENT_COMPLETE)
            {
                break;
            }
            if (io_status >= ADDHERO_CARD_EVENT_COUNT)
            {
                break;
            }
        }
        else
        {
            if (g_addhero_has_free_entry_space != 0)
            {
                _card_wait(g_addhero_card_slot);
                attempts = 0;
                do
                {
                    if (erase(g_addhero_save_file_path) != 0)
                    {
                        break;
                    }
                    attempts++;
                } while (attempts < 0x14);
            }
            _card_wait(g_addhero_card_slot);
            attempts = 0;
            do
            {
                if (rename(g_addhero_target_file_path, g_addhero_save_file_path) != 0)
                {
                    break;
                }
                attempts++;
            } while (attempts < 0x14);
            g_addhero_write_in_progress = 0;
            g_addhero_load_step++;
            close(g_addhero_file_handle);
            break;
        }
        g_addhero_retry_count--;
        if (g_addhero_retry_count == 0)
        {
            g_addhero_progress_bar_active = 0;
            goto show_write_error;
        }

    retry_previous_step:
        close(g_addhero_file_handle);
        g_addhero_load_step--;

    default:
        break;
    }

    return result;
}
