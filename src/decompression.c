#include "common.h"

/**
 * @brief Decompresses a custom bytecode-encoded data stream
 *
 * Processes a sequence of opcodes from a source buffer and emits uncompressed
 * bytes into a destination buffer. Both the source and destination pointers
 * are updated in-place on every iteration so the caller can resume across
 * multiple calls.
 *
 * @details
 * Each iteration reads a single opcode byte from `*srcStart` and dispatches
 * on its value. Opcodes 0xF0–0xFF are control opcodes; any other value is a
 * raw-copy opcode. The encoding is summarised below:
 *
 *   Opcode  Encoding (bytes after opcode)  Operation
 *   ------  -----------------------------  ---------
 *   0xF0    [packed]                       Repeat upper nibble (count = lower nibble + 3)
 *   0xF1    [count] [value]                Repeat value (count + 4) times
 *   0xF2    [count] [packed]               Alternate lo/hi nibbles as 2-byte pairs (count + 2)
 *   0xF3    [count] [b0] [b1]              Repeat 2-byte pattern {b0, b1} (count + 2) times
 *   0xF4    [count] [b0] [b1] [b2]         Repeat 3-byte pattern {b0, b1, b2} (count + 2) times
 *   0xF5    [count] [fixed] + stream        Write {fixed, next_src_byte} pairs (count + 4) times
 *   0xF6    [count] [b0] [b1] + stream      Write {b0, b1, next_src_byte} triplets (count + 3) times
 *   0xF7    [count] [b0] [b1] [b2] + stream Write {b0, b1, b2, next_src_byte} quads (count + 2) times
 *   0xF8    [count] [start]                Ascending arithmetic run from start (count + 4 bytes)
 *   0xF9    [count] [start]                Descending arithmetic run from start (count + 4 bytes)
 *   0xFA    [count] [start] [step]         Arithmetic run: start, start+step, ... (count + 5 bytes)
 *   0xFB    [count] [b0] [b1] [delta]      Arithmetic 16-bit pair run; b0/b1 form a 16-bit
 *                                          accumulator incremented by signed delta each step
 *   0xFC    [offLo] [offHi_cnt]            Back-reference: 12-bit offset, count = upper nibble + 4
 *   0xFD    [offset] [count]               Back-reference: 8-bit offset, count + 0x14 bytes
 *   0xFE    [packed]                       Back-reference: offset = (upper nibble << 3) + 8,
 *                                          count = lower nibble + 3
 *   0xFF    (none)                         End-of-stream; updates pointers and returns 0
 *   default (opcode value)                 Raw copy: opcode + 1 bytes follow in the stream
 *
 * The loop terminates early (returning 1) if the source pointer reaches
 * `srcEnd` or the destination pointer reaches `dstEnd` before a 0xFF is seen.
 *
 * @param srcStart  Pointer to the current source read position; updated on return
 * @param dstStart  Pointer to the current destination write position; updated on return
 * @param srcEnd    Exclusive upper bound of the source buffer (loop guard)
 * @param dstEnd    Exclusive upper bound of the destination buffer (loop guard)
 *
 * @return 0 if a 0xFF end-of-stream opcode was encountered,
 *         1 if the source or destination buffer was exhausted first
 *
 * @note
 * - `valueHigh` is initialised from `srcEnd` and persists across loop iterations;
 *   it forms the high byte of the 16-bit running accumulator used by opcode 0xFB
 * - `*srcStart` is written inside the loop (not just on exit) to keep the
 *   caller's pointer current even if the outer while-condition terminates early
 *
 * @warning
 * - No bounds checking is performed on back-reference offsets (0xFC–0xFE);
 *   a malformed stream can read before the start of the destination buffer
 * - The destination buffer must be large enough to hold the fully decompressed
 *   output; no overflow check is performed beyond the `dstEnd` guard
 *
 * @see decomp.me: (99.70%) https://decomp.me/scratch/RH1ll
 */
