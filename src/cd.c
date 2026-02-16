#include "cd.h"
#include "psyq/libetc.h"
#include "psyq/libcd.h"

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
 * 5. Sets CD mode to CdlModeSpeed | CdlModeSize1 (double speed + XA filter)
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
void CD_Initialize(void)
{
    int queueEndMarker;
    int queueCount;
    volatile CdCommandQueueItem *queueItem;
    CdResourceEntry* scratchpadAddr;
    CdStatusFlags *statusFlagsPtr;
    int cdResult;
   
    // Wait for CD-ROM system to initialize
    while (TRUE) {
        if (CdInit() != 0) {
            break;
        }
    }
    
    CdSetDebug(0);
    
    // Save previous callbacks, then clear them
    g_cdSyncCallbackResult = CdSyncCallback(NULL);
    g_cdReadyCallbackResult = CdReadyCallback(NULL);
    
    statusFlagsPtr = &g_cdSystem.statusFlags;
    
    queueCount = CD_COMMAND_QUEUE_SIZE - 1;
    scratchpadAddr = (CdResourceEntry*)g_scratchpad;
    
    queueEndMarker = -1;
    
    // g_commandQueueOffset is commandQueue.items[11]. 
    // The loop uses queueItem[4] to walk items[15] down to items[0] (all 16 entries).
    queueItem = &g_commandQueueOffset;
    
    // 0xFFFE = invalid/no resource loaded
    g_cdSystem.resourceIndex = CD_RESOURCE_INDEX_INVALID;
    
    // Reset all runtime state to zero
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
    
    // Clear statusFlags bits 0-6, preserving only bit 7 (0x80).
    // Each bit is cleared individually to match the original assembly output.
    statusFlagsPtr->word &= ~0x01;
    statusFlagsPtr->word &= ~0x02;
    statusFlagsPtr->word &= ~0x04;
    statusFlagsPtr->word &= ~0x08;
    statusFlagsPtr->word &= ~0x10;
    statusFlagsPtr->word &= ~0x40;
    statusFlagsPtr->word &= ~0x20;
    
    // Clear upper 3 bytes of status flags
    statusFlagsPtr->bytes.b1 = 0;
    statusFlagsPtr->bytes.b2 = 0;
    statusFlagsPtr->bytes.b3 = 0;
    
    // Zero all 16 command queue entries, setting default buffer to scratchpad
    while (queueCount != queueEndMarker) {
        queueItem[4].command = 0;
        queueItem[4].resourceIndex = 0;
        queueItem[4].dstBuffer = scratchpadAddr;
        queueItem[4].entry = scratchpadAddr;
        queueItem[4].callback = 0;
        queueItem--;
        queueCount--;
    }
    
    g_cdSystem.setModeBuffer = (CdlModeSpeed | CdlModeSize1);
    g_cdSystem.u_151 = 0;
    g_cdSystem.u_152 = 0;
    g_cdSystem.u_153 = 0;
    
    // CdlNop (1) — read current drive status into statusByte
    while (TRUE) {
        cdResult = CdControlB(CdlNop, NULL, &g_cdSystem.statusByte);
        
        if (cdResult != 0) {
            break;
        }
    }
    
    // If shell-open flag (0x10) is set, block until disc becomes ready
    if ((g_cdStatusByte & CdlStatShellOpen) != 0) {
        cdResult = CdDiskReady(1);
        
        while (cdResult != CdlComplete) {
            cdResult = CdDiskReady(0);
        }
    }
    
    // CdlSetmode (14) — apply mode byte (0xA0) to the drive
    while (TRUE) {
        cdResult = CdControlB(CdlSetmode, &g_cdSystem.setModeBuffer, NULL);
        
        if (cdResult != 0) {
            break;
        }
    }
    
    // Record current frame counter for timeout tracking
    g_cdVSyncTimestamp = VSync(-1);
}

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
void CD_Stop(void)
{
    int cdResult;
    CdSystem* cdSystem;
    
    cdSystem = &g_cdSystem;
    
    if (g_cdAudioEnabled != 0) {
        CD_ResetSystem();
    }
    
    // Clear the "playing" flag (bit 6) while preserving other status bits
    cdSystem->statusFlags.word &= 0xFFFFFFBF;
    
    CdSyncCallback(NULL);
    CdReadyCallback(NULL);

    // Repeatedly send pause command until drive acknowledges
    while (TRUE) {
        cdResult = CdControlB(CdlPause, NULL, NULL);
        if (cdResult != 0) {
            break;
        }
    }
    
    g_cdSystem.resourceIndex = CD_RESOURCE_INDEX_INVALID;
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
    g_cdSystem.statusFlags.word &= 0xFFFFFFEF;
    g_cdSystem.vsyncTimestamp = VSync(-1);
    g_cdSystem.statusFlags.bytes.b1 = 0;
    g_cdSystem.statusFlags.bytes.b2 = 0;
    g_cdSystem.queueReadIndex = 0;
    g_cdSystem.queueWriteIndex = 0;
    
    CdFlush();
}

