#include "internal/menu_internal.h"
#include "common/card_directory.h"
#include "common/card_events.h"
#include <fcntl.h>

/** @brief Bytes of g_card_save_title_sjis copied into the title. */
#define MEMORY_CARD_SAVE_TITLE_SIZE 0x11

s32 memory_card_wait_software_event(void);
void memory_card_clear_software_events(void);
s32 memory_card_wait_hardware_event(void);
void memory_card_clear_hardware_events(void);
s32 memory_card_scan_files(char* path, struct DIRENTRY* entry);
s32 memory_card_create_save_file(char* path, void* save_buffer);
void memory_card_fill_test_data(void* buf);

/** @brief SwCARD completion-event descriptor. */
extern s32 g_card_sw_io_event;
/** @brief SwCARD error-event descriptor. */
extern s32 g_card_sw_error_event;
/** @brief SwCARD timeout-event descriptor. */
extern s32 g_card_sw_timeout_event;
/** @brief SwCARD new-card event descriptor. */
extern s32 g_card_sw_new_event;
/** @brief HwCARD completion-event descriptor. */
extern s32 g_card_hw_io_event;
/** @brief HwCARD error-event descriptor. */
extern s32 g_card_hw_error_event;
/** @brief HwCARD timeout-event descriptor. */
extern s32 g_card_hw_timeout_event;
/** @brief HwCARD new-card event descriptor. */
extern s32 g_card_hw_new_event;

/** @brief Memory-card slot 1 device path, "bu00:". */
extern char g_card_slot1_path[];
/** @brief Directory entries filled while scanning memory-card slot 1. */
extern struct DIRENTRY g_card_dir_entries[];
/** @brief Number of directory entries found in memory-card slot 1. */
extern s32 g_card_file_count;

/** @brief Test-save path, "bu00:HAND". */
extern char g_card_save_path[];
/** @brief Shared buffer used to assemble and write a memory-card save block. */
extern u8 g_card_work_buffer[];

extern char g_card_wildcard[];

extern u8 g_card_save_title_sjis[];
extern u8 g_card_header[];
extern u8 g_card_header_block_count;

extern u8 g_card_test_payload[];

/**
 * @brief Open and enable the eight memory-card events used by the save/load menu.
 */
