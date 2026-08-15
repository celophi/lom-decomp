#include "cdrom.h"
#include "psyq/libetc.h"
#include "psyq/libcd.h"
#include "psyq/libpress.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "akao.h"

#define CD_RESOURCE_INDEX_INVALID 0xFFFE
#define CD_RESOURCE_INDEX_DEFAULT 0xFFFF
#define CD_COMMAND_NONE 0
#define CD_COMMAND_QUEUE_SIZE 16
#define CD_COMMAND_QUEUE_MASK (CD_COMMAND_QUEUE_SIZE - 1)
#define CD_COMMAND_QUEUE_BASE_OFFSET 4
#define CD_INIT_STATE_ERROR_PAUSE 0x20
#define CD_INIT_STATE_RETRY_READ 0x21
#define CD_STATUS_POLL_FRAMES 30
#define CD_ACTIVE_COMMAND_TIMEOUT_FRAMES 240
#define CD_RECOVERY_READ_TIMEOUT_FRAMES 270
#define CD_SET_MODE_DELAY_FRAMES 4
#define CD_RECOVERY_FLUSH_DELAY_FRAMES 1
#define CD_RESOURCE_LOAD_VSYNC_OFFSET (-3)
#define CD_DEFAULT_AUDIO_VOLUME 128
#define CD_RECOVERY_FILTER_FILE 1
#define CD_RECOVERY_FILTER_CHANNEL 1
#define CD_READY_CALLBACK_PENDING 1
#define CD_SECTOR_HEADER_WORDS 3
#define CD_SECTOR_POSITION_MASK 0x00FFFFFF
#define CD_DISC_VALIDATION_WORDS 8
#define CD_IS_MULTIBYTE_ID_CHAR(character) (((u8)((character) + 0x80) < 0x20U) || ((u8)((character) + 0x20) < 0x10U))
#define CD_DATA_SECTOR_SIZE 0x800
#define CD_DATA_SECTOR_WORDS 0x200
#define CD_BYTES_PER_WORD 4
#define CD_BYTES_PER_WORD_SHIFT 2
#define CD_BYTES_TO_WORDS(size) (((size) + (CD_BYTES_PER_WORD - 1)) >> CD_BYTES_PER_WORD_SHIFT)
#define CD_RECOVERY_SECTOR_RETRY_LIMIT 17
#define CD_DISC_READY_RETRY_LIMIT 13
#define CD_IDLE_STATUS_RETRY_LIMIT 11
#define CD_STREAM_TIMEOUT_FRAMES 30
#define CD_STREAM_DECOMPRESS_GUARD_SIZE 280
#define CD_STREAM_BUFFER_END 0x801DC118
#define CD_STREAM_BUFFER_START ((u8*)0x801DC000)
#define CD_STREAM_PAYLOAD_START ((u8*)0x801DC001)
#define CD_STREAM_WRAP_START ((u8*)0x801DC118)
#define CD_STREAM_BUFFER_LIMIT ((u8*)0x801DE000)
#define CD_DECOMPRESS_UNBOUNDED_END ((u8*)0xFFFFFFFCU)
#define CD_STREAM_COPY_WORD_SIZE 4
#define CD_STREAM_COPY_WORD_MASK 3
#define CD_STREAM_NEGATIVE_WORD_ROUNDING_BIAS 6
#define CD_STREAM_DIRECT_MODE 0x1000
#define CD_STREAM_CHUNK_GUARD_SIZE 0x418
#define CD_STREAM_STAGING_START ((u8*)0x801DA000)
#define CD_STREAM_STAGING_END ((u8*)0x801DBBE8)
#define CD_STREAM_LZ_WINDOW_SIZE 0x1000
#define CD_INIT_COMMAND_RETRY_FLAG 0x80
#define CD_INIT_COMMAND_MASK 0x7F
#define CD_DECOMPRESS_LOW_NIBBLE_MASK 0x0F
#define CD_DECOMPRESS_HIGH_NIBBLE_MASK 0xF0
#define CD_DECOMPRESS_OFFSET_MASK 0xFFFF
#define CD_DECOMPRESS_COPY_8_BIT_BASE_LENGTH 0x14

typedef void (*DecDCToutCallbackHandler)();
typedef void (*DrawSyncCallbackHandler)();
typedef union
{
    CdlLOC pos;
    u32 raw;
    u8 bytes[sizeof(u32)];
} CdlLOCRaw;

typedef struct CdResourceEntry
{
    CdlLOCRaw location;
    s32 data_size;
} CdResourceEntry;

typedef struct CdCommandQueueItem
{
    u8 command;
    u8 padding;
    u16 resource_index;
    CdResourceEntry* entry;
    void* dst_buffer;
    CdCommandCallback callback;
} CdCommandQueueItem;

typedef struct CdCommandQueue
{
    CdCommandQueueItem items[CD_COMMAND_QUEUE_SIZE];
} CdCommandQueue;

typedef union
{
    u32 word;
    struct
    {
        u8 flags;
        u8 defer_data_ready;
        u8 data_ready_pending;
        u8 retry_exhausted;
    } bytes;
} CdStatusFlags;

typedef enum CdStatusFlag
{
    CD_STATUS_SYNC_ERROR = 0x01,
    CD_STATUS_INVALID_DISC = 0x02,
    CD_STATUS_NO_DISC = 0x04,
    CD_STATUS_RECOVERY_PENDING = 0x08,
    CD_STATUS_COMMAND_ACTIVE = 0x10,
    CD_STATUS_SUPPRESS_IDLE_POLL = 0x20,
    CD_STATUS_QUEUE_LOCK = 0x40,
} CdStatusFlag;

#define CD_STATUS_ERROR_MASK (CD_STATUS_SYNC_ERROR | CD_STATUS_INVALID_DISC | CD_STATUS_NO_DISC)
#define CD_STATUS_RECOVERY_MASK (CD_STATUS_ERROR_MASK | CD_STATUS_RECOVERY_PENDING)

typedef enum CdRecoveryState
{
    CD_RECOVERY_STATE_IDLE = 0,
    CD_RECOVERY_STATE_POLL_STATUS = 1,
    CD_RECOVERY_STATE_CHECK_DISC = 2,
    CD_RECOVERY_STATE_WAIT_FOR_DISC = 3,
    CD_RECOVERY_STATE_WAIT_FOR_DRIVE = 4,
    CD_RECOVERY_STATE_CHECK_DISC_TYPE = 5,
    CD_RECOVERY_STATE_SET_MODE = 6,
    CD_RECOVERY_STATE_READ_DISC_ID = 7,
    CD_RECOVERY_STATE_WAIT_FOR_READ = 8,
} CdRecoveryState;

typedef enum CdRecoveryCommand
{
    CD_RECOVERY_COMMAND_SET_MODE = 0x20,
    CD_RECOVERY_COMMAND_READ_DISC_ID = 0x21,
    CD_RECOVERY_COMMAND_RETRY_READ = 0x22,
    CD_RECOVERY_COMMAND_COMPLETE = 0x23,
} CdRecoveryCommand;

typedef enum CdReconfigureState
{
    CD_RECONFIGURE_STATE_FLUSH = 0,
    CD_RECONFIGURE_STATE_SET_MODE = 1,
    CD_RECONFIGURE_STATE_SET_FILTER = 2,
    CD_RECONFIGURE_STATE_WAIT = 3,
} CdReconfigureState;

typedef enum CdReconfigureStep
{
    CD_RECONFIGURE_STEP_NONE = 0,
    CD_RECONFIGURE_STEP_SET_FILTER = 0x10,
    CD_RECONFIGURE_STEP_DEMUTE = 0x11,
    CD_RECONFIGURE_STEP_PAUSE = 0x12,
    CD_RECONFIGURE_STEP_COMPLETE = 0x13,
} CdReconfigureStep;

typedef enum CdSyncCommand
{
    CD_SYNC_COMMAND_NONE = 0,
    CD_SYNC_COMMAND_PAUSE = 1,
    CD_SYNC_COMMAND_AUDIO_PAUSE = 2,
    CD_SYNC_COMMAND_RESTORE_MODE = 3,
} CdSyncCommand;

typedef enum CdExecutionMode
{
    CD_EXECUTION_MODE_ASYNC = 0,
    CD_EXECUTION_MODE_COMMAND_THEN_READ = 1,
    CD_EXECUTION_MODE_READ_THEN_COMMAND = 2,
} CdExecutionMode;

typedef enum CdAudioMixMode
{
    CD_AUDIO_MIX_LEFT_TO_BOTH = 0,
    CD_AUDIO_MIX_BOTH_TO_LEFT = 1,
} CdAudioMixMode;

typedef enum CdQueueCommandError
{
    CD_QUEUE_ERROR_FULL = -1,
    CD_QUEUE_ERROR_INVALID_RESOURCE = -2,
    CD_QUEUE_ERROR_LOCKED = -3,
} CdQueueCommandError;

typedef enum CdErrorStatus
{
    CD_ERROR_STATUS_NONE = 0,
    CD_ERROR_STATUS_SYNC_ERROR = 1,
    CD_ERROR_STATUS_DISC_CHECK_PENDING = 2,
    CD_ERROR_STATUS_INVALID_DISC = 3,
    CD_ERROR_STATUS_NO_DISC = 4,
    CD_ERROR_STATUS_RETRIES_EXHAUSTED = 5,
} CdErrorStatus;

typedef enum CdDecompressOpcode
{
    CD_DECOMPRESS_REPEAT_NIBBLE = 0xF0,
    CD_DECOMPRESS_REPEAT_BYTE = 0xF1,
    CD_DECOMPRESS_REPEAT_NIBBLE_PAIR = 0xF2,
    CD_DECOMPRESS_REPEAT_PAIR = 0xF3,
    CD_DECOMPRESS_REPEAT_TRIPLET = 0xF4,
    CD_DECOMPRESS_INTERLEAVE_BYTE = 0xF5,
    CD_DECOMPRESS_INTERLEAVE_PAIR = 0xF6,
    CD_DECOMPRESS_INTERLEAVE_TRIPLET = 0xF7,
    CD_DECOMPRESS_ASCENDING_RUN = 0xF8,
    CD_DECOMPRESS_DESCENDING_RUN = 0xF9,
    CD_DECOMPRESS_STEPPED_RUN = 0xFA,
    CD_DECOMPRESS_PAIR_DELTA_RUN = 0xFB,
    CD_DECOMPRESS_COPY_12_BIT = 0xFC,
    CD_DECOMPRESS_COPY_8_BIT = 0xFD,
    CD_DECOMPRESS_COPY_NIBBLE = 0xFE,
    CD_DECOMPRESS_END = 0xFF,
} CdDecompressOpcode;

typedef union CdDecompressCursor
{
    u32 offset;
    u8* bytes;
} CdDecompressCursor;

// CD-ROM controller commands omitted from PSYQ libcd.h.
// Reference: https://psx-spx.consoledev.net/cdromdrive/
typedef enum CdControllerCommand
{
    CD_COMMAND_INIT = 0x0A,
    CD_COMMAND_SET_SESSION = 0x12,
    CD_COMMAND_UNUSED_17 = 0x17,
    CD_COMMAND_UNUSED_18 = 0x18,
    CD_COMMAND_TEST = 0x19,
    CD_COMMAND_GET_ID = 0x1A,
} CdControllerCommand;

typedef struct CdSystem
{
    CdStatusFlags status_flags;
    u8 audio_enabled;
    u8 playback_state;
    u8 pending_queue_count;
    u8 padding_0x7;
    u16 current_resource_index;
    u16 padding_0x0a;
    s32 current_data_size;
    s32 target_data_size;
    u8 sync_complete;
    u8 init_state;
    u8 current_command;
    u8 init_command;
    u8 retry_count;
    u8 retry_counter;
    u8 last_command;
    u8 padding_0x1b;
    u16 resource_index;
    u16 padding_0x1e;
    void* dst_buffer;
    CdCommandCallback callback;
    u32 read_remaining_bytes;
    u32 total_data_size;
    u8* current_write_ptr;
    CdCommandCallback transfer_callback;
    s32 queue_read_index;
    s32 queue_write_index;
    CdCommandQueue command_queue;
    u32 sector_header_buffer[3];
    s32 vsync_timestamp;
    u8 set_mode_param_blocking[4];
    u8 set_mode_param_async[4];
    CdlLOCRaw current_location;
    CdlLOCRaw recovery_read_position;
    u8 status_byte;
    u8 mode_flags;
    u8 u_162;
    u8 u_163;
    u32 u_164;
    CdlCB previous_sync_callback;
    CdlCB previous_ready_callback;
    u8 disc_validation_id[32];
    CdResourceEntry default_cd_resource;
} CdSystem;

typedef union CdQueueSystemCursor
{
    u32 address;
    CdSystem* system;
    CdCommandQueueItem* items;
    u8* bytes;
} CdQueueSystemCursor;

typedef struct
{
    u8 data_ready;
    u8 input_complete;
    u8 pad[2];
    u8* read_ptr;
    u8* write_ptr;
    s32 bytes_buffered;
    s32 wrap_overflow;
    s32 bytes_consumed;
    s32 deferred_sectors;
} CdStreamState;

typedef union CdStreamRelocation
{
    u32 address;
    s32 alignment_adjustment;
    u8* dst;
} CdStreamRelocation;

typedef union CdStreamCopyCursor
{
    u32 address;
    u8* bytes;
    u32* words;
} CdStreamCopyCursor;

typedef struct
{
    u8 u_0[0x38];
    DecDCToutCallbackHandler dec_dct_out_callback_handler;
    DrawSyncCallbackHandler draw_sync_callback_handler;
    u8 u_1[82];
    u8 audio_stream_state;
    u8 u_2[9];
    s8 mdec_busy;
} AudioSystem;

typedef struct SKCDPOSE_DAT
{
    CdResourceEntry resources[178];
    s8 unknown[45065];
} SKCDPOSE_DAT;

extern CdlCB g_cd_sync_callback_result;
extern CdlCB g_cd_ready_callback_result;
extern s32 g_cd_vsync_timestamp;
extern u8 g_cd_status_byte;
extern u8 g_cd_audio_enabled;
extern u8 g_cd_audio_ready;
extern u8 g_playback_state;
extern u32 g_cd_read_remaining_bytes;
extern s32 g_cd_resource_176;
extern u8 g_cd_status_byte_3;
extern u8 g_cd_init_state;
extern u8 g_cd_defer_data_ready;
extern u8 g_cd_pending_queue_count;
extern CdSystem g_cd_system;
extern const u8 g_disc_validation_id[21];
extern u8 g_gpu_mode;

