#include "cd.h"
#include "psyq/libetc.h"
#include "psyq/libcd.h"
#include "psyq/libpress.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "akao.h"

#define CD_RESOURCE_INDEX_INVALID 0xFFFE
#define CD_RESOURCE_INDEX_DEFAULT 0xFFFF
#define CD_COMMAND_QUEUE_SIZE 16
#define CD_INIT_STATE_ERROR_PAUSE 0x20

typedef void (*DecDCToutCallbackHandler)();
typedef void (*DrawSyncCallbackHandler)();
typedef CdStreamGetBufferCallback codeA;
typedef CdStreamChunkDoneCallback codeB;

typedef union
{
    CdlLOC pos;
    u32 raw;
} CdlLOCRaw;

typedef struct CdResourceEntry
{
    CdlLOCRaw location;
    int dataSize;
} CdResourceEntry;

typedef struct CdCommandQueueItem
{
    u_char command;
    u_char padding;
    unsigned short resourceIndex;
    CdResourceEntry* entry;
    CdResourceEntry* dstBuffer;
    CdCommandCallback callback;
} CdCommandQueueItem;

typedef struct CdCommandQueue
{
    CdCommandQueueItem items[CD_COMMAND_QUEUE_SIZE];
} CdCommandQueue;

typedef union
{
    u_int word;
    struct
    {
        u_char b0;
        u_char b1;
        u_char b2;
        u_char retryExhausted;
    } bytes;
} CdStatusFlags;

typedef struct CdSystem
{
    CdStatusFlags statusFlags;
    undefined1 audioEnabled;
    undefined1 playbackState;
    u8 pendingQueueCount;
    u8 padding_0x7;
    undefined2 currentResourceIndex;
    u16 padding_0xA;
    undefined4 currentDataSize;
    undefined4 targetDataSize;
    undefined1 syncComplete;
    undefined1 initState;
    undefined1 currentCommand;
    undefined1 initCommand;
    undefined1 retryCount;
    undefined1 retryCounter;
    undefined1 lastCommand;
    u8 padding_0x1B;
    undefined2 resourceIndex;
    u16 padding_0x1E;
    CdResourceEntry* dstBuffer;
    CdCommandCallback callback;
    u32 readRemainingBytes;
    u32 totalDataSize;
    void* currentWritePtr;
    CdCommandCallback transferCallback;
    undefined4 queueReadIndex;
    undefined4 queueWriteIndex;
    CdCommandQueue commandQueue;
    u32 sectorHeaderBuffer[3];
    undefined4 vsyncTimestamp;
    u_char setModeParamBlocking[4];
    u_char setModeParamAsync[4];
    CdlLOCRaw currentLocation;
    CdlLOCRaw recoveryReadPosition;
    undefined1 statusByte;
    undefined1 filterModeFlags;
    u8 u_162;
    u8 u_163;
    u32 u_164;
    CdlCB previousSyncCallback;
    CdlCB previousReadyCallback;
    u_char discValidationId[32];
    CdResourceEntry defaultCdResource;
} CdSystem;

typedef struct
{
    u8 dataReady;
    u8 bufferWrapped;
    u8 pad[2];
    s32 readPtr;
    s32 writePtr;
    s32 bytesBuffered;
    s32 wrapOverflow;
    s32 bytesConsumed;
    s32 dropped_sectors;
} CdStreamState;

typedef struct
{
    u8 u_0[0x38];
    DecDCToutCallbackHandler decDCToutCallbackHandler;
    DrawSyncCallbackHandler drawSyncCallbackHandler;
    u8 u_1[82];
    u8 readFlag;
    u8 u2[3];
} AudioSystem;

typedef struct SKCDPOSE_DAT
{
    CdResourceEntry resources[178];
    char unknown[45065];
} SKCDPOSE_DAT;

extern CdlCB g_cdSyncCallbackResult;
extern CdlCB g_cdReadyCallbackResult;
extern int g_cdVSyncTimestamp;
extern u_char g_cdStatusByte;
extern u_char g_cdAudioEnabled;
extern u_char g_cdAudioReady;
extern u8 g_playbackState;
extern u32 g_cdReadRemainingBytes;
extern s32 g_cdResource176;
extern s8 g_cdStatusByte3;
extern u8 g_initState;
extern s8 D_801ED801;
extern u8 g_cdPendingQueueCount;
extern CdSystem g_cdSystem;
extern const u_char g_DiscValidationId[21];
extern u8 D_801ED590;

#define CD_SYSTEM (*(struct CdSystem*)0x801ED800)
#define CD_SYSTEM_V (*(volatile CdSystem*)0x801ED800)
#define AUDIO_SYSTEM (*(AudioSystem*)0x801ED500)
#define CD_SECTOR_HEADER_BUFFER (*(u32*)0x801ED940)
#define CD_COMMAND_PARAM_BUFFER ((u_char*)0x801ED958)
#define g_defaultCdResource (*(CdResourceEntry*)0x801ED990)
#define CD_RESOURCE_ENTRIES ((CdResourceEntry*)0x801ED998)
#define g_commandQueueOffset (*(CdCommandQueueItem*)0x801ED8F0)
#define SCRATCHPAD ((void*)0x1F800000)
#define CD_STREAM_STATE (*(CdStreamState*)0x1F800000)
#define CdControlF_1(cmd) ((int (*)(u_char))CdControlF)(cmd)
#define QUEUE_ITEM_BASE(idx) ((void*)(((idx) * 0x10) + (u8*)&CD_SYSTEM))
#define QUEUE_ITEM_DST_BUFFER(ptr) (*((u32*)(ptr) + 0x12))
#define QUEUE_ITEM_CALLBACK(ptr) (*((CdCommandCallback*)(ptr) + 0x13))
#define VCD (*(volatile CdSystem*)0x801ED800)

int cdrom_recover(void);
void cdrom_complete_command(u_char intr, u_char* result);
void cdrom_handle_recovery_sync(u_char intr, u_char* result);
void cdrom_handle_ready_intr(u_char intr, u_char* result);
void cdrom_process_sector(s32 arg0);
void cdrom_run_command(u8 command, void* sectorBuffer, s32 executionMode);
void cdrom_verify_disc(u_char intr, u_char* result);
void cdrom_handle_sync_error(void);
void cdrom_set_audio_volume(u_char volume, int stereoChannel);
s32 cdrom_decompress_data(u8** srcStart, u8** dstStart, u8* srcEnd, u8* dstEnd);
void func_80014434(void);
s32* cdrom_handle_stream_data(s32 bytes_transferred, u32 bytes_remaining);
void cdrom_decompress_buffer(u8* srcStart, u8* dstStart);
void cdrom_clear_data_ready(s8* dataReady);
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
 * @brief Cold-start initialization of the CD-ROM subsystem.
 *
 * Performs complete hardware and software initialization of the PlayStation's
 * CD-ROM drive. Must be called before any other CD operations.
 *
 * @details
 * Spin-waits on CdInit() until the hardware is ready, then:
 * 1. Saves and clears previous sync/ready callbacks
 * 2. Resets all CdSystem state (flags, counters, queue indices, command state)
 * 3. Zeros all 16 command queue entries, defaulting buffers to scratchpad RAM
 * 4. Sets CD mode to CdlModeSpeed | CdlModeSize1 (double speed + 2340-byte sectors)
 * 5. Polls CdlNop to read current drive status
 * 6. If shell is open, blocks until disc becomes ready
 * 7. Applies mode via CdlSetmode and records VSync timestamp
 *
 * @warning Blocks until the CD hardware is initialized. If the disc tray is open,
 *          blocks until a disc is inserted. Should only be called once at startup.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/DBYkw
 */
void cdrom_init()
{
    int queueEndMarker;
    int queueCount;
    volatile CdCommandQueueItem* queueItem;
    CdResourceEntry* scratchpadAddr;
    CdStatusFlags* statusFlagsPtr;
    int cdResult;

    // Wait for CD-ROM system to initialize
    while (CdInit() == 0);

    CdSetDebug(0);

    // Save previous callbacks, then clear them
    g_cdSyncCallbackResult = CdSyncCallback(NULL);
    g_cdReadyCallbackResult = CdReadyCallback(NULL);

    statusFlagsPtr = &CD_SYSTEM.statusFlags;

    queueCount = CD_COMMAND_QUEUE_SIZE - 1;
    scratchpadAddr = (CdResourceEntry*)SCRATCHPAD;

    queueEndMarker = -1;

    // g_commandQueueOffset is commandQueue.items[11]; the loop uses queueItem[4]
    // to walk items[15] down to items[0] (all 16 entries).
    queueItem = &g_commandQueueOffset;

    // 0xFFFE = invalid/no resource loaded
    CD_SYSTEM.resourceIndex = CD_RESOURCE_INDEX_INVALID;

    // Reset all runtime state to zero
    CD_SYSTEM.audioEnabled = 0;
    CD_SYSTEM.playbackState = 0;
    CD_SYSTEM.transferCallback = NULL;
    CD_SYSTEM.pendingQueueCount = 0;
    CD_SYSTEM.currentResourceIndex = 0;
    CD_SYSTEM.currentDataSize = 0;
    CD_SYSTEM.targetDataSize = 0;
    CD_SYSTEM.syncComplete = 0;
    CD_SYSTEM.initState = 0;
    CD_SYSTEM.currentCommand = 0;
    CD_SYSTEM.initCommand = 0;
    CD_SYSTEM.retryCount = 0;
    CD_SYSTEM.retryCounter = 0;
    CD_SYSTEM.lastCommand = 0;
    CD_SYSTEM.dstBuffer = 0;
    CD_SYSTEM.callback = 0;
    CD_SYSTEM.queueReadIndex = 0;
    CD_SYSTEM.queueWriteIndex = 0;

    // Clear statusFlags bits 0-6, preserving only bit 7 (0x80).
    // Each bit is cleared individually to match the original assembly output.
    statusFlagsPtr->word &= ~0x01;
    statusFlagsPtr->word &= ~0x02;
    statusFlagsPtr->word &= ~0x04;
    statusFlagsPtr->word &= ~0x08;
    statusFlagsPtr->word &= ~0x10;
    statusFlagsPtr->word &= ~0x40;
    statusFlagsPtr->word &= ~0x20;

    // Clear upper 3 bytes of status flags
    statusFlagsPtr->bytes.b1 = 0;
    statusFlagsPtr->bytes.b2 = 0;
    statusFlagsPtr->bytes.retryExhausted = 0;

    // Zero all 16 command queue entries, setting default buffer to scratchpad
    while (queueCount != queueEndMarker)
    {
        queueItem[4].command = 0;
        queueItem[4].resourceIndex = 0;
        queueItem[4].dstBuffer = scratchpadAddr;
        queueItem[4].entry = scratchpadAddr;
        queueItem[4].callback = 0;
        queueItem--;
        queueCount--;
    }

    CD_SYSTEM.setModeParamBlocking[0] = (CdlModeSpeed | CdlModeSize1);
    CD_SYSTEM.setModeParamBlocking[1] = 0;
    CD_SYSTEM.setModeParamBlocking[2] = 0;
    CD_SYSTEM.setModeParamBlocking[3] = 0;

    // CdlNop (1) — read current drive status into statusByte
    while (CdControlB(CdlNop, NULL, &CD_SYSTEM.statusByte) == 0);

    // If shell-open flag (0x10) is set, block until disc becomes ready
    if ((g_cdStatusByte & CdlStatShellOpen) != 0)
    {
        cdResult = CdDiskReady(1);

        while (cdResult != CdlComplete)
        {
            cdResult = CdDiskReady(0);
        }
    }

    // CdlSetmode (14) — apply mode byte (0xA0) to the drive
    while (CdControlB(CdlSetmode, CD_SYSTEM.setModeParamBlocking, NULL) == 0);

    // Record current frame counter for timeout tracking
    g_cdVSyncTimestamp = VSync(-1);
}

/**
 * @brief Stops all CD-ROM operations and resets the subsystem state.
 *
 * Gracefully halts CD-ROM playback and clears all internal state.
 *
 * @details
 * 1. If audio is enabled, performs a full system reset
 * 2. Clears the playing status flag (bit 6)
 * 3. Removes all sync and ready callbacks
 * 4. Sends pause commands until the drive acknowledges
 * 5. Resets all CdSystem state variables
 * 6. Flushes the CD command queue
 *
 * @warning Blocks until the drive acknowledges the pause. Pending operations will
 *          be aborted. Do not call from within a CD callback.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/M39vT
 */
void cdrom_stop(void)
{
    int cdResult;
    CdSystem* cdSystem;

    cdSystem = &CD_SYSTEM;

    if (g_cdAudioEnabled != 0)
    {
        cdrom_reset();
    }

    cdSystem->statusFlags.word &= 0xFFFFFFBF;

    CdSyncCallback(NULL);
    CdReadyCallback(NULL);

    while (CdControlB(CdlPause, NULL, NULL) == 0);

    CD_SYSTEM.resourceIndex = CD_RESOURCE_INDEX_INVALID;
    CD_SYSTEM.pendingQueueCount = 0;
    CD_SYSTEM.currentResourceIndex = 0;
    CD_SYSTEM.currentDataSize = 0;
    CD_SYSTEM.targetDataSize = 0;
    CD_SYSTEM.playbackState = 0;
    CD_SYSTEM.transferCallback = NULL;
    CD_SYSTEM.currentCommand = 0;
    CD_SYSTEM.initCommand = 0;
    CD_SYSTEM.retryCount = 0;
    CD_SYSTEM.retryCounter = 0;
    CD_SYSTEM.lastCommand = 0;
    CD_SYSTEM.dstBuffer = 0;
    CD_SYSTEM.callback = 0;
    CD_SYSTEM.statusFlags.word &= 0xFFFFFFEF;
    CD_SYSTEM.vsyncTimestamp = VSync(-1);
    CD_SYSTEM.statusFlags.bytes.b1 = 0;
    CD_SYSTEM.statusFlags.bytes.b2 = 0;
    CD_SYSTEM.queueReadIndex = 0;
    CD_SYSTEM.queueWriteIndex = 0;

    CdFlush();
}

/**
 * @brief Streams and decompresses CD-ROM sector data into a destination buffer.
 *
 * Reads sectors from disc via DMA into a ring buffer in scratchpad RAM, then
 * incrementally decompresses the buffered data into the caller's destination.
 *
 * @details
 * Scratchpad RAM (0x1F800000) is used as a shared CdStreamState struct
 * between this function and cdrom_handle_stream_data:
 *
 *   Offset  Type  Description
 *   ------  ----  -----------
 *   +0x00   u8    dataReady      — Set to 1 by callback when new sectors arrive
 *   +0x01   u8    bufferWrapped  — Set to 1 when the ring buffer has wrapped
 *   +0x04   s32   readPtr        — Current read position in ring buffer
 *   +0x08   s32   writePtr       — Current write position in ring buffer
 *   +0x0C   s32   bytesBuffered  — Number of valid bytes available to read
 *   +0x10   s32   wrapOverflow   — Bytes that overflowed past ring buffer end
 *   +0x14   s32   bytesConsumed  — Bytes consumed by decompressor this pass
 *   +0x18   s32   (reserved)     — Initialized to 0
 *
 * The ring buffer ends at 0x801DC118. When it wraps, leftover unprocessed bytes
 * are relocated just before that address (word-aligned) and wrapOverflow is
 * merged into bytesBuffered.
 *
 * @param resourceIndex Resource index (lower 16 bits) identifying the disc data to read
 * @param destination   RAM address where decompressed output is written
 *
 * @return Total number of decompressed bytes written to destination.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/SvWOg
 */
