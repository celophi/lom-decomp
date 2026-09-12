#include "common.h"

extern u8 *func_800C1E40(s32 arg0);
extern u32 D_80051C50[];
extern s8 D_800F0C38[];
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern u8 g_menuLayoutBuffer[];

typedef struct { u8 pad[0x2B0C]; u8 unk2B0C; } NameView;
typedef struct { s32 a[27]; } LocalTableCopy;
typedef struct { u8 pad[4]; u8 value; } ResourceByte;

typedef struct { u8 pad[0x2B30]; unsigned int a0:4; unsigned int a1:4; unsigned int a2:4; unsigned int a3:4; unsigned int a4:4; unsigned int a5:4; unsigned int a6:4; unsigned int a7:4; unsigned int a8:4; unsigned int a9:4; unsigned int a10:4; unsigned int a11:4; unsigned int a12:4; unsigned int a13:4; unsigned int a14:4; unsigned int a15:4; } StatNibbles;

typedef struct { u8 pad[0x2B22]; u16 hp, stat0, stat1, stat2, stat3, stat4; } OutputStats;
typedef struct {u8 pad[0x2B48];u8 flags0,flags1,flags2,enabled;u32 zero;unsigned int low:4;unsigned int high:4;unsigned int rest:24;} GroupOutput;
typedef struct {u8 pad[0x2B38];u16 resistance;} ResistanceView;
typedef struct {u8 pad[0xCF4];unsigned int id:8;unsigned int type:2;unsigned int category:6;unsigned int rest:16;} ItemHeader;
#define U8(p,o)  (*(u8 *)((u8 *)(p) + (o)))
#define U16(p,o) (*(u16 *)((u8 *)(p) + (o)))
#define U32(p,o) (*(u32 *)((u8 *)(p) + (o)))

/**
 * @brief Copies derived group stats into an active record.
 * @note Initial nonmatching C; preserves packed stores and overlapping access widths.
 */
