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
        u_char b3;
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
    u32 u_168;
    CdlCB previousSyncCallback;
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

// Prototypes
void CD_Initialize(void);
void CD_Stop(void);
s32 CD_StreamData(s32 command, u32 destination);
void CD_HandleSyncError(void);
void CD_SetAudioVolume(u_char volume, int stereoChannel);
void CD_InitResources(int lba, int dataSizeBytes);
u_int CD_UpdateAndProcessQueue(void);
s32 CD_QueueCommand(u8 command, u16 resourceIndex, CdResourceEntry* dstBuffer, CdCommandCallback callback);
void CD_SyncCallback_Handler(u_char intr, u_char* result);
void CD_OnCommandComplete(u_char intr, u_char *result);
s32 CD_DecompressData(u32* srcStart, u32* dstStart, u32 srcEnd, u32 dstEnd);
void ClearPointer(void *pointer);
u32* CD_StreamDataCallback(s32 param_1, u32 param_2);
void CD_ReadyCallback(u_char intr, u_char *result);
void CD_ExecuteCommand(u8 command, void* sectorBuffer, s32 executionMode);
void CD_ResetSystem(void);
void CD_DiskValidationCallback(u_char intr, u_char *result);
void FUN_80022400(u_int param_1);
undefined FUN_80140d48(void);
int CD_RecoveryStateMachine(void);
void FUN_80023010(void);
void CD_HandleSectorReadComplete(s32 arg0);
void CD_StreamDataChunked(undefined2 param_1, codeA param_2, codeB param_3);
void CD_RecoveryReadyHandler(void);
void* func_80016E7C(u_char *param_1,u_char *param_2,int param_3);
void func_80022AE8(undefined4 param_1,undefined4 param_2);
s32 func_80022040(u8 *param_1);
void FUN_8002279c(undefined4 param_1,u_int param_2);
void CD_WaitForQueueEmpty(void);
void func_800227D0(u32 param_1, u32 param_2, u32 param_3);

#endif