s32 cdrom_stream(s32 resourceIndex, u32 destination)
{
    s32 unprocessedBytes;
    s32 relocDstAddr;
    s32 wrapAmount;
    s32 bytesBuffered;
    s32 bytesConsumed;
    s32 prevReadPtr;
    s32 copySize;
    s32 timestamp;
    s32 remainingDataSize;
    s32* wrapDestinationPtr;
    s32* relocSrcPtr;
    u32 decompressEnd;
    CdStreamState* streamState;
    CdStreamState* streamState2;
    u32 destStart;
    s32 alignRemainder;
    s32 sentinel;

    /* Block until any in-progress CD commands finish */
    while (cdrom_process_state() != 0)
    {
        VSync(0);
    }

    destStart = destination;
    streamState = &CD_STREAM_STATE;

    /* Zero out the streaming state */
    streamState->dropped_sectors = 0;
    streamState->dataReady = 0U;
    streamState->bufferWrapped = 0U;
    streamState->bytesConsumed = 0;

    /* Enqueue a CdlReadN command; return value is the resource's total data size.
     * Subtract 1 to get the last valid byte offset for streaming. */
    remainingDataSize = cdrom_queue_command(CdlReadN, resourceIndex, NULL, &cdrom_handle_stream_data) - 1;
    timestamp = VSync(-1);

    streamState2 = &CD_STREAM_STATE;

    /* === Main streaming loop === */
    while (TRUE)
    {
        if (VSync(-1) < (timestamp + 30))
        {
            /* Timeout hasn't elapsed — check if callback signaled new data */
            if (streamState2->dataReady != 1)
            {
                continue;
            }

            /* === Decompression loop: process all available buffered data === */
            do
            {
                bytesBuffered = streamState2->bytesBuffered;

                /* Calculate source-end boundary for decompression.
                 * If we have fewer bytes buffered than total remaining, hold back
                 * 280 bytes as a safety margin to avoid reading incomplete sectors.
                 * Otherwise use the exact remaining size as the boundary. */
                if (bytesBuffered < remainingDataSize)
                {
                    decompressEnd = (streamState2->readPtr + bytesBuffered) - 280;
                }
                else
                {
                    decompressEnd = streamState2->readPtr + remainingDataSize;
                }

                /* Decompress a chunk; returns 0 when all output is complete */
                if (cdrom_decompress_data(&CD_STREAM_STATE.writePtr, &destination, decompressEnd, -4U) == 0)
                {
                    return destination - destStart;
                }
            } while (bytesBuffered != CD_STREAM_STATE.bytesBuffered);

            /* All currently buffered data has been fed to the decompressor */
            bytesConsumed = streamState2->writePtr - streamState2->readPtr;
            streamState2->bytesConsumed = bytesConsumed;
            cdrom_clear_data_ready(&CD_STREAM_STATE.dataReady);
            remainingDataSize -= bytesConsumed;

            /* If the ring buffer hasn't wrapped, yield to let more data arrive */
            if (streamState2->bufferWrapped != 1)
            {
                timestamp = VSync(-1);
                continue;
            }

            /* --- Handle ring buffer wrap-around --- */

            if (streamState2->wrapOverflow != 0)
            {

                decompressEnd = streamState2->wrapOverflow; /* wrapOverflow amount */

                /* Relocate unprocessed tail bytes to just before the ring buffer
                 * end (0x801DC118), making the data contiguous again. */
                unprocessedBytes = streamState2->bytesBuffered - bytesConsumed;
                alignRemainder = (unprocessedBytes & 3);
                relocDstAddr = 0x801DC118 - unprocessedBytes;
                prevReadPtr = streamState2->readPtr;

                /* Word-align the relocation destination downward */
                copySize = 4 - alignRemainder;
                streamState2->writePtr = relocDstAddr;
                streamState2->readPtr = relocDstAddr;
                copySize = copySize & 3;
                alignRemainder = unprocessedBytes + 3;

                /* Adjust pointers to include alignment padding bytes */
                relocDstAddr = (s32)(relocDstAddr - copySize);
                relocSrcPtr = (s32*)((prevReadPtr + bytesConsumed) - copySize);

                /* Merge overflow bytes into the new contiguous buffer region */
                streamState2->bytesBuffered = decompressEnd + unprocessedBytes;
                copySize = alignRemainder;

                if (copySize < 0)
                {
                    copySize = unprocessedBytes + 6;
                }

                /* Copy unprocessed bytes word-by-word to the relocated position */

                unprocessedBytes = (copySize >> 2);
                unprocessedBytes--;

                if (unprocessedBytes != -1)
                {
                    sentinel = -1;
                    while (unprocessedBytes != sentinel)
                    {
                        *(s32*)relocDstAddr = *relocSrcPtr++;
                        relocDstAddr += 4;
                        unprocessedBytes--;
                    }
                }
            }
            else
            {
                /* No wrap — simply advance the read pointer past consumed data */
                streamState2->readPtr += bytesConsumed;
                streamState2->bytesBuffered -= bytesConsumed;
            }

            /* Memory barrier: prevent compiler from reordering the ready flag write */
            *(volatile u8*)streamState2 = 1;

            timestamp = VSync(-1);
            continue;
        }

        /* Timeout elapsed without data — pump the CD command queue */
        cdrom_process_state();
        timestamp = VSync(-1);
    }
}

/**
 * @brief Streams and decompresses CD-ROM data into caller-supplied chunks via callbacks.
 *
 * A variant of cdrom_stream that delivers decompressed output through a
 * callback-based chunked buffer interface instead of a single fixed destination.
 * Supports two output modes:
 *
 *   DIRECT MODE  — When pfnGetBuffer sets *outChunkSize = -1, the decompressor
 *                  writes straight into the caller's buffer (same as cdrom_stream).
 *
 *   CHUNKED MODE — When pfnGetBuffer sets *outChunkSize to a positive value,
 *                  data is first decompressed into a staging buffer at 0x801DA000,
 *                  then copied out into caller-supplied chunks. Each time a chunk
 *                  is fully filled, pfnChunkDone is called and pfnGetBuffer is
 *                  called again for the next chunk.
 *
 * STAGING BUFFER (chunked mode only):
 *   0x801DA000 — stagingWritePtr starts here
 *   0x801DBBE8 — stagingEnd; decompressor stops here
 *
 *   When the staging buffer fills before the stream ends, the last 4096 bytes
 *   (the LZ sliding-window dictionary) are copied back to 0x801DA000 and
 *   decompression resumes at 0x801DB000, preserving back-reference validity.
 *
 * @param resourceIndex   CD resource index (lower 16 bits) passed to cdrom_queue_command.
 * @param pfnGetBuffer    Callback: u8* fn(int totalBytesDelivered, int* outChunkSize)
 *                          Returns the next destination buffer and sets its capacity,
 *                          or -1 for unlimited. Called at startup and after each chunk.
 * @param pfnChunkDone    Callback: void fn(int chunkIndex)
 *                          Called when each chunk is filled and once at end-of-stream.
 *
 * @see decomp.me (93.03% scratch) https://decomp.me/scratch/4WZBs
 * @note Local best 99.78% (objdiff, 2026-08-04): 290/299 rows exact, no
 *       structural or instruction-count difference. The 9 remaining rows are
 *       all register names in the ring-buffer wrap block: the 0x801DC118
 *       constant carrier wants a0 (we get a1), the `4 - alignRemainder` temp
 *       wants v0 (we get a0), copySize wants a1 (v1), and prevReadPtr wants
 *       a2 (a1). Statement order already matches exactly, so this is pure
 *       register allocation. See working/cdrom_stream_chunked/status.md.
 */
void cdrom_stream_chunked(undefined2 resourceIndex, codeA pfnGetBuffer, codeB pfnChunkDone)
{
    int timestamp;
    u8 srcByte;
    int decompressResult; // Return from cdrom_decompress_data: 0 = end-of-stream, 1 = output full
    u32 srcWord;
    int loopCount;
    int decompressEnd;          // Compressed-source end guard passed to cdrom_decompress_data // a2
    u32 alignCheck;
    u8* srcPtr;               // Read cursor into the staging buffer during the copy-out phase // s1
    int totalBytesDelivered;  // Total decompressed bytes given to caller so far // s5
    int chunkIndex;           // How many chunks delivered so far // s7
    int chunkBytesRemaining;  // Capacity left in the current destination chunk (or -1 = unlimited)
    u8* destination;          // Write cursor into the current caller-supplied destination chunk
    u8* stagingWritePtr;      // Write cursor into the intermediate staging buffer
    u8* stagingEnd;           // End of the staging buffer (0x801DBBE8)
    u8* dstEnd;               // End of the current caller chunk (or 0xFFFFFFFC in direct mode)
    s32 remainingDataSize;    // Bytes of compressed input still to consume (counts down to 0)
    int isDirectMode;         // Non-zero (0x1000) = direct mode; 0 = chunked/staging mode
    s32 bytesBuffered;        // Also reused for bytesConsumed in the tail // s3
    s32 stagingBytesProduced; // Bytes written into staging by decompress; reused in the wrap tail // s0
    s32 alignRemainder;
    s32 copySize;
    s32 negOne; // Per-loop -1 terminator for the copy-out do-while loops // a0
    s32 relocDstAddr;
    s32 prevReadPtr; // Also reused as the relocation source cursor // a2
    u32 wrapOverflow;
    volatile CdStreamState* scratchpad;
    CdStreamState* streamState;
    s32 sentinel;          // Always -1; compared against only in loop GUARDS // s8
    u8** pDestination;     // Pointer-to-pointer to `destination`; kept in register for copy loops // s2
    u8** pStagingWritePtr; // Same pattern, used during LZ window copy

    // --- Initialise streaming state in scratchpad RAM ---
    scratchpad = &CD_STREAM_STATE;
    scratchpad->dropped_sectors = 0;
    scratchpad->dataReady = 0;
    scratchpad->bufferWrapped = 0;

    // Enqueue CdlReadN for this resource; subtract 1 to get the last valid compressed-byte offset.
    remainingDataSize = cdrom_queue_command(CdlReadN, resourceIndex, NULL, cdrom_handle_stream_data) - 1;

    totalBytesDelivered = 0;
    chunkIndex = 0;

    // Ask the caller for the first destination chunk.
    destination = pfnGetBuffer(0, &chunkBytesRemaining);

    if (chunkBytesRemaining == -1)
    {
        // --- Direct mode: caller provided an "unlimited" buffer ---
        dstEnd = (u8*)0xfffffffc;
        isDirectMode = 0x1000;
    }
    else
    {
        // --- Chunked mode: caller provided a fixed-size chunk ---
        // Reserve 0x418 (1048) bytes at the end as a guard region.
        dstEnd = destination + chunkBytesRemaining - 0x418;
        isDirectMode = 0;
    }

    // Staging buffer lives in main RAM. Decompressor fills this; we copy out to caller chunks.
    srcPtr = (u8*)0x801da000;
    stagingWritePtr = srcPtr;     // Start of staging buffer
    stagingEnd = (u8*)0x801dbbe8; // End of staging buffer

    timestamp = VSync(-1);
    streamState = &CD_STREAM_STATE;
    sentinel = -1;
    pDestination = &destination; // Kept in a register so the copy loops can update `destination` indirectly

    while (TRUE)
    {

        if (VSync(-1) < timestamp + 30)
        {

            if (((volatile CdStreamState*)streamState)->dataReady != 1)
            {
                continue;
            }

            // === Decompression loop: process all currently buffered sector data ===
            do
            {
                bytesBuffered = streamState->bytesBuffered;

                // Calculate the compressed-source end-guard for cdrom_decompress_data.
                // Hold back 280 bytes when the full stream hasn't arrived yet,
                // to avoid consuming an incomplete sector boundary.
                if (bytesBuffered < remainingDataSize)
                {
                    decompressEnd = (streamState->readPtr + bytesBuffered) - 280;
                }
                else
                {
                    decompressEnd = streamState->readPtr + remainingDataSize;
                }

                // --- Direct mode: decompress straight into the caller's destination ---
                if (isDirectMode != 0 && destination < dstEnd)
                {
                    cdrom_decompress_data(&CD_STREAM_STATE.writePtr, (u32*)&destination, decompressEnd, (u32)dstEnd);
                    continue; // Loop back; decompressResult check happens below on exit
                }

                // --- Chunked mode: decompress into staging buffer, then copy to caller ---
                srcPtr = stagingWritePtr; // Remember staging write position before this call
                decompressResult = cdrom_decompress_data(&CD_STREAM_STATE.writePtr, (u32*)&stagingWritePtr, decompressEnd, (u32)stagingEnd);

                // How many bytes did the decompressor write to the staging buffer this pass?
                stagingBytesProduced = (int)stagingWritePtr - (int)srcPtr;

                // === Copy staging bytes into the caller's chunk(s) ===
                while (stagingBytesProduced != 0)
                {

                    if ((stagingBytesProduced < chunkBytesRemaining) || (chunkBytesRemaining == sentinel))
                    {
                        // All remaining staging bytes fit within the current chunk (or chunk is unlimited).
                        // Copy everything and break out.
                        totalBytesDelivered += stagingBytesProduced;
                        chunkBytesRemaining -= stagingBytesProduced;

                        // --- Destination alignment preamble ---
                        // Byte-copy until `destination` is word-aligned.
                        loopCount = (u32)destination & 3;
                        if ((loopCount != 0) && (loopCount < stagingBytesProduced))
                        {
                            stagingBytesProduced -= loopCount;
                            loopCount--;
                            if (loopCount != sentinel)
                            {
                                negOne = -1;
                                do
                                {
                                    u8* dest;
                                    srcByte = *srcPtr++;
                                    dest = *pDestination;
                                    *dest = srcByte;
                                    *pDestination = dest + 1;
                                    loopCount--;
                                } while (loopCount != negOne);
                            }
                        }

                        // --- Fast word-copy (only if source is also word-aligned) ---
                        alignCheck = (u32)srcPtr & 3;
                        if (alignCheck == 0)
                        {
                            loopCount = stagingBytesProduced >> 2;
                            stagingBytesProduced -= loopCount * 4;
                            loopCount--;
                            if (loopCount != sentinel)
                            {
                                negOne = -1;
                                do
                                {
                                    u32* dest;
                                    srcWord = *(u32*)srcPtr;
                                    srcPtr += 4;
                                    dest = (u32*)*pDestination;
                                    *dest = srcWord;
                                    *pDestination = (u8*)(dest + 1);
                                    loopCount--;
                                } while (loopCount != negOne);
                            }
                        }

                        // --- Byte-copy for any remaining tail bytes ---
                        stagingBytesProduced--;
                        if (stagingBytesProduced != sentinel)
                        {
                            negOne = -1;
                            do
                            {
                                u8* dest;
                                srcByte = *srcPtr++;
                                dest = *pDestination;
                                *dest = srcByte;
                                *pDestination = dest + 1;
                                stagingBytesProduced--;
                            } while (stagingBytesProduced != negOne);
                        }

                        break; // Done with this batch of staging bytes
                    }

                    // --- Staging bytes span a chunk boundary: fill the current chunk ---
                    // Copy exactly chunkBytesRemaining bytes, fire pfnChunkDone,
                    // then get the next chunk from the caller and keep copying.
                    stagingBytesProduced -= chunkBytesRemaining; // Bytes that will spill into next chunk
                    totalBytesDelivered += chunkBytesRemaining;
                    chunkBytesRemaining--;

                    if (chunkBytesRemaining != sentinel)
                    {
                        negOne = -1;
                        do
                        {
                            u8* dest = *pDestination;
                            *dest = *srcPtr;
                            *pDestination = dest + 1;
                            srcPtr++;
                            chunkBytesRemaining--;
                        } while (chunkBytesRemaining != negOne);
                    }

                    // Notify caller chunk is complete and request the next one --
                    // but only if there's still data to copy or the decompressor has more pending.
                    if (stagingBytesProduced > 0 || decompressResult != 0)
                    {
                        negOne = chunkIndex++;
                        pfnChunkDone(negOne); // Chunk complete
                        destination = pfnGetBuffer(totalBytesDelivered, &chunkBytesRemaining); // Next chunk
                    }
                }

                if (decompressResult != 0)
                {
                    // Staging buffer was exhausted before the stream ended
                    // (decompressResult == 1 means "output buffer full, more input remains").
                    //
                    // Preserve the LZ back-reference sliding window (last 4096 bytes of staging
                    // output) so that back-references remain valid on the next decompress pass.
                    // Copy those 4096 bytes back to 0x801DA000; the decompressor will then
                    // resume writing fresh output starting at 0x801DB000.
                    s32 loopSentinel;
                    stagingWritePtr = (u8*)0x801da000; // Reset staging write cursor to start
                    srcPtr = srcPtr - 0x1000;          // Step back 4096 bytes to start of LZ window
                    stagingBytesProduced = 0xfff;      // Loop count: copy 4096 bytes (terminates at -1)
                    pStagingWritePtr = &stagingWritePtr;
                    loopSentinel = -1;

                    // Copy the 4096-byte LZ window to 0x801DA000.
                    // After this, stagingWritePtr == 0x801DB000; fresh decompressor output follows there.
                    do
                    {
                        u8* dest;
                        srcByte = *srcPtr++;
                        dest = *pStagingWritePtr;
                        stagingBytesProduced--;
                        *dest = srcByte;
                        *pStagingWritePtr = dest + 1;
                    } while (stagingBytesProduced != loopSentinel);

                    continue; // Go back and decompress another pass into the (now reset) staging buffer
                }

                // --- Decompression complete (end-of-stream opcode 0xFF was reached) ---
                // Fire pfnChunkDone for the final (possibly partial) chunk and return.
                pfnChunkDone(chunkIndex);
                return;

            } while (bytesBuffered != CD_STREAM_STATE.bytesBuffered);
            // Re-check: if bytesBuffered changed while we were running (new sectors arrived from
            // the CD callback), loop again immediately to consume the fresh data.

            // --- Update streaming state after processing all buffered data ---
            // bytesBuffered is reused here to hold the consumed-byte count.
            bytesBuffered = streamState->writePtr - streamState->readPtr;
            prevReadPtr = streamState->readPtr;

            streamState->dataReady = 0; // Clear the "new data ready" flag
            streamState->bytesConsumed = bytesBuffered;
            remainingDataSize -= bytesBuffered;

            if (streamState->bufferWrapped != 1)
            {
                // Ring buffer has not wrapped; just yield and wait for more sectors.
                timestamp = VSync(-1);
                continue;
            }

            // --- Handle ring buffer wrap-around ---
            // (Identical logic to cdrom_stream; see that function for detailed comments)
            wrapOverflow = streamState->wrapOverflow;

            if (wrapOverflow != 0)
            {
                stagingBytesProduced = streamState->bytesBuffered - bytesBuffered;
                alignRemainder = (stagingBytesProduced & 3);
                relocDstAddr = 4 - alignRemainder;
                copySize = relocDstAddr & 3;
                loopCount = 0x801dc118;
                relocDstAddr = loopCount - stagingBytesProduced;
                prevReadPtr = (prevReadPtr + bytesBuffered) - copySize;

                streamState->writePtr = relocDstAddr;
                streamState->readPtr = relocDstAddr;
                relocDstAddr = relocDstAddr - copySize;

                alignRemainder = stagingBytesProduced + 3;
                streamState->bytesBuffered = wrapOverflow + stagingBytesProduced;

                if (alignRemainder < 0)
                {
                    alignRemainder = stagingBytesProduced + 6;
                }

                stagingBytesProduced = (alignRemainder >> 2);
                stagingBytesProduced--;

                if (stagingBytesProduced != sentinel)
                {
                    s32 wrapNegOne = -1;
                    do
                    {
                        *(s32*)relocDstAddr = *(s32*)prevReadPtr;
                        prevReadPtr += 4;
                        relocDstAddr += 4;
                        stagingBytesProduced--;
                    } while (stagingBytesProduced != wrapNegOne);
                }
            }
            else
            {
                streamState->readPtr = prevReadPtr + bytesBuffered;
                streamState->bytesBuffered -= bytesBuffered;
            }

            *(volatile u8*)streamState = 1; // Memory barrier: signal callback that buffer is ready again
            timestamp = VSync(-1);
            continue;
        }

        // Timeout -- pump the CD command queue and reset the timer.
        cdrom_process_state();
        timestamp = VSync(-1);
    }
}

