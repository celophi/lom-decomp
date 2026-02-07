#include "cd.h"
#include "psyq/libetc.h"

//INCLUDE_ASM("asm/nonmatchings/cd", CD_InitializeSubsystem);

void CD_InitializeSubsystem(void)
{
    volatile CdCommandQueueItem *queueItem;
    u_int *statusFlagsPtr;
    int result;
    int queueCount;
    u_int scratchpadAddr;
    int endMarker;
    
    // Wait for CD-ROM system to initialize
    do {
        result = CdInit();
    } while (result == 0);
    
    CdSetDebug(0);
    
    // Store previous callbacks before setting new ones
    g_bigCdStruct.g_cdPreviousSyncCallback = CdSyncCallback(0);
    g_bigCdStruct.g_cdPreviousReadyCallback = CdReadyCallback(0);
    
    // Initialize queue loop variables
    queueCount = 15;
    scratchpadAddr = 0x1f800000;
    endMarker = -1;
    queueItem = &g_bigCdStruct.g_CdCommandQueue.Items[11];
    
    // Reset resource index to invalid value
    g_bigCdStruct.g_cdResourceIndex = 0xfffe;
    
    // Clear all CD state flags and counters
    g_bigCdStruct.g_cdAudioEnabled = 0;
    g_bigCdStruct.g_cdPlaybackState = 0;
    g_bigCdStruct.g_cdLoopCounter = 0;
    g_bigCdStruct.g_cdPlaybackFlag = 0;
    g_bigCdStruct.g_cdCurrentResourceIndex = 0;
    g_bigCdStruct.g_cdCurrentDataSize = 0;
    g_bigCdStruct.g_cdTargetDataSize = 0;
    g_bigCdStruct.g_cdSyncComplete = 0;
    g_bigCdStruct.g_cdInitState = 0;
    g_bigCdStruct.g_cdCurrentCommand = 0;
    g_bigCdStruct.g_cdInitCommand = 0;
    g_bigCdStruct.g_cdRetryCount = 0;
    g_bigCdStruct.g_cdRetryCounter = 0;
    g_bigCdStruct.g_cdLastCommand = 0;
    g_bigCdStruct.g_cdDstBuffer = 0;
    g_bigCdStruct.g_cdCallback = 0;
    g_bigCdStruct.g_cdQueueReadIndex = 0;
    g_bigCdStruct.g_cdQueueWriteIndex = 0;
    
    // Preserve only bit 7 (0x80) by masking off all other bits
    statusFlagsPtr = &g_bigCdStruct.g_cdStatusFlags;
    *statusFlagsPtr = *statusFlagsPtr & ~0x01;
    *statusFlagsPtr = *statusFlagsPtr & ~0x02;
    *statusFlagsPtr = *statusFlagsPtr & ~0x04;
    *statusFlagsPtr = *statusFlagsPtr & ~0x08;
    *statusFlagsPtr = *statusFlagsPtr & ~0x10;
    *statusFlagsPtr = *statusFlagsPtr & ~0x40;
    *statusFlagsPtr = *statusFlagsPtr & ~0x20;
    
    // Clear upper 3 bytes of status flags
    ((u_char*)statusFlagsPtr)[1] = 0;
    ((u_char*)statusFlagsPtr)[2] = 0;
    ((u_char*)statusFlagsPtr)[3] = 0;
    
    // Initialize command queue entries with scratchpad buffer
    do {
        queueItem[4].command = 0;
        queueItem[4].resourceIndex = 0;
        queueItem[4].dstBuffer = scratchpadAddr;
        queueItem[4].location = (CdlLOC*)scratchpadAddr;
        queueItem[4].callback = 0;
        queueItem--;
        queueCount--;
    } while (queueCount != endMarker);
    
    // Set CD-ROM mode parameters
    g_bigCdStruct.g_cdSetModeBuffer = 0xa0;
    g_bigCdStruct.field42_0x151 = 0;
    g_bigCdStruct.field43_0x152 = 0;
    g_bigCdStruct.field44_0x153 = 0;
    
    // Get CD-ROM status
    do {
        result = CdControlB(1, 0, &g_bigCdStruct.g_cdStatusByte);
    } while (result == 0);
    
    // Wait for disc to be ready if shell is open
    if ((g_bigCdStruct.g_cdStatusByte & 0x10) != 0) {
        result = CdDiskReady(1);
        while (result != 2) {
            result = CdDiskReady(0);
        }
    }
    
    // Set CD-ROM mode
    do {
        result = CdControlB(14, &g_bigCdStruct.g_cdSetModeBuffer, 0);
    } while (result == 0);
    
    // Store current VSync counter
    g_bigCdStruct.g_cdVSyncTimestamp = VSync(-1);
}

INCLUDE_ASM("asm/nonmatchings/cd", func_800118DC);

INCLUDE_ASM("asm/nonmatchings/cd", func_800119C0);

INCLUDE_ASM("asm/nonmatchings/cd", CD_QueueAudioPlayback);

INCLUDE_ASM("asm/nonmatchings/cd", CD_UpdateAndProcessQueue);

INCLUDE_ASM("asm/nonmatchings/cd", FUN_80012b48);

INCLUDE_ASM("asm/nonmatchings/cd", CD_SyncCallback_Handler2);

INCLUDE_ASM("asm/nonmatchings/cd", CD_SyncCallback_Handler);

INCLUDE_ASM("asm/nonmatchings/cd", CD_ReadyCallback);

INCLUDE_ASM("asm/nonmatchings/cd", CD_HandleSectorReadComplete);

