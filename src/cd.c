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

/* Description:
     Prepares and queues CD audio playback starting from a specified LBA.
     Initializes global playback state, synchronizes timing with VSync,
     converts the LBA to MSF format, and enqueues a CD read command.
   
   Params:
     sector        - Logical Block Address (LBA) on the CD to start playback from.
     dataSizeBytes - Size of audio data in bytes to be associated with the playback.
   
   Returns:
     void
   
   Behavior:
     - Computes a delay relative to the last recorded CD VSync timestamp and
       synchronizes with VSync to align CD operations with video timing.
     - Clears and initializes the global CD location structure.
     - Stores dataSizeBytes in g_cdDefaultLocation.dataSize for later reference.
     - Converts the starting LBA to CD-ROM minute:second:sector (MSF) format
       via CdIntToPos and stores it in g_cdDefaultLocation.Location.
     - Queues a CD read command (CdlReadN) with parameter 0xFFFF and a predefined
       audio data descriptor.
     - Blocks until the CD command queue becomes empty.
     - Sets the CD audio mixer volume to 0x80 (mid-level stereo volume).
   
   Notes:
     - Uses g_cdVSyncTimestamp to enforce a minimum inter-command delay.
     - The queued command uses a fixed parameter (0xFFFF), likely indicating
       an open-ended or streaming read length.
     - Audio playback does not necessarily begin immediately; this function
       prepares and schedules the necessary CD operations.
   
   Decompilation:
     https://decomp.me/scratch/pF5sN */

void CD_InitAudioPlayback(int lba,int dataSizeBytes)
{
  int vsyncDelta;
  
  vsyncDelta = VSync(-1);
  vsyncDelta = g_bigCdStruct.g_cdVSyncTimestamp - (vsyncDelta + -3);
  if (0 < vsyncDelta) {
    if (vsyncDelta == 1) {
      vsyncDelta = 0;
    }
    VSync(vsyncDelta);
  }

  g_bigCdStruct.g_defaultCdResource.Location.minute = 0;
  g_bigCdStruct.g_defaultCdResource.Location.second = 0;
  g_bigCdStruct.g_defaultCdResource.Location.sector = 0;
  g_bigCdStruct.g_defaultCdResource.Location.track = 0;
  g_bigCdStruct.g_defaultCdResource.dataSize = dataSizeBytes;

  CdIntToPos(lba, &g_bigCdStruct.g_defaultCdResource.Location);
  CD_QueueAudioPlayback(CdlReadN, 0xffff, 0x801ed998, 0);
  CD_WaitForQueueEmpty();
  CD_SetAudioVolume(128, 1);
  return;
}