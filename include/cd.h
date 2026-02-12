#ifndef _CD_H
#define _CD_H

#include "common.h"
#include "psyq/libcd.h"

// Structures
typedef struct CdResourceEntry {
    CdlLOC Location;
    int dataSize;
} CdResourceEntry;

typedef struct CdStatusArray {
    u_char u0;
    u_char u1;
    u_char u2;
    u_char u3;
} CdStatusArray;

typedef struct CdCommandQueueItem {
    u_char command;
    u_char padding;
    unsigned short resourceIndex;
    CdlLOC *location;
    unsigned int dstBuffer;
    unsigned int callback;
} CdCommandQueueItem;

typedef struct CdCommandQueue {
    CdCommandQueueItem Items[16];
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
    undefined1 playbackFlag;
    undefined u_7;
    undefined2 currentResourceIndex;
    undefined u_a;
    undefined u_b;
    undefined4 currentDataSize;
    undefined4 targetDataSize;
    undefined1 syncComplete;
    undefined1 initState;
    undefined1 currentCommand;
    undefined1 initCommand;
    undefined1 retryCount;
    undefined1 retryCounter;
    undefined1 lastCommand;
    undefined u_1b;
    undefined2 resourceIndex;
    undefined u_1e;
    undefined u_1f;
    undefined4 dstBuffer;
    undefined4 callback;
    u_int size;
    undefined4 sizeCopy;
    undefined4 dstBuffer2;
    undefined4 loopCounter;
    undefined4 queueReadIndex;
    undefined4 queueWriteIndex;
    CdCommandQueue commandQueue;
    undefined4 readSectorBuffer;
    undefined4 u_144;
    undefined4 u_148;
    undefined4 vsyncTimestamp;
    undefined1 setModeBuffer;
    undefined1 u_151;
    undefined1 u_152;
    undefined1 u_153;
    undefined1 modeParams;
    undefined1 u_155;
    undefined1 u_156;
    undefined1 u_157;
    undefined4 commandParamBuffer;
    undefined4 readParams;
    undefined1 statusByte;
    undefined1 filterModeFlags;
    undefined4 u_162;
    undefined2 u_166;
    CdlCB previousSyncCallback;
    CdlCB previousReadyCallback;
    undefined1 u_170;
    undefined1 u_171;
    undefined1 u_172;
    undefined1 u_173;
    undefined4 u_174;
    undefined4 u_178;
    undefined4 u_17c;
    undefined4 u_180;
    undefined4 u_184;
    undefined4 u_188;
    undefined4 u_18c;
    CdResourceEntry defaultCdResource;
} CdSystem;

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

#define g_cdSystem (*(struct CdSystem*)0x801ed800)
#define g_SKCDPOSE_DAT (*(struct SKCDPOSE_DAT*)0x801ed998)
#define g_otherQueue (*(CdCommandQueueItem*)0x801ed8f0)

// Prototypes
void CD_HandleSyncError(void);
void CD_SetAudioVolume(u_char volume, int stereoChannel);
void CD_InitLocationEntries(int lba, int dataSizeBytes);
u_int CD_UpdateAndProcessQueue(void);
int CD_EnqueueCommand(char command, u_short resourceIndex, u_int dstBuffer, u_int callback);
void CD_SyncCallback_Handler(char intr, u_char *status);

#endif