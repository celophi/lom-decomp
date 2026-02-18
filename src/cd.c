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
 * @see decomp.me: (98.05%) https://decomp.me/scratch/CSYVd
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
                *(volatile u8*)streamState = 1;
                
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
 * - Installs CD_OnCommandComplete and sends CdlNop to begin processing
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
 * @see decomp.me: (100%) https://decomp.me/scratch/izXP3
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
            CdSyncCallback(&CD_OnCommandComplete);
            CdSync(0, NULL);
            CdControlF(CdlNop, NULL);
        }
    }

    return resourceEntry->dataSize;
}

/**
 * @brief Drains the CD command queue and drives the disc-recovery state machine
 *
 * Called once per frame to advance all pending CD-ROM operations. Handles
 * three mutually exclusive execution paths depending on the current state
 * of the CD subsystem, and updates the audio system when enabled.
 *
 * @details
 * The function inspects statusFlags to choose one of three branches:
 *
 * **Branch 1 — Error/init recovery (statusFlags bits 0-2 set):**
 * Runs a multi-state recovery state machine (states 1-8, 32) that attempts
 * to re-initialize the disc drive after an error or shell-open event:
 *   1. Sends CdlNop to poll drive status every 30 VSync frames
 *   2. Progresses through GetStat, DiskReady, DiskType detection
 *   3. Re-applies CdlSetmode (0xA0) and installs sync/ready callbacks
 *   4. Issues CdlReadN to resume reading, with 270-frame timeout retries
 *   5. On persistent errors, pauses the drive and resets to state 1
 *
 * **Branch 2 — Active command execution (currentCommand or initCommand != 0):**
 * Monitors the currently executing command for completion:
 *   - Polls syncComplete flag set by the sync callback
 *   - Updates currentResourceIndex and currentDataSize from the queue head
 *   - On 240-frame timeout, re-installs callbacks and retries via CdlNop
 *   - Records VSync timestamp and remaining queue depth each frame
 *
 * **Branch 3 — Idle with queued commands (queue non-empty, no active command):**
 * Bootstraps execution of the next queued command:
 *   - Sets currentCommand to 1, marks busy flag (bit 4)
 *   - Installs CD_OnCommandComplete and sends CdlNop to start processing
 *   - If the queue is empty, performs periodic 30-frame status polls via CdlNop
 *     and triggers CD_HandleSyncError if the drive reports an error (bit 4)
 *
 * After all branches, calls FUN_80140d48() to update the audio subsystem
 * when g_cdAudioEnabled is set.
 *
 * @param None
 *
 * @return The number of commands remaining in the queue (0 when idle or
 *         when the system is in the recovery state machine with bit 3 set)
 *
 * @note
 * - Returns 0 immediately if statusFlags bit 3 is set (processing deferred
 *   to CD_RecoveryStateMachine)
 * - Raw pointer arithmetic for queue item access is preserved from the
 *   original decompilation to maintain register-level matching
 * - The recovery state machine shares state numbers (initState) and command
 *   codes (initCommand 0x20-0x23) with CD_RecoveryStateMachine but
 *   operates on a different set of transitions
 *
 * @warning
 * - Must be called every frame for correct timeout and retry behavior
 * - Not interrupt-safe; must not be called from within a CD callback
 * - The 30/240/270-frame timeout constants assume NTSC (60 Hz) VSync rate
 *
 * @see decomp.me: (96.81%) https://decomp.me/scratch/Jfb6t
 */