#define CD_SYSTEM_ADDRESS 0x801ED800
#define CD_SYSTEM (*(struct CdSystem*)CD_SYSTEM_ADDRESS)
#define CD_SYSTEM_V (*(volatile CdSystem*)CD_SYSTEM_ADDRESS)
#define AUDIO_SYSTEM (*(AudioSystem*)0x801ED500)
#define g_default_cd_resource (*(CdResourceEntry*)0x801ED990)
#define CD_RESOURCE_ENTRIES ((CdResourceEntry*)0x801ED998)
#define CD_SCRATCHPAD_BUFFER ((CdResourceEntry*)0x1F800000)
#define CD_STREAM_STATE (*(CdStreamState*)0x1F800000)
#define CdControlF_1(cmd) ((int (*)(u_char))CdControlF)(cmd)
s32 cdrom_recover(void);
void cdrom_complete_command(u8 intr, u8* result);
void cdrom_handle_recovery_sync(u8 intr, u8* result);
void cdrom_handle_ready_intr(u8 intr, u8* result);
void cdrom_process_sector(s32 execution_mode);
void cdrom_run_command(u8 command, u8* sector_buffer, s32 execution_mode);
void cdrom_verify_disc(u8 interrupt, u8* result);
void cdrom_handle_sync_error(void);
void cdrom_set_audio_volume(u8 volume, s32 mix_mode);
s32 cdrom_decompress_data(u8** src_cursor, u8** dst_cursor, u8* src_end, u8* dst_end);
u8* cdrom_handle_stream_data(s32 bytes_transferred, u32 bytes_remaining);
void cdrom_decompress_buffer(u8* source, u8* destination);
void cdrom_clear_data_ready(u8* data_ready);
void cdrom_restore_callbacks(void);
s32 cdrom_enter_recovery_mode(void);

extern void akao_cmd_c1(u32 param_1, u32 param_2, u32 param_3);
extern void akao_cmd_99_9b_9d_9f(u_int param_1);
extern undefined FUN_80140d48(void);
extern void akao_cmd_e2(void);
extern void akao_play_sequence_blocking(AkaoSeqHeader* sequenceData, s32 waitForCompletion);
extern s32 akao_play_song(u8* param_1);
extern void akao_cmd_c0(undefined4 param_1, u_int param_2);

/**
 * @brief Initializes the CD-ROM hardware and command system.
 *
 * Saves and clears the libcd callbacks, resets command state, and configures
 * double-speed reads with 2340-byte sectors. Blocks until the drive is ready.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/DBYkw
 */
void cdrom_init(void)
{
    s32 queue_end_marker;
    s32 queue_count;
    volatile CdCommandQueueItem* queue_base;
    CdResourceEntry* scratchpad_addr;
    CdStatusFlags* status_flags;
    s32 cd_result;

    while (CdInit() == 0);

    CdSetDebug(0);

    g_cd_sync_callback_result = CdSyncCallback(NULL);
    g_cd_ready_callback_result = CdReadyCallback(NULL);

    status_flags = &CD_SYSTEM.status_flags;

    queue_count = CD_COMMAND_QUEUE_SIZE - 1;
    scratchpad_addr = CD_SCRATCHPAD_BUFFER;

    queue_end_marker = -1;

    // The target addresses each queue item from a base four slots earlier.
    queue_base = &CD_SYSTEM.command_queue.items[CD_COMMAND_QUEUE_SIZE - CD_COMMAND_QUEUE_BASE_OFFSET - 1];

    CD_SYSTEM.resource_index = CD_RESOURCE_INDEX_INVALID;

    CD_SYSTEM.audio_enabled = 0;
    CD_SYSTEM.playback_state = 0;
    CD_SYSTEM.transfer_callback = NULL;
    CD_SYSTEM.pending_queue_count = 0;
    CD_SYSTEM.current_resource_index = 0;
    CD_SYSTEM.current_data_size = 0;
    CD_SYSTEM.target_data_size = 0;
    CD_SYSTEM.sync_complete = 0;
    CD_SYSTEM.init_state = 0;
    CD_SYSTEM.current_command = 0;
    CD_SYSTEM.init_command = 0;
    CD_SYSTEM.retry_count = 0;
    CD_SYSTEM.retry_counter = 0;
    CD_SYSTEM.last_command = 0;
    CD_SYSTEM.dst_buffer = 0;
    CD_SYSTEM.callback = 0;
    CD_SYSTEM.queue_read_index = 0;
    CD_SYSTEM.queue_write_index = 0;

    // Bit 7 is externally managed and intentionally preserved.
    status_flags->word &= ~CD_STATUS_SYNC_ERROR;
    status_flags->word &= ~CD_STATUS_INVALID_DISC;
    status_flags->word &= ~CD_STATUS_NO_DISC;
    status_flags->word &= ~CD_STATUS_RECOVERY_PENDING;
    status_flags->word &= ~CD_STATUS_COMMAND_ACTIVE;
    status_flags->word &= ~CD_STATUS_QUEUE_LOCK;
    status_flags->word &= ~CD_STATUS_SUPPRESS_IDLE_POLL;

    status_flags->bytes.defer_data_ready = 0;
    status_flags->bytes.data_ready_pending = 0;
    status_flags->bytes.retry_exhausted = 0;

    while (queue_count != queue_end_marker)
    {
        queue_base[CD_COMMAND_QUEUE_BASE_OFFSET].command = 0;
        queue_base[CD_COMMAND_QUEUE_BASE_OFFSET].resource_index = 0;
        queue_base[CD_COMMAND_QUEUE_BASE_OFFSET].dst_buffer = scratchpad_addr;
        queue_base[CD_COMMAND_QUEUE_BASE_OFFSET].entry = scratchpad_addr;
        queue_base[CD_COMMAND_QUEUE_BASE_OFFSET].callback = 0;
        queue_base--;
        queue_count--;
    }

    CD_SYSTEM.set_mode_param_blocking[0] = (CdlModeSpeed | CdlModeSize1);
    CD_SYSTEM.set_mode_param_blocking[1] = 0;
    CD_SYSTEM.set_mode_param_blocking[2] = 0;
    CD_SYSTEM.set_mode_param_blocking[3] = 0;

    while (CdControlB(CdlNop, NULL, &CD_SYSTEM.status_byte) == 0);

    // An open tray requires a blocking readiness check before setting the mode.
    if ((g_cd_status_byte & CdlStatShellOpen) != 0)
    {
        cd_result = CdDiskReady(1);

        while (cd_result != CdlComplete)
        {
            cd_result = CdDiskReady(0);
        }
    }

    while (CdControlB(CdlSetmode, CD_SYSTEM.set_mode_param_blocking, NULL) == 0);

    // Start timeout tracking from the current frame.
    g_cd_vsync_timestamp = VSync(-1);
}

/**
 * @brief Stops CD-ROM activity and clears the command system.
 *
 * Stops active CD audio, clears libcd callbacks, blocks until the drive pauses,
 * then resets and flushes the command queue.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/M39vT
 */
void cdrom_stop(void)
{
    CdSystem* cd_system;

    cd_system = &CD_SYSTEM;

    if (g_cd_audio_enabled != 0)
    {
        cdrom_reset();
    }

    cd_system->status_flags.word &= ~CD_STATUS_QUEUE_LOCK;

    CdSyncCallback(NULL);
    CdReadyCallback(NULL);

    while (CdControlB(CdlPause, NULL, NULL) == 0);

    CD_SYSTEM.resource_index = CD_RESOURCE_INDEX_INVALID;
    CD_SYSTEM.pending_queue_count = 0;
    CD_SYSTEM.current_resource_index = 0;
    CD_SYSTEM.current_data_size = 0;
    CD_SYSTEM.target_data_size = 0;
    CD_SYSTEM.playback_state = 0;
    CD_SYSTEM.transfer_callback = NULL;
    CD_SYSTEM.current_command = 0;
    CD_SYSTEM.init_command = 0;
    CD_SYSTEM.retry_count = 0;
    CD_SYSTEM.retry_counter = 0;
    CD_SYSTEM.last_command = 0;
    CD_SYSTEM.dst_buffer = 0;
    CD_SYSTEM.callback = 0;
    CD_SYSTEM.status_flags.word &= ~CD_STATUS_COMMAND_ACTIVE;
    CD_SYSTEM.vsync_timestamp = VSync(-1);
    CD_SYSTEM.status_flags.bytes.defer_data_ready = 0;
    CD_SYSTEM.status_flags.bytes.data_ready_pending = 0;
    CD_SYSTEM.queue_read_index = 0;
    CD_SYSTEM.queue_write_index = 0;

    CdFlush();
}

/**
 * @brief Streams and decompresses a CD resource into memory.
 *
 * Decompresses sectors as they arrive in the shared ring buffer, compacting
 * unread input when the buffer wraps.
 *
 * @param resource_index Resource table index.
 * @param destination   Destination address for decompressed data.
 *
 * @return Number of decompressed bytes written.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/SvWOg
 */
s32 cdrom_stream(s32 resource_index, u32 destination)
{
    s32 unprocessed_bytes;
    u8* relocation_dst;
    s32 bytes_buffered;
    s32 bytes_consumed;
    u8* previous_read_ptr;
    s32 copy_size;
    s32 overflow_size;
    s32 timestamp;
    s32 remaining_size;
    s32* relocation_src;
    u8* decompress_end;
    CdStreamState* stream_state;
    CdStreamState* active_stream;
    u32 destination_start;
    s32 alignment;
    s32 sentinel;

    while (cdrom_process_state() != 0)
    {
        VSync(0);
    }

    destination_start = destination;
    stream_state = &CD_STREAM_STATE;

    stream_state->deferred_sectors = 0;
    stream_state->data_ready = 0U;
    stream_state->input_complete = 0U;
    stream_state->bytes_consumed = 0;

    remaining_size = cdrom_queue_command(CdlReadN, resource_index, NULL, &cdrom_handle_stream_data) - 1;
    timestamp = VSync(-1);

    active_stream = &CD_STREAM_STATE;

    while (TRUE)
    {
        if (VSync(-1) < (timestamp + CD_STREAM_TIMEOUT_FRAMES))
        {
            if (active_stream->data_ready != 1)
            {
                continue;
            }

            do
            {
                bytes_buffered = active_stream->bytes_buffered;

                // Retain a guard region until the final input chunk is buffered.
                if (bytes_buffered < remaining_size)
                {
                    decompress_end = (active_stream->read_ptr + bytes_buffered) - CD_STREAM_DECOMPRESS_GUARD_SIZE;
                }
                else
                {
                    decompress_end = active_stream->read_ptr + remaining_size;
                }

                if (cdrom_decompress_data(&CD_STREAM_STATE.write_ptr, (u8**)&destination, decompress_end, CD_DECOMPRESS_UNBOUNDED_END) == 0)
                {
                    return destination - destination_start;
                }
            } while (bytes_buffered != CD_STREAM_STATE.bytes_buffered);

            bytes_consumed = active_stream->write_ptr - active_stream->read_ptr;
            active_stream->bytes_consumed = bytes_consumed;
            cdrom_clear_data_ready(&CD_STREAM_STATE.data_ready);
            remaining_size -= bytes_consumed;

            if (active_stream->input_complete != 1)
            {
                timestamp = VSync(-1);
                continue;
            }

            if (active_stream->wrap_overflow != 0)
            {
                // Compact unread bytes so wrapped input remains contiguous.
                overflow_size = active_stream->wrap_overflow;
                unprocessed_bytes = active_stream->bytes_buffered - bytes_consumed;
                alignment = unprocessed_bytes & CD_STREAM_COPY_WORD_MASK;
                relocation_dst = (u8*)(CD_STREAM_BUFFER_END - unprocessed_bytes);
                previous_read_ptr = active_stream->read_ptr;

                // Keep logical pointers at the data start; only the copy includes alignment padding.
                copy_size = CD_STREAM_COPY_WORD_SIZE - alignment;
                active_stream->write_ptr = relocation_dst;
                active_stream->read_ptr = relocation_dst;
                copy_size &= CD_STREAM_COPY_WORD_MASK;
                alignment = unprocessed_bytes + CD_STREAM_COPY_WORD_MASK;

                relocation_dst -= copy_size;
                relocation_src = (s32*)((previous_read_ptr + bytes_consumed) - copy_size);

                active_stream->bytes_buffered = overflow_size + unprocessed_bytes;
                copy_size = alignment;

                if (copy_size < 0)
                {
                    copy_size = unprocessed_bytes + 6;
                }

                // Round the byte length up to the number of words copied.
                unprocessed_bytes = copy_size >> 2;
                unprocessed_bytes--;

                if (unprocessed_bytes != -1)
                {
                    sentinel = -1;
                    while (unprocessed_bytes != sentinel)
                    {
                        *(s32*)relocation_dst = *relocation_src++;
                        relocation_dst += 4;
                        unprocessed_bytes--;
                    }
                }
            }
            else
            {
                active_stream->read_ptr += bytes_consumed;
                active_stream->bytes_buffered -= bytes_consumed;
            }

            // Publish the updated buffer state to the transfer callback.
            *(volatile u8*)active_stream = 1;

            timestamp = VSync(-1);
            continue;
        }

        cdrom_process_state();
        timestamp = VSync(-1);
    }
}

/**
 * @brief Streams decompressed CD data through caller-provided buffers.
 *
 * A capacity of -1 selects direct output. Fixed-size output is staged and copied
 * across chunks while preserving the 4 KiB LZ history when staging fills.
 *
 * @param resource_index Resource table index.
 * @param get_buffer     Returns the next output buffer and its capacity.
 * @param chunk_done     Called after each completed or final chunk.
 *
 * @see decomp.me: (99.78%) https://decomp.me/scratch/aZWx6
 */
