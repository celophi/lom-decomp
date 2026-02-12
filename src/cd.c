#include "cd.h"
#include "psyq/libetc.h"
#include "psyq/libcd.h"

/**
 * Initializes the CD-ROM subsystem and resets all CD state
 * 
 * Params:
 *  None
 * 
 * Returns: 
 *  void
 * 
 * Notes: Blocks until CdInit succeeds before proceeding with initialization.
 *  Stores previous sync and ready callbacks before clearing them.
 *  Resets resource index to 0xfffe (invalid marker value).
 *  Clears all CD state flags, counters, and command queue indices.
 *  Preserves only bit 7 (0x80) of status flags by masking bits 0-6 and 5.
 *  Initializes 16 command queue entries with scratchpad buffer addresses.
 *  Sets CD mode to 0xa0 (double speed with auto-pause).
 *  Waits for disc ready if shell open flag (0x10) is set in status byte.
 *  Applies CD mode settings via CdControlB command 0x0e.
 *  Captures VSync timestamp at completion for timing reference.
 * 
 * decomp.me link: https://decomp.me/scratch/xzkLK
 * decomp.me (%): 100%
 */
void CD_InitializeSubsystem(void)
{
    int endMarker;
    int queueCount;
    volatile CdCommandQueueItem *queueItem;
    u_int scratchpadAddr;
    u_int *statusFlagsPtr;
    int result;
   
    // Wait for CD-ROM system to initialize
    do {
        result = CdInit();
    } while (result == 0);
    
    CdSetDebug(0);
    
    // Store previous callbacks before setting new ones
    g_cdSyncCallbackResult = CdSyncCallback(0);

    // Force branch delay slot for "0" argument
    do {} while (0);
     
    g_cdReadyCallbackResult = CdReadyCallback(0);
    
    // Reset resource index to invalid value
    statusFlagsPtr = &g_cdSystem.statusFlags.word;
    
    queueCount = 15;
    scratchpadAddr = 0x1f800000;
    
    endMarker = -1;
    queueItem = &g_otherQueue;
    
    g_cdSystem.resourceIndex = 0xfffe;
    
    // Clear all CD state flags and counters
    g_cdSystem.audioEnabled = 0;
    g_cdSystem.playbackState = 0;
    g_cdSystem.loopCounter = 0;
    g_cdSystem.playbackFlag = 0;
    g_cdSystem.currentResourceIndex = 0;
    g_cdSystem.currentDataSize = 0;
    g_cdSystem.targetDataSize = 0;
    g_cdSystem.syncComplete = 0;
    g_cdSystem.initState = 0;
    g_cdSystem.currentCommand = 0;
    g_cdSystem.initCommand = 0;
    g_cdSystem.retryCount = 0;
    g_cdSystem.retryCounter = 0;
    g_cdSystem.lastCommand = 0;
    g_cdSystem.dstBuffer = 0;
    g_cdSystem.callback = 0;
    g_cdSystem.queueReadIndex = 0;
    g_cdSystem.queueWriteIndex = 0;
    
    
    // Preserve only bit 7 (0x80) by masking off all other bits
    
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
    g_cdSystem.setModeBuffer = 0xa0;
    g_cdSystem.u_151 = 0;
    g_cdSystem.u_152 = 0;
    g_cdSystem.u_153 = 0;
    
    // Get CD-ROM status
    do {
        result = CdControlB(1, 0, &g_cdSystem.statusByte);
    } while (result == 0);
    
    // Wait for disc to be ready if shell is open
    if ((g_cdStatusByte & 0x10) != 0) {
        result = CdDiskReady(1);
        while (result != 2) {
            result = CdDiskReady(0);
        }
    }
    
    // Set CD-ROM mode
    do {
        result = CdControlB(14, &g_cdSystem.setModeBuffer, 0);
    } while (result == 0);
    
    // Store current VSync counter
    g_cdVSyncTimestamp = VSync(-1);
}