/**
 * @brief Streams and decompresses CD-ROM sector data into a destination buffer
 *
 * Reads sectors from disc via DMA into a ring buffer (managed through
 * scratchpad RAM), then incrementally decompresses the buffered data
 * into the caller's destination address.
 *
 * @details
 * Scratchpad RAM (0x1F800000) is used as a shared communication struct
 * between this function and the CD read callback (FUN_80014888):
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
 * @note decomp.me: (97.84%) https://decomp.me/scratch/CSYVd
 */
s32 CD_StreamData(s32 command, u32 destination) {
    s32 unprocessedBytes;
    s32 relocDstAddr;
    s32 wrapAmount;
    s32 bytesBuffered;
    s32 bytesConsumed;
    s32 prevReadPtr;
    s32 copySize;
    s32 timestamp;
    s32 remainingDataSize;
    s32* wrapDestinationPtr;
    s32* relocSrcPtr;
    u32 decompressEnd;
    u8* scratchpad;
    u32 destStart;
    u8* streamState;
    s32 alignRemainder;
    s32 sentinel;
    s32 addr;

    /* Block until any in-progress CD commands finish */
    while (CD_UpdateAndProcessQueue() != 0) {
        VSync(0);
    }

    destStart = destination;
    scratchpad = (u8*)g_scratchpad;
    
    /* Zero out the scratchpad streaming state */
    *(s32*)(scratchpad + 0x18) = 0;       /* reserved */
    *(u8*)(scratchpad) = 0U;              /* dataReady = false */
    *(u8*)(scratchpad + 0x01) = 0U;       /* bufferWrapped = false */
    *(s32*)(scratchpad + 0x14) = 0;       /* bytesConsumed = 0 */
    
    /* Enqueue a CdlReadN command; return value is the resource's total data size.
     * Subtract 1 to get the last valid byte offset for streaming. */
    remainingDataSize = CD_EnqueueCommand(CdlReadN, command, NULL, &FUN_80014888) - 1;
    timestamp = VSync(-1);

    streamState = (u8*)g_scratchpad;

    /* === Main streaming loop === */
    while (TRUE) {
        if (VSync(-1) < (timestamp + 30)) {
            /* Timeout hasn't elapsed — check if callback signaled new data */
            if (*streamState != 1) {
                continue;
            }
            
            /* === Decompression loop: process all available buffered data === */
            while (TRUE) {
                bytesBuffered = *(s32*)(streamState + 0x0C);
                
                /* Calculate source-end boundary for decompression.
                 * If we have fewer bytes buffered than total remaining, hold back
                 * 280 bytes as a safety margin to avoid reading incomplete sectors.
                 * Otherwise use the exact remaining size as the boundary. */
                if (bytesBuffered < remainingDataSize) {
                    decompressEnd = (*(s32*)(streamState + 0x04) + bytesBuffered) - 280;
                } else {
                    decompressEnd = *(s32*)(streamState + 0x04) + remainingDataSize;
                }
                
                /* Decompress a chunk; returns 0 when all output is complete */
                if (CD_DecompressData((u32*)(g_scratchpad + 0x08), &destination, decompressEnd, -4U) == 0) {
                    return destination - destStart;
                }
        
                /* If bytesBuffered changed mid-iteration (callback wrote more data),
                 * re-loop to recalculate the decompression boundary */
                if (bytesBuffered != *(s32*)(g_scratchpad + 0x0C)) {
                    continue;
                }
        
                /* All currently buffered data has been fed to the decompressor */
                bytesConsumed = *(s32*)(streamState + 0x08) - *(s32*)(streamState + 0x04);
                *(s32*)(streamState + 0x14) = bytesConsumed;
                ClearPointer(g_scratchpad);
                remainingDataSize -= bytesConsumed;
                
                /* If the ring buffer hasn't wrapped, yield to let more data arrive */
                if (*(u8*)(streamState + 0x01) != 1) {
                    goto do_vsync;
                }

                /* --- Handle ring buffer wrap-around --- */
                decompressEnd = *(s32*)(streamState + 0x10);  /* wrapOverflow amount */
                if (decompressEnd != 0) {
                    /* Relocate unprocessed tail bytes to just before the ring buffer
                     * end (0x801DC118), making the data contiguous again. */
                    unprocessedBytes = *(s32*)(streamState + 0x0C) - bytesConsumed;
                    alignRemainder = (unprocessedBytes & 3);
                    relocDstAddr = 0x801DC118 - unprocessedBytes;
                    prevReadPtr = *(s32*)(streamState + 0x04);
                    
                    /* Word-align the relocation destination downward */
                    copySize = 4 - alignRemainder;
                    *(s32*)(streamState + 0x08) = relocDstAddr;
                    *(s32*)(streamState + 0x04) = relocDstAddr;
                    copySize = copySize & 3;
                    
                    /* Adjust pointers to include alignment padding bytes */
                    relocDstAddr = (s32*)(relocDstAddr - copySize);
                    relocSrcPtr = (s32*)((prevReadPtr + bytesConsumed) - copySize);
                    
                    /* Merge overflow bytes into the new contiguous buffer region */
                    *(s32*)(streamState + 0x0C) = decompressEnd + unprocessedBytes;
                    copySize = unprocessedBytes + 3;
                    
                    if (copySize < 0) {
                        copySize = unprocessedBytes + 6;
                    }

                    /* Copy unprocessed bytes word-by-word to the relocated position */
                    sentinel = -1;
                    unprocessedBytes = (copySize >> 2) - 1;
                    if (unprocessedBytes != -1) {
                        while(TRUE) {
                            *(s32*)relocDstAddr = *relocSrcPtr++;
                            relocDstAddr+=4;
                            unprocessedBytes--;
                            if (unprocessedBytes == sentinel) {
                                break;
                            }
                        }
                    }
                    
                } else {
                    /* No wrap — simply advance the read pointer past consumed data */
                    *(s32*)(streamState + 0x04) += bytesConsumed;
                    *(s32*)(streamState + 0x0C) -= bytesConsumed;
                }

                /* Memory barrier: prevent compiler from reordering the ready flag write */
                __asm__ volatile ("" ::: "memory");
                *(u8*)streamState = 1;
                
                goto do_vsync;
            }
        } 
        
        /* Timeout elapsed without data — pump the CD command queue */
        CD_UpdateAndProcessQueue();
do_vsync:
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
            g_cdSystem.commandQueue.items[queueWriteIndex].entry = resourceEntry;

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

/**
 * decomp.me link: https://decomp.me/scratch/Jfb6t
 * decomp.me (%): 87.92%
 */
u32 CD_UpdateAndProcessQueue(void) {
    s32 statusFlags;
    s32 controlResult;
    s32 temp_a0_3;
    s32 queueReadIndex;
    s32 temp_a1_2;
    s32 diskReadyResult;
    s32 temp_v0_5;
    s32 temp_v1;
    s32 var_a2;
    s32 var_v0_2;
    s32 var_v1_2;
    s8 indexDiff;
    s8 var_v0;
    u8 initState;
    u8 retryCounter;
    u8 temp_v0_6;
    u8 var_a0;
    u8* var_a1;
    void* commandItem;
    void* temp_v1_2;
    volatile CdSystem* cdSystem;
    CdCommandQueueItem* queueItem;
    s32 indexDiff2;

    statusFlags = g_cdSystem.statusFlags.word;
    var_v0 = 0;
    
    if (!(statusFlags & 8)) {
        
        initState = 1U;
        
        if (statusFlags & 7) {

            
            indexDiff2 = (g_cdSystem.queueWriteIndex - g_cdSystem.queueReadIndex);
            
            g_cdSystem.playbackFlag = indexDiff2 & 0xF;

            
            
            if (g_cdSystem.initState == 0) {
                
                g_cdSystem.initState = initState;

                
                if (g_cdSystem.playbackFlag != 0) {

                    queueItem = &g_cdSystem.commandQueue.items[g_cdSystem.queueReadIndex];
                    
                    g_cdSystem.currentResourceIndex = (u16) queueItem->resourceIndex;
                    g_cdSystem.targetDataSize = (s32) g_cdSystem.size;
                    g_cdSystem.currentDataSize = (s32) (queueItem->entry)->dataSize;
                }
                
                if (g_cdSystem.audioEnabled != 0) {
                    cdSystem = (void* )0x801ED800;
                    if (g_cdAudioReady != 0) {
                        FUN_80022400(3);
                        goto block_8;
                    }
                } else {
block_8:
                    cdSystem = (CdSystem* )0x801ED800;
                }
                if (cdSystem->loopCounter != 0) {
                    cdSystem->playbackState = 1;
                } else {
                    cdSystem->playbackState = 0;
                }
                g_cdStatusByte3 = 0;
            }
            
            if (VSync(-1) >= ((s32)g_cdSystem.vsyncTimestamp + 0x1E)) {

                
                if (g_cdSystem.initState != 8) {
                    g_cdSystem.vsyncTimestamp = VSync(-1);
                }
                
                controlResult = CdControlB(1U, 0, (u8* )0x801ED960);
                if (!(g_cdSystem.statusByte & 0x10)) {
                    if (controlResult != 0) {
                        initState = g_cdSystem.initState;
                        switch (initState) {        /* switch 1 */
                        case 1:                     /* switch 1 */
                            g_cdSystem.initState = 2U;
                            g_cdSystem.statusFlags.word = (s32) ((g_cdSystem.statusFlags.word & ~1) | 6);
                            /* fallthrough */
                        case 2:                     /* switch 1 */
                            temp_a0_3 = CdControlB(0x13U, 0, (u8* )0x801ED960);
                            if ((g_cdSystem.statusByte & 2) && (temp_a0_3 != 0)) {
                                g_cdSystem.initState = 3U;
                                g_cdSystem.retryCounter = 0U;
                            }
                            break;
                        case 3:                     /* switch 1 */
                            if (CdDiskReady(1) == 2) {
                                g_initState = 4;
                            } else {
                                retryCounter = g_cdSystem.retryCounter;
                                g_cdSystem.retryCounter = (u8) (retryCounter + 2);
                                if ((u32) ((retryCounter + 1) & 0xFF) >= 0xDU) {
                                    g_cdSystem.initState = 4U;
                                }
                            }
                            break;
                        case 4:                     /* switch 1 */
                            diskReadyResult = CdDiskReady(0);
                            if (diskReadyResult != 2) {
                                if (diskReadyResult == 0x10) {
                                    g_initState = 1;
                                } else {
                                    goto block_32;
                                }
                            } else {
block_32:
                                g_initState = 5;
                            }
                            break;
                        case 5:                     /* switch 1 */
                            temp_v0_5 = CdGetDiskType();
                            switch (temp_v0_5) {    /* switch 2; irregular */
                            case 0:                 /* switch 2 */
                                g_cdSystem.initState = 0x20U;
                                var_v0_2 = g_cdSystem.statusFlags.word;
                                var_v1_2 = -3;
                                goto block_63;
                            case 1:                 /* switch 2 */
                                CdDiskReady(0);
                                CdGetDiskType();
                                /* fallthrough */
                            case 2:                 /* switch 2 */
                                g_cdSystem.initState = 6U;
                                g_cdSystem.vsyncTimestamp = (s32) (g_cdSystem.vsyncTimestamp - 0x1E);
                                break;
                            }
                            break;
                        case 6:                     /* switch 1 */
                            g_cdSystem.modeParams = 0xA0;
                            g_cdSystem.u_155 = 0;
                            g_cdSystem.u_156 = 0;
                            g_cdSystem.u_157 = 0;
                            CdSyncCallback(CD_SyncCallback_Handler);
                            CdReadyCallback(0);
                            g_cdSystem.initCommand = 0x20U;
                            CdControlF(0xEU, (u8* )0x801ED954);
                            g_cdSystem.vsyncTimestamp = (s32) (g_cdSystem.vsyncTimestamp - 0x1A);
                            break;
                        case 7:                     /* switch 1 */
                            g_cdSystem.readParams = (s32) g_cdResource176;
                            g_cdSystem.statusFlags.word = (s32) (g_cdSystem.statusFlags.word | 0x10);
                            CdSyncCallback(CD_SyncCallback_Handler);
                            CdReadyCallback((void (*)(u8, u8*)) FUN_80013d74);
                            g_cdSystem.initCommand = 0x21U;
                            g_cdSystem.initState = 8U;
                            CdControlF(6U, (u8* )0x801ED95C);
                            g_cdSystem.vsyncTimestamp = (s32) (g_cdSystem.vsyncTimestamp - 0x1E);
                            break;
                        case 8:                     /* switch 1 */

                            cdSystem = &g_cdSystem;
                            if (cdSystem->syncComplete == 1) {
                                cdSystem->vsyncTimestamp = VSync(-1);
                                cdSystem->syncComplete = 0U;
                            } else if (VSync(-1) >= ((s32)g_cdSystem.vsyncTimestamp + 0x10E)) {
                                temp_v1 = g_cdSystem.initCommand & 0xFF;
                                switch (temp_v1) {  /* switch 3; irregular */
                                default:            /* switch 3 */
                                    CdSyncCallback(CD_SyncCallback_Handler);
                                    CdReadyCallback((void (*)(u8, u8*)) FUN_80013d74);
                                    g_cdSystem.initCommand = 0x21U;
                                    var_a0 = 6;
                                    var_a1 = (u8* )0x801ED95C;
                                    break;
                                case 34:            /* switch 3 */
                                    CdSyncCallback(CD_SyncCallback_Handler);
                                    var_a0 = 9;
                                    var_a1 = 0;
                                    break;
                                case 35:            /* switch 3 */
                                    CdSyncCallback(CD_SyncCallback_Handler);
                                    var_a0 = 0xE;
                                    var_a1 = (u8* )0x801ED950;
                                    break;
                                }
                                CdControlF(var_a0, var_a1);
                                g_cdSystem.vsyncTimestamp = (s32) (g_cdSystem.vsyncTimestamp - 0x1E);
                            }
                            break;
                        case 32:                    /* switch 1 */
                            do {

                            } while (CdControlB(8U, 0, 0) == 0);
                            g_initState = 0x21;
                            break;
                        }
                    } else {
                        goto block_58;
                    }
                } else {
block_58:
                    if ((u8) g_initState >= 6U) {
                        g_cdSystem.statusFlags.word = (s32) (g_cdSystem.statusFlags.word & ~0x10);
                        CdSyncCallback(0);
                        CdReadyCallback(0);
                        do {

                        } while (CdControlB(9U, 0, 0) == 0);
                        g_cdSystem.initCommand = 0U;
                    }
                    g_cdSystem.initState = 1U;
                    var_v0_2 = (g_cdSystem.statusFlags.word | 1) & ~2;
                    var_v1_2 = -5;
block_63:
                    g_cdSystem.statusFlags.word = (s32) (var_v0_2 & var_v1_2);
                }
            }
        } else {
            var_a2 = 0;
            if ((g_cdSystem.currentCommand != 0) || (g_cdSystem.initCommand != 0)) {
                do {
                    if (g_cdSystem.syncComplete == 1) {
                        var_a2 = 1;
                        g_cdSystem.syncComplete = 0U;
                    }
                    temp_a1_2 = g_cdSystem.queueReadIndex;
                    indexDiff = (g_cdSystem.queueWriteIndex - temp_a1_2) & 0xF;
                    if (indexDiff != 0) {
                        g_cdSystem.currentResourceIndex= (u16) g_cdSystem.commandQueue.items[g_cdSystem.queueReadIndex].resourceIndex;
                        g_cdSystem.targetDataSize = (s32) g_cdSystem.size;
                        g_cdSystem.currentDataSize = (s32) (g_cdSystem.commandQueue.items[g_cdSystem.queueReadIndex].entry)->dataSize;
                    }
                } while (g_cdSystem.syncComplete != 0);
                if (var_a2 == 0) {
                    if (VSync(-1) >= (g_cdSystem.vsyncTimestamp + 0xF0)) {
                        if (g_cdSystem.initCommand == 0) {
                            g_cdSystem.currentCommand = 1U;
                            if (g_cdSystem.loopCounter != 0) {
                                g_cdSystem.playbackState = 1;
                            } else {
                                g_cdSystem.playbackState = 0;
                            }
                            CdSyncCallback((void (*)(u8, u8*)) CD_SyncCallback_Handler2);
                            CdReadyCallback(0);
                            do {

                            } while (CdControlB(1U, 0, (u8* )0x801ED960) == 0);
                        } else {
                            CdSyncCallback(CD_SyncCallback_Handler);
                            CdReadyCallback(0);
                            do {

                            } while (CdControlB(1U, 0, (u8* )0x801ED960) == 0);
                        }
                        goto block_83;
                    }
                } else {
block_83:
                    g_cdVSyncTimestamp = VSync(-1);
                }
                g_playbackFlag = indexDiff;
            } else if (g_cdSystem.queueReadIndex != g_cdSystem.queueWriteIndex) {
                g_cdSystem.vsyncTimestamp = VSync(-1);
                g_cdSystem.currentCommand = 1U;
                g_cdSystem.statusFlags.word = (s32) (g_cdSystem.statusFlags.word | 0x10);
                if (g_cdSystem.loopCounter != 0) {
                    g_cdSystem.playbackState = 1;
                } else {
                    g_cdSystem.playbackState = 0;
                }
                CdSyncCallback((void (*)(u8, u8*)) CD_SyncCallback_Handler2);
                CdReadyCallback(0);
                CdSync(0, 0);
                CdControlF(1U, 0);
                indexDiff2 = (g_cdSystem.queueWriteIndex - g_cdSystem.queueReadIndex);
                indexDiff = indexDiff2 & 0xF;
            } else {
                g_cdSystem.loopCounter = 0;
                g_cdSystem.playbackState = 0;
                if (!(statusFlags & 0x20)) {
                    indexDiff = 0;
                    if (VSync(-1) >= (g_cdSystem.vsyncTimestamp + 0x1E)) {
                        if (CdControlB(1U, 0, (u8* )0x801ED960) != 0) {
                            if (g_cdSystem.statusByte & 0x10) {
                                CD_HandleSyncError();
                            }
                            g_cdSystem.syncComplete = 0U;
                            g_cdSystem.retryCounter = 0U;
                            g_cdSystem.vsyncTimestamp = VSync(-1);
                        } else {
                            temp_v0_6 = g_cdSystem.retryCounter;
                            g_cdSystem.retryCounter = (u8) (temp_v0_6 + 1);
                            if ((u32) (temp_v0_6 & 0xFF) >= 0xBU) {
                                CD_HandleSyncError();
                            }
                        }
                        goto block_98;
                    }
                } else {
block_98:
                    indexDiff = 0;
                }
                g_playbackFlag = 0;
            }
        }
        var_v0 = indexDiff;
        if (g_cdAudioEnabled != 0) {
            FUN_80140d48();
            var_v0 = indexDiff;
        }
    }
    return (u32) var_v0;
}


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

/**
 * decomp.me link: https://decomp.me/scratch/byGEu
 * decomp.me (%): 96.59%
 */
void CD_ExecuteCommand(u8 command, void* sectorBuffer, s32 executionMode) 
{
    u8* paramBufferSpecialCmd;
    s32 cmdId;
    s32 nextReadIndex;
    s32 dataSize;
    s32 controlParam;
    s32 commandCheck;
    s32* queuedLocation;
    u8 actualCommand;
    void (*callbackHandler)(u8, u8*);
    void* queueEntryPtr;
    void* queueBufferPtr;
    CdSystem *cdSystem;

    actualCommand = command;
    queuedLocation = 0;

    // Handle SeekL command specially - skip past any queued SeekL commands
    if ((actualCommand & 0xFF) == CdlSeekL) {

        while (1) {
            // Calculate next read index with circular buffer wrapping
            nextReadIndex = (g_cdSystem.queueReadIndex + 1) & 0xF;

            // Wait if buffer is full (write index == next read index)
            if (g_cdSystem.queueWriteIndex == nextReadIndex) {
                continue;
            }

            // Advance read index and get next command
            g_cdSystem.queueReadIndex = nextReadIndex;
            actualCommand = g_cdSystem.commandQueue.items[nextReadIndex].command;

            // Skip if it's another SeekL command
            if (actualCommand == CdlSeekL) {
                continue;
            }

            break;
        }

        if ((actualCommand & 0xFF) == CdlSeekL) {
            goto reset_playback_state;
        }
    }
    
    cmdId = actualCommand & 0xFF;

    // Handle Read/Play commands
    if ((cmdId == CdlReadN) || (cmdId == CdlReadS)) {
        if (cmdId != CdlSeekL) {
            if (g_playbackState == 0) {
                goto reset_playback_state;
            }
        } else {
reset_playback_state:
            // Reset playback state and get queue location
            queuedLocation = (s32*)g_cdSystem.queueReadIndex;
            g_cdSystem.loopCounter = 0;
            g_cdSystem.playbackState = 0;
            
            queuedLocation = g_cdSystem.commandQueue.items[(u_int)queuedLocation].entry;
            g_cdSystem.commandParamBuffer = (s32) *queuedLocation;
        }

        // Handle different execution modes
        switch (executionMode) {
            case 1:
                g_cdSystem.currentCommand = actualCommand;
                CdControlF(actualCommand & 0xFF, 0x801ED958);
    
                while (1) {
                    if (CdGetSector(sectorBuffer, (u32) (g_size + 3) >> 2) != 0) {
                        break;
                    }
                }
                
                commandCheck = actualCommand & 0xFF;
                break;

            case 2:
                commandCheck = actualCommand & 0xFF;
                
                if (executionMode == 2) {
    
                    while (1) {
                        if (CdGetSector(sectorBuffer, (u32) (g_size + 3) >> 2) != 0) {
                            break;
                        }
                    }
                        
                    CdSync(0, 0);
                    commandCheck = actualCommand & 0xFF;
                }
                
                break;
        }
        
        if ((commandCheck == CdlReadN) || (commandCheck == CdlReadS)) {
            
            queueEntryPtr = (g_cdSystem.queueReadIndex * 0x10) + 0x801ED800;
            
            if (( *((u32*)queueEntryPtr + 0x13) == 0) && (g_cdSystem.dstBuffer2 == *((u32*)queueEntryPtr + 0x12) )) {
                g_cdSystem.playbackState = 0;
            }
            cdSystem = &g_cdSystem;
            if (g_playbackState == 0) {
                dataSize = *((s32*)queuedLocation + 1);
                queueBufferPtr = (void*)((cdSystem->queueReadIndex * 0x10) + 0x801ED800);
                g_cdSystem.sizeCopy = dataSize;
                g_cdSystem.size = dataSize;
                g_cdSystem.dstBuffer2 = (s32) *((u32*)queueBufferPtr + 0x12);
                g_cdSystem.loopCounter = (s32) *((u32*)queueBufferPtr + 0x13);
            }
            if (executionMode == 0) {
                g_cdSystem.statusFlags.bytes.b2 = 0;
                callbackHandler = CD_ReadyCallback;
                goto set_callback;
            }
            goto after_callback;
        }
        
        if (executionMode == 1) {
            callbackHandler = 0;
set_callback:
            CdReadyCallback(callbackHandler);
after_callback:
            if (executionMode != 1) {
                goto continue_execution;
            }
        } else {
continue_execution:
            g_cdSystem.currentCommand = actualCommand;
            CdControlF(actualCommand & 0xFF, 0x801ED958);
        }
        g_playbackState = 0;
        return;
    }

    // Handle other commands based on execution mode
    switch (executionMode) {
        case 0:
            g_cdSystem.currentCommand = actualCommand;
            
            if (cmdId == 0xE) {
                controlParam = 0xE;
                paramBufferSpecialCmd = 0x801ED950;
            } else {
                controlParam = cmdId;
                paramBufferSpecialCmd = 0;
            }
            break;
        case 1:
            CdReadyCallback(0);
            g_cdSystem.currentCommand = actualCommand;
            CdControlF(cmdId, 0);

             // Wait for sector read
            while (1) {
                if (CdGetSector(sectorBuffer, (u32) (g_size + 3) >> 2) != 0) {
                    break;
                }
            }
            return;
        case 2:
            // Wait for sector read first
            while(1) {
                if (CdGetSector(sectorBuffer, (u32) (g_size + 3) >> 2) != 0) {
                    break;
                }
            }
            g_cdSystem.currentCommand = actualCommand;
            controlParam = actualCommand & 0xFF;
            paramBufferSpecialCmd = 0;
            break;
        default:
            return;
    }

    CdControlF(controlParam, paramBufferSpecialCmd);
}

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

/**
 * Resets CD subsystem to idle state and stops any ongoing audio playback
 * 
 * Params:
 *  None
 *
 * Returns:
 *  void
 * 
 * decomp.me link: https://decomp.me/scratch/fnucZ
 * decomp.me (%): 100%
 */
void CD_ResetSystem(void)
{
    u32 value;
    u32* callback;
    volatile u32* ptr = (u32*)0x801ED500;
    
    callback = (u32*)ptr[0x0E];
    DecDCToutCallback(callback);

    callback = (u32*)ptr[0x0F];
    DrawSyncCallback(callback);
    
    CdSyncCallback(0);
    CdReadyCallback(0);
    
    do {
        value = CdControlB(9U, 0, 0);
    } while (value == 0);
    
    if (g_cdAudioReady != 0) {
        FUN_80023010();
    }
    
    g_cdSystem.audioEnabled = 0;
    g_cdSystem.currentCommand = 0;
    g_cdSystem.initCommand = 0;
    g_cdSystem.queueReadIndex = 0;
    g_cdSystem.queueWriteIndex = 0;
    g_cdSystem.retryCounter = 0;
    g_cdSystem.playbackState = 0;
    g_cdSystem.loopCounter = 0;
    
    g_cdSystem.statusFlags.word = (s32) (g_cdSystem.statusFlags.word & ~0x10);
    g_cdSystem.vsyncTimestamp = VSync(-1);
}

/**
 * Checks if a CD command with the specified resource index can be queued without duplication
 * 
 * decomp.me link: https://decomp.me/scratch/rGrTJ
 * decomp.me (%): 98.89%
 */
s32 CD_CanQueueResourceIndex(s32 arg0) {
    
    s32 writeIndex;
    s32 index;
    
    index = g_cdSystem.queueReadIndex;
    writeIndex = g_cdSystem.queueWriteIndex;
    
    index = ((- index + writeIndex ) & 0x0F);
    index -= 1;
    
    if (index != -1) {

        while (1) {

            if (g_cdSystem.commandQueue.items[writeIndex].resourceIndex == (arg0 & 0xFFFF)) {
                return 0;
            }

            writeIndex = (writeIndex & 0xF);
            writeIndex += 1;
            index -= 1;
            
            if (index == -1) {
                break;
            }
        }
    }

    return 1;
}


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