void cdrom_stream_chunked(u16 resource_index, CdStreamGetBufferCallback get_buffer, CdStreamChunkDoneCallback chunk_done)
{
    s32 timestamp;
    u8 src_byte;
    s32 decompress_result;
    u32 src_word;
    s32 loop_count;
    u8* decompress_end;
    u32 alignment_check;
    u8* src_ptr;
    s32 total_bytes_delivered;
    s32 chunk_index;
    s32 chunk_bytes_remaining;
    u8* destination;
    u8* staging_write_ptr;
    u8* staging_end;
    u8* destination_end;
    s32 remaining_size;
    s32 direct_mode;
    s32 bytes_buffered;
    s32 staging_bytes_produced;
    s32 alignment;
    s32 copy_size;
    s32 loop_end;
    CdStreamRelocation relocation;
    u8* previous_read_ptr;
    u32 wrap_overflow;
    volatile CdStreamState* scratchpad;
    CdStreamState* stream_state;
    s32 count_sentinel;
    u8** destination_ptr;
    u8** staging_write_ptr_ref;
    CdStreamCopyCursor alignment_cursor;

    scratchpad = &CD_STREAM_STATE;
    scratchpad->deferred_sectors = 0;
    scratchpad->data_ready = 0;
    scratchpad->input_complete = 0;

    remaining_size = cdrom_queue_command(CdlReadN, resource_index, NULL, cdrom_handle_stream_data) - 1;

    total_bytes_delivered = 0;
    chunk_index = 0;

    destination = get_buffer(0, &chunk_bytes_remaining);

    if (chunk_bytes_remaining == -1)
    {
        destination_end = CD_DECOMPRESS_UNBOUNDED_END;
        direct_mode = CD_STREAM_DIRECT_MODE;
    }
    else
    {
        destination_end = destination + chunk_bytes_remaining - CD_STREAM_CHUNK_GUARD_SIZE;
        direct_mode = 0;
    }

    src_ptr = CD_STREAM_STAGING_START;
    staging_write_ptr = src_ptr;
    staging_end = CD_STREAM_STAGING_END;

    timestamp = VSync(-1);
    stream_state = &CD_STREAM_STATE;
    count_sentinel = -1;
    destination_ptr = &destination;

    while (TRUE)
    {

        if (VSync(-1) < timestamp + CD_STREAM_TIMEOUT_FRAMES)
        {

            if (((volatile CdStreamState*)stream_state)->data_ready != 1)
            {
                continue;
            }

            do
            {
                bytes_buffered = stream_state->bytes_buffered;

                // Retain a guard region until the final input chunk is buffered.
                if (bytes_buffered < remaining_size)
                {
                    decompress_end = (stream_state->read_ptr + bytes_buffered) - CD_STREAM_DECOMPRESS_GUARD_SIZE;
                }
                else
                {
                    decompress_end = stream_state->read_ptr + remaining_size;
                }

                if (direct_mode != 0 && destination < destination_end)
                {
                    cdrom_decompress_data(&CD_STREAM_STATE.write_ptr, &destination, decompress_end, destination_end);
                    continue;
                }

                src_ptr = staging_write_ptr;
                decompress_result = cdrom_decompress_data(&CD_STREAM_STATE.write_ptr, &staging_write_ptr, decompress_end, staging_end);

                staging_bytes_produced = staging_write_ptr - src_ptr;

                while (staging_bytes_produced != 0)
                {

                    if ((staging_bytes_produced < chunk_bytes_remaining) || (chunk_bytes_remaining == count_sentinel))
                    {
                        total_bytes_delivered += staging_bytes_produced;
                        chunk_bytes_remaining -= staging_bytes_produced;

                        // Align the destination before copying whole words.
                        alignment_cursor.bytes = destination;
                        loop_count = alignment_cursor.address & CD_STREAM_COPY_WORD_MASK;
                        if ((loop_count != 0) && (loop_count < staging_bytes_produced))
                        {
                            staging_bytes_produced -= loop_count;
                            loop_count--;
                            if (loop_count != count_sentinel)
                            {
                                loop_end = -1;
                                for (;;)
                                {
                                    u8* dest;
                                    src_byte = *src_ptr++;
                                    dest = *destination_ptr;
                                    *dest = src_byte;
                                    *destination_ptr = dest + 1;
                                    loop_count--;
                                    if (loop_count == loop_end)
                                    {
                                        break;
                                    }
                                }
                            }
                        }

                        alignment_cursor.bytes = src_ptr;
                        alignment_check = alignment_cursor.address & CD_STREAM_COPY_WORD_MASK;
                        if (alignment_check == 0)
                        {
                            loop_count = staging_bytes_produced >> 2;
                            staging_bytes_produced -= loop_count * CD_STREAM_COPY_WORD_SIZE;
                            loop_count--;
                            if (loop_count != count_sentinel)
                            {
                                loop_end = -1;
                                for (;;)
                                {
                                    CdStreamCopyCursor src_cursor;
                                    CdStreamCopyCursor dst_cursor;

                                    src_cursor.bytes = src_ptr;
                                    src_word = *src_cursor.words;
                                    src_cursor.words++;
                                    src_ptr = src_cursor.bytes;

                                    dst_cursor.bytes = *destination_ptr;
                                    *dst_cursor.words = src_word;
                                    dst_cursor.words++;
                                    *destination_ptr = dst_cursor.bytes;
                                    loop_count--;
                                    if (loop_count == loop_end)
                                    {
                                        break;
                                    }
                                }
                            }
                        }

                        staging_bytes_produced--;
                        if (staging_bytes_produced != count_sentinel)
                        {
                            loop_end = -1;
                            for (;;)
                            {
                                u8* dest;
                                src_byte = *src_ptr++;
                                dest = *destination_ptr;
                                *dest = src_byte;
                                *destination_ptr = dest + 1;
                                staging_bytes_produced--;
                                if (staging_bytes_produced == loop_end)
                                {
                                    break;
                                }
                            }
                        }

                        break;
                    }

                    // Fill the current chunk before requesting the next one.
                    staging_bytes_produced -= chunk_bytes_remaining;
                    total_bytes_delivered += chunk_bytes_remaining;
                    chunk_bytes_remaining--;
                    if (chunk_bytes_remaining != count_sentinel)
                    {
                        loop_end = -1;
                        for (;;)
                        {
                            u8* dest = *destination_ptr;
                            *dest = *src_ptr;
                            *destination_ptr = dest + 1;
                            src_ptr++;
                            chunk_bytes_remaining--;
                            if (chunk_bytes_remaining == loop_end)
                            {
                                break;
                            }
                        }
                    }

                    if (staging_bytes_produced > 0 || decompress_result != 0)
                    {
                        loop_end = chunk_index++;
                        chunk_done(loop_end);
                        destination = get_buffer(total_bytes_delivered, &chunk_bytes_remaining);
                    }
                }

                if (decompress_result != 0)
                {
                    // Preserve the LZ history before reusing the staging buffer.
                    s32 loop_sentinel;
                    staging_write_ptr = CD_STREAM_STAGING_START;
                    src_ptr = src_ptr - CD_STREAM_LZ_WINDOW_SIZE;
                    staging_bytes_produced = CD_STREAM_LZ_WINDOW_SIZE - 1;
                    staging_write_ptr_ref = &staging_write_ptr;
                    loop_sentinel = -1;

                    do
                    {
                        u8* dest;
                        src_byte = *src_ptr++;
                        dest = *staging_write_ptr_ref;
                        staging_bytes_produced--;
                        *dest = src_byte;
                        *staging_write_ptr_ref = dest + 1;
                    } while (staging_bytes_produced != loop_sentinel);

                    continue;
                }

                chunk_done(chunk_index);
                return;

            } while (bytes_buffered != CD_STREAM_STATE.bytes_buffered);

            bytes_buffered = stream_state->write_ptr - stream_state->read_ptr;
            previous_read_ptr = stream_state->read_ptr;

            stream_state->data_ready = 0;
            stream_state->bytes_consumed = bytes_buffered;
            remaining_size -= bytes_buffered;

            if (stream_state->input_complete != 1)
            {
                timestamp = VSync(-1);
                continue;
            }

            wrap_overflow = stream_state->wrap_overflow;

            if (wrap_overflow != 0)
            {
                // Compact unread bytes so wrapped input remains contiguous.
                staging_bytes_produced = stream_state->bytes_buffered - bytes_buffered;
                alignment = (staging_bytes_produced & CD_STREAM_COPY_WORD_MASK);
                relocation.alignment_adjustment = CD_STREAM_COPY_WORD_SIZE - alignment;
                copy_size = relocation.alignment_adjustment & CD_STREAM_COPY_WORD_MASK;
                loop_count = CD_STREAM_BUFFER_END;
                relocation.address = loop_count - staging_bytes_produced;
                previous_read_ptr = (previous_read_ptr + bytes_buffered) - copy_size;

                stream_state->write_ptr = relocation.dst;
                stream_state->read_ptr = relocation.dst;
                relocation.dst = relocation.dst - copy_size;

                alignment = staging_bytes_produced + CD_STREAM_COPY_WORD_MASK;
                stream_state->bytes_buffered = wrap_overflow + staging_bytes_produced;

                if (alignment < 0)
                {
                    alignment = staging_bytes_produced + 6;
                }

                staging_bytes_produced = (alignment >> 2);
                staging_bytes_produced--;
                if (staging_bytes_produced != count_sentinel)
                {
                    s32 wrap_loop_end = -1;
                    for (;;)
                    {
                        CdStreamCopyCursor relocation_cursor;
                        CdStreamCopyCursor previous_cursor;

                        relocation_cursor.bytes = relocation.dst;
                        previous_cursor.bytes = previous_read_ptr;
                        *relocation_cursor.words = *previous_cursor.words;
                        previous_cursor.words++;
                        relocation_cursor.words++;
                        previous_read_ptr = previous_cursor.bytes;
                        relocation.dst = relocation_cursor.bytes;
                        staging_bytes_produced--;
                        if (staging_bytes_produced == wrap_loop_end)
                        {
                            break;
                        }
                    }
                }
            }
            else
            {
                stream_state->read_ptr = previous_read_ptr + bytes_buffered;
                stream_state->bytes_buffered -= bytes_buffered;
            }

            // Publish the updated buffer state to the transfer callback.
            *(volatile u8*)stream_state = 1;
            timestamp = VSync(-1);
            continue;
        }

        cdrom_process_state();
        timestamp = VSync(-1);
    }
}

/**
 * @brief Queues a CD-ROM command and starts processing when idle.
 *
 * @param command        CD-ROM command.
 * @param resource_index Resource table index, or CD_RESOURCE_INDEX_DEFAULT.
 * @param dst_buffer     Destination for read data.
 * @param callback       Completion callback.
 *
 * @return Resource size, or a negative CdQueueCommandError.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/izXP3
 */
s32 cdrom_queue_command(u8 command, u16 resource_index, void* dst_buffer, CdCommandCallback callback)
{
    s32 timestamp;
    s32 write_index;
    u32 status_flags;
    s32 data_size;
    u8 active_command;
    CdResourceEntry* resource_entry;

    if (g_cd_system.status_flags.word & CD_STATUS_QUEUE_LOCK)
    {
        return CD_QUEUE_ERROR_LOCKED;
    }

    if (resource_index == CD_RESOURCE_INDEX_DEFAULT)
    {
        resource_entry = &g_default_cd_resource;
    }
    else
    {
        resource_entry = &CD_RESOURCE_ENTRIES[resource_index];
    }

    // Suppress only consecutive duplicate commands while the drive is busy.
    if ((CD_SYSTEM_V.current_command == 0 && CD_SYSTEM_V.init_command == 0) || (CD_SYSTEM.last_command != command) ||
        (CD_SYSTEM.resource_index != resource_index) || (CD_SYSTEM.dst_buffer != dst_buffer) || (CD_SYSTEM.callback != callback))
    {
        if ((resource_entry->location.raw == 0) || (resource_entry->data_size == 0))
        {
            return CD_QUEUE_ERROR_INVALID_RESOURCE;
        }

        write_index = CD_SYSTEM.queue_write_index;

        if (CD_SYSTEM.queue_read_index == ((write_index + 1) & CD_COMMAND_QUEUE_MASK))
        {
            return CD_QUEUE_ERROR_FULL;
        }

        CD_SYSTEM.command_queue.items[CD_SYSTEM.queue_write_index].command = command;
        CD_SYSTEM.last_command = command;

        CD_SYSTEM.command_queue.items[CD_SYSTEM.queue_write_index].resource_index = resource_index;
        CD_SYSTEM.resource_index = resource_index;

        CD_SYSTEM.command_queue.items[CD_SYSTEM.queue_write_index].entry = resource_entry;
        CD_SYSTEM.command_queue.items[CD_SYSTEM.queue_write_index].dst_buffer = dst_buffer;

        CD_SYSTEM.dst_buffer = dst_buffer;

        CD_SYSTEM.command_queue.items[CD_SYSTEM.queue_write_index].callback = callback;
        CD_SYSTEM.callback = callback;

        CD_SYSTEM.queue_write_index = (CD_SYSTEM.queue_write_index + 1) & CD_COMMAND_QUEUE_MASK;

        timestamp = VSync(-1);

        active_command = CD_SYSTEM.current_command;

        if ((active_command != 0) || (CD_SYSTEM.init_command != 0))
        {
            return resource_entry->data_size;
        }

        status_flags = CD_SYSTEM.status_flags.word;

        if (!(status_flags & CD_STATUS_RECOVERY_MASK))
        {
            // Start queue processing through the normal sync callback path.
            CD_SYSTEM.vsync_timestamp = timestamp;
            CD_SYSTEM.pending_queue_count = 1;
            CD_SYSTEM.current_resource_index = resource_index;
            data_size = resource_entry->data_size;
            CD_SYSTEM.current_command = CdlNop;
            CD_SYSTEM.status_flags.word = (status_flags | CD_STATUS_COMMAND_ACTIVE);
            CD_SYSTEM.playback_state = 0;
            CD_SYSTEM.transfer_callback = NULL;
            CD_SYSTEM.target_data_size = data_size;
            CD_SYSTEM.current_data_size = data_size;

            CdSyncCallback(cdrom_complete_command);
            CdSync(0, NULL);
            CdControlF(CdlNop, NULL);
        }
    }

    return resource_entry->data_size;
}

/**
 * @brief Advances the CD command queue and drive-recovery state machine.
 *
 * @return Number of queued commands, or zero while idle or recovering.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/xxcgW
 */