u32 CD_UpdateAndProcessQueue(void) {
    // Status and control variables
    s32 statusFlags;
    s32 controlResult;
    s32 checkDiskResult;
    s32 queueReadIndex;
    s32 queueReadIndex2;
    s32 diskReadyResult;
    s32 diskType;
    s32 temp_v1;

    // Return value and state tracking
    s32 syncCompleteFlag;
    s32 flagsForUpdate;
    s32 flagsMask;
    s32 indexDiff;

    // Initialization state variables
    u8 initState;
    u8 retryCounter;
    u8 currentCommand;

    // Command execution variables
    u8 cdCommand;
    u8* cdCommandParams;
    void* commandItem;
    void* temp_v1_2;

    // System pointer (volatile for hardware access)
    volatile CdSystem* cdSystem;
    volatile CdSystem* cdSystemPtr;

    CdCommandQueueItem* queueItem;
    s32 queueDiff;
    u32 readIndex;

    // Command parsing (for case 8 state machine)
    s32 initCommand;
    u8 initCommandByte;

    // Get current CD system status
    cdSystem = &CD_SYSTEM;
    statusFlags = CD_SYSTEM.statusFlags.word;

    // Check if CD system is busy (bit 3 set)
    if (statusFlags & 8) {
        return 0;
    }

    initState = 1U;

    // Branch 1: Active command processing (bits 0-2 set)
    if (statusFlags & 7) {
        // Calculate queue difference
        readIndex = CD_SYSTEM.queueReadIndex;
        queueDiff = (CD_SYSTEM.queueWriteIndex - readIndex);

        CD_SYSTEM.playbackFlag = queueDiff & 0xF;

        // Initialize queue processing if not already initialized
        if (CD_SYSTEM.initState == 0) {
            CD_SYSTEM.initState = initState;

            // Load current queue item if queue is not empty
            if (CD_SYSTEM.playbackFlag != 0) {
                // Manual pointer arithmetic to access queue item
                // (preserved from decompilation for register matching)
                readIndex = (u32)&CD_SYSTEM + (readIndex << 4);
                CD_SYSTEM.currentResourceIndex = *(u16*) (readIndex + 0x42);
                CD_SYSTEM.currentDataSize = *(s32*)(*((s32*) (readIndex + 0x44)) + 4);
                CD_SYSTEM.targetDataSize = CD_SYSTEM.size;
            }

            // Handle audio playback initialization
            if (CD_SYSTEM.audioEnabled != 0) {
                cdSystemPtr = &CD_SYSTEM;
                if (g_cdAudioReady != 0) {
                    FUN_80022400(3);  // Audio function
                }
            }

            // Set playback state based on loop counter
            if (CD_SYSTEM.loopCounter != 0) {
                CD_SYSTEM.playbackState = 1;
            } else {
                CD_SYSTEM.playbackState = 0;
            }

            g_cdStatusByte3 = 0;
        }

        // Check if enough time has passed (30 VSync frames)
        if (VSync(-1) >= ((s32)CD_SYSTEM.vsyncTimestamp + 30)) {

            // Update timestamp if not in state 8
            if (CD_SYSTEM.initState != 8) {
                CD_SYSTEM.vsyncTimestamp = VSync(-1);
            }

            // Send NOP command to check CD status
            controlResult = CdControlB(CdlNop, 0, (u8*)0x801ED960);

            // Check if CD error bit (0x10) is NOT set
            if (!(CD_SYSTEM.statusByte & 0x10)) {
                cdSystem = &CD_SYSTEM;
                if (controlResult != 0) {
                    // CD-ROM initialization state machine
                    initState = cdSystem->initState;
                    switch (initState) {

                    case 1:  // Initial state - start initialization
                        CD_SYSTEM.initState = 2U;
                        CD_SYSTEM.statusFlags.word = (s32)((CD_SYSTEM.statusFlags.word & ~1) | 6);
                        /* fallthrough */

                    case 2:  // GetStat command
                        checkDiskResult = CdControlB(0x13U, 0, (u8*)0x801ED960);
                        if ((CD_SYSTEM.statusByte & 2) && (checkDiskResult != 0)) {
                            CD_SYSTEM.initState = 3U;
                            CD_SYSTEM.retryCounter = 0U;
                        }
                        break;

                    case 3:  // Wait for disk ready with retries
                        if (CdDiskReady(1) == 2) {
                            g_initState = 4;
                        } else {
                            retryCounter = CD_SYSTEM.retryCounter;
                            CD_SYSTEM.retryCounter = (u8)(retryCounter + 1);
                            if ((u32)((retryCounter + 2) & 0xFF) >= 0xDU) {
                                CD_SYSTEM.initState = 4U;
                            }
                        }
                        break;

                    case 4:  // Check disk status
                        diskReadyResult = CdDiskReady(0);
                        if (diskReadyResult != 2) {
                            if (diskReadyResult == 0x10) {
                                g_initState = 1;  // No disk, restart
                            } else {
                                goto SetInitState5;
                            }
                        } else {
SetInitState5:
                            g_initState = 5;
                        }
                        break;

                    case 5:  // Detect disk type
                        diskType = CdGetDiskType();
                        switch (diskType) {
                        case 0:  // No disk
                            do {
                                CD_SYSTEM.initState = 0x20U;
                            } while (0);
                            flagsForUpdate = CD_SYSTEM.statusFlags.word;
                            flagsMask = -3;
                            goto UpdateStatusFlags;

                        case 1:  // Audio CD (needs verification)
                            CdDiskReady(0);
                            CdGetDiskType();
                            /* fallthrough */

                        case 2:  // Valid CD-ROM
                            CD_SYSTEM.initState = 6U;
                            CD_SYSTEM.vsyncTimestamp = (s32)(CD_SYSTEM.vsyncTimestamp - 0x1E);
                            break;
                        }
                        break;

                    case 6:  // Set CD mode parameters
                        cdSystem = &CD_SYSTEM;
                        cdSystem->modeParams = 0xA0;      // Mode flags
                        cdSystem->u_155 = 0;
                        cdSystem->u_156 = 0;
                        CD_SYSTEM.u_157 = 0;
                        CdSyncCallback(CD_SyncCallback_Handler);
                        CdReadyCallback(0);
                        cdSystem->initCommand = 0x20U;
                        CdControlF(CdlSetmode, (u8*)0x801ED954);
                        CD_SYSTEM.vsyncTimestamp = (s32)(cdSystem->vsyncTimestamp - 0x1A);
                        break;

                    case 7:  // Start reading
                        CD_SYSTEM.readParams = (s32)g_cdResource176;
                        CD_SYSTEM.statusFlags.word = (s32)(CD_SYSTEM.statusFlags.word | 0x10);
                        CdSyncCallback(CD_SyncCallback_Handler);
                        CdReadyCallback((void (*)(u8, u8*))FUN_80013d74);
                        CD_SYSTEM.initCommand = 0x21U;
                        CD_SYSTEM.initState = 8U;
                        CdControlF(CdlReadN, (u8*)0x801ED95C);
                        CD_SYSTEM.vsyncTimestamp = (s32)(CD_SYSTEM.vsyncTimestamp - 0x1E);
                        break;

                    case 8:  // Reading state - handle timeouts
                        cdSystem = &CD_SYSTEM;
                        if (cdSystem->syncComplete == 1) {
                            cdSystem->vsyncTimestamp = VSync(-1);
                            cdSystem->syncComplete = 0U;
                        } else if (VSync(-1) >= ((s32)CD_SYSTEM.vsyncTimestamp + 270)) {
                            // Timeout occurred - check what command to retry
                            initCommandByte = CD_SYSTEM.initCommand;
                            initCommand = initCommandByte & 0xFF;

                            if (initCommand == 0x22) {
                                goto RetryPause;
                            }

                            if (initCommand < 0x23) {
                                goto RetryRead;
                            }

                            if (0x23 == initCommand) {
                                goto RetrySetmode;
                            }

RetryRead:  // Retry read command
                            CdSyncCallback(CD_SyncCallback_Handler);
                            CdReadyCallback((void (*)(u8, u8*))FUN_80013d74);

                            CD_SYSTEM.initCommand = 0x21;
                            cdCommand = CdlReadN;
                            cdCommandParams = (u8*)0x801ED95C;
                            goto ExecuteCommand;

RetryPause:  // Retry pause command
                            CdSyncCallback(CD_SyncCallback_Handler);
                            cdCommand = CdlPause;
                            cdCommandParams = 0;
                            goto ExecuteCommand;

RetrySetmode:  // Retry setmode command
                            CdSyncCallback(CD_SyncCallback_Handler);
                            cdCommand = CdlSetmode;
                            cdCommandParams = (u8*)0x801ED950;

ExecuteCommand:  // Common command execution
                            CdControlF(cdCommand, cdCommandParams);
                            CD_SYSTEM.vsyncTimestamp -= 30;
                        }
                        break;

                    case 32:  // Error recovery - pause
                        do {

                        } while (CdControlB(8U, 0, 0) == 0);
                        g_initState = 0x21;
                        break;
                    }
                } else {
                    goto ErrorRecovery;
                }
            } else {
ErrorRecovery:  // Handle CD errors
                cdSystem = &CD_SYSTEM;
                if ((u8)g_initState >= 6U) {
                    // Clear busy flag and reset callbacks
                    CD_SYSTEM.statusFlags.word = (s32)(CD_SYSTEM.statusFlags.word & ~0x10);
                    CdSyncCallback(0);
                    CdReadyCallback(0);
                    do {

                    } while (CdControlB(CdlPause, 0, 0) == 0);
                    cdSystem = &CD_SYSTEM;
                    cdSystem->initCommand = 0U;
                }
                CD_SYSTEM.initState = 1U;
                flagsForUpdate = (CD_SYSTEM.statusFlags.word | 1) & ~2;

                do {
                    flagsMask = -5;
                } while (0);

UpdateStatusFlags:  // Update status flags with mask
                cdSystem->statusFlags.word = (s32)(flagsForUpdate & flagsMask);
            }
        }

    } else {
        // Branch 2: Idle state - handle queued commands
        syncCompleteFlag = 0;
        currentCommand = CD_SYSTEM.currentCommand;

        if ((currentCommand != 0) || (CD_SYSTEM.initCommand != 0)) {
            // Process sync completion and update queue
            while (1) {
                if (CD_SYSTEM.syncComplete == 1) {
                    syncCompleteFlag = 1;
                    CD_SYSTEM.syncComplete = 0U;
                }
                queueReadIndex2 = CD_SYSTEM.queueReadIndex;
                indexDiff = (CD_SYSTEM.queueWriteIndex - queueReadIndex2) & 0xF;

                if (indexDiff != 0) {
                    CD_SYSTEM.currentResourceIndex = (u16)CD_SYSTEM.commandQueue.items[CD_SYSTEM.queueReadIndex].resourceIndex;
                    CD_SYSTEM.currentDataSize = (s32)(CD_SYSTEM.commandQueue.items[CD_SYSTEM.queueReadIndex].entry)->dataSize;
                    CD_SYSTEM.targetDataSize = (s32)CD_SYSTEM.size;
                }

                if (CD_SYSTEM.syncComplete == 0) {
                    break;
                }
            }

            // Check for command timeout
            if (syncCompleteFlag == 0) {
                if (VSync(-1) >= (s32)(CD_SYSTEM.vsyncTimestamp + 240)) {
                    if (CD_SYSTEM.initCommand == 0) {
                        CD_SYSTEM.currentCommand = 1U;

                        if (CD_SYSTEM.loopCounter != 0) {
                            CD_SYSTEM.playbackState = 1;
                        } else {
                            CD_SYSTEM.playbackState = 0;
                        }

                        CdSyncCallback((void (*)(u8, u8*))CD_OnCommandComplete);
                        CdReadyCallback(0);
                        do {

                        } while (CdControlB(CdlNop, 0, (u8*)0x801ED960) == 0);
                    } else {
                        CdSyncCallback(CD_SyncCallback_Handler);
                        CdReadyCallback(0);
                        do {

                        } while (CdControlB(CdlNop, 0, (u8*)0x801ED960) == 0);
                    }
                }
            }
            
            g_cdVSyncTimestamp = VSync(-1);
            g_playbackFlag = indexDiff;

        } else if (CD_SYSTEM.queueReadIndex != CD_SYSTEM.queueWriteIndex) {
            // Queue has items - start processing
            CD_SYSTEM.vsyncTimestamp = VSync(-1);
            CD_SYSTEM.currentCommand = 1U;
            CD_SYSTEM.statusFlags.word = (s32)(CD_SYSTEM.statusFlags.word | 0x10);

            if (CD_SYSTEM.loopCounter != 0) {
                CD_SYSTEM.playbackState = 1;
            } else {
                CD_SYSTEM.playbackState = 0;
            }

            CdSyncCallback((void (*)(u8, u8*))CD_OnCommandComplete);
            CdReadyCallback(0);
            CdSync(0, 0);
            CdControlF(CdlNop, 0);
            indexDiff = (CD_SYSTEM.queueWriteIndex - CD_SYSTEM.queueReadIndex) & 0xF;

        } else {
            // Queue is empty - idle state
            CD_SYSTEM.loopCounter = 0;
            CD_SYSTEM.playbackState = 0;

            if (!(statusFlags & 0x20)) {
                // Periodic status check
                if (VSync(-1) >= (s32)(CD_SYSTEM.vsyncTimestamp + 30)) {
                    if (CdControlB(CdlNop, 0, (u8*)0x801ED960) != 0) {
                        if (CD_SYSTEM.statusByte & 0x10) {
                            CD_HandleSyncError();
                        }
                        CD_SYSTEM.syncComplete = 0U;
                        CD_SYSTEM.retryCounter = 0U;
                        CD_SYSTEM.vsyncTimestamp = VSync(-1);
                    } else {
                        // Retry counter for failed status checks
                        currentCommand = CD_SYSTEM.retryCounter;
                        CD_SYSTEM.retryCounter = (u8)(currentCommand + 1);
                        if ((u32)(currentCommand & 0xFF) >= 0xBU) {
                            CD_HandleSyncError();
                        }
                    }
                }
            }
            indexDiff = 0;
            g_playbackFlag = 0;
        }
    }

    // Update audio system if enabled
    if (g_cdAudioEnabled != 0) {
        FUN_80140d48();
    }

    return indexDiff;
}

