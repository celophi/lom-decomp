#include "cdrom.h"
#include "cdrom_internal.h"
#include "sdk/libetc.h"
#include "sdk/libcd.h"
#include "sdk/libpress.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "akao.h"
#include "movie.h"
#include "movie_state.h"

#define CD_BYTES_PER_WORD 4
#define CD_BYTES_PER_WORD_SHIFT 2
#define CD_STREAM_TIMEOUT_FRAMES 30
#define CD_STREAM_DECOMPRESS_GUARD_SIZE 280
#define CD_STREAM_DIRECT_MODE 0x1000
#define CD_STREAM_CHUNK_GUARD_SIZE 0x418
#define CD_STREAM_STAGING_START ((u8*)0x801DA000)
#define CD_STREAM_STAGING_END ((u8*)0x801DBBE8)
#define CD_STREAM_LZ_WINDOW_SIZE 0x1000

#define CD_RESOURCE_INDEX_INVALID 0xFFFE
#define CD_RESOURCE_INDEX_DEFAULT 0xFFFF
#define CD_COMMAND_NONE 0
#define CD_COMMAND_QUEUE_SIZE 16
#define CD_COMMAND_QUEUE_MASK (CD_COMMAND_QUEUE_SIZE - 1)
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
#define CD_IS_MULTIBYTE_ID_CHAR(character) (((character) >= 0x80 && (character) <= 0x9F) || ((character) >= 0xE0 && (character) <= 0xEF))
#define CD_DATA_SECTOR_WORDS 0x200
#define CD_BYTES_TO_WORDS(size) (((size) + (CD_BYTES_PER_WORD - 1)) >> CD_BYTES_PER_WORD_SHIFT)
#define CD_RECOVERY_SECTOR_RETRY_LIMIT 17
#define CD_DISC_READY_RETRY_LIMIT 13
#define CD_IDLE_STATUS_RETRY_LIMIT 11
#define CD_INIT_COMMAND_RETRY_FLAG 0x80
#define CD_INIT_COMMAND_MASK 0x7F

#define CD_SYSTEM_ADDRESS 0x801ED800
#define CD_SYSTEM (*(struct CdSystem*)CD_SYSTEM_ADDRESS)
/* Volatile view for the two sites that publish an intermediate status word before the final one. */
#define CD_SYSTEM_V (*(volatile struct CdSystem*)CD_SYSTEM_ADDRESS)
#define CD_RESOURCE_ENTRIES ((CdResourceEntry*)0x801ED998)
#define CD_SCRATCHPAD_BUFFER ((CdResourceEntry*)0x1F800000)

/** @brief A CD position as MSF fields, a packed word, or command bytes. */
typedef union
{
    CdlLOC pos;
    u32 raw;
    u8 bytes[sizeof(u32)];
} CdlLOCRaw;

/** @brief Disc location and byte length of one resource. */
typedef struct CdResourceEntry
{
    CdlLOCRaw location;
    s32 data_size;
} CdResourceEntry;

/** @brief One queued CD command and its transfer destination. */
typedef struct CdCommandQueueItem
{
    u8 command;
    u8 _pad01;
    u16 resource_index;
    CdResourceEntry* entry;
    void* dst_buffer;
    CdCommandCallback callback;
} CdCommandQueueItem;

/** @brief Pending commands in the CD request ring. */
typedef struct CdCommandQueue
{
    CdCommandQueueItem items[CD_COMMAND_QUEUE_SIZE];
} CdCommandQueue;

/** @brief Drive flags and sector-callback handshaking bytes. */
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

/** @brief Drive errors, queue ownership, and active-command flags. */
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

/** @brief Disc validation phases after a drive error. */
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

/** @brief Commands issued during disc validation. */
typedef enum CdRecoveryCommand
{
    CD_RECOVERY_COMMAND_SET_MODE = 0x20,
    CD_RECOVERY_COMMAND_READ_DISC_ID = 0x21,
    CD_RECOVERY_COMMAND_RETRY_READ = 0x22,
    CD_RECOVERY_COMMAND_COMPLETE = 0x23,
} CdRecoveryCommand;

/** @brief Drive reconfiguration phases. */
typedef enum CdReconfigureState
{
    CD_RECONFIGURE_STATE_FLUSH = 0,
    CD_RECONFIGURE_STATE_SET_MODE = 1,
    CD_RECONFIGURE_STATE_SET_FILTER = 2,
    CD_RECONFIGURE_STATE_WAIT = 3,
} CdReconfigureState;

/** @brief Pending drive reconfiguration commands. */
typedef enum CdReconfigureStep
{
    CD_RECONFIGURE_STEP_NONE = 0,
    CD_RECONFIGURE_STEP_SET_FILTER = 0x10,
    CD_RECONFIGURE_STEP_DEMUTE = 0x11,
    CD_RECONFIGURE_STEP_PAUSE = 0x12,
    CD_RECONFIGURE_STEP_COMPLETE = 0x13,
} CdReconfigureStep;

/** @brief Pending pause and mode-restore commands. */
typedef enum CdSyncCommand
{
    CD_SYNC_COMMAND_NONE = 0,
    CD_SYNC_COMMAND_PAUSE = 1,
    CD_SYNC_COMMAND_AUDIO_PAUSE = 2,
    CD_SYNC_COMMAND_RESTORE_MODE = 3,
} CdSyncCommand;

/** @brief Ordering of a new command and the previous sector transfer. */
typedef enum CdExecutionMode
{
    CD_EXECUTION_MODE_ASYNC = 0,
    CD_EXECUTION_MODE_COMMAND_THEN_READ = 1,
    CD_EXECUTION_MODE_READ_THEN_COMMAND = 2,
} CdExecutionMode;

/** @brief Routing of the two CD audio channels. */
typedef enum CdAudioMixMode
{
    CD_AUDIO_MIX_LEFT_TO_BOTH = 0,
    CD_AUDIO_MIX_BOTH_TO_LEFT = 1,
} CdAudioMixMode;