u32 cdrom_process_state(void)
{
    s32 control_result;
    s32 saw_sync_completion;
    u32 pending_count;
    u8 current_command;
    u8 cd_command;
    u8* command_params;
    u32 read_index;
    s32 recovery_command;
    u8 recovery_state;
    u32 status_flags;
    CdSystem* cd_system;

    if (CD_SYSTEM.status_flags.word & CD_STATUS_RECOVERY_PENDING)
    {
        return 0;
    }

    recovery_state = CD_RECOVERY_STATE_POLL_STATUS;

    // Error flags suspend queue dispatch until drive recovery completes.
    if (CD_SYSTEM.status_flags.word & CD_STATUS_ERROR_MASK)
    {
        read_index = CD_SYSTEM.queue_read_index;
        pending_count = (CD_SYSTEM.queue_write_index - read_index) & CD_COMMAND_QUEUE_MASK;

        CD_SYSTEM.pending_queue_count = pending_count;

        if (CD_SYSTEM.init_state == 0)
        {
            CD_SYSTEM.init_state = recovery_state;

            if (pending_count != 0)
            {
                CdQueueSystemCursor queue_cursor;
                queue_cursor.system = &CD_SYSTEM;
                queue_cursor.bytes += read_index << 4;
                CD_SYSTEM.current_resource_index = queue_cursor.system->command_queue.items[0].resource_index;
                CD_SYSTEM.current_data_size = queue_cursor.system->command_queue.items[0].entry->data_size;
                CD_SYSTEM.target_data_size = CD_SYSTEM.read_remaining_bytes;
            }

            if (CD_SYSTEM.audio_enabled != 0)
            {
                if (g_cd_audio_ready != 0)
                {
                    akao_cmd_99_9b_9d_9f(3);
                }
            }

            if (CD_SYSTEM.transfer_callback != NULL)
            {
                CD_SYSTEM.playback_state = 1;
            }
            else
            {
                CD_SYSTEM.playback_state = 0;
            }

            g_cd_status_byte_3 = 0;
        }

        if (VSync(-1) >= (CD_SYSTEM.vsync_timestamp + CD_STATUS_POLL_FRAMES))
        {
            if (CD_SYSTEM.init_state != CD_RECOVERY_STATE_WAIT_FOR_READ)
            {
                CD_SYSTEM.vsync_timestamp = VSync(-1);
            }

            control_result = CdControlB(CdlNop, NULL, &CD_SYSTEM.status_byte);

            if (!(CD_SYSTEM.status_byte & CdlStatShellOpen) && (control_result != 0))
            {
                switch (CD_SYSTEM.init_state)
                {
                case CD_RECOVERY_STATE_POLL_STATUS:
                    CD_SYSTEM.init_state = CD_RECOVERY_STATE_CHECK_DISC;
                    CD_SYSTEM.status_flags.word = (CD_SYSTEM.status_flags.word & ~CD_STATUS_SYNC_ERROR) | CD_STATUS_INVALID_DISC | CD_STATUS_NO_DISC;
                    /* fallthrough */

                case CD_RECOVERY_STATE_CHECK_DISC:
                    control_result = CdControlB(CdlGetTN, NULL, &CD_SYSTEM.status_byte);
                    if ((CD_SYSTEM.status_byte & CdlStatStandby) && (control_result != 0))
                    {
                        CD_SYSTEM.init_state = CD_RECOVERY_STATE_WAIT_FOR_DISC;
                        CD_SYSTEM.retry_counter = 0;
                    }
                    break;

                case CD_RECOVERY_STATE_WAIT_FOR_DISC:
                    if (CdDiskReady(1) == CdlComplete)
                    {
                        g_cd_init_state = CD_RECOVERY_STATE_WAIT_FOR_DRIVE;
                    }
                    else
                    {
                        u8 retry_count = CD_SYSTEM.retry_counter + 1;

                        // This recovery path advances the retry counter twice per poll.
                        CD_SYSTEM.retry_counter = retry_count + 1;
                        if (retry_count >= CD_DISC_READY_RETRY_LIMIT)
                        {
                            CD_SYSTEM.init_state = CD_RECOVERY_STATE_WAIT_FOR_DRIVE;
                        }
                    }
                    break;

                case CD_RECOVERY_STATE_WAIT_FOR_DRIVE:
                    control_result = CdDiskReady(0);
                    if (control_result == CdlComplete)
                    {
                        g_cd_init_state = CD_RECOVERY_STATE_CHECK_DISC_TYPE;
                    }
                    else if (control_result == CdlStatShellOpen)
                    {
                        g_cd_init_state = CD_RECOVERY_STATE_POLL_STATUS;
                    }
                    else
                    {
                        g_cd_init_state = CD_RECOVERY_STATE_CHECK_DISC_TYPE;
                    }
                    break;

                case CD_RECOVERY_STATE_CHECK_DISC_TYPE:
                    control_result = CdGetDiskType();
                    switch (control_result)
                    {
                    case CdlStatNoDisk:
                        CD_SYSTEM.init_state = CD_INIT_STATE_ERROR_PAUSE;
                        CD_SYSTEM.status_flags.word &= ~CD_STATUS_INVALID_DISC;
                        break;

                    case CdlOtherFormat:
                        CdDiskReady(0);
                        CdGetDiskType();
                        /* fallthrough */

                    case CdlCdromFormat:
                        CD_SYSTEM.init_state = CD_RECOVERY_STATE_SET_MODE;
                        CD_SYSTEM.vsync_timestamp -= CD_STATUS_POLL_FRAMES;
                        break;
                    }
                    break;

                case CD_RECOVERY_STATE_SET_MODE:
                    CD_SYSTEM.set_mode_param_async[0] = (CdlModeSpeed | CdlModeSize1);
                    CD_SYSTEM.set_mode_param_async[1] = 0;
                    CD_SYSTEM.set_mode_param_async[2] = 0;
                    CD_SYSTEM.set_mode_param_async[3] = 0;
                    CdSyncCallback(cdrom_handle_recovery_sync);
                    CdReadyCallback(NULL);
                    CD_SYSTEM_V.init_command = CD_RECOVERY_COMMAND_SET_MODE;
                    CdControlF(CdlSetmode, CD_SYSTEM.set_mode_param_async);
                    CD_SYSTEM.vsync_timestamp -= CD_STATUS_POLL_FRAMES - CD_SET_MODE_DELAY_FRAMES;
                    break;

                case CD_RECOVERY_STATE_READ_DISC_ID:
                    CD_SYSTEM.recovery_read_position.raw = g_cd_resource_176;
                    CD_SYSTEM.status_flags.word |= CD_STATUS_COMMAND_ACTIVE;
                    CdSyncCallback(cdrom_handle_recovery_sync);
                    CdReadyCallback(cdrom_verify_disc);
                    CD_SYSTEM.init_command = CD_RECOVERY_COMMAND_READ_DISC_ID;
                    CD_SYSTEM.init_state = CD_RECOVERY_STATE_WAIT_FOR_READ;
                    CdControlF(CdlReadN, CD_SYSTEM.recovery_read_position.bytes);
                    CD_SYSTEM.vsync_timestamp -= CD_STATUS_POLL_FRAMES;
                    break;

                case CD_RECOVERY_STATE_WAIT_FOR_READ:
                    if (CD_SYSTEM_V.sync_complete == 1)
                    {
                        CD_SYSTEM.vsync_timestamp = VSync(-1);
                        CD_SYSTEM_V.sync_complete = 0;
                    }
                    else if (VSync(-1) >= (CD_SYSTEM.vsync_timestamp + CD_RECOVERY_READ_TIMEOUT_FRAMES))
                    {
                        recovery_command = CD_SYSTEM_V.init_command;

                        // Retry Pause or Setmode; otherwise restart the validation read.
                        if (recovery_command != CD_RECOVERY_COMMAND_RETRY_READ)
                        {
                            if (recovery_command >= CD_RECOVERY_COMMAND_COMPLETE)
                            {
                                if (recovery_command == CD_RECOVERY_COMMAND_COMPLETE)
                                {
                                    goto retry_set_mode;
                                }
                            }

                            CdSyncCallback(cdrom_handle_recovery_sync);
                            CdReadyCallback(cdrom_verify_disc);
                            CD_SYSTEM.init_command = CD_RECOVERY_COMMAND_READ_DISC_ID;
                            cd_command = CdlReadN;
                            command_params = CD_SYSTEM.recovery_read_position.bytes;
                        }
                        else
                        {
                            CdSyncCallback(cdrom_handle_recovery_sync);
                            cd_command = CdlPause;
                            command_params = NULL;
                        }
                        goto execute_command;

                    retry_set_mode:
                        CdSyncCallback(cdrom_handle_recovery_sync);
                        cd_command = CdlSetmode;
                        command_params = CD_SYSTEM.set_mode_param_blocking;

                    execute_command:
                        CdControlF(cd_command, command_params);
                        CD_SYSTEM.vsync_timestamp -= CD_STATUS_POLL_FRAMES;
                    }
                    break;

                case CD_INIT_STATE_ERROR_PAUSE:
                    while (CdControlB(CdlStop, NULL, NULL) == 0);
                    g_cd_init_state = CD_INIT_STATE_RETRY_READ;
                    break;
                }
            }
            else
            {
                cd_system = &CD_SYSTEM;
                if (g_cd_init_state >= CD_RECOVERY_STATE_SET_MODE)
                {
                    cd_system->status_flags.word &= ~CD_STATUS_COMMAND_ACTIVE;
                    CdSyncCallback(NULL);
                    CdReadyCallback(NULL);
                    while (CdControlB(CdlPause, NULL, NULL) == 0);
                    CD_SYSTEM_V.init_command = 0;
                }
                CD_SYSTEM.init_state = CD_RECOVERY_STATE_POLL_STATUS;
                status_flags = (CD_SYSTEM.status_flags.word | CD_STATUS_SYNC_ERROR) & ~CD_STATUS_INVALID_DISC;
                CD_SYSTEM.status_flags.word = status_flags & ~CD_STATUS_NO_DISC;
            }
        }
    }
    else
    {
        saw_sync_completion = 0;
        current_command = CD_SYSTEM.current_command;

        if ((current_command != 0) || (CD_SYSTEM.init_command != 0))
        {
            // Resample until no completion arrives while the queue state is read.
            while (TRUE)
            {
                if (CD_SYSTEM_V.sync_complete == 1)
                {
                    saw_sync_completion = 1;
                    CD_SYSTEM.sync_complete = 0;
                }
                read_index = CD_SYSTEM.queue_read_index;

                pending_count = (CD_SYSTEM.queue_write_index - read_index) & CD_COMMAND_QUEUE_MASK;

                if (pending_count != 0)
                {
                    CD_SYSTEM.current_resource_index = CD_SYSTEM.command_queue.items[CD_SYSTEM.queue_read_index].resource_index;
                    CD_SYSTEM.current_data_size = CD_SYSTEM.command_queue.items[CD_SYSTEM.queue_read_index].entry->data_size;
                    CD_SYSTEM.target_data_size = CD_SYSTEM.read_remaining_bytes;
                }

                if (CD_SYSTEM.sync_complete == 0)
                {
                    break;
                }
            }

            if (saw_sync_completion == 0)
            {
                if (VSync(-1) >= (CD_SYSTEM.vsync_timestamp + CD_ACTIVE_COMMAND_TIMEOUT_FRAMES))
                {
                    if (CD_SYSTEM.init_command == 0)
                    {
                        CD_SYSTEM_V.current_command = CdlNop;

                        if (CD_SYSTEM.transfer_callback != NULL)
                        {
                            CD_SYSTEM.playback_state = 1;
                        }
                        else
                        {
                            CD_SYSTEM.playback_state = 0;
                        }

                        CdSyncCallback(cdrom_complete_command);
                        CdReadyCallback(NULL);
                        while (CdControlB(CdlNop, NULL, &CD_SYSTEM.status_byte) == 0)
                        {
                        }
                    }
                    else
                    {
                        CdSyncCallback(cdrom_handle_recovery_sync);
                        CdReadyCallback(NULL);
                        while (CdControlB(CdlNop, NULL, &CD_SYSTEM.status_byte) == 0)
                        {
                        }
                    }
                    g_cd_vsync_timestamp = VSync(-1);
                }
            }
            else
            {
                g_cd_vsync_timestamp = VSync(-1);
            }

            g_cd_pending_queue_count = pending_count;
        }
        else if (CD_SYSTEM.queue_read_index != CD_SYSTEM.queue_write_index)
        {
            CD_SYSTEM.vsync_timestamp = VSync(-1);
            CD_SYSTEM.current_command = CdlNop;
            CD_SYSTEM.status_flags.word |= CD_STATUS_COMMAND_ACTIVE;

            if (CD_SYSTEM.transfer_callback != NULL)
            {
                CD_SYSTEM.playback_state = 1;
            }
            else
            {
                CD_SYSTEM.playback_state = 0;
            }

            CdSyncCallback(cdrom_complete_command);
            CdReadyCallback(NULL);
            CdSync(0, NULL);
            CdControlF(CdlNop, NULL);
            pending_count = (CD_SYSTEM.queue_write_index - CD_SYSTEM.queue_read_index) & CD_COMMAND_QUEUE_MASK;
        }
        else
        {
            CD_SYSTEM.transfer_callback = NULL;
            CD_SYSTEM.playback_state = 0;

            if (!(CD_SYSTEM.status_flags.word & CD_STATUS_SUPPRESS_IDLE_POLL))
            {
                if (VSync(-1) >= (CD_SYSTEM.vsync_timestamp + CD_STATUS_POLL_FRAMES))
                {
                    if (CdControlB(CdlNop, NULL, &CD_SYSTEM.status_byte) != 0)
                    {
                        if (CD_SYSTEM.status_byte & CdlStatShellOpen)
                        {
                            cdrom_handle_sync_error();
                        }
                        CD_SYSTEM.sync_complete = 0;
                        CD_SYSTEM.retry_counter = 0;
                        CD_SYSTEM.vsync_timestamp = VSync(-1);
                    }
                    else
                    {
                        if (CD_SYSTEM.retry_counter++ >= CD_IDLE_STATUS_RETRY_LIMIT)
                        {
                            cdrom_handle_sync_error();
                        }
                    }
                }
            }
            pending_count = 0;
            g_cd_pending_queue_count = 0;
        }
    }

    if (g_cd_audio_enabled != 0)
    {
        FUN_80140d48();
    }

    return pending_count;
}

/**
 * @brief Advances asynchronous CD-ROM reconfiguration after recovery.
 *
 * @return Zero while reconfiguring, otherwise one.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/IvxZG
 */
s32 cdrom_recover(void)
{
    u8 filter_params[2];
    s32 timestamp;
    u8 reconfigure_step;

    if (!(CD_SYSTEM.status_flags.word & CD_STATUS_RECOVERY_PENDING))
    {
        return TRUE;
    }

    switch (CD_SYSTEM.init_state)
    {
    case CD_RECONFIGURE_STATE_FLUSH:
        CdFlush();
        CD_SYSTEM.init_state = CD_RECONFIGURE_STATE_SET_MODE;
        CD_SYSTEM.vsync_timestamp = VSync(-1) + CD_RECOVERY_FLUSH_DELAY_FRAMES;
        break;

    case CD_RECONFIGURE_STATE_SET_MODE:
        timestamp = VSync(-1);
        if (timestamp >= CD_SYSTEM.vsync_timestamp)
        {
            CD_SYSTEM.set_mode_param_async[0] = (CdlModeSpeed | CdlModeSize1);
            CD_SYSTEM.set_mode_param_async[1] = 0;
            CD_SYSTEM.set_mode_param_async[2] = 0;
            CD_SYSTEM.set_mode_param_async[3] = 0;

            CdSyncCallback(cdrom_handle_recovery_sync);

            CdReadyCallback(NULL);
            CD_SYSTEM_V.init_command = CD_RECONFIGURE_STEP_SET_FILTER;
            CdControlF(CdlSetmode, CD_SYSTEM.set_mode_param_async);
            timestamp = VSync(-1);
            CD_SYSTEM.vsync_timestamp = timestamp + CD_SET_MODE_DELAY_FRAMES;
        }
        break;

    case CD_RECONFIGURE_STATE_SET_FILTER:
        CdSyncCallback(cdrom_handle_recovery_sync);
        CD_SYSTEM.init_command = CD_RECONFIGURE_STEP_DEMUTE;

        filter_params[0] = CD_RECOVERY_FILTER_FILE;
        filter_params[1] = CD_RECOVERY_FILTER_CHANNEL;
        CdControlF(CdlSetfilter, filter_params);
        CD_SYSTEM.init_state = CD_RECONFIGURE_STATE_WAIT;
        CD_SYSTEM.vsync_timestamp = VSync(-1);
        break;

    case CD_RECONFIGURE_STATE_WAIT:
        if (CD_SYSTEM.sync_complete == 1)
        {
            CD_SYSTEM.vsync_timestamp = VSync(-1);
            CD_SYSTEM_V.sync_complete = 0;
            break;
        }

        timestamp = VSync(-1);
        if (timestamp < (CD_SYSTEM.vsync_timestamp + CD_STATUS_POLL_FRAMES))
        {
            break;
        }

        // Retry the step that has not reported completion.
        CdSyncCallback(cdrom_handle_recovery_sync);

        reconfigure_step = CD_SYSTEM.init_command;

        switch (reconfigure_step)
        {
        case CD_RECONFIGURE_STEP_NONE:
        default:
            filter_params[0] = CD_RECOVERY_FILTER_FILE;
            filter_params[1] = CD_RECOVERY_FILTER_CHANNEL;
            CdControlF(CdlSetfilter, filter_params);
            CD_SYSTEM_V.init_command = CD_RECONFIGURE_STEP_SET_FILTER;
            break;

        case CD_RECONFIGURE_STEP_DEMUTE:
            CdControlF(CdlDemute, NULL);
            break;

        case CD_RECONFIGURE_STEP_PAUSE:
            CdControlF(CdlPause, NULL);
            break;
        }

        CD_SYSTEM.vsync_timestamp -= CD_STATUS_POLL_FRAMES;
        break;
    }

    return FALSE;
}