/**
 * @brief Enqueues a CD-ROM command into the circular command queue.
 *
 * Validates and inserts a command into the 16-entry circular command queue.
 * If the system is idle and no error/init flags are active, immediately
 * starts command execution by issuing CdlNop to kick off the state machine.
 *
 * @details
 * Validation steps before enqueueing:
 * 1. Rejects if the playing status flag (bit 6) is set
 * 2. Resolves the resource index: 0xFFFF maps to g_defaultCdResource,
 *    all other values index into CD_RESOURCE_ENTRIES
 * 3. Deduplicates: if the system is busy and the new command matches the
 *    last-enqueued (command, resourceIndex, dstBuffer, callback), skips
 *    the enqueue and returns the existing dataSize
 * 4. Validates the resource has a non-zero disc location and data size
 * 5. Checks the queue is not full ((writeIndex + 1) & 0xF != readIndex)
 *
 * @param command        CD-ROM command byte (e.g., CdlReadN, CdlSeekL)
 * @param resourceIndex  Index into CD_RESOURCE_ENTRIES, or 0xFFFF for the default resource
 * @param dstBuffer      Destination buffer for read data (NULL for non-read commands)
 * @param callback       Invoked on command completion
 *
 * @return The resource's dataSize on success, or:
 *         -3 if the system is in playing state (bit 6 set)
 *         -2 if the resource has no valid location or zero data size
 *         -1 if the command queue is full
 *
 * @warning Not interrupt-safe; must not be called from within a CD callback.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/izXP3
 */
s32 cdrom_queue_command(u8 command, u16 resourceIndex, void* dstBuffer, CdCommandCallback callback)
{
    s32 timestamp;
    s32 writeIndex;
    s32 writeIndex2;
    s32 statusFlags;
    s32 dataSize;
    u8 activeCommand;
    CdResourceEntry* resourceEntry;
    volatile CdSystem* cdSystem;

    // Reject immediately if the "playing" flag (bit 6) is set
    if (g_cdSystem.statusFlags.word & 0x40)
    {
        return -3;
    }

    // Resolve resource index to entry pointer
    if (resourceIndex == CD_RESOURCE_INDEX_DEFAULT)
    {
        resourceEntry = &g_defaultCdResource;
    }
    else
    {
        resourceEntry = &CD_RESOURCE_ENTRIES[resourceIndex];
    }

    cdSystem = &CD_SYSTEM;

    // Deduplicate: skip enqueue if system is busy AND the command matches
    // the previously enqueued one exactly (same command, resource, buffer, callback)
    if ((cdSystem->currentCommand == 0 && cdSystem->initCommand == 0) || ((CD_SYSTEM.lastCommand != command) || (CD_SYSTEM.resourceIndex != resourceIndex) ||
                                                                          (CD_SYSTEM.dstBuffer != dstBuffer) || (CD_SYSTEM.callback != callback)))
    {

        // Validate resource entry has a valid disc location and non-zero size
        if ((*(u32*)&resourceEntry->location == 0) || (resourceEntry->dataSize == 0))
        {
            return -2;
        }

        // Check circular queue is not full
        writeIndex = CD_SYSTEM.queueWriteIndex;

        if (CD_SYSTEM.queueReadIndex == ((writeIndex + 1) & 0xF))
        {
            return -1;
        }

        // Write command fields into the queue entry at the current write index.
        // Each field re-reads queueWriteIndex to match the original volatile access pattern.
        writeIndex = CD_SYSTEM.queueWriteIndex;
        CD_SYSTEM.commandQueue.items[writeIndex].command = command;
        CD_SYSTEM.lastCommand = command;

        writeIndex2 = CD_SYSTEM.queueWriteIndex;
        CD_SYSTEM.commandQueue.items[writeIndex2].resourceIndex = resourceIndex;
        CD_SYSTEM.resourceIndex = resourceIndex;

        writeIndex2 = CD_SYSTEM.queueWriteIndex;
        CD_SYSTEM.commandQueue.items[writeIndex2].entry = resourceEntry;

        writeIndex2 = CD_SYSTEM.queueWriteIndex;
        CD_SYSTEM.commandQueue.items[writeIndex2].dstBuffer = dstBuffer;

        CD_SYSTEM.dstBuffer = dstBuffer;

        CD_SYSTEM.commandQueue.items[CD_SYSTEM.queueWriteIndex].callback = callback;
        CD_SYSTEM.callback = callback;

        // Advance write index with circular wrap (mod 16)
        CD_SYSTEM.queueWriteIndex = ((CD_SYSTEM.queueWriteIndex + 1) & 0xF);

        timestamp = VSync(-1);

        // If a command is already in progress, just queue and return
        activeCommand = CD_SYSTEM.currentCommand;

        if ((activeCommand != 0) || (CD_SYSTEM.initCommand != 0))
        {
            return resourceEntry->dataSize;
        }

        statusFlags = CD_SYSTEM.statusFlags.word;

        // If no error/init flags (bits 0-3) are active, bootstrap execution
        if (!(statusFlags & 0xF))
        {

            CD_SYSTEM.vsyncTimestamp = timestamp;
            CD_SYSTEM.pendingQueueCount = 1;
            CD_SYSTEM.currentResourceIndex = resourceIndex;
            dataSize = resourceEntry->dataSize;
            CD_SYSTEM.currentCommand = 1;
            CD_SYSTEM.statusFlags.word = (statusFlags | 0x10);
            CD_SYSTEM.playbackState = 0;
            CD_SYSTEM.transferCallback = NULL;
            CD_SYSTEM.targetDataSize = dataSize;
            CD_SYSTEM.currentDataSize = dataSize;

            // Install sync callback and send CdlNop to kick off the state machine
            CdSyncCallback(&cdrom_complete_command);
            CdSync(0, NULL);
            CdControlF(CdlNop, NULL);
        }
    }

    return resourceEntry->dataSize;
}

/**
 * @brief Drains the CD command queue and drives the disc-recovery state machine.
 *
 * Called once per frame to advance all pending CD-ROM operations. Handles
 * three mutually exclusive execution paths depending on current subsystem state,
 * and updates the audio system when enabled.
 *
 * @details
 * Inspects statusFlags to choose one of three branches:
 *
 * **Branch 1 -- Error/init recovery (statusFlags bits 0-2 set):**
 * Runs a multi-state recovery state machine (states 1-8, 32):
 *   1. Polls drive status via CdlNop every 30 VSync frames
 *   2. Progresses through GetStat, DiskReady, DiskType detection
 *   3. Re-applies CdlSetmode (0xA0) and installs sync/ready callbacks
 *   4. Issues CdlReadN to resume reading, with 270-frame timeout retries
 *   5. On persistent errors, pauses the drive and resets to state 1
 *
 * **Branch 2 -- Active command (currentCommand or initCommand != 0):**
 *   - Polls syncComplete flag set by the sync callback
 *   - Updates currentResourceIndex and currentDataSize from the queue head
 *   - On 240-frame timeout, re-installs callbacks and retries via CdlNop
 *
 * **Branch 3 -- Idle with queued commands:**
 *   - Sets currentCommand to 1, marks busy flag (bit 4)
 *   - Installs cdrom_complete_command and sends CdlNop to start processing
 *   - If the queue is empty, performs periodic 30-frame status polls via CdlNop
 *
 * @return Number of commands remaining in the queue, or 0 if idle or in recovery
 *         (statusFlags bit 3 set).
 *
 * @warning Must be called every frame. Not interrupt-safe.
 *          The 30/240/270-frame timeouts assume NTSC (60 Hz).
 *
 * @see decomp.me (100%) https://decomp.me/scratch/xxcgW
 */
