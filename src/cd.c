#include "cd.h"

void CD_HandleSyncError(void)
{
  u_int temp;
  volatile u_int *pStatus;
    int result;
  
  CdSyncCallback((CdlCB)0x0);
  CdReadyCallback((CdlCB)0x0);
  g_bigCdStruct.g_cdInitState = 0;
  g_bigCdStruct.g_cdCurrentCommand = 0;
  g_bigCdStruct.g_cdInitCommand = 0;
  g_bigCdStruct.g_cdRetryCount = 0;
  g_bigCdStruct.g_cdRetryCounter = 0;
  pStatus = (volatile u_int*)&g_bigCdStruct;
  temp = *pStatus;
  temp = temp | 1;
  *pStatus = temp;
  temp = temp & 0xffffffef;
  *pStatus = temp;
    result = VSync(-1);
  g_bigCdStruct.g_cdVSyncTimestamp = result;
  return;
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