void memory_card_open_events(void)
{
    reset_controller_vsync_state();
    EnterCriticalSection();
    g_card_sw_io_event = OpenEvent(SwCARD, EvSpIOE, EvMdNOINTR, NULL);
    g_card_sw_error_event = OpenEvent(SwCARD, EvSpERROR, EvMdNOINTR, NULL);
    g_card_sw_timeout_event = OpenEvent(SwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
    g_card_sw_new_event = OpenEvent(SwCARD, EvSpNEW, EvMdNOINTR, NULL);
    g_card_hw_io_event = OpenEvent(HwCARD, EvSpIOE, EvMdNOINTR, NULL);
    g_card_hw_error_event = OpenEvent(HwCARD, EvSpERROR, EvMdNOINTR, NULL);
    g_card_hw_timeout_event = OpenEvent(HwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
    g_card_hw_new_event = OpenEvent(HwCARD, EvSpNEW, EvMdNOINTR, NULL);
    EnableEvent(g_card_sw_io_event);
    EnableEvent(g_card_sw_error_event);
    EnableEvent(g_card_sw_timeout_event);
    EnableEvent(g_card_sw_new_event);
    EnableEvent(g_card_hw_io_event);
    EnableEvent(g_card_hw_error_event);
    EnableEvent(g_card_hw_timeout_event);
    EnableEvent(g_card_hw_new_event);
    ExitCriticalSection();
}

/**
 * @brief Close the eight memory-card events opened by memory_card_open_events.
 */
void memory_card_close_events(void)
{
    reset_controller_vsync_state();
    EnterCriticalSection();
    CloseEvent(g_card_sw_io_event);
    CloseEvent(g_card_sw_error_event);
    CloseEvent(g_card_sw_timeout_event);
    CloseEvent(g_card_sw_new_event);
    CloseEvent(g_card_hw_io_event);
    CloseEvent(g_card_hw_error_event);
    CloseEvent(g_card_hw_timeout_event);
    CloseEvent(g_card_hw_new_event);
    ExitCriticalSection();
}

/**
 * @brief Scan memory card slot 1 and record how many save files it holds.
 */
void memory_card_scan_slot1_files(void)
{
    g_card_file_count = 0;
    g_card_file_count = memory_card_scan_files(g_card_slot1_path, g_card_dir_entries);
}

/**
 * @brief Bring memory card slot 1 up to a usable state, formatting it if needed.
 * @return 1 if the card is ready for use, 0 if it was rejected up front or the format attempt failed.
 */
s32 memory_card_prepare_slot1(void)
{
    s32 status;

    _card_info(0);
    status = memory_card_wait_software_event();
    if ((status == CARD_EVENT_ERROR) || (status == CARD_EVENT_TIMEOUT))
    {
        return 0;
    }
    if (status == CARD_EVENT_NEW_CARD)
    {
        memory_card_clear_hardware_events();
        _card_clear(0);
        memory_card_wait_hardware_event();
    }
    memory_card_clear_software_events();
    _card_load(0);
    if (memory_card_wait_software_event() == CARD_EVENT_NEW_CARD)
    {
        if (_card_format(0) == 0)
        {
            return 0;
        }
    }
    return 1;
}

/**
 * @brief Populate the card work buffer and write it as "bu00:HAND".
 */
void memory_card_write_test_save(void)
{
    memory_card_fill_test_data(g_card_work_buffer);
    memory_card_create_save_file(g_card_save_path, g_card_work_buffer);
}

/**
 * @brief Block until one of the four SwCARD events fires and report which.
 * @return CardEvent result for the first event observed.
 */
s32 memory_card_wait_software_event(void)
{
    while (1)
    {
        if (TestEvent(g_card_sw_io_event) == 1)
        {
            return CARD_EVENT_COMPLETE;
        }
        if (TestEvent(g_card_sw_error_event) == 1)
        {
            return CARD_EVENT_ERROR;
        }
        if (TestEvent(g_card_sw_timeout_event) == 1)
        {
            return CARD_EVENT_TIMEOUT;
        }
        if (TestEvent(g_card_sw_new_event) == 1)
        {
            return CARD_EVENT_NEW_CARD;
        }
    }
}

/**
 * @brief Drain the four SwCARD events by testing each one once.
 */
void memory_card_clear_software_events(void)
{
    TestEvent(g_card_sw_io_event);
    TestEvent(g_card_sw_error_event);
    TestEvent(g_card_sw_timeout_event);
    TestEvent(g_card_sw_new_event);
}

/**
 * @brief Block until one of the four HwCARD events fires and report which.
 * @return CardEvent result for the first event observed.
 */
s32 memory_card_wait_hardware_event(void)
{
    while (1)
    {
        if (TestEvent(g_card_hw_io_event) == 1)
        {
            return CARD_EVENT_COMPLETE;
        }
        if (TestEvent(g_card_hw_error_event) == 1)
        {
            return CARD_EVENT_ERROR;
        }
        if (TestEvent(g_card_hw_timeout_event) == 1)
        {
            return CARD_EVENT_TIMEOUT;
        }
        if (TestEvent(g_card_hw_new_event) == 1)
        {
            return CARD_EVENT_NEW_CARD;
        }
    }
}

/**
 * @brief Drain the four HwCARD events by testing each one once.
 */
void memory_card_clear_hardware_events(void)
{
    TestEvent(g_card_hw_io_event);
    TestEvent(g_card_hw_error_event);
    TestEvent(g_card_hw_timeout_event);
    TestEvent(g_card_hw_new_event);
}

/**
 * @brief Count the files on a memory card matching a path prefix.
 * @param path Memory-card path prefix; the wildcard suffix is appended internally.
 * @param entry Start of the caller's directory-entry table; one struct DIRENTRY is filled per file found, so it must have room for every match.
 * @return Number of files found; 0 if the card holds no match at all.
 */
s32 memory_card_scan_files(char* path, struct DIRENTRY* entry)
{
    char pattern[0x80];
    s32 count;
    struct DIRENTRY* found;

    strcpy(pattern, path);
    strcat(pattern, g_card_wildcard);
    count = 0;
    for (found = firstfile(pattern, entry); found == entry; found = nextfile(entry))
    {
        count++;
        entry++;
    }
    return count;
}

/**
 * @brief Read card sector 0 and check its format signature.
 * @param channel Card channel passed to _card_read.
 * @return 1 if the card is formatted, 0 if the "MC" magic is absent, -1 if the event poll reported anything other than completion.
 */
s32 memory_card_check_formatted(s32 channel)
{
    u8 header[CARD_SECTOR_BYTES];
    s32 status;
    s32 event_ready;
    s32* io_event;
    s32* error_event;

    bzero(header, sizeof(header));
    TestEvent(g_card_hw_io_event);
    TestEvent(g_card_hw_error_event);
    TestEvent(g_card_hw_timeout_event);
    TestEvent(g_card_hw_new_event);
    _new_card();
    _card_read(channel, 0, header);

    while (1)
    {
        io_event = &g_card_hw_io_event;
        event_ready = 1;
        error_event = &g_card_hw_error_event;
        status = CARD_EVENT_NEW_CARD;
        if (TestEvent(*io_event) == event_ready)
        {
            status = CARD_EVENT_COMPLETE;
            break;
        }
        if (TestEvent(*error_event) == event_ready)
        {
            status = CARD_EVENT_ERROR;
            break;
        }
        if (TestEvent(g_card_hw_timeout_event) == event_ready)
        {
            status = CARD_EVENT_TIMEOUT;
            break;
        }
        if (TestEvent(g_card_hw_new_event) == event_ready)
        {
            break;
        }
    }
    if (status != CARD_EVENT_COMPLETE)
    {
        return -1;
    }

    if ((header[0] == 'M') && (header[1] == 'C'))
    {
        return 1;
    }

    return 0;
}

/**
 * @brief Create a one-block memory-card save file and write its data.
 * @param path Memory-card file path to create.
 * @param save_buffer Writable 8 KiB buffer; its first 512 bytes are replaced with the save header.
 * @return 1 if the complete file is written; otherwise 0.
 */
s32 memory_card_create_save_file(char* path, void* save_buffer)
{
    s32 block_count;
    s32 file_descriptor;
    s32 byte_count;

    /* Build the 512-byte memory-card file header. */
    ((CardFileHeader*)g_card_header)->magic[0] = 'S';
    ((CardFileHeader*)g_card_header)->magic[1] = 'C';
    ((CardFileHeader*)g_card_header)->icon_type = CARD_FILE_ICON_TYPE(CARD_FILE_ICON_MAX_FRAMES);
    block_count = 1;
    ((CardFileHeader*)g_card_header)->block_count = block_count;
    memcpy(((CardFileHeader*)g_card_header)->title, g_card_save_title_sjis, MEMORY_CARD_SAVE_TITLE_SIZE);
    bzero(((CardFileHeader*)g_card_header)->reserved, CARD_FILE_RESERVED_BYTES);
    memcpy(save_buffer, g_card_header, sizeof(CardFileHeader));

    /* Allocate one card block, then reopen the file for writing. */
    file_descriptor = open(path, CARD_FILE_BLOCKS(g_card_header_block_count) | FCREAT);
    if (file_descriptor == CARD_FILE_ERROR)
    {
        return 0;
    }
    close(file_descriptor);
    file_descriptor = open(path, FWRITE);
    if (file_descriptor == CARD_FILE_ERROR)
    {
        return 0;
    }

    /* Each memory-card block contains 8 KiB. */
    byte_count = block_count * CARD_BLOCK_BYTES;
    if (write(file_descriptor, save_buffer, byte_count) != byte_count)
    {
        close(file_descriptor);
        return 0;
    }
    close(file_descriptor);
    return 1;
}

/**
 * @brief Copy the five-byte test payload into a save buffer.
 * @param buf Destination buffer with space for at least five bytes.
 */
void memory_card_fill_test_data(void* buf)
{
    memcpy(buf, g_card_test_payload, 5);
}