void func_800C3F18(s32 arg0, void *destination)
{
    u8 *arg1 = destination;
    s32 temp_a2;
    s32 temp_a2_2;
    s32 temp_v0_10;
    s32 temp_v0_11;
    s32 temp_v0_12;
    s32 temp_v0_13;
    s32 temp_v0_14;
    s32 temp_v0_15;
    s32 temp_v0_16;
    s32 temp_v0_17;
    s32 temp_v0_18;
    s32 temp_v0_19;
    s32 temp_v0_20;
    s32 temp_v0_2;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 var_a1;
    s32 var_a2;
    s32 var_a2_2;
    s32 var_a2_3;
    s32 var_a2_4;
    s32 var_a2_5;
    s32 var_a3_2;
    u16 temp_v0_3;
    u16 temp_v0_5;
    u16 temp_v0_6;
    u8 *temp_v1;
    u8 temp_v0;
    u8 temp_v0_7;
    u8 temp_v0_8;
    u8 temp_v0_9;
    u8 *temp_a0;
    u8 *temp_a0_2;
    u8 *temp_a1;
    u8 *temp_v0_4;
    u8 *var_a1_2;
    u8 *var_a3;
    u8 *var_v0;
    u8 *var_v1;
    u8 *var_v1_2;

    var_a2 = 0;
    U8(arg1, 0x50) = 1;
    U8(arg1, 0x90) = 1;
    U8(arg1, 0xD0) = 0;
    U8(arg1, 0x110) = 0;
    do
    {
        temp_v1 = arg1 + var_a2;
        temp_v0 = U8(g_menuLayoutBuffer, var_a2 + arg0 * 0x14C + 0x2B0C);
        var_a2 += 1;
        *temp_v1 = temp_v0;
    } while (var_a2 < 0x15);
    var_a2_2 = 1;
    U32(arg1, 0x18) = (s32) (((U32(arg1, 0x18) & ~0x7F) | 4) & ~0x80);
    var_v1 = arg1 + 1;
    U8(arg1, 0x19) = (s8) (U8(g_menuLayoutBuffer, arg0 * 0x14C + 0x2B50) & 0xF);
    do
    {
        U8(var_v1, 0x1A) = 0;
        var_a2_2 -= 1;
        var_v1 -= 1;
    } while (var_a2_2 >= 0);
    var_a2_3 = 3;
    var_v0 = arg1 + 3;
    do
    {
        U8(var_v0, 0x1C) = 0;
        var_a2_3 -= 1;
        var_v0 -= 1;
    } while (var_a2_3 >= 0);
    var_a2_4 = 0;
    U8(arg1, 0x20) = 0x63;
    var_a3 = arg1 + 0x90;
    U32(arg1, 0x20) = (s32) (0x63 & 0xFF);
    temp_v0_2 = arg0 * 0x14C;
    temp_a0 = temp_v0_2 + g_menuLayoutBuffer;
    var_a1 = temp_v0_2;
    U16(arg1, 0x24) = (u16) U16(temp_a0, 0x2B22);
    temp_v0_3 = U16(temp_a0, 0x2B24);
    var_v1_2 = arg1;
    U16(arg1, 0x26) = temp_v0_3;
    U16(arg1, 0x74) = temp_v0_3;
    do
    {
        temp_v0_4 = var_a1 + g_menuLayoutBuffer;
        var_a1 += 2;
        temp_v0_5 = U16(temp_v0_4, 0x2B26);
        var_a2_4 += 1;
        U16(var_v1_2, 0x28) = temp_v0_5;
        U16(var_a3, 0x24) = temp_v0_5;
        var_a3 += 2;
        var_v1_2 += 2;
    } while (var_a2_4 < 4);
    var_a2_5 = 0;
    var_a3_2 = arg0 * 0x14C;
    var_a1_2 = arg1;
    do
    {
        temp_a0_2 = var_a3_2 + g_menuLayoutBuffer;
        var_a3_2 += 2;
        var_a2_5 += 1;
        temp_v0_6 = (U16(var_a1_2, 0x30) & 0xFE00) | (U16(temp_a0_2, 0x2B38) & 0x1FF);
        U16(var_a1_2, 0x30) = temp_v0_6;
        U16(var_a1_2, 0x30) = (u16) ((temp_v0_6 & 0x1FF) | (U16(temp_a0_2, 0x2B38) & 0xFE00));
        var_a1_2 += 2;
    } while (var_a2_5 < 8);
    temp_a1 = (arg0 * 0x14C) + g_menuLayoutBuffer;
    temp_v0_7 = U8(temp_a1, 0x2B48);
    U8(arg1, 0x40) = temp_v0_7;
    U8(arg1, 0xBC) = temp_v0_7;
    temp_v0_8 = U8(temp_a1, 0x2B49);
    U8(arg1, 0x41) = temp_v0_8;
    U8(arg1, 0x7C) = temp_v0_8;
    temp_v0_9 = U8(temp_a1, 0x2B4A);
    U8(arg1, 0x42) = temp_v0_9;
    U8(arg1, 0xBD) = temp_v0_9;
    U8(arg1, 0x48) = 0;
    U8(arg1, 0x49) = 0;
    U8(arg1, 0x4A) = 0;
    U8(arg1, 0x4B) = 0;
    U8(arg1, 0x4C) = 0;
    U8(arg1, 0x4D) = 0;
    U8(arg1, 0x4E) = 0;
    U8(arg1, 0x4F) = 0;
    U8(arg1, 0x43) = (u8) U8(temp_a1, 0x2B4B);
    temp_v0_10 = (U32(arg1, 0x68) & ~0xF) | (U32(temp_a1, 0x2B30) & 0xF);
    U32(arg1, 0x68) = temp_v0_10;
    temp_v0_11 = (temp_v0_10 & ~0xF0) | (U8(temp_a1, 0x2B30) & 0xF0);
    U32(arg1, 0x68) = temp_v0_11;
    temp_v0_12 = (temp_v0_11 & ~0xF00) | (U32(temp_a1, 0x2B30) & 0xF00);
    U32(arg1, 0x68) = temp_v0_12;
    temp_v0_13 = (temp_v0_12 & 0xFFFF0FFF) | (U32(temp_a1, 0x2B30) & 0xF000);
    U32(arg1, 0x68) = temp_v0_13;
    temp_v0_14 = (temp_v0_13 & 0xFFF0FFFF) | ((U16(temp_a1, 0x2B32) & 0xF) << 0x10);
    U32(arg1, 0x68) = temp_v0_14;
    temp_v1_2 = (temp_v0_14 & 0xFF0FFFFF) | (U32(temp_a1, 0x2B30) & 0xF00000);
    U32(arg1, 0x68) = temp_v1_2;
    temp_v1_3 = (temp_v1_2 & 0xF0FFFFFF) | ((U8(temp_a1, 0x2B33) & 0xF) << 0x18);
    U32(arg1, 0x68) = temp_v1_3;
    U32(arg1, 0x68) = (s32) ((temp_v1_3 & 0x0FFFFFFF) | (((u32) U32(temp_a1, 0x2B30) >> 0x1C) << 0x1C));
    temp_v0_15 = (U32(arg1, 0xA8) & ~0xF) | (U32(temp_a1, 0x2B34) & 0xF);
    U32(arg1, 0xA8) = temp_v0_15;
    temp_v0_16 = (temp_v0_15 & ~0xF0) | (U8(temp_a1, 0x2B34) & 0xF0);
    U32(arg1, 0xA8) = temp_v0_16;
    temp_v0_17 = (temp_v0_16 & ~0xF00) | (U32(temp_a1, 0x2B34) & 0xF00);
    U32(arg1, 0xA8) = temp_v0_17;
    temp_v0_18 = (temp_v0_17 & 0xFFFF0FFF) | (U32(temp_a1, 0x2B34) & 0xF000);
    U32(arg1, 0xA8) = temp_v0_18;
    temp_v0_19 = (temp_v0_18 & 0xFFF0FFFF) | ((U16(temp_a1, 0x2B36) & 0xF) << 0x10);
    U32(arg1, 0xA8) = temp_v0_19;
    temp_a2 = (temp_v0_19 & 0xFF0FFFFF) | (U32(temp_a1, 0x2B34) & 0xF00000);
    U32(arg1, 0xA8) = temp_a2;
    temp_a2_2 = (temp_a2 & 0xF0FFFFFF) | ((U8(temp_a1, 0x2B37) & 0xF) << 0x18);
    U32(arg1, 0xA8) = temp_a2_2;
    U32(arg1, 0xA8) = (s32) ((temp_a2_2 & 0x0FFFFFFF) | (((u32) U32(temp_a1, 0x2B34) >> 0x1C) << 0x1C));
    temp_v0_20 = (U32(arg1, 0x174) & ~0xF) | (U8(temp_a1, 0x2B50) & 0xF);
    U32(arg1, 0x174) = temp_v0_20;
    U32(arg1, 0x174) = (s32) ((temp_v0_20 & ~0xF0) | (U8(temp_a1, 0x2B50) & 0xF0));
    U8(arg1, 0x175) = (u8) U8(temp_a1, 0x2B51);
    U8(arg1, 0x176) = (u8) U8(temp_a1, 0x2B52);
    U8(arg1, 0x177) = (u8) U8(temp_a1, 0x2B53);
    U32(arg1, 0x178) = (s32) U32(temp_a1, 0x2B54);
}

