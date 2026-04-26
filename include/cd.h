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
 * @param resourceIndex   CD resource index (lower 16 bits) passed to CD_QueueCommand.
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

void CD_HandleSyncError(void);
void CD_SetAudioVolume(u_char volume, int stereoChannel);
void CD_InitResources(int lba, int dataSizeBytes);
u_int CD_UpdateAndProcessQueue(void);
s32 CD_QueueCommand(u8 command, u16 resourceIndex, void* dstBuffer, CdCommandCallback callback);
void CD_SyncCallback_Handler(u_char intr, u_char* result);
void CD_OnCommandComplete(u_char intr, u_char *result);
s32 CD_DecompressData(u8** srcStart, u8** dstStart, u8* srcEnd, u8* dstEnd);
void ClearPointer(s8* arg0);
s32* CD_StreamDataCallback(s32 param_1, u32 param_2);
void CD_ReadyHandler(u_char intr, u_char *result);
void CD_ExecuteCommand(u8 command, void* sectorBuffer, s32 executionMode);
void CD_ResetSystem(void);
void CD_DiskValidationCallback(u_char intr, u_char *result);
void FUN_80022400(u_int param_1);
undefined FUN_80140d48(void);
int CD_RecoveryStateMachine(void);
void FUN_80023010(void);
void CD_HandleSectorReadComplete(s32 arg0);
void CD_RecoveryReadyHandler(void);
void func_80022AE8(undefined4 param_1,undefined4 param_2);
s32 func_80022040(u8 *param_1);
void FUN_8002279c(undefined4 param_1,u_int param_2);
void CD_WaitForQueueEmpty(void);
void func_800227D0(u32 param_1, u32 param_2, u32 param_3);
void CD_QueueRead(s32 arg0, void* arg1);

#endif