u32 cdrom_process_state(void)
{
    s32 controlResult;

    s32 syncCompleteFlag;
    s32 indexDiff;

    u8 currentCommand;
    u8 cdCommand;
    u8* cdCommandParams;
    u32 readIndex;
    s32 initCommand;

    u8 initState;
    u32 flagsTmp;
    CdSystem* cdSystem;


    // Fast-path out. This natively generates the `bnez v0, 870; move v0, zero`
    if (CD_SYSTEM.statusFlags.word & 8)
    {
        return 0;
    }

    initState = 1;

    if (CD_SYSTEM.statusFlags.word & 7)
    {

        readIndex = CD_SYSTEM.queueReadIndex;
        indexDiff = (CD_SYSTEM.queueWriteIndex - readIndex) & 0xF;

        CD_SYSTEM.pendingQueueCount = indexDiff;

        if (CD_SYSTEM.initState == 0)
        {

            CD_SYSTEM.initState = initState;

            if (indexDiff != 0)
            {
                u32 readIndex2;
                readIndex2 = (u32)&CD_SYSTEM + (readIndex << 4);
                CD_SYSTEM.currentResourceIndex = *(u16*)(readIndex2 + 0x42);
                CD_SYSTEM.currentDataSize = *(s32*)(*((s32*)(readIndex2 + 0x44)) + 4);
                CD_SYSTEM.targetDataSize = CD_SYSTEM.readRemainingBytes;
            }

            if (CD_SYSTEM.audioEnabled != 0)
            {
                if (g_cdAudioReady != 0)
                {
                    akao_cmd_99_9b_9d_9f(3);
                }
            }

            if (CD_SYSTEM.transferCallback != NULL)
            {
                CD_SYSTEM.playbackState = 1;
            }
            else
            {
                CD_SYSTEM.playbackState = 0;
            }

            g_cdStatusByte3 = 0;
        }

        if (VSync(-1) >= ((s32)CD_SYSTEM.vsyncTimestamp + 30))
        {

            if (CD_SYSTEM.initState != 8)
            {
                CD_SYSTEM.vsyncTimestamp = VSync(-1);
            }

            controlResult = CdControlB(CdlNop, 0, (u8*)0x801ED960);

            if (!(CD_SYSTEM.statusByte & 0x10) && (controlResult != 0))
            {
                switch (CD_SYSTEM.initState)
                {
                case 1:
                    CD_SYSTEM.initState = 2;
                    CD_SYSTEM.statusFlags.word = (CD_SYSTEM.statusFlags.word & ~1) | 6;
                    /* fallthrough */

                case 2:
                    controlResult = CdControlB(0x13, 0, (u8*)0x801ED960);
                    if ((CD_SYSTEM.statusByte & 2) && (controlResult != 0))
                    {
                        CD_SYSTEM.initState = 3;
                        CD_SYSTEM.retryCounter = 0;
                    }
                    break;

                case 3:
                    if (CdDiskReady(1) == 2)
                    {
                        g_initState = 4;
                    }
                    else
                    {
                        u8 counter = CD_SYSTEM.retryCounter + 1;
                        CD_SYSTEM.retryCounter = counter + 1;
                        if (counter >= 13)
                        {
                            CD_SYSTEM.initState = 4;
                        }
                    }
                    break;

                case 4:
                    controlResult = CdDiskReady(0);
                    if (controlResult == 2)
                    {
                        g_initState = 5;
                    }
                    else if (controlResult == 0x10)
                    {
                        g_initState = 1;
                    }
                    else
                    {
                        g_initState = 5;
                    }
                    break;

                case 5:
                    controlResult = CdGetDiskType();
                    switch (controlResult)
                    {
                    case 0:
                        CD_SYSTEM.initState = 0x20;
                        CD_SYSTEM.statusFlags.word &= ~2;
                        break;

                    case 1:
                        CdDiskReady(0);
                        CdGetDiskType();
                        /* fallthrough */

                    case 2:
                        CD_SYSTEM.initState = 6;
                        CD_SYSTEM.vsyncTimestamp -= 30;
                        break;
                    }
                    break;

                case 6:
                    CD_SYSTEM.setModeParamAsync[0] = (CdlModeSpeed | CdlModeSize1);
                    CD_SYSTEM.setModeParamAsync[1] = 0;
                    CD_SYSTEM.setModeParamAsync[2] = 0;
                    CD_SYSTEM.setModeParamAsync[3] = 0;
                    CdSyncCallback(cdrom_handle_recovery_sync);
                    CdReadyCallback(NULL);
                    CD_SYSTEM_V.initCommand = 0x20;
                    CdControlF(CdlSetmode, (u8*)0x801ED954);
                    CD_SYSTEM.vsyncTimestamp -= 26;
                    break;

                case 7:
                    CD_SYSTEM.recoveryReadPosition.raw = (s32)g_cdResource176;
                    CD_SYSTEM.statusFlags.word |= 0x10;
                    CdSyncCallback(cdrom_handle_recovery_sync);
                    CdReadyCallback((void (*)(u8, u8*))cdrom_verify_disc);
                    CD_SYSTEM.initCommand = 0x21;
                    CD_SYSTEM.initState = 8;
                    CdControlF(CdlReadN, (u8*)0x801ED95C);
                    CD_SYSTEM.vsyncTimestamp -= 30;
                    break;

                case 8:
                    if (CD_SYSTEM_V.syncComplete == 1)
                    {
                        CD_SYSTEM.vsyncTimestamp = VSync(-1);
                        CD_SYSTEM_V.syncComplete = 0;
                    }
                    else if (VSync(-1) >= ((s32)CD_SYSTEM.vsyncTimestamp + 270))
                    {
                        initCommand = CD_SYSTEM_V.initCommand & 0xFF;

                        if (initCommand != 0x22)
                        {
                            if (initCommand >= 0x23)
                            {
                                if (initCommand == 0x23)
                                {
                                    goto RetrySetmode;
                                }
                            }

                            CdSyncCallback(cdrom_handle_recovery_sync);
                            CdReadyCallback((void (*)(u8, u8*))cdrom_verify_disc);
                            CD_SYSTEM.initCommand = 0x21;
                            cdCommand = CdlReadN;
                            cdCommandParams = (u8*)0x801ED95C;
                        }
                        else
                        {
                            CdSyncCallback(cdrom_handle_recovery_sync);
                            cdCommand = CdlPause;
                            cdCommandParams = NULL;
                        }
                        goto ExecuteCommand;

                    RetrySetmode:
                        CdSyncCallback(cdrom_handle_recovery_sync);
                        cdCommand = CdlSetmode;
                        cdCommandParams = (u8*)0x801ED950;

                    ExecuteCommand:
                        CdControlF(cdCommand, cdCommandParams);
                        CD_SYSTEM.vsyncTimestamp -= 30;
                    }
                    break;

                case 32:
                    while (CdControlB(8, 0, NULL) == 0);
                    g_initState = 0x21;
                    break;
                }
            }
            else
            {
                cdSystem = &CD_SYSTEM;
                if (g_initState >= 6)
                {
                    cdSystem->statusFlags.word &= ~0x10;
                    CdSyncCallback(NULL);
                    CdReadyCallback(NULL);
                    while (CdControlB(CdlPause, 0, NULL) == 0);
                    CD_SYSTEM_V.initCommand = 0;
                }
                CD_SYSTEM.initState = 1;
                flagsTmp = (CD_SYSTEM.statusFlags.word | 1) & ~2;
                CD_SYSTEM.statusFlags.word = flagsTmp & ~4;
            }
        }
    }
    else
    {
        syncCompleteFlag = 0;
        currentCommand = CD_SYSTEM.currentCommand;

        if ((currentCommand != 0) || (CD_SYSTEM.initCommand != 0))
        {
            while (1)
            {
                if (CD_SYSTEM_V.syncComplete == 1)
                {
                    syncCompleteFlag = 1;
                    CD_SYSTEM.syncComplete = 0;
                }
                readIndex = CD_SYSTEM.queueReadIndex;

                indexDiff = (CD_SYSTEM.queueWriteIndex - readIndex) & 0xF;

                if (indexDiff != 0)
                {
                    CD_SYSTEM.currentResourceIndex = CD_SYSTEM.commandQueue.items[CD_SYSTEM.queueReadIndex].resourceIndex;
                    CD_SYSTEM.currentDataSize = CD_SYSTEM.commandQueue.items[CD_SYSTEM.queueReadIndex].entry->dataSize;
                    CD_SYSTEM.targetDataSize = CD_SYSTEM.readRemainingBytes;
                }

                if (CD_SYSTEM.syncComplete == 0)
                {
                    break;
                }
            }

            if (syncCompleteFlag == 0)
            {
                if (VSync(-1) >= (s32)(CD_SYSTEM.vsyncTimestamp + 240))
                {
                    if (CD_SYSTEM.initCommand == 0)
                    {
                        CD_SYSTEM_V.currentCommand = 1;

                        if (CD_SYSTEM.transferCallback != NULL)
                        {
                            CD_SYSTEM.playbackState = 1;
                        }
                        else
                        {
                            CD_SYSTEM.playbackState = 0;
                        }

                        CdSyncCallback((void (*)(u8, u8*))cdrom_complete_command);
                        CdReadyCallback(NULL);
                        while (CdControlB(CdlNop, 0, (u8*)0x801ED960) == 0);
                    }
                    else
                    {
                        CdSyncCallback(cdrom_handle_recovery_sync);
                        CdReadyCallback(NULL);
                        while (CdControlB(CdlNop, 0, (u8*)0x801ED960) == 0);
                    }
                    g_cdVSyncTimestamp = VSync(-1);
                }
            }
            else
            {
                g_cdVSyncTimestamp = VSync(-1);
            }

            g_cdPendingQueueCount = indexDiff;
        }
        else if (CD_SYSTEM.queueReadIndex != CD_SYSTEM.queueWriteIndex)
        {
            CD_SYSTEM.vsyncTimestamp = VSync(-1);
            CD_SYSTEM.currentCommand = 1;
            CD_SYSTEM.statusFlags.word |= 0x10;

            if (CD_SYSTEM.transferCallback != NULL)
            {
                CD_SYSTEM.playbackState = 1;
            }
            else
            {
                CD_SYSTEM.playbackState = 0;
            }

            CdSyncCallback((void (*)(u8, u8*))cdrom_complete_command);
            CdReadyCallback(NULL);
            CdSync(0, 0);
            CdControlF(CdlNop, NULL);
            indexDiff = (CD_SYSTEM.queueWriteIndex - CD_SYSTEM.queueReadIndex) & 0xF;
        }
        else
        {
            CD_SYSTEM.transferCallback = NULL;
            CD_SYSTEM.playbackState = 0;

            if (!(CD_SYSTEM.statusFlags.word & 0x20))
            {
                if (VSync(-1) >= (s32)(CD_SYSTEM.vsyncTimestamp + 30))
                {
                    if (CdControlB(CdlNop, 0, (u8*)0x801ED960) != 0)
                    {
                        if (CD_SYSTEM.statusByte & 0x10)
                        {
                            cdrom_handle_sync_error();
                        }
                        CD_SYSTEM.syncComplete = 0;
                        CD_SYSTEM.retryCounter = 0;
                        CD_SYSTEM.vsyncTimestamp = VSync(-1);
                    }
                    else
                    {
                        if (CD_SYSTEM.retryCounter++ >= 11)
                        {
                            cdrom_handle_sync_error();
                        }
                    }
                }
            }
            indexDiff = 0;
            g_cdPendingQueueCount = 0;
        }
    }

    if (g_cdAudioEnabled != 0)
    {
        FUN_80140d48();
    }

    return indexDiff;
}

/**
 * @brief Drives the CD-ROM recovery/init state machine each VSync frame.
 *
 * Advances a 4-state asynchronous state machine that reconfigures the CD-ROM
 * subsystem after an error or shell-open event. Each call advances at most
 * one state transition.
 *
 * @details
 * State machine (CD_SYSTEM.initState):
 *
 * - **State 0 — Flush:** Calls CdFlush(), advances to state 1 with a 1-frame delay.
 *
 * - **State 1 — Set mode:** Waits for the delay, configures CD mode to 0xA0
 *   (CdlModeSpeed | CdlModeSize1), installs cdrom_handle_recovery_sync,
 *   sends CdlSetmode, and waits 4 frames.
 *
 * - **State 2 — Set filter:** Installs sync callback, sends CdlSetfilter
 *   (file=1, channel=1), sets initCommand to 0x11, advances to state 3.
 *
 * - **State 3 — Dispatch:** Waits for syncComplete or 30-frame timeout, then
 *   dispatches based on initCommand:
 *     - 0x10: Re-sends CdlSetfilter
 *     - 0x11: Sends CdlDemute
 *     - 0x12: Sends CdlPause
 *
 * @return 1 if not in recovery mode (statusFlags bit 3 clear),
 *         0 while the state machine is still processing.
 *
 * @warning Must be called every frame. The 1/4/30-frame delays assume NTSC (60 Hz).
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/IvxZG
 */
s32 cdrom_recover(void)
{
    u_char filterParams[2];
    s32 timestamp;
    u8 initCommandByte;

    // Bail out if bit 3 (recovery mode) is not set
    if (!(CD_SYSTEM.statusFlags.word & 8))
    {
        return 1;
    }

    switch (CD_SYSTEM.initState)
    {
    case 0:
        // State 0: Flush pending CD commands and advance to state 1
        CdFlush();
        CD_SYSTEM.initState = 1U;
        CD_SYSTEM.vsyncTimestamp = (s32)(VSync(-1) + 1);
        break;
    case 1:
        // State 1: Wait for 1-frame delay, then configure CD mode
        timestamp = VSync(-1);
        if (timestamp >= (s32)CD_SYSTEM.vsyncTimestamp)
        {
            // Set mode to 0xA0 (double speed + 2340-byte sectors)
            CD_SYSTEM.setModeParamAsync[0] = (CdlModeSpeed | CdlModeSize1);
            CD_SYSTEM.setModeParamAsync[1] = 0;
            CD_SYSTEM.setModeParamAsync[2] = 0;
            CD_SYSTEM.setModeParamAsync[3] = 0;

            CdSyncCallback(cdrom_handle_recovery_sync);

            CdReadyCallback(NULL);
            // initCommand 0x10 = pending setfilter; must be stored before CdControlF (not in delay slot)
            CD_SYSTEM_V.initCommand = 0x10U;
            CdControlF(CdlSetmode, (u8*)0x801ED954);
            timestamp = VSync(-1);
            CD_SYSTEM.vsyncTimestamp = (s32)(timestamp + 4);
        }
        break;
    case 2:
        // State 2: Send CdlSetfilter with file=1, channel=1, advance to state 3
        CdSyncCallback(cdrom_handle_recovery_sync);
        CD_SYSTEM.initCommand = 0x11;

        filterParams[0] = 1;
        filterParams[1] = 1;
        CdControlF(CdlSetfilter, filterParams);
        CD_SYSTEM.initState = 3U;
        CD_SYSTEM.vsyncTimestamp = VSync(-1);
        break;
    case 3:
        // State 3: Wait for syncComplete or 30-frame timeout, then dispatch
        if (CD_SYSTEM.syncComplete == 1)
        {
            CD_SYSTEM.vsyncTimestamp = VSync(-1);
            CD_SYSTEM_V.syncComplete = 0U;
            break;
        }

        timestamp = VSync(-1);
        if (timestamp < (s32)(CD_SYSTEM.vsyncTimestamp + 30))
        {
            break;
        }

        // Timeout expired — dispatch follow-up command based on initCommand
        CdSyncCallback(cdrom_handle_recovery_sync);

        initCommandByte = CD_SYSTEM.initCommand;

        switch (initCommandByte)
        {
        case 0x0:
        default:
            filterParams[0] = 1;
            filterParams[1] = 1;
            CdControlF(CdlSetfilter, filterParams);
            CD_SYSTEM_V.initCommand = 0x10U;
            break;
        case 0x11:
            CdControlF(CdlDemute, NULL);
            break;
        case 0x12:
            CdControlF(CdlPause, NULL);
            break;
        }

        // Subtract 30 to allow immediate re-entry on the next timeout cycle
        CD_SYSTEM.vsyncTimestamp -= 30;
        break;
    }

    return 0;
}

/**
 * @brief Ready callback that verifies the next sector header during error recovery.
 *
 * Invoked when the CD-ROM drive signals readiness while in a recovery state.
 * Reads the sector header, verifies the disc position matches currentLocation,
 * and either completes the sector read, retries, or falls back to CdlNop
 * after exhausting 16 retries.
 *
 * @details
 * Requires g_cdStatusByte3 == 1 to proceed; clears it to 0 on exit.
 *
 * - **Audio disabled:** Reads the sector header (3 words) and compares the lower
 *   24 bits against currentLocation. On match, calls cdrom_process_sector(1).
 *   On mismatch, increments retryCount (up to 16). After 16 failures, marks
 *   retryExhausted and issues CdlNop.
 *
 * - **Audio enabled:** Skips position check; immediately calls cdrom_process_sector(1).
 *
 * @note Installed as the CdReadyCallback after cdrom_recover() enters a waiting state.
 *
 * @warning Spin-waits on CdGetSector() until the sector header is available.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/iWEyM
 */