/**
 * @brief Recomputes a party member's derived stat block from its gosub result set.
 *
 * Decodes the digit-name glyph runs for the member's level, sums element/attribute
 * counts and resistances across the selected inventory records, clamps each derived
 * value into range, and writes the packed stat fields back into the member's layout
 * record at @c arg0 * 0x14C inside the global menu layout buffer.
 *
 * @param arg0 Party member / layout record index.
 * @note WIP m2c-derived match (87.02%); not yet byte-for-byte. Preserves original
 *       codegen forms; do not clean up.
 */
void func_800C4364(s32 arg0)
{
    s32 localTable[27];
    s32 accum[16];
    s32 count;
    s32 i;
    s32 digit;
    s32 resourceIndex; s32 highIndex; s32 nextDigit;
    s32 value;
    s32 recordOffset;
    s32 itemOffset;
    s32 type;
    u8 *p;
    u8 *base;
    u8 *outBase;
    u8 *tb0,*tb1,*tb2,*tb3,*tb4,*tb5;
    s32 *results;
    s32 *ap;

    *(LocalTableCopy *)localTable = *(LocalTableCopy *)D_80051C50;

    count = 0;
    base = g_menuLayoutBuffer;
    accum[15] = (s32)base[0x29D5];
    digit = accum[15] / 50 + 11;
    {s32 highIndex;accum[0] = ((ResourceByte *)(func_800C1E40(0x100) + (resourceIndex = digit * 2)))->value + (((ResourceByte *)(func_800C1E40(0x100) + ((highIndex = digit * 2 + 1))))->value << 8);}
    {s32 highIndex;accum[1] = ((ResourceByte *)(func_800C1E40(0x100) + (resourceIndex = (digit + 1) * 2)))->value + (((ResourceByte *)(func_800C1E40(0x100) + ((highIndex = (digit + 1) * 2 + 1))))->value << 8);}
    i = accum[0];
    if (i < accum[1]) {
        outBase = base;
        recordOffset = arg0 * 0x14C;
        do {
            if (count < 21) ((NameView *)((count + recordOffset) + (u32)outBase))->unk2B0C = ((ResourceByte *)(func_800C1E40(0x100) + i))->value;
            count++; i++;
        } while (i < accum[1]);
    }

    value = accum[15];
    if (value < 200) {
        accum[15] = value % 50 + 1;
        digit = accum[15] / 100;
        if (accum[15] >= 100) {
            {s32 highIndex;accum[0] = ((ResourceByte *)(func_800C1E40(0x100) + (resourceIndex = digit * 2)))->value + (((ResourceByte *)(func_800C1E40(0x100) + ((highIndex = digit * 2 + 1))))->value << 8);}
            {s32 highIndex;accum[1] = ((ResourceByte *)(func_800C1E40(0x100) + (resourceIndex = (digit + 1) * 2)))->value + (((ResourceByte *)(func_800C1E40(0x100) + ((highIndex = (digit + 1) * 2 + 1))))->value << 8);}
            i = accum[0];
            if (i < accum[1]) {
                outBase = g_menuLayoutBuffer; recordOffset = arg0 * 0x14C;
                do { if (count < 21) ((NameView *)((count + recordOffset) + (u32)outBase))->unk2B0C = ((ResourceByte *)(func_800C1E40(0x100) + i))->value; count++; i++; } while (i < accum[1]);
            }
        }
        digit = (accum[15] % 100) / 10;
        if (accum[15] >= 10) {
            {s32 highIndex;accum[0] = ((ResourceByte *)(func_800C1E40(0x100) + (resourceIndex = digit * 2)))->value + (((ResourceByte *)(func_800C1E40(0x100) + ((highIndex = digit * 2 + 1))))->value << 8);}
            {s32 highIndex;accum[1] = ((ResourceByte *)(func_800C1E40(0x100) + (resourceIndex = (digit + 1) * 2)))->value + (((ResourceByte *)(func_800C1E40(0x100) + ((highIndex = (digit + 1) * 2 + 1))))->value << 8);}
            i = accum[0];
            if (i < accum[1]) {
                outBase = g_menuLayoutBuffer; recordOffset = arg0 * 0x14C;
                do { if (count < 21) ((NameView *)((count + recordOffset) + (u32)outBase))->unk2B0C = ((ResourceByte *)(func_800C1E40(0x100) + i))->value; count++; i++; } while (i < accum[1]);
            }
        }
        digit = accum[15] % 10;
        {s32 highIndex;accum[0] = ((ResourceByte *)(func_800C1E40(0x100) + (resourceIndex = digit * 2)))->value + (((ResourceByte *)(func_800C1E40(0x100) + ((highIndex = digit * 2 + 1))))->value << 8);}
        {s32 highIndex;accum[1] = ((ResourceByte *)(func_800C1E40(0x100) + (resourceIndex = (digit + 1) * 2)))->value + (((ResourceByte *)(func_800C1E40(0x100) + ((highIndex = (digit + 1) * 2 + 1))))->value << 8);}
        i = accum[0];
        if (i < accum[1]) {
            outBase = g_menuLayoutBuffer; recordOffset = arg0 * 0x14C;
            do { if (count < 21) ((NameView *)((count + recordOffset) + (u32)outBase))->unk2B0C = ((ResourceByte *)(func_800C1E40(0x100) + i))->value; count++; i++; } while (i < accum[1]);
        }
        accum[0] = func_800C1E40(0x100)[0x18] + (func_800C1E40(0x100)[0x19] << 8);
        accum[1] = func_800C1E40(0x100)[0x1A] + (func_800C1E40(0x100)[0x1B] << 8);
        i = accum[0];
        if (i < accum[1]) {
            outBase = g_menuLayoutBuffer; recordOffset = arg0 * 0x14C;
            do { if (count < 21) ((NameView *)((count + recordOffset) + (u32)outBase))->unk2B0C = ((ResourceByte *)(func_800C1E40(0x100) + i))->value; count++; i++; } while (i < accum[1]);
        }
    }
    if (count < 21) ((NameView *)(g_menuLayoutBuffer+(count+arg0*0x14C)))->unk2B0C = 0;

    i = 0; accum[0] = 0;
    if (g_gosub_result_count > 0) {
        u8 *scanBase = g_menuLayoutBuffer;
        u8 *recBase = scanBase + 0xCE0;
        s32 resultCount = g_gosub_result_count; s32 *results;
        results = g_gosub_result_values;
        do {
            itemOffset = *results << 6;
            if (((U32(scanBase, itemOffset + 0xCF4) >> 8) & 3) == 0) accum[0] += U16(recBase, itemOffset + 0x24);
            i++; results++;
        } while (i < resultCount);
    }
    accum[0] = accum[0]<10 ? 10 : accum[0]>200 ? 200 : accum[0]; ((OutputStats *)(g_menuLayoutBuffer+arg0*0x14C))->stat0 = (u16)accum[0];

    i = 0; accum[0]=0; accum[1]=0; accum[2]=0; accum[3]=0;
    if (g_gosub_result_count > 0) {s32 selectedType=1;
        u8 *scanBase = g_menuLayoutBuffer;
        u8 *recBase = scanBase + 0xCE0;
        s32 resultCount = g_gosub_result_count; s32 *results;
        results = g_gosub_result_values;
        do {
            itemOffset = *results << 6;
            if (((U32(scanBase,itemOffset+0xCF4)>>8)&3)==selectedType) {
                accum[0]+=U16(recBase,itemOffset+0x24); accum[1]+=U16(recBase,itemOffset+0x26); accum[2]+=U16(recBase,itemOffset+0x28); accum[3]+=U16(recBase,itemOffset+0x2A);
            }
            i++; results++;
        } while (i < resultCount);
        i=0;
    }
    {u8 *statBase;i=0;statBase=g_menuLayoutBuffer;
    for(;i<4;i++) {s32 outputOffset;
        accum[i] = accum[i]<0 ? 0 : accum[i]>99 ? 99 : accum[i];
        ((OutputStats *)(statBase+(outputOffset=arg0*0x14C+i*2)))->stat1=(u16)accum[i];
    }}

    accum[0]=0; accum[1]=0; accum[2]=0; accum[3]=0; accum[4]=0; accum[5]=0; accum[6]=0; accum[7]=0;
    accum[8]=0; accum[9]=0; accum[10]=0; accum[11]=0; accum[12]=0; accum[13]=0; accum[14]=0; accum[15]=0;
    i=0;
    if (g_gosub_result_count>i) {
        u8 *itemBase=g_menuLayoutBuffer; s32 resultCount=g_gosub_result_count; s32 *results=g_gosub_result_values;
        do {
            u8 *item = (u8 *)((*results << 6) + (u32)itemBase);
            type=(U32(item,0xCF4)>>8)&3;
            if(type==0){
                accum[0]+=U32(item,0xCF8)&0xF;
                accum[1]+=U8(item,0xCF8)>>4;
                accum[2]+=(U32(item,0xCF8)>>8)&0xF;
                accum[3]+=(U32(item,0xCF8)>>12)&0xF;
                accum[4]+=U16(item,0xCFA)&0xF;
                accum[5]+=(U32(item,0xCF8)>>20)&0xF;
                accum[6]+=U8(item,0xCFB)&0xF;
                accum[7]+=U32(item,0xCF8)>>28;
            } else if(type==1){
                accum[8]+=U32(item,0xCF8)&0xF;
                accum[9]+=U8(item,0xCF8)>>4;
                accum[10]+=(U32(item,0xCF8)>>8)&0xF;
                accum[11]+=(U32(item,0xCF8)>>12)&0xF;
                accum[12]+=U16(item,0xCFA)&0xF;
                accum[13]+=(U32(item,0xCF8)>>20)&0xF;
                accum[14]+=U8(item,0xCFB)&0xF;
                accum[15]+=U32(item,0xCF8)>>28;
            }
            i++;results++;
        }while(i<resultCount);
    }
    for (i=0;i<16;i++) {
        if (g_menuLayoutBuffer[0x29D5] >= 200) accum[i] += 2;
        accum[i] = accum[i]<0 ? 0 : accum[i]>9 ? 9 : accum[i];
    }

    
    ((StatNibbles *)(g_menuLayoutBuffer+arg0*0x14C))->a0 = accum[0];
    ((StatNibbles *)(g_menuLayoutBuffer+arg0*0x14C))->a1 = accum[1];
    ((StatNibbles *)(g_menuLayoutBuffer+arg0*0x14C))->a2 = accum[2];
    ((StatNibbles *)(g_menuLayoutBuffer+arg0*0x14C))->a3 = accum[3];
    ((StatNibbles *)(g_menuLayoutBuffer+arg0*0x14C))->a4 = accum[4];
    ((StatNibbles *)(g_menuLayoutBuffer+arg0*0x14C))->a5 = accum[5];
    ((StatNibbles *)(g_menuLayoutBuffer+arg0*0x14C))->a6 = accum[6];
    ((StatNibbles *)(g_menuLayoutBuffer+arg0*0x14C))->a7 = accum[7];
    ((StatNibbles *)(g_menuLayoutBuffer+arg0*0x14C))->a8 = accum[8];
    ((StatNibbles *)(g_menuLayoutBuffer+arg0*0x14C))->a9 = accum[9];
    ((StatNibbles *)(g_menuLayoutBuffer+arg0*0x14C))->a10 = accum[10];
    ((StatNibbles *)(g_menuLayoutBuffer+arg0*0x14C))->a11 = accum[11];
    ((StatNibbles *)(g_menuLayoutBuffer+arg0*0x14C))->a12 = accum[12];
    ((StatNibbles *)(g_menuLayoutBuffer+arg0*0x14C))->a13 = accum[13];
    ((StatNibbles *)(g_menuLayoutBuffer+arg0*0x14C))->a14 = accum[14];
    ((StatNibbles *)(g_menuLayoutBuffer+arg0*0x14C))->a15 = accum[15];


    accum[0]=0;accum[1]=0;accum[2]=0;accum[3]=0;accum[4]=0;accum[5]=0;accum[6]=0;accum[7]=0;i=0;
    for(i=0;i<g_gosub_result_count;i++){s8 *resistanceTable=D_800F0C38;u8 *item=g_menuLayoutBuffer+(g_gosub_result_values[i]<<6);accum[0]+=resistanceTable[U32(item,0xCFC)&0xF];accum[1]+=resistanceTable[U8(item,0xCFC)>>4];accum[2]+=resistanceTable[(U32(item,0xCFC)>>8)&0xF];accum[3]+=resistanceTable[(U32(item,0xCFC)>>12)&0xF];accum[4]+=resistanceTable[U16(item,0xCFE)&0xF];accum[5]+=resistanceTable[(U32(item,0xCFC)>>20)&0xF];accum[6]+=resistanceTable[U8(item,0xCFF)&0xF];accum[7]+=resistanceTable[U32(item,0xCFC)>>28];}
    for(i=0;i<8;i++){accum[i]=(accum[i]*5)+20;accum[i] = accum[i]<20 ? 20 : accum[i]>99 ? 99 : accum[i];((ResistanceView *)(g_menuLayoutBuffer+arg0*0x14C+i*2))->resistance&=0xFE00;((ResistanceView *)(g_menuLayoutBuffer+arg0*0x14C+i*2))->resistance=(u16)accum[i]<<9;}

    accum[0]=0;i=0;if(g_gosub_result_count>i){s32 selectedType=1;u8 *scanBase=g_menuLayoutBuffer;u8 *recBase=scanBase+0xCE0;s32 resultCount=g_gosub_result_count;s32 *results;results=g_gosub_result_values;do{itemOffset=*results<<6;if(((U32(scanBase,itemOffset+0xCF4)>>8)&3)==selectedType)accum[0]|=U8(recBase,itemOffset+0x2c);i++;results++;}while(i<resultCount);}g_menuLayoutBuffer[arg0*0x14C+0x2B48]=(u8)accum[0];
    accum[0]=0;i=0;if(g_gosub_result_count>0){u8 *scanBase=g_menuLayoutBuffer;u8 *recBase=scanBase+0xCE0;s32 resultCount=g_gosub_result_count;s32 *results;results=g_gosub_result_values;do{itemOffset=*results<<6;if(((U32(scanBase,itemOffset+0xCF4)>>8)&3)==0)accum[0]|=U8(recBase,itemOffset+0x2c);i++;results++;}while(i<resultCount);}g_menuLayoutBuffer[arg0*0x14C+0x2B49]=(u8)accum[0];
    accum[0]=0;i=0;if(g_gosub_result_count>0){s32 selectedType=1;u8 *scanBase=g_menuLayoutBuffer;u8 *recBase=scanBase+0xCE0;s32 resultCount=g_gosub_result_count;s32 *results;results=g_gosub_result_values;do{itemOffset=*results<<6;if(((U32(scanBase,itemOffset+0xCF4)>>8)&3)==selectedType)accum[0]|=U8(recBase,itemOffset+0x2d);i++;results++;}while(i<resultCount);i=0;}

    tb0=g_menuLayoutBuffer;(tb0+arg0*0x14C)[0x2B4A]=(u8)accum[0];(tb0+arg0*0x14C)[0x2B4B]=1;U32((tb0+arg0*0x14C),0x2B4C)=0;((GroupOutput *)(tb0+arg0*0x14C))->low=0;
    for(i=0;i<g_gosub_result_count;i++){if(((ItemHeader *)(g_menuLayoutBuffer+(g_gosub_result_values[i]<<6)))->type==0)((GroupOutput *)(tb0+arg0*0x14C))->low=(u8)localTable[((ItemHeader *)(g_menuLayoutBuffer+(g_gosub_result_values[i]<<6)))->category];}

    i=0; accum[0]=0;
    tb1=g_menuLayoutBuffer;
    ((GroupOutput *)(tb1+arg0*0x14C))->high=4;
    { s32 resultCount = g_gosub_result_count; s32 *results; if(resultCount>0){u8 *scanBase=g_menuLayoutBuffer;s32 selectedType=1;results=g_gosub_result_values;do{if(((U32(scanBase,(*results<<6)+0xCF4)>>8)&3)==selectedType)accum[0]++;i++;results++;}while(i<resultCount);} }
    if(accum[0]==2){ tb2=g_menuLayoutBuffer; ((GroupOutput *)(tb2+arg0*0x14C))->high=5; }
    if(accum[0]==3){ tb3=g_menuLayoutBuffer; ((GroupOutput *)(tb3+arg0*0x14C))->high=6; }
    tb4=g_menuLayoutBuffer;
    (tb4+arg0*0x14C)[0x2B51]=0;
    accum[0]=75-(((tb4+arg0*0x14C)[0x2B50]>>4)*10);
    accum[0] = accum[0]<0 ? 0 : accum[0]>50 ? 50 : accum[0];
    tb5=g_menuLayoutBuffer;
    (tb5+arg0*0x14C)[0x2B52]=(u8)accum[0];(tb5+arg0*0x14C)[0x2B53]=0; U32((tb5+arg0*0x14C),0x2B54)=0; 
    accum[0]=U16((tb5+arg0*0x14C),0x2B24);accum[1]=U16((tb5+arg0*0x14C),0x2B26);accum[2]=U16((tb5+arg0*0x14C),0x2B28);accum[3]=U16((tb5+arg0*0x14C),0x2B2A);accum[4]=U16((tb5+arg0*0x14C),0x2B2C);
    count=accum[0]+accum[1]+accum[2]+accum[3]+accum[4];count=count*5>>1;
    if(count>=50){itemOffset=999;if(count<1000)itemOffset=count;}else itemOffset=50;
    {u8 *hpBase;
    hpBase=g_menuLayoutBuffer;
    U16((hpBase+arg0*0x14C),0x2B22)=(s16)itemOffset;
}
}