/**
 * @brief Processes the CD-ROM recovery/init state machine across multiple VSync frames
 *
 * Drives a 4-state asynchronous state machine that reconfigures the CD-ROM
 * subsystem after an error or shell-open event. Each call advances at most
 * one state transition, allowing the caller to poll once per frame.
 *
 * @details
 * The state machine (stored in CD_SYSTEM.initState) progresses as follows:
 *
 * - **State 0 — Flush:** Calls CdFlush() to discard pending commands, then
 *   advances to state 1 with a 1-frame delay.
 *
 * - **State 1 — Set mode:** Waits for the delay to expire, then configures
 *   CD mode to 0xA0 (CdlModeSpeed | CdlModeSize1), installs
 *   CD_SyncCallback_Handler, sends CdlSetmode, and waits 4 frames.
 *   Returns 0 (still waiting) if the delay has not yet elapsed.
 *
 * - **State 2 — Set filter:** Installs sync callback, sends CdlSetfilter
 *   with file=1 channel=1, sets initCommand to 0x11 (pending demute),
 *   and advances to state 3 immediately.
 *
 * - **State 3 — Wait for sync / dispatch:** Waits for either the
 *   syncComplete flag or a 30-frame timeout, then dispatches based on
 *   initCommand:
 *     - 0x10: Re-sends CdlSetfilter (retry/default)
 *     - 0x11: Sends CdlDemute to unmute CD audio
 *     - 0x12: Sends CdlPause (command 0x09) to halt the drive
 *   After CdlSetfilter, resets initCommand to 0x10.
 *   Subtracts 30 from vsyncTimestamp to allow immediate re-entry on the
 *   next timeout cycle rather than resetting the timer.
 *
 * @param None
 *
 * @return 1 if the CD subsystem is not in recovery mode (statusFlags bit 3 clear),
 *         0 while the state machine is still processing
 *
 * @note
 * - initCommand acts as a sub-state within state 3 to sequence multiple
 *   CD commands (setfilter -> demute -> pause) across successive timeouts
 * - The filterParams buffer is written byte-by-byte to match the original
 *   assembly's sb instructions for register-level matching
 * - State 1 returns 0 early (not via the common exit) when the timestamp
 *   delay has not yet expired, matching a distinct return instruction in
 *   the original binary
 *
 * @warning
 * - Must be called every frame for correct timeout behavior
 * - The 1/4/30-frame delay constants assume NTSC (60 Hz) VSync rate
 *
 * @see decomp.me: (96.19%) https://decomp.me/scratch/rvy43
 */
