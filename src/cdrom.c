#include "cd.h"
#include "psyq/libetc.h"
#include "psyq/libcd.h"
#include "psyq/libpress.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"

void cdrom_init()
{
    int queueEndMarker;
    int queueCount;
    volatile CdCommandQueueItem* queueItem;
    CdResourceEntry* scratchpadAddr;
    CdStatusFlags* statusFlagsPtr;
    int cdResult;

    // Wait for CD-ROM system to initialize
    while (CdInit() == 0);

    CdSetDebug(0);

    // Save previous callbacks, then clear them
    g_cdSyncCallbackResult = CdSyncCallback(NULL);
    g_cdReadyCallbackResult = CdReadyCallback(NULL);

    statusFlagsPtr = &CD_SYSTEM.statusFlags;

    queueCount = CD_COMMAND_QUEUE_SIZE - 1;
    scratchpadAddr = (CdResourceEntry*)SCRATCHPAD;

    queueEndMarker = -1;

    // g_commandQueueOffset is commandQueue.items[11]; the loop uses queueItem[4]
    // to walk items[15] down to items[0] (all 16 entries).
    queueItem = &g_commandQueueOffset;

    // 0xFFFE = invalid/no resource loaded
    CD_SYSTEM.resourceIndex = CD_RESOURCE_INDEX_INVALID;

    // Reset all runtime state to zero
    CD_SYSTEM.audioEnabled = 0;
    CD_SYSTEM.playbackState = 0;
    CD_SYSTEM.transferCallback = NULL;
    CD_SYSTEM.pendingQueueCount = 0;
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
    statusFlagsPtr->bytes.retryExhausted = 0;

    // Zero all 16 command queue entries, setting default buffer to scratchpad
    while (queueCount != queueEndMarker)
    {
        queueItem[4].command = 0;
        queueItem[4].resourceIndex = 0;
        queueItem[4].dstBuffer = scratchpadAddr;
        queueItem[4].entry = scratchpadAddr;
        queueItem[4].callback = 0;
        queueItem--;
        queueCount--;
    }

    CD_SYSTEM.setModeParamBlocking[0] = (CdlModeSpeed | CdlModeSize1);
    CD_SYSTEM.setModeParamBlocking[1] = 0;
    CD_SYSTEM.setModeParamBlocking[2] = 0;
    CD_SYSTEM.setModeParamBlocking[3] = 0;

    // CdlNop (1) — read current drive status into statusByte
    while (CdControlB(CdlNop, NULL, &CD_SYSTEM.statusByte) == 0);

    // If shell-open flag (0x10) is set, block until disc becomes ready
    if ((g_cdStatusByte & CdlStatShellOpen) != 0)
    {
        cdResult = CdDiskReady(1);

        while (cdResult != CdlComplete)
        {
            cdResult = CdDiskReady(0);
        }
    }

    // CdlSetmode (14) — apply mode byte (0xA0) to the drive
    while (CdControlB(CdlSetmode, CD_SYSTEM.setModeParamBlocking, NULL) == 0);

    // Record current frame counter for timeout tracking
    g_cdVSyncTimestamp = VSync(-1);
}