void cdrom_verify_recovery(void)
{
    volatile CdSystem* cdSystem;
    volatile CdSystem** new_var;

    cdSystem = &CD_SYSTEM;

    if ((u8)g_cdStatusByte3 != 1)
    {
        return;
    }

    if (cdSystem->audioEnabled != (u8)g_cdStatusByte3)
    {
        while (CdGetSector((void*)0x801ED940, 3) == 0);

        if ((CD_SYSTEM.sectorHeaderBuffer[0] & 0xFFFFFF) == (CD_SYSTEM.currentLocation.raw & 0xFFFFFF))
        {
            cdrom_process_sector(1);
            return;
        }

        if (CD_SYSTEM.retryCount++ <= 16)
        {
            CdControlF(CD_SYSTEM.currentCommand, (u8*)0x801ED958);
        }
        else
        {
            CD_SYSTEM.statusFlags.bytes.retryExhausted = 1;
            CD_SYSTEM.retryCount = 0U;
            if (CD_SYSTEM.transferCallback != NULL)
            {
                CD_SYSTEM.playbackState = 1;
            }
            else
            {
                CD_SYSTEM.playbackState = 0;
            }
            cdSystem = &CD_SYSTEM;
            (*(new_var = &cdSystem))->currentCommand = 1U;
            CdControlF(1U, NULL);
        }
    }
    else
    {
        cdrom_process_sector(1);
    }

    g_cdStatusByte3 = 0;
}

/**
 * @brief Sync callback invoked when a CD-ROM command completes or fails.
 *
 * Installed as the CdSyncCallback during normal command queue processing.
 * Advances the circular queue, dispatches the next command, or cleans up
 * when the queue is drained.
 *
 * @details
 * Sets syncComplete to 1 on entry, then branches based on interrupt status:
 *
 * **Error path:** If currentCommand is CdlNop (1) and result bit 4 is set,
 * calls cdrom_handle_sync_error() and returns.
 *
 * **Incomplete path (status != CdlComplete):**
 * - If currentCommand != 1, resets to CdlNop and retries.
 * - Otherwise re-reads the queue head and executes it.
 *
 * **Complete path (status == CdlComplete):**
 * - CdlPause (21): Resets playback state, advances queue. If empty, clears all
 *   execution state. Otherwise dispatches the next command.
 * - All others: Reads queue head, skipping consecutive CdlNop entries, then
 *   dispatches via cdrom_run_command.
 *
 * Special: command 0x1B (audio start) sets audioEnabled and remaps to CdlSeekL (6).
 *
 * @param intr    CD-ROM interrupt status byte (CdlComplete on success)
 * @param result  Pointer to the CD-ROM result byte array from the hardware
 *
 * @warning Runs in interrupt context; must not call blocking functions.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/BXisc
 */
void cdrom_complete_command(u_char intr, u_char* result)
{
    u8 nextCommand;
    u32 writeIndex;
    u32 readIndex;
    s32 statusFlags;
    volatile CdSystem* cdSystem;
    CdSystem* cd_sys;

    // Signal to main-loop poller that a sync event occurred
    CD_SYSTEM.syncComplete = 1;

    // If we sent CdlNop to probe intr, check for drive errors
    if (CD_SYSTEM.currentCommand == 1)
    {
        if (*result & 0x10)
        {
            cdrom_handle_sync_error();
            return;
        }
    }

    // Command completed: dispatch based on which command just finished.
    // Otherwise drop to the retry/fallthrough handler in the else branch.
    if ((intr & 0xFF) == CdlComplete)
    {
        // The switch reads currentCommand through the volatile alias so the
        // base address is re-materialized independently (matches the original
        // codegen for the jump-table dispatch).
        switch (VCD.currentCommand)
        {
        default:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
            // Read the command at the current queue head
            nextCommand = VCD.commandQueue.items[VCD.queueReadIndex].command;

            // Skip past consecutive CdlNop (1) entries in the queue
            if (nextCommand == 1)
            {
                cd_sys = &CD_SYSTEM;
                writeIndex = cd_sys->queueWriteIndex;
                do
                {
                    readIndex = cd_sys->queueReadIndex;
                    if (readIndex == writeIndex)
                    {
                        // All commands consumed: drop the sync callback and
                        // reset execution state.
                        CdSyncCallback(NULL);
                        cd_sys->playbackState = 0;
                        cd_sys->transferCallback = NULL;
                        cd_sys->currentCommand = 0;
                        cd_sys->retryCounter = 0;
                        cd_sys->statusFlags.word &= ~0x10;
                        cd_sys->vsyncTimestamp = VSync(-1);
                        return;
                    }
                    readIndex = (readIndex + 1) & 0xF;
                    cd_sys->queueReadIndex = readIndex;
                    nextCommand = (cd_sys->commandQueue.items + readIndex)->command;
                } while (nextCommand == 1);
            }
            break;

        case 21:
            // CdlPause completed: reset playback state and advance queue
            cd_sys = &CD_SYSTEM;
            cd_sys->playbackState = 0;
            cd_sys->transferCallback = NULL;
            readIndex = (cd_sys->queueReadIndex + 1) & 0xF;
            cd_sys->queueReadIndex = readIndex;

            // If queue is now empty after pause, clean up and return
            if (readIndex == cd_sys->queueWriteIndex)
            {
                CdSyncCallback(NULL);
                VCD.currentCommand = 0;
                VCD.initCommand = 0;
                cd_sys->retryCounter = 0;
                cd_sys->statusFlags.word &= ~0x10;
                cd_sys->vsyncTimestamp = VSync(-1);
                return;
            }

            // Queue still has entries: dispatch the next one
            nextCommand = cd_sys->commandQueue.items[readIndex].command;
            break;

        case 6:
        case 27:
            // These commands complete without dispatching a follow-up:
            // their jump-table entries point straight at the epilogue.
            return;
        }

        // Special case: command CdlReadS - used for CD-DA/XA streaming.
        // This translation only applies on the completed-command path.
        if (nextCommand == CdlReadS)
        {
            cdSystem = &CD_SYSTEM;
            if (g_cdAudioEnabled == 0)
            {
                CD_SYSTEM.audioEnabled = 1;
            }
            nextCommand = CdlReadN;
        }
    }
    else
    {
        // Command did not complete: if not already probing with CdlNop, retry
        // with CdlNop. Otherwise re-read the queue head and execute it directly
        // (no CdlReadS translation on this path).
        cdSystem = &CD_SYSTEM;
        if (cdSystem->currentCommand != 1)
        {
            CD_SYSTEM.currentCommand = 1;
            CdControlF(1, NULL);
            return;
        }
        nextCommand = CD_SYSTEM.commandQueue.items[CD_SYSTEM.queueReadIndex].command;
    }
    cdrom_run_command(nextCommand, 0, 0);
}

/**
 * @brief Sync callback for CD-ROM init, disc validation, and error recovery.
 *
 * Installed via CdSyncCallback() during startup and error recovery. Drives a
 * state machine (CD_SYSTEM.initState / initCommand) that configures the drive,
 * validates the disc, and transitions to normal processing via cdrom_complete_command.
 *
 * @details
 * Manages four phases:
 * 1. **Initialization** – steps through states 1→6 (status check, set mode,
 *    set filter, read validation sector).
 * 2. **Disc validation** – states 7/8: verifies the disc ID string.
 * 3. **Error recovery** – on timeout or error, retries commands or enters
 *    recovery states 0x20–0x23.
 * 4. **Normal hand-off** – replaces itself with cdrom_complete_command and
 *    dispatches the first queued command.
 *
 * The high bit of initCommand (0x80) is a retry flag: on command failure it is
 * set and CdlNop is re-issued; the next callback sees the pending retry.
 *
 * @param intr    Completion code from the CD-ROM drive; only acts on intr == 2 (CdlComplete).
 * @param result  Pointer to the drive's status byte; bit 4 indicates error/shell-open.
 *
 * @note Runs from the CD-ROM library's interrupt handler.
 *
 * @see decomp.me: (99.71%) https://decomp.me/scratch/0Dz2i
 */
void cdrom_handle_recovery_sync(u_char intr, u_char* result)
{
    s32 temp_v1;
    s32 queue_read;
    s32 queue_write;
    u8 sp10[2];
    u8 temp_a0;
    AudioSystem* audioSystem;
    CdSystem* cdSystem;
    s32* new_var;

    // Volatile (CD_SYSTEM_V) stores of syncComplete/initCommand throughout
    // this function are required to match: they pin the stores in program
    // order and keep the delay-slot filler from moving them into jump slots.
    CD_SYSTEM_V.syncComplete = 1;

    // The (s8) sign test reproduces the original's sll/bltz check of the
    // 0x80 retry flag. cdSystem is assigned in both branches (not hoisted)
    // to match the original register allocation.
    if (((s8)CD_SYSTEM_V.initCommand < 0) && !(CD_SYSTEM.statusFlags.word & 8))
    {
        cdSystem = &CD_SYSTEM;
        if (*result & 0x10)
        {
            cdrom_handle_sync_error();
            return;
        }
    }
    else
    {
        cdSystem = &CD_SYSTEM;
    }
    if (((cdSystem->initCommand & 0x7F) == 0x21) && (cdSystem->statusByte & 1))
    {
        if (cdSystem->filterModeFlags & 0x40)
        {
            CdSyncCallback(NULL);
            CdReadyCallback(NULL);
            cdSystem->initState = 0x20;
            cdSystem->initCommand = 0;
            cdSystem->statusFlags.word &= ~0x10;
            cdSystem->statusFlags.word &= ~4;
        }
    }
    // Reusing queue_write for the intr test extends its live range, which
    // demotes its register-allocation priority below temp_v1 in case 35
    // (required to match).
    queue_write = intr;
    if ((queue_write & 0xFF) == 2)
    {
        CD_SYSTEM.initCommand &= 0x7F;
        // Volatile read forces the reload of initCommand after the masking
        // store above (required to match).
        switch (CD_SYSTEM_V.initCommand)
        {       /* switch 1 */
        case 1: /* switch 1 */
        case 3: /* switch 1 */
            CD_SYSTEM_V.initCommand = 0;
            if (CD_SYSTEM.queueReadIndex != CD_SYSTEM.queueWriteIndex)
            {
                CdSyncCallback(cdrom_complete_command);
                temp_a0 = CD_SYSTEM.commandQueue.items[CD_SYSTEM.queueReadIndex].command;
                if ((temp_a0 == 0x1B) && (CD_SYSTEM.audioEnabled == 0))
                {
                    CD_SYSTEM.audioEnabled = 1;
                }
                CD_SYSTEM.playbackState = 0;
                CD_SYSTEM.transferCallback = NULL;
                cdrom_run_command(temp_a0 & 0xFF, 0, 0);
            }
            else
            {
                CdSyncCallback(NULL);
            }
            break;
        case 2: /* switch 1 */
            CD_SYSTEM_V.initCommand = CD_SYSTEM.initCommand + 1;
            CdControlF(0xE, (u8*)0x801ED950);
            break;
        case 16: /* switch 1 */
            CD_SYSTEM.initState = 2;
            CdSyncCallback(NULL);
            CD_SYSTEM_V.initCommand = 0;
            break;
        case 17: /* switch 1 */
            CD_SYSTEM_V.initCommand = CD_SYSTEM.initCommand + 1;
            CdControlF(0xC, NULL);
            break;
        case 18: /* switch 1 */
            CD_SYSTEM_V.initCommand = CD_SYSTEM.initCommand + 1;
            CdControlF(9, NULL);
            break;
        case 19: /* switch 1 */
            CdSyncCallback(NULL);
            CD_SYSTEM.initState = 0;
            CD_SYSTEM_V.initCommand = 0;
            CD_SYSTEM.statusFlags.word &= ~8;
            break;
        case 33: /* switch 1 */
            CdSyncCallback(NULL);
            CD_SYSTEM_V.initCommand = 0;
            break;
        case 32: /* switch 1 */
        case 34: /* switch 1 */
            CD_SYSTEM.initState = 7;
            CdSyncCallback(NULL);
            CD_SYSTEM_V.initCommand = 0;
            break;
        case 35: /* switch 1 */
            CD_SYSTEM_V.initCommand = 0;
            CD_SYSTEM.initState = 0;
            CD_SYSTEM.retryCounter = 0;
            // Single reused temp keeps gcc's combine pass from folding the
            // ~2 and ~4 masks into one AND; the first store must be volatile
            // or it is eliminated as dead (required to match).
            temp_v1 = CD_SYSTEM.statusFlags.word;

            queue_write = CD_SYSTEM.queueReadIndex;
            queue_read = CD_SYSTEM.queueWriteIndex;

            temp_v1 &= ~1;
            CD_SYSTEM_V.statusFlags.word = temp_v1;
            temp_v1 &= ~2;
            temp_v1 &= ~4;
            CD_SYSTEM.statusFlags.word = *(new_var = &temp_v1);
            if (queue_write != queue_read)
            {
                CD_SYSTEM.currentCommand = 1;
                CD_SYSTEM.statusFlags.word = temp_v1 | 0x10;
                CdSyncCallback(cdrom_complete_command);
                CdSync(0, NULL);
                CdControlF(1, NULL);
            }
            else
            {
                CdSyncCallback(NULL);
                CD_SYSTEM.statusFlags.word &= ~0x10;
            }
            if (g_cdAudioEnabled != 0)
            {
                // Pointer assigned between the two tests so the base address
                // stays in a register (required to match).
                audioSystem = &AUDIO_SYSTEM;
                if (g_cdAudioReady != 0)
                {
                    audioSystem->readFlag = 1;
                }
            }
            break;
        }
        g_cdVSyncTimestamp = VSync(-1);
        return;
    }
    if ((s8)CD_SYSTEM_V.initCommand >= 0)
    {
        CD_SYSTEM_V.initCommand |= 0x80;
        CdControlF(1, NULL);
        return;
    }
    CD_SYSTEM_V.initCommand &= 0x7F;
    switch (CD_SYSTEM_V.initCommand)
    {       /* switch 2 */
    case 3: /* switch 2 */
        CdControlF(0xE, (u8*)0x801ED950);
        return;
    case 16: /* switch 2 */
        CD_SYSTEM.initState = 1;
        CdSyncCallback(NULL);
        CD_SYSTEM_V.initCommand = 0;
        return;
    case 17: /* switch 2 */
        // Two-byte array (not two scalars): keeps the second byte's store
        // from being eliminated as dead (required to match).
        sp10[0] = 1;
        sp10[1] = 1;
        CdControlF(0xD, sp10);
        return;
    case 18: /* switch 2 */
        CdControlF(0xC, NULL);
        return;
    case 1:  /* switch 2 */
    case 2:  /* switch 2 */
    case 19: /* switch 2 */
        CdControlF(9, NULL);
        return;
    case 33: /* switch 2 */
        CdControlF(6, (u8*)0x801ED95C);
        return;
    case 34: /* switch 2 */
        CD_SYSTEM.initState = 7;
        CD_SYSTEM_V.initCommand = 0;
        CdSyncCallback(NULL);
        return;
    case 32: /* switch 2 */
    case 35: /* switch 2 */
        CD_SYSTEM.initState = 6;
        CD_SYSTEM_V.initCommand = 0;
        CdSyncCallback(NULL);
        return;
    default: /* switch 2 */
        return;
    }
}