/**
 * Description: Stops CD playback and resets CD subsystem to idle state
 * 
 * Params:
 *   None
 * 
 * Returns: void
 * 
 * Notes: Stops audio playback via func_80014014 if CD audio is enabled.
 *   Clears bit 6 (0x40) from status flags before pausing.
 *   Removes sync and ready callbacks before issuing pause command.
 *   Blocks until CdControlB pause command succeeds.
 *   Resets resource index to 0xfffe (invalid marker value).
 *   Clears all playback state, counters, command flags, and buffers to 0.
 *   Clears bit 4 (0x10) from status flags after state reset.
 *   Captures VSync timestamp for timing reference.
 *   Resets status bytes b1 and b2 along with queue read/write indices.
 *   Flushes CD buffer at completion.
 * 
 * decomp.me link: https://decomp.me/scratch/izusq
 * decomp.me (%): 100%
 */
void CD_PauseAndClearState(void)
{
    int result;
    CdSystem* reference;

    reference = &g_cdSystem;

    if (g_cdAudioEnabled != 0) {
        func_80014014();
    }

    reference->statusFlags.word = g_cdSystem.statusFlags.word & 0xffffffbf;

    CdSyncCallback((CdlCB)0x0);
    CdReadyCallback((CdlCB)0x0);

    do {
    result = CdControlB(CdlPause,(u_char *)0x0,(u_char *)0x0);
    } while (result == 0);

    g_cdSystem.resourceIndex = 0xfffe;
    g_cdSystem.playbackFlag = 0;
    g_cdSystem.currentResourceIndex = 0;
    g_cdSystem.currentDataSize = 0;
    g_cdSystem.targetDataSize = 0;
    g_cdSystem.playbackState = 0;
    g_cdSystem.loopCounter = 0;
    g_cdSystem.currentCommand = 0;
    g_cdSystem.initCommand = 0;
    g_cdSystem.retryCount = 0;
    g_cdSystem.retryCounter = 0;
    g_cdSystem.lastCommand = 0;
    g_cdSystem.dstBuffer = 0;
    g_cdSystem.callback = 0;
    g_cdSystem.statusFlags.word = g_cdSystem.statusFlags.word & 0xffffffef;
    g_cdSystem.vsyncTimestamp = VSync(-1);
    g_cdSystem.statusFlags.bytes.b1 = 0;
    g_cdSystem.statusFlags.bytes.b2 = 0;
    g_cdSystem.queueReadIndex = 0;
    g_cdSystem.queueWriteIndex = 0;

    CdFlush();
}

/**
 * decom.me link: https://decomp.me/scratch/8sOtx
 * decomp.me (%): 90.71%
 */