/** @brief Reasons a command cannot enter the queue. */
typedef enum CdQueueCommandError
{
    CD_QUEUE_ERROR_FULL = -1,
    CD_QUEUE_ERROR_INVALID_RESOURCE = -2,
    CD_QUEUE_ERROR_LOCKED = -3,
} CdQueueCommandError;

/** @brief Drive status reported to callers. */
typedef enum CdErrorStatus
{
    CD_ERROR_STATUS_NONE = 0,
    CD_ERROR_STATUS_SYNC_ERROR = 1,
    CD_ERROR_STATUS_DISC_CHECK_PENDING = 2,
    CD_ERROR_STATUS_INVALID_DISC = 3,
    CD_ERROR_STATUS_NO_DISC = 4,
    CD_ERROR_STATUS_RETRIES_EXHAUSTED = 5,
} CdErrorStatus;

/** @brief Controller opcodes absent from the SDK command enum. */
typedef enum CdControllerCommand
{
    CD_COMMAND_INIT = 0x0A,
    CD_COMMAND_SET_SESSION = 0x12,
    CD_COMMAND_UNUSED_17 = 0x17,
    CD_COMMAND_UNUSED_18 = 0x18,
    CD_COMMAND_TEST = 0x19,
    CD_COMMAND_GET_ID = 0x1A,
} CdControllerCommand;

/**
 * @brief CD command queue, transfer state, and recovery state.
 * @note sync_complete, current_command and init_command are shared with the libcd
 *       sync/ready callbacks, which run in interrupt context.
 */
typedef struct CdSystem
{
    CdStatusFlags status_flags;
    u8 audio_enabled;
    u8 playback_state;
    u8 pending_queue_count;
    u8 _pad07;
    u16 current_resource_index;
    u16 _pad0A;
    s32 current_data_size;
    s32 target_data_size;
    volatile u8 sync_complete;
    u8 init_state;
    volatile u8 current_command;
    volatile u8 init_command;
    u8 retry_count;
    u8 retry_counter;
    u8 last_command;
    u8 _pad1B;
    u16 resource_index;
    u16 _pad1E;
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
    u8 unk162[6];
    CdlCB previous_sync_callback;
    CdlCB previous_ready_callback;
    u8 disc_validation_id[32];
    CdResourceEntry default_cd_resource;
} CdSystem;

extern CdlCB g_cd_sync_callback_result;
extern CdlCB g_cd_ready_callback_result;
extern s32 g_cd_vsync_timestamp;
extern u8 g_cd_status_byte;
extern u8 g_cd_audio_enabled;
extern u8 g_cd_playback_state;
extern u32 g_cd_read_remaining_bytes;
extern CdlLOCRaw g_cd_disc_validation_location;
extern u8 g_cd_init_state;
extern u8 g_cd_defer_data_ready;
extern u8 g_cd_pending_queue_count;
extern CdSystem g_cd_system;
extern const u8 g_disc_validation_id[21];

s32 cdrom_recover(void);
void cdrom_complete_command(u8 intr, u8* result);
void cdrom_handle_recovery_sync(u8 intr, u8* result);
void cdrom_handle_ready_intr(u8 intr, u8* result);
void cdrom_process_sector(s32 execution_mode);
void cdrom_run_command(u8 command, u8* sector_buffer, s32 execution_mode);
void cdrom_verify_disc(u8 interrupt, u8* result);
void cdrom_handle_sync_error(void);
void cdrom_restore_callbacks(void);
s32 cdrom_enter_recovery_mode(void);

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
    s32 queue_count;
    CdStatusFlags* status_flags;
    s32 cd_result;

    while (CdInit() == 0)
    {
    }

    CdSetDebug(0);

    g_cd_sync_callback_result = CdSyncCallback(NULL);
    g_cd_ready_callback_result = CdReadyCallback(NULL);

    status_flags = &CD_SYSTEM.status_flags;


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

    for (queue_count = CD_COMMAND_QUEUE_SIZE - 1; queue_count != -1; queue_count--)
    {
        CD_SYSTEM.command_queue.items[queue_count].command = 0;
        CD_SYSTEM.command_queue.items[queue_count].resource_index = 0;
        CD_SYSTEM.command_queue.items[queue_count].dst_buffer = CD_SCRATCHPAD_BUFFER;
        CD_SYSTEM.command_queue.items[queue_count].entry = CD_SCRATCHPAD_BUFFER;
        CD_SYSTEM.command_queue.items[queue_count].callback = 0;
    }

    CD_SYSTEM.set_mode_param_blocking[0] = (CdlModeSpeed | CdlModeSize1);
    CD_SYSTEM.set_mode_param_blocking[1] = 0;
    CD_SYSTEM.set_mode_param_blocking[2] = 0;
    CD_SYSTEM.set_mode_param_blocking[3] = 0;

    while (CdControlB(CdlNop, NULL, &CD_SYSTEM.status_byte) == 0)
    {
    }

    if ((g_cd_status_byte & CdlStatShellOpen) != 0)
    {
        cd_result = CdDiskReady(1);

        while (cd_result != CdlComplete)
        {
            cd_result = CdDiskReady(0);
        }
    }

    while (CdControlB(CdlSetmode, CD_SYSTEM.set_mode_param_blocking, NULL) == 0)
    {
    }

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

    while (CdControlB(CdlPause, NULL, NULL) == 0)
    {
    }

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
 * @param destination Destination for decompressed data.
 *
 * @return Number of decompressed bytes written.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/SvWOg
 */
