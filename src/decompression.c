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
 * @see decomp.me: (99.83%) https://decomp.me/scratch/MlH6P
 */
s32 CD_DecompressData(u8** srcStart, u8** dstStart, u8* srcEnd, u8* dstEnd) 
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
                }
                while (--iterations != 0);
                break;
    
            case 0xF1:
                param1 = srcPtr[2];
                nextByte = srcPtr[1];
                
                srcPtr += 3;
                iterations = nextByte + 4;
                
                do 
                {
                    *dstPtr++ = param1;
                } 
                while (--iterations != 0);
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
                } 
                while (--iterations != 0);
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
                } 
                while (--iterations != 0); 
                break;
                
            case 0xF4:
                param1 = srcPtr[2];
                param0 = srcPtr[3];
                param3 = srcPtr[4];
                nextByte = srcPtr[1];
                
                srcPtr += 5;
                iterations = nextByte + 2;
                
                do {
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
                
                do {
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
                
                do {
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
                
                do {
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
                
                do {
                    *dstPtr++ = param1;
                    param1 += 1;
                } while (--iterations != 0);
                
                break;
                
            case 0xF9:
                param1 = srcPtr[2];
                nextByte = srcPtr[1];
                
                srcPtr += 3;
                iterations = nextByte + 4;
                
                do {
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
                
                do {
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
                seed = param3 << 24;   // place param3 in the high byte for sign extension
                srcPtr += 5;
            
                do {
                    // Write the two current bytes
                    ((u8*)dstPtr)[0] = param2;
                    ((u8*)dstPtr)[1] = something;
                    dstPtr += 2;
            
                    // Sign-extend param3 via arithmetic right shift
                     tempSum = seed >> 24;
            
                    // Form the 16-bit value (param0 << 8) | param2
                    
                    
                    tempSum += (something << 8) | param2;    // add to the sign-extended constant
            
                    // Update for next iteration
                    param2 = tempSum ;         // low byte
                    something = (tempSum >> 8);  // high byte
                } while (--iterations != 0);
                break;
                
            case 0xFC:
                param1 = srcPtr[1];
                offsetLow = (opcode = srcPtr[2]);
                
                srcPtr += 3;
                iterations = (offsetLow >> 4) + 4;
              
                tempPtr = (u8*)((u32)param1 | (u32)((offsetLow & 0xF) << 8));
                tempPtr = (u8 *) (dstPtr - (((u32) tempPtr) & 0xFFFF));
                
                do {
                    *dstPtr++ = tempPtr++[-1];
                } while (--iterations != 0);
                
                break;
                
            case 0xFD:
                param1 = srcPtr[1];
                param2 = param1;
                nextByte = srcPtr[2];
    
                srcPtr += 3;
                iterations = nextByte + 0x14;
                tempPtr = (u8*)(dstPtr - param2);
                
                do {
                    *dstPtr++ = tempPtr++[-1];
                } while (--iterations != 0);
                
                break;
                
            case 0xFE:
                param1 = srcPtr[1];
                
                srcPtr += 2;
                iterations = (param1 & 0xF) + 3;
                tempPtr = (u8*)(dstPtr - ((u32)(param1 & 0xF0) >> 1));
                
                do {
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
                
                do {
                    *dstPtr++ = *srcPtr++;
                } while (--iterations != 0);
                
                break;
        }

        *srcStart = srcPtr;
    }
    
    
    *dstStart = dstPtr;
    return 1;
}