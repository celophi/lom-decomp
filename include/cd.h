#ifndef _CD_H
#define _CD_H

#include "common.h"
#include "akao.h"
#include "psyq/libcd.h"

#define CD_RESOURCE_INDEX_INVALID 0xFFFE
#define CD_RESOURCE_INDEX_DEFAULT 0xFFFF
#define CD_COMMAND_QUEUE_SIZE 16

// Structures

/**
 * Invoked per sector during a read. Receives bytes-delivered and bytes-remaining;
 * returns the destination buffer address for the incoming sector.
 */
typedef u32* (*CdCommandCallback)(s32 param_1, u32 param_2);

/**
 * Saved DecDCT output callback stored in AudioSystem and restored by cdrom_reset
 * via DecDCToutCallback().
 */
typedef void (*DecDCToutCallbackHandler)();

/**
 * Saved DrawSync callback stored in AudioSystem and restored by cdrom_reset
 * via DrawSyncCallback().
 */
typedef void (*DrawSyncCallbackHandler)();

/**
 * cdrom_stream_chunked get-buffer callback.
 * Returns the next destination chunk and sets its byte capacity via the out-param
 * (−1 = unlimited / direct mode).
 */
typedef u8* (*codeA)(int, int*);

/**
 * cdrom_stream_chunked chunk-done callback.
 * Invoked with the chunk index each time a chunk is fully filled and once at end-of-stream.
 */
typedef void (*codeB)(int);

/**
 * Disc position that can be accessed as a structured CdlLOC (minute/second/frame)
 * or as a raw 32-bit word for fast comparison.
 */
typedef union
{
    CdlLOC pos;
    u32 raw;
} CdlLOCRaw;

/**
 * Identifies one disc resource: where it starts on disc and how many bytes it contains.
 */
typedef struct CdResourceEntry
{
    CdlLOCRaw location;
    int dataSize;
} CdResourceEntry;

/**
 * One entry in the 16-slot circular CD command queue.
 */
typedef struct CdCommandQueueItem
{
    u_char command;
    u_char padding;
    unsigned short resourceIndex;
    CdResourceEntry* entry;
    CdResourceEntry* dstBuffer;
    CdCommandCallback callback;
} CdCommandQueueItem;

/**
 * Circular buffer holding up to CD_COMMAND_QUEUE_SIZE pending CD commands.
 */
typedef struct CdCommandQueue
{
    CdCommandQueueItem items[CD_COMMAND_QUEUE_SIZE];
} CdCommandQueue;

/**
 * CD subsystem status word. b0 bits 0-2 are error flags; bit 4 is the busy flag;
 * retryExhausted is set when the sector-read retry limit is exhausted.
 */
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

/**
 * Central state block for the CD subsystem, mapped to 0x801ED800.
 */
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

/**
 * Ring buffer control block for streaming CD data, resident in scratchpad RAM (0x1F800000).
 * Shared between cdrom_stream / cdrom_stream_chunked and cdrom_handle_stream_data.
 */
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
    s32 reserved;
} CdStreamState;

/**
 * Audio subsystem state block, mapped to 0x801ED500.
 * Stores the DecDCT and DrawSync callbacks that were active before XA audio playback
 * began, so cdrom_reset can restore them.
 */
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

// Externs
extern CdlCB g_cdSyncCallbackResult;
extern CdlCB g_cdReadyCallbackResult;
extern int g_cdVSyncTimestamp;
extern u_char g_cdStatusByte;
extern u_char g_cdAudioEnabled;
extern u_char g_cdAudioReady;
extern u8 g_playbackState;

/**
 * Tracks the remaining byte count during multi-sector CD read operations.
 * Starts from the resource's dataSize and decrements by 0x800 (2048) for each sector read until it reaches zero.
 */
extern u32 g_cdReadRemainingBytes;
extern u8 g_cdAudioReady;
extern s32 g_cdResource176;
extern s8 g_cdStatusByte3;
extern u8 g_initState;
extern s8 D_801ED801;

/**
 * This is a flag that indicates the number of pending commands in the CD command queue.
 * It is used to track how many commands are waiting to be processed.
 * It can be used to manage the flow of commands and ensure that the system does not become overwhelmed with too many
 * pending commands.
 */
