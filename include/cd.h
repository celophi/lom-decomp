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

typedef struct BigCdStruct {
    CdStatusFlags cdStatusFlags;
    undefined1 cdAudioEnabled;
    undefined1 g_cdPlaybackState;
    undefined1 g_cdPlaybackFlag;
    undefined field4_0x7;
    undefined2 g_cdCurrentResourceIndex;
    undefined field6_0xa;
    undefined field7_0xb;
    undefined4 g_cdCurrentDataSize;
    undefined4 g_cdTargetDataSize;
    undefined1 g_cdSyncComplete;
    undefined1 g_cdInitState;
    undefined1 g_cdCurrentCommand;
    undefined1 g_cdInitCommand;
    undefined1 g_cdRetryCount;
    undefined1 g_cdRetryCounter;
    undefined1 g_cdLastCommand;
    undefined field17_0x1b;
    undefined2 g_cdResourceIndex;
    undefined field19_0x1e;
    undefined field20_0x1f;
    undefined4 g_cdDstBuffer;
    undefined4 g_cdCallback;
    unsigned int g_cdSize;
    undefined4 g_cdSizeCopy;
    undefined4 g_cdDstBuffer2;
    undefined4 g_cdLoopCounter;
    undefined4 g_cdQueueReadIndex;
    undefined4 g_cdQueueWriteIndex;
    CdCommandQueue g_CdCommandQueue;
    undefined4 g_cdReadSectorBuffer;
    undefined field31_0x144;
    undefined field32_0x145;
    undefined field33_0x146;
    undefined field34_0x147;
    undefined field35_0x148;
    undefined field36_0x149;
    undefined field37_0x14a;
    undefined field38_0x14b;
    undefined4 cdVSyncTimestamp;
    undefined1 g_cdSetModeBuffer;
    undefined1 field42_0x151;
    undefined1 field43_0x152;
    undefined1 field44_0x153;
    undefined1 g_cdModeParams;
    undefined1 field45_0x155;
    undefined1 field46_0x156;
    undefined1 field47_0x157;
    undefined4 g_cdCommandParamBuffer;
    undefined4 g_cdReadParams;
    undefined1 cdStatusByte;
    undefined1 g_cdFilterModeFlags;
    undefined field52_0x162;
    undefined field53_0x163;
    undefined field54_0x164;
    undefined field55_0x165;
    undefined field56_0x166;
    undefined field57_0x167;
    CdlCB g_cdPreviousSyncCallback;
    CdlCB g_cdPreviousReadyCallback;
    undefined1 field60_0x170;
    undefined1 field61_0x171;
    undefined1 field62_0x172;
    undefined1 field63_0x173;
    undefined field64_0x174;
    undefined field65_0x175;
    undefined field66_0x176;
    undefined field67_0x177;
    undefined field68_0x178;
    undefined field69_0x179;
    undefined field70_0x17a;
    undefined field71_0x17b;
    undefined field72_0x17c;
    undefined field73_0x17d;
    undefined field74_0x17e;
    undefined field75_0x17f;
    undefined field76_0x180;
    undefined field77_0x181;
    undefined field78_0x182;
    undefined field79_0x183;
    undefined field80_0x184;
    undefined field81_0x185;
    undefined field82_0x186;
    undefined field83_0x187;
    undefined field84_0x188;
    undefined field85_0x189;
    undefined field86_0x18a;
    undefined field87_0x18b;
    undefined field88_0x18c;
    undefined field89_0x18d;
    undefined field90_0x18e;
    undefined field91_0x18f;
    CdResourceEntry defaultCdResource;
} BigCdStruct;

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

#define g_bigCdStruct (*(struct BigCdStruct*)0x801ed800)
#define g_SKCDPOSE_DAT (*(struct SKCDPOSE_DAT*)0x801ed998)
#define g_otherQueue (*(CdCommandQueueItem*)0x801ed8f0)

// Prototypes
void CD_HandleSyncError(void);
void CD_SetAudioVolume(u_char volume, int stereoChannel);
void CD_InitLocationEntries(int lba, int dataSizeBytes);
u_int CD_UpdateAndProcessQueue(void);
int CD_QueueAudioPlayback(char command, u_short resourceIndex, u_int dstBuffer, u_int callback);
void CD_SyncCallback_Handler(char intr, u_char *status);

#endif