/**
 * @brief Validates sector position while recovering an interrupted read.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/iWEyM
 */
void cdrom_verify_recovery(void)
{
    volatile CdSystem* cd_system = &CD_SYSTEM;

    if (g_cd_status_byte_3 != CD_READY_CALLBACK_PENDING)
    {
        return;
    }

    if (cd_system->audio_enabled != g_cd_status_byte_3)
    {
        // Wait until the sector header is available.
        while (CdGetSector(CD_SYSTEM.sector_header_buffer, CD_SECTOR_HEADER_WORDS) == 0);

        if ((CD_SYSTEM.sector_header_buffer[0] & CD_SECTOR_POSITION_MASK) == (CD_SYSTEM.current_location.raw & CD_SECTOR_POSITION_MASK))
        {
            cdrom_process_sector(TRUE);
            return;
        }

        if (CD_SYSTEM.retry_count++ < CD_RECOVERY_SECTOR_RETRY_LIMIT)
        {
            CdControlF(CD_SYSTEM.current_command, CD_SYSTEM.current_location.bytes);
        }
        else
        {
            CD_SYSTEM.status_flags.bytes.retry_exhausted = TRUE;
            CD_SYSTEM.retry_count = 0;
            if (CD_SYSTEM.transfer_callback != NULL)
            {
                CD_SYSTEM.playback_state = TRUE;
            }
            else
            {
                CD_SYSTEM.playback_state = FALSE;
            }
            CD_SYSTEM_V.current_command = CdlNop;
            CdControlF(CdlNop, NULL);
        }
    }
    else
    {
        cdrom_process_sector(TRUE);
    }

    g_cd_status_byte_3 = FALSE;
}

/**
 * @brief Advances the command queue after a CD-ROM sync event.
 *
 * @param intr   CD-ROM interrupt status.
 * @param result CD-ROM result bytes.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/BXisc
 */
void cdrom_complete_command(u8 intr, u8* result)
{
    u8 next_command;
    u32 write_index;
    u32 read_index;
    volatile CdSystem* cd_system;
    CdSystem* queue_system;

    CD_SYSTEM.sync_complete = TRUE;

    // A failed status probe transitions into recovery.
    if ((CD_SYSTEM.current_command == CdlNop) && (*result & CdlStatShellOpen))
    {
        cdrom_handle_sync_error();
        return;
    }

    if (intr == CdlComplete)
    {
        switch (CD_SYSTEM_V.current_command)
        {
        default:
        case CdlNop:
        case CdlSetloc:
        case CdlPlay:
        case CdlForward:
        case CdlBackward:
        case CdlStandby:
        case CdlStop:
        case CdlPause:
        case CD_COMMAND_INIT:
        case CdlMute:
        case CdlDemute:
        case CdlSetfilter:
        case CdlSetmode:
        case CdlGetparam:
        case CdlGetlocL:
        case CdlGetlocP:
        case CD_COMMAND_SET_SESSION:
        case CdlGetTN:
        case CdlGetTD:
        case CdlSeekP:
        case CD_COMMAND_UNUSED_17:
        case CD_COMMAND_UNUSED_18:
        case CD_COMMAND_TEST:
        case CD_COMMAND_GET_ID:
            next_command = CD_SYSTEM_V.command_queue.items[CD_SYSTEM_V.queue_read_index].command;

            // Discard queued probes before dispatching the next real command.
            if (next_command == CdlNop)
            {
                queue_system = &CD_SYSTEM;
                write_index = queue_system->queue_write_index;
                do
                {
                    read_index = queue_system->queue_read_index;
                    if (read_index == write_index)
                    {
                        CdSyncCallback(NULL);
                        queue_system->playback_state = FALSE;
                        queue_system->transfer_callback = NULL;
                        queue_system->current_command = 0;
                        queue_system->retry_counter = 0;
                        queue_system->status_flags.word &= ~CD_STATUS_COMMAND_ACTIVE;
                        queue_system->vsync_timestamp = VSync(-1);
                        return;
                    }
                    read_index = (read_index + 1) & CD_COMMAND_QUEUE_MASK;
                    queue_system->queue_read_index = read_index;
                    next_command = (queue_system->command_queue.items + read_index)->command;
                } while (next_command == CdlNop);
            }
            break;

        case CdlSeekL:
            queue_system = &CD_SYSTEM;
            queue_system->playback_state = FALSE;
            queue_system->transfer_callback = NULL;
            read_index = (queue_system->queue_read_index + 1) & CD_COMMAND_QUEUE_MASK;
            queue_system->queue_read_index = read_index;

            if (read_index == queue_system->queue_write_index)
            {
                CdSyncCallback(NULL);
                CD_SYSTEM_V.current_command = 0;
                CD_SYSTEM_V.init_command = 0;
                queue_system->retry_counter = 0;
                queue_system->status_flags.word &= ~CD_STATUS_COMMAND_ACTIVE;
                queue_system->vsync_timestamp = VSync(-1);
                return;
            }

            next_command = queue_system->command_queue.items[read_index].command;
            break;

        case CdlReadN:
        case CdlReadS:
            return;
        }

        // ReadS uses the streaming path but is dispatched through ReadN.
        if (next_command == CdlReadS)
        {
            cd_system = &CD_SYSTEM;
            if (g_cd_audio_enabled == 0)
            {
                cd_system->audio_enabled = TRUE;
            }
            next_command = CdlReadN;
        }
    }
    else
    {
        // Probe once with Nop before retrying the queue head.
        cd_system = &CD_SYSTEM;
        if (cd_system->current_command != CdlNop)
        {
            CD_SYSTEM.current_command = CdlNop;
            CdControlF(CdlNop, NULL);
            return;
        }
        next_command = CD_SYSTEM.command_queue.items[CD_SYSTEM.queue_read_index].command;
    }
    cdrom_run_command(next_command, NULL, FALSE);
}

/**
 * @brief Advances CD-ROM initialization and recovery after a sync event.
 *
 * @param intr   CD-ROM interrupt status.
 * @param result CD-ROM result bytes.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/0Dz2i
 */
void cdrom_handle_recovery_sync(u8 intr, u8* result)
{
    s32 status_flags;
    s32 write_index;
    s32 read_index;
    u8 filter_params[2];
    u8 next_command;
    AudioSystem* audio_system;
    CdSystem* cd_system;
    s32* status_ptr;

    CD_SYSTEM_V.sync_complete = TRUE;

    if (((s8)CD_SYSTEM_V.init_command < 0) && !(CD_SYSTEM.status_flags.word & CD_STATUS_RECOVERY_PENDING))
    {
        cd_system = &CD_SYSTEM;
        if (*result & CdlStatShellOpen)
        {
            cdrom_handle_sync_error();
            return;
        }
    }
    else
    {
        cd_system = &CD_SYSTEM;
    }

    // Abort a failed disc-ID read if XA mode is still active.
    if (((cd_system->init_command & CD_INIT_COMMAND_MASK) == CD_RECOVERY_COMMAND_READ_DISC_ID) && (cd_system->status_byte & CdlStatError))
    {
        if (cd_system->mode_flags & CdlModeRT)
        {
            CdSyncCallback(NULL);
            CdReadyCallback(NULL);
            cd_system->init_state = CD_INIT_STATE_ERROR_PAUSE;
            cd_system->init_command = 0;
            cd_system->status_flags.word &= ~CD_STATUS_COMMAND_ACTIVE;
            cd_system->status_flags.word &= ~CD_STATUS_NO_DISC;
        }
    }

    if (intr == CdlComplete)
    {
        CD_SYSTEM.init_command &= CD_INIT_COMMAND_MASK;

        // Advance the operation that completed.
        switch (CD_SYSTEM_V.init_command)
        {
        case CD_SYNC_COMMAND_PAUSE:
        case CD_SYNC_COMMAND_RESTORE_MODE:
            CD_SYSTEM_V.init_command = 0;
            if (CD_SYSTEM.queue_read_index != CD_SYSTEM.queue_write_index)
            {
                CdSyncCallback(cdrom_complete_command);
                next_command = CD_SYSTEM.command_queue.items[CD_SYSTEM.queue_read_index].command;
                if ((next_command == CdlReadS) && (CD_SYSTEM.audio_enabled == 0))
                {
                    CD_SYSTEM.audio_enabled = TRUE;
                }
                CD_SYSTEM.playback_state = FALSE;
                CD_SYSTEM.transfer_callback = NULL;
                cdrom_run_command(next_command, NULL, FALSE);
            }
            else
            {
                CdSyncCallback(NULL);
            }
            break;
        case CD_SYNC_COMMAND_AUDIO_PAUSE:
            CD_SYSTEM_V.init_command = CD_SYSTEM.init_command + 1;
            CdControlF(CdlSetmode, CD_SYSTEM.set_mode_param_blocking);
            break;
        case CD_RECONFIGURE_STEP_SET_FILTER:
            CD_SYSTEM.init_state = CD_RECOVERY_STATE_CHECK_DISC;
            CdSyncCallback(NULL);
            CD_SYSTEM_V.init_command = 0;
            break;
        case CD_RECONFIGURE_STEP_DEMUTE:
            CD_SYSTEM_V.init_command = CD_SYSTEM.init_command + 1;
            CdControlF(CdlDemute, NULL);
            break;
        case CD_RECONFIGURE_STEP_PAUSE:
            CD_SYSTEM_V.init_command = CD_SYSTEM.init_command + 1;
            CdControlF(CdlPause, NULL);
            break;
        case CD_RECONFIGURE_STEP_COMPLETE:
            CdSyncCallback(NULL);
            CD_SYSTEM.init_state = 0;
            CD_SYSTEM_V.init_command = 0;
            CD_SYSTEM.status_flags.word &= ~CD_STATUS_RECOVERY_PENDING;
            break;
        case CD_RECOVERY_COMMAND_READ_DISC_ID:
            CdSyncCallback(NULL);
            CD_SYSTEM_V.init_command = 0;
            break;
        case CD_RECOVERY_COMMAND_SET_MODE:
        case CD_RECOVERY_COMMAND_RETRY_READ:
            CD_SYSTEM.init_state = CD_RECOVERY_STATE_READ_DISC_ID;
            CdSyncCallback(NULL);
            CD_SYSTEM_V.init_command = 0;
            break;
        case CD_RECOVERY_COMMAND_COMPLETE:
            CD_SYSTEM_V.init_command = 0;
            CD_SYSTEM.init_state = 0;
            CD_SYSTEM.retry_counter = 0;

            // Publish each recovery flag transition in order.
            status_flags = CD_SYSTEM.status_flags.word;

            read_index = CD_SYSTEM.queue_read_index;
            write_index = CD_SYSTEM.queue_write_index;

            status_flags &= ~CD_STATUS_SYNC_ERROR;
            CD_SYSTEM_V.status_flags.word = status_flags;
            status_flags &= ~CD_STATUS_INVALID_DISC;
            status_flags &= ~CD_STATUS_NO_DISC;
            // The pointer assignment preserves the original register allocation.
            CD_SYSTEM.status_flags.word = *(status_ptr = &status_flags);
            if (read_index != write_index)
            {
                CD_SYSTEM.current_command = CdlNop;
                CD_SYSTEM.status_flags.word = status_flags | CD_STATUS_COMMAND_ACTIVE;
                CdSyncCallback(cdrom_complete_command);
                CdSync(0, NULL);
                CdControlF(CdlNop, NULL);
            }
            else
            {
                CdSyncCallback(NULL);
                CD_SYSTEM.status_flags.word &= ~CD_STATUS_COMMAND_ACTIVE;
            }
            if (g_cd_audio_enabled != 0)
            {
                audio_system = &AUDIO_SYSTEM;
                if (g_cd_audio_ready != 0)
                {
                    audio_system->audio_stream_state = TRUE;
                }
            }
            break;
        }
        g_cd_vsync_timestamp = VSync(-1);
        return;
    }
    if ((s8)CD_SYSTEM_V.init_command >= 0)
    {
        CD_SYSTEM_V.init_command |= CD_INIT_COMMAND_RETRY_FLAG;
        CdControlF(CdlNop, NULL);
        return;
    }

    CD_SYSTEM_V.init_command &= CD_INIT_COMMAND_MASK;

    // Retry the operation that failed.
    switch (CD_SYSTEM_V.init_command)
    {
    case CD_SYNC_COMMAND_RESTORE_MODE:
        CdControlF(CdlSetmode, CD_SYSTEM.set_mode_param_blocking);
        return;
    case CD_RECONFIGURE_STEP_SET_FILTER:
        CD_SYSTEM.init_state = CD_RECOVERY_STATE_POLL_STATUS;
        CdSyncCallback(NULL);
        CD_SYSTEM_V.init_command = 0;
        return;
    case CD_RECONFIGURE_STEP_DEMUTE:
        filter_params[0] = CD_RECOVERY_FILTER_FILE;
        filter_params[1] = CD_RECOVERY_FILTER_CHANNEL;
        CdControlF(CdlSetfilter, filter_params);
        return;
    case CD_RECONFIGURE_STEP_PAUSE:
        CdControlF(CdlDemute, NULL);
        return;
    case CD_SYNC_COMMAND_PAUSE:
    case CD_SYNC_COMMAND_AUDIO_PAUSE:
    case CD_RECONFIGURE_STEP_COMPLETE:
        CdControlF(CdlPause, NULL);
        return;
    case CD_RECOVERY_COMMAND_READ_DISC_ID:
        CdControlF(CdlReadN, CD_SYSTEM.recovery_read_position.bytes);
        return;
    case CD_RECOVERY_COMMAND_RETRY_READ:
        CD_SYSTEM.init_state = CD_RECOVERY_STATE_READ_DISC_ID;
        CD_SYSTEM_V.init_command = 0;
        CdSyncCallback(NULL);
        return;
    case CD_RECOVERY_COMMAND_SET_MODE:
    case CD_RECOVERY_COMMAND_COMPLETE:
        CD_SYSTEM.init_state = CD_RECOVERY_STATE_SET_MODE;
        CD_SYSTEM_V.init_command = 0;
        CdSyncCallback(NULL);
        return;
    default:
        return;
    }
}