s32 CD_StreamData(s32 index, u32 dst) 
{
    s32 bufferRemainder;
    s32 newBufferStart;
    s32 wrapAmount;
    s32 currentBufferSize;
    s32 processedLength;
    s32 oldBufferStart;
    
    s32 timestamp;
    s32 remainingDataSize;
    s32 bytesToCopy;
    s32* wrapDstPtr;
    s32* wrapSrcPtr;
    u32 srcEnd;

    u8* scratchpad;
    u8 isReady;
    u8* scratchRef;
    u32 initialDst;

    while (CD_UpdateAndProcessQueue() != 0) {
        VSync(0);
    }

    initialDst = dst;
    scratchpad = (u8*) 0x1F800000;
    
    *(s32* )(scratchpad + 0x18) = 0;
    *(u8* )(scratchpad) = 0U;
    *(u8* )(scratchpad + 0x01) = 0U;
    *(s32* )(scratchpad + 0x14) = 0;
    
    remainingDataSize = CD_EnqueueCommand(6, index & 0xFFFF, 0U, (u32) &FUN_80014888) - 1;
   
    while (1) {

        timestamp = VSync(-1);
        scratchRef = scratchpad;
        isReady = 1;
        
wait_for_command_complete:
        if (VSync(-1) >= (timestamp + 30)) {
            goto CD_StreamData_process;
        }
        
        if (*(u8* )scratchRef != isReady) {
            goto wait_for_command_complete;
        }
    
        while (1) {
                
                currentBufferSize = *(s32* )(scratchRef + 0x0C);
                
                if (currentBufferSize < remainingDataSize) {
                    srcEnd = (*(s32* )(scratchRef + 0x04) + currentBufferSize) - 280;
                } else {
                    srcEnd = *(s32* )(scratchRef + 0x04) + remainingDataSize;
                }
                
                if (CD_DecompressData((u32* )0x1F800008, &dst, srcEnd, -4U) == 0) {
                    return dst - initialDst;
                }
    
                if (currentBufferSize != *(s32* )(0x1F80000C)) {
                    continue;
                }
    
                processedLength = *(s32* )(scratchRef + 0x08) - *(s32* )(scratchRef + 0x04);
                *(s32* )(scratchRef + 0x14) = processedLength;
    
                // zero's out the address, so clear out the src data pointer.
                FUN_80014ad0(0x1F800000);
                
                remainingDataSize -= processedLength;
                if (*(u8* )(scratchRef + 0x01) != isReady) {
                    continue;
                }
    
                wrapAmount = *(s32* )(scratchRef + 0x10);
                
                if (wrapAmount != 0) {
                    bufferRemainder = *(s32* )(scratchRef + 0x0C) - processedLength;
                    newBufferStart = 0x801DC118 - bufferRemainder;
                    
                    oldBufferStart = *(s32* )(scratchRef + 0x04);
                    *(s32* )(scratchRef + 0x08) = newBufferStart;
                    *(s32* )(scratchRef + 0x04) = newBufferStart;
                    
                    bytesToCopy = ((4 - (bufferRemainder & 3) & 3));

                    wrapDstPtr = newBufferStart - bytesToCopy;
                    wrapSrcPtr = (oldBufferStart + processedLength) - bytesToCopy;
    
                    bytesToCopy = bufferRemainder + 3;
    
                    *(s32* )(scratchRef + 0x0C) = wrapAmount + bufferRemainder;
                    
                    if (bytesToCopy < 0) {
                        bytesToCopy = bufferRemainder + 6;
                    }
    
                    for (bufferRemainder = (bytesToCopy >> 2) - 1; bufferRemainder != 1; bufferRemainder--) {
                        *wrapDstPtr++ = *wrapSrcPtr++;
                    }
                } else {
                    *(s32* )(scratchRef + 0x04) += processedLength;
                    *(s32* )(scratchRef + 0x0C) -= processedLength;
                }

                *(u8* )(scratchRef) = isReady;    
                continue;
            }

CD_StreamData_process:
        CD_UpdateAndProcessQueue();
        timestamp = VSync(-1);
    }
}


/**
 * decomp.me link: https://decomp.me/scratch/tWHW2
 * decomp.me (%): 97.54% 
 */
