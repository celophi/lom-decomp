#include "common.h"

/**
 * Description: Decompresses some data
 * 
 * Params:
 *   None
 * 
 * Returns: void
 * 
 * decomp.me link: https://decomp.me/scratch/ecuJv
 * decomp.me (%): 96.42% (register usage)
 */
s32 CD_DecompressData(u32* srcStart, u32* dstStart, u32 srcEnd, u32 dstEnd) 
{

    u32 iterations;
    u32 opcode;

    u8* tempPtr;
    u8 nextByte;
    
    u8 countAndOffsetHigh;
    u8 offsetLow;

    u8 param0;
    u8 param1;
    u8 param2;
    u8 param3;
    
    u32 valueHigh;
    u32 tempSum;
    
    u32 srcPtr;
    u32 dstPtr;
    
    srcPtr = *srcStart;
    dstPtr = *dstStart;

    if (srcPtr < srcEnd) {
        
        while (srcPtr < srcEnd && dstPtr < dstEnd) {
            opcode = *(u8*)srcPtr;  
            
            switch (opcode) {
            case 0xF0:
                param0 = ((u8*)(srcPtr + 1))[0]; 
                param0 = (param0 & 0xF);
                iterations = param0 + 3;
                srcPtr += 2;
                
                do {
                    *(u8*)dstPtr++ = (param0 >> 4);
                } while (--iterations != 0);
                
                break;
    
            case 0xF1:
                param1 = ((u8*)(srcPtr + 1))[1];
                param0 = ((u8*)(srcPtr + 1))[0];
                
                srcPtr += 3;
                iterations = param0 + 4;
                
                do {
                    *(u8*)dstPtr++ = param1;
                } while (--iterations != 0);
                
                break;
                
            case 0xF2:
                param1 = ((u8*)(srcPtr + 1))[1];
                param0 = ((u8*)(srcPtr + 1))[0];
                
                srcPtr += 3;
                iterations = param0 + 2;
                
                do {
                    ((u8*)dstPtr)[1] = (param1 >> 4);
                    ((u8*)dstPtr)[0] = (param1 & 0x0F);
                    dstPtr += 2;
                } while (--iterations != 0);
                
                break;
                
            case 0xF3:
                param1 = ((u8*)(srcPtr + 1))[1];
                param2 = ((u8*)(srcPtr + 1))[2];
                param0 = ((u8*)(srcPtr + 1))[0];
                
                srcPtr += 4;
                iterations = param0 + 2;
                
                do {
                    ((u8*)dstPtr)[0] = param1;
                    ((u8*)dstPtr)[1] = param2;
                    dstPtr += 2;
                } while (--iterations != 0); 
    
                break;
                
            case 0xF4:
                param1 = ((u8*)(srcPtr + 1))[1];
                param2 = ((u8*)(srcPtr + 1))[2];
                param3 = ((u8*)(srcPtr + 1))[3];
                param0 = ((u8*)(srcPtr + 1))[0];
                
                srcPtr += 5;
                tempPtr = (u8*)(dstPtr + 2);
                iterations = param0 + 2;
                
                do {
                    *(u8*)dstPtr = param1;
                    tempPtr[-1] = param2;
                    tempPtr[0] = param3;
                    tempPtr += 3;
                    dstPtr += 3;
                } while (--iterations != 0);
                
                break;
                
            case 0xF5:
                param1 = ((u8*)(srcPtr + 1))[1];
                param0 = ((u8*)(srcPtr + 1))[0];
               
                srcPtr += 3;
                iterations = param0 + 4;
                
                do {
                    ((u8*)dstPtr)[0] = param1;
                    ((u8*)dstPtr)[1] = *(u8*)srcPtr++;
                    dstPtr += 2;
                } while (--iterations != 0);
                
                break;
                
            case 0xF6:
                param1 = ((u8*)(srcPtr + 1))[1];
                param2 = ((u8*)(srcPtr + 1))[2];
                param0 = ((u8*)(srcPtr + 1))[0];
               
                srcPtr += 4;
                tempPtr = (u8*)(dstPtr + 2);
                iterations = param0 + 3;
                
                do {
                    *(u8*)dstPtr = param1;
                    tempPtr[-1] = param2;
                    nextByte = *(u8*)srcPtr;
                    srcPtr += 1;
                    dstPtr += 3;
                    tempPtr[0] = nextByte;
                    tempPtr += 3;
                } while (--iterations != 0);
                
                break;
                
            case 0xF7:
                param1 = ((u8*)(srcPtr + 1))[1];
                param2 = ((u8*)(srcPtr + 1))[2];
                param3 = ((u8*)(srcPtr + 1))[3];
                param0 = ((u8*)(srcPtr + 1))[0];
    
                srcPtr += 5;
                iterations = param0 + 2;
                
                do {
                    *(u8*)dstPtr = param1;
                    ((u8*)(dstPtr + 3))[-2] = param2;
                    ((u8*)(dstPtr + 3))[-1] = param3;
                    ((u8*)(dstPtr + 3))[0] = *(u8*)srcPtr++;
                    dstPtr += 4;
                } while (--iterations != 0);
    
                break;
                
            case 0xF8:
                param1 = ((u8*)(srcPtr + 1))[1];
                param0 = ((u8*)(srcPtr + 1))[0];
                
                srcPtr += 3;
                iterations = param0 + 4;
                
                do {
                    *(u8*)dstPtr++ = param1;
                    param1 += 1;
                } while (--iterations != 0);
                
                break;
                
            case 0xF9:
                param1 = ((u8*)(srcPtr + 1))[1];
                param0 = ((u8*)(srcPtr + 1))[0];
                
                srcPtr += 3;
                iterations = param0 + 4;
                
                do {
                    *(u8*)dstPtr++ = param1;
                    param1 -= 1;
                } while (--iterations != 0);
                
                break;
                
            case 0xFA:
                param1 = ((u8*)(srcPtr + 1))[1];
                param2 = ((u8*)(srcPtr + 1))[2];
                param0 = ((u8*)(srcPtr + 1))[0];
                
                srcPtr += 4;
                iterations = param0 + 5;
                
                do {
                    *(u8*)dstPtr++ = param1;
                    param1 += param2;
                } while (--iterations != 0);
                
                break;
                
            case 0xFB:
             
                param1 = ((u8*)(srcPtr + 1))[1];
                valueHigh = ((u8*)(srcPtr + 1))[2];
                param0 = ((u8*)(srcPtr + 1))[0];
                param3 = ((u8*)(srcPtr + 1))[3];
                
                srcPtr += 5;
                iterations = param0 + 3; // count + the following 3 bytes
                
                do {
                    
                    ((u8*)dstPtr)[0] = param1;
                    ((u8*)dstPtr)[1] = valueHigh;
                    
                    // 2fc
                    dstPtr += 2;
    
                    // 300
                    tempSum = ((param3 << 0x18) >> 0x18);

                
                    // 310
                    tempSum = tempSum + ((valueHigh << 8) | (param1 & 0xFF) );
    
                    // 314 
                    param1 = tempSum;
    
                    // 320
                    valueHigh = tempSum >> 8;
                    
                } while (--iterations != 0);
    
                break;
                
            case 0xFC:
                offsetLow = ((u8*)(srcPtr + 1))[0];
                countAndOffsetHigh = ((u8*)(srcPtr + 1))[1];
                
                srcPtr += 3;
                iterations = (countAndOffsetHigh >> 4) + 4;
                tempPtr = (u8*)(dstPtr - (offsetLow | ((countAndOffsetHigh & 0xF) << 8)));
                
                do {
                    *(u8*)dstPtr++ = tempPtr++[-1];
                } while (--iterations != 0);
                
                break;
                
            case 0xFD:
                param0 = ((u8*)(srcPtr + 1))[0];
                param1 = ((u8*)(srcPtr + 1))[1];

                srcPtr += 3;
                iterations = param1 + 0x14;
                tempPtr = (u8*)(dstPtr - param0);
                
                do {
                    *(u8*)dstPtr++ = tempPtr++[-1];
                } while (--iterations != 0);
                
                break;
                
            case 0xFE:
                param0 = ((u8*)(srcPtr + 1))[0];
                
                srcPtr += 2;
                iterations = (param0 & 0xF) + 3;
                tempPtr = (u8*)(dstPtr - ((u32)(param0 & 0xF0) >> 1));
                
                do {
                    *(u8*)dstPtr++ = tempPtr++[-8];
                } while (--iterations != 0);
                
                break;
                
            case 0xFF:
                *srcStart = (u32)((u8*)(srcPtr + 1));
                *dstStart = dstPtr;
                return 0;
                
            default:
                srcPtr += 1;
                iterations = opcode + 1;
                
                do {
                    *(u8*)dstPtr++ = *(u8*)srcPtr++;
                } while (--iterations != 0);
                
                break;
            }

            *srcStart = srcPtr;
        }
    }
    
    *dstStart = dstPtr;
    return 1;
}