/**
 * @brief Handles CD-ROM data-ready interrupts and sector-read retries.
 *
 * @param intr   CD-ROM interrupt status.
 * @param result CD-ROM result bytes (unused).
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/kgBY4
 */
void cdrom_handle_ready_intr(u8 intr, u8* result)
{
    u8 audio_enabled;
    u8 defer_data_ready;
    s32 ready_state;
    u8 retry_count;
    u8 audio_retry_count;
    AudioSystem* audio_system;
    volatile CdSystem* cd_system;

    CD_SYSTEM.sync_complete = TRUE;
    audio_enabled = CD_SYSTEM.audio_enabled;

    // Data reads validate the sector position before consuming it.
    if (audio_enabled != TRUE)
    {
        if ((intr == CdlDataReady) && (CD_SYSTEM.status_flags.bytes.data_ready_pending == FALSE))
        {
            defer_data_ready = CD_SYSTEM.status_flags.bytes.defer_data_ready;
            ready_state = defer_data_ready;

            if (ready_state == intr)
            {
                CD_SYSTEM.status_flags.bytes.data_ready_pending = ready_state;
                return;
            }

            while (CdGetSector(CD_SYSTEM.sector_header_buffer, CD_SECTOR_HEADER_WORDS) == 0);

            if ((CD_SYSTEM.sector_header_buffer[0] & CD_SECTOR_POSITION_MASK) == (CD_SYSTEM.current_location.raw & CD_SECTOR_POSITION_MASK))
            {
                cdrom_process_sector(FALSE);
                return;
            }
        }

        retry_count = CD_SYSTEM.retry_count;
        CD_SYSTEM.retry_count = (u8)(retry_count + 1);
        if (retry_count < CD_RECOVERY_SECTOR_RETRY_LIMIT)
        {
            CdControlF(CD_SYSTEM.current_command, CD_SYSTEM.current_location.bytes);
            return;
        }

        CD_SYSTEM.status_flags.bytes.retry_exhausted = TRUE;
        CD_SYSTEM.retry_count = 0;

        if (CD_SYSTEM.transfer_callback != NULL)
        {
            CD_SYSTEM.playback_state = TRUE;
        }
        else
        {
            CD_SYSTEM.playback_state = FALSE;
        }

        CdReadyCallback(NULL);
        // The pointer form preserves the original absolute-address sequence.
        cd_system = &CD_SYSTEM;
        cd_system->current_command = CdlNop;
        CdControlF(CdlNop, NULL);
        return;
    }

    // XA delivery waits while the movie decoder owns the shared pipeline.
    ready_state = audio_enabled;
    if (intr == ready_state)
    {
        audio_system = &AUDIO_SYSTEM;
        if ((g_gpu_mode == 0) && (audio_system->mdec_busy != 0))
        {
            CD_SYSTEM.status_flags.bytes.data_ready_pending = ready_state;
            return;
        }
        cdrom_process_sector(FALSE);
        return;
    }

    audio_retry_count = CD_SYSTEM.retry_count;
    CD_SYSTEM.retry_count = (u8)(audio_retry_count + 1);
    if (audio_retry_count >= CD_RECOVERY_SECTOR_RETRY_LIMIT)
    {
        CD_SYSTEM.status_flags.bytes.retry_exhausted = TRUE;
        CD_SYSTEM.retry_count = 0;
        CD_SYSTEM.playback_state = TRUE;
        CdReadyCallback(NULL);
        CD_SYSTEM.current_command = CdlNop;
        CdControlF(CdlNop, NULL);
    }
}

/**
 * @brief Consumes a ready CD sector and advances the active transfer.
 *
 * @param execution_mode Current dispatch mode; zero for asynchronous reads.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/43gwj
 */
void cdrom_process_sector(s32 execution_mode)
{
    u8* buffer;
    volatile CdSystem* cd_system;

    cd_system = &CD_SYSTEM;
    CD_SYSTEM.retry_count = 0;
    CD_SYSTEM.status_flags.bytes.retry_exhausted = FALSE;
    CD_SYSTEM.status_flags.bytes.data_ready_pending = FALSE;

    if (CD_SYSTEM.audio_enabled != TRUE)
    {
        if (CD_SYSTEM.transfer_callback != NULL)
        {
            buffer = CD_SYSTEM.transfer_callback(CD_SYSTEM.total_data_size - CD_SYSTEM.read_remaining_bytes, CD_SYSTEM.read_remaining_bytes);
            if (buffer == NULL)
            {
                // The callback deferred this sector; retry the current read.
                CdControlF(cd_system->current_command, CD_SYSTEM.current_location.bytes);
                return;
            }
        }
        else
        {
            buffer = CD_SYSTEM.current_write_ptr;
        }

        if (CD_SYSTEM.read_remaining_bytes >= (CD_DATA_SECTOR_SIZE + 1))
        {
            while (CdGetSector(buffer, CD_DATA_SECTOR_WORDS) == 0);
            CdIntToPos(CdPosToInt(&CD_SYSTEM.current_location.pos) + 1, &CD_SYSTEM.current_location.pos);
            CD_SYSTEM.read_remaining_bytes -= CD_DATA_SECTOR_SIZE;
            if (CD_SYSTEM.transfer_callback == NULL)
            {
                CD_SYSTEM.current_write_ptr += CD_DATA_SECTOR_SIZE;
            }
        }
        else
        {
            // The final sector completes this queue entry.
            CD_SYSTEM.playback_state = FALSE;
            CD_SYSTEM.transfer_callback = NULL;
            CD_SYSTEM.queue_read_index = (CD_SYSTEM.queue_read_index + 1) & CD_COMMAND_QUEUE_MASK;
            if (CD_SYSTEM.queue_read_index != CD_SYSTEM.queue_write_index)
            {
                cdrom_run_command(CD_SYSTEM.command_queue.items[CD_SYSTEM.queue_read_index].command, buffer, execution_mode + 1);
                return;
            }

            CD_SYSTEM.init_command = CD_SYNC_COMMAND_PAUSE;
            CdSyncCallback(cdrom_handle_recovery_sync);
            CdReadyCallback(NULL);
            if (execution_mode == CD_EXECUTION_MODE_ASYNC)
            {
                CdControlF(CdlPause, NULL);
            }

            while (CdGetSector(buffer, CD_BYTES_TO_WORDS(g_cd_read_remaining_bytes)) == 0);

            cd_system = &CD_SYSTEM;
            CD_SYSTEM.status_flags.word &= ~CD_STATUS_COMMAND_ACTIVE;
            cd_system->current_command = 0;
            cd_system->retry_counter = 0;
            if (execution_mode != CD_EXECUTION_MODE_ASYNC)
            {
                CdControlF(CdlPause, NULL);
            }
            CD_SYSTEM.vsync_timestamp = VSync(-1);
        }

        return;
    }

    // XA sectors are accepted only at the expected disc position.
    while (CdGetSector(CD_SYSTEM.sector_header_buffer, CD_SECTOR_HEADER_WORDS) == 0);

    cd_system = &CD_SYSTEM;
    if ((CD_SYSTEM.sector_header_buffer[0] & CD_SECTOR_POSITION_MASK) == (CD_SYSTEM.current_location.raw & CD_SECTOR_POSITION_MASK))
    {
        if (CD_SYSTEM.transfer_callback(CD_SYSTEM.total_data_size - CD_SYSTEM.read_remaining_bytes, CD_SYSTEM.read_remaining_bytes) == NULL)
        {
            // The callback ended the XA stream; restore normal data-read mode.
            CD_SYSTEM.queue_read_index = (CD_SYSTEM.queue_read_index + 1) & CD_COMMAND_QUEUE_MASK;
            CdSyncCallback(cdrom_handle_recovery_sync);
            CdReadyCallback(NULL);
            cd_system->set_mode_param_blocking[0] = CdlModeSpeed | CdlModeSize1;
            cd_system->current_command = 0;
            cd_system->init_command = CD_SYNC_COMMAND_AUDIO_PAUSE;
            cd_system->audio_enabled = FALSE;
            cd_system->playback_state = FALSE;
            cd_system->transfer_callback = NULL;
            cd_system->retry_counter = 0;
            CD_SYSTEM.status_flags.word &= ~CD_STATUS_COMMAND_ACTIVE;
            CdControlF(CdlPause, NULL);
            CD_SYSTEM.vsync_timestamp = VSync(-1);
        }
        else
        {
            CdIntToPos(CdPosToInt(&CD_SYSTEM.current_location.pos) + 1, &CD_SYSTEM.current_location.pos);
        }

        return;
    }

    // Ignore an out-of-position XA sector and retry the expected location.
    CdControlF(cd_system->current_command, CD_SYSTEM.current_location.bytes);
}

/**
 * @brief Dispatches a queued CD-ROM command in the requested execution mode.
 *
 * @param command CD-ROM controller command.
 * @param sector_buffer Sector destination used by synchronous modes.
 * @param execution_mode Dispatch order defined by CdExecutionMode.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/KM6id
 */
void cdrom_run_command(u8 command, u8* sector_buffer, s32 execution_mode)
{
    u8* command_params;
    s32 next_read_index;
    s32 data_size;
    s32 control_command;
    CdResourceEntry* queued_resource;
    CdQueueSystemCursor queue_entry;
    CdQueueSystemCursor queue_buffer;
    volatile CdSystem* cd_system;

    queued_resource = NULL;

    // Skip seeks that are superseded by another pending command.
    while (command == CdlSeekL)
    {
        next_read_index = (CD_SYSTEM_V.queue_read_index + 1) & CD_COMMAND_QUEUE_MASK;

        if (CD_SYSTEM.queue_write_index == next_read_index)
        {
            break;
        }

        CD_SYSTEM_V.queue_read_index = next_read_index;
        command = CD_SYSTEM_V.command_queue.items[next_read_index].command;
    }

    if ((command == CdlSeekL) || (command == CdlReadN) || (command == CdlReadS))
    {
        if ((command == CdlSeekL) || (g_playback_state == FALSE))
        {
            CD_SYSTEM_V.transfer_callback = NULL;
            CD_SYSTEM_V.playback_state = FALSE;
            queued_resource = CD_SYSTEM_V.command_queue.items[CD_SYSTEM.queue_read_index].entry;
            CD_SYSTEM.current_location = queued_resource->location;
        }

        switch (execution_mode)
        {
        case CD_EXECUTION_MODE_COMMAND_THEN_READ:
            CD_SYSTEM_V.current_command = command;
            CdControlF(command, CD_SYSTEM.current_location.bytes);
            while (CdGetSector(sector_buffer, CD_BYTES_TO_WORDS(g_cd_read_remaining_bytes)) == 0);
            break;

        case CD_EXECUTION_MODE_READ_THEN_COMMAND:
            while (CdGetSector(sector_buffer, CD_BYTES_TO_WORDS(g_cd_read_remaining_bytes)) == 0);
            CdSync(0, NULL);
            break;
        }

        if ((command == CdlReadN) || (command == CdlReadS))
        {
            queue_entry.address = (CD_SYSTEM_V.queue_read_index * sizeof(CdCommandQueueItem)) + CD_SYSTEM_ADDRESS;
            if ((queue_entry.items[CD_COMMAND_QUEUE_BASE_OFFSET].callback == NULL) &&
                (CD_SYSTEM_V.current_write_ptr == queue_entry.items[CD_COMMAND_QUEUE_BASE_OFFSET].dst_buffer))
            {
                CD_SYSTEM_V.playback_state = FALSE;
            }

            cd_system = &CD_SYSTEM_V;
            if (g_playback_state == FALSE)
            {
                data_size = queued_resource->data_size;
                queue_buffer.system = &CD_SYSTEM;
                queue_buffer.bytes += cd_system->queue_read_index * sizeof(CdCommandQueueItem);
                CD_SYSTEM_V.total_data_size = data_size;
                CD_SYSTEM_V.read_remaining_bytes = data_size;
                CD_SYSTEM_V.current_write_ptr = queue_buffer.items[CD_COMMAND_QUEUE_BASE_OFFSET].dst_buffer;
                CD_SYSTEM_V.transfer_callback = queue_buffer.items[CD_COMMAND_QUEUE_BASE_OFFSET].callback;
            }

            if (execution_mode == CD_EXECUTION_MODE_ASYNC)
            {
                CD_SYSTEM_V.status_flags.bytes.data_ready_pending = FALSE;
                CdReadyCallback(cdrom_handle_ready_intr);
            }
        }
        else if (execution_mode == CD_EXECUTION_MODE_COMMAND_THEN_READ)
        {
            CdReadyCallback(NULL);
        }

        if (execution_mode != CD_EXECUTION_MODE_COMMAND_THEN_READ)
        {
            CD_SYSTEM_V.current_command = command;
            CdControlF(command, CD_SYSTEM.current_location.bytes);
        }

        g_playback_state = FALSE;
        return;
    }

    switch (execution_mode)
    {
    case CD_EXECUTION_MODE_ASYNC:
        CD_SYSTEM_V.current_command = command;

        if (command == CdlSetmode)
        {
            control_command = CdlSetmode;
            command_params = CD_SYSTEM.set_mode_param_blocking;
        }
        else
        {
            control_command = command;
            command_params = NULL;
        }
        break;

    case CD_EXECUTION_MODE_COMMAND_THEN_READ:
        CdReadyCallback(NULL);
        CD_SYSTEM_V.current_command = command;
        control_command = CdlNop;
        CdControlF(command, NULL);
        while (CdGetSector(sector_buffer, CD_BYTES_TO_WORDS(g_cd_read_remaining_bytes)) == 0);
        return;

    case CD_EXECUTION_MODE_READ_THEN_COMMAND:
        while (CdGetSector(sector_buffer, CD_BYTES_TO_WORDS(g_cd_read_remaining_bytes)) == 0);
        CD_SYSTEM_V.current_command = command;
        control_command = command;
        command_params = NULL;
        break;

    default:
        return;
    }

    CdControlF(control_command, command_params);
}

/**
 * @brief Validates disc identification data read during recovery.
 *
 * @param interrupt CD-ROM ready callback reason.
 * @param result Drive result buffer; unused by this callback.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/XrcPe
 */