s32 cdrom_stream(s32 resource_index, u8* destination)
{
    s32 unprocessed_bytes;
    CdStreamCopyCursor relocation_dst;
    s32 bytes_buffered;
    s32 bytes_consumed;
    u8* previous_read_ptr;
    s32 copy_size;
    s32 overflow_size;
    s32 timestamp;
    s32 remaining_size;
    CdStreamCopyCursor relocation_src;
    u8* decompress_end;
    u8* destination_start;
    s32 alignment;

    while (cdrom_process_state() != 0)
    {
        VSync(0);
    }

    destination_start = destination;

    CD_STREAM_STATE.deferred_sectors = 0;
    CD_STREAM_STATE.data_ready = FALSE;
    CD_STREAM_STATE.input_complete = FALSE;
    CD_STREAM_STATE.bytes_consumed = 0;

    remaining_size = cdrom_queue_command(CdlReadN, resource_index, NULL, &cdrom_handle_stream_data) - 1;
    timestamp = VSync(-1);

    while (TRUE)
    {
        if (VSync(-1) < (timestamp + CD_STREAM_TIMEOUT_FRAMES))
        {
            if (CD_STREAM_STATE.data_ready != 1)
            {
                continue;
            }

            do
            {
                bytes_buffered = CD_STREAM_STATE.bytes_buffered;

                // Retain a guard region until the final input chunk is buffered.
                if (bytes_buffered < remaining_size)
                {
                    decompress_end = (CD_STREAM_STATE.read_ptr + bytes_buffered) - CD_STREAM_DECOMPRESS_GUARD_SIZE;
                }
                else
                {
                    decompress_end = CD_STREAM_STATE.read_ptr + remaining_size;
                }

                if (cdrom_decompress_data(&CD_STREAM_STATE.write_ptr, &destination, decompress_end, CD_DECOMPRESS_UNBOUNDED_END) == 0)
                {
                    return destination - destination_start;
                }
            } while (bytes_buffered != CD_STREAM_STATE.bytes_buffered);

            bytes_consumed = CD_STREAM_STATE.write_ptr - CD_STREAM_STATE.read_ptr;
            CD_STREAM_STATE.bytes_consumed = bytes_consumed;
            cdrom_clear_data_ready(&CD_STREAM_STATE.data_ready);
            remaining_size -= bytes_consumed;

            if (CD_STREAM_STATE.input_complete != 1)
            {
                timestamp = VSync(-1);
                continue;
            }

            if (CD_STREAM_STATE.wrap_overflow != 0)
            {
                overflow_size = CD_STREAM_STATE.wrap_overflow;
                unprocessed_bytes = CD_STREAM_STATE.bytes_buffered - bytes_consumed;
                alignment = unprocessed_bytes & CD_STREAM_COPY_WORD_MASK;
                relocation_dst.bytes = CD_STREAM_WRAP_START - unprocessed_bytes;
                previous_read_ptr = CD_STREAM_STATE.read_ptr;

                copy_size = CD_STREAM_COPY_WORD_SIZE - alignment;
                CD_STREAM_STATE.write_ptr = relocation_dst.bytes;
                CD_STREAM_STATE.read_ptr = relocation_dst.bytes;
                copy_size &= CD_STREAM_COPY_WORD_MASK;
                alignment = unprocessed_bytes + CD_STREAM_COPY_WORD_MASK;

                relocation_dst.bytes -= copy_size;
                relocation_src.bytes = (previous_read_ptr + bytes_consumed) - copy_size;

                CD_STREAM_STATE.bytes_buffered = overflow_size + unprocessed_bytes;
                unprocessed_bytes = alignment / CD_STREAM_COPY_WORD_SIZE;
                for (unprocessed_bytes--; unprocessed_bytes != -1; unprocessed_bytes--)
                {
                    *relocation_dst.words = *relocation_src.words;
                    relocation_src.bytes += CD_STREAM_COPY_WORD_SIZE;
                    relocation_dst.bytes += CD_STREAM_COPY_WORD_SIZE;
                }
            }
            else
            {
                CD_STREAM_STATE.read_ptr += bytes_consumed;
                CD_STREAM_STATE.bytes_buffered -= bytes_consumed;
            }

            CD_STREAM_STATE.data_ready = TRUE;

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
 * @see decomp.me: (100%) https://decomp.me/scratch/aZWx6
 */
void cdrom_stream_chunked(u16 resource_index, CdStreamGetBufferCallback get_buffer, CdStreamChunkDoneCallback chunk_done)
{
    s32 timestamp;
    u8 source_byte;
    s32 decompress_result;
    u32 source_word;
    s32 loop_count;
    u8* decompress_end;
    u32 alignment_check;
    u8* source_ptr;
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
    s32 copy_size;
    s32 loop_sentinel;
    CdStreamCopyCursor relocation_dst;
    u8* previous_read_ptr;
    u32 overflow_size;
    CdStreamState* stream_state;
    s32 guard_sentinel;
    u8** destination_ref;
    u8** staging_write_ref;

    CD_STREAM_STATE.deferred_sectors = 0;
    CD_STREAM_STATE.data_ready = FALSE;
    CD_STREAM_STATE.input_complete = FALSE;

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
        // Fixed-size output: reserve a guard region at the end of the chunk.
        destination_end = destination + chunk_bytes_remaining - CD_STREAM_CHUNK_GUARD_SIZE;
        direct_mode = 0;
    }

    source_ptr = CD_STREAM_STAGING_START;
    staging_write_ptr = source_ptr;
    staging_end = CD_STREAM_STAGING_END;

    timestamp = VSync(-1);
    stream_state = &CD_STREAM_STATE;
    guard_sentinel = -1;
    destination_ref = &destination;

    while (TRUE)
    {
        if (VSync(-1) < timestamp + CD_STREAM_TIMEOUT_FRAMES)
        {
            if (stream_state->data_ready != TRUE)
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

                source_ptr = staging_write_ptr;
                decompress_result = cdrom_decompress_data(&CD_STREAM_STATE.write_ptr, &staging_write_ptr, decompress_end, staging_end);

                staging_bytes_produced = staging_write_ptr - source_ptr;

                while (staging_bytes_produced != 0)
                {
                    if ((staging_bytes_produced < chunk_bytes_remaining) || (chunk_bytes_remaining == guard_sentinel))
                    {
                        total_bytes_delivered += staging_bytes_produced;
                        chunk_bytes_remaining -= staging_bytes_produced;

                        loop_count = (u32)destination & CD_STREAM_COPY_WORD_MASK;
                        if ((loop_count != 0) && (loop_count < staging_bytes_produced))
                        {
                            staging_bytes_produced -= loop_count;
                            loop_count--;
                            if (loop_count != guard_sentinel)
                            {
                                loop_sentinel = -1;
                                do
                                {
                                    u8* dst;
                                    source_byte = *source_ptr++;
                                    dst = *destination_ref;
                                    *dst = source_byte;
                                    *destination_ref = dst + 1;
                                    loop_count--;
                                } while (loop_count != loop_sentinel);
                            }
                        }

                        alignment_check = (u32)source_ptr & CD_STREAM_COPY_WORD_MASK;
                        if (alignment_check == 0)
                        {
                            loop_count = staging_bytes_produced >> CD_BYTES_PER_WORD_SHIFT;
                            staging_bytes_produced -= loop_count * CD_STREAM_COPY_WORD_SIZE;
                            loop_count--;
                            if (loop_count != guard_sentinel)
                            {
                                loop_sentinel = -1;
                                do
                                {
                                    u32* dst;
                                    source_word = *(u32*)source_ptr;
                                    source_ptr += CD_STREAM_COPY_WORD_SIZE;
                                    dst = (u32*)*destination_ref;
                                    *dst = source_word;
                                    *destination_ref = (u8*)(dst + 1);
                                    loop_count--;
                                } while (loop_count != loop_sentinel);
                            }
                        }

                        staging_bytes_produced--;
                        if (staging_bytes_produced != guard_sentinel)
                        {
                            loop_sentinel = -1;
                            do
                            {
                                u8* dst;
                                source_byte = *source_ptr++;
                                dst = *destination_ref;
                                *dst = source_byte;
                                *destination_ref = dst + 1;
                                staging_bytes_produced--;
                            } while (staging_bytes_produced != loop_sentinel);
                        }

                        break;
                    }

                    staging_bytes_produced -= chunk_bytes_remaining;
                    total_bytes_delivered += chunk_bytes_remaining;
                    chunk_bytes_remaining--;

                    if (chunk_bytes_remaining != guard_sentinel)
                    {
                        loop_sentinel = -1;
                        do
                        {
                            u8* dst = *destination_ref;
                            *dst = *source_ptr;
                            *destination_ref = dst + 1;
                            source_ptr++;
                            chunk_bytes_remaining--;
                        } while (chunk_bytes_remaining != loop_sentinel);
                    }

                    if (staging_bytes_produced > 0 || decompress_result != 0)
                    {
                        chunk_done(chunk_index++);
                        destination = get_buffer(total_bytes_delivered, &chunk_bytes_remaining);
                    }
                }

                if (decompress_result != 0)
                {
                    s32 window_sentinel;
                    staging_write_ptr = CD_STREAM_STAGING_START;
                    source_ptr = source_ptr - CD_STREAM_LZ_WINDOW_SIZE;
                    staging_bytes_produced = CD_STREAM_LZ_WINDOW_SIZE - 1;
                    staging_write_ref = &staging_write_ptr;
                    window_sentinel = -1;

                    // Carry the LZ history to the front of staging so back-references stay valid.
                    do
                    {
                        u8* dst;
                        source_byte = *source_ptr++;
                        dst = *staging_write_ref;
                        staging_bytes_produced--;
                        *dst = source_byte;
                        *staging_write_ref = dst + 1;
                    } while (staging_bytes_produced != window_sentinel);

                    continue;
                }

                chunk_done(chunk_index);
                return;

            } while (bytes_buffered != CD_STREAM_STATE.bytes_buffered);

            bytes_buffered = stream_state->write_ptr - stream_state->read_ptr;
            previous_read_ptr = stream_state->read_ptr;

            stream_state->data_ready = FALSE;
            stream_state->bytes_consumed = bytes_buffered;
            remaining_size -= bytes_buffered;

            if (stream_state->input_complete != TRUE)
            {
                timestamp = VSync(-1);
                continue;
            }

            overflow_size = stream_state->wrap_overflow;

            if (stream_state->wrap_overflow != 0)
            {
                staging_bytes_produced = stream_state->bytes_buffered - bytes_buffered;
                copy_size = (staging_bytes_produced & CD_STREAM_COPY_WORD_MASK);
                alignment_check = CD_STREAM_COPY_WORD_SIZE - copy_size;
                loop_count = alignment_check & CD_STREAM_COPY_WORD_MASK;

                relocation_dst.bytes = CD_STREAM_WRAP_START - staging_bytes_produced;
                previous_read_ptr = (previous_read_ptr + bytes_buffered) - loop_count;

                stream_state->write_ptr = relocation_dst.bytes;
                stream_state->read_ptr = relocation_dst.bytes;
                relocation_dst.bytes = relocation_dst.bytes - loop_count;

                stream_state->bytes_buffered = overflow_size + staging_bytes_produced;
                staging_bytes_produced = (staging_bytes_produced + CD_STREAM_COPY_WORD_MASK) / CD_STREAM_COPY_WORD_SIZE;
                staging_bytes_produced--;

                if (staging_bytes_produced != guard_sentinel)
                {
                    do
                    {
                        *relocation_dst.words = *(u32*)previous_read_ptr;
                        previous_read_ptr += CD_STREAM_COPY_WORD_SIZE;
                        relocation_dst.bytes += CD_STREAM_COPY_WORD_SIZE;
                        staging_bytes_produced--;
                    } while (staging_bytes_produced != -1);
                }
            }
            else
            {
                stream_state->read_ptr = previous_read_ptr + bytes_buffered;
                stream_state->bytes_buffered -= bytes_buffered;
            }

            stream_state->data_ready = TRUE;
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
        resource_entry = &CD_SYSTEM.default_cd_resource;
    }
    else
    {
        resource_entry = &CD_RESOURCE_ENTRIES[resource_index];
    }

    // Suppress only consecutive duplicate commands while the drive is busy.
    if ((CD_SYSTEM.current_command == 0 && CD_SYSTEM.init_command == 0) || (CD_SYSTEM.last_command != command) ||
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
 * @return Number of queued commands; unspecified on some recovery paths.
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
                CD_SYSTEM.current_resource_index = CD_SYSTEM.command_queue.items[read_index].resource_index;
                CD_SYSTEM.current_data_size = CD_SYSTEM.command_queue.items[read_index].entry->data_size;
                CD_SYSTEM.target_data_size = CD_SYSTEM.read_remaining_bytes;
            }

            if (CD_SYSTEM.audio_enabled != 0)
            {
                if (g_movie_use_cd_audio != 0)
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

            g_cd_data_ready_pending = 0;
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
                    CD_SYSTEM.init_command = CD_RECOVERY_COMMAND_SET_MODE;
                    CdControlF(CdlSetmode, CD_SYSTEM.set_mode_param_async);
                    CD_SYSTEM.vsync_timestamp -= CD_STATUS_POLL_FRAMES - CD_SET_MODE_DELAY_FRAMES;
                    break;

                case CD_RECOVERY_STATE_READ_DISC_ID:
                    CD_SYSTEM.recovery_read_position.raw = g_cd_disc_validation_location.raw;
                    CD_SYSTEM.status_flags.word |= CD_STATUS_COMMAND_ACTIVE;
                    CdSyncCallback(cdrom_handle_recovery_sync);
                    CdReadyCallback(cdrom_verify_disc);
                    CD_SYSTEM.init_command = CD_RECOVERY_COMMAND_READ_DISC_ID;
                    CD_SYSTEM.init_state = CD_RECOVERY_STATE_WAIT_FOR_READ;
                    CdControlF(CdlReadN, CD_SYSTEM.recovery_read_position.bytes);
                    CD_SYSTEM.vsync_timestamp -= CD_STATUS_POLL_FRAMES;
                    break;

                case CD_RECOVERY_STATE_WAIT_FOR_READ:
                    if (CD_SYSTEM.sync_complete == 1)
                    {
                        CD_SYSTEM.vsync_timestamp = VSync(-1);
                        CD_SYSTEM.sync_complete = 0;
                    }
                    else if (VSync(-1) >= (CD_SYSTEM.vsync_timestamp + CD_RECOVERY_READ_TIMEOUT_FRAMES))
                    {
                        recovery_command = CD_SYSTEM.init_command;

                        switch (recovery_command)
                        {
                        case CD_RECOVERY_COMMAND_READ_DISC_ID:
                        default:
                            CdSyncCallback(cdrom_handle_recovery_sync);
                            CdReadyCallback(cdrom_verify_disc);
                            CD_SYSTEM.init_command = CD_RECOVERY_COMMAND_READ_DISC_ID;
                            cd_command = CdlReadN;
                            command_params = CD_SYSTEM.recovery_read_position.bytes;
                            break;
                        case CD_RECOVERY_COMMAND_RETRY_READ:
                            CdSyncCallback(cdrom_handle_recovery_sync);
                            cd_command = CdlPause;
                            command_params = NULL;
                            break;
                        case CD_RECOVERY_COMMAND_COMPLETE:
                            CdSyncCallback(cdrom_handle_recovery_sync);
                            cd_command = CdlSetmode;
                            command_params = CD_SYSTEM.set_mode_param_blocking;
                            break;
                        }

                        CdControlF(cd_command, command_params);
                        CD_SYSTEM.vsync_timestamp -= CD_STATUS_POLL_FRAMES;
                    }
                    break;

                case CD_INIT_STATE_ERROR_PAUSE:
                    while (CdControlB(CdlStop, NULL, NULL) == 0)
                    {
                    }
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
                    while (CdControlB(CdlPause, NULL, NULL) == 0)
                    {
                    }
                    CD_SYSTEM.init_command = 0;
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
                if (CD_SYSTEM.sync_complete == 1)
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
                        CD_SYSTEM.current_command = CdlNop;

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
        movie_service_video_ops();
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
            CD_SYSTEM.init_command = CD_RECONFIGURE_STEP_SET_FILTER;
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
            CD_SYSTEM.sync_complete = 0;
            break;
        }

        timestamp = VSync(-1);
        if (timestamp < (CD_SYSTEM.vsync_timestamp + CD_STATUS_POLL_FRAMES))
        {
            break;
        }

        CdSyncCallback(cdrom_handle_recovery_sync);

        reconfigure_step = CD_SYSTEM.init_command;

        switch (reconfigure_step)
        {
        case CD_RECONFIGURE_STEP_NONE:
        default:
            filter_params[0] = CD_RECOVERY_FILTER_FILE;
            filter_params[1] = CD_RECOVERY_FILTER_CHANNEL;
            CdControlF(CdlSetfilter, filter_params);
            CD_SYSTEM.init_command = CD_RECONFIGURE_STEP_SET_FILTER;
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
    CdSystem* cd_system = &CD_SYSTEM;

    if (g_cd_data_ready_pending != CD_READY_CALLBACK_PENDING)
    {
        return;
    }

    if (cd_system->audio_enabled != g_cd_data_ready_pending)
    {
        while (CdGetSector(CD_SYSTEM.sector_header_buffer, CD_SECTOR_HEADER_WORDS) == 0)
        {
        }

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
            CD_SYSTEM.current_command = CdlNop;
            CdControlF(CdlNop, NULL);
        }
    }
    else
    {
        cdrom_process_sector(TRUE);
    }

    g_cd_data_ready_pending = FALSE;
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
    CdSystem* cd_system;
    CdSystem* queue_system;

    CD_SYSTEM.sync_complete = TRUE;

    if ((CD_SYSTEM.current_command == CdlNop) && (*result & CdlStatShellOpen))
    {
        cdrom_handle_sync_error();
        return;
    }

    if (intr == CdlComplete)
    {
        switch (CD_SYSTEM.current_command)
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
            next_command = CD_SYSTEM.command_queue.items[CD_SYSTEM.queue_read_index].command;

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
                CD_SYSTEM.current_command = 0;
                CD_SYSTEM.init_command = 0;
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
    CdStatusFlags status;
    s32 write_index;
    s32 read_index;
    u8 filter_params[2];
    u8 next_command;
    MovieState* movie_state;

    CD_SYSTEM.sync_complete = TRUE;

    if (((s8)CD_SYSTEM.init_command < 0) && !(CD_SYSTEM.status_flags.word & CD_STATUS_RECOVERY_PENDING) && (*result & CdlStatShellOpen))
    {
        cdrom_handle_sync_error();
        return;
    }

    if (((CD_SYSTEM.init_command & CD_INIT_COMMAND_MASK) == CD_RECOVERY_COMMAND_READ_DISC_ID) && (CD_SYSTEM.status_byte & CdlStatError))
    {
        if (CD_SYSTEM.mode_flags & CdlModeRT)
        {
            CdSyncCallback(NULL);
            CdReadyCallback(NULL);
            CD_SYSTEM.init_state = CD_INIT_STATE_ERROR_PAUSE;
            CD_SYSTEM.init_command = 0;
            CD_SYSTEM.status_flags.word &= ~CD_STATUS_COMMAND_ACTIVE;
            CD_SYSTEM.status_flags.word &= ~CD_STATUS_NO_DISC;
        }
    }

    if (intr == CdlComplete)
    {
        CD_SYSTEM.init_command &= CD_INIT_COMMAND_MASK;

        switch (CD_SYSTEM.init_command)
        {
        case CD_SYNC_COMMAND_PAUSE:
        case CD_SYNC_COMMAND_RESTORE_MODE:
            CD_SYSTEM.init_command = 0;
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
            CD_SYSTEM.init_command++;
            CdControlF(CdlSetmode, CD_SYSTEM.set_mode_param_blocking);
            break;
        case CD_RECONFIGURE_STEP_SET_FILTER:
            CD_SYSTEM.init_state = CD_RECOVERY_STATE_CHECK_DISC;
            CdSyncCallback(NULL);
            CD_SYSTEM.init_command = 0;
            break;
        case CD_RECONFIGURE_STEP_DEMUTE:
            CD_SYSTEM.init_command++;
            CdControlF(CdlDemute, NULL);
            break;
        case CD_RECONFIGURE_STEP_PAUSE:
            CD_SYSTEM.init_command++;
            CdControlF(CdlPause, NULL);
            break;
        case CD_RECONFIGURE_STEP_COMPLETE:
            CdSyncCallback(NULL);
            CD_SYSTEM.init_state = 0;
            CD_SYSTEM.init_command = 0;
            CD_SYSTEM.status_flags.word &= ~CD_STATUS_RECOVERY_PENDING;
            break;
        case CD_RECOVERY_COMMAND_READ_DISC_ID:
            CdSyncCallback(NULL);
            CD_SYSTEM.init_command = 0;
            break;
        case CD_RECOVERY_COMMAND_SET_MODE:
        case CD_RECOVERY_COMMAND_RETRY_READ:
            CD_SYSTEM.init_state = CD_RECOVERY_STATE_READ_DISC_ID;
            CdSyncCallback(NULL);
            CD_SYSTEM.init_command = 0;
            break;
        case CD_RECOVERY_COMMAND_COMPLETE:
            CD_SYSTEM.init_command = 0;
            CD_SYSTEM.init_state = 0;
            CD_SYSTEM.retry_counter = 0;

            status.word = CD_SYSTEM.status_flags.word;

            read_index = CD_SYSTEM.queue_read_index;
            write_index = CD_SYSTEM.queue_write_index;

            status.word &= ~CD_STATUS_SYNC_ERROR;
            CD_SYSTEM_V.status_flags.word = status.word;
            status.word &= ~CD_STATUS_INVALID_DISC;
            status.word &= ~CD_STATUS_NO_DISC;

            CD_SYSTEM.status_flags.word = status.word;
            if (read_index != write_index)
            {
                CD_SYSTEM.current_command = CdlNop;
                CD_SYSTEM.status_flags.word = status.word | CD_STATUS_COMMAND_ACTIVE;
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
                movie_state = MOVIE_STATE;
                if (g_movie_use_cd_audio != 0)
                {
                    movie_state->audio_stream_state = TRUE;
                }
            }
            break;
        }
        g_cd_vsync_timestamp = VSync(-1);
        return;
    }
    if ((s8)CD_SYSTEM.init_command >= 0)
    {
        CD_SYSTEM.init_command |= CD_INIT_COMMAND_RETRY_FLAG;
        CdControlF(CdlNop, NULL);
        return;
    }

    CD_SYSTEM.init_command &= CD_INIT_COMMAND_MASK;

    switch (CD_SYSTEM.init_command)
    {
    case CD_SYNC_COMMAND_RESTORE_MODE:
        CdControlF(CdlSetmode, CD_SYSTEM.set_mode_param_blocking);
        return;
    case CD_RECONFIGURE_STEP_SET_FILTER:
        CD_SYSTEM.init_state = CD_RECOVERY_STATE_POLL_STATUS;
        CdSyncCallback(NULL);
        CD_SYSTEM.init_command = 0;
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
        CD_SYSTEM.init_command = 0;
        CdSyncCallback(NULL);
        return;
    case CD_RECOVERY_COMMAND_SET_MODE:
    case CD_RECOVERY_COMMAND_COMPLETE:
        CD_SYSTEM.init_state = CD_RECOVERY_STATE_SET_MODE;
        CD_SYSTEM.init_command = 0;
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
    MovieState* movie_state;

    CD_SYSTEM.sync_complete = TRUE;
    audio_enabled = CD_SYSTEM.audio_enabled;

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

            while (CdGetSector(CD_SYSTEM.sector_header_buffer, CD_SECTOR_HEADER_WORDS) == 0)
            {
            }

            if ((CD_SYSTEM.sector_header_buffer[0] & CD_SECTOR_POSITION_MASK) == (CD_SYSTEM.current_location.raw & CD_SECTOR_POSITION_MASK))
            {
                cdrom_process_sector(FALSE);
                return;
            }
        }

        if (CD_SYSTEM.retry_count++ < CD_RECOVERY_SECTOR_RETRY_LIMIT)
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

        CD_SYSTEM.current_command = CdlNop;
        CdControlF(CdlNop, NULL);
        return;
    }

    // XA delivery waits while the movie decoder owns the shared pipeline.
    ready_state = audio_enabled;
    if (intr == ready_state)
    {
        movie_state = MOVIE_STATE;
        if ((g_gpu_mode == 0) && (movie_state->mdec_busy != 0))
        {
            CD_SYSTEM.status_flags.bytes.data_ready_pending = ready_state;
            return;
        }
        cdrom_process_sector(FALSE);
        return;
    }

    if (CD_SYSTEM.retry_count++ >= CD_RECOVERY_SECTOR_RETRY_LIMIT)
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
                CdControlF(CD_SYSTEM.current_command, CD_SYSTEM.current_location.bytes);
                return;
            }
        }
        else
        {
            buffer = CD_SYSTEM.current_write_ptr;
        }

        if (CD_SYSTEM.read_remaining_bytes >= (CD_DATA_SECTOR_SIZE + 1))
        {
            while (CdGetSector(buffer, CD_DATA_SECTOR_WORDS) == 0)
            {
            }
            CdIntToPos(CdPosToInt(&CD_SYSTEM.current_location.pos) + 1, &CD_SYSTEM.current_location.pos);
            CD_SYSTEM.read_remaining_bytes -= CD_DATA_SECTOR_SIZE;
            if (CD_SYSTEM.transfer_callback == NULL)
            {
                CD_SYSTEM.current_write_ptr += CD_DATA_SECTOR_SIZE;
            }
        }
        else
        {
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

            while (CdGetSector(buffer, CD_BYTES_TO_WORDS(g_cd_read_remaining_bytes)) == 0)
            {
            }

            CD_SYSTEM.status_flags.word &= ~CD_STATUS_COMMAND_ACTIVE;
            CD_SYSTEM.current_command = 0;
            CD_SYSTEM.retry_counter = 0;
            if (execution_mode != CD_EXECUTION_MODE_ASYNC)
            {
                CdControlF(CdlPause, NULL);
            }
            CD_SYSTEM.vsync_timestamp = VSync(-1);
        }

        return;
    }

    while (CdGetSector(CD_SYSTEM.sector_header_buffer, CD_SECTOR_HEADER_WORDS) == 0)
    {
    }

    if ((CD_SYSTEM.sector_header_buffer[0] & CD_SECTOR_POSITION_MASK) == (CD_SYSTEM.current_location.raw & CD_SECTOR_POSITION_MASK))
    {
        if (CD_SYSTEM.transfer_callback(CD_SYSTEM.total_data_size - CD_SYSTEM.read_remaining_bytes, CD_SYSTEM.read_remaining_bytes) == NULL)
        {
            CD_SYSTEM.queue_read_index = (CD_SYSTEM.queue_read_index + 1) & CD_COMMAND_QUEUE_MASK;
            CdSyncCallback(cdrom_handle_recovery_sync);
            CdReadyCallback(NULL);
            CD_SYSTEM.set_mode_param_blocking[0] = CdlModeSpeed | CdlModeSize1;
            CD_SYSTEM.current_command = 0;
            CD_SYSTEM.init_command = CD_SYNC_COMMAND_AUDIO_PAUSE;
            CD_SYSTEM.audio_enabled = FALSE;
            CD_SYSTEM.playback_state = FALSE;
            CD_SYSTEM.transfer_callback = NULL;
            CD_SYSTEM.retry_counter = 0;
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

    CdControlF(CD_SYSTEM.current_command, CD_SYSTEM.current_location.bytes);
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
    s32 entry_index;
    s32 buffer_index;
    CdSystem* cd_system;

    queued_resource = NULL;

    // Skip seeks that are superseded by another pending command.
    while (command == CdlSeekL)
    {
        next_read_index = (CD_SYSTEM.queue_read_index + 1) & CD_COMMAND_QUEUE_MASK;

        if (CD_SYSTEM.queue_write_index == next_read_index)
        {
            break;
        }

        CD_SYSTEM.queue_read_index = next_read_index;
        command = CD_SYSTEM.command_queue.items[next_read_index].command;
    }

    if ((command == CdlSeekL) || (command == CdlReadN) || (command == CdlReadS))
    {
        if ((command == CdlSeekL) || (g_cd_playback_state == FALSE))
        {
            CD_SYSTEM.transfer_callback = NULL;
            CD_SYSTEM.playback_state = FALSE;
            queued_resource = CD_SYSTEM.command_queue.items[CD_SYSTEM.queue_read_index].entry;
            CD_SYSTEM.current_location = queued_resource->location;
        }

        switch (execution_mode)
        {
        case CD_EXECUTION_MODE_COMMAND_THEN_READ:
            CD_SYSTEM.current_command = command;
            CdControlF(command, CD_SYSTEM.current_location.bytes);
            while (CdGetSector(sector_buffer, CD_BYTES_TO_WORDS(g_cd_read_remaining_bytes)) == 0)
            {
            }
            break;

        case CD_EXECUTION_MODE_READ_THEN_COMMAND:
            while (CdGetSector(sector_buffer, CD_BYTES_TO_WORDS(g_cd_read_remaining_bytes)) == 0)
            {
            }
            CdSync(0, NULL);
            break;
        }

        if ((command == CdlReadN) || (command == CdlReadS))
        {
            entry_index = CD_SYSTEM.queue_read_index;
            if ((CD_SYSTEM.command_queue.items[entry_index].callback == NULL) &&
                (CD_SYSTEM.current_write_ptr == CD_SYSTEM.command_queue.items[entry_index].dst_buffer))
            {
                CD_SYSTEM.playback_state = FALSE;
            }

            cd_system = &CD_SYSTEM;
            if (g_cd_playback_state == FALSE)
            {
                data_size = queued_resource->data_size;
                buffer_index = cd_system->queue_read_index;
                CD_SYSTEM.total_data_size = data_size;
                CD_SYSTEM.read_remaining_bytes = data_size;
                CD_SYSTEM.current_write_ptr = CD_SYSTEM.command_queue.items[buffer_index].dst_buffer;
                CD_SYSTEM.transfer_callback = CD_SYSTEM.command_queue.items[buffer_index].callback;
            }

            if (execution_mode == CD_EXECUTION_MODE_ASYNC)
            {
                CD_SYSTEM.status_flags.bytes.data_ready_pending = FALSE;
                CdReadyCallback(cdrom_handle_ready_intr);
            }
        }
        else if (execution_mode == CD_EXECUTION_MODE_COMMAND_THEN_READ)
        {
            CdReadyCallback(NULL);
        }

        if (execution_mode != CD_EXECUTION_MODE_COMMAND_THEN_READ)
        {
            CD_SYSTEM.current_command = command;
            CdControlF(command, CD_SYSTEM.current_location.bytes);
        }

        g_cd_playback_state = FALSE;
        return;
    }

    switch (execution_mode)
    {
    case CD_EXECUTION_MODE_ASYNC:
        CD_SYSTEM.current_command = command;

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
        CD_SYSTEM.current_command = command;
        control_command = CdlNop;
        CdControlF(command, NULL);
        while (CdGetSector(sector_buffer, CD_BYTES_TO_WORDS(g_cd_read_remaining_bytes)) == 0)
        {
        }
        return;

    case CD_EXECUTION_MODE_READ_THEN_COMMAND:
        while (CdGetSector(sector_buffer, CD_BYTES_TO_WORDS(g_cd_read_remaining_bytes)) == 0)
        {
        }
        CD_SYSTEM.current_command = command;
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

    CD_SYSTEM.sync_complete = TRUE;

    if (interrupt == CdlDataReady)
    {
        while ((expected_character = (CdGetSector(CD_SYSTEM.sector_header_buffer, CD_SECTOR_HEADER_WORDS) == 0)))
        {
        }

        if ((CD_SYSTEM.sector_header_buffer[0] & CD_SECTOR_POSITION_MASK) == (CD_SYSTEM.recovery_read_position.raw & CD_SECTOR_POSITION_MASK))
        {
            while (CdGetSector(CD_SYSTEM.disc_validation_id, CD_DISC_VALIDATION_WORDS) == 0)
            {
            }

            expected_id = g_disc_validation_id;
            disc_id = CD_SYSTEM.disc_validation_id;
            expected_character = *expected_id++;

            while (expected_character != '\0')
            {
                // Multibyte ID characters must match both encoded bytes.
                if (CD_IS_MULTIBYTE_ID_CHAR(expected_character))
                {
                    disc_character = *disc_id++;
                    if (expected_character == disc_character)
                    {
                        expected_character = *disc_id++;
                        disc_character = *expected_id++;
                    }
                }
                else
                {
                    disc_character = *disc_id++;
                }

                if (expected_character != disc_character)
                {
                    status_flags = CD_SYSTEM.status_flags.word & ~CD_STATUS_NO_DISC;
                    CD_SYSTEM.init_state = CD_INIT_STATE_ERROR_PAUSE;
                    CD_SYSTEM_V.status_flags.word = status_flags;
                    CD_SYSTEM.status_flags.word = status_flags & ~CD_STATUS_COMMAND_ACTIVE;
                    CdReadyCallback(NULL);
                    return;
                }

                expected_character = *expected_id++;
            }

            CdReadyCallback(NULL);
            CD_SYSTEM.init_command = CD_RECOVERY_COMMAND_COMPLETE;
            CdSyncCallback(cdrom_handle_recovery_sync);
            CdControlF(CdlSetmode, CD_SYSTEM.set_mode_param_blocking);
            return;
        }
    }

    CdReadyCallback(NULL);
    CD_SYSTEM.init_command = CD_RECOVERY_COMMAND_RETRY_READ;
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

    if (mix_mode != 0)
    {
        audio_mix.val0 = volume;
        audio_mix.val1 = 0;
        audio_mix.val2 = volume;
        audio_mix.val3 = 0;
    }
    else
    {
        audio_mix.val0 = volume;
        audio_mix.val1 = volume;
        audio_mix.val2 = 0;
        audio_mix.val3 = 0;
    }

    CdMix(&audio_mix);
}

/**
 * @brief Stops CD/XA playback and resets command and callback state.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/fnucZ
 */
void cdrom_reset(void)
{
    MovieState* movie_state = MOVIE_STATE;

    DecDCToutCallback(movie_state->dec_dct_out_callback.handler);
    DrawSyncCallback(movie_state->draw_sync_callback.handler);

    CdSyncCallback(NULL);
    CdReadyCallback(NULL);

    while (CdControlB(CdlPause, NULL, NULL) == 0)
    {
    }

    if (g_movie_use_cd_audio != FALSE)
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
    s32 vsync_offset;
    s32 vsync_delta;

    vsync_offset = CD_RESOURCE_LOAD_VSYNC_OFFSET;
    vsync_delta = g_cd_vsync_timestamp - (VSync(-1) + vsync_offset);

    if (vsync_delta > 0)
    {
        if (vsync_delta == 1)
        {
            vsync_delta = 0;
        }

        VSync(vsync_delta);
    }

    CD_SYSTEM.default_cd_resource.location.raw = 0;
    CD_SYSTEM.default_cd_resource.data_size = data_size_bytes;

    CdIntToPos(lba, &CD_SYSTEM.default_cd_resource.location.pos);
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
 * @return Resource size, or a negative queue error.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/OxunQ
 */
s32 cdrom_queue_read(s32 resource_index, void* dst_buffer)
{
    return cdrom_queue_command(CdlReadN, resource_index, dst_buffer, NULL);
}

/**
 * @brief Queues a resource read handled by a transfer callback.
 *
 * @param resource_index Resource table index.
 * @param callback Transfer callback.
 *
 * @return Resource size, or a negative queue error.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/5M5cV
 */
s32 cdrom_queue_read_with_callback(s32 resource_index, CdCommandCallback callback)
{
    return cdrom_queue_command(CdlReadN, resource_index, NULL, callback);
}

/**
 * @brief Queues a logical seek to a resource.
 *
 * @param resource_index Resource table index.
 *
 * @return Resource size, or a negative queue error.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/iUUQh
 */
s32 cdrom_queue_seek(s32 resource_index)
{
    return cdrom_queue_command(CdlSeekL, resource_index, NULL, NULL);
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

    while (CdControlB(CdlPause, NULL, NULL) == 0)
    {
    }

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