extern u8 g_cdPendingQueueCount;
extern CdSystem g_cdSystem;

/**
 * This is the disc validation ID that is read from the disc during the disc validation process.
 * It is used to verify that the correct disc is inserted and can be used to prevent unauthorized copies of the game
 * from being played.
 */
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

// Macros
#define CdControlF_1(cmd) ((int (*)(u_char))CdControlF)(cmd)

// Raw queue item access macros (required for asm matching)
// Equivalent to accessing CD_SYSTEM.commandQueue.items[idx] but generates matching code
#define QUEUE_ITEM_BASE(idx) ((void*)(((idx) * 0x10) + (u8*)&CD_SYSTEM))
#define QUEUE_ITEM_DST_BUFFER(ptr) (*((u32*)(ptr) + 0x12))
#define QUEUE_ITEM_CALLBACK(ptr) (*((CdCommandCallback*)(ptr) + 0x13))

#define CD_INIT_STATE_ERROR_PAUSE 0x20

// Prototypes

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
void cdrom_init(void);

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
void cdrom_stop(void);

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
s32 cdrom_stream(s32 resourceIndex, u32 destination);

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
 * @see decomp.me: (93.03%) https://decomp.me/scratch/4WZBs
 */
void cdrom_stream_chunked(undefined2 resourceIndex, codeA pfnGetBuffer, codeB pfnChunkDone);

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
s32 cdrom_queue_command(u8 command, u16 resourceIndex, void* dstBuffer, CdCommandCallback callback);

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
 * **Branch 1 — Error/init recovery (statusFlags bits 0-2 set):**
 * Runs a multi-state recovery state machine (states 1-8, 32):
 *   1. Polls drive status via CdlNop every 30 VSync frames
 *   2. Progresses through GetStat, DiskReady, DiskType detection
 *   3. Re-applies CdlSetmode (0xA0) and installs sync/ready callbacks
 *   4. Issues CdlReadN to resume reading, with 270-frame timeout retries
 *   5. On persistent errors, pauses the drive and resets to state 1
 *
 * **Branch 2 — Active command (currentCommand or initCommand != 0):**
 *   - Polls syncComplete flag set by the sync callback
 *   - Updates currentResourceIndex and currentDataSize from the queue head
 *   - On 240-frame timeout, re-installs callbacks and retries via CdlNop
 *
 * **Branch 3 — Idle with queued commands:**
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
 * @see decomp.me: (96.81%) https://decomp.me/scratch/Jfb6t
 */
u_int cdrom_process_state(void);

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
int cdrom_recover(void);

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
void cdrom_verify_recovery(void);

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
 * @see decomp.me: (91.68%) https://decomp.me/scratch/F0oiy
 */
void cdrom_complete_command(u_char intr, u_char* result);

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
 * @see decomp.me: (73.39%) https://decomp.me/scratch/0Dz2i
 */
void cdrom_handle_recovery_sync(u_char intr, u_char* result);

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
void cdrom_handle_ready_intr(u_char intr, u_char* result);

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
void cdrom_process_sector(s32 arg0);

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
void cdrom_run_command(u8 command, void* sectorBuffer, s32 executionMode);

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
void cdrom_verify_disc(u_char intr, u_char* result);

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
void cdrom_wait_queue_empty(void);

/**
 * @brief Resets CD state and enters error recovery after a sync failure.
 *
 * Clears sync and ready callbacks, sets the error flag (statusFlags bit 0),
 * resets all command and retry counters, clears the busy flag (bit 4),
 * and records the current VSync timestamp.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/lU7lO
 */
void cdrom_handle_sync_error(void);

/**
 * @brief Sets the CD-DA audio mix volume and channel routing.
 *
 * @param volume         Volume level (0–255).
 * @param stereoChannel  0 = route both CD channels to SPU left only,
 *                       1 = route CD left to both speakers (mono).
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/lwzx1
 */
void cdrom_set_audio_volume(u_char volume, int stereoChannel);

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
void cdrom_load_resource_table(int lba, int dataSizeBytes);

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
s32 cdrom_decompress_data(u8** srcStart, u8** dstStart, u8* srcEnd, u8* dstEnd);