/**
 * @brief Ready callback invoked when the CD-ROM drive signals a sector is ready.
 *
 * Installed as the CdReadyCallback. Handles the transition from hardware "ready"
 * to software sector processing, for both data reads and XA audio streaming.
 *
 * @details
 * **Data Mode (audioEnabled != 1):**
 * 1. Checks interrupt status; on mismatch/error, reads the sector header to
 *    verify disc position.
 * 2. On correct position, calls cdrom_process_sector.
 * 3. On failure, retries up to 17 times; on exhaustion marks retryExhausted
 *    and issues CdlNop.
 *
 * **Audio Mode (audioEnabled == 1):**
 * 1. Verifies interrupt status against audio state.
 * 2. On success, calls cdrom_process_sector.
 * 3. Uses the same retry mechanism as data mode on failure.
 *
 * @param intr    Completion code from the CD-ROM drive.
 * @param result  Pointer to the drive's status byte.
 *
 * @note Runs in interrupt context; must not call blocking functions.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/kgBY4
 */
void cdrom_handle_ready_intr(u_char intr, u_char* result)
{
    s32 temp_a0;
    u8 temp_s1;
    u8 temp_v0;
    int temp2;
    u8 temp_v0_2;
    u8 temp_v0_3;
    u8 var_a0;
    u8* var_a1;
    u8* addr;
    int new_var;

    volatile CdSystem* cdSystem;

    CD_SYSTEM.syncComplete = 1;
    temp_s1 = CD_SYSTEM.audioEnabled;
    if (temp_s1 != 1)
    {

        temp_a0 = intr & 0xFF;
        if ((temp_a0 == 1) && (CD_SYSTEM.statusFlags.bytes.b2 == 0))
        {
            temp_v0 = CD_SYSTEM.statusFlags.bytes.b1;
            temp2 = temp_v0 & 0xFF;

            if (temp2 == temp_a0)
            {
                CD_SYSTEM.statusFlags.bytes.b2 = temp2;
                return;
            }

            do
            {

            } while (CdGetSector((void*)0x801ED940, 3) == 0);

            if ((CD_SYSTEM.sectorHeaderBuffer[0] & 0xFFFFFF) == (CD_SYSTEM.currentLocation.raw & 0xFFFFFF))
            {
                cdrom_process_sector(0);
                return;
            }
        }

        temp_v0_2 = CD_SYSTEM.retryCount;
        CD_SYSTEM.retryCount = (u8)(temp_v0_2 + 1);
        if ((u32)(temp_v0_2 & 0xFF) < 0x11U)
        {
            var_a1 = (u8*)0x801ED958;
            var_a0 = CD_SYSTEM.currentCommand;
            CdControlF(var_a0, var_a1);
            return;
        }

        CD_SYSTEM.statusFlags.bytes.retryExhausted = 1U;
        CD_SYSTEM.retryCount = 0U;

        if (CD_SYSTEM.transferCallback != NULL)
        {
            CD_SYSTEM.playbackState = 1U;
        }
        else
        {
            CD_SYSTEM.playbackState = 0U;
        }

        CdReadyCallback(NULL);
        cdSystem = &CD_SYSTEM;
        cdSystem->currentCommand = 1U;
        var_a0 = 1;
        var_a1 = NULL;
        CdControlF(var_a0, var_a1);
        return;
    }

    new_var = intr & 0xFF;
    temp2 = temp_s1;
    if (new_var == temp2)
    {
        var_a0 = D_801ED590 == 0;
        addr = (u8*)0x801ED500;
        if (var_a0 && ((*(((u8*)addr) + 0x9C)) != 0))
        {
            (*((CdSystem*)0x801ED800)).statusFlags.bytes.b2 = temp2;
            return;
        }
        cdrom_process_sector(0);
        return;
    }

    temp_v0_3 = CD_SYSTEM.retryCount;
    CD_SYSTEM.retryCount = (u8)(temp_v0_3 + 1);
    if ((u32)(temp_v0_3 & 0xFF) >= 0x11U)
    {
        (*((CdSystem*)0x801ED800)).statusFlags.bytes.retryExhausted = temp2;
        CD_SYSTEM.retryCount = 0U;
        (*((CdSystem*)0x801ED800)).playbackState = temp2;
        CdReadyCallback(NULL);
        var_a0 = 1;
        var_a1 = NULL;
        (*((CdSystem*)0x801ED800)).currentCommand = temp2;
        CdControlF(var_a0, var_a1);
    }
    return;
}

/**
 * @brief Handles completion of a CD-ROM sector read operation.
 *
 * Called when the CD drive signals a sector is ready. Processes data in either
 * data mode or audio (XA) mode, and manages multi-sector transfers by re-issuing
 * read commands until all data is received.
 *
 * @details
 * **Data mode (audioEnabled != 1):**
 * 1. If transferCallback is set, calls it for the destination buffer; NULL return
 *    retries the current read. Otherwise uses currentWritePtr.
 * 2. If more than one sector remains (>= 0x801 bytes): reads 0x800 bytes via
 *    CdGetSector, advances disc position, decrements remaining size.
 * 3. On the final sector: resets playbackState/transferCallback, advances queue.
 *    If more commands are queued, dispatches the next via cdrom_run_command.
 *    Otherwise transitions to idle, reads the final partial sector, issues CdlPause.
 *
 * **Audio mode (audioEnabled == 1):**
 * 1. Reads 3 words into sectorHeaderBuffer; compares lower 24 bits against
 *    currentLocation. On mismatch, re-issues the current command.
 * 2. On match, calls transferCallback. NULL return (end of track) advances the
 *    queue, disables audio, and pauses the drive. Non-NULL continues streaming.
 *
 * @param arg0  0 = initial call from ready callback (pause before final read),
 *              non-zero = chained from cdrom_run_command (pause after final read).
 *
 * @note
 * - CD_COMMAND_PARAM_BUFFER (0x801ED958) holds the current CdlLOC for read commands.
 * - The 0xFFFFFF mask extracts MSF BCD position, ignoring the mode byte.
 * - Final partial sector size in words: (g_cdReadRemainingBytes + 3) >> 2.
 *
 * @warning Spin-waits on CdGetSector. Must only be called from the CD ready callback.
 *          In audio mode, transferCallback must be non-NULL.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/43gwj
 */
void cdrom_process_sector(s32 arg0)
{
    void* buffer;
    volatile CdSystem* cdSystem;

    cdSystem = &CD_SYSTEM;

    // Reset retry tracking and clear status flag bytes b2/retryExhausted
    CD_SYSTEM.retryCount = 0;
    CD_SYSTEM.statusFlags.bytes.retryExhausted = 0;
    CD_SYSTEM.statusFlags.bytes.b2 = 0;

    // === Data mode path ===
    if (CD_SYSTEM.audioEnabled != 1)
    {

        // Determine destination buffer via transferCallback callback or currentWritePtr
        if (CD_SYSTEM.transferCallback != NULL)
        {
            // transferCallback(bytesTransferred, bytesRemaining) returns destination buffer
            buffer = CD_SYSTEM.transferCallback(CD_SYSTEM.totalDataSize - CD_SYSTEM.readRemainingBytes, CD_SYSTEM.readRemainingBytes);
            if (buffer == NULL)
            {
                // Callback rejected the transfer — re-issue the current read command
                CdControlF(cdSystem->currentCommand, (u8*)0x801ED958);
                return;
            }
        }
        else
        {
            buffer = CD_SYSTEM.currentWritePtr;
        }

        // More than one sector remaining — read a full 0x800-byte sector
        if (CD_SYSTEM.readRemainingBytes >= 0x801U)
        {
            // Spin-wait until sector data is available (0x200 words = 0x800 bytes)
            while (CdGetSector(buffer, 0x200) == 0);

            // Advance disc position to next sector
            CdIntToPos(CdPosToInt((CdlLOC*)0x801ED958) + 1, (CdlLOC*)0x801ED958);

            // Decrease remaining byte count by one sector
            CD_SYSTEM.readRemainingBytes = (CD_SYSTEM.readRemainingBytes - 0x800);

            // If no callback, linearly advance the destination pointer
            if (CD_SYSTEM.transferCallback == NULL)
            {
                CD_SYSTEM.currentWritePtr = (void*)(CD_SYSTEM.currentWritePtr + 0x800);
            }
        }
        else
        {
            // === Final sector — complete the transfer ===
            CD_SYSTEM.playbackState = 0;
            CD_SYSTEM.transferCallback = NULL;

            // Advance queue read index (circular, mod 16)
            CD_SYSTEM.queueReadIndex = (CD_SYSTEM.queueReadIndex + 1) & 0xF;

            // If more commands are queued, dispatch the next one immediately
            if (CD_SYSTEM.queueReadIndex != CD_SYSTEM.queueWriteIndex)
            {
                cdrom_run_command(CD_SYSTEM.commandQueue.items[CD_SYSTEM.queueReadIndex].command, buffer, arg0 + 1);
                return;
            }

            // No more queued commands — transition to idle state
            CD_SYSTEM.initCommand = 1;
            CdSyncCallback(cdrom_handle_recovery_sync);
            CdReadyCallback(NULL);

            // If initial call (arg0 == 0), pause drive before reading final sector
            if (arg0 == 0)
            {
                CdControlF(CdlPause, NULL);
            }

            // Read the final partial sector (size converted from bytes to words)
            while (CdGetSector(buffer, (g_cdReadRemainingBytes + 3) >> 2) == 0);

            cdSystem = &CD_SYSTEM;

            // Clear busy flag (bit 4) and reset command/retry state
            CD_SYSTEM.statusFlags.word &= ~0x10;
            cdSystem->currentCommand = 0U;
            cdSystem->retryCounter = 0;

            // If chained call (arg0 != 0), pause drive after reading final sector
            if (arg0 != 0)
            {
                CdControlF(CdlPause, NULL);
            }

            // Record frame counter for timeout tracking
            CD_SYSTEM.vsyncTimestamp = VSync(-1);
        }

        return;
    }

    // === Audio (XA) mode path ===

    // Read 3 words (12 bytes) of sector header into read buffer
    while (CdGetSector(&CD_SECTOR_HEADER_BUFFER, 3) == 0);

    cdSystem = &CD_SYSTEM;

    // Verify disc position: compare lower 24 bits (min/sec/sector BCD)
    // of the read sector against the expected command parameter position
    if ((CD_SYSTEM.sectorHeaderBuffer[0] & 0xFFFFFF) == (CD_SYSTEM.currentLocation.raw & 0xFFFFFF))
    {

        // Position matches — invoke transferCallback callback to check if audio is complete
        if (CD_SYSTEM.transferCallback(CD_SYSTEM.totalDataSize - CD_SYSTEM.readRemainingBytes, CD_SYSTEM.readRemainingBytes) == NULL)
        {

            // Audio track complete — shut down audio playback
            CD_SYSTEM.queueReadIndex = ((CD_SYSTEM.queueReadIndex + 1) & 0xF);
            CdSyncCallback(cdrom_handle_recovery_sync);
            CdReadyCallback(NULL);

            // Restore default CD mode (double speed + 2340-byte sectors)
            cdSystem->setModeParamBlocking[0] = 0xA0;
            cdSystem->currentCommand = 0U;
            cdSystem->initCommand = 2;
            cdSystem->audioEnabled = 0U;
            cdSystem->playbackState = 0;
            cdSystem->transferCallback = NULL;
            cdSystem->retryCounter = 0;

            // Clear busy flag (bit 4) and pause drive
            CD_SYSTEM.statusFlags.word &= ~0x10;
            CdControlF(CdlPause, NULL);
            CD_SYSTEM.vsyncTimestamp = VSync(-1);
        }
        else
        {
            // Audio continues — advance disc position to next sector
            CdIntToPos(CdPosToInt((CdlLOC*)0x801ED958) + 1, (CdlLOC*)0x801ED958);
        }

        return;
    }

    // Position mismatch — re-issue read command with expected position
    CdControlF(cdSystem->currentCommand, (u8*)0x801ED958);
}

/**
 * @brief Dispatches a CD-ROM command and configures hardware for sector reads.
 *
 * Translates a command byte and execution mode into the appropriate PsyQ CD
 * library calls, handling seeking, reading, and callback management.
 *
 * @details
 * 1. **Seek skipping:** CdlSeekL is treated as a no-op; skips forward in the
 *    queue until a non-seek command is found.
 *
 * 2. **Read commands (CdlReadN, CdlReadS, etc.):**
 *    - Configures CD_SYSTEM with resource data size, remaining bytes,
 *      destination buffer, and transfer callback.
 *    - In async mode (executionMode == 0), installs cdrom_handle_ready_intr
 *      as the CdReadyCallback.
 *
 * 3. **Generic commands (CdlPause, CdlSetmode, etc.):**
 *    - Mode 0: async dispatch via CdControlF.
 *    - Mode 1: synchronous; blocks on CdGetSector during dispatch.
 *    - Mode 2: synchronous; reads sector first, then issues command.
 *
 * @param command        CD-ROM command byte to execute.
 * @param sectorBuffer   Buffer for sector data in synchronous/blocking modes.
 * @param executionMode  0 = async, 1 = sync (block during), 2 = sync (block before).
 *
 * @note CdlSetmode (0xE) reads its parameter from CD_COMMAND_PARAM_BUFFER (0x801ED950).
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/KM6id
 */