s32 CD_RecoveryStateMachine(void) {
    u16 filterParams;
    s32 temp_v1_2;
    u8 temp_v1;
    u8 var_a0;
    s32 timestamp;
    u8 initCommandByte;
    s32 initCommand;
    
    // Bail out if bit 3 (recovery mode) is not set
    if (!(CD_SYSTEM.statusFlags.word & 8)) {
        return 1;
    }
    
    switch (CD_SYSTEM.initState) {
    case 0:
        // State 0: Flush pending CD commands and advance to state 1
        CdFlush();
        CD_SYSTEM.initState = 1U;
        CD_SYSTEM.vsyncTimestamp = (s32) (VSync(-1) + 1);
        goto Done;
    case 1:
        // State 1: Wait for 1-frame delay, then configure CD mode
        timestamp = VSync(-1);
        if (timestamp >= (s32)CD_SYSTEM.vsyncTimestamp) {
            // Set mode to 0xA0 (double speed + XA filter size)
            CD_SYSTEM.modeParams = 0xA0;
            CD_SYSTEM.u_155 = 0;
            CD_SYSTEM.u_156 = 0;
            CD_SYSTEM.u_157 = 0;
            
            CdSyncCallback(CD_SyncCallback_Handler);
            
            CdReadyCallback(NULL);
            // initCommand 0x10 = pending setfilter; must be stored before CdControlF (not in delay slot)
            CD_SYSTEM.initCommand = 0x10U;
            CdControlF(CdlSetmode, (u8* )0x801ED954);
            timestamp = VSync(-1);
            CD_SYSTEM.vsyncTimestamp = (s32) (timestamp + 4);
            goto Done;
        }
        // Delay not yet elapsed — return without advancing state
        return 0;
    case 2:
        // State 2: Send CdlSetfilter with file=1, channel=1, advance to state 3
        CdSyncCallback(CD_SyncCallback_Handler);
        CD_SYSTEM.initCommand = 0x11;
        
        *(u8*)(&filterParams) = 1;
        *(u8*)(&filterParams + 1) = 1;
        CdControlF(CdlSetfilter, &filterParams);
        CD_SYSTEM.initState = 3U;
        CD_SYSTEM.vsyncTimestamp = VSync(-1);
        goto Done;
    case 3:
        // State 3: Wait for syncComplete or 30-frame timeout, then dispatch
        if (CD_SYSTEM.syncComplete == 1) {
            CD_SYSTEM.vsyncTimestamp = VSync(-1);
            CD_SYSTEM.syncComplete = 0U;
            goto Done;
        }
        timestamp = VSync(-1);
        if (timestamp < (s32)(CD_SYSTEM.vsyncTimestamp + 30)) {
            goto Done;
        }
        
        // Timeout expired — dispatch follow-up command based on initCommand
        CdSyncCallback(CD_SyncCallback_Handler);

        initCommandByte = CD_SYSTEM.initCommand;
        initCommand = (s32) initCommandByte;
        
        if (initCommand != 0x11) {
            do {
                if (initCommand < 0x12) {
                    goto ApplySetfilter;
                }
                // initCommand 0x12: send CdlPause to halt the drive
                if (initCommand == 0x12) {
                    CdControlF(0x09, NULL);
                    goto UpdateTimestamp;
                }
ApplySetfilter:
                // initCommand 0x10 or unknown: re-send CdlSetfilter with file=1, channel=1
                *(u8*)(&filterParams) = 1;
        *(u8*)(&filterParams + 1) = 1;
                
                CdControlF(CdlSetfilter, &filterParams);
                
            } while (0);
            CD_SYSTEM.initCommand = 0x10U;
        } else {
            // initCommand 0x11: unmute CD audio
            CdControlF(CdlDemute, NULL);
        }
UpdateTimestamp:
        // Subtract 30 to allow immediate re-entry on the next timeout cycle
        CD_SYSTEM.vsyncTimestamp = (s32) (CD_SYSTEM.vsyncTimestamp - 30);
        goto Done;
    default:
        goto Done;
    }
  
Done:
    return 0;
}