void cdrom_verify_disc(u8 interrupt, u8* result)
{
    u32 status_flags;
    u8 expected_character;
    u8 disc_character;
    const u8* expected_id;
    u8* disc_id;

    CD_SYSTEM_V.sync_complete = TRUE;

    if (interrupt == CdlDataReady)
    {
        // Retaining the result assignment preserves the validation-loop register layout.
        while ((expected_character = (CdGetSector(CD_SYSTEM.sector_header_buffer, CD_SECTOR_HEADER_WORDS) == 0)));

        if ((CD_SYSTEM.sector_header_buffer[0] & CD_SECTOR_POSITION_MASK) == (CD_SYSTEM.recovery_read_position.raw & CD_SECTOR_POSITION_MASK))
        {
            while (CdGetSector(CD_SYSTEM.disc_validation_id, CD_DISC_VALIDATION_WORDS) == 0);

            expected_id = g_disc_validation_id;
            disc_id = CD_SYSTEM.disc_validation_id;
            expected_character = *expected_id++;

            while (expected_character != '\0')
            {
                // Multibyte ID characters must match both encoded bytes.
                if (CD_IS_MULTIBYTE_ID_CHAR(expected_character))
                {
                    if (expected_character != *disc_id++)
                    {
                        goto disc_invalid;
                    }

                    expected_character = *disc_id++;
                    disc_character = *expected_id++;
                }
                else
                {
                    disc_character = *disc_id++;
                }

                if (expected_character != disc_character)
                {
                disc_invalid:
                    status_flags = CD_SYSTEM.status_flags.word & ~CD_STATUS_NO_DISC;
                    CD_SYSTEM_V.init_state = CD_INIT_STATE_ERROR_PAUSE;
                    CD_SYSTEM_V.status_flags.word = status_flags;
                    CD_SYSTEM.status_flags.word = status_flags & ~CD_STATUS_COMMAND_ACTIVE;
                    CdReadyCallback(NULL);
                    return;
                }

                expected_character = *expected_id++;
            }

            CdReadyCallback(NULL);
            CD_SYSTEM_V.init_command = CD_RECOVERY_COMMAND_COMPLETE;
            CdSyncCallback(cdrom_handle_recovery_sync);
            CdControlF(CdlSetmode, CD_SYSTEM.set_mode_param_blocking);
            return;
        }
    }

    CdReadyCallback(NULL);
    CD_SYSTEM_V.init_command = CD_RECOVERY_COMMAND_RETRY_READ;
    CdSyncCallback(cdrom_handle_recovery_sync);
    CdControlF(CdlPause, NULL);
}

/**
 * @brief Processes CD-ROM state once per frame until the command queue is empty.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/rE8hd
 */
void cdrom_wait_queue_empty(void)
{
    while (cdrom_process_state() != 0)
    {
        VSync(0);
    }
}

/**
 * @brief Clears callbacks and resets CD command state after a sync failure.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/lU7lO
 */
void cdrom_handle_sync_error(void)
{
    CdSyncCallback(NULL);
    CdReadyCallback(NULL);

    CD_SYSTEM.init_state = CD_RECOVERY_STATE_IDLE;
    CD_SYSTEM.status_flags.word |= CD_STATUS_SYNC_ERROR;
    CD_SYSTEM.current_command = CD_COMMAND_NONE;
    CD_SYSTEM.init_command = CD_SYNC_COMMAND_NONE;
    CD_SYSTEM.retry_count = 0;
    CD_SYSTEM.retry_counter = 0;
    CD_SYSTEM.status_flags.word &= ~CD_STATUS_COMMAND_ACTIVE;
    CD_SYSTEM.vsync_timestamp = VSync(-1);
}

/**
 * @brief Configures CD audio volume and mono routing.
 *
 * @param volume Volume level from 0 to 255.
 * @param mix_mode Zero routes CD left to both SPU outputs; nonzero routes both
 *                  CD inputs to the SPU-left output.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/lwzx1
 */
void cdrom_set_audio_volume(u8 volume, s32 mix_mode)
{
    CdlATV audio_mix;

    // The single-iteration form preserves the original channel-store scheduling.
    do
    {
        if (mix_mode != 0)
        {
            audio_mix.val0 = volume;
            audio_mix.val1 = 0;
            audio_mix.val2 = volume;
        }
        else
        {
            audio_mix.val0 = volume;
            audio_mix.val1 = volume;
            audio_mix.val2 = 0;
        }

        audio_mix.val3 = 0;
    } while (FALSE);

    CdMix(&audio_mix);
}

/**
 * @brief Stops CD/XA playback and resets command and callback state.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/fnucZ
 */
void cdrom_reset(void)
{
    AudioSystem* audio_system = &AUDIO_SYSTEM;

    DecDCToutCallback(audio_system->dec_dct_out_callback_handler);
    DrawSyncCallback(audio_system->draw_sync_callback_handler);

    CdSyncCallback(NULL);
    CdReadyCallback(NULL);

    while (CdControlB(CdlPause, NULL, NULL) == 0);

    if (g_cd_audio_ready != FALSE)
    {
        akao_cmd_e2();
    }

    CD_SYSTEM.audio_enabled = FALSE;
    CD_SYSTEM.current_command = CD_COMMAND_NONE;
    CD_SYSTEM.init_command = CD_SYNC_COMMAND_NONE;
    CD_SYSTEM.queue_read_index = 0;
    CD_SYSTEM.queue_write_index = 0;
    CD_SYSTEM.retry_counter = 0;
    CD_SYSTEM.playback_state = FALSE;
    CD_SYSTEM.transfer_callback = NULL;
    CD_SYSTEM.status_flags.word &= ~CD_STATUS_COMMAND_ACTIVE;
    CD_SYSTEM.vsync_timestamp = VSync(-1);
}

/**
 * @brief Tests whether a resource is absent from the pending command queue.
 *
 * @param resource_index Resource index to search for.
 * @return TRUE when absent; FALSE when already queued.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/l4HlL
 */
s32 cdrom_can_queue_resource(s32 resource_index)
{
    u16 queued_resource_index;
    u16 target_resource_index;
    s32 scan_index;
    s32 remaining_entries;

    scan_index = CD_SYSTEM.queue_read_index;
    target_resource_index = resource_index;

    remaining_entries = (CD_SYSTEM.queue_write_index - scan_index) & CD_COMMAND_QUEUE_MASK;

    for (--remaining_entries; remaining_entries != -1; remaining_entries--)
    {
        queued_resource_index = CD_SYSTEM.command_queue.items[scan_index].resource_index;

        if (target_resource_index == queued_resource_index)
        {
            return FALSE;
        }

        scan_index &= CD_COMMAND_QUEUE_MASK;
        scan_index++;
    }

    return TRUE;
}

/**
 * @brief Loads the CD resource table and initializes default read settings.
 *
 * @param lba Logical block address of the resource table.
 * @param data_size_bytes Resource table size in bytes.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/Y9z7y
 */
void cdrom_load_resource_table(s32 lba, s32 data_size_bytes)
{
    CdlLOCRaw* location;
    s32 vsync_offset;
    s32 vsync_delta;
    CdSystem* cd_system;

    vsync_offset = CD_RESOURCE_LOAD_VSYNC_OFFSET;
    vsync_delta = g_cd_vsync_timestamp - (VSync(-1) + vsync_offset);

    if (vsync_delta > 0)
    {
        if (vsync_delta == 1)
        {
            // A one-frame delay is requested through VSync(0).
            vsync_delta = 0;
        }

        VSync(vsync_delta);
    }

    cd_system = &CD_SYSTEM;
    location = &cd_system->default_cd_resource.location;
    cd_system->default_cd_resource.location.raw = 0;
    cd_system->default_cd_resource.data_size = data_size_bytes;

    CdIntToPos(lba, &location->pos);
    cdrom_queue_command(CdlReadN, CD_RESOURCE_INDEX_DEFAULT, CD_RESOURCE_ENTRIES, NULL);
    cdrom_wait_queue_empty();
    cdrom_set_audio_volume(CD_DEFAULT_AUDIO_VOLUME, CD_AUDIO_MIX_BOTH_TO_LEFT);
}

/**
 * @brief Queues a resource read into a destination buffer.
 *
 * @param resource_index Resource table index.
 * @param dst_buffer Destination buffer.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/OxunQ
 */
void cdrom_queue_read(s32 resource_index, void* dst_buffer)
{
    cdrom_queue_command(CdlReadN, resource_index, dst_buffer, NULL);
}

/**
 * @brief Queues a resource read handled by a transfer callback.
 *
 * @param resource_index Resource table index.
 * @param callback Transfer callback.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/5M5cV
 */
void cdrom_queue_read_with_callback(s32 resource_index, CdCommandCallback callback)
{
    u16 queued_resource_index;

    queued_resource_index = resource_index;
    cdrom_queue_command(CdlReadN, queued_resource_index, NULL, callback);
}

/**
 * @brief Queues a logical seek to a resource.
 *
 * @param resource_index Resource table index.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/iUUQh
 */
void cdrom_queue_seek(s32 resource_index)
{
    cdrom_queue_command(CdlSeekL, resource_index, NULL, NULL);
}

/**
 * @brief Returns a resource's data size.
 *
 * @param resource_index Resource table index.
 * @return Resource size in bytes.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/SGZF5
 */
s32 cdrom_get_resource_size(s32 resource_index)
{
    u16 table_index;

    table_index = resource_index;
    return CD_RESOURCE_ENTRIES[table_index].data_size;
}

/**
 * @brief Returns the current CD recovery error status.
 *
 * @return CdErrorStatus value.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/vfLUw
 */
s32 cdrom_get_error_status(void)
{
    u32 status_flags;

    status_flags = CD_SYSTEM.status_flags.word;

    if (status_flags & CD_STATUS_SYNC_ERROR)
    {
        return CD_ERROR_STATUS_SYNC_ERROR;
    }

    if ((status_flags & CD_STATUS_INVALID_DISC) != 0)
    {
        if (status_flags & CD_STATUS_NO_DISC)
        {
            return CD_ERROR_STATUS_DISC_CHECK_PENDING;
        }

        return CD_ERROR_STATUS_INVALID_DISC;
    }

    if (status_flags & CD_STATUS_NO_DISC)
    {
        return CD_ERROR_STATUS_NO_DISC;
    }

    if (CD_SYSTEM.status_flags.bytes.retry_exhausted == TRUE)
    {
        return CD_ERROR_STATUS_RETRIES_EXHAUSTED;
    }

    return CD_ERROR_STATUS_NONE;
}

/**
 * @brief Restores saved callbacks, pauses the drive, and clears CD state.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/HSXMR
 */
void cdrom_restore_callbacks(void)
{
    CdSyncCallback(CD_SYSTEM.previous_sync_callback);
    CdReadyCallback(CD_SYSTEM.previous_ready_callback);

    while (CdControlB(CdlPause, NULL, NULL) == 0);

    CD_SYSTEM.resource_index = CD_RESOURCE_INDEX_INVALID;
    CD_SYSTEM.pending_queue_count = 0;
    CD_SYSTEM.current_resource_index = 0;
    CD_SYSTEM.current_data_size = 0;
    CD_SYSTEM.target_data_size = 0;
    CD_SYSTEM.playback_state = FALSE;
    CD_SYSTEM.transfer_callback = NULL;
    CD_SYSTEM.current_command = CD_COMMAND_NONE;
    CD_SYSTEM.init_command = CD_SYNC_COMMAND_NONE;
    CD_SYSTEM.retry_count = 0;
    CD_SYSTEM.retry_counter = 0;
    CD_SYSTEM.last_command = CD_COMMAND_NONE;
    CD_SYSTEM.dst_buffer = NULL;
    CD_SYSTEM.callback = NULL;
    CD_SYSTEM.status_flags.word &= ~CD_STATUS_COMMAND_ACTIVE;
    CD_SYSTEM.status_flags.bytes.defer_data_ready = FALSE;
    CD_SYSTEM.status_flags.bytes.data_ready_pending = FALSE;
    CD_SYSTEM.vsync_timestamp = VSync(-1);
    CD_SYSTEM.queue_read_index = 0;
    CD_SYSTEM.queue_write_index = 0;

    CdFlush();
}

/**
 * @brief Requests recovery mode when command processing is idle.
 *
 * @return TRUE if recovery is active or entered; FALSE if the subsystem is busy.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/gsUc3
 */
s32 cdrom_enter_recovery_mode(void)
{
    u32 status_flags;
    s32 recovery_started;

    status_flags = CD_SYSTEM.status_flags.word;
    recovery_started = FALSE;

    if (status_flags & CD_STATUS_RECOVERY_PENDING)
    {
        return TRUE;
    }

    if (CD_SYSTEM.current_command == CD_COMMAND_NONE)
    {
        if ((CD_SYSTEM.init_command == CD_SYNC_COMMAND_NONE) && !(status_flags & CD_STATUS_ERROR_MASK) &&
            (CD_SYSTEM.queue_read_index == CD_SYSTEM.queue_write_index))
        {
            recovery_started = TRUE;
            CD_SYSTEM.status_flags.word |= CD_STATUS_RECOVERY_PENDING;
            CD_SYSTEM.init_state = CD_RECOVERY_STATE_IDLE;
        }
    }

    return recovery_started;
}

/**
 * @brief Defers CD data-ready processing until the flag is cleared.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/9bgSH
 */
void cdrom_defer_data_ready(void)
{
    g_cd_defer_data_ready = TRUE;
}

/**
 * @brief Decompresses a custom bytecode-encoded data stream.
 *
 * Processes opcodes from a source buffer and emits uncompressed bytes into a
 * destination buffer. Both pointers are updated in-place so the caller can
 * resume across multiple calls.
 *
 * @details
 * Each iteration reads one opcode byte. Opcodes 0xF0–0xFF are control codes;
 * all others are raw-copy codes:
 *
 *   Opcode  Encoding                        Operation
 *   ------  --------                        ---------
 *   0xF0    [packed]                        Repeat upper nibble (count = lower nibble + 3)
 *   0xF1    [count] [value]                 Repeat value (count + 4) times
 *   0xF2    [count] [packed]                Alternate lo/hi nibbles as 2-byte pairs (count + 2)
 *   0xF3    [count] [b0] [b1]               Repeat 2-byte pattern (count + 2) times
 *   0xF4    [count] [b0] [b1] [b2]          Repeat 3-byte pattern (count + 2) times
 *   0xF5    [count] [fixed] + stream        Write {fixed, next_src_byte} pairs (count + 4) times
 *   0xF6    [count] [b0] [b1] + stream      Write {b0, b1, next_src_byte} triplets (count + 3) times
 *   0xF7    [count] [b0] [b1] [b2] + stream Write {b0, b1, b2, next_src_byte} quads (count + 2) times
 *   0xF8    [count] [start]                 Ascending arithmetic run (count + 4 bytes)
 *   0xF9    [count] [start]                 Descending arithmetic run (count + 4 bytes)
 *   0xFA    [count] [start] [step]          Arithmetic run: start, start+step, ... (count + 5 bytes)
 *   0xFB    [count] [b0] [b1] [delta]       16-bit pair run; b0/b1 incremented by signed delta
 *   0xFC    [offLo] [offHi_cnt]             Back-reference: 12-bit offset + 1, count = upper nibble + 4
 *   0xFD    [offset] [count]                Back-reference: 8-bit offset + 1, count + 0x14 bytes
 *   0xFE    [packed]                        Back-reference: offset = (upper nibble << 3) + 8, count = lower nibble + 3
 *   0xFF    (none)                          End-of-stream; updates pointers and returns 0
 *   default (opcode value)                  Raw copy: opcode + 1 bytes follow
 *
 * Bounds are checked between opcode expansions. Decoding pauses when either
 * cursor reaches its end pointer.
 *
 * @param src_cursor Current source position; updated on return.
 * @param dst_cursor Current destination position; updated on return.
 * @param src_end Exclusive source bound.
 * @param dst_end Exclusive destination bound.
 *
 * @return FALSE at the end marker; TRUE when a buffer bound pauses decoding.
 *
 * @warning No bounds checking on back-reference offsets (0xFC–0xFE); a malformed
 *          stream can read before the start of the destination buffer.
 *
 * @see decomp.me: (99.78%) https://decomp.me/scratch/MlH6P
 */