/**
 * @brief Resets the CD subsystem and stops any ongoing XA audio playback.
 *
 * Restores DecDCT and DrawSync callbacks, clears CD sync/ready callbacks,
 * pauses the drive, stops CD audio if active, and resets all internal state.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/fnucZ
 */
void cdrom_reset(void);

/**
 * @brief Enqueues a CdlReadN command for the given resource and destination buffer.
 *
 * @param resourceIndex  Index into CD_RESOURCE_ENTRIES identifying the data to read.
 * @param dstBuffer      Destination buffer for the sector data.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/OxunQ
 */
void cdrom_queue_read(s32 resourceIndex, void* dstBuffer);

/**
 * @brief Sets byte 1 of CD_SYSTEM.statusFlags to 1.
 *
 * Writes 1 to D_801ED801 (CdStatusFlags.bytes.b1), signalling a status
 * condition in the CD subsystem. No callers exist in the main binary;
 * this function is invoked from overlay code.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/9bgSH
 */
void func_80014434(void);

/**
 * @brief Transfer callback that manages the ring buffer during sector streaming.
 *
 * Installed as CD_SYSTEM.transferCallback during cdrom_stream and cdrom_stream_chunked.
 * On the first call (arg0 == 0), initializes CdStreamState in scratchpad RAM and
 * returns the ring buffer base address. On subsequent calls, compacts unconsumed
 * bytes and advances the write pointer for the next incoming sector.
 *
 * @param bytesTransferred  Bytes delivered so far; 0 on the first call (initialization), non-zero on each subsequent
 * sector arrival.
 * @param bytesRemaining    Bytes still to read in the stream, passed as readRemainingBytes; clamped to 0x800 per
 * sector.
 *
 * @return Destination address for the next sector DMA write.
 *
 * @see decomp.me: (95.08%) https://decomp.me/scratch/UDwSD
 */
s32* cdrom_handle_stream_data(s32 bytesTransferred, u32 bytesRemaining);

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
void cdrom_decompress_buffer(u8* srcStart, u8* dstStart);

/**
 * @brief Writes 0 to a volatile byte, preventing the compiler from eliding the write.
 *
 * @param dataReady  Pointer to the flag to clear (always &CdStreamState.dataReady).
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/Y4pUH
 */
void cdrom_clear_data_ready(s8* dataReady);

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
s32 cdrom_can_queue_resource(s32 resourceIndex);

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
void cdrom_queue_read_with_callback(s32 resourceIndex, CdCommandCallback callback);

/**
 * @brief Enqueues a CdlSeekL command to pre-position the disc head.
 *
 * @param resourceIndex  Index into CD_RESOURCE_ENTRIES identifying the target position.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/iUUQh
 */
void cdrom_queue_seek(s32 resourceIndex);

/**
 * @brief Returns the data size of a CD resource entry.
 *
 * @param resourceIndex  Index into CD_RESOURCE_ENTRIES (lower 16 bits used).
 *
 * @return The dataSize field of the resource entry in bytes.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/SGZF5
 */
s32 cdrom_get_resource_size(s32 resourceIndex);

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
s32 cdrom_get_error_status(void);

/**
 * @brief Restores previously saved CD callbacks and resets all subsystem state.
 *
 * Reinstalls the sync and ready callbacks that were saved before the last
 * cdrom_init or cdrom_stop call, issues a blocking CdControlB(9) to pause
 * the drive, then clears all queue indices, command state, and status flags.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/HSXMR
 */
void cdrom_restore_callbacks(void);

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
s32 cdrom_enter_recovery_mode(void);

extern void func_800227D0(u32 param_1, u32 param_2, u32 param_3);
extern void FUN_80022400(u_int param_1);
extern undefined FUN_80140d48(void);
extern void FUN_80023010(void);
extern void akao_play_sequence_blocking(AkaoSeqHeader* sequenceData, s32 waitForCompletion);
extern s32 func_80022040(u8* param_1);
extern void FUN_8002279c(undefined4 param_1, u_int param_2);

#endif