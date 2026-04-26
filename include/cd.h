#ifndef _CD_H
#define _CD_H

#include "common.h"
#include "psyq/libcd.h"

#define CD_RESOURCE_INDEX_INVALID 0xFFFE
#define CD_RESOURCE_INDEX_DEFAULT 0xFFFF
#define CD_COMMAND_QUEUE_SIZE 16

// Structures
typedef u32* (*CdCommandCallback)(s32 param_1, u32 param_2);
typedef void (*DecDCToutCallbackHandler)();
typedef void (*DrawSyncCallbackHandler)();
typedef u8* (*codeA)(int, int *);
typedef void (*codeB)(int);

typedef union {
    CdlLOC pos;
    u32 raw;
} CdlLOCRaw;

typedef struct CdResourceEntry {
    CdlLOCRaw location;
    int dataSize;
} CdResourceEntry;

typedef struct CdCommandQueueItem {
    u_char command;
    u_char padding;
    unsigned short resourceIndex;
    CdResourceEntry *entry;
    CdResourceEntry *dstBuffer;
    CdCommandCallback callback;
} CdCommandQueueItem;

typedef struct CdCommandQueue {
    CdCommandQueueItem items[CD_COMMAND_QUEUE_SIZE];
} CdCommandQueue;

typedef union {
    u_int word;
    struct {
        u_char b0;
        u_char b1;
        u_char b2;
        u_char retryExhausted;
    } bytes;
} CdStatusFlags;