/**
 * @brief Sync callback invoked when a CD-ROM command completes or fails
 *
 * Installed as the CdSyncCallback during normal command queue processing.
 * Handles command completion by advancing the circular queue, dispatching
 * the next queued command, or cleaning up when the queue is drained.
 *
 * @details
 * On entry, sets syncComplete to 1 so the main-loop poller knows progress
 * was made. Then branches based on the interrupt status:
 *
 * **Error path (status byte bit 4 set during CdlNop):**
 * If currentCommand is 1 (CdlNop probe) and the drive reports an error,
 * calls CD_HandleSyncError() and returns immediately.
 *
 * **Incomplete path (status != CdlComplete):**
 * If the command did not finish successfully:
 *   - If currentCommand != 1, resets to CdlNop and retries
 *   - Otherwise falls through to re-read the queue head and execute it
 *
 * **Complete path (status == CdlComplete):**
 * Switches on currentCommand:
 *   - **Case 21 (CdlPause):** Resets playback state and loop counter,
 *     advances the queue read index. If the queue is now empty, clears
 *     all execution state and returns. Otherwise dispatches the next command.
 *   - **All other cases (default):** Reads the command at the queue head.
 *     If it is CdlNop (1), skips forward through consecutive CdlNop entries
 *     until a different command is found or the queue is exhausted.
 *     Then dispatches the resolved command via CD_ExecuteCommand.
 *
 * Special handling: if the resolved command is 0x1B (audio start), enables
 * CD_SYSTEM.audioEnabled and remaps the command to CdlSeekL (6) for execution.
 *
 * @param status    CD-ROM interrupt status byte (CdlComplete on success)
 * @param resultPtr Pointer to the CD-ROM result byte array from the hardware
 *
 * @return void
 *
 * @note
 * - This function runs in interrupt context as a CdSyncCallback
 * - The volatile vsyncArg variable and separate statusFlags assignment
 *   match the original assembly's register usage and must not be optimized
 * - The large switch with explicit case labels for 1-27 (excluding 6, 21)
 *   matches the original jump table layout in the binary
 *
 * @warning
 * - Executes in interrupt context; must not call blocking functions
 * - Modifies CD_SYSTEM state directly; not safe to call from main thread
 *
 * @see decomp.me: (87.29%) https://decomp.me/scratch/F0oiy
 */