INCLUDE_ASM("asm/nonmatchings/cd", CD_ExecuteCommand);

INCLUDE_ASM("asm/nonmatchings/cd", FUN_80013d74);


/**
 * Blocks execution until CD command queue is completely empty
 * 
 * Params:
 *  None
 * 
 * Returns: 
 *  void
 * 
 * Notes: Polls CD_UpdateAndProcessQueue in a tight loop with VSync sync.
 *  Each iteration waits one frame via VSync(0) to avoid busy-waiting.
 *  Returns only when queue size reaches zero.
 *  Used to ensure all pending CD commands complete before proceeding.
 * 
 * decomp.me link: https://decomp.me/scratch/rE8hd
 * decomp.me (%): 100%
 */
void CD_WaitForQueueEmpty(void)
{
    int state;
    
    while (state = CD_UpdateAndProcessQueue(), state != 0) {
        VSync(0);
    }
}

/*
 * Handle CD synchronization error and reset CD subsystem state.
 *
 * Clears active CD callbacks, resets command and retry state,
 * updates status flags to signal an error condition, and records
 * the current VSync timestamp for recovery timing.
 *
 * Params:
 *  None
 *
 * Returns:
 *  void
 * 
 * TODO: Verify the discrepency between makefile produced binary and decomp.me
 * This indicates some kind of toolchain problem. 
 * Consider looking at decomp.me compilers repository to see if there are any differences in the image.
 * 
 * decomp.me link: https://decomp.me/scratch/lU7lO
 * decomp.me (%): 100%
 */
void CD_HandleSyncError(void)
{
    CdSyncCallback((CdlCB) 0x0);
    CdReadyCallback((CdlCB) 0x0);
    
    g_bigCdStruct.g_cdStatusFlags |= 1;    
    g_bigCdStruct.g_cdInitState = 0;
    g_bigCdStruct.g_cdCurrentCommand = 0;
    g_bigCdStruct.g_cdInitCommand = 0;
    g_bigCdStruct.g_cdRetryCount = 0;
    g_bigCdStruct.g_cdRetryCounter = 0;
    g_bigCdStruct.g_cdStatusFlags &= ~0x10;
    g_bigCdStruct.g_cdVSyncTimestamp = VSync(-1);
}

/*
 * Set audio volume for a specific stereo channel
 *
 * Params:
 *  volume - Volume level to set (0-255)
 *  stereoChannel - 0 for left channel, non-zero for right channel
 *
 * Returns: void
 * 
 * decomp.me link: https://decomp.me/scratch/lwzx1
 * decomp.me (%): 100%
 */

void CD_SetAudioVolume(u_char volume, int stereoChannel)
{
  CdlATV audioConfig[2];
 do { 
     if (stereoChannel != 0) { 
         audioConfig[0].val0 = volume; 
         audioConfig[0].val1 = 0; 
         audioConfig[0].val2 = volume;
     } else { 
         audioConfig[0].val0 = volume; 
         audioConfig[0].val1 = volume; 
         audioConfig[0].val2 = 0; 
     } audioConfig[0].val3 = 0; 
 } while (0);
  CdMix(audioConfig);
}

INCLUDE_ASM("asm/nonmatchings/cd", func_80014014);

INCLUDE_ASM("asm/nonmatchings/cd", func_800140D4);

/**
 * Initializes CD resource entry for disc location seeking
 * 
 * Params:
 * lba - Logical Block Address (sector) on the CD to prepare
 * dataSizeBytes - Size of data in bytes associated with this location
 * 
 * Returns:
 * void
 * 
 * Notes: Synchronizes with VSync using stored timestamp to prevent command conflicts.
 * Clears the default CD resource location structure (4 bytes zeroed).
 * Converts LBA to CD-ROM MSF format and stores in global resource entry.
 * Queues command 0x06 with SKCDPOSE_DAT as target buffer.
 * SKCDPOSE_DAT likely stands for "Seek CD POSition Entry DATa".
 * This appears to be a table of CdlLOC positions for disc seeking operations.
 * Blocks until CD command queue is empty before setting audio volume.
 * Sets CD audio volume to 128 (0x80) which may be default/mid-level.
 * 
 * decomp.me link: https://decomp.me/scratch/4PljL
 * decomp.me (%): 100%
 * 
 * TODO: Figure out why g_SKCDPOSE_DAT is an undefined reference and not included in splat output.
 */
void CD_InitLocationEntries (int lba, int dataSizeBytes)
{
    CdlLOC *location;
    int vsyncOffset;
    int vsyncDelta;
    BigCdStruct *cdStruct;
    
    vsyncOffset = -3;
    vsyncDelta = VSync(-1);
    vsyncDelta = g_bigCdStruct.g_cdVSyncTimestamp - (vsyncDelta + vsyncOffset);
    
    if (vsyncDelta > 0)
    {
        if (vsyncDelta == 1)
        {
            vsyncDelta = 0;
        }
        
        VSync(vsyncDelta);
    }
    
    cdStruct = &g_bigCdStruct;
    location = &cdStruct->g_defaultCdResource.Location;
    *(u_int*)&cdStruct->g_defaultCdResource.Location = 0;
    cdStruct->g_defaultCdResource.dataSize = dataSizeBytes;
    
    CdIntToPos(lba, location);

    // 0x801ed998 is &g_SKCDPOSE_DAT
    CD_QueueAudioPlayback(6, 0xffff, 0x801ed998, 0);

    CD_WaitForQueueEmpty();
    CD_SetAudioVolume(128, 1);
}