typedef struct CdSystem {
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
    CdResourceEntry * dstBuffer;
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

typedef struct {
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

typedef struct {
    u8 u_0[0x38];
    DecDCToutCallbackHandler decDCToutCallbackHandler;
    DrawSyncCallbackHandler drawSyncCallbackHandler;
    u8 u_1[82];
    u8 readFlag;
    u8 u2[3];
} AudioSystem;

typedef struct SKCDPOSE_DAT {
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
 * It can be used to manage the flow of commands and ensure that the system does not become overwhelmed with too many pending commands.
 */
extern u8 g_cdPendingQueueCount;
extern CdSystem g_cdSystem;

/**
 * This is the disc validation ID that is read from the disc during the disc validation process. 
 * It is used to verify that the correct disc is inserted and can be used to prevent unauthorized copies of the game from being played.
 */
extern const u_char g_DiscValidationId[21];
extern u8 D_801ED590;

#define CD_SYSTEM (*(struct CdSystem*)0x801ED800)
#define CD_SYSTEM_V (*(volatile CdSystem*) 0x801ED800)
#define AUDIO_SYSTEM (*(AudioSystem*)0x801ED500)
#define CD_SECTOR_HEADER_BUFFER (*(u32*)0x801ED940)
#define CD_COMMAND_PARAM_BUFFER ((u_char*) 0x801ED958)
#define g_defaultCdResource (*(CdResourceEntry*) 0x801ED990)
#define CD_RESOURCE_ENTRIES ((CdResourceEntry*)0x801ED998)
#define g_commandQueueOffset (*(CdCommandQueueItem*) 0x801ED8F0)
#define SCRATCHPAD ((void*)0x1F800000)
#define CD_STREAM_STATE (*(CdStreamState*)0x1F800000)

// Macros
#define CdControlF_1(cmd) ((int (*)(u_char))CdControlF)(cmd)

// Raw queue item access macros (required for asm matching)
// Equivalent to accessing CD_SYSTEM.commandQueue.items[idx] but generates matching code
#define QUEUE_ITEM_BASE(idx)        ((void*)(((idx) * 0x10) + (u8*)&CD_SYSTEM))
#define QUEUE_ITEM_DST_BUFFER(ptr)  (*((u32*)(ptr) + 0x12))
#define QUEUE_ITEM_CALLBACK(ptr)    (*((CdCommandCallback*)(ptr) + 0x13))

#define CD_INIT_STATE_ERROR_PAUSE 0x20

// Prototypes

/**
 * @brief Cold-start initialization of the CD-ROM subsystem
 *
 * Performs complete hardware and software initialization of the PlayStation's
 * CD-ROM drive. This function must be called before any other CD operations.
 *
 * @details
 * Spin-waits on CdInit() until the hardware is ready, then performs the
 * following initialization steps:
 *
 * 1. Saves and clears previous sync/ready callbacks
 * 2. Resets all CdSystem state (flags, counters, queue indices, command state)
 * 3. Clears statusFlags bits 0-6 individually (preserves only bit 7)
 * 4. Zeros all 16 command queue entries, defaulting buffers to scratchpad RAM
 * 5. Sets CD mode to CdlModeSpeed | CdlModeSize1 (double speed + 2340-byte sectors)
 * 6. Polls CdlNop to read current drive status
 * 7. If shell is open, blocks until disc becomes ready
 * 8. Applies mode via CdlSetmode and records VSync timestamp
 *
 * @note
 * - The per-bit status flag clearing (0x01 through 0x40, out of order) matches
 *   the original assembly's individual AND instructions exactly for 100% matching
 * - g_commandQueueOffset points to items[11]; the loop uses queueItem[4] to walk
 *   through all 16 entries via negative indexing
 * - Scratchpad RAM at 0x1F800000 is used as default buffer for queue entries
 * - Spin-waits on CdInit() and CdControlB() ensure hardware is ready before proceeding
 *
 * @warning
 * - This function blocks until the CD hardware is initialized
 * - If the disc tray is open, it will block until a disc is inserted and ready
 * - Should only be called once during system startup
 *
 * @param None
 * @return void
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/DBYkw
 */
void cdrom_init(void);


/**
 * @brief Stops all CD-ROM operations and resets the subsystem state
 *
 * Gracefully halts CD-ROM playback and prepares the system for either
 * shutdown or new operations. Pauses the drive and clears all internal state.
 *
 * @details
 * Performs a complete stop of the CD-ROM subsystem with the following steps:
 *
 * 1. If audio is enabled, performs a full system reset
 * 2. Clears the "playing" status flag (bit 6)
 * 3. Removes all sync and ready callbacks
 * 4. Repeatedly sends pause commands until successful
 * 5. Resets all CdSystem state variables to their default values
 * 6. Updates timestamp and clears status flags
 * 7. Flushes the CD command queue
 *
 * The function ensures the CD drive is properly paused before clearing
 * internal state to prevent any unexpected behavior.
 *
 * @note
 * - The status flag clearing (0xFFFFFFBF) specifically targets bit 6 (playing flag)
 *   while preserving all other bits
 * - The while(TRUE) loop with CdControlB ensures the pause command is
 *   successfully received by the CD hardware
 * - All state variables are explicitly zeroed to ensure clean state
 * - VSync timestamp is recorded at the end of the operation for timeout tracking
 *
 * @warning
 * - This function blocks until the CD drive acknowledges the pause command
 * - Any pending CD operations will be aborted
 * - Callbacks are cleared, so any pending operations relying on them will be lost
 * - Should not be called from within a CD callback to avoid deadlock
 *
 * @param None
 * @return void
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/M39vT
 */
void cdrom_stop(void);

/**
 * @brief Streams and decompresses CD-ROM sector data into a destination buffer
 *
 * Reads sectors from disc via DMA into a ring buffer (managed through
 * scratchpad RAM), then incrementally decompresses the buffered data
 * into the caller's destination address.
 *
 * @details
 * Scratchpad RAM (0x1F800000) is used as a shared communication struct
 * between this function and the CD read callback (CD_StreamDataCallback):
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
 * The ring buffer ends at 0x801DC118. When it wraps, leftover unprocessed
 * bytes are relocated to just before that address with word alignment,
 * and the wrapOverflow count is merged into bytesBuffered.
 *
 * @param command      Resource index (lower 16 bits) identifying the disc data to read
 * @param destination  RAM address where decompressed output is written
 *
 * @return Total number of decompressed bytes written to destination
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/SvWOg
 */
s32 cdrom_stream(s32 command, u32 destination);


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
 *                  data is first decompressed into an intermediate staging buffer
 *                  at 0x801DA000, then copied out into caller-supplied chunks.
 *                  Each time a chunk is fully filled, pfnChunkDone is called and
 *                  pfnGetBuffer is called again for the next chunk.
 *
 * STAGING BUFFER (chunked mode only):
 *   0x801DA000 — stagingWritePtr starts here; decompressor writes fresh output
 *   0x801DBBE8 — stagingEnd; decompressor stops when it reaches this address
 *
 *   When the staging buffer fills before the stream ends, the last 4096 bytes
 *   of output (the LZ sliding-window dictionary) are copied back to 0x801DA000
 *   and decompression resumes at 0x801DB000. This preserves back-reference
 *   validity across staging-buffer resets.
 *
 * @param resourceIndex   CD resource index (lower 16 bits) passed to cdrom_queue_command.
 * @param pfnGetBuffer    Callback: u8* fn(int totalBytesDelivered, int* outChunkSize)
 *                          Returns a pointer to the next destination buffer.
 *                          Sets *outChunkSize to that buffer's capacity, or -1 for unlimited.
 *                          Called at startup (totalBytesDelivered=0) and after each completed chunk.
 * @param pfnChunkDone    Callback: void fn(int chunkIndex)
 *                          Called when each destination chunk is completely filled, and
 *                          once more at end-of-stream for the final (possibly partial) chunk.
 *
 * TODO: Confirm why dstEnd is set 0x418 (1048) bytes before chunk end in chunked mode —
 *       is this a safety guard to prevent overrun during a partial sector flush?
 * TODO: Determine whether pfnGetBuffer's totalBytesDelivered argument is used as a byte
 *       offset into an asset/resource table by any caller.
 * TODO: Verify whether pfnChunkDone's chunkIndex is ever used by callers or always ignored.
 *
 * @see decomp.me: (93.03%) https://decomp.me/scratch/4WZBs
 */
void cdrom_stream_chunked(undefined2 param_1, codeA param_2, codeB param_3);

/**
 * @brief Enqueues a CD-ROM command into the circular command queue
 *
 * Validates and inserts a command into the 16-entry circular command queue.
 * If the system is idle and no error/init flags are active, immediately
 * starts command execution by issuing CdlNop to kick off the state machine.
 *
 * @details
 * The function performs several layers of validation before enqueueing:
 *
 * 1. Rejects the command immediately if the "playing" status flag (bit 6) is set
 * 2. Resolves the resource index to a CdResourceEntry pointer:
 *    - 0xFFFF (CD_RESOURCE_INDEX_DEFAULT) maps to g_defaultCdResource
 *    - All other indices index into CD_RESOURCE_ENTRIES
 * 3. Deduplicates: if the system is already processing a command and the
 *    new command matches the last-enqueued (command, resourceIndex, dstBuffer,
 *    callback), the enqueue is skipped and the existing dataSize is returned
 * 4. Validates the resource entry has a non-zero disc location and data size
 * 5. Checks the circular queue is not full ((writeIndex + 1) & 0xF != readIndex)
 *
 * Once enqueued, if no command is currently active and no low-nibble status
 * flags (bits 0-3) are set, the function bootstraps execution:
 * - Sets currentCommand to 1, marks the "busy" flag (bit 4)
 * - Installs cdrom_complete_command and sends CdlNop to begin processing
 *
 * @param command        CD-ROM command byte (e.g., CdlReadN, CdlSeekL)
 * @param resourceIndex  Index into CD_RESOURCE_ENTRIES, or 0xFFFF for the default resource
 * @param dstBuffer      Destination buffer for read data (may be NULL for non-read commands)
 * @param callback       Callback function pointer invoked on command completion
 *
 * @return The resource's dataSize on success, or a negative error code:
 *         -3 if the system is in "playing" state (bit 6 set)
 *         -2 if the resource entry has no valid location or zero data size
 *         -1 if the command queue is full
 *
 * @note
 * - The separate re-reads of queueWriteIndex for each field store match the
 *   original assembly's volatile access pattern and must not be optimized
 * - The "last command" cache (lastCommand, resourceIndex, dstBuffer, callback)
 *   enables the deduplication check on subsequent calls
 * - When the system is already busy (currentCommand or initCommand != 0),
 *   the command is silently queued without starting execution
 *
 * @warning
 * - Not interrupt-safe; must not be called from within a CD callback
 * - The caller must ensure resourceIndex is valid or 0xFFFF
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/izXP3
 */
s32 cdrom_queue_command(u8 command, u16 resourceIndex, void* dstBuffer, CdCommandCallback callback);


/**
 * @brief Drains the CD command queue and drives the disc-recovery state machine
 *
 * Called once per frame to advance all pending CD-ROM operations. Handles
 * three mutually exclusive execution paths depending on the current state
 * of the CD subsystem, and updates the audio system when enabled.
 *
 * @details
 * The function inspects statusFlags to choose one of three branches:
 *
 * **Branch 1 — Error/init recovery (statusFlags bits 0-2 set):**
 * Runs a multi-state recovery state machine (states 1-8, 32) that attempts
 * to re-initialize the disc drive after an error or shell-open event:
 *   1. Sends CdlNop to poll drive status every 30 VSync frames
 *   2. Progresses through GetStat, DiskReady, DiskType detection
 *   3. Re-applies CdlSetmode (0xA0) and installs sync/ready callbacks
 *   4. Issues CdlReadN to resume reading, with 270-frame timeout retries
 *   5. On persistent errors, pauses the drive and resets to state 1
 *
 * **Branch 2 — Active command execution (currentCommand or initCommand != 0):**
 * Monitors the currently executing command for completion:
 *   - Polls syncComplete flag set by the sync callback
 *   - Updates currentResourceIndex and currentDataSize from the queue head
 *   - On 240-frame timeout, re-installs callbacks and retries via CdlNop
 *   - Records VSync timestamp and remaining queue depth each frame
 *
 * **Branch 3 — Idle with queued commands (queue non-empty, no active command):**
 * Bootstraps execution of the next queued command:
 *   - Sets currentCommand to 1, marks busy flag (bit 4)
 *   - Installs cdrom_complete_command and sends CdlNop to start processing
 *   - If the queue is empty, performs periodic 30-frame status polls via CdlNop
 *     and triggers CD_HandleSyncError if the drive reports an error (bit 4)
 *
 * After all branches, calls FUN_80140d48() to update the audio subsystem
 * when g_cdAudioEnabled is set.
 *
 * @param None
 *
 * @return The number of commands remaining in the queue (0 when idle or
 *         when the system is in the recovery state machine with bit 3 set)
 *
 * @note
 * - Returns 0 immediately if statusFlags bit 3 is set (processing deferred
 *   to cdrom_recover)
 * - Raw pointer arithmetic for queue item access is preserved from the
 *   original decompilation to maintain register-level matching
 * - The recovery state machine shares state numbers (initState) and command
 *   codes (initCommand 0x20-0x23) with cdrom_recover but
 *   operates on a different set of transitions
 *
 * @warning
 * - Must be called every frame for correct timeout and retry behavior
 * - Not interrupt-safe; must not be called from within a CD callback
 * - The 30/240/270-frame timeout constants assume NTSC (60 Hz) VSync rate
 *
 * @see decomp.me: (96.81%) https://decomp.me/scratch/Jfb6t
 */
u_int cdrom_process_state(void);



/**
 * @brief Processes the CD-ROM recovery/init state machine across multiple VSync frames
 *
 * Drives a 4-state asynchronous state machine that reconfigures the CD-ROM
 * subsystem after an error or shell-open event. Each call advances at most
 * one state transition, allowing the caller to poll once per frame.
 *
 * @details
 * The state machine (stored in CD_SYSTEM.initState) progresses as follows:
 *
 * - **State 0 — Flush:** Calls CdFlush() to discard pending commands, then
 *   advances to state 1 with a 1-frame delay.
 *
 * - **State 1 — Set mode:** Waits for the delay to expire, then configures
 *   CD mode to 0xA0 (CdlModeSpeed | CdlModeSize1), installs
 *   CD_SyncCallback_Handler, sends CdlSetmode, and waits 4 frames.
 *   Returns 0 (still waiting) if the delay has not yet elapsed.
 *
 * - **State 2 — Set filter:** Installs sync callback, sends CdlSetfilter
 *   with file=1 channel=1, sets initCommand to 0x11 (pending demute),
 *   and advances to state 3 immediately.
 *
 * - **State 3 — Wait for sync / dispatch:** Waits for either the
 *   syncComplete flag or a 30-frame timeout, then dispatches based on
 *   initCommand:
 *     - 0x10: Re-sends CdlSetfilter (retry/default)
 *     - 0x11: Sends CdlDemute to unmute CD audio
 *     - 0x12: Sends CdlPause (command 0x09) to halt the drive
 *   After CdlSetfilter, resets initCommand to 0x10.
 *   Subtracts 30 from vsyncTimestamp to allow immediate re-entry on the
 *   next timeout cycle rather than resetting the timer.
 *
 * @param None
 *
 * @return 1 if the CD subsystem is not in recovery mode (statusFlags bit 3 clear),
 *         0 while the state machine is still processing
 *
 * @note
 * - initCommand acts as a sub-state within state 3 to sequence multiple
 *   CD commands (setfilter -> demute -> pause) across successive timeouts
 * - The filterParams buffer is written byte-by-byte to match the original
 *   assembly's sb instructions for register-level matching
 * - State 1 returns 0 early (not via the common exit) when the timestamp
 *   delay has not yet expired, matching a distinct return instruction in
 *   the original binary
 *
 * @warning
 * - Must be called every frame for correct timeout behavior
 * - The 1/4/30-frame delay constants assume NTSC (60 Hz) VSync rate
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/IvxZG
 */
int cdrom_recover(void);


 /**
 * @brief Verifies the next CD sector header during error recovery.
 *
 * This function is invoked when the CD-ROM drive signals readiness while the
 * system is in a recovery state (e.g., after a timeout or command failure).
 * It checks the current audio mode, reads the next sector's header, and either
 * completes the sector read, retries the current command, or falls back to a
 * safe NOP command after exhausting retries.
 *
 * @details
 * The function relies on the global flag `g_cdStatusByte3` being set to 1
 * before it is called; otherwise it returns immediately.
 *
 * The logic branches based on whether audio output is enabled:
 *
 * - **Audio disabled (`audioEnabled != 1`):**
 *   1. Waits for the sector header (3 words / 12 bytes) to be read into
 *      `sectorHeaderBuffer`.
 *   2. Compares the lower 24 bits of the header against the expected
 *      `currentLocation.raw` disc position.
 *   3. If they match → calls `cdrom_process_sector(1)` to finish the
 *      transfer.
 *   4. If they mismatch → increments `retryCount` and re‑issues the current
 *      command (up to 16 retries).
 *   5. After 16 failures → marks `retryExhausted`, resets the retry counter,
 *      sets `playbackState` based on `transferCallback`, issues a CdlNop
 *      (command 1), and clears the recovery flag.
 *
 * - **Audio enabled (`audioEnabled == 1`):**
 *   Assumes the sector is correct and immediately calls
 *   `cdrom_process_sector(1)`.
 *
 * Finally, the function clears `g_cdStatusByte3` to 0 to indicate that recovery
 * verification has been handled.
 *
 * @return void
 *
 * @note This function is intended to be installed as a callback during recovery,
 *       typically after `cdrom_recover()` enters a waiting state and the drive
 *       responds with a "ready" interrupt.
 *
 * @warning The function spins on `CdGetSector()` until the sector header is
 *          available; this may block execution in certain contexts.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/iWEyM
 */
void cdrom_verify_recovery(void);

/**
 * @brief Sync callback invoked when a CD-ROM command completes or fails
 *
 * Installed as the CdSyncCallback during normal command queue processing.
 * Handles command completion by advancing the circular queue, dispatching
 * the next queued command, or cleaning up when the queue is drained.
 *
 * @details
 * On entry, sets syncComplete to 1 so the main-loop poller knows progress
 * was made. Then branches based on the interrupt status:
 *
 * **Error path (status byte bit 4 set during CdlNop):**
 * If currentCommand is 1 (CdlNop probe) and the drive reports an error,
 * calls CD_HandleSyncError() and returns immediately.
 *
 * **Incomplete path (status != CdlComplete):**
 * If the command did not finish successfully:
 *   - If currentCommand != 1, resets to CdlNop and retries
 *   - Otherwise falls through to re-read the queue head and execute it
 *
 * **Complete path (status == CdlComplete):**
 * Switches on currentCommand:
 *   - **Case 21 (CdlPause):** Resets playback state and loop counter,
 *     advances the queue read index. If the queue is now empty, clears
 *     all execution state and returns. Otherwise dispatches the next command.
 *   - **All other cases (default):** Reads the command at the queue head.
 *     If it is CdlNop (1), skips forward through consecutive CdlNop entries
 *     until a different command is found or the queue is exhausted.
 *     Then dispatches the resolved command via CD_ExecuteCommand.
 *
 * Special handling: if the resolved command is 0x1B (audio start), enables
 * CD_SYSTEM.audioEnabled and remaps the command to CdlSeekL (6) for execution.
 *
 * @param status    CD-ROM interrupt status byte (CdlComplete on success)
 * @param resultPtr Pointer to the CD-ROM result byte array from the hardware
 *
 * @return void
 *
 * @note
 * - This function runs in interrupt context as a CdSyncCallback
 * - The volatile vsyncArg variable and separate statusFlags assignment
 *   match the original assembly's register usage and must not be optimized
 * - The large switch with explicit case labels for 1-27 (excluding 6, 21)
 *   matches the original jump table layout in the binary
 *
 * @warning
 * - Executes in interrupt context; must not call blocking functions
 * - Modifies CD_SYSTEM state directly; not safe to call from main thread
 *
 * @see decomp.me: (91.68%) https://decomp.me/scratch/F0oiy
 */
void cdrom_complete_command(u_char intr, u_char *result);

void CD_SyncCallback_Handler(u_char intr, u_char* result);

 /**
 * @brief Low-level ready callback invoked when the CD-ROM drive signals a sector is ready.
 *
 * This function is installed as the CdReadyCallback. It handles the transition from 
 * the hardware signaling "ready" to the software processing the sector data. 
 * It distinguishes between standard data reads and audio (XA) streaming.
 *
 * @details
 * The handler operates in two primary modes:
 * 
 * **Data Mode (audioEnabled != 1):**
 * 1. Checks if the interrupt status matches the expected state.
 * 2. If a mismatch or error occurs, it attempts to read the sector header to verify 
 *    the current disc position.
 * 3. If the position is correct, it hands off to `cdrom_process_sector`.
 * 4. If the read fails, it implements a retry mechanism (up to 17 attempts). 
 *    On failure, it marks the system as `retryExhausted` and issues a `CdlNop` 
 *    to reset the drive state.
 *
 * **Audio Mode (audioEnabled == 1):**
 * 1. Verifies if the interrupt status matches the audio state.
 * 2. Checks a specific hardware flag (at 0x801ED59C) to determine if the 
 *    sector should be processed immediately or if the status should be recorded.
 * 3. On success, invokes `cdrom_process_sector`.
 * 4. Implements a similar retry mechanism to Data Mode if the audio read fails.
 *
 * @param intr   Completion code from the CD-ROM drive.
 * @param result Pointer to the drive's status byte/result.
 *
 * @note This function runs in interrupt context and should not call blocking functions.
 * @see decomp.me: (100%) https://decomp.me/scratch/kgBY4
 */
void cdrom_handle_ready_intr(u_char intr, u_char *result);


/**
 * @brief Handles completion of a CD-ROM sector read operation
 *
 * Called when the CD drive signals that a sector has been read into memory.
 * Processes the received data differently depending on whether the system
 * is in data mode or audio (XA) mode, and manages multi-sector transfers
 * by re-issuing read commands until all data has been received.
 *
 * @details
 * The function operates in two distinct modes based on audioEnabled:
 *
 * **Data mode (audioEnabled != 1):**
 * 1. Invokes the transferCallback callback (if set) to obtain the destination
 *    buffer; if the callback returns NULL, re-issues the current read
 *    command to retry. Falls back to currentWritePtr when no callback is set.
 * 2. If more than one sector remains (size >= 0x801):
 *    - Reads one full sector (0x800 bytes / 0x200 words) via CdGetSector
 *    - Advances the disc position by one sector in the command param buffer
 *    - Decrements remaining size by 0x800
 *    - Advances currentWritePtr by 0x800 if no transferCallback callback is set
 * 3. If this is the final sector (size < 0x801):
 *    - Resets playbackState and transferCallback
 *    - Advances queueReadIndex; if more commands are queued, dispatches
 *      the next one via CD_ExecuteCommand and returns
 *    - Otherwise, transitions to idle: installs sync callback, removes
 *      ready callback, reads the final partial sector, clears busy flag
 *      (bit 4), and issues CdlPause
 *    - The pause command timing depends on arg0: issued before the final
 *      read when arg0 == 0, or after when arg0 != 0
 *
 * **Audio mode (audioEnabled == 1):**
 * 1. Reads 3 words (12 bytes) from the sector into sectorHeaderBuffer
 * 2. Compares the lower 24 bits of sectorHeaderBuffer[0] against currentLocation
 *    to verify the correct disc position; if mismatched, re-issues the
 *    current command with the expected position parameters
 * 3. If positions match, invokes the transferCallback:
 *    - If callback returns NULL (end of audio track): advances the queue,
 *      resets mode to 0xA0, disables audio, pauses the drive, and records
 *      the VSync timestamp
 *    - If callback returns non-NULL: advances disc position by one sector
 *      and returns to continue streaming
 *
 * @param arg0  Execution mode passed from the caller:
 *              0 = initial call from the ready callback (pause before final read)
 *              non-zero = chained call from CD_ExecuteCommand (pause after final read)
 *
 * @return void
 *
 * @note
 * - 0x801ED958 is used as the command parameter buffer holding the current
 *   CdlLOC disc position for read commands
 * - The 0xFFFFFF mask in audio mode extracts the minute/second/sector BCD
 *   position, ignoring the mode byte
 * - g_cdReadRemainingBytes is used for the final partial sector read, converted from bytes
 *   to words via (g_cdReadRemainingBytes + 3) >> 2
 *
 * @warning
 * - Spin-waits on CdGetSector until the sector data is available
 * - Must only be called from the CD ready callback context
 * - The transferCallback must be valid (non-NULL) in audio mode
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/43gwj
 */
void cdrom_process_sector(s32 arg0);

void CD_HandleSyncError(void);
void CD_SetAudioVolume(u_char volume, int stereoChannel);
void CD_InitResources(int lba, int dataSizeBytes);







s32 CD_DecompressData(u8** srcStart, u8** dstStart, u8* srcEnd, u8* dstEnd);
void ClearPointer(s8* arg0);
s32* CD_StreamDataCallback(s32 param_1, u32 param_2);

void CD_ExecuteCommand(u8 command, void* sectorBuffer, s32 executionMode);
void CD_ResetSystem(void);
void CD_DiskValidationCallback(u_char intr, u_char *result);
void FUN_80022400(u_int param_1);
undefined FUN_80140d48(void);

void FUN_80023010(void);




void func_80022AE8(undefined4 param_1,undefined4 param_2);
s32 func_80022040(u8 *param_1);
void FUN_8002279c(undefined4 param_1,u_int param_2);
void CD_WaitForQueueEmpty(void);
void func_800227D0(u32 param_1, u32 param_2, u32 param_3);
void CD_QueueRead(s32 arg0, void* arg1);

#endif