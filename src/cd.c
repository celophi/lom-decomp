#include "cd.h"
#include "psyq/libetc.h"

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

/* Description: Initializes CD resource entry for disc location seeking

Params:
  lba - Logical Block Address (sector) on the CD to prepare
  dataSizeBytes - Size of data in bytes associated with this location

Returns: 
void

Notes: Synchronizes with VSync using stored timestamp to prevent command conflicts.
  Clears the default CD resource location structure (4 bytes zeroed).
  Converts LBA to CD-ROM MSF format and stores in global resource entry.
  Queues command 0x06 with SKCDPOSE_DAT as target buffer.
  SKCDPOSE_DAT likely stands for "Seek CD POSition Entry DATa".
  This appears to be a table of CdlLOC positions for disc seeking operations.
  Blocks until CD command queue is empty before setting audio volume.
  Sets CD audio volume to 128 (0x80) which may be default/mid-level. */

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
 * decomp.me (%): 99.65% (Hexadecimal immediates instead of decimal)
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
    CD_QueueAudioPlayback(6, 0xffff, &g_SKCDPOSE_DAT, 0);
    CD_WaitForQueueEmpty();
    CD_SetAudioVolume(128, 1);
}