void cdrom_stop(void)
{
    int cdResult;
    CdSystem* cdSystem;

    cdSystem = &CD_SYSTEM;

    if (g_cdAudioEnabled != 0)
    {
        cdrom_reset();
    }

    cdSystem->statusFlags.word &= 0xFFFFFFBF;

    CdSyncCallback(NULL);
    CdReadyCallback(NULL);

    while (CdControlB(CdlPause, NULL, NULL) == 0);

    CD_SYSTEM.resourceIndex = CD_RESOURCE_INDEX_INVALID;
    CD_SYSTEM.pendingQueueCount = 0;
    CD_SYSTEM.currentResourceIndex = 0;
    CD_SYSTEM.currentDataSize = 0;
    CD_SYSTEM.targetDataSize = 0;
    CD_SYSTEM.playbackState = 0;
    CD_SYSTEM.transferCallback = NULL;
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

s32 cdrom_stream(s32 command, u32 destination)
{
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
    CdStreamState* streamState;
    CdStreamState* streamState2;
    u32 destStart;
    s32 alignRemainder;
    s32 sentinel;

    /* Block until any in-progress CD commands finish */
    while (cdrom_process_state() != 0)
    {
        VSync(0);
    }

    destStart = destination;
    streamState = &CD_STREAM_STATE;

    /* Zero out the streaming state */
    streamState->reserved = 0;
    streamState->dataReady = 0U;
    streamState->bufferWrapped = 0U;
    streamState->bytesConsumed = 0;

    /* Enqueue a CdlReadN command; return value is the resource's total data size.
     * Subtract 1 to get the last valid byte offset for streaming. */
    remainingDataSize = cdrom_queue_command(CdlReadN, command, NULL, &cdrom_handle_stream_data) - 1;
    timestamp = VSync(-1);

    streamState2 = &CD_STREAM_STATE;

    /* === Main streaming loop === */
    while (TRUE)
    {
        if (VSync(-1) < (timestamp + 30))
        {
            /* Timeout hasn't elapsed — check if callback signaled new data */
            if (streamState2->dataReady != 1)
            {
                continue;
            }

            /* === Decompression loop: process all available buffered data === */
            do
            {
                bytesBuffered = streamState2->bytesBuffered;

                /* Calculate source-end boundary for decompression.
                 * If we have fewer bytes buffered than total remaining, hold back
                 * 280 bytes as a safety margin to avoid reading incomplete sectors.
                 * Otherwise use the exact remaining size as the boundary. */
                if (bytesBuffered < remainingDataSize)
                {
                    decompressEnd = (streamState2->readPtr + bytesBuffered) - 280;
                }
                else
                {
                    decompressEnd = streamState2->readPtr + remainingDataSize;
                }

                /* Decompress a chunk; returns 0 when all output is complete */
                if (cdrom_decompress_data(&CD_STREAM_STATE.writePtr, &destination, decompressEnd, -4U) == 0)
                {
                    return destination - destStart;
                }
            } while (bytesBuffered != CD_STREAM_STATE.bytesBuffered);

            /* All currently buffered data has been fed to the decompressor */
            bytesConsumed = streamState2->writePtr - streamState2->readPtr;
            streamState2->bytesConsumed = bytesConsumed;
            cdrom_clear_data_ready(&CD_STREAM_STATE.dataReady);
            remainingDataSize -= bytesConsumed;

            /* If the ring buffer hasn't wrapped, yield to let more data arrive */
            if (streamState2->bufferWrapped != 1)
            {
                timestamp = VSync(-1);
                continue;
            }

            /* --- Handle ring buffer wrap-around --- */

            if (streamState2->wrapOverflow != 0)
            {

                decompressEnd = streamState2->wrapOverflow; /* wrapOverflow amount */

                /* Relocate unprocessed tail bytes to just before the ring buffer
                 * end (0x801DC118), making the data contiguous again. */
                unprocessedBytes = streamState2->bytesBuffered - bytesConsumed;
                alignRemainder = (unprocessedBytes & 3);
                relocDstAddr = 0x801DC118 - unprocessedBytes;
                prevReadPtr = streamState2->readPtr;

                /* Word-align the relocation destination downward */
                copySize = 4 - alignRemainder;
                streamState2->writePtr = relocDstAddr;
                streamState2->readPtr = relocDstAddr;
                copySize = copySize & 3;
                alignRemainder = unprocessedBytes + 3;

                /* Adjust pointers to include alignment padding bytes */
                relocDstAddr = (s32)(relocDstAddr - copySize);
                relocSrcPtr = (s32*)((prevReadPtr + bytesConsumed) - copySize);

                /* Merge overflow bytes into the new contiguous buffer region */
                streamState2->bytesBuffered = decompressEnd + unprocessedBytes;
                copySize = alignRemainder;

                if (copySize < 0)
                {
                    copySize = unprocessedBytes + 6;
                }

                /* Copy unprocessed bytes word-by-word to the relocated position */

                unprocessedBytes = (copySize >> 2);
                unprocessedBytes--;

                if (unprocessedBytes != -1)
                {
                    sentinel = -1;
                    while (unprocessedBytes != sentinel)
                    {
                        *(s32*)relocDstAddr = *relocSrcPtr++;
                        relocDstAddr += 4;
                        unprocessedBytes--;
                    }
                }
            }
            else
            {
                /* No wrap — simply advance the read pointer past consumed data */
                streamState2->readPtr += bytesConsumed;
                streamState2->bytesBuffered -= bytesConsumed;
            }

            /* Memory barrier: prevent compiler from reordering the ready flag write */
            *(volatile u8*)streamState2 = 1;

            timestamp = VSync(-1);
            continue;
        }

        /* Timeout elapsed without data — pump the CD command queue */
        cdrom_process_state();
        timestamp = VSync(-1);
    }
}

void cdrom_stream_chunked(undefined2 resourceIndex, codeA pfnGetBuffer, codeB pfnChunkDone)
{
    int timestamp;
    u8 srcByte;
    int decompressResult; // Return from cdrom_decompress_data: 0 = end-of-stream, 1 = output full
    u32 srcWord;
    int loopCount;
    u32 alignCheck;
    u32 decompressEnd;        // Source-side guard address passed to cdrom_decompress_data
    u8* srcPtr;               // Read cursor into the staging buffer during the copy-out phase
    int totalBytesDelivered;  // Total decompressed bytes given to caller so far (passed to pfnGetBuffer)
    int chunkIndex;           // How many chunks delivered so far (passed to pfnChunkDone)
    int chunkBytesRemaining;  // Capacity left in the current destination chunk (or -1 = unlimited)
    u8* destination;          // Write cursor into the current caller-supplied destination chunk
    u8* stagingWritePtr;      // Write cursor into the intermediate staging buffer // s1
    u8* stagingEnd;           // End of the staging buffer (0x801DBBE8) // t0
    u8* dstEnd;               // End of the current caller chunk (or 0xFFFFFFFC in direct mode) // t0
    s32 remainingDataSize;    // Bytes of compressed input still to consume (counts down to 0)
    int isDirectMode;         // Non-zero (0x1000) = direct mode; 0 = chunked/staging mode
    s32 bytesBuffered;        // s3
    s32 stagingBytesProduced; // Bytes written into staging buffer by last cdrom_decompress_data call
    s32 bytesConsumed;
    s32 unprocessedBytes;
    s32 alignRemainder;
    s32 copySize;
    s32 relocDstAddr;
    s32 prevReadPtr;
    s32* relocSrcPtr;
    u32 wrapOverflow;
    CdStreamState* scratchpad;
    CdStreamState* streamState;
    s32 sentinel;          // Always -1; used as a do-while-not-(-1) terminator (MIPS loop idiom) // s8
    u8** pDestination;     // Pointer-to-pointer to `destination`; kept in register for copy loops // s2
    u8** pStagingWritePtr; // Same pattern, used during LZ window copy
    s32 new_var;

    // --- Initialise streaming state in scratchpad RAM ---
    scratchpad = &CD_STREAM_STATE;
    scratchpad->reserved = 0;
    scratchpad->dataReady = 0;
    scratchpad->bufferWrapped = 0;

    // Enqueue CdlReadN for this resource; subtract 1 to get the last valid compressed-byte offset.
    remainingDataSize = cdrom_queue_command(CdlReadN, resourceIndex, NULL, cdrom_handle_stream_data) - 1;

    totalBytesDelivered = 0;
    chunkIndex = 0;

    // Ask the caller for the first destination chunk.
    destination = pfnGetBuffer(0, &chunkBytesRemaining);

    if (chunkBytesRemaining == -1)
    {
        // --- Direct mode: caller provided an "unlimited" buffer ---
        // Sentinel dstEnd well past any real address; isDirectMode flag routes
        // the inner loop to decompress straight into `destination`.
        dstEnd = (u8*)0xfffffffc;
        isDirectMode = 0x1000;
    }
    else
    {
        // --- Chunked mode: caller provided a fixed-size chunk ---
        // Reserve 0x418 (1048) bytes at the end as a guard region.
        // TODO: Determine exactly why this guard is needed.
        dstEnd = destination + chunkBytesRemaining - 0x418;
        isDirectMode = 0;
    }

    // Staging buffer lives in main RAM. Decompressor fills this; we copy out to caller chunks.
    stagingWritePtr = (u8*)0x801da000; // Start of staging buffer
    stagingEnd = (u8*)0x801dbbe8;      // End of staging buffer

    timestamp = VSync(-1);
    streamState = &CD_STREAM_STATE;
    sentinel = -1;
    pDestination = &destination; // Kept in a register so the copy loops can update `destination` indirectly

    while (TRUE)
    {

        if (VSync(-1) < timestamp + 30)
        {

            if (streamState->dataReady != 1)
            {
                continue;
            }

            // === Decompression loop: process all currently buffered sector data ===
            do
            {
                bytesBuffered = streamState->bytesBuffered;

                // Calculate the compressed-source end-guard for cdrom_decompress_data.
                // Hold back 280 bytes when the full stream hasn't arrived yet,
                // to avoid consuming an incomplete sector boundary.
                if (bytesBuffered < remainingDataSize)
                {
                    decompressEnd = (streamState->readPtr + bytesBuffered) - 280;
                }
                else
                {
                    decompressEnd = streamState->readPtr + remainingDataSize;
                }

                // --- Direct mode: decompress straight into the caller's destination ---
                if (isDirectMode != 0 && destination < dstEnd)
                {
                    cdrom_decompress_data(&CD_STREAM_STATE.writePtr, (u32*)&destination, decompressEnd, (u32)dstEnd);
                    continue; // Loop back; decompressResult check happens below on exit
                }

                // --- Chunked mode: decompress into staging buffer, then copy to caller ---
                srcPtr = stagingWritePtr; // Remember staging write position before this call
                decompressResult = cdrom_decompress_data(&CD_STREAM_STATE.writePtr, (u32*)&stagingWritePtr, decompressEnd,
                                                     (u32)stagingEnd);

                // How many bytes did the decompressor write to the staging buffer this pass?
                stagingBytesProduced = (int)stagingWritePtr - (int)srcPtr;

                // === Copy staging bytes into the caller's chunk(s) ===
                while (stagingBytesProduced != 0)
                {

                    if ((stagingBytesProduced < chunkBytesRemaining) || (chunkBytesRemaining == sentinel))
                    {
                        // All remaining staging bytes fit within the current chunk (or chunk is unlimited).
                        // Copy everything and break out.
                        totalBytesDelivered += stagingBytesProduced;
                        chunkBytesRemaining -= stagingBytesProduced;

                        // --- Destination alignment preamble ---
                        // Byte-copy until `destination` is word-aligned.
                        alignCheck = (u32)destination & 3;
                        relocDstAddr = (int)alignCheck;
                        if ((alignCheck != 0) && (relocDstAddr < stagingBytesProduced))
                        {
                            stagingBytesProduced -= alignCheck;
                            new_var = sentinel;
                            loopCount = alignCheck - 1;
                            while (loopCount != new_var)
                            {
                                u8* dest;
                                sentinel = -1;
                                srcByte = *srcPtr++;
                                dest = *pDestination;
                                *dest = srcByte;
                                *pDestination = dest + 1;
                                loopCount--;
                            }
                        }

                        // --- Fast word-copy (only if source is also word-aligned) ---
                        alignCheck = (u32)srcPtr & 3;
                        if (alignCheck == 0)
                        {
                            loopCount = stagingBytesProduced >> 2;
                            stagingBytesProduced -= loopCount * 4;
                            loopCount--;
                            while (loopCount != sentinel)
                            {
                                u32* dest;
                                sentinel = -1;
                                srcWord = *(u32*)srcPtr;
                                srcPtr += 4;
                                dest = (u32*)*pDestination;
                                *dest = srcWord;
                                *pDestination = (u8*)(dest + 1);
                                loopCount--;
                            }
                        }

                        // --- Byte-copy for any remaining tail bytes ---
                        stagingBytesProduced--;
                        while (stagingBytesProduced != sentinel)
                        {
                            u8* dest;
                            sentinel = -1;
                            srcByte = *srcPtr++;
                            dest = *pDestination;
                            *dest = srcByte;
                            *pDestination = dest + 1;
                            stagingBytesProduced--;
                        }

                        break; // Done with this batch of staging bytes
                    }

                    // --- Staging bytes span a chunk boundary: fill the current chunk ---
                    // Copy exactly chunkBytesRemaining bytes, fire pfnChunkDone,
                    // then get the next chunk from the caller and keep copying.
                    loopCount = chunkBytesRemaining - 1;
                    stagingBytesProduced -= chunkBytesRemaining; // Bytes that will spill into next chunk
                    totalBytesDelivered += chunkBytesRemaining;
                    chunkBytesRemaining = loopCount;

                    while (loopCount != sentinel)
                    {
                        u8* dest = *pDestination;
                        sentinel = -1;
                        srcByte = *srcPtr;
                        *dest = srcByte;
                        *pDestination = dest + 1;
                        srcPtr++;
                        loopCount = chunkBytesRemaining;
                        loopCount--;
                        chunkBytesRemaining = loopCount;
                    }

                    // Notify caller chunk is complete and request the next one —
                    // but only if there's still data to copy or the decompressor has more pending.
                    if (stagingBytesProduced > 0 || decompressResult != 0)
                    {
                        pfnChunkDone(chunkIndex); // Chunk complete
                        chunkIndex++;
                        destination = pfnGetBuffer(totalBytesDelivered, &chunkBytesRemaining); // Next chunk
                    }
                }

                if (decompressResult != 0)
                {
                    // Staging buffer was exhausted before the stream ended
                    // (decompressResult == 1 means "output buffer full, more input remains").
                    //
                    // Preserve the LZ back-reference sliding window (last 4096 bytes of staging
                    // output) so that back-references remain valid on the next decompress pass.
                    // Copy those 4096 bytes back to 0x801DA000; the decompressor will then
                    // resume writing fresh output starting at 0x801DB000.
                    s32 loopSentinel;
                    stagingWritePtr = (u8*)0x801da000; // Reset staging write cursor to start
                    srcPtr = srcPtr - 0x1000;          // Step back 4096 bytes to start of LZ window
                    stagingBytesProduced = 0xfff;      // Loop count: copy 4096 bytes (terminates at -1)
                    pStagingWritePtr = &stagingWritePtr;
                    loopSentinel = -1;

                    // Copy the 4096-byte LZ window to 0x801DA000.
                    // After this, stagingWritePtr == 0x801DB000; fresh decompressor output follows there.
                    do
                    {
                        u8* dest;
                        srcByte = *srcPtr++;
                        dest = *pStagingWritePtr;
                        stagingBytesProduced--;
                        *dest = srcByte;
                        *pStagingWritePtr = dest + 1;
                    } while (stagingBytesProduced != loopSentinel);

                    continue; // Go back and decompress another pass into the (now reset) staging buffer
                }

                // --- Decompression complete (end-of-stream opcode 0xFF was reached) ---
                // Fire pfnChunkDone for the final (possibly partial) chunk and return.
                pfnChunkDone(chunkIndex);
                return;

            } while (bytesBuffered != CD_STREAM_STATE.bytesBuffered);
            // Re-check: if bytesBuffered changed while we were running (new sectors arrived from
            // the CD callback), loop again immediately to consume the fresh data.

            // --- Update streaming state after processing all buffered data ---
            bytesConsumed = streamState->writePtr - streamState->readPtr;
            prevReadPtr = streamState->readPtr;

            streamState->dataReady = 0; // Clear the "new data ready" flag
            streamState->bytesConsumed = bytesConsumed;
            remainingDataSize -= bytesConsumed;

            if (streamState->bufferWrapped != 1)
            {
                // Ring buffer has not wrapped; just yield and wait for more sectors.
                timestamp = VSync(-1);
                continue;
            }

            // --- Handle ring buffer wrap-around ---
            // (Identical logic to cdrom_stream; see that function for detailed comments)
            wrapOverflow = streamState->wrapOverflow;

            if (wrapOverflow != 0)
            {
                unprocessedBytes = streamState->bytesBuffered - bytesConsumed;
                alignRemainder = (unprocessedBytes & 3);
                relocDstAddr = 0x801dc118 - unprocessedBytes;

                copySize = 4 - alignRemainder;
                copySize = copySize & 3;

                relocDstAddr = relocDstAddr - copySize;
                streamState->writePtr = relocDstAddr;
                streamState->readPtr = relocDstAddr;

                relocSrcPtr = (s32*)((prevReadPtr + bytesConsumed) - copySize);

                streamState->bytesBuffered = wrapOverflow + unprocessedBytes;
                alignRemainder = unprocessedBytes + 3;
                copySize = alignRemainder;

                if (copySize < 0)
                {
                    copySize = unprocessedBytes + 6;
                }

                unprocessedBytes = (copySize >> 2);
                unprocessedBytes--;

                while (unprocessedBytes != sentinel)
                {
                    *(s32*)relocDstAddr = *relocSrcPtr++;
                    relocDstAddr += 4;
                    unprocessedBytes--;
                }
            }
            else
            {
                streamState->readPtr = prevReadPtr + bytesConsumed;
                streamState->bytesBuffered -= bytesConsumed;
            }

            *(volatile u8*)streamState = 1; // Memory barrier: signal callback that buffer is ready again
            timestamp = VSync(-1);
            continue;
        }

        // Timeout — pump the CD command queue and reset the timer.
        cdrom_process_state();
        timestamp = VSync(-1);
    }
}

s32 cdrom_queue_command(u8 command, u16 resourceIndex, void* dstBuffer, CdCommandCallback callback)
{
    s32 timestamp;
    s32 writeIndex;
    s32 writeIndex2;
    s32 statusFlags;
    s32 dataSize;
    u8 activeCommand;
    CdResourceEntry* resourceEntry;
    volatile CdSystem* cdSystem;

    // Reject immediately if the "playing" flag (bit 6) is set
    if (g_cdSystem.statusFlags.word & 0x40)
    {
        return -3;
    }

    // Resolve resource index to entry pointer
    if (resourceIndex == CD_RESOURCE_INDEX_DEFAULT)
    {
        resourceEntry = &g_defaultCdResource;
    }
    else
    {
        resourceEntry = &CD_RESOURCE_ENTRIES[resourceIndex];
    }

    cdSystem = &CD_SYSTEM;

    // Deduplicate: skip enqueue if system is busy AND the command matches
    // the previously enqueued one exactly (same command, resource, buffer, callback)
    if ((cdSystem->currentCommand == 0 && cdSystem->initCommand == 0) ||
        ((CD_SYSTEM.lastCommand != command) || (CD_SYSTEM.resourceIndex != resourceIndex) ||
         (CD_SYSTEM.dstBuffer != dstBuffer) || (CD_SYSTEM.callback != callback)))
    {

        // Validate resource entry has a valid disc location and non-zero size
        if ((*(u32*)&resourceEntry->location == 0) || (resourceEntry->dataSize == 0))
        {
            return -2;
        }

        // Check circular queue is not full
        writeIndex = CD_SYSTEM.queueWriteIndex;

        if (CD_SYSTEM.queueReadIndex == ((writeIndex + 1) & 0xF))
        {
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

        if ((activeCommand != 0) || (CD_SYSTEM.initCommand != 0))
        {
            return resourceEntry->dataSize;
        }

        statusFlags = CD_SYSTEM.statusFlags.word;

        // If no error/init flags (bits 0-3) are active, bootstrap execution
        if (!(statusFlags & 0xF))
        {

            CD_SYSTEM.vsyncTimestamp = timestamp;
            CD_SYSTEM.pendingQueueCount = 1;
            CD_SYSTEM.currentResourceIndex = resourceIndex;
            dataSize = resourceEntry->dataSize;
            CD_SYSTEM.currentCommand = 1;
            CD_SYSTEM.statusFlags.word = (statusFlags | 0x10);
            CD_SYSTEM.playbackState = 0;
            CD_SYSTEM.transferCallback = NULL;
            CD_SYSTEM.targetDataSize = dataSize;
            CD_SYSTEM.currentDataSize = dataSize;

            // Install sync callback and send CdlNop to kick off the state machine
            CdSyncCallback(&cdrom_complete_command);
            CdSync(0, NULL);
            CdControlF(CdlNop, NULL);
        }
    }

    return resourceEntry->dataSize;
}

u32 cdrom_process_state(void)
{
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
    if (statusFlags & 8)
    {
        return 0;
    }

    initState = 1U;

    // Branch 1: Active command processing (bits 0-2 set)
    if (statusFlags & 7)
    {
        // Calculate queue difference
        readIndex = CD_SYSTEM.queueReadIndex;
        queueDiff = (CD_SYSTEM.queueWriteIndex - readIndex);

        CD_SYSTEM.pendingQueueCount = queueDiff & 0xF;

        // Initialize queue processing if not already initialized
        if (CD_SYSTEM.initState == 0)
        {
            CD_SYSTEM.initState = initState;

            // Load current queue item if queue is not empty
            if (CD_SYSTEM.pendingQueueCount != 0)
            {
                // Manual pointer arithmetic to access queue item
                // (preserved from decompilation for register matching)
                readIndex = (u32)&CD_SYSTEM + (readIndex << 4);
                CD_SYSTEM.currentResourceIndex = *(u16*)(readIndex + 0x42);
                CD_SYSTEM.currentDataSize = *(s32*)(*((s32*)(readIndex + 0x44)) + 4);
                CD_SYSTEM.targetDataSize = CD_SYSTEM.readRemainingBytes;
            }

            // Handle audio playback initialization
            if (CD_SYSTEM.audioEnabled != 0)
            {
                cdSystemPtr = &CD_SYSTEM;
                if (g_cdAudioReady != 0)
                {
                    FUN_80022400(3); // Audio function
                }
            }

            // Set playback state based on transfer callback
            if (CD_SYSTEM.transferCallback != NULL)
            {
                CD_SYSTEM.playbackState = 1;
            }
            else
            {
                CD_SYSTEM.playbackState = 0;
            }

            g_cdStatusByte3 = 0;
        }

        // Check if enough time has passed (30 VSync frames)
        if (VSync(-1) >= ((s32)CD_SYSTEM.vsyncTimestamp + 30))
        {

            // Update timestamp if not in state 8
            if (CD_SYSTEM.initState != 8)
            {
                CD_SYSTEM.vsyncTimestamp = VSync(-1);
            }

            // Send NOP command to check CD status
            controlResult = CdControlB(CdlNop, 0, (u8*)0x801ED960);

            // Check if CD error bit (0x10) is NOT set
            if (!(CD_SYSTEM.statusByte & 0x10))
            {
                cdSystem = &CD_SYSTEM;
                if (controlResult != 0)
                {
                    // CD-ROM initialization state machine
                    initState = cdSystem->initState;
                    switch (initState)
                    {

                    case 1: // Initial state - start initialization
                        CD_SYSTEM.initState = 2U;
                        CD_SYSTEM.statusFlags.word = (s32)((CD_SYSTEM.statusFlags.word & ~1) | 6);
                        /* fallthrough */

                    case 2: // GetStat command
                        checkDiskResult = CdControlB(0x13U, 0, (u8*)0x801ED960);
                        if ((CD_SYSTEM.statusByte & 2) && (checkDiskResult != 0))
                        {
                            CD_SYSTEM.initState = 3U;
                            CD_SYSTEM.retryCounter = 0U;
                        }
                        break;

                    case 3: // Wait for disk ready with retries
                        if (CdDiskReady(1) == 2)
                        {
                            g_initState = 4;
                        }
                        else
                        {
                            retryCounter = CD_SYSTEM.retryCounter;
                            CD_SYSTEM.retryCounter = (u8)(retryCounter + 1);
                            if ((u32)((retryCounter + 2) & 0xFF) >= 0xDU)
                            {
                                CD_SYSTEM.initState = 4U;
                            }
                        }
                        break;

                    case 4: // Check disk status
                        diskReadyResult = CdDiskReady(0);
                        if (diskReadyResult != 2)
                        {
                            if (diskReadyResult == 0x10)
                            {
                                g_initState = 1; // No disk, restart
                            }
                            else
                            {
                                goto SetInitState5;
                            }
                        }
                        else
                        {
                        SetInitState5:
                            g_initState = 5;
                        }
                        break;

                    case 5: // Detect disk type
                        diskType = CdGetDiskType();
                        switch (diskType)
                        {
                        case 0: // No disk
                            do
                            {
                                CD_SYSTEM.initState = 0x20U;
                            } while (0);
                            flagsForUpdate = CD_SYSTEM.statusFlags.word;
                            flagsMask = -3;
                            goto UpdateStatusFlags;

                        case 1: // Audio CD (needs verification)
                            CdDiskReady(0);
                            CdGetDiskType();
                            /* fallthrough */

                        case 2: // Valid CD-ROM
                            CD_SYSTEM.initState = 6U;
                            CD_SYSTEM.vsyncTimestamp = (s32)(CD_SYSTEM.vsyncTimestamp - 0x1E);
                            break;
                        }
                        break;

                    case 6: // Set CD mode parameters
                        cdSystem = &CD_SYSTEM;
                        cdSystem->setModeParamAsync[0] = (CdlModeSpeed | CdlModeSize1);
                        cdSystem->setModeParamAsync[1] = 0;
                        cdSystem->setModeParamAsync[2] = 0;
                        CD_SYSTEM.setModeParamAsync[3] = 0;
                        CdSyncCallback(cdrom_handle_recovery_sync);
                        CdReadyCallback(0);
                        cdSystem->initCommand = 0x20U;
                        CdControlF(CdlSetmode, (u8*)0x801ED954);
                        CD_SYSTEM.vsyncTimestamp = (s32)(cdSystem->vsyncTimestamp - 0x1A);
                        break;

                    case 7: // Start reading
                        CD_SYSTEM.recoveryReadPosition.raw = (s32)g_cdResource176;
                        CD_SYSTEM.statusFlags.word = (s32)(CD_SYSTEM.statusFlags.word | 0x10);
                        CdSyncCallback(cdrom_handle_recovery_sync);
                        CdReadyCallback((void (*)(u8, u8*))cdrom_verify_disc);
                        CD_SYSTEM.initCommand = 0x21U;
                        CD_SYSTEM.initState = 8U;
                        CdControlF(CdlReadN, (u8*)0x801ED95C);
                        CD_SYSTEM.vsyncTimestamp = (s32)(CD_SYSTEM.vsyncTimestamp - 0x1E);
                        break;

                    case 8: // Reading state - handle timeouts
                        cdSystem = &CD_SYSTEM;
                        if (cdSystem->syncComplete == 1)
                        {
                            cdSystem->vsyncTimestamp = VSync(-1);
                            cdSystem->syncComplete = 0U;
                        }
                        else if (VSync(-1) >= ((s32)CD_SYSTEM.vsyncTimestamp + 270))
                        {
                            // Timeout occurred - check what command to retry
                            initCommandByte = CD_SYSTEM.initCommand;
                            initCommand = initCommandByte & 0xFF;

                            if (initCommand == 0x22)
                            {
                                goto RetryPause;
                            }

                            if (initCommand < 0x23)
                            {
                                goto RetryRead;
                            }

                            if (0x23 == initCommand)
                            {
                                goto RetrySetmode;
                            }

                        RetryRead: // Retry read command
                            CdSyncCallback(cdrom_handle_recovery_sync);
                            CdReadyCallback((void (*)(u8, u8*))cdrom_verify_disc);

                            CD_SYSTEM.initCommand = 0x21;
                            cdCommand = CdlReadN;
                            cdCommandParams = (u8*)0x801ED95C;
                            goto ExecuteCommand;

                        RetryPause: // Retry pause command
                            CdSyncCallback(cdrom_handle_recovery_sync);
                            cdCommand = CdlPause;
                            cdCommandParams = 0;
                            goto ExecuteCommand;

                        RetrySetmode: // Retry setmode command
                            CdSyncCallback(cdrom_handle_recovery_sync);
                            cdCommand = CdlSetmode;
                            cdCommandParams = (u8*)0x801ED950;

                        ExecuteCommand: // Common command execution
                            CdControlF(cdCommand, cdCommandParams);
                            CD_SYSTEM.vsyncTimestamp -= 30;
                        }
                        break;

                    case 32: // Error recovery - pause
                        do
                        {

                        } while (CdControlB(8U, 0, 0) == 0);
                        g_initState = 0x21;
                        break;
                    }
                }
                else
                {
                    goto ErrorRecovery;
                }
            }
            else
            {
            ErrorRecovery: // Handle CD errors
                cdSystem = &CD_SYSTEM;
                if ((u8)g_initState >= 6U)
                {
                    // Clear busy flag and reset callbacks
                    CD_SYSTEM.statusFlags.word = (s32)(CD_SYSTEM.statusFlags.word & ~0x10);
                    CdSyncCallback(0);
                    CdReadyCallback(0);
                    do
                    {

                    } while (CdControlB(CdlPause, 0, 0) == 0);
                    cdSystem = &CD_SYSTEM;
                    cdSystem->initCommand = 0U;
                }
                CD_SYSTEM.initState = 1U;
                flagsForUpdate = (CD_SYSTEM.statusFlags.word | 1) & ~2;

                do
                {
                    flagsMask = -5;
                } while (0);

            UpdateStatusFlags: // Update status flags with mask
                cdSystem->statusFlags.word = (s32)(flagsForUpdate & flagsMask);
            }
        }
    }
    else
    {
        // Branch 2: Idle state - handle queued commands
        syncCompleteFlag = 0;
        currentCommand = CD_SYSTEM.currentCommand;

        if ((currentCommand != 0) || (CD_SYSTEM.initCommand != 0))
        {
            // Process sync completion and update queue
            while (1)
            {
                if (CD_SYSTEM.syncComplete == 1)
                {
                    syncCompleteFlag = 1;
                    CD_SYSTEM.syncComplete = 0U;
                }
                queueReadIndex2 = CD_SYSTEM.queueReadIndex;
                indexDiff = (CD_SYSTEM.queueWriteIndex - queueReadIndex2) & 0xF;

                if (indexDiff != 0)
                {
                    CD_SYSTEM.currentResourceIndex =
                        (u16)CD_SYSTEM.commandQueue.items[CD_SYSTEM.queueReadIndex].resourceIndex;
                    CD_SYSTEM.currentDataSize =
                        (s32)(CD_SYSTEM.commandQueue.items[CD_SYSTEM.queueReadIndex].entry)->dataSize;
                    CD_SYSTEM.targetDataSize = (s32)CD_SYSTEM.readRemainingBytes;
                }

                if (CD_SYSTEM.syncComplete == 0)
                {
                    break;
                }
            }

            // Check for command timeout
            if (syncCompleteFlag == 0)
            {
                if (VSync(-1) >= (s32)(CD_SYSTEM.vsyncTimestamp + 240))
                {
                    if (CD_SYSTEM.initCommand == 0)
                    {
                        CD_SYSTEM.currentCommand = 1U;

                        if (CD_SYSTEM.transferCallback != NULL)
                        {
                            CD_SYSTEM.playbackState = 1;
                        }
                        else
                        {
                            CD_SYSTEM.playbackState = 0;
                        }

                        CdSyncCallback((void (*)(u8, u8*))cdrom_complete_command);
                        CdReadyCallback(0);
                        do
                        {

                        } while (CdControlB(CdlNop, 0, (u8*)0x801ED960) == 0);
                    }
                    else
                    {
                        CdSyncCallback(cdrom_handle_recovery_sync);
                        CdReadyCallback(0);
                        do
                        {

                        } while (CdControlB(CdlNop, 0, (u8*)0x801ED960) == 0);
                    }
                }
            }

            g_cdVSyncTimestamp = VSync(-1);
            g_cdPendingQueueCount = indexDiff;
        }
        else if (CD_SYSTEM.queueReadIndex != CD_SYSTEM.queueWriteIndex)
        {
            // Queue has items - start processing
            CD_SYSTEM.vsyncTimestamp = VSync(-1);
            CD_SYSTEM.currentCommand = 1U;
            CD_SYSTEM.statusFlags.word = (s32)(CD_SYSTEM.statusFlags.word | 0x10);

            if (CD_SYSTEM.transferCallback != NULL)
            {
                CD_SYSTEM.playbackState = 1;
            }
            else
            {
                CD_SYSTEM.playbackState = 0;
            }

            CdSyncCallback((void (*)(u8, u8*))cdrom_complete_command);
            CdReadyCallback(0);
            CdSync(0, 0);
            CdControlF(CdlNop, 0);
            indexDiff = (CD_SYSTEM.queueWriteIndex - CD_SYSTEM.queueReadIndex) & 0xF;
        }
        else
        {
            // Queue is empty - idle state
            CD_SYSTEM.transferCallback = NULL;
            CD_SYSTEM.playbackState = 0;

            if (!(statusFlags & 0x20))
            {
                // Periodic status check
                if (VSync(-1) >= (s32)(CD_SYSTEM.vsyncTimestamp + 30))
                {
                    if (CdControlB(CdlNop, 0, (u8*)0x801ED960) != 0)
                    {
                        if (CD_SYSTEM.statusByte & 0x10)
                        {
                            cdrom_handle_sync_error();
                        }
                        CD_SYSTEM.syncComplete = 0U;
                        CD_SYSTEM.retryCounter = 0U;
                        CD_SYSTEM.vsyncTimestamp = VSync(-1);
                    }
                    else
                    {
                        // Retry counter for failed status checks
                        currentCommand = CD_SYSTEM.retryCounter;
                        CD_SYSTEM.retryCounter = (u8)(currentCommand + 1);
                        if ((u32)(currentCommand & 0xFF) >= 0xBU)
                        {
                            cdrom_handle_sync_error();
                        }
                    }
                }
            }
            indexDiff = 0;
            g_cdPendingQueueCount = 0;
        }
    }

    // Update audio system if enabled
    if (g_cdAudioEnabled != 0)
    {
        FUN_80140d48();
    }

    return indexDiff;
}

s32 cdrom_recover(void)
{
    u_char filterParams[2];
    s32 timestamp;
    u8 initCommandByte;

    // Bail out if bit 3 (recovery mode) is not set
    if (!(CD_SYSTEM.statusFlags.word & 8))
    {
        return 1;
    }

    switch (CD_SYSTEM.initState)
    {
    case 0:
        // State 0: Flush pending CD commands and advance to state 1
        CdFlush();
        CD_SYSTEM.initState = 1U;
        CD_SYSTEM.vsyncTimestamp = (s32)(VSync(-1) + 1);
        break;
    case 1:
        // State 1: Wait for 1-frame delay, then configure CD mode
        timestamp = VSync(-1);
        if (timestamp >= (s32)CD_SYSTEM.vsyncTimestamp)
        {
            // Set mode to 0xA0 (double speed + 2340-byte sectors)
            CD_SYSTEM.setModeParamAsync[0] = (CdlModeSpeed | CdlModeSize1);
            CD_SYSTEM.setModeParamAsync[1] = 0;
            CD_SYSTEM.setModeParamAsync[2] = 0;
            CD_SYSTEM.setModeParamAsync[3] = 0;

            CdSyncCallback(cdrom_handle_recovery_sync);

            CdReadyCallback(NULL);
            // initCommand 0x10 = pending setfilter; must be stored before CdControlF (not in delay slot)
            CD_SYSTEM_V.initCommand = 0x10U;
            CdControlF(CdlSetmode, (u8*)0x801ED954);
            timestamp = VSync(-1);
            CD_SYSTEM.vsyncTimestamp = (s32)(timestamp + 4);
        }
        break;
    case 2:
        // State 2: Send CdlSetfilter with file=1, channel=1, advance to state 3
        CdSyncCallback(cdrom_handle_recovery_sync);
        CD_SYSTEM.initCommand = 0x11;

        filterParams[0] = 1;
        filterParams[1] = 1;
        CdControlF(CdlSetfilter, filterParams);
        CD_SYSTEM.initState = 3U;
        CD_SYSTEM.vsyncTimestamp = VSync(-1);
        break;
    case 3:
        // State 3: Wait for syncComplete or 30-frame timeout, then dispatch
        if (CD_SYSTEM.syncComplete == 1)
        {
            CD_SYSTEM.vsyncTimestamp = VSync(-1);
            CD_SYSTEM_V.syncComplete = 0U;
            break;
        }

        timestamp = VSync(-1);
        if (timestamp < (s32)(CD_SYSTEM.vsyncTimestamp + 30))
        {
            break;
        }

        // Timeout expired — dispatch follow-up command based on initCommand
        CdSyncCallback(cdrom_handle_recovery_sync);

        initCommandByte = CD_SYSTEM.initCommand;

        switch (initCommandByte)
        {
        case 0x0:
        default:
            filterParams[0] = 1;
            filterParams[1] = 1;
            CdControlF(CdlSetfilter, filterParams);
            CD_SYSTEM_V.initCommand = 0x10U;
            break;
        case 0x11:
            CdControlF(CdlDemute, NULL);
            break;
        case 0x12:
            CdControlF(CdlPause, NULL);
            break;
        }

        // Subtract 30 to allow immediate re-entry on the next timeout cycle
        CD_SYSTEM.vsyncTimestamp -= 30;
        break;
    }

    return 0;
}

void cdrom_verify_recovery(void)
{
    volatile CdSystem* cdSystem;
    volatile CdSystem** new_var;

    cdSystem = &CD_SYSTEM;

    if ((u8)g_cdStatusByte3 != 1)
    {
        return;
    }

    if (cdSystem->audioEnabled != (u8)g_cdStatusByte3)
    {
        while (CdGetSector((void*)0x801ED940, 3) == 0);

        if ((CD_SYSTEM.sectorHeaderBuffer[0] & 0xFFFFFF) == (CD_SYSTEM.currentLocation.raw & 0xFFFFFF))
        {
            cdrom_process_sector(1);
            return;
        }

        if (CD_SYSTEM.retryCount++ <= 16)
        {
            CdControlF(CD_SYSTEM.currentCommand, (u8*)0x801ED958);
        }
        else
        {
            CD_SYSTEM.statusFlags.bytes.retryExhausted = 1;
            CD_SYSTEM.retryCount = 0U;
            if (CD_SYSTEM.transferCallback != NULL)
            {
                CD_SYSTEM.playbackState = 1;
            }
            else
            {
                CD_SYSTEM.playbackState = 0;
            }
            cdSystem = &CD_SYSTEM;
            (*(new_var = &cdSystem))->currentCommand = 1U;
            CdControlF(1U, NULL);
        }
    }
    else
    {
        cdrom_process_sector(1);
    }

    g_cdStatusByte3 = 0;
}

void cdrom_complete_command(u_char intr, u_char* result)
{
    u8 nextCommand;
    u32 readIndex;
    u32 writeIndex;
    u8 nopCommand;
    s32 statusFlags;
    volatile s32 vsyncArg;
    volatile CdSystem* cdSystem;

    // Signal to main-loop poller that a sync event occurred
    CD_SYSTEM.syncComplete = 1;

    // If we sent CdlNop to probe intr, check for drive errors
    if (CD_SYSTEM.currentCommand == 1)
    {
        if (*result & 0x10)
        {
            cdrom_handle_sync_error();
            return;
        }
    }

    // If the command did not complete successfully, handle retry/fallthrough
    if ((intr & 0xFF) != CdlComplete)
    {
        goto HandleIncomplete;
    }

    // Command completed — dispatch based on which command just finished
    cdSystem = &CD_SYSTEM;
    switch (cdSystem->currentCommand)
    {
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
        cdSystem = &CD_SYSTEM;
        nextCommand = cdSystem->commandQueue.items[CD_SYSTEM.queueReadIndex].command;

        // Skip past consecutive CdlNop (1) entries in the queue
        if (nextCommand == 1)
        {
            writeIndex = CD_SYSTEM.queueWriteIndex;
            nopCommand = 1;
            do
            {
                readIndex = CD_SYSTEM.queueReadIndex;
                if (readIndex == writeIndex)
                {
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
        CD_SYSTEM.transferCallback = NULL;
        readIndex = (CD_SYSTEM.queueReadIndex + 1) & 0xF;
        CD_SYSTEM.queueReadIndex = readIndex;

        // If queue is now empty after pause, clean up and return
        if (readIndex == CD_SYSTEM.queueWriteIndex)
        {
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
    // cdSystem = &CD_SYSTEM; // this seems to be necessary SOMEWHERE in order to force loading 801ed800, but I don't
    // know where.

    // Special case: command CdlReadS — used for CD-DA/XA streaming
    if (nextCommand == CdlReadS)
    {
        cdSystem = &CD_SYSTEM;
        if (g_cdAudioEnabled == 0)
        {
            CD_SYSTEM.audioEnabled = 1;
        }
        nextCommand = CdlReadN;
    }
    goto ExecuteNext;

HandleIncomplete:
    // Command did not complete — if not already probing with CdlNop, retry with CdlNop
    cdSystem = &CD_SYSTEM;
    if (cdSystem->currentCommand != 1)
    {
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
    CD_SYSTEM.transferCallback = NULL;
    CD_SYSTEM.currentCommand = 0;
    CD_SYSTEM.retryCounter = 0;

ApplyFlagsAndTimestamp:
    CD_SYSTEM.statusFlags.word = statusFlags;
    CD_SYSTEM.vsyncTimestamp = VSync(vsyncArg);
    return;

ReadQueueHead:
    nextCommand = CD_SYSTEM.commandQueue.items[CD_SYSTEM.queueReadIndex].command;

ExecuteNext:
    cdrom_run_command(nextCommand, 0, 0);
}

void cdrom_handle_recovery_sync(u_char intr, u_char* result)
{
    u8 sp10;
    s8 sp11;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 var_v1;
    s8 var_v0_2;
    s8 var_v0_3;
    u8 temp_a0;
    u8 temp_v0;
    u8 temp_v0_2;
    u8 var_a0;
    u8 var_a0_2;
    u8 var_v0;
    u8* var_a1;
    CdSystem* cdSystem;

    CD_SYSTEM.syncComplete = 1;
    if (CD_SYSTEM.initCommand & 0x80)
    {
        if (!(CD_SYSTEM.statusFlags.word & 8))
        {
            cdSystem = &CD_SYSTEM;
            if (*result & 0x10)
            {
                cdrom_handle_sync_error();
                return;
            }
            goto block_6;
        }
        goto block_5;
    }
block_5:
    cdSystem = &CD_SYSTEM;
block_6:
    var_v1 = intr & 0xFF;
    if (((cdSystem->initCommand & 0x7F) == 0x21) && (cdSystem->statusByte & 1))
    {
        if (cdSystem->filterModeFlags & 0x40)
        {
            CdSyncCallback(NULL);
            CdReadyCallback(NULL);
            cdSystem->initState = 0x20;
            cdSystem->initCommand = 0U;
            cdSystem->statusFlags.word = (s32)(cdSystem->statusFlags.word & ~0x10 & ~4);
            var_v1 = intr & 0xFF;
        }
    }
    if (var_v1 == 2)
    {
        CD_SYSTEM.initCommand = (u8)(CD_SYSTEM.initCommand & 0x7F);
        temp_v0 = CD_SYSTEM.initCommand;
        switch (temp_v0)
        {       /* switch 1 */
        case 1: /* switch 1 */
        case 3: /* switch 1 */
            CD_SYSTEM.initCommand = 0U;
            if (CD_SYSTEM.queueReadIndex != CD_SYSTEM.queueWriteIndex)
            {
                CdSyncCallback(cdrom_complete_command);
                temp_a0 = CD_SYSTEM.commandQueue.items[CD_SYSTEM.queueReadIndex].command;
                if ((temp_a0 == 0x1B) && (CD_SYSTEM.audioEnabled == 0))
                {
                    CD_SYSTEM.audioEnabled = 1U;
                }
                CD_SYSTEM.playbackState = 0;
                CD_SYSTEM.transferCallback = NULL;
                cdrom_run_command(temp_a0 & 0xFF, 0, 0);
            }
            else
            {
                CdSyncCallback(NULL);
            }
            break;
        case 2: /* switch 1 */
            var_a0 = 0xE;
            var_v0 = CD_SYSTEM.initCommand;
            var_a1 = (u8*)0x801ED950;
        block_25:
            CD_SYSTEM.initCommand = (u8)(var_v0 + 1);
            CdControlF(var_a0, var_a1);
            break;
        case 16: /* switch 1 */
            var_v0_2 = 2;
        block_29:
            CD_SYSTEM.initState = var_v0_2;
            CdSyncCallback(NULL);
            CD_SYSTEM.initCommand = 0U;
            break;
        case 17: /* switch 1 */
            var_a0 = 0xC;
        block_24:
            var_v0 = CD_SYSTEM.initCommand;
            var_a1 = NULL;
            goto block_25;
        case 18: /* switch 1 */
            var_a0 = 9;
            goto block_24;
        case 19: /* switch 1 */
            CdSyncCallback(NULL);
            CD_SYSTEM.initState = 0;
            CD_SYSTEM.initCommand = 0U;
            CD_SYSTEM.statusFlags.word = (s32)(CD_SYSTEM.statusFlags.word & ~8);
            break;
        case 33: /* switch 1 */
            CdSyncCallback(NULL);
            CD_SYSTEM.initCommand = 0U;
            break;
        case 32: /* switch 1 */
        case 34: /* switch 1 */
            var_v0_2 = 7;
            goto block_29;
        case 35: /* switch 1 */
            CD_SYSTEM.initCommand = 0U;
            CD_SYSTEM.initState = 0;
            CD_SYSTEM.retryCounter = 0;
            temp_v1 = CD_SYSTEM.statusFlags.word & ~1;
            CD_SYSTEM.statusFlags.word = temp_v1;
            temp_v1_2 = temp_v1 & ~2 & ~4;
            CD_SYSTEM.statusFlags.word = temp_v1_2;
            if (CD_SYSTEM.queueReadIndex != CD_SYSTEM.queueWriteIndex)
            {
                CD_SYSTEM.currentCommand = 1;
                CD_SYSTEM.statusFlags.word = (s32)(temp_v1_2 | 0x10);
                CdSyncCallback(cdrom_complete_command);
                CdSync(0, NULL);
                CdControlF(1U, NULL);
            }
            else
            {
                CdSyncCallback(NULL);
                CD_SYSTEM.statusFlags.word = (s32)(CD_SYSTEM.statusFlags.word & ~0x10);
            }
            if ((g_cdAudioEnabled != 0) && (g_cdAudioReady != 0))
            {
                AUDIO_SYSTEM.readFlag = 1;
            }
            break;
        }
        g_cdVSyncTimestamp = VSync(-1);
        return;
    }
    if (!(CD_SYSTEM.initCommand & 0x80))
    {
        CD_SYSTEM.initCommand = (u8)(CD_SYSTEM.initCommand | 0x80);
        CdControlF(1U, NULL);
        return;
    }
    CD_SYSTEM.initCommand = (u8)(CD_SYSTEM.initCommand & 0x7F);
    temp_v0_2 = CD_SYSTEM.initCommand;
    switch (temp_v0_2)
    {       /* switch 2 */
    case 3: /* switch 2 */
        CdControlF(0xEU, (u8*)0x801ED950);
        return;
    case 16: /* switch 2 */
        CD_SYSTEM.initState = 1;
        CdSyncCallback(NULL);
        CD_SYSTEM.initCommand = 0U;
        return;
    case 17: /* switch 2 */
        sp10 = 1;
        sp11 = 1;
        CdControlF(0xDU, &sp10);
        return;
    case 18: /* switch 2 */
        var_a0_2 = 0xC;
    block_46:
        CdControlF(var_a0_2, NULL);
        return;
    case 1:  /* switch 2 */
    case 2:  /* switch 2 */
    case 19: /* switch 2 */
        var_a0_2 = 9;
        goto block_46;
    case 33: /* switch 2 */
        CdControlF(6U, (u8*)0x801ED95C);
        return;
    case 34: /* switch 2 */
        var_v0_3 = 7;
    block_50:
        CD_SYSTEM.initState = var_v0_3;
        CD_SYSTEM.initCommand = 0U;
        CdSyncCallback(NULL);
    default: /* switch 2 */
        return;
    case 32: /* switch 2 */
    case 35: /* switch 2 */
        var_v0_3 = 6;
        goto block_50;
    }
}

void cdrom_handle_ready_intr(u_char intr, u_char* result)
{
    s32 temp_a0;
    u8 temp_s1;
    u8 temp_v0;
    int temp2;
    u8 temp_v0_2;
    u8 temp_v0_3;
    u8 var_a0;
    u8* var_a1;
    u8* addr;
    int new_var;

    volatile CdSystem* cdSystem;

    CD_SYSTEM.syncComplete = 1;
    temp_s1 = CD_SYSTEM.audioEnabled;
    if (temp_s1 != 1)
    {

        temp_a0 = intr & 0xFF;
        if ((temp_a0 == 1) && (CD_SYSTEM.statusFlags.bytes.b2 == 0))
        {
            temp_v0 = CD_SYSTEM.statusFlags.bytes.b1;
            temp2 = temp_v0 & 0xFF;

            if (temp2 == temp_a0)
            {
                CD_SYSTEM.statusFlags.bytes.b2 = temp2;
                return;
            }

            do
            {

            } while (CdGetSector((void*)0x801ED940, 3) == 0);

            if ((CD_SYSTEM.sectorHeaderBuffer[0] & 0xFFFFFF) == (CD_SYSTEM.currentLocation.raw & 0xFFFFFF))
            {
                cdrom_process_sector(0);
                return;
            }
        }

        temp_v0_2 = CD_SYSTEM.retryCount;
        CD_SYSTEM.retryCount = (u8)(temp_v0_2 + 1);
        if ((u32)(temp_v0_2 & 0xFF) < 0x11U)
        {
            var_a1 = (u8*)0x801ED958;
            var_a0 = CD_SYSTEM.currentCommand;
            CdControlF(var_a0, var_a1);
            return;
        }

        CD_SYSTEM.statusFlags.bytes.retryExhausted = 1U;
        CD_SYSTEM.retryCount = 0U;

        if (CD_SYSTEM.transferCallback != NULL)
        {
            CD_SYSTEM.playbackState = 1U;
        }
        else
        {
            CD_SYSTEM.playbackState = 0U;
        }

        CdReadyCallback(NULL);
        cdSystem = &CD_SYSTEM;
        cdSystem->currentCommand = 1U;
        var_a0 = 1;
        var_a1 = NULL;
        CdControlF(var_a0, var_a1);
        return;
    }

    new_var = intr & 0xFF;
    temp2 = temp_s1;
    if (new_var == temp2)
    {
        var_a0 = D_801ED590 == 0;
        addr = (u8*)0x801ED500;
        if (var_a0 && ((*(((u8*)addr) + 0x9C)) != 0))
        {
            (*((CdSystem*)0x801ED800)).statusFlags.bytes.b2 = temp2;
            return;
        }
        cdrom_process_sector(0);
        return;
    }

    temp_v0_3 = CD_SYSTEM.retryCount;
    CD_SYSTEM.retryCount = (u8)(temp_v0_3 + 1);
    if ((u32)(temp_v0_3 & 0xFF) >= 0x11U)
    {
        (*((CdSystem*)0x801ED800)).statusFlags.bytes.retryExhausted = temp2;
        CD_SYSTEM.retryCount = 0U;
        (*((CdSystem*)0x801ED800)).playbackState = temp2;
        CdReadyCallback(NULL);
        var_a0 = 1;
        var_a1 = NULL;
        (*((CdSystem*)0x801ED800)).currentCommand = temp2;
        CdControlF(var_a0, var_a1);
    }
    return;
}

void cdrom_process_sector(s32 arg0)
{
    void* buffer;
    volatile CdSystem* cdSystem;

    cdSystem = &CD_SYSTEM;

    // Reset retry tracking and clear status flag bytes b2/retryExhausted
    CD_SYSTEM.retryCount = 0;
    CD_SYSTEM.statusFlags.bytes.retryExhausted = 0;
    CD_SYSTEM.statusFlags.bytes.b2 = 0;

    // === Data mode path ===
    if (CD_SYSTEM.audioEnabled != 1)
    {

        // Determine destination buffer via transferCallback callback or currentWritePtr
        if (CD_SYSTEM.transferCallback != NULL)
        {
            // transferCallback(bytesTransferred, bytesRemaining) returns destination buffer
            buffer = CD_SYSTEM.transferCallback(CD_SYSTEM.totalDataSize - CD_SYSTEM.readRemainingBytes,
                                                CD_SYSTEM.readRemainingBytes);
            if (buffer == NULL)
            {
                // Callback rejected the transfer — re-issue the current read command
                CdControlF(cdSystem->currentCommand, (u8*)0x801ED958);
                return;
            }
        }
        else
        {
            buffer = CD_SYSTEM.currentWritePtr;
        }

        // More than one sector remaining — read a full 0x800-byte sector
        if (CD_SYSTEM.readRemainingBytes >= 0x801U)
        {
            // Spin-wait until sector data is available (0x200 words = 0x800 bytes)
            while (CdGetSector(buffer, 0x200) == 0);

            // Advance disc position to next sector
            CdIntToPos(CdPosToInt((CdlLOC*)0x801ED958) + 1, (CdlLOC*)0x801ED958);

            // Decrease remaining byte count by one sector
            CD_SYSTEM.readRemainingBytes = (CD_SYSTEM.readRemainingBytes - 0x800);

            // If no callback, linearly advance the destination pointer
            if (CD_SYSTEM.transferCallback == NULL)
            {
                CD_SYSTEM.currentWritePtr = (void*)(CD_SYSTEM.currentWritePtr + 0x800);
            }
        }
        else
        {
            // === Final sector — complete the transfer ===
            CD_SYSTEM.playbackState = 0;
            CD_SYSTEM.transferCallback = NULL;

            // Advance queue read index (circular, mod 16)
            CD_SYSTEM.queueReadIndex = (CD_SYSTEM.queueReadIndex + 1) & 0xF;

            // If more commands are queued, dispatch the next one immediately
            if (CD_SYSTEM.queueReadIndex != CD_SYSTEM.queueWriteIndex)
            {
                cdrom_run_command(CD_SYSTEM.commandQueue.items[CD_SYSTEM.queueReadIndex].command, buffer, arg0 + 1);
                return;
            }

            // No more queued commands — transition to idle state
            CD_SYSTEM.initCommand = 1;
            CdSyncCallback(cdrom_handle_recovery_sync);
            CdReadyCallback(NULL);

            // If initial call (arg0 == 0), pause drive before reading final sector
            if (arg0 == 0)
            {
                CdControlF(CdlPause, NULL);
            }

            // Read the final partial sector (size converted from bytes to words)
            while (CdGetSector(buffer, (g_cdReadRemainingBytes + 3) >> 2) == 0);

            cdSystem = &CD_SYSTEM;

            // Clear busy flag (bit 4) and reset command/retry state
            CD_SYSTEM.statusFlags.word &= ~0x10;
            cdSystem->currentCommand = 0U;
            cdSystem->retryCounter = 0;

            // If chained call (arg0 != 0), pause drive after reading final sector
            if (arg0 != 0)
            {
                CdControlF(CdlPause, NULL);
            }

            // Record frame counter for timeout tracking
            CD_SYSTEM.vsyncTimestamp = VSync(-1);
        }

        return;
    }

    // === Audio (XA) mode path ===

    // Read 3 words (12 bytes) of sector header into read buffer
    while (CdGetSector(&CD_SECTOR_HEADER_BUFFER, 3) == 0);

    cdSystem = &CD_SYSTEM;

    // Verify disc position: compare lower 24 bits (min/sec/sector BCD)
    // of the read sector against the expected command parameter position
    if ((CD_SYSTEM.sectorHeaderBuffer[0] & 0xFFFFFF) == (CD_SYSTEM.currentLocation.raw & 0xFFFFFF))
    {

        // Position matches — invoke transferCallback callback to check if audio is complete
        if (CD_SYSTEM.transferCallback(CD_SYSTEM.totalDataSize - CD_SYSTEM.readRemainingBytes,
                                       CD_SYSTEM.readRemainingBytes) == NULL)
        {

            // Audio track complete — shut down audio playback
            CD_SYSTEM.queueReadIndex = ((CD_SYSTEM.queueReadIndex + 1) & 0xF);
            CdSyncCallback(cdrom_handle_recovery_sync);
            CdReadyCallback(NULL);

            // Restore default CD mode (double speed + 2340-byte sectors)
            cdSystem->setModeParamBlocking[0] = 0xA0;
            cdSystem->currentCommand = 0U;
            cdSystem->initCommand = 2;
            cdSystem->audioEnabled = 0U;
            cdSystem->playbackState = 0;
            cdSystem->transferCallback = NULL;
            cdSystem->retryCounter = 0;

            // Clear busy flag (bit 4) and pause drive
            CD_SYSTEM.statusFlags.word &= ~0x10;
            CdControlF(CdlPause, NULL);
            CD_SYSTEM.vsyncTimestamp = VSync(-1);
        }
        else
        {
            // Audio continues — advance disc position to next sector
            CdIntToPos(CdPosToInt((CdlLOC*)0x801ED958) + 1, (CdlLOC*)0x801ED958);
        }

        return;
    }

    // Position mismatch — re-issue read command with expected position
    CdControlF(cdSystem->currentCommand, (u8*)0x801ED958);
}

void cdrom_run_command(u8 cmd, void* sectorBuffer, s32 executionMode)
{
    u8* paramBufferSpecialCmd;
    s32 nextReadIndex;
    s32 dataSize;
    s32 controlParam;
    CdResourceEntry* queuedLocation;
    u32 queueEntryPtr;
    void* queueBufferPtr;
    volatile CdSystem* cdSystem;

    queuedLocation = 0;

    while (cmd == CdlSeekL)
    {
        // Calculate next read index with circular buffer wrapping
        nextReadIndex = (CD_SYSTEM_V.queueReadIndex + 1) & 0xF;

        // Stop once the process has iterated through all indices
        if (CD_SYSTEM.queueWriteIndex == nextReadIndex)
        {
            break;
        }

        // Advance read index and get next command
        CD_SYSTEM_V.queueReadIndex = nextReadIndex;
        cmd = CD_SYSTEM_V.commandQueue.items[nextReadIndex].command;
    }

    if ((cmd == 0x15) || (cmd == 0x06) || (cmd == 0x1B))
    {
        // Reset playback state and get queue location
        if (cmd == 0x15 || g_playbackState == 0)
        {
            CD_SYSTEM_V.transferCallback = NULL;
            CD_SYSTEM_V.playbackState = 0;
            queuedLocation = CD_SYSTEM_V.commandQueue.items[CD_SYSTEM.queueReadIndex].entry;
            CD_SYSTEM.currentLocation = queuedLocation->location;
        }

        switch (executionMode)
        {
        case 1:
            CD_SYSTEM_V.currentCommand = cmd;
            CdControlF(cmd, CD_COMMAND_PARAM_BUFFER);
            while (CdGetSector(sectorBuffer, (g_cdReadRemainingBytes + 3) >> 2) == 0);
            break;

        case 2:
            while (CdGetSector(sectorBuffer, (g_cdReadRemainingBytes + 3) >> 2) == 0);
            CdSync(0, 0);
            break;
        }

        if ((cmd == 0x06) || (cmd == 0x1B))
        {
            queueEntryPtr = (CD_SYSTEM_V.queueReadIndex * 0x10) + 0x801ED800;
            if (((*(((u32*)queueEntryPtr) + 0x13)) == 0) &&
                (CD_SYSTEM_V.currentWritePtr == *(void**)((u8*)queueEntryPtr + 0x48)))
            {
                CD_SYSTEM_V.playbackState = 0;
            }

            cdSystem = &CD_SYSTEM_V;
            if (g_playbackState == 0)
            {
                dataSize = queuedLocation->dataSize;
                queueBufferPtr = QUEUE_ITEM_BASE(cdSystem->queueReadIndex);
                CD_SYSTEM_V.totalDataSize = dataSize;
                CD_SYSTEM_V.readRemainingBytes = dataSize;
                CD_SYSTEM_V.currentWritePtr = (void*)QUEUE_ITEM_DST_BUFFER(queueBufferPtr);
                CD_SYSTEM_V.transferCallback = QUEUE_ITEM_CALLBACK(queueBufferPtr);
            }

            if (executionMode == 0)
            {
                CD_SYSTEM_V.statusFlags.bytes.b2 = 0;
                CdReadyCallback(cdrom_handle_ready_intr);
            }
        }
        else if (executionMode == 1)
        {
            CdReadyCallback(NULL);
        }

        if (executionMode != 1)
        {
            CD_SYSTEM_V.currentCommand = cmd;
            CdControlF(cmd, CD_COMMAND_PARAM_BUFFER);
        }

        g_playbackState = 0;
        return;
    }

    // Handle other commands based on execution mode
    switch (executionMode)
    {
    case 0:
        CD_SYSTEM_V.currentCommand = cmd;

        if (cmd == 0xE)
        {
            controlParam = 0xE;
            paramBufferSpecialCmd = (u8*)0x801ED950;
        }
        else
        {
            controlParam = cmd;
            paramBufferSpecialCmd = 0;
        }
        break;

    case 1:
        CdReadyCallback(0);
        CD_SYSTEM_V.currentCommand = cmd;
        controlParam = 0;
        CdControlF(cmd, 0);
        while (CdGetSector(sectorBuffer, (g_cdReadRemainingBytes + 3) >> 2) == 0);
        return;

    case 2:
        while (CdGetSector(sectorBuffer, (g_cdReadRemainingBytes + 3) >> 2) == 0);
        CD_SYSTEM_V.currentCommand = cmd;
        controlParam = cmd;
        paramBufferSpecialCmd = 0;
        break;

    default:
        return;
    }

    CdControlF(controlParam, paramBufferSpecialCmd);
}

void cdrom_verify_disc(u_char intr, u_char* result)
{
    s32 statusWord;
    u_char expectedChar;
    u_char discChar;
    const u_char* expectedId;
    u_char* discId;

    CD_SYSTEM_V.syncComplete = TRUE;

    if (intr == 1)
    {
        // Wait until the sector header (3 words) is ready
        while ((expectedChar = (CdGetSector(&CD_SYSTEM.sectorHeaderBuffer, 3) == 0)));

        // Verify the sector position matches the expected recovery read position (24-bit compare)
        if ((CD_SYSTEM.sectorHeaderBuffer[0] & 0xFFFFFF) == (CD_SYSTEM.recoveryReadPosition.raw & 0xFFFFFF))
        {
            // Wait until the disc validation ID (8 words) is ready
            while (CdGetSector(&CD_SYSTEM.discValidationId, 8) == 0);

            // Walk both strings simultaneously, verifying the disc contains the expected ID.
            // Shift-JIS lead bytes (0x81-0x9F or 0xE0-0xEF) introduce two-byte characters;
            // both the lead and trail byte must match before continuing.
            expectedId = g_DiscValidationId;
            discId = CD_SYSTEM.discValidationId;
            expectedChar = *expectedId++;

            while (expectedChar != 0)
            {
                // Special-range characters [0x80,0x9F] or [0xE0,0xEF] use a two-byte match
                // expectedChar must equal *pDiscData, then the following bytes are compared.
                if (((u8)(expectedChar + 0x80) < 0x20u) || ((u8)(expectedChar + 0x20) < 0x10u))
                {
                    // Two-byte character: verify the lead byte matches first
                    if (expectedChar != *discId++)
                    {
                        goto validation_failed;
                    }

                    // Then load the trail bytes from each string for the main compare below
                    expectedChar = *discId++;
                    discChar = *expectedId++;
                }
                else
                {
                    // Single-byte ASCII character: just advance the disc pointer
                    discChar = *discId++;
                }

                if (expectedChar != discChar)
                {
                validation_failed:
                    statusWord = CD_SYSTEM.statusFlags.word & ~4u;
                    CD_SYSTEM_V.initState = CD_INIT_STATE_ERROR_PAUSE;
                    CD_SYSTEM_V.statusFlags.word = statusWord;
                    CD_SYSTEM.statusFlags.word = statusWord & ~CdlStatShellOpen;
                    CdReadyCallback(NULL);
                    return;
                }

                expectedChar = *expectedId++;
            }

            // Validation passed: set disc mode and begin CD reads
            CdReadyCallback(NULL);
            CD_SYSTEM_V.initCommand = 0x23; // TODO: name this constant
            CdSyncCallback(cdrom_handle_recovery_sync);
            CdControlF(CdlSetmode, (u_char*)0x801ED950);
            return;
        }
    }

    // Sector position mismatch or wrong interrupt type: pause and signal error
    CdReadyCallback(NULL);
    CD_SYSTEM_V.initCommand = 0x22; // TODO: name this constant
    CdSyncCallback(cdrom_handle_recovery_sync);
    CdControlF(CdlPause, NULL);
}

void cdrom_wait_queue_empty(void)
{
    int remaining;

    while (remaining = cdrom_process_state(), remaining != 0)
    {
        VSync(0);
    }
}

void cdrom_handle_sync_error(void)
{
    CdSyncCallback(NULL);
    CdReadyCallback(NULL);

    CD_SYSTEM.initState = 0;
    CD_SYSTEM.statusFlags.word |= 1;
    CD_SYSTEM.currentCommand = 0;
    CD_SYSTEM.initCommand = 0;
    CD_SYSTEM.retryCount = 0;
    CD_SYSTEM.retryCounter = 0;
    CD_SYSTEM.statusFlags.word &= ~0x10;
    CD_SYSTEM.vsyncTimestamp = VSync(-1);
}

void cdrom_set_audio_volume(u_char volume, s32 mixMode)
{
    CdlATV audioConfig[2];

    while (TRUE)
    {
        if (mixMode != 0)
        {
            audioConfig[0].val0 = volume;
            audioConfig[0].val1 = 0;
            audioConfig[0].val2 = volume;
        }
        else
        {
            audioConfig[0].val0 = volume;
            audioConfig[0].val1 = volume;
            audioConfig[0].val2 = 0;
        }

        audioConfig[0].val3 = 0;
        break;
    }

    CdMix(audioConfig);
}

void cdrom_reset(void)
{
    AudioSystem* audioSystem = &AUDIO_SYSTEM;

    DecDCToutCallback(audioSystem->decDCToutCallbackHandler);
    DrawSyncCallback(audioSystem->drawSyncCallbackHandler);

    CdSyncCallback(NULL);
    CdReadyCallback(NULL);

    while (CdControlB(CdlPause, NULL, NULL) == 0);

    if (g_cdAudioReady != 0)
    {
        FUN_80023010();
    }

    CD_SYSTEM.audioEnabled = 0;
    CD_SYSTEM.currentCommand = 0;
    CD_SYSTEM.initCommand = 0;
    CD_SYSTEM.queueReadIndex = 0;
    CD_SYSTEM.queueWriteIndex = 0;
    CD_SYSTEM.retryCounter = 0;
    CD_SYSTEM.playbackState = 0;
    CD_SYSTEM.transferCallback = NULL;
    CD_SYSTEM.statusFlags.word &= ~0x10;
    CD_SYSTEM.vsyncTimestamp = VSync(-1);
}

/**
 * @brief Checks whether a resource index is absent from the pending command queue
 *
 * Scans every pending entry in the circular command queue and returns whether
 * the given resource index is not already present, indicating it is safe to
 * enqueue a new command for that resource without creating a duplicate.
 *
 * @details
 * The scan performs the following steps:
 *
 * 1. Reads queueReadIndex as the starting scan position
 * 2. Computes the number of pending entries as (queueWriteIndex - queueReadIndex) & 0xF
 * 3. Decrements that count by 1 and compares against a sentinel of -1 to detect an
 *    empty queue (no iterations performed)
 * 4. For each pending slot, compares the stored resourceIndex against the lower 16
 *    bits of the argument; returns 0 immediately on a match (duplicate found)
 * 5. Advances scanIndex by masking with 0xF before incrementing to maintain circular
 *    wrap semantics within the 16-entry buffer
 * 6. Returns 1 if the full queue was scanned with no match
 *
 * @note
 * - Only the lower 16 bits of resourceIndex are compared, matching the u16 storage
 *   in CdCommandQueueItem
 * - The decrement-before-loop pattern and sentinel value of -1 match the original
 *   assembly's register usage exactly and must not be restructured
 * - The mask-then-increment sequence (scanIndex = (scanIndex & 0xF) + 1) matches
 *   the original assembly's andi + addiu pair for register-level equivalence
 *
 * @warning
 * - Not interrupt-safe; the queue indices and entries may change between reads if
 *   called while a CD callback is active
 * - Does not prevent a race between this check and a subsequent cdrom_queue_command call;
 *   the caller must not assume the result remains valid across VSync frames
 *
 * @param resourceIndex  Resource index to search for in the queue (lower 16 bits used)
 * @return 1 if the resource index is not already queued (safe to enqueue),
 *         0 if a matching entry was found (duplicate present)
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/l4HlL
 */
s32 cdrom_can_queue_resource(s32 resourceIndex)
{
    s32 queuedResourceIndex;
    s32 scanIndex;
    s32 remainingEntries;

    scanIndex = CD_SYSTEM.queueReadIndex;

    // Calculate number of pending entries in the circular queue
    remainingEntries = ((CD_SYSTEM.queueWriteIndex - scanIndex) & 0x0F);

    // If queue is non-empty, scan all pending entries for a match
    while (--remainingEntries != -1)
    {

        // Check if this queued entry already targets the same resource
        queuedResourceIndex = CD_SYSTEM.commandQueue.items[scanIndex].resourceIndex;

        if ((resourceIndex & 0xFFFF) == queuedResourceIndex)
        {
            return 0;
        }

        // Advance scan index with circular wrap (mod 16)
        scanIndex &= 0xF;
        scanIndex++;
    }

    return 1;
}

void cdrom_load_resource_table(s32 lba, s32 dataSizeBytes)
{
    CdlLOCRaw* location;
    int vsyncOffset;
    int vsyncDelta;
    CdSystem* cdStruct;

    vsyncOffset = -3;
    vsyncDelta = g_cdVSyncTimestamp - (VSync(-1) + vsyncOffset);

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
    cdStruct->defaultCdResource.location.raw = 0;
    cdStruct->defaultCdResource.dataSize = dataSizeBytes;

    CdIntToPos(lba, &location->pos);
    cdrom_queue_command(CdlReadN, CD_RESOURCE_INDEX_DEFAULT, CD_RESOURCE_ENTRIES, NULL);
    cdrom_wait_queue_empty();
    cdrom_set_audio_volume(128, 1);
}

/**
 * decomp.me link: (100%) https://decomp.me/scratch/OxunQ
 */
void cdrom_queue_read(s32 resourceIndex, void* dstBuffer)
{
    cdrom_queue_command(CdlReadN, resourceIndex, dstBuffer, 0);
}

/**
 * decomp.me link: (100%) https://decomp.me/scratch/5M5cV
 */
void cdrom_queue_read_with_callback(s32 resourceIndex, CdCommandCallback callback)
{
    cdrom_queue_command(CdlReadN, resourceIndex & 0xFFFF, 0, callback);
}

/**
 * decomp.me link: (100%) https://decomp.me/scratch/iUUQh
 */
void cdrom_queue_seek(s32 resourceIndex)
{
    cdrom_queue_command(CdlSeekL, resourceIndex, 0, 0);
}

/**
 * decomp.me link: (100%) https://decomp.me/scratch/SGZF5
 */
s32 cdrom_get_resource_size(s32 resourceIndex)
{
    return CD_RESOURCE_ENTRIES[resourceIndex & 0xffff].dataSize;
}

/**
 * decomp.me link: (100%) https://decomp.me/scratch/vfLUw
 */
s32 cdrom_get_error_status(void)
{
    CdStatusFlags flags;

    flags = CD_SYSTEM.statusFlags;

    if (flags.bytes.b0 & 1)
    {
        return 1;
    }

    if ((flags.bytes.b0 & 2) != 0)
    {
        if (flags.bytes.b0 & 4)
        {
            return 2;
        }

        return 3;
    }

    if (flags.bytes.b0 & 4)
    {
        return 4;
    }

    if (CD_SYSTEM.statusFlags.bytes.retryExhausted == 1)
    {
        return 5;
    }

    return 0;
}

/**
 * decomp.me link: (100%) https://decomp.me/scratch/HSXMR
 */
void cdrom_restore_callbacks(void)
{
    CdSyncCallback(CD_SYSTEM.previousSyncCallback);
    CdReadyCallback(CD_SYSTEM.previousReadyCallback);

    while (CdControlB(9U, NULL, NULL) == 0);

    CD_SYSTEM.resourceIndex = 0xFFFE;
    CD_SYSTEM.pendingQueueCount = 0;
    CD_SYSTEM.currentResourceIndex = 0;
    CD_SYSTEM.currentDataSize = 0;
    CD_SYSTEM.targetDataSize = 0;
    CD_SYSTEM.playbackState = 0;
    CD_SYSTEM.transferCallback = NULL;
    CD_SYSTEM.currentCommand = 0;
    CD_SYSTEM.initCommand = 0;
    CD_SYSTEM.retryCount = 0;
    CD_SYSTEM.retryCounter = 0;
    CD_SYSTEM.lastCommand = 0;
    CD_SYSTEM.dstBuffer = 0;
    CD_SYSTEM.callback = NULL;
    CD_SYSTEM.statusFlags.word &= ~0x10;
    CD_SYSTEM.statusFlags.bytes.b1 = 0;
    CD_SYSTEM.statusFlags.bytes.b2 = 0;
    CD_SYSTEM.vsyncTimestamp = VSync(-1);
    CD_SYSTEM.queueReadIndex = 0;
    CD_SYSTEM.queueWriteIndex = 0;

    CdFlush();
}

/**
 * decomp.me link: (100%) https://decomp.me/scratch/gsUc3
 */
s32 cdrom_enter_recovery_mode(void)
{
    s32 flags;
    s32 result;

    flags = CD_SYSTEM.statusFlags.word;
    result = 0;

    if (flags & 8)
    {
        return 1;
    }

    if (CD_SYSTEM.currentCommand == 0)
    {
        if ((CD_SYSTEM.initCommand == 0) && !(flags & 7) && (CD_SYSTEM.queueReadIndex == CD_SYSTEM.queueWriteIndex))
        {
            result = 1;
            CD_SYSTEM.statusFlags.word |= 8;
            CD_SYSTEM.initState = 0;
        }
    }

    return result;
}

void func_80014434(void)
{
    D_801ED801 = 1;
}

s32 cdrom_decompress_data(u8** srcStart, u8** dstStart, u8* srcEnd, u8* dstEnd)
{
    u8* srcPtr;
    u8* dstPtr;
    u32 iterations;
    u32 opcode;

    u8* tempPtr;
    u8 nextByte;

    u8 offsetLow;

    u8 param0;
    u8 param1;
    u8 param2;
    u8 param3;

    u32 something;
    u32 tempSum;

    s32 seed;

    srcPtr = *srcStart;
    dstPtr = *dstStart;

    while (srcPtr < srcEnd && dstPtr < dstEnd)
    {
        opcode = *srcPtr;

        switch (opcode)
        {
        case 0xF0:
            param1 = srcPtr[1];

            srcPtr += 2;
            iterations = (param1 & 0xf) + 3;
            param1 = param1 >> 4;

            do
            {
                *dstPtr++ = param1;
            } while (--iterations != 0);
            break;

        case 0xF1:
            param1 = srcPtr[2];
            nextByte = srcPtr[1];

            srcPtr += 3;
            iterations = nextByte + 4;

            do
            {
                *dstPtr++ = param1;
            } while (--iterations != 0);
            break;

        case 0xF2:
            param1 = srcPtr[2];
            nextByte = srcPtr[1];

            srcPtr += 3;
            iterations = nextByte + 2;
            param2 = param1 >> 4;
            param1 = param1 & 0xf;

            do
            {
                dstPtr[0] = param1;
                dstPtr[1] = param2;
                dstPtr += 2;
            } while (--iterations != 0);
            break;

        case 0xF3:
            param1 = srcPtr[2];
            param0 = srcPtr[3];
            nextByte = srcPtr[1];

            srcPtr += 4;
            iterations = nextByte + 2;

            do
            {
                dstPtr[0] = param1;
                dstPtr[1] = param0;
                dstPtr += 2;
            } while (--iterations != 0);
            break;

        case 0xF4:
            param1 = srcPtr[2];
            param0 = srcPtr[3];
            param3 = srcPtr[4];
            nextByte = srcPtr[1];

            srcPtr += 5;
            iterations = nextByte + 2;

            do
            {
                *dstPtr = param1;
                (&dstPtr[2])[-1] = param0;
                (&dstPtr[2])[0] = param3;
                dstPtr += 3;
            } while (--iterations != 0);

            break;

        case 0xF5:
            param1 = srcPtr[2];
            nextByte = srcPtr[1];

            srcPtr += 3;
            iterations = nextByte + 4;

            do
            {
                dstPtr[0] = param1;
                dstPtr[1] = *srcPtr++;
                dstPtr += 2;
            } while (--iterations != 0);

            break;

        case 0xF6:
            param1 = srcPtr[2];
            param0 = srcPtr[3];
            nextByte = srcPtr[1];

            srcPtr += 4;
            tempPtr = &dstPtr[2];
            iterations = nextByte + 3;

            do
            {
                *dstPtr = param1;
                tempPtr[-1] = param0;
                nextByte = *(u8*)srcPtr;
                srcPtr += 1;
                dstPtr += 3;
                tempPtr[0] = nextByte;
                tempPtr += 3;
            } while (--iterations != 0);

            break;

        case 0xF7:
            param1 = srcPtr[2];
            param0 = srcPtr[3];
            param3 = srcPtr[4];
            nextByte = srcPtr[1];

            srcPtr += 5;
            iterations = nextByte + 2;

            do
            {
                dstPtr[0] = param1;
                dstPtr[1] = param0;
                dstPtr[2] = param3;
                dstPtr[3] = *srcPtr++;
                dstPtr += 4;
            } while (--iterations != 0);

            break;

        case 0xF8:
            param1 = srcPtr[2];
            nextByte = srcPtr[1];

            srcPtr += 3;
            iterations = nextByte + 4;

            do
            {
                *dstPtr++ = param1;
                param1 += 1;
            } while (--iterations != 0);

            break;

        case 0xF9:
            param1 = srcPtr[2];
            nextByte = srcPtr[1];

            srcPtr += 3;
            iterations = nextByte + 4;

            do
            {
                *dstPtr++ = param1;
                param1 -= 1;
            } while (--iterations != 0);

            break;

        case 0xFA:
            param1 = srcPtr[2];
            param0 = srcPtr[3];
            nextByte = srcPtr[1];

            srcPtr += 4;
            iterations = nextByte + 5;

            do
            {
                *dstPtr++ = param1;
                param1 += param0;
            } while (--iterations != 0);

            break;

        case 0xFB:
            param2 = srcPtr[2];
            something = srcPtr[3];
            nextByte = srcPtr[1];
            param3 = srcPtr[4];

            iterations = nextByte + 3;
            seed = param3 << 24; // place param3 in the high byte for sign extension
            srcPtr += 5;

            do
            {
                // Write the two current bytes
                ((u8*)dstPtr)[0] = param2;
                ((u8*)dstPtr)[1] = something;
                dstPtr += 2;

                // Sign-extend param3 via arithmetic right shift
                tempSum = seed >> 24;

                // Form the 16-bit value (param0 << 8) | param2

                tempSum += (something << 8) | param2; // add to the sign-extended constant

                // Update for next iteration
                param2 = tempSum;           // low byte
                something = (tempSum >> 8); // high byte
            } while (--iterations != 0);
            break;

        case 0xFC:
            param1 = srcPtr[1];
            offsetLow = (opcode = srcPtr[2]);

            srcPtr += 3;
            iterations = (offsetLow >> 4) + 4;

            tempPtr = (u8*)((u32)param1 | (u32)((offsetLow & 0xF) << 8));
            tempPtr = (u8*)(dstPtr - (((u32)tempPtr) & 0xFFFF));

            do
            {
                *dstPtr++ = tempPtr++ [-1];
            } while (--iterations != 0);

            break;

        case 0xFD:
            param1 = srcPtr[1];
            param2 = param1;
            nextByte = srcPtr[2];

            srcPtr += 3;
            iterations = nextByte + 0x14;
            tempPtr = (u8*)(dstPtr - param2);

            do
            {
                *dstPtr++ = tempPtr++ [-1];
            } while (--iterations != 0);

            break;

        case 0xFE:
            param1 = srcPtr[1];

            srcPtr += 2;
            iterations = (param1 & 0xF) + 3;
            tempPtr = (u8*)(dstPtr - ((u32)(param1 & 0xF0) >> 1));

            do
            {
                offsetLow = (tempPtr++)[-8];
                *dstPtr++ = offsetLow;
            } while (--iterations != 0);

            break;

        case 0xFF:
            *srcStart = &srcPtr[1];
            *dstStart = dstPtr;
            return 0;

        default:
            srcPtr++;
            iterations = opcode + 1;

            do
            {
                *dstPtr++ = *srcPtr++;
            } while (--iterations != 0);

            break;
        }

        *srcStart = srcPtr;
    }

    *dstStart = dstPtr;
    return 1;
}

s32* cdrom_handle_stream_data(s32 arg0, u32 arg1)
{
    s32 remaining;
    s32 temp_t1;
    s32 temp_v0;
    s32 temp_v0_3;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a1;
    s32 var_a1_2;
    u8* result;
    u8* var_a2_2;
    u8* var_a2_3;
    u8* var_v1;
    u8* var_v1_2;
    u32 temp_a0_2;
    u32 temp_a2;
    u32 bytesBuffered;
    u32 bytesConsumed;
    u32 wrapOverflow;
    u32 temp_v0_2;
    u32 readPtr;
    u32 temp_v1_2;
    u32 temp_v1_3;
    u32 var_t0;

    var_t0 = arg1;

    if (arg1 >= 0x801U)
    {
        var_t0 = 0x800;
    }

    if (arg0 == 0)
    {
        CD_STREAM_STATE.dataReady = 1;
        CD_STREAM_STATE.writePtr = 0x801DC001U;
        CD_STREAM_STATE.readPtr = 0x801DC001U;
        CD_STREAM_STATE.bytesBuffered = (s32)(var_t0 - 1);
        CD_STREAM_STATE.wrapOverflow = 0;
        return (s32*)0x801DC000;
    }

    if (CD_STREAM_STATE.dataReady == 0)
    {

        remaining = CD_STREAM_STATE.bytesBuffered - CD_STREAM_STATE.bytesConsumed;
        bytesConsumed = CD_STREAM_STATE.bytesConsumed;
        wrapOverflow = CD_STREAM_STATE.wrapOverflow;
        temp_t1 = (4 - (remaining & 3)) & 3;

        if (wrapOverflow != 0)
        {
            readPtr = CD_STREAM_STATE.readPtr;
            temp_a2 = 0x801DC118 - remaining;
            CD_STREAM_STATE.writePtr = temp_a2;
            CD_STREAM_STATE.readPtr = temp_a2;

            var_a1 = remaining + 3;

            CD_STREAM_STATE.bytesBuffered = wrapOverflow + remaining + var_t0;
            var_a2_3 = (u8*)(temp_a2 - temp_t1);
            var_v1 = (u8*)((readPtr + bytesConsumed) - temp_t1);

            if (var_a1 < 0)
            {
                var_a1 = remaining + 6;
            }

            temp_v1_2 = (var_a1 >> 2) - 1;
            var_a0 = temp_v1_2;
            temp_v0 = -1;
            if (var_a0 != temp_v0)
            {
                do
                {
                    *(s32*)var_a2_3 = *(s32*)var_v1;
                    var_v1 += 4;
                    var_a0 -= 1;
                    var_a2_3 += 4;
                } while (var_a0 != temp_v0);
            }

            temp_v1_2 = CD_STREAM_STATE.wrapOverflow;
            CD_STREAM_STATE.wrapOverflow = 0U;
            var_a2_2 = var_a2_3 + temp_v1_2;
        }
        else
        {

            // does nothing
            if ((*((CdStreamState*)0x1F800000)).readPtr)
            {
            }

            var_a2_2 = (u8*)0x801DC000;
            var_a1_2 = remaining + 3;
            CD_STREAM_STATE.bytesBuffered = remaining + var_t0;
            temp_v0_2 = CD_STREAM_STATE.readPtr;
            temp_v1_3 = temp_t1 + 0x801DC000;
            CD_STREAM_STATE.writePtr = temp_v1_3;
            CD_STREAM_STATE.readPtr = temp_v1_3;
            var_v1_2 = (u8*)((temp_v0_2 + bytesConsumed) - temp_t1);
            if (var_a1_2 < 0)
            {
                var_a1_2 = remaining + 6;
            }
            var_a0_2 = (var_a1_2 >> 2) - 1;
            temp_v0_3 = -1;
            if (var_a0_2 != temp_v0_3)
            {
                do
                {
                    *(s32*)var_a2_2 = *(s32*)var_v1_2;
                    var_v1_2 += 4;
                    var_a0_2 -= 1;
                    var_a2_2 += 4;
                } while (var_a0_2 != temp_v0_3);
            }
        }
        CD_STREAM_STATE.dataReady = 1U;
        return (s32*)var_a2_2;
    }

    temp_v0_2 = CD_STREAM_STATE.readPtr;
    bytesBuffered = CD_STREAM_STATE.bytesBuffered;
    temp_a0_2 = CD_STREAM_STATE.wrapOverflow;
    result = (u8*)(temp_v0_2 + bytesBuffered);

    if ((temp_a0_2 != 0) || ((u32)(result + var_t0) > 0x801DE000U))
    {
        result = (u8*)(temp_a0_2 + 0x801DC118);

        if ((u32)CD_STREAM_STATE.writePtr >= (u32)(result + var_t0))
        {
            CD_STREAM_STATE.wrapOverflow = temp_a0_2 + var_t0;
        }
        else
        {
            CD_STREAM_STATE.reserved += 1;
            return NULL;
        }
    }
    else
    {
        CD_STREAM_STATE.bytesBuffered = bytesBuffered + var_t0;
    }

    if (arg1 == var_t0)
    {
        CD_STREAM_STATE.bufferWrapped = 1;
    }
    return (s32*)result;
}

void cdrom_decompress_buffer(u8* srcStart, u8* dstStart)
{
    srcStart++;
    while (cdrom_decompress_data(&srcStart, &dstStart, (u8*)-4U, (u8*)-4U) != 0);
}

void cdrom_clear_data_ready(s8* arg0)
{
    volatile s8* ref = arg0;
    *ref = 0;
}
