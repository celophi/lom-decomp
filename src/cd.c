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
void CD_Initialize()
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
    
    statusFlagsPtr = &CD_SYSTEM.statusFlags;
    
    queueCount = CD_COMMAND_QUEUE_SIZE - 1;
    scratchpadAddr = (CdResourceEntry*)g_scratchpad;
    
    queueEndMarker = -1;
    
    // g_commandQueueOffset is commandQueue.items[11]; the loop uses queueItem[4]
    // to walk items[15] down to items[0] (all 16 entries).
    queueItem = &g_commandQueueOffset;
    
    // 0xFFFE = invalid/no resource loaded
    CD_SYSTEM.resourceIndex = CD_RESOURCE_INDEX_INVALID;
    
    // Reset all runtime state to zero
    CD_SYSTEM.audioEnabled = 0;
    CD_SYSTEM.playbackState = 0;
    CD_SYSTEM.loopCounter = 0;
    CD_SYSTEM.playbackFlag = 0;
    CD_SYSTEM.currentResourceIndex = 0;
    CD_SYSTEM.currentDataSize = 0;
    CD_SYSTEM.targetDataSize = 0;
    CD_SYSTEM.syncComplete = 0;
    CD_SYSTEM.initState = 0;
    CD_SYSTEM.currentCommand = 0;
    CD_SYSTEM.initCommand = 0;
    CD_SYSTEM.retryCount = 0;
    CD_SYSTEM.retryCounter = 0;
    CD_SYSTEM.lastCommand = 0;
    CD_SYSTEM.dstBuffer = 0;
    CD_SYSTEM.callback = 0;
    CD_SYSTEM.queueReadIndex = 0;
    CD_SYSTEM.queueWriteIndex = 0;
    
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
    
    CD_SYSTEM.setModeBuffer = (CdlModeSpeed | CdlModeSize1);
    CD_SYSTEM.u_151 = 0;
    CD_SYSTEM.u_152 = 0;
    CD_SYSTEM.u_153 = 0;
    
    // CdlNop (1) — read current drive status into statusByte
    while (TRUE) {
        cdResult = CdControlB(CdlNop, NULL, &CD_SYSTEM.statusByte);
        
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
        cdResult = CdControlB(CdlSetmode, &CD_SYSTEM.setModeBuffer, NULL);
        
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
    
    cdSystem = &CD_SYSTEM;
    
    if (g_cdAudioEnabled != 0) {
        CD_ResetSystem();
    }
    
    cdSystem->statusFlags.word &= 0xFFFFFFBF;
    
    CdSyncCallback(NULL);
    CdReadyCallback(NULL);

    while(TRUE) {
        cdResult = CdControlB(CdlPause, NULL, NULL);
        if (cdResult != 0) {
            break;
        }
    }
    
    CD_SYSTEM.resourceIndex = CD_RESOURCE_INDEX_INVALID;
    CD_SYSTEM.playbackFlag = 0;
    CD_SYSTEM.currentResourceIndex = 0;
    CD_SYSTEM.currentDataSize = 0;
    CD_SYSTEM.targetDataSize = 0;
    CD_SYSTEM.playbackState = 0;
    CD_SYSTEM.loopCounter = 0;
    CD_SYSTEM.currentCommand = 0;
    CD_SYSTEM.initCommand = 0;
    CD_SYSTEM.retryCount = 0;
    CD_SYSTEM.retryCounter = 0;
    CD_SYSTEM.lastCommand = 0;
    CD_SYSTEM.dstBuffer = 0;
    CD_SYSTEM.callback = 0;
    CD_SYSTEM.statusFlags.word &= 0xFFFFFFEF;
    CD_SYSTEM.vsyncTimestamp = VSync(-1);
    CD_SYSTEM.statusFlags.bytes.b1 = 0;
    CD_SYSTEM.statusFlags.bytes.b2 = 0;
    CD_SYSTEM.queueReadIndex = 0;
    CD_SYSTEM.queueWriteIndex = 0;
    
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
    remainingDataSize = CD_QueueCommand(CdlReadN, command, NULL, &FUN_80014888) - 1;
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
 * @brief Enqueues a CD-ROM command into the circular command queue
 *
 * Validates and inserts a command into the 16-entry circular command queue.
 * If the system is idle and no error/init flags are active, immediately
 * starts command execution by issuing CdlNop to kick off the state machine.
 *
 * @details
 * The function performs several layers of validation before enqueueing:
 *
 * 1. Rejects the command immediately if the "playing" status flag (bit 6) is set
 * 2. Resolves the resource index to a CdResourceEntry pointer:
 *    - 0xFFFF (CD_RESOURCE_INDEX_DEFAULT) maps to g_defaultCdResource
 *    - All other indices index into g_cdResourceArray
 * 3. Deduplicates: if the system is already processing a command and the
 *    new command matches the last-enqueued (command, resourceIndex, dstBuffer,
 *    callback), the enqueue is skipped and the existing dataSize is returned
 * 4. Validates the resource entry has a non-zero disc location and data size
 * 5. Checks the circular queue is not full ((writeIndex + 1) & 0xF != readIndex)
 *
 * Once enqueued, if no command is currently active and no low-nibble status
 * flags (bits 0-3) are set, the function bootstraps execution:
 * - Sets currentCommand to 1, marks the "busy" flag (bit 4)
 * - Installs CD_SyncCallback_Handler2 and sends CdlNop to begin processing
 *
 * @param command        CD-ROM command byte (e.g., CdlReadN, CdlSeekL)
 * @param resourceIndex  Index into g_cdResourceArray, or 0xFFFF for the default resource
 * @param dstBuffer      Destination buffer for read data (may be NULL for non-read commands)
 * @param callback       Callback function pointer invoked on command completion
 *
 * @return The resource's dataSize on success, or a negative error code:
 *         -3 if the system is in "playing" state (bit 6 set)
 *         -2 if the resource entry has no valid location or zero data size
 *         -1 if the command queue is full
 *
 * @note
 * - The separate re-reads of queueWriteIndex for each field store match the
 *   original assembly's volatile access pattern and must not be optimized
 * - The "last command" cache (lastCommand, resourceIndex, dstBuffer, callback)
 *   enables the deduplication check on subsequent calls
 * - When the system is already busy (currentCommand or initCommand != 0),
 *   the command is silently queued without starting execution
 *
 * @warning
 * - Not interrupt-safe; must not be called from within a CD callback
 * - The caller must ensure resourceIndex is valid or 0xFFFF
 *
 * @see CD_UpdateAndProcessQueue — drains the queue each frame
 * @see CD_SyncCallback_Handler2 — installed as sync callback when execution starts
 * @see decomp.me: (97.54%) https://decomp.me/scratch/tWHW2
 */
s32 CD_QueueCommand(u8 command, u16 resourceIndex, CdResourceEntry* dstBuffer, s32 callback) {
    s32 timestamp;
    s32 writeIndex;
    s32 writeIndex2;
    s32 statusFlags;
    s32 dataSize;
    u8 activeCommand;
    CdResourceEntry* resourceEntry;
    volatile CdSystem* cdSystem;
    
    // Reject immediately if the "playing" flag (bit 6) is set
    if (g_cdSystem.statusFlags.word & 0x40) {
        return -3;
    }
    
    // Resolve resource index to entry pointer
    if (resourceIndex == CD_RESOURCE_INDEX_DEFAULT) {
        resourceEntry = &g_defaultCdResource;
    } else {
        resourceEntry = &g_cdResourceArray[resourceIndex];
    }

    cdSystem = &CD_SYSTEM;

    // Deduplicate: skip enqueue if system is busy AND the command matches
    // the previously enqueued one exactly (same command, resource, buffer, callback)
    if ((cdSystem->currentCommand == 0 && cdSystem->initCommand == 0) || 
        (
            (CD_SYSTEM.lastCommand != command) || 
            (CD_SYSTEM.resourceIndex != resourceIndex) || 
            (CD_SYSTEM.dstBuffer != dstBuffer) || 
            (CD_SYSTEM.callback != callback))
       ) {
        
        // Validate resource entry has a valid disc location and non-zero size
        if ((*(u32*)&resourceEntry->location == 0) || (resourceEntry->dataSize == 0)) {
            return -2;
        }
        
        // Check circular queue is not full
        writeIndex = CD_SYSTEM.queueWriteIndex;
        
        if (CD_SYSTEM.queueReadIndex == ((writeIndex + 1) & 0xF)) {
            return -1;
        }

        // Write command fields into the queue entry at the current write index.
        // Each field re-reads queueWriteIndex to match the original volatile access pattern.
        writeIndex = CD_SYSTEM.queueWriteIndex;
        CD_SYSTEM.commandQueue.items[writeIndex].command = command;
        CD_SYSTEM.lastCommand = command;

        writeIndex2 = CD_SYSTEM.queueWriteIndex;
        CD_SYSTEM.commandQueue.items[writeIndex2].resourceIndex = resourceIndex;
        CD_SYSTEM.resourceIndex = resourceIndex;

        writeIndex2 = CD_SYSTEM.queueWriteIndex;
        CD_SYSTEM.commandQueue.items[writeIndex2].entry = resourceEntry;

        writeIndex2 = CD_SYSTEM.queueWriteIndex;
        CD_SYSTEM.commandQueue.items[writeIndex2].dstBuffer = dstBuffer;

        CD_SYSTEM.dstBuffer = dstBuffer;
        
        CD_SYSTEM.commandQueue.items[CD_SYSTEM.queueWriteIndex].callback = callback;
        CD_SYSTEM.callback = callback;
        
        // Advance write index with circular wrap (mod 16)
        CD_SYSTEM.queueWriteIndex = ((CD_SYSTEM.queueWriteIndex + 1) & 0xF);
        
        timestamp = VSync(-1);

        // If a command is already in progress, just queue and return
        activeCommand = CD_SYSTEM.currentCommand;
        
        if ((activeCommand != 0) || (CD_SYSTEM.initCommand != 0)) {
            return resourceEntry->dataSize;
        }

        statusFlags = CD_SYSTEM.statusFlags.word;
        
        // If no error/init flags (bits 0-3) are active, bootstrap execution
        if (!(statusFlags & 0xF)) {
            
            CD_SYSTEM.vsyncTimestamp = timestamp;
            CD_SYSTEM.playbackFlag = 1;
            CD_SYSTEM.currentResourceIndex = resourceIndex;
            dataSize = resourceEntry->dataSize;
            CD_SYSTEM.currentCommand = 1;
            CD_SYSTEM.statusFlags.word = (statusFlags | 0x10);
            CD_SYSTEM.playbackState  = 0;
            CD_SYSTEM.loopCounter = 0;
            CD_SYSTEM.targetDataSize = dataSize;
            CD_SYSTEM.currentDataSize = dataSize;
            
            // Install sync callback and send CdlNop to kick off the state machine
            CdSyncCallback(&CD_SyncCallback_Handler2);
            CdSync(0, NULL);
            CdControlF(CdlNop, NULL);
        }
    }

    return resourceEntry->dataSize;
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

    statusFlags = CD_SYSTEM.statusFlags.word;
    var_v0 = 0;
    
    if (!(statusFlags & 8)) {
        
        initState = 1U;
        
        if (statusFlags & 7) {

            
            indexDiff2 = (CD_SYSTEM.queueWriteIndex - CD_SYSTEM.queueReadIndex);
            
            CD_SYSTEM.playbackFlag = indexDiff2 & 0xF;

            
            
            if (CD_SYSTEM.initState == 0) {
                
                CD_SYSTEM.initState = initState;

                
                if (CD_SYSTEM.playbackFlag != 0) {

                    queueItem = &CD_SYSTEM.commandQueue.items[CD_SYSTEM.queueReadIndex];
                    
                    CD_SYSTEM.currentResourceIndex = (u16) queueItem->resourceIndex;
                    CD_SYSTEM.targetDataSize = (s32) CD_SYSTEM.size;
                    CD_SYSTEM.currentDataSize = (s32) (queueItem->entry)->dataSize;
                }
                
                if (CD_SYSTEM.audioEnabled != 0) {
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
            
            if (VSync(-1) >= ((s32)CD_SYSTEM.vsyncTimestamp + 0x1E)) {

                
                if (CD_SYSTEM.initState != 8) {
                    CD_SYSTEM.vsyncTimestamp = VSync(-1);
                }
                
                controlResult = CdControlB(1U, 0, (u8* )0x801ED960);
                if (!(CD_SYSTEM.statusByte & 0x10)) {
                    if (controlResult != 0) {
                        initState = CD_SYSTEM.initState;
                        switch (initState) {        /* switch 1 */
                        case 1:                     /* switch 1 */
                            CD_SYSTEM.initState = 2U;
                            CD_SYSTEM.statusFlags.word = (s32) ((CD_SYSTEM.statusFlags.word & ~1) | 6);
                            /* fallthrough */
                        case 2:                     /* switch 1 */
                            temp_a0_3 = CdControlB(0x13U, 0, (u8* )0x801ED960);
                            if ((CD_SYSTEM.statusByte & 2) && (temp_a0_3 != 0)) {
                                CD_SYSTEM.initState = 3U;
                                CD_SYSTEM.retryCounter = 0U;
                            }
                            break;
                        case 3:                     /* switch 1 */
                            if (CdDiskReady(1) == 2) {
                                g_initState = 4;
                            } else {
                                retryCounter = CD_SYSTEM.retryCounter;
                                CD_SYSTEM.retryCounter = (u8) (retryCounter + 2);
                                if ((u32) ((retryCounter + 1) & 0xFF) >= 0xDU) {
                                    CD_SYSTEM.initState = 4U;
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
                                CD_SYSTEM.initState = 0x20U;
                                var_v0_2 = CD_SYSTEM.statusFlags.word;
                                var_v1_2 = -3;
                                goto block_63;
                            case 1:                 /* switch 2 */
                                CdDiskReady(0);
                                CdGetDiskType();
                                /* fallthrough */
                            case 2:                 /* switch 2 */
                                CD_SYSTEM.initState = 6U;
                                CD_SYSTEM.vsyncTimestamp = (s32) (CD_SYSTEM.vsyncTimestamp - 0x1E);
                                break;
                            }
                            break;
                        case 6:                     /* switch 1 */
                            CD_SYSTEM.modeParams = 0xA0;
                            CD_SYSTEM.u_155 = 0;
                            CD_SYSTEM.u_156 = 0;
                            CD_SYSTEM.u_157 = 0;
                            CdSyncCallback(CD_SyncCallback_Handler);
                            CdReadyCallback(0);
                            CD_SYSTEM.initCommand = 0x20U;
                            CdControlF(0xEU, (u8* )0x801ED954);
                            CD_SYSTEM.vsyncTimestamp = (s32) (CD_SYSTEM.vsyncTimestamp - 0x1A);
                            break;
                        case 7:                     /* switch 1 */
                            CD_SYSTEM.readParams = (s32) g_cdResource176;
                            CD_SYSTEM.statusFlags.word = (s32) (CD_SYSTEM.statusFlags.word | 0x10);
                            CdSyncCallback(CD_SyncCallback_Handler);
                            CdReadyCallback((void (*)(u8, u8*)) FUN_80013d74);
                            CD_SYSTEM.initCommand = 0x21U;
                            CD_SYSTEM.initState = 8U;
                            CdControlF(6U, (u8* )0x801ED95C);
                            CD_SYSTEM.vsyncTimestamp = (s32) (CD_SYSTEM.vsyncTimestamp - 0x1E);
                            break;
                        case 8:                     /* switch 1 */

                            cdSystem = &CD_SYSTEM;
                            if (cdSystem->syncComplete == 1) {
                                cdSystem->vsyncTimestamp = VSync(-1);
                                cdSystem->syncComplete = 0U;
                            } else if (VSync(-1) >= ((s32)CD_SYSTEM.vsyncTimestamp + 0x10E)) {
                                temp_v1 = CD_SYSTEM.initCommand & 0xFF;
                                switch (temp_v1) {  /* switch 3; irregular */
                                default:            /* switch 3 */
                                    CdSyncCallback(CD_SyncCallback_Handler);
                                    CdReadyCallback((void (*)(u8, u8*)) FUN_80013d74);
                                    CD_SYSTEM.initCommand = 0x21U;
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
                                CD_SYSTEM.vsyncTimestamp = (s32) (CD_SYSTEM.vsyncTimestamp - 0x1E);
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
                        CD_SYSTEM.statusFlags.word = (s32) (CD_SYSTEM.statusFlags.word & ~0x10);
                        CdSyncCallback(0);
                        CdReadyCallback(0);
                        do {

                        } while (CdControlB(9U, 0, 0) == 0);
                        CD_SYSTEM.initCommand = 0U;
                    }
                    CD_SYSTEM.initState = 1U;
                    var_v0_2 = (CD_SYSTEM.statusFlags.word | 1) & ~2;
                    var_v1_2 = -5;
block_63:
                    CD_SYSTEM.statusFlags.word = (s32) (var_v0_2 & var_v1_2);
                }
            }
        } else {
            var_a2 = 0;
            if ((CD_SYSTEM.currentCommand != 0) || (CD_SYSTEM.initCommand != 0)) {
                do {
                    if (CD_SYSTEM.syncComplete == 1) {
                        var_a2 = 1;
                        CD_SYSTEM.syncComplete = 0U;
                    }
                    temp_a1_2 = CD_SYSTEM.queueReadIndex;
                    indexDiff = (CD_SYSTEM.queueWriteIndex - temp_a1_2) & 0xF;
                    if (indexDiff != 0) {
                        CD_SYSTEM.currentResourceIndex= (u16) CD_SYSTEM.commandQueue.items[CD_SYSTEM.queueReadIndex].resourceIndex;
                        CD_SYSTEM.targetDataSize = (s32) CD_SYSTEM.size;
                        CD_SYSTEM.currentDataSize = (s32) (CD_SYSTEM.commandQueue.items[CD_SYSTEM.queueReadIndex].entry)->dataSize;
                    }
                } while (CD_SYSTEM.syncComplete != 0);
                if (var_a2 == 0) {
                    if (VSync(-1) >= (CD_SYSTEM.vsyncTimestamp + 0xF0)) {
                        if (CD_SYSTEM.initCommand == 0) {
                            CD_SYSTEM.currentCommand = 1U;
                            if (CD_SYSTEM.loopCounter != 0) {
                                CD_SYSTEM.playbackState = 1;
                            } else {
                                CD_SYSTEM.playbackState = 0;
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
            } else if (CD_SYSTEM.queueReadIndex != CD_SYSTEM.queueWriteIndex) {
                CD_SYSTEM.vsyncTimestamp = VSync(-1);
                CD_SYSTEM.currentCommand = 1U;
                CD_SYSTEM.statusFlags.word = (s32) (CD_SYSTEM.statusFlags.word | 0x10);
                if (CD_SYSTEM.loopCounter != 0) {
                    CD_SYSTEM.playbackState = 1;
                } else {
                    CD_SYSTEM.playbackState = 0;
                }
                CdSyncCallback((void (*)(u8, u8*)) CD_SyncCallback_Handler2);
                CdReadyCallback(0);
                CdSync(0, 0);
                CdControlF(1U, 0);
                indexDiff2 = (CD_SYSTEM.queueWriteIndex - CD_SYSTEM.queueReadIndex);
                indexDiff = indexDiff2 & 0xF;
            } else {
                CD_SYSTEM.loopCounter = 0;
                CD_SYSTEM.playbackState = 0;
                if (!(statusFlags & 0x20)) {
                    indexDiff = 0;
                    if (VSync(-1) >= (CD_SYSTEM.vsyncTimestamp + 0x1E)) {
                        if (CdControlB(1U, 0, (u8* )0x801ED960) != 0) {
                            if (CD_SYSTEM.statusByte & 0x10) {
                                CD_HandleSyncError();
                            }
                            CD_SYSTEM.syncComplete = 0U;
                            CD_SYSTEM.retryCounter = 0U;
                            CD_SYSTEM.vsyncTimestamp = VSync(-1);
                        } else {
                            temp_v0_6 = CD_SYSTEM.retryCounter;
                            CD_SYSTEM.retryCounter = (u8) (temp_v0_6 + 1);
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

    if ((CD_SYSTEM.statusFlags.word & 8) == 0) {
        return 1;
    }

    switch (CD_SYSTEM.initState) {
    case 0:
        CdFlush();
        CD_SYSTEM.initState = 1;

        CD_SYSTEM.vsyncTimestamp = VSync(-1) + 1;
        goto return_zero;

    case 1:
        timestamp = VSync(-1);

        if (CD_SYSTEM.vsyncTimestamp <= timestamp) {
            CD_SYSTEM.modeParams = 0xa0;
            CD_SYSTEM.u_155 = 0;
            CD_SYSTEM.u_156 = 0;
            CD_SYSTEM.u_157 = 0;

            CdSyncCallback(CD_SyncCallback_Handler);
            CdReadyCallback((CdlCB) 0x0);

            CD_SYSTEM.initCommand = 0x10;

            CdControlF(CdlSetmode, & CD_SYSTEM.modeParams);

            timestamp = VSync(-1);

            CD_SYSTEM.vsyncTimestamp = timestamp + 4;
            goto return_zero;
        }

        return 0;

    case 2:
        CdSyncCallback(CD_SyncCallback_Handler);
        CD_SYSTEM.initCommand = 0x11;
        local_18[0] = 1;
        local_18[1] = 1;
        CdControlF('\r', local_18);
        CD_SYSTEM.initState = 3;
        CD_SYSTEM.vsyncTimestamp = VSync(-1);
        goto return_zero;

    case 3:
        if (CD_SYSTEM.syncComplete == 1) {
            CD_SYSTEM.vsyncTimestamp = VSync(-1);
            CD_SYSTEM.syncComplete = 0;
            goto return_zero;
        }
        timestamp = VSync(-1);
        if (timestamp < CD_SYSTEM.vsyncTimestamp + 0x1e) {
            goto return_zero;
        }
        CdSyncCallback(CD_SyncCallback_Handler);
        if (CD_SYSTEM.initCommand != 0x11) {
            do {
                if (CD_SYSTEM.initCommand < 0x12) {
                    goto do_setfilter;
                }
                if (CD_SYSTEM.initCommand == 0x12) {
                    CdControlF(0x09, (u_char * ) 0x0);
                    goto LAB_80012d48;
                }
                do_setfilter: local_18[0] = 1;
                local_18[1] = 1;
                CdControlF(0x0d, local_18);
                CD_SYSTEM.initCommand = 0x10;
            } while (0);
        } else {
            CdControlF(0x0c, (u_char * ) 0x0);
        }
        LAB_80012d48:
            CD_SYSTEM.vsyncTimestamp = CD_SYSTEM.vsyncTimestamp + -0x1e;
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
            nextReadIndex = (CD_SYSTEM.queueReadIndex + 1) & 0xF;

            // Wait if buffer is full (write index == next read index)
            if (CD_SYSTEM.queueWriteIndex == nextReadIndex) {
                continue;
            }

            // Advance read index and get next command
            CD_SYSTEM.queueReadIndex = nextReadIndex;
            actualCommand = CD_SYSTEM.commandQueue.items[nextReadIndex].command;

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
            queuedLocation = (s32*)CD_SYSTEM.queueReadIndex;
            CD_SYSTEM.loopCounter = 0;
            CD_SYSTEM.playbackState = 0;
            
            queuedLocation = CD_SYSTEM.commandQueue.items[(u_int)queuedLocation].entry;
            CD_SYSTEM.commandParamBuffer = (s32) *queuedLocation;
        }

        // Handle different execution modes
        switch (executionMode) {
            case 1:
                CD_SYSTEM.currentCommand = actualCommand;
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
            
            queueEntryPtr = (CD_SYSTEM.queueReadIndex * 0x10) + 0x801ED800;
            
            if (( *((u32*)queueEntryPtr + 0x13) == 0) && (CD_SYSTEM.dstBuffer2 == *((u32*)queueEntryPtr + 0x12) )) {
                CD_SYSTEM.playbackState = 0;
            }
            cdSystem = &CD_SYSTEM;
            if (g_playbackState == 0) {
                dataSize = *((s32*)queuedLocation + 1);
                queueBufferPtr = (void*)((cdSystem->queueReadIndex * 0x10) + 0x801ED800);
                CD_SYSTEM.sizeCopy = dataSize;
                CD_SYSTEM.size = dataSize;
                CD_SYSTEM.dstBuffer2 = (s32) *((u32*)queueBufferPtr + 0x12);
                CD_SYSTEM.loopCounter = (s32) *((u32*)queueBufferPtr + 0x13);
            }
            if (executionMode == 0) {
                CD_SYSTEM.statusFlags.bytes.b2 = 0;
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
            CD_SYSTEM.currentCommand = actualCommand;
            CdControlF(actualCommand & 0xFF, 0x801ED958);
        }
        g_playbackState = 0;
        return;
    }

    // Handle other commands based on execution mode
    switch (executionMode) {
        case 0:
            CD_SYSTEM.currentCommand = actualCommand;
            
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
            CD_SYSTEM.currentCommand = actualCommand;
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
            CD_SYSTEM.currentCommand = actualCommand;
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
    
    CD_SYSTEM.initState = 0;
    CD_SYSTEM.statusFlags.word |= 1;    
    CD_SYSTEM.currentCommand = 0;
    CD_SYSTEM.initCommand = 0;
    CD_SYSTEM.retryCount = 0;
    CD_SYSTEM.retryCounter = 0;
    CD_SYSTEM.statusFlags.word &= ~0x10;
    CD_SYSTEM.vsyncTimestamp = VSync(-1);
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
    
    CD_SYSTEM.audioEnabled = 0;
    CD_SYSTEM.currentCommand = 0;
    CD_SYSTEM.initCommand = 0;
    CD_SYSTEM.queueReadIndex = 0;
    CD_SYSTEM.queueWriteIndex = 0;
    CD_SYSTEM.retryCounter = 0;
    CD_SYSTEM.playbackState = 0;
    CD_SYSTEM.loopCounter = 0;
    
    CD_SYSTEM.statusFlags.word = (s32) (CD_SYSTEM.statusFlags.word & ~0x10);
    CD_SYSTEM.vsyncTimestamp = VSync(-1);
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
    
    index = CD_SYSTEM.queueReadIndex;
    writeIndex = CD_SYSTEM.queueWriteIndex;
    
    index = ((- index + writeIndex ) & 0x0F);
    index -= 1;
    
    if (index != -1) {

        while (1) {

            if (CD_SYSTEM.commandQueue.items[writeIndex].resourceIndex == (arg0 & 0xFFFF)) {
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
    
    cdStruct = &CD_SYSTEM;
    location = &cdStruct->defaultCdResource.location;
    *(u_int*)&cdStruct->defaultCdResource.location = 0;
    cdStruct->defaultCdResource.dataSize = dataSizeBytes;
    
    CdIntToPos(lba, location);
    CD_QueueCommand(6, 0xffff, &g_SKCDPOSE_DAT, 0);
    CD_WaitForQueueEmpty();
    CD_SetAudioVolume(128, 1);
}