#include "cd.h"
#include "psyq/libetc.h"

/*
 * Handle CD synchronization error and reset CD subsystem state.
 *
 * Clears active CD callbacks, resets command and retry state,
 * updates status flags to signal an error condition, and records
 * the current VSync timestamp for recovery timing.
 *
 * Params:
 *   None
 *
 * Returns:
 *   void
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
 *   volume - Volume level to set (0-255)
 *   stereoChannel - 0 for left channel, non-zero for right channel
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