s32 CD_DecompressData(u32* srcStart, u32* dstStart, u32 srcEnd, u32 dstEnd) {

    u32 iterations;
    u32 opcode;

    u8* tempPtr;
    u8 nextByte;
    
    u8 offsetLow;

    u8 param0;
    u8 param1;
    u8 param2;
    u8 param3;
    
    u32 tempSum;
    u32 srcPtr;
    u32 dstPtr;
    u32 valueHigh = srcEnd;  /* Upper bound for src; doubles as high byte of the 0xFB accumulator */
    
    srcPtr = *srcStart;
    dstPtr = *dstStart;

    /* Only enter the decompression loop if there is input to process */
    if (srcPtr < valueHigh) {
        
        while (srcPtr < valueHigh && dstPtr < dstEnd) {
            opcode = *(u8*)srcPtr;
            
            switch (opcode) {
            case 0xF0:
                /* 1 packed byte: upper nibble = value, lower nibble = count-3 */
                param1 = (((u8*)(srcPtr + 1))[0]);
                srcPtr += 2;
                iterations = (param1 & 0xf) + 3;
                param1 = param1 >> 4;
                
                do {
                    *(u8*)dstPtr++ = param1;
                } while (--iterations != 0);
                
                break;
    
            case 0xF1:
                /* [count] [value]: repeat value (count + 4) times */
                param1 = ((u8*)(srcPtr + 1))[1];
                nextByte = ((u8*)(srcPtr + 1))[0];
                
                srcPtr += 3;
                iterations = nextByte + 4;
                
                do {
                    *(u8*)dstPtr++ = param1;
                } while (--iterations != 0);
                
                break;
                
            case 0xF2:
                /* [count] [packed]: packed byte holds two nibbles; write {lo, hi} pairs
                 * (count + 2) times */
                param1 = ((u8*)(srcPtr + 1))[1];
                nextByte = ((u8*)(srcPtr + 1))[0];
                
                srcPtr += 3;
                iterations = nextByte + 2;

                param0 = param1 >> 4;   /* high nibble -> second byte of each pair */
                param1 = param1 & 0xf;  /* low nibble  -> first byte of each pair */
                do {
                    ((u8*)dstPtr)[0] = param1;
                    ((u8*)dstPtr)[1] = param0;
                    dstPtr += 2;
                } while (--iterations != 0);
                
                break;
                
            case 0xF3:
                /* [count] [b0] [b1]: repeat 2-byte pattern {b0, b1} (count + 2) times */
                param1 = ((u8*)(srcPtr + 1))[1];
                param0 = ((u8*)(srcPtr + 1))[2];
                nextByte = ((u8*)(srcPtr + 1))[0];
                
                srcPtr += 4;
                iterations = nextByte + 2;
                
                do {
                    ((u8*)dstPtr)[0] = param1;
                    ((u8*)dstPtr)[1] = param0;
                    dstPtr += 2;
                } while (--iterations != 0); 
    
                break;
                
            case 0xF4:
                /* [count] [b0] [b1] [b2]: repeat 3-byte pattern {b0, b1, b2} (count + 2) times */
                param1 = ((u8*)(srcPtr + 1))[1];
                param0 = ((u8*)(srcPtr + 1))[2];
                param3 = ((u8*)(srcPtr + 1))[3];
                nextByte = ((u8*)(srcPtr + 1))[0];
                
                srcPtr += 5;
                tempPtr = (u8*)(dstPtr + 2);  /* points two bytes ahead to allow [-1] / [0] writes */
                iterations = nextByte + 2;
                
                do {
                    *(u8*)dstPtr = param1;
                    tempPtr[-1] = param0;
                    tempPtr[0] = param3;
                    tempPtr += 3;
                    dstPtr += 3;
                } while (--iterations != 0);
                
                break;
                
            case 0xF5:
                /* [count] [fixed] + stream: write {fixed, next_src_byte} pairs
                 * (count + 4) times, consuming one byte from the stream per pair */
                param1 = ((u8*)(srcPtr + 1))[1];
                nextByte = ((u8*)(srcPtr + 1))[0];
               
                srcPtr += 3;
                iterations = nextByte + 4;
                
                do {
                    ((u8*)dstPtr)[0] = param1;
                    ((u8*)dstPtr)[1] = *(u8*)srcPtr++;
                    dstPtr += 2;
                } while (--iterations != 0);
                
                break;
                
            case 0xF6:
                /* [count] [b0] [b1] + stream: write {b0, b1, next_src_byte} triplets
                 * (count + 3) times */
                param1 = ((u8*)(srcPtr + 1))[1];
                param0 = ((u8*)(srcPtr + 1))[2];
                nextByte = ((u8*)(srcPtr + 1))[0];
               
                srcPtr += 4;
                tempPtr = (u8*)(dstPtr + 2);  /* points two bytes ahead for [-1] / [0] writes */
                iterations = nextByte + 3;
                
                do {
                    *(u8*)dstPtr = param1;
                    tempPtr[-1] = param0;
                    nextByte = *(u8*)srcPtr;
                    srcPtr += 1;
                    dstPtr += 3;
                    tempPtr[0] = nextByte;  /* stream byte fills the third slot */
                    tempPtr += 3;
                } while (--iterations != 0);
                
                break;
                
            case 0xF7:
                /* [count] [b0] [b1] [b2] + stream: write {b0, b1, b2, next_src_byte}
                 * 4-byte groups (count + 2) times */
                param1 = ((u8*)(srcPtr + 1))[1];
                param0 = ((u8*)(srcPtr + 1))[2];
                param3 = ((u8*)(srcPtr + 1))[3];
                nextByte = ((u8*)(srcPtr + 1))[0];
    
                srcPtr += 5;
                iterations = nextByte + 2;
                
                do {
                    *(u8*)dstPtr = param1;
                    ((u8*)(dstPtr + 3))[-2] = param0;       /* dstPtr[1] */
                    ((u8*)(dstPtr + 3))[-1] = param3;       /* dstPtr[2] */
                    ((u8*)(dstPtr + 3))[0] = *(u8*)srcPtr++;/* dstPtr[3] from stream */
                    dstPtr += 4;
                } while (--iterations != 0);
    
                break;
                
            case 0xF8:
                /* [count] [start]: ascending run — writes start, start+1, start+2 ...
                 * for (count + 4) bytes */
                param1 = ((u8*)(srcPtr + 1))[1];
                nextByte = ((u8*)(srcPtr + 1))[0];
                
                srcPtr += 3;
                iterations = nextByte + 4;
                
                do {
                    *(u8*)dstPtr++ = param1;
                    param1 += 1;
                } while (--iterations != 0);
                
                break;
                
            case 0xF9:
                /* [count] [start]: descending run — writes start, start-1, start-2 ...
                 * for (count + 4) bytes */
                param1 = ((u8*)(srcPtr + 1))[1];
                nextByte = ((u8*)(srcPtr + 1))[0];
                
                srcPtr += 3;
                iterations = nextByte + 4;
                
                do {
                    *(u8*)dstPtr++ = param1;
                    param1 -= 1;
                } while (--iterations != 0);
                
                break;
                
            case 0xFA:
                /* [count] [start] [step]: arithmetic run — writes start, start+step,
                 * start+2*step ... for (count + 5) bytes */
                param1 = ((u8*)(srcPtr + 1))[1];
                param0 = ((u8*)(srcPtr + 1))[2];
                nextByte = ((u8*)(srcPtr + 1))[0];
                
                srcPtr += 4;
                iterations = nextByte + 5;
                
                do {
                    *(u8*)dstPtr++ = param1;
                    param1 += param0;
                } while (--iterations != 0);
                
                break;
                
            case 0xFB:
                /* [count] [lo] [hi] [delta]: arithmetic 16-bit pair run.
                 * param2/param0 form the initial 16-bit value (lo byte first).
                 * param3 is a signed 8-bit step added to the 16-bit accumulator
                 * (valueHigh:param0) each iteration. Writes (count + 3) pairs. */
                param2 = ((u8*)(srcPtr + 1))[1];
                param0 = ((u8*)(srcPtr + 1))[2];
                nextByte = ((u8*)(srcPtr + 1))[0];
                param3 = ((u8*)(srcPtr + 1))[3];
                
                srcPtr += 5;
                iterations = nextByte + 3;
                
                do {
                    /* Write the current 16-bit value as two bytes */
                    ((u8*)dstPtr)[0] = param2;
                    ((u8*)dstPtr)[1] = param0;
                    dstPtr += 2;
    
                    /* Sign-extend param3 from 8 bits to 32 bits */
                    tempSum = ((param3 << 0x18) >> 0x18);
    
                    /* Add signed delta to the 16-bit accumulator (valueHigh:param0) */
                    tempSum = tempSum + ((valueHigh << 8) | (param0 & 0xFF));
    
                    /* Split result back into low byte and carry */
                    param0 = tempSum;          /* low byte of next value */
                    valueHigh = tempSum >> 8;  /* carry into high byte */
                    
                } while (--iterations != 0);
                
                break;
                
            case 0xFC:
                /* [offLo] [offHi_cnt]: 12-bit back-reference.
                 * Upper nibble of second byte encodes (count - 4).
                 * Lower nibble of second byte + first byte form a 12-bit
                 * back-reference offset into the already-written output. */
                param1 = ((u8*)(srcPtr + 1))[0];
                offsetLow = ((u8*)(srcPtr + 1))[1];
                
                srcPtr += 3;
                iterations = (offsetLow >> 4) + 4;
                
                tempPtr = (u8*)((u32)param1 | (u32)((offsetLow & 0xF) << 8)); /* merge high 4 bits -> 12-bit offset */
                tempPtr = (u8*)(dstPtr - (u32)tempPtr);                        /* back-reference start = dst - offset */
                
                do {
                    *(u8*)dstPtr++ = tempPtr++[-1];  /* copy from offset-1 to handle overlap */
                } while (--iterations != 0);
                
                break;
                
            case 0xFD:
                /* [offset] [count]: 8-bit back-reference.
                 * Copies (count + 0x14) bytes from (dstPtr - offset - 1). */
                param2 = ((u8*)(srcPtr + 1))[0];
                nextByte = ((u8*)(srcPtr + 1))[1];

                srcPtr += 3;
                iterations = nextByte + 0x14;
                tempPtr = (u8*)(dstPtr - param2);  /* back-reference start */
                
                do {
                    *(u8*)dstPtr++ = tempPtr++[-1];  /* copy from offset-1 to handle overlap */
                } while (--iterations != 0);
                
                break;
                
            case 0xFE:
                /* [packed]: compact back-reference.
                 * Upper nibble encodes offset: offset = (upper_nibble << 3) + 8.
                 * Lower nibble encodes (count - 3). Copies count bytes from
                 * (dstPtr - offset - 8). */
                param1 = ((u8*)(srcPtr + 1))[0];
                
                srcPtr += 2;
                iterations = (param1 & 0xF) + 3;
                tempPtr = (u8*)(dstPtr - ((u32)(param1 & 0xF0) >> 1));  /* offset = upper_nibble << 3 */
                
                do {
                    *(u8*)dstPtr++ = tempPtr++[-8];  /* extra -8 bias baked into the addressing */
                } while (--iterations != 0);
                
                break;
                
            case 0xFF:
                /* End-of-stream marker: advance src past this byte and save both
                 * pointers so the caller knows where decompression stopped */
                *srcStart = (u32)((u8*)(srcPtr + 1));
                *dstStart = dstPtr;
                return 0;
                
            default:
                /* Raw copy: opcode value encodes (count - 1); the following
                 * (opcode + 1) bytes are copied verbatim to the output */
                srcPtr += 1;
                iterations = opcode + 1;
                
                do {
                    *(u8*)dstPtr++ = *(u8*)srcPtr++;
                } while (--iterations != 0);
                
                break;
            }

            /* Update caller's src pointer after every opcode so it stays current
             * if the outer while-condition terminates the loop early */
            *srcStart = srcPtr;
        }
    }
    
    /* Source or destination buffer exhausted before end-of-stream */
    *dstStart = dstPtr;
    return 1;
}