void CD_OnCommandComplete(s32 status, u8* resultPtr) {
    u8 nextCommand;
    u32 readIndex;
    u32 writeIndex;
    u8 nopCommand;
    s32 statusFlags;
    volatile s32 vsyncArg;
    CdSystem* cdSystem;

    // Signal to main-loop poller that a sync event occurred
    CD_SYSTEM.syncComplete = 1;

    // If we sent CdlNop to probe status, check for drive errors
    if (CD_SYSTEM.currentCommand == 1) {
        if (*resultPtr & 0x10) {
            CD_HandleSyncError();
            return;
        }
    }

    // If the command did not complete successfully, handle retry/fallthrough
    if ((status & 0xFF) != CdlComplete) {
        goto HandleIncomplete;
    }

    // Command completed — dispatch based on which command just finished
    switch (CD_SYSTEM.currentCommand) {
    default:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
        // Read the command at the current queue head
        nextCommand = CD_SYSTEM.commandQueue.items[CD_SYSTEM.queueReadIndex].command;

        // Skip past consecutive CdlNop (1) entries in the queue
        if (nextCommand == 1) {
            writeIndex = CD_SYSTEM.queueWriteIndex;
            nopCommand = 1;
            do {
                readIndex = CD_SYSTEM.queueReadIndex;
                if (readIndex == writeIndex) {
                    goto QueueDrained;
                }
                readIndex = (readIndex + 1) & 0xF;
                CD_SYSTEM.queueReadIndex = readIndex;
                nextCommand = CD_SYSTEM.commandQueue.items[readIndex].command;
            } while (nextCommand == nopCommand);
        }
        goto DispatchCommand;

    case 21:
        // CdlPause completed — reset playback state and advance queue
        CD_SYSTEM.playbackState = 0;
        CD_SYSTEM.loopCounter = 0;
        readIndex = (CD_SYSTEM.queueReadIndex + 1) & 0xF;
        CD_SYSTEM.queueReadIndex = readIndex;

        // If queue is now empty after pause, clean up and return
        if (readIndex == CD_SYSTEM.queueWriteIndex) {
            CdSyncCallback(NULL);
            statusFlags = CD_SYSTEM.statusFlags.word;
            CD_SYSTEM.currentCommand = 0;
            CD_SYSTEM.initCommand = 0;
            CD_SYSTEM.retryCounter = 0;
            goto ApplyFlagsAndTimestamp;
        }

        // Queue still has entries — dispatch the next one
        nextCommand = CD_SYSTEM.commandQueue.items[readIndex].command;
        goto DispatchCommand;
    }

DispatchCommand:
    //cdSystem = &CD_SYSTEM; // this seems to be necessary SOMEWHERE in order to force loading 801ed800, but I don't know where.

    // Special case: command 0x1B (audio start) — enable audio and remap to CdlSeekL
    if (nextCommand == 0x1B) {
        if (g_cdAudioEnabled == 0) {
            CD_SYSTEM.audioEnabled = 1;
        }
        nextCommand = 6;
    }
    goto ExecuteNext;

HandleIncomplete:
    // Command did not complete — if not already probing with CdlNop, retry with CdlNop
    if (CD_SYSTEM.currentCommand != 1) {
        CD_SYSTEM.currentCommand = 1;
        CdControlF(1, NULL);
        return;
    }
    // Already was CdlNop — fall through to re-read queue head and execute
    goto ReadQueueHead;

QueueDrained:
    // All commands consumed — remove sync callback and reset execution state
    CdSyncCallback(NULL);
  
    statusFlags = CD_SYSTEM.statusFlags.word & ~0x10;
    vsyncArg = -1;
    CD_SYSTEM.playbackState = 0;
    CD_SYSTEM.loopCounter = 0;
    CD_SYSTEM.currentCommand = 0;
    CD_SYSTEM.retryCounter = 0;

ApplyFlagsAndTimestamp:
    CD_SYSTEM.statusFlags.word = statusFlags;
    CD_SYSTEM.vsyncTimestamp = VSync(vsyncArg);
    return;

ReadQueueHead:
    nextCommand = CD_SYSTEM.commandQueue.items[CD_SYSTEM.queueReadIndex].command;

ExecuteNext:
    CD_ExecuteCommand(nextCommand, 0, 0);
    return;

}

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
 * @brief Checks whether a resource index is absent from the pending command queue
 *
 * Scans every entry currently sitting in the circular command queue and
 * compares its resource index against the requested one. Returns 1 (true)
 * if no matching entry is found, meaning the caller is free to enqueue a
 * new command for that resource without creating a duplicate.
 *
 * @details
 * The scan walks from the current read index forward through the number
 * of pending entries (writeIndex - readIndex, masked to 4 bits for the
 * 16-entry circular buffer). For each slot, the stored resourceIndex is
 * compared against the lower 16 bits of the argument.
 *
 * The index variable is masked with 0xF on each iteration before
 * incrementing, which keeps it within the 16-entry circular buffer bounds.
 *
 * @param resourceIndex  Resource index to search for (lower 16 bits used)
 *
 * @return 1 if the resource index is not already queued (safe to enqueue),
 *         0 if a matching entry was found (duplicate present)
 *
 * @note
 * - Only the lower 16 bits of resourceIndex are compared, matching the
 *   unsigned short storage in CdCommandQueueItem
 * - The sentinel value -1 and the separate compare variable match the
 *   original assembly's register usage
 *
 * @see CD_QueueCommand — the function that actually enqueues commands
 * @see decomp.me: (100%) https://decomp.me/scratch/mpPwi
 */
s32 CD_CanQueueResourceIndex(s32 resourceIndex) {
    s32 queuedResourceIndex;
    s32 scanIndex;
    s32 remainingEntries;
    s32 sentinel;
    
    scanIndex = CD_SYSTEM.queueReadIndex;
    
    // Calculate number of pending entries in the circular queue
    remainingEntries = ((CD_SYSTEM.queueWriteIndex - scanIndex) & 0x0F);
    remainingEntries -= 1;
    sentinel = -1;
    
    // If queue is non-empty, scan all pending entries for a match
    if (remainingEntries != sentinel) {

        while (1) {

            // Check if this queued entry already targets the same resource
            queuedResourceIndex = CD_SYSTEM.commandQueue.items[scanIndex].resourceIndex;
            
            if ((resourceIndex & 0xFFFF) == queuedResourceIndex) {
                return 0;
            }

            // Advance scan index with circular wrap (mod 16)
            scanIndex = (scanIndex & 0xF);
            scanIndex += 1;
            remainingEntries -= 1;
            
            if (remainingEntries == -1) {
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