void cdrom_run_command(u8 cmd, void* sectorBuffer, s32 executionMode)
{
    u8* paramBufferSpecialCmd;
    s32 nextReadIndex;
    s32 dataSize;
    s32 controlParam;
    CdResourceEntry* queuedLocation;
    u32 queueEntryPtr;
    void* queueBufferPtr;
    volatile CdSystem* cdSystem;

    queuedLocation = 0;

    while (cmd == CdlSeekL)
    {
        // Calculate next read index with circular buffer wrapping
        nextReadIndex = (CD_SYSTEM_V.queueReadIndex + 1) & 0xF;

        // Stop once the process has iterated through all indices
        if (CD_SYSTEM.queueWriteIndex == nextReadIndex)
        {
            break;
        }

        // Advance read index and get next command
        CD_SYSTEM_V.queueReadIndex = nextReadIndex;
        cmd = CD_SYSTEM_V.commandQueue.items[nextReadIndex].command;
    }

    if ((cmd == 0x15) || (cmd == 0x06) || (cmd == 0x1B))
    {
        // Reset playback state and get queue location
        if (cmd == 0x15 || g_playbackState == 0)
        {
            CD_SYSTEM_V.transferCallback = NULL;
            CD_SYSTEM_V.playbackState = 0;
            queuedLocation = CD_SYSTEM_V.commandQueue.items[CD_SYSTEM.queueReadIndex].entry;
            CD_SYSTEM.currentLocation = queuedLocation->location;
        }

        switch (executionMode)
        {
        case 1:
            CD_SYSTEM_V.currentCommand = cmd;
            CdControlF(cmd, CD_COMMAND_PARAM_BUFFER);
            while (CdGetSector(sectorBuffer, (g_cdReadRemainingBytes + 3) >> 2) == 0);
            break;

        case 2:
            while (CdGetSector(sectorBuffer, (g_cdReadRemainingBytes + 3) >> 2) == 0);
            CdSync(0, 0);
            break;
        }

        if ((cmd == 0x06) || (cmd == 0x1B))
        {
            queueEntryPtr = (CD_SYSTEM_V.queueReadIndex * 0x10) + 0x801ED800;
            if (((*(((u32*)queueEntryPtr) + 0x13)) == 0) && (CD_SYSTEM_V.currentWritePtr == *(void**)((u8*)queueEntryPtr + 0x48)))
            {
                CD_SYSTEM_V.playbackState = 0;
            }

            cdSystem = &CD_SYSTEM_V;
            if (g_playbackState == 0)
            {
                dataSize = queuedLocation->dataSize;
                queueBufferPtr = QUEUE_ITEM_BASE(cdSystem->queueReadIndex);
                CD_SYSTEM_V.totalDataSize = dataSize;
                CD_SYSTEM_V.readRemainingBytes = dataSize;
                CD_SYSTEM_V.currentWritePtr = (void*)QUEUE_ITEM_DST_BUFFER(queueBufferPtr);
                CD_SYSTEM_V.transferCallback = QUEUE_ITEM_CALLBACK(queueBufferPtr);
            }

            if (executionMode == 0)
            {
                CD_SYSTEM_V.statusFlags.bytes.b2 = 0;
                CdReadyCallback(cdrom_handle_ready_intr);
            }
        }
        else if (executionMode == 1)
        {
            CdReadyCallback(NULL);
        }

        if (executionMode != 1)
        {
            CD_SYSTEM_V.currentCommand = cmd;
            CdControlF(cmd, CD_COMMAND_PARAM_BUFFER);
        }

        g_playbackState = 0;
        return;
    }

    // Handle other commands based on execution mode
    switch (executionMode)
    {
    case 0:
        CD_SYSTEM_V.currentCommand = cmd;

        if (cmd == 0xE)
        {
            controlParam = 0xE;
            paramBufferSpecialCmd = (u8*)0x801ED950;
        }
        else
        {
            controlParam = cmd;
            paramBufferSpecialCmd = 0;
        }
        break;

    case 1:
        CdReadyCallback(0);
        CD_SYSTEM_V.currentCommand = cmd;
        controlParam = 0;
        CdControlF(cmd, 0);
        while (CdGetSector(sectorBuffer, (g_cdReadRemainingBytes + 3) >> 2) == 0);
        return;

    case 2:
        while (CdGetSector(sectorBuffer, (g_cdReadRemainingBytes + 3) >> 2) == 0);
        CD_SYSTEM_V.currentCommand = cmd;
        controlParam = cmd;
        paramBufferSpecialCmd = 0;
        break;

    default:
        return;
    }

    CdControlF(controlParam, paramBufferSpecialCmd);
}

/**
 * @brief Verifies the disc's authenticity against a hardcoded validation ID.
 *
 * Installed as a CdReadyCallback during initialization. Reads a specific sector
 * and compares its contents against g_DiscValidationId to confirm the disc is valid.
 *
 * @details
 * 1. **Position check:** Reads the sector header (3 words); compares lower 24 bits
 *    against recoveryReadPosition.
 * 2. **ID extraction:** Reads 8 words into CD_SYSTEM.discValidationId.
 * 3. **Comparison:** Byte-by-byte against g_DiscValidationId, with Shift-JIS
 *    two-byte character handling (lead bytes 0x80–0x9F, 0xE0–0xEF).
 * 4. **Outcome:**
 *    - Match: advances initState, installs cdrom_handle_recovery_sync, sends CdlSetmode.
 *    - Mismatch: enters CD_INIT_STATE_ERROR_PAUSE and sends CdlPause.
 *
 * @param intr    Completion code from the CD-ROM drive.
 * @param result  Pointer to the drive's status byte.
 *
 * @note Called in interrupt context.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/XrcPe
 */
void cdrom_verify_disc(u_char intr, u_char* result)
{
    s32 statusWord;
    u_char expectedChar;
    u_char discChar;
    const u_char* expectedId;
    u_char* discId;

    CD_SYSTEM_V.syncComplete = TRUE;

    if (intr == 1)
    {
        // Wait until the sector header (3 words) is ready
        while ((expectedChar = (CdGetSector(&CD_SYSTEM.sectorHeaderBuffer, 3) == 0)));

        // Verify the sector position matches the expected recovery read position (24-bit compare)
        if ((CD_SYSTEM.sectorHeaderBuffer[0] & 0xFFFFFF) == (CD_SYSTEM.recoveryReadPosition.raw & 0xFFFFFF))
        {
            // Wait until the disc validation ID (8 words) is ready
            while (CdGetSector(&CD_SYSTEM.discValidationId, 8) == 0);

            // Walk both strings simultaneously, verifying the disc contains the expected ID.
            // Shift-JIS lead bytes (0x81-0x9F or 0xE0-0xEF) introduce two-byte characters;
            // both the lead and trail byte must match before continuing.
            expectedId = g_DiscValidationId;
            discId = CD_SYSTEM.discValidationId;
            expectedChar = *expectedId++;

            while (expectedChar != 0)
            {
                // Special-range characters [0x80,0x9F] or [0xE0,0xEF] use a two-byte match
                // expectedChar must equal *pDiscData, then the following bytes are compared.
                if (((u8)(expectedChar + 0x80) < 0x20u) || ((u8)(expectedChar + 0x20) < 0x10u))
                {
                    // Two-byte character: verify the lead byte matches first
                    if (expectedChar != *discId++)
                    {
                        goto validation_failed;
                    }

                    // Then load the trail bytes from each string for the main compare below
                    expectedChar = *discId++;
                    discChar = *expectedId++;
                }
                else
                {
                    // Single-byte ASCII character: just advance the disc pointer
                    discChar = *discId++;
                }

                if (expectedChar != discChar)
                {
                validation_failed:
                    statusWord = CD_SYSTEM.statusFlags.word & ~4u;
                    CD_SYSTEM_V.initState = CD_INIT_STATE_ERROR_PAUSE;
                    CD_SYSTEM_V.statusFlags.word = statusWord;
                    CD_SYSTEM.statusFlags.word = statusWord & ~CdlStatShellOpen;
                    CdReadyCallback(NULL);
                    return;
                }

                expectedChar = *expectedId++;
            }

            // Validation passed: set disc mode and begin CD reads
            CdReadyCallback(NULL);
            CD_SYSTEM_V.initCommand = 0x23; // TODO: name this constant
            CdSyncCallback(cdrom_handle_recovery_sync);
            CdControlF(CdlSetmode, (u_char*)0x801ED950);
            return;
        }
    }

    // Sector position mismatch or wrong interrupt type: pause and signal error
    CdReadyCallback(NULL);
    CD_SYSTEM_V.initCommand = 0x22; // TODO: name this constant
    CdSyncCallback(cdrom_handle_recovery_sync);
    CdControlF(CdlPause, NULL);
}

/**
 * @brief Blocks until all pending CD-ROM commands have been processed.
 *
 * Polls cdrom_process_state() each frame, yielding via VSync(0) between calls,
 * until the command queue is empty.
 *
 * @warning Blocking; may stall for several frames. Use only when absolute
 *          synchronization is required.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/rE8hd
 */
void cdrom_wait_queue_empty(void)
{
    int remaining;

    while (remaining = cdrom_process_state(), remaining != 0)
    {
        VSync(0);
    }
}

/**
 * @brief Resets CD state and enters error recovery after a sync failure.
 *
 * Clears sync and ready callbacks, sets the error flag (statusFlags bit 0),
 * resets all command and retry counters, clears the busy flag (bit 4),
 * and records the current VSync timestamp.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/lU7lO
 */
void cdrom_handle_sync_error(void)
{
    CdSyncCallback(NULL);
    CdReadyCallback(NULL);

    CD_SYSTEM.initState = 0;
    CD_SYSTEM.statusFlags.word |= 1;
    CD_SYSTEM.currentCommand = 0;
    CD_SYSTEM.initCommand = 0;
    CD_SYSTEM.retryCount = 0;
    CD_SYSTEM.retryCounter = 0;
    CD_SYSTEM.statusFlags.word &= ~0x10;
    CD_SYSTEM.vsyncTimestamp = VSync(-1);
}

/**
 * @brief Sets the CD-DA audio mix volume and channel routing.
 *
 * @param volume         Volume level (0–255).
 * @param stereoChannel  0 = route both CD channels to SPU left only,
 *                       1 = route CD left to both speakers (mono).
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/lwzx1
 */
void cdrom_set_audio_volume(u_char volume, s32 mixMode)
{
    CdlATV audioConfig[2];

    while (TRUE)
    {
        if (mixMode != 0)
        {
            audioConfig[0].val0 = volume;
            audioConfig[0].val1 = 0;
            audioConfig[0].val2 = volume;
        }
        else
        {
            audioConfig[0].val0 = volume;
            audioConfig[0].val1 = volume;
            audioConfig[0].val2 = 0;
        }

        audioConfig[0].val3 = 0;
        break;
    }

    CdMix(audioConfig);
}

/**
 * @brief Resets the CD subsystem and stops any ongoing XA audio playback.
 *
 * Restores DecDCT and DrawSync callbacks, clears CD sync/ready callbacks,
 * pauses the drive, stops CD audio if active, and resets all internal state.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/fnucZ
 */
void cdrom_reset(void)
{
    AudioSystem* audioSystem = &AUDIO_SYSTEM;

    DecDCToutCallback(audioSystem->decDCToutCallbackHandler);
    DrawSyncCallback(audioSystem->drawSyncCallbackHandler);

    CdSyncCallback(NULL);
    CdReadyCallback(NULL);

    while (CdControlB(CdlPause, NULL, NULL) == 0);

    if (g_cdAudioReady != 0)
    {
        akao_cmd_e2();
    }

    CD_SYSTEM.audioEnabled = 0;
    CD_SYSTEM.currentCommand = 0;
    CD_SYSTEM.initCommand = 0;
    CD_SYSTEM.queueReadIndex = 0;
    CD_SYSTEM.queueWriteIndex = 0;
    CD_SYSTEM.retryCounter = 0;
    CD_SYSTEM.playbackState = 0;
    CD_SYSTEM.transferCallback = NULL;
    CD_SYSTEM.statusFlags.word &= ~0x10;
    CD_SYSTEM.vsyncTimestamp = VSync(-1);
}

/**
 * @brief Checks whether a resource index is absent from the pending command queue.
 *
 * Scans every pending entry in the circular command queue and returns whether
 * the given resource index is not already present, indicating it is safe to
 * enqueue a new command for that resource without creating a duplicate.
 *
 * @details
 * The scan performs the following steps:
 *
 * 1. Reads queueReadIndex as the starting scan position
 * 2. Computes the number of pending entries as (queueWriteIndex - queueReadIndex) & 0xF
 * 3. Decrements that count by 1 and compares against a sentinel of -1 to detect an
 *    empty queue (no iterations performed)
 * 4. For each pending slot, compares the stored resourceIndex against the lower 16
 *    bits of the argument; returns 0 immediately on a match (duplicate found)
 * 5. Advances scanIndex by masking with 0xF before incrementing to maintain circular
 *    wrap semantics within the 16-entry buffer
 * 6. Returns 1 if the full queue was scanned with no match
 *
 * @note
 * - Only the lower 16 bits of resourceIndex are compared, matching the u16 storage
 *   in CdCommandQueueItem
 * - The decrement-before-loop pattern and sentinel value of -1 match the original
 *   assembly's register usage exactly and must not be restructured
 * - The mask-then-increment sequence (scanIndex = (scanIndex & 0xF) + 1) matches
 *   the original assembly's andi + addiu pair for register-level equivalence
 *
 * @warning
 * - Not interrupt-safe; the queue indices and entries may change between reads if
 *   called while a CD callback is active
 * - Does not prevent a race between this check and a subsequent cdrom_queue_command call;
 *   the caller must not assume the result remains valid across VSync frames
 *
 * @param resourceIndex  Resource index to search for in the queue (lower 16 bits used)
 *
 * @return 1 if the resource index is not already queued (safe to enqueue),
 *         0 if a matching entry was found (duplicate present)
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/l4HlL
 */
s32 cdrom_can_queue_resource(s32 resourceIndex)
{
    s32 queuedResourceIndex;
    s32 scanIndex;
    s32 remainingEntries;

    scanIndex = CD_SYSTEM.queueReadIndex;

    // Calculate number of pending entries in the circular queue
    remainingEntries = ((CD_SYSTEM.queueWriteIndex - scanIndex) & 0x0F);

    // If queue is non-empty, scan all pending entries for a match
    while (--remainingEntries != -1)
    {

        // Check if this queued entry already targets the same resource
        queuedResourceIndex = CD_SYSTEM.commandQueue.items[scanIndex].resourceIndex;

        if ((resourceIndex & 0xFFFF) == queuedResourceIndex)
        {
            return 0;
        }

        // Advance scan index with circular wrap (mod 16)
        scanIndex &= 0xF;
        scanIndex++;
    }

    return 1;
}

/**
 * @brief Initializes the default CD resource and loads the resource table from disc.
 *
 * Converts a raw LBA sector address to MSF, stores it as the default CD resource,
 * enqueues a CdlReadN to load the resource entry table, blocks until complete,
 * then applies a default audio volume of 128.
 *
 * @details
 * Synchronizes with g_cdVSyncTimestamp before issuing commands to avoid conflicts
 * with any in-flight CD operation.
 *
 * @param lba           Logical block address of the target sector.
 * @param dataSizeBytes Size in bytes stored as the default resource's dataSize.
 *
 * @warning Blocks until the CD command queue is drained. Must not be called
 *          from within a CD callback.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/Y9z7y
 */
void cdrom_load_resource_table(s32 lba, s32 dataSizeBytes)
{
    CdlLOCRaw* location;
    int vsyncOffset;
    int vsyncDelta;
    CdSystem* cdStruct;

    vsyncOffset = -3;
    vsyncDelta = g_cdVSyncTimestamp - (VSync(-1) + vsyncOffset);

    if (vsyncDelta > 0)
    {
        if (vsyncDelta == 1)
        {
            vsyncDelta = 0;
        }

        VSync(vsyncDelta);
    }

    cdStruct = &CD_SYSTEM;
    location = &cdStruct->defaultCdResource.location;
    cdStruct->defaultCdResource.location.raw = 0;
    cdStruct->defaultCdResource.dataSize = dataSizeBytes;

    CdIntToPos(lba, &location->pos);
    cdrom_queue_command(CdlReadN, CD_RESOURCE_INDEX_DEFAULT, CD_RESOURCE_ENTRIES, NULL);
    cdrom_wait_queue_empty();
    cdrom_set_audio_volume(128, 1);
}