s32 CD_EnqueueCommand(u8 command, u16 resourceIndex, s32 dstBuffer, s32 callback) {
    s32 temp_a0;
    s32 temp_a1;
    s32 queueWriteIndex;
    s32 temp_a1_3;
    s32 temp_v1;
    s32 var_v0;
    CdResourceEntry* resourceEntry;
    
    u8 currentCommand;
    
    if (g_cdSystem.statusFlags.word & 0x40) {
        return -3;
    }
    
    temp_a1 = resourceIndex & 0xFFFF;
    
    if (temp_a1 == 0xFFFF) {
        resourceEntry = (void* )0x801ED990;
    } else {
        resourceEntry = (temp_a1 * 8) + 0x801ED998;
    }

    if ( g_cdSystem.currentCommand == 0) {
        
        if (g_cdSystem.initCommand != 0) {
            goto CD_EnqueueCommand_check_params;
        }

        goto CD_EnqueueCommand_end;
    }

CD_EnqueueCommand_check_params:
    
    if (
        (g_cdSystem.lastCommand != (command & 0xFF)) || 
        (g_cdSystem.resourceIndex != (resourceIndex & 0xFFFF)) || (g_cdSystem.dstBuffer != dstBuffer) || (g_cdSystem.callback != callback)) {
        
        var_v0 = -2;
        if (*(u_int*)&resourceEntry->location != 0) {
            
            if (resourceEntry->dataSize == 0) {
                return -2;
            }
            
            queueWriteIndex = g_cdSystem.queueWriteIndex;
            
            if (g_cdSystem.queueReadIndex == ((queueWriteIndex + 1) & 0xF)) {
                return -1;
            }

            queueWriteIndex = g_cdSystem.queueWriteIndex;
            g_cdSystem.commandQueue.items[queueWriteIndex].command = command;
            g_cdSystem.lastCommand = command;

            queueWriteIndex = g_cdSystem.queueWriteIndex;
            g_cdSystem.commandQueue.items[queueWriteIndex].resourceIndex = resourceIndex;
            g_cdSystem.resourceIndex = resourceIndex;

            queueWriteIndex = g_cdSystem.queueWriteIndex;
            g_cdSystem.commandQueue.items[queueWriteIndex].location = &resourceEntry->location;

            queueWriteIndex = g_cdSystem.queueWriteIndex;
            g_cdSystem.commandQueue.items[queueWriteIndex].dstBuffer = dstBuffer;

            g_cdSystem.dstBuffer = dstBuffer;
            
            queueWriteIndex = g_cdSystem.queueWriteIndex;
            g_cdSystem.commandQueue.items[g_cdSystem.queueWriteIndex].callback = callback;
            g_cdSystem.callback = callback;
            
            queueWriteIndex = g_cdSystem.queueWriteIndex;
            g_cdSystem.queueWriteIndex = (s32) ((g_cdSystem.queueWriteIndex + 1) & 0xF);
            
            temp_a0 = VSync(-1);

            currentCommand = g_cdSystem.currentCommand;
            if ((currentCommand == 0) && (g_cdSystem.initCommand  == 0)) {
                
                temp_a1_3 = g_cdSystem.statusFlags.word;
                
                if (!(temp_a1_3 & 0xF)) {
                    
                    g_cdSystem.vsyncTimestamp = temp_a0;
                    g_cdSystem.playbackFlag = 1;
                    g_cdSystem.currentResourceIndex = resourceIndex;
                    temp_v1 = resourceEntry->dataSize;
                    g_cdSystem.currentCommand = 1U;
                    g_cdSystem.statusFlags.word = (s32) (temp_a1_3 | 0x10);
                    g_cdSystem.playbackState  = 0;
                    g_cdSystem.loopCounter = 0;
                    g_cdSystem.targetDataSize = temp_v1;
                    g_cdSystem.currentDataSize = temp_v1;
                    
                    CdSyncCallback(&CD_SyncCallback_Handler2);
                    CdSync(0, 0);
                    CdControlF(CdlNop, 0);
                }
            }
            goto CD_EnqueueCommand_end;
        }
        /* Duplicate return node #21. Try simplifying control flow for better match */
        return var_v0;
    }

CD_EnqueueCommand_end:
    var_v0 = resourceEntry->dataSize;
    return var_v0;
}


INCLUDE_ASM("asm/nonmatchings/cd", CD_UpdateAndProcessQueue);

/**
 * Description: Processes CD-ROM initialization state machine across multiple VSync frames
 * 
 * Params:
 *   None
 * 
 * Returns: 1 if CD not initialized, 0 otherwise
 * 
 * TODO: Verify exact timing values for VSync delays (1, 4, 30 frames)
 * TODO: Confirm command codes 0x10, 0x11, 0x12 mappings
 * TODO: Determine why state 3 subtracts 30 from timestamp instead of resetting
 * TODO: Decomp better :)
 * 
 * Notes: State machine with 4 states (0-3) for asynchronous CD initialization.
 * State 0: Calls CdFlush() and advances to state 1 after 1 VSync frame.
 * State 1: Configures mode 0xa0, sets sync/ready callbacks, sends CdlSetmode command, waits 4 VSync frames.
 * State 2: Sets filter mode with parameters (1,1), advances to state 3 immediately.
 * State 3: Waits for syncComplete flag or 30 frame timeout, then issues follow-up commands.
 * Uses cdVSyncTimestamp field to track frame delays between state transitions.
 * Command 0x11 triggers CdlDemute, 0x12 triggers CdlPause, others trigger CdlSetfilter with reset to 0x10.
 * Early exits return 0 to indicate "still processing", non-zero only if CD hardware not ready.
 * 
 * decomp.me link: https://decomp.me/scratch/J6JQ8
 * decomp.me (%): 94.89%
 */