s32 cdrom_decompress_data(u8** srcStart, u8** dstStart, u8* srcEnd, u8* dstEnd)
{
    u8* srcPtr;
    u8* dstPtr;
    u32 iterations;
    u32 opcode;

    u8* tempPtr;
    u8 nextByte;

    u8 offsetLow;

    u8 param0;
    u8 param1;
    u8 param2;
    u8 param3;

    u32 highB;
    u32 wordTemp;
    u32 tempSum;

    s32 seed;

    srcPtr = *srcStart;
    dstPtr = *dstStart;

    while (srcPtr < srcEnd && dstPtr < dstEnd)
    {
        opcode = *srcPtr;

        switch (opcode)
        {
        case 0xF0:
            param1 = srcPtr[1];

            srcPtr += 2;
            iterations = (param1 & 0xf) + 3;
            param1 = param1 >> 4;

            do
            {
                *dstPtr++ = param1;
            } while (--iterations != 0);
            break;

        case 0xF1:
            param1 = srcPtr[2];
            nextByte = srcPtr[1];

            srcPtr += 3;
            iterations = nextByte + 4;

            do
            {
                *dstPtr++ = param1;
            } while (--iterations != 0);
            break;

        case 0xF2:
            param1 = srcPtr[2];
            nextByte = srcPtr[1];

            srcPtr += 3;
            iterations = nextByte + 2;
            highB = param1 >> 4;
            param1 = param1 & 0xf;

            do
            {
                dstPtr[0] = param1;
                dstPtr[1] = highB;
                dstPtr += 2;
            } while (--iterations != 0);
            break;

        case 0xF3:
            param1 = srcPtr[2];
            param0 = srcPtr[3];
            nextByte = srcPtr[1];

            srcPtr += 4;
            iterations = nextByte + 2;

            do
            {
                dstPtr[0] = param1;
                dstPtr[1] = param0;
                dstPtr += 2;
            } while (--iterations != 0);
            break;

        case 0xF4:
            param1 = srcPtr[2];
            param0 = srcPtr[3];
            param3 = srcPtr[4];
            nextByte = srcPtr[1];

            srcPtr += 5;
            iterations = nextByte + 2;

            do
            {
                *dstPtr = param1;
                (&dstPtr[2])[-1] = param0;
                (&dstPtr[2])[0] = param3;
                dstPtr += 3;
            } while (--iterations != 0);

            break;

        case 0xF5:
            param1 = srcPtr[2];
            nextByte = srcPtr[1];

            srcPtr += 3;
            iterations = nextByte + 4;

            do
            {
                dstPtr[0] = param1;
                dstPtr[1] = *srcPtr++;
                dstPtr += 2;
            } while (--iterations != 0);

            break;

        case 0xF6:
            param1 = srcPtr[2];
            param0 = srcPtr[3];
            nextByte = srcPtr[1];

            srcPtr += 4;
            tempPtr = &dstPtr[2];
            iterations = nextByte + 3;

            do
            {
                *dstPtr = param1;
                tempPtr[-1] = param0;
                nextByte = *(u8*)srcPtr;
                srcPtr += 1;
                dstPtr += 3;
                tempPtr[0] = nextByte;
                tempPtr += 3;
            } while (--iterations != 0);

            break;

        case 0xF7:
            param1 = srcPtr[2];
            param0 = srcPtr[3];
            param3 = srcPtr[4];
            nextByte = srcPtr[1];

            srcPtr += 5;
            iterations = nextByte + 2;

            do
            {
                dstPtr[0] = param1;
                dstPtr[1] = param0;
                dstPtr[2] = param3;
                dstPtr[3] = *srcPtr++;
                dstPtr += 4;
            } while (--iterations != 0);

            break;

        case 0xF8:
            param1 = srcPtr[2];
            nextByte = srcPtr[1];

            srcPtr += 3;
            iterations = nextByte + 4;

            do
            {
                *dstPtr++ = param1;
                param1 += 1;
            } while (--iterations != 0);

            break;

        case 0xF9:
            param1 = srcPtr[2];
            nextByte = srcPtr[1];

            srcPtr += 3;
            iterations = nextByte + 4;

            do
            {
                *dstPtr++ = param1;
                param1 -= 1;
            } while (--iterations != 0);

            break;

        case 0xFA:
            param1 = srcPtr[2];
            param0 = srcPtr[3];
            nextByte = srcPtr[1];

            srcPtr += 4;
            iterations = nextByte + 5;

            do
            {
                *dstPtr++ = param1;
                param1 += param0;
            } while (--iterations != 0);

            break;

        case 0xFB:
            param2 = srcPtr[2];
            highB = srcPtr[3];
            nextByte = srcPtr[1];
            param3 = srcPtr[4];

            iterations = nextByte + 3;
            seed = param3 << 24; // place param3 in the high byte for sign extension
            srcPtr += 5;

            do
            {
                // Write the two current bytes
                ((u8*)dstPtr)[0] = param2;
                ((u8*)dstPtr)[1] = highB;
                dstPtr += 2;

                // Sign-extend param3 via arithmetic right shift
                tempSum = seed >> 24;

                // Form the 16-bit value (param0 << 8) | param2

                wordTemp = highB << 8;
                tempSum += param2 | wordTemp; // add to the sign-extended constant

                // Update for next iteration
                param2 = tempSum;           // low byte
                highB = (tempSum >> 8); // high byte
            } while (--iterations != 0);
            break;

        case 0xFC:
            param1 = srcPtr[1];
            offsetLow = (opcode = srcPtr[2]);

            srcPtr += 3;
            iterations = (offsetLow >> 4) + 4;

            tempPtr = (u8*)((u32)param1 | (u32)((offsetLow & 0xF) << 8));
            tempPtr = (u8*)(dstPtr - (((u32)tempPtr) & 0xFFFF));

            do
            {
                *dstPtr++ = tempPtr++ [-1];
            } while (--iterations != 0);

            break;

        case 0xFD:
            param1 = srcPtr[1];
            param2 = param1;
            nextByte = srcPtr[2];

            srcPtr += 3;
            iterations = nextByte + 0x14;
            tempPtr = (u8*)(dstPtr - param2);

            do
            {
                *dstPtr++ = tempPtr++ [-1];
            } while (--iterations != 0);

            break;

        case 0xFE:
            param1 = srcPtr[1];

            srcPtr += 2;
            iterations = (param1 & 0xF) + 3;
            tempPtr = (u8*)(dstPtr - ((u32)(param1 & 0xF0) >> 1));

            do
            {
                offsetLow = (tempPtr++)[-8];
                *dstPtr++ = offsetLow;
            } while (--iterations != 0);

            break;

        case 0xFF:
            *srcStart = &srcPtr[1];
            *dstStart = dstPtr;
            return 0;

        default:
            srcPtr++;
            iterations = opcode + 1;

            do
            {
                *dstPtr++ = *srcPtr++;
            } while (--iterations != 0);

            break;
        }

        *srcStart = srcPtr;
    }

    *dstStart = dstPtr;
    return 1;
}


/**
 * @brief Supplies ring-buffer destinations for streamed CD sectors.
 *
 * Initializes the scratchpad stream state, compacts unread input after the
 * consumer releases it, and wraps incoming sectors when the upper buffer fills.
 *
 * @param bytes_transferred Bytes delivered before the pending sector.
 * @param bytes_remaining Bytes remaining, including the pending sector.
 *
 * @return Next sector destination, or NULL when the sector must be retried.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/UDwSD
 */
u8* cdrom_handle_stream_data(s32 bytes_transferred, u32 bytes_remaining)
{
    s32 unconsumed_bytes;
    s32 alignment_padding;
    s32 wrapped_word_count;
    s32 linear_word_count;
    s32 wrapped_aligned_bytes;
    CdStreamState* stream_state;
    s32 linear_aligned_bytes;
    CdStreamCopyCursor destination;
    CdStreamCopyCursor wrapped_source;
    CdStreamCopyCursor linear_source;
    u32 bytes_buffered;
    u32 bytes_consumed;
    u32 wrap_overflow;
    u32 wrap_write_offset;
    u8* wrapped_read_ptr;
    u8* linear_read_ptr;
    u8* current_read_ptr;
    u32 previous_wrap_overflow;
    u8* aligned_buffer_start;
    u32 transfer_size;
    u8* next_sector_dst;
    volatile CdStreamState* published_state;

    transfer_size = bytes_remaining;
    if (bytes_remaining > CD_DATA_SECTOR_SIZE)
    {
        transfer_size = CD_DATA_SECTOR_SIZE;
    }

    if (bytes_transferred == 0)
    {
        // The first byte is a stream header; compressed input begins at byte one.
        CD_STREAM_STATE.data_ready = TRUE;
        CD_STREAM_STATE.write_ptr = CD_STREAM_PAYLOAD_START;
        CD_STREAM_STATE.read_ptr = CD_STREAM_PAYLOAD_START;
        CD_STREAM_STATE.bytes_buffered = transfer_size - 1;
        CD_STREAM_STATE.wrap_overflow = 0;
        return CD_STREAM_BUFFER_START;
    }

    stream_state = &CD_STREAM_STATE;
    if (!stream_state->data_ready)
    {
        // Reclaim consumed input while retaining the decompressor's unread bytes.
        unconsumed_bytes = stream_state->bytes_buffered - CD_STREAM_STATE.bytes_consumed;
        bytes_consumed = CD_STREAM_STATE.bytes_consumed;
        wrap_overflow = stream_state->wrap_overflow;
        alignment_padding = (CD_STREAM_COPY_WORD_SIZE - (unconsumed_bytes & CD_STREAM_COPY_WORD_MASK)) & CD_STREAM_COPY_WORD_MASK;
        if (wrap_overflow != 0)
        {
            wrapped_read_ptr = stream_state->read_ptr;
            destination.bytes = CD_STREAM_WRAP_START - unconsumed_bytes;
            stream_state->write_ptr = destination.bytes;
            stream_state->read_ptr = destination.bytes;
            destination.bytes -= alignment_padding;
            CD_STREAM_STATE.bytes_buffered = (wrap_overflow + unconsumed_bytes) + transfer_size;
            wrapped_source.bytes = (wrapped_read_ptr + bytes_consumed) - alignment_padding;
            wrapped_aligned_bytes = unconsumed_bytes + CD_STREAM_COPY_WORD_MASK;
            // Preserve truncation toward zero if the signed byte count is negative.
            if (wrapped_aligned_bytes < 0)
            {
                wrapped_aligned_bytes = unconsumed_bytes + CD_STREAM_NEGATIVE_WORD_ROUNDING_BIAS;
            }
            wrapped_word_count = wrapped_aligned_bytes >> CD_BYTES_PER_WORD_SHIFT;
            wrapped_word_count--;
            for (; wrapped_word_count != -1; wrapped_word_count--)
            {
                *destination.words = *wrapped_source.words;
                wrapped_source.bytes += CD_STREAM_COPY_WORD_SIZE;
                destination.bytes += CD_STREAM_COPY_WORD_SIZE;
            }
            previous_wrap_overflow = CD_STREAM_STATE.wrap_overflow;
            CD_STREAM_STATE.wrap_overflow = 0U;
            destination.bytes = destination.bytes + previous_wrap_overflow;
        }
        else
        {
            destination.bytes = CD_STREAM_BUFFER_START;
            CD_STREAM_STATE.bytes_buffered = unconsumed_bytes + transfer_size;
            linear_read_ptr = stream_state->read_ptr;
            aligned_buffer_start = CD_STREAM_BUFFER_START + alignment_padding;
            stream_state->write_ptr = aligned_buffer_start;
            stream_state->read_ptr = aligned_buffer_start;
            linear_source.bytes = (linear_read_ptr + bytes_consumed) - alignment_padding;
            linear_aligned_bytes = unconsumed_bytes + CD_STREAM_COPY_WORD_MASK;
            if (linear_aligned_bytes < 0)
            {
                linear_aligned_bytes = unconsumed_bytes + CD_STREAM_NEGATIVE_WORD_ROUNDING_BIAS;
            }
            linear_word_count = linear_aligned_bytes >> CD_BYTES_PER_WORD_SHIFT;
            linear_word_count--;
            for (; linear_word_count != -1; linear_word_count--)
            {
                *destination.words = *linear_source.words;
                linear_source.bytes += CD_STREAM_COPY_WORD_SIZE;
                destination.bytes += CD_STREAM_COPY_WORD_SIZE;
            }
        }
        published_state = &CD_STREAM_STATE;
        published_state->data_ready = TRUE;
        return destination.bytes;
    }

    current_read_ptr = stream_state->read_ptr;
    bytes_buffered = stream_state->bytes_buffered;
    wrap_write_offset = CD_STREAM_STATE.wrap_overflow;
    destination.bytes = current_read_ptr + bytes_buffered;

    // Continue in the lower wrap area when the upper span cannot fit this sector.
    if ((wrap_write_offset != 0) || ((destination.bytes + transfer_size) > CD_STREAM_BUFFER_LIMIT))
    {
        destination.bytes = CD_STREAM_WRAP_START + wrap_write_offset;
        if (stream_state->write_ptr >= (destination.bytes + transfer_size))
        {
            CD_STREAM_STATE.wrap_overflow = wrap_write_offset + transfer_size;
        }
        else
        {
            CD_STREAM_STATE.deferred_sectors += 1;
            return NULL;
        }
    }
    else
    {
        unconsumed_bytes = bytes_buffered;
        stream_state->bytes_buffered = unconsumed_bytes + transfer_size;
    }

    next_sector_dst = destination.bytes;
    if (bytes_remaining == transfer_size)
    {
        published_state = &CD_STREAM_STATE;
        published_state->input_complete = TRUE;
        next_sector_dst = destination.bytes;
        return next_sector_dst;
    }
    return next_sector_dst;
}

/**
 * @brief Decompresses a complete bytecode-encoded buffer.
 *
 * Skips the stream header and decodes without source or destination bounds.
 *
 * @param source Compressed stream including its one-byte header.
 * @param destination Output buffer.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/JFLMN
 */
void cdrom_decompress_buffer(u8* source, u8* destination)
{
    source++;
    while (cdrom_decompress_data(&source, &destination, CD_DECOMPRESS_UNBOUNDED_END, CD_DECOMPRESS_UNBOUNDED_END) != FALSE);
}

/**
 * @brief Clears the stream data-ready flag through volatile access.
 *
 * @param data_ready Flag to clear.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/Y4pUH
 */
void cdrom_clear_data_ready(u8* data_ready)
{
    volatile u8* volatile_flag = data_ready;

    *volatile_flag = FALSE;
}