/**
 * @brief Enqueues a CdlReadN command for the given resource and destination buffer.
 *
 * @param resourceIndex  Index into CD_RESOURCE_ENTRIES identifying the data to read.
 * @param dstBuffer      Destination buffer for the sector data.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/OxunQ
 */
void cdrom_queue_read(s32 resourceIndex, void* dstBuffer)
{
    cdrom_queue_command(CdlReadN, resourceIndex, dstBuffer, 0);
}

/**
 * @brief Enqueues a CdlReadN command for the given resource with a completion callback.
 *
 * Like cdrom_queue_read, but delivers sector data through a callback instead of
 * writing directly to a destination buffer. Passes only the lower 16 bits of
 * resourceIndex to cdrom_queue_command.
 *
 * @param resourceIndex  Index into CD_RESOURCE_ENTRIES (lower 16 bits used).
 * @param callback       Invoked on command completion with sector data.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/5M5cV
 */
void cdrom_queue_read_with_callback(s32 resourceIndex, CdCommandCallback callback)
{
    cdrom_queue_command(CdlReadN, resourceIndex & 0xFFFF, 0, callback);
}

/**
 * @brief Enqueues a CdlSeekL command to pre-position the disc head.
 *
 * @param resourceIndex  Index into CD_RESOURCE_ENTRIES identifying the target position.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/iUUQh
 */
void cdrom_queue_seek(s32 resourceIndex)
{
    cdrom_queue_command(CdlSeekL, resourceIndex, 0, 0);
}

/**
 * @brief Returns the data size of a CD resource entry.
 *
 * @param resourceIndex  Index into CD_RESOURCE_ENTRIES (lower 16 bits used).
 *
 * @return The dataSize field of the resource entry in bytes.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/SGZF5
 */
s32 cdrom_get_resource_size(s32 resourceIndex)
{
    return CD_RESOURCE_ENTRIES[resourceIndex & 0xffff].dataSize;
}

/**
 * @brief Returns a numeric code describing the current CD subsystem error state.
 *
 * Reads CD_SYSTEM.statusFlags and maps the active error bits to a code:
 *
 *   Code  Condition
 *   ----  ---------
 *   0     No error (all flags clear, retryExhausted == 0)
 *   1     statusFlags bit 0 set
 *   2     statusFlags bits 1 and 2 both set
 *   3     statusFlags bit 1 set, bit 2 clear
 *   4     statusFlags bit 2 set, bit 1 clear
 *   5     retryExhausted == 1
 *
 * @return Error code (0 = OK, 1–5 = error condition).
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/vfLUw
 */
s32 cdrom_get_error_status(void)
{
    CdStatusFlags flags;

    flags = CD_SYSTEM.statusFlags;

    if (flags.bytes.b0 & 1)
    {
        return 1;
    }

    if ((flags.bytes.b0 & 2) != 0)
    {
        if (flags.bytes.b0 & 4)
        {
            return 2;
        }

        return 3;
    }

    if (flags.bytes.b0 & 4)
    {
        return 4;
    }

    if (CD_SYSTEM.statusFlags.bytes.retryExhausted == 1)
    {
        return 5;
    }

    return 0;
}

/**
 * @brief Restores previously saved CD callbacks and resets all subsystem state.
 *
 * Reinstalls the sync and ready callbacks that were saved before the last
 * cdrom_init or cdrom_stop call, issues a blocking CdControlB(9) to pause
 * the drive, then clears all queue indices, command state, and status flags.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/HSXMR
 */
void cdrom_restore_callbacks(void)
{
    CdSyncCallback(CD_SYSTEM.previousSyncCallback);
    CdReadyCallback(CD_SYSTEM.previousReadyCallback);

    while (CdControlB(9U, NULL, NULL) == 0);

    CD_SYSTEM.resourceIndex = 0xFFFE;
    CD_SYSTEM.pendingQueueCount = 0;
    CD_SYSTEM.currentResourceIndex = 0;
    CD_SYSTEM.currentDataSize = 0;
    CD_SYSTEM.targetDataSize = 0;
    CD_SYSTEM.playbackState = 0;
    CD_SYSTEM.transferCallback = NULL;
    CD_SYSTEM.currentCommand = 0;
    CD_SYSTEM.initCommand = 0;
    CD_SYSTEM.retryCount = 0;
    CD_SYSTEM.retryCounter = 0;
    CD_SYSTEM.lastCommand = 0;
    CD_SYSTEM.dstBuffer = 0;
    CD_SYSTEM.callback = NULL;
    CD_SYSTEM.statusFlags.word &= ~0x10;
    CD_SYSTEM.statusFlags.bytes.b1 = 0;
    CD_SYSTEM.statusFlags.bytes.b2 = 0;
    CD_SYSTEM.vsyncTimestamp = VSync(-1);
    CD_SYSTEM.queueReadIndex = 0;
    CD_SYSTEM.queueWriteIndex = 0;

    CdFlush();
}

/**
 * @brief Requests entry into CD recovery mode if the subsystem is idle.
 *
 * Sets statusFlags bit 3 (the recovery flag) and resets initState to 0 only
 * when all of the following are true: no command is active (currentCommand == 0),
 * no init command is pending (initCommand == 0), no error flags are set (bits 0-2
 * clear), and the queue is empty (queueReadIndex == queueWriteIndex).
 *
 * @return 1 if recovery mode is active (either already set, or just entered),
 *         0 if the subsystem was busy and the request was not applied.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/gsUc3
 */
s32 cdrom_enter_recovery_mode(void)
{
    s32 flags;
    s32 result;

    flags = CD_SYSTEM.statusFlags.word;
    result = 0;

    if (flags & 8)
    {
        return 1;
    }

    if (CD_SYSTEM.currentCommand == 0)
    {
        if ((CD_SYSTEM.initCommand == 0) && !(flags & 7) && (CD_SYSTEM.queueReadIndex == CD_SYSTEM.queueWriteIndex))
        {
            result = 1;
            CD_SYSTEM.statusFlags.word |= 8;
            CD_SYSTEM.initState = 0;
        }
    }

    return result;
}

/**
 * @brief Sets byte 1 of CD_SYSTEM.statusFlags to 1.
 *
 * Writes 1 to D_801ED801 (CdStatusFlags.bytes.b1), signalling a status
 * condition in the CD subsystem. No callers exist in the main binary;
 * this function is invoked from overlay code.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/9bgSH
 */
void func_80014434(void)
{
    D_801ED801 = 1;
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
 *   0xFC    [offLo] [offHi_cnt]             Back-reference: 12-bit offset, count = upper nibble + 4
 *   0xFD    [offset] [count]                Back-reference: 8-bit offset, count + 0x14 bytes
 *   0xFE    [packed]                        Back-reference: offset = (upper nibble << 3) + 8, count = lower nibble + 3
 *   0xFF    (none)                          End-of-stream; updates pointers and returns 0
 *   default (opcode value)                  Raw copy: opcode + 1 bytes follow
 *
 * Terminates early (returning 1) if srcStart reaches srcEnd or dstStart reaches dstEnd.
 *
 * @param srcStart  Current source read position; updated on return.
 * @param dstStart  Current destination write position; updated on return.
 * @param srcEnd    Exclusive upper bound of the source buffer.
 * @param dstEnd    Exclusive upper bound of the destination buffer.
 *
 * @return 0 on 0xFF end-of-stream, 1 if a buffer limit was reached first.
 *
 * @warning No bounds checking on back-reference offsets (0xFC–0xFE); a malformed
 *          stream can read before the start of the destination buffer.
 *
 * @see decomp.me: (99.83%) https://decomp.me/scratch/MlH6P
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

    u32 something;
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
            param2 = param1 >> 4;
            param1 = param1 & 0xf;

            do
            {
                dstPtr[0] = param1;
                dstPtr[1] = param2;
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
            something = srcPtr[3];
            nextByte = srcPtr[1];
            param3 = srcPtr[4];

            iterations = nextByte + 3;
            seed = param3 << 24; // place param3 in the high byte for sign extension
            srcPtr += 5;

            do
            {
                // Write the two current bytes
                ((u8*)dstPtr)[0] = param2;
                ((u8*)dstPtr)[1] = something;
                dstPtr += 2;

                // Sign-extend param3 via arithmetic right shift
                tempSum = seed >> 24;

                // Form the 16-bit value (param0 << 8) | param2

                tempSum += (something << 8) | param2; // add to the sign-extended constant

                // Update for next iteration
                param2 = tempSum;           // low byte
                something = (tempSum >> 8); // high byte
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
 * @brief Transfer callback that manages the ring buffer during sector streaming.
 *
 * Installed as CD_SYSTEM.transferCallback during cdrom_stream and cdrom_stream_chunked.
 * On the first call (arg0 == 0), initializes CdStreamState in scratchpad RAM and
 * returns the ring buffer base address. On subsequent calls, compacts unconsumed
 * bytes and advances the write pointer for the next incoming sector.
 *
 * @param bytes_transferred  Bytes delivered so far; 0 on the first call (initialization), non-zero on each subsequent
 * sector arrival.
 * @param bytes_remaining    Bytes still to read in the stream, passed as readRemainingBytes; clamped to 0x800 per
 * sector.
 *
 * @return Destination address for the next sector DMA write.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/UDwSD
 */
s32* cdrom_handle_stream_data(s32 bytes_transferred, u32 bytes_remaining)
{
    s32 unconsumed;
    s32 align_pad;
    s32 word_count;
    s32 word_count_b;
    s32 aligned_bytes;
    CdStreamState* state;
    s32 aligned_bytes_b;
    u8* dst;
    u8* src;
    u8* src_b;
    u32 bytes_buffered;
    u32 bytes_consumed;
    u32 wrap_overflow;
    u32 read_ptr;
    u32 read_ptr_b;
    u32 read_ptr_c;
    u32 old_wrap_overflow;
    u32 aligned_write_base;
    u32 chunk_size;
    s32* result;
    volatile CdStreamState* flag_state;

    chunk_size = bytes_remaining;
    if (bytes_remaining >= 0x801U)
    {
        chunk_size = 0x800;
    }

    if (bytes_transferred == 0)
    {
        CD_STREAM_STATE.dataReady = 1;
        CD_STREAM_STATE.writePtr = 0x801DC001U;
        CD_STREAM_STATE.readPtr = 0x801DC001U;
        CD_STREAM_STATE.bytesBuffered = (s32)(chunk_size - 1);
        CD_STREAM_STATE.wrapOverflow = 0;
        return (s32*)0x801DC000;
    }

    state = (CdStreamState*)0x1F800000;
    if (!state->dataReady)
    {
        unconsumed = state->bytesBuffered - CD_STREAM_STATE.bytesConsumed;
        bytes_consumed = CD_STREAM_STATE.bytesConsumed;
        wrap_overflow = state->wrapOverflow;
        align_pad = (4 - (unconsumed & 3)) & 3;
        if (wrap_overflow != 0)
        {
            read_ptr = state->readPtr;
            dst = (u8*)(0x801DC118 - unconsumed);
            state->writePtr = (s32)dst;
            state->readPtr = (s32)dst;
            dst -= align_pad;
            aligned_bytes = unconsumed + 3;
            CD_STREAM_STATE.bytesBuffered = (wrap_overflow + unconsumed) + chunk_size;
            src = (u8*)((read_ptr + bytes_consumed) - align_pad);
            if (aligned_bytes < 0)
            {
                aligned_bytes = unconsumed + 6;
            }
            word_count = aligned_bytes >> 2;
            word_count = word_count - 1;
            if (word_count != -1)
            {
                do
                {
                    *((s32*)dst) = *((s32*)src);
                    src += 4;
                    word_count -= 1;
                    dst += 4;
                } while (word_count != -1);
            }
            old_wrap_overflow = CD_STREAM_STATE.wrapOverflow;
            CD_STREAM_STATE.wrapOverflow = 0U;
            dst = dst + old_wrap_overflow;
        }
        else
        {
            dst = (u8*)0x801DC000;
            aligned_bytes_b = unconsumed + 3;
            read_ptr_b = (CD_STREAM_STATE.bytesBuffered = unconsumed + chunk_size);
            read_ptr_b = state->readPtr;
            aligned_write_base = align_pad + 0x801DC000;
            state->writePtr = aligned_write_base;
            state->readPtr = aligned_write_base;
            src_b = (u8*)((read_ptr_b + bytes_consumed) - align_pad);
            if (aligned_bytes_b < 0)
            {
                aligned_bytes_b = unconsumed + 6;
            }
            word_count_b = aligned_bytes_b >> 2;
            word_count_b = word_count_b - 1;
            if (word_count_b != -1)
            {
                do
                {
                    *((s32*)dst) = *((s32*)src_b);
                    src_b += 4;
                    word_count_b -= 1;
                    dst += 4;
                } while (word_count_b != (-1));
            }
        }
        (*((volatile CdStreamState*)(0x1F800000))).dataReady = 1U;
        return (s32*)dst;
    }

    read_ptr_c = state->readPtr;
    bytes_buffered = state->bytesBuffered;
    bytes_transferred = CD_STREAM_STATE.wrapOverflow;
    dst = (u8*)(read_ptr_c + bytes_buffered);
    if ((bytes_transferred != 0) || (((u32)(dst + chunk_size)) > 0x801DE000U))
    {
        dst = (u8*)(bytes_transferred + 0x801DC118);
        if (((u32)state->writePtr) >= ((u32)(dst + chunk_size)))
        {
            CD_STREAM_STATE.wrapOverflow = bytes_transferred + chunk_size;
        }
        else
        {
            CD_STREAM_STATE.dropped_sectors += 1;
            return (void*)0;
        }
    }
    else
    {
        unconsumed = bytes_buffered;
        state->bytesBuffered = unconsumed + chunk_size;
    }

    result = (s32*)dst;
    if (bytes_remaining == chunk_size)
    {
        flag_state = (CdStreamState*)0x1F800000;
        flag_state->bufferWrapped = 1;
        result = (s32*)dst;
        return result;
    }
    return result;
}

/**
 * @brief Decompresses a run-length encoded block from srcStart into dstStart.
 *
 * Skips the first source byte (header), then repeatedly calls cdrom_decompress_data
 * until the stream is exhausted. Bounds are set to the maximum address so no
 * output clamping occurs.
 *
 * @param srcStart  Pointer to the start of the compressed source data.
 * @param dstStart  Pointer to the destination buffer for decompressed output.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/JFLMN
 */
void cdrom_decompress_buffer(u8* srcStart, u8* dstStart)
{
    srcStart++;
    while (cdrom_decompress_data(&srcStart, &dstStart, (u8*)-4U, (u8*)-4U) != 0);
}

/**
 * @brief Writes 0 to a volatile byte, preventing the compiler from eliding the write.
 *
 * @param dataReady  Pointer to the flag to clear (always &CdStreamState.dataReady).
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/Y4pUH
 */
void cdrom_clear_data_ready(s8* dataReady)
{
    volatile s8* ref = dataReady;
    *ref = 0;
}