int CD_ProcessInitStateMachine(void) {
    int timestamp;
    u_char com;
    u_char local_18[8];
    u_char * p;

    if ((g_cdSystem.statusFlags.word & 8) == 0) {
        return 1;
    }

    switch (g_cdSystem.initState) {
    case 0:
        CdFlush();
        g_cdSystem.initState = 1;

        g_cdSystem.vsyncTimestamp = VSync(-1) + 1;
        goto return_zero;

    case 1:
        timestamp = VSync(-1);

        if (g_cdSystem.vsyncTimestamp <= timestamp) {
            g_cdSystem.modeParams = 0xa0;
            g_cdSystem.u_155 = 0;
            g_cdSystem.u_156 = 0;
            g_cdSystem.u_157 = 0;

            CdSyncCallback(CD_SyncCallback_Handler);
            CdReadyCallback((CdlCB) 0x0);

            g_cdSystem.initCommand = 0x10;

            CdControlF(CdlSetmode, & g_cdSystem.modeParams);

            timestamp = VSync(-1);

            g_cdSystem.vsyncTimestamp = timestamp + 4;
            goto return_zero;
        }

        return 0;

    case 2:
        CdSyncCallback(CD_SyncCallback_Handler);
        g_cdSystem.initCommand = 0x11;
        local_18[0] = 1;
        local_18[1] = 1;
        CdControlF('\r', local_18);
        g_cdSystem.initState = 3;
        g_cdSystem.vsyncTimestamp = VSync(-1);
        goto return_zero;

    case 3:
        if (g_cdSystem.syncComplete == 1) {
            g_cdSystem.vsyncTimestamp = VSync(-1);
            g_cdSystem.syncComplete = 0;
            goto return_zero;
        }
        timestamp = VSync(-1);
        if (timestamp < g_cdSystem.vsyncTimestamp + 0x1e) {
            goto return_zero;
        }
        CdSyncCallback(CD_SyncCallback_Handler);
        if (g_cdSystem.initCommand != 0x11) {
            do {
                if (g_cdSystem.initCommand < 0x12) {
                    goto do_setfilter;
                }
                if (g_cdSystem.initCommand == 0x12) {
                    CdControlF(0x09, (u_char * ) 0x0);
                    goto LAB_80012d48;
                }
                do_setfilter: local_18[0] = 1;
                local_18[1] = 1;
                CdControlF(0x0d, local_18);
                g_cdSystem.initCommand = 0x10;
            } while (0);
        } else {
            CdControlF(0x0c, (u_char * ) 0x0);
        }
        LAB_80012d48:
            g_cdSystem.vsyncTimestamp = g_cdSystem.vsyncTimestamp + -0x1e;
        goto return_zero;

    default:
        goto return_zero;
    }

return_zero:
        return 0;
}

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
    CdSyncCallback(0);
    CdReadyCallback(0);
    
    g_cdSystem.initState = 0;
    g_cdSystem.statusFlags.word |= 1;    
    g_cdSystem.currentCommand = 0;
    g_cdSystem.initCommand = 0;
    g_cdSystem.retryCount = 0;
    g_cdSystem.retryCounter = 0;
    g_cdSystem.statusFlags.word &= ~0x10;
    g_cdSystem.vsyncTimestamp = VSync(-1);
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

INCLUDE_ASM("asm/nonmatchings/cd", CD_ResetSystem);

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
    CdSystem *cdStruct;
    
    vsyncOffset = -3;
    vsyncDelta = VSync(-1);
    vsyncDelta = g_cdVSyncTimestamp - (vsyncDelta + vsyncOffset);
    
    if (vsyncDelta > 0)
    {
        if (vsyncDelta == 1)
        {
            vsyncDelta = 0;
        }
        
        VSync(vsyncDelta);
    }
    
    cdStruct = &g_cdSystem;
    location = &cdStruct->defaultCdResource.location;
    *(u_int*)&cdStruct->defaultCdResource.location = 0;
    cdStruct->defaultCdResource.dataSize = dataSizeBytes;
    
    CdIntToPos(lba, location);
    CD_EnqueueCommand(6, 0xffff, &g_SKCDPOSE_DAT, 0);
    CD_WaitForQueueEmpty();
    CD_SetAudioVolume(128, 1);
}