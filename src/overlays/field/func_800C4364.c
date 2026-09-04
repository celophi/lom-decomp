#include "common.h"

extern u8 *func_800C1E40(s32 arg0);
extern u32 D_80051C50[];
extern s8 D_800F0C38[];
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern u8 g_menuLayoutBuffer[];

typedef struct { u8 pad[0x2B0C]; u8 unk2B0C; } NameView;
typedef struct { s32 a[27]; } LocalTableCopy;

#define U8(p,o)  (*(u8 *)((u8 *)(p) + (o)))
#define U16(p,o) (*(u16 *)((u8 *)(p) + (o)))
#define U32(p,o) (*(u32 *)((u8 *)(p) + (o)))

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
    accum[0] = func_800C1E40(0x100)[digit * 2 + 4] + (func_800C1E40(0x100)[digit * 2 + 5] << 8);
    accum[1] = func_800C1E40(0x100)[(digit + 1) * 2 + 4] + (func_800C1E40(0x100)[(digit + 1) * 2 + 5] << 8);
    i = accum[0];
    if (i < accum[1]) {
        outBase = base;
        recordOffset = arg0 * 0x14C;
        do {
            if (count < 21) ((NameView *)(outBase + count + recordOffset))->unk2B0C = func_800C1E40(0x100)[i + 4];
            i++; count++;
        } while (i < accum[1]);
    }

    value = accum[15];
    if (value < 200) {
        accum[15] = value % 50 + 1;
        digit = accum[15] / 100;
        if (accum[15] >= 100) {
            accum[0] = func_800C1E40(0x100)[digit * 2 + 4] + (func_800C1E40(0x100)[digit * 2 + 5] << 8);
            accum[1] = func_800C1E40(0x100)[(digit + 1) * 2 + 4] + (func_800C1E40(0x100)[(digit + 1) * 2 + 5] << 8);
            i = accum[0];
            if (i < accum[1]) {
                outBase = g_menuLayoutBuffer; recordOffset = arg0 * 0x14C;
                do { if (count < 21) ((NameView *)(outBase + count + recordOffset))->unk2B0C = func_800C1E40(0x100)[i + 4]; i++; count++; } while (i < accum[1]);
            }
        }
        digit = (accum[15] % 100) / 10;
        if (accum[15] >= 10) {
            accum[0] = func_800C1E40(0x100)[digit * 2 + 4] + (func_800C1E40(0x100)[digit * 2 + 5] << 8);
            accum[1] = func_800C1E40(0x100)[(digit + 1) * 2 + 4] + (func_800C1E40(0x100)[(digit + 1) * 2 + 5] << 8);
            i = accum[0];
            if (i < accum[1]) {
                outBase = g_menuLayoutBuffer; recordOffset = arg0 * 0x14C;
                do { if (count < 21) ((NameView *)(outBase + count + recordOffset))->unk2B0C = func_800C1E40(0x100)[i + 4]; i++; count++; } while (i < accum[1]);
            }
        }
        digit = accum[15] % 10;
        accum[0] = func_800C1E40(0x100)[digit * 2 + 4] + (func_800C1E40(0x100)[digit * 2 + 5] << 8);
        accum[1] = func_800C1E40(0x100)[(digit + 1) * 2 + 4] + (func_800C1E40(0x100)[(digit + 1) * 2 + 5] << 8);
        i = accum[0];
        if (i < accum[1]) {
            outBase = g_menuLayoutBuffer; recordOffset = arg0 * 0x14C;
            do { if (count < 21) ((NameView *)(outBase + count + recordOffset))->unk2B0C = func_800C1E40(0x100)[i + 4]; i++; count++; } while (i < accum[1]);
        }
        accum[0] = func_800C1E40(0x100)[0x18] + (func_800C1E40(0x100)[0x19] << 8);
        accum[1] = func_800C1E40(0x100)[0x1A] + (func_800C1E40(0x100)[0x1B] << 8);
        i = accum[0];
        if (i < accum[1]) {
            outBase = g_menuLayoutBuffer; recordOffset = arg0 * 0x14C;
            do { if (count < 21) ((NameView *)(outBase + count + recordOffset))->unk2B0C = func_800C1E40(0x100)[i + 4]; i++; count++; } while (i < accum[1]);
        }
    }
    if (count < 21) { outBase = g_menuLayoutBuffer; recordOffset = arg0 * 0x14C; ((NameView *)(outBase + count + recordOffset))->unk2B0C = 0; }

    i = 0; accum[0] = 0;
    if (g_gosub_result_count > 0) {
        u8 *scanBase = g_menuLayoutBuffer;
        u8 *recBase = scanBase + 0xCE0;
        s32 resultCount = g_gosub_result_count;
        results = g_gosub_result_values;
        do {
            itemOffset = *results << 6;
            if (((U32(scanBase, itemOffset + 0xCF4) >> 8) & 3) == 0) accum[0] += U16(recBase, itemOffset + 0x24);
            i++; results++;
        } while (i < resultCount);
    }
    value = 10; if (accum[0] >= 10) { value = 200; if (accum[0] < 201) value = accum[0]; }
    accum[0] = value; U16(g_menuLayoutBuffer, arg0 * 0x14C + 0x2B24) = (u16)accum[0];

    i = 0; accum[0]=0; accum[1]=0; accum[2]=0; accum[3]=0;
    if (g_gosub_result_count > 0) {
        u8 *scanBase = g_menuLayoutBuffer;
        u8 *recBase = scanBase + 0xCE0;
        s32 resultCount = g_gosub_result_count;
        results = g_gosub_result_values;
        do {
            itemOffset = *results << 6;
            if (((U32(scanBase,itemOffset+0xCF4)>>8)&3)==1) {
                accum[0]+=U16(recBase,itemOffset+0x24); accum[1]+=U16(recBase,itemOffset+0x26); accum[2]+=U16(recBase,itemOffset+0x28); accum[3]+=U16(recBase,itemOffset+0x2A);
            }
            i++; results++;
        } while (i < resultCount);
        i=0;
    }
    ap=accum; recordOffset=arg0*0x14C;
    do { if (*ap>=0) { value=99; if (*ap<100) value=*ap; } else value=0; *ap=value; U16(g_menuLayoutBuffer,recordOffset+0x2B26)=(u16)*ap; ap++; i++; recordOffset+=2; } while(i<4);

    accum[0]=0; accum[1]=0; accum[2]=0; accum[3]=0; accum[4]=0; accum[5]=0; accum[6]=0; accum[7]=0;
    accum[8]=0; accum[9]=0; accum[10]=0; accum[11]=0; accum[12]=0; accum[13]=0; accum[14]=0; accum[15]=0;
    i=0;
    if (g_gosub_result_count>0) {
        results=g_gosub_result_values;
        do {
            u8 *item = g_menuLayoutBuffer + (*results << 6);
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
        }while(i<g_gosub_result_count);
    }
    i = 0;
    do {
        if (g_menuLayoutBuffer[0x29D5] >= 200) accum[i] += 2;
        if (accum[i] >= 0) {
            value = 9;
            if (accum[i] < 10) value = accum[i];
        } else {
            value = 0;
        }
        accum[i] = value;
        i++;
    } while (i < 16);

    p=g_menuLayoutBuffer+arg0*0x14C;
    value=U32(p,0x2B30); value=(value&~0xF)|(accum[0]&0xF);U32(p,0x2B30)=value; value=(value&~0xF0)|((accum[1]&0xF)<<4);U32(p,0x2B30)=value; value=(value&~0xF00)|((accum[2]&0xF)<<8);U32(p,0x2B30)=value; value=(value&0xFFFF0FFF)|((accum[3]&0xF)<<12);U32(p,0x2B30)=value; value=(value&0xFFF0FFFF)|((accum[4]&0xF)<<16);U32(p,0x2B30)=value; value=(value&0xFF0FFFFF)|((accum[5]&0xF)<<20);U32(p,0x2B30)=value; value=(value&0xF0FFFFFF)|((accum[6]&0xF)<<24);U32(p,0x2B30)=value; U32(p,0x2B30)=(value&0x0FFFFFFF)|(accum[7]<<28);
    value=U32(p,0x2B34); value=(value&~0xF)|(accum[8]&0xF);U32(p,0x2B34)=value; value=(value&~0xF0)|((accum[9]&0xF)<<4);U32(p,0x2B34)=value; value=(value&~0xF00)|((accum[10]&0xF)<<8);U32(p,0x2B34)=value; value=(value&0xFFFF0FFF)|((accum[11]&0xF)<<12);U32(p,0x2B34)=value; value=(value&0xFFF0FFFF)|((accum[12]&0xF)<<16);U32(p,0x2B34)=value; value=(value&0xFF0FFFFF)|((accum[13]&0xF)<<20);U32(p,0x2B34)=value; value=(value&0xF0FFFFFF)|((accum[14]&0xF)<<24);U32(p,0x2B34)=value; U32(p,0x2B34)=(value&0x0FFFFFFF)|(accum[15]<<28);

    accum[0]=0;accum[1]=0;accum[2]=0;accum[3]=0;accum[4]=0;accum[5]=0;accum[6]=0;accum[7]=0;i=0;
    if(g_gosub_result_count>0){results=g_gosub_result_values;do{u8 *item=g_menuLayoutBuffer+(*results<<6);accum[0]+=D_800F0C38[U32(item,0xCFC)&0xF];accum[1]+=D_800F0C38[U8(item,0xCFC)>>4];accum[2]+=D_800F0C38[(U32(item,0xCFC)>>8)&0xF];accum[3]+=D_800F0C38[(U32(item,0xCFC)>>12)&0xF];accum[4]+=D_800F0C38[U16(item,0xCFE)&0xF];accum[5]+=D_800F0C38[(U32(item,0xCFC)>>20)&0xF];accum[6]+=D_800F0C38[U8(item,0xCFF)&0xF];accum[7]+=D_800F0C38[U32(item,0xCFC)>>28];results++;i++;}while(i<g_gosub_result_count);i=0;}
    ap=accum;recordOffset=arg0*0x14C;do{value=(*ap*5)+20;*ap=value;if(value>=20){if(value<100)value=value;else value=99;}else value=20;*ap=value;U16(g_menuLayoutBuffer,recordOffset+0x2B38+i*2)&=0xFE00;U16(g_menuLayoutBuffer,recordOffset+0x2B38+i*2)=(u16)*ap<<9;ap++;i++;}while(i<8);

    accum[0]=0;i=0;if(g_gosub_result_count>0){results=g_gosub_result_values;do{itemOffset=*results<<6;if(((U32(g_menuLayoutBuffer,itemOffset+0xCF4)>>8)&3)==1)accum[0]|=U8(g_menuLayoutBuffer,itemOffset+0xD0C);i++;results++;}while(i<g_gosub_result_count);}g_menuLayoutBuffer[arg0*0x14C+0x2B48]=(u8)accum[0];
    accum[0]=0;i=0;if(g_gosub_result_count>0){results=g_gosub_result_values;do{itemOffset=*results<<6;if(((U32(g_menuLayoutBuffer,itemOffset+0xCF4)>>8)&3)==0)accum[0]|=U8(g_menuLayoutBuffer,itemOffset+0xD0C);i++;results++;}while(i<g_gosub_result_count);}g_menuLayoutBuffer[arg0*0x14C+0x2B49]=(u8)accum[0];
    accum[0]=0;i=0;if(g_gosub_result_count>0){results=g_gosub_result_values;do{itemOffset=*results<<6;if(((U32(g_menuLayoutBuffer,itemOffset+0xCF4)>>8)&3)==1)accum[0]|=U8(g_menuLayoutBuffer,itemOffset+0xD0D);i++;results++;}while(i<g_gosub_result_count);i=0;}

    tb0=g_menuLayoutBuffer;p=tb0+arg0*0x14C;p[0x2B4B]=1;p[0x2B4C]=0;p[0x2B4A]=(u8)accum[0];U32(p,0x2B50)&=~0xF;
    { s32 resultCount = g_gosub_result_count; if(resultCount>0){u8 *scanBase=g_menuLayoutBuffer;results=g_gosub_result_values;do{value=U32(scanBase,(*results<<6)+0xCF4);if(((value>>8)&3)==0)U32(p,0x2B50)=(U32(p,0x2B50)&~0xF)|(localTable[((value>>8)&0xFC)>>2]&0xF);i++;results++;}while(i<resultCount);} }
    i=0; accum[0]=0;
    tb1=g_menuLayoutBuffer;p=tb1+arg0*0x14C;
    U32(p,0x2B50)=(U32(p,0x2B50)&~0xF0)|0x40;
    { s32 resultCount = g_gosub_result_count; if(resultCount>0){u8 *scanBase=g_menuLayoutBuffer;results=g_gosub_result_values;do{if(((U32(scanBase,(*results<<6)+0xCF4)>>8)&3)==1)accum[0]++;i++;results++;}while(i<resultCount);} }
    if(accum[0]==2){ tb2=g_menuLayoutBuffer;p=tb2+arg0*0x14C; U32(p,0x2B50)=(U32(p,0x2B50)&~0xF0)|0x50; }
    if(accum[0]==3){ tb3=g_menuLayoutBuffer;p=tb3+arg0*0x14C; U32(p,0x2B50)=(U32(p,0x2B50)&~0xF0)|0x60; }
    tb4=g_menuLayoutBuffer;p=tb4+arg0*0x14C;
    p[0x2B51]=0;
    accum[0]=75-((p[0x2B50]>>4)*10);
    if(accum[0]>=0){value=50;if(accum[0]<51)value=accum[0];}else value=0;
    accum[0]=value;
    tb5=g_menuLayoutBuffer;p=tb5+arg0*0x14C;
    p[0x2B53]=0; U32(p,0x2B54)=0; p[0x2B52]=(u8)accum[0];
    accum[0]=U16(p,0x2B24);accum[1]=U16(p,0x2B26);accum[2]=U16(p,0x2B28);accum[3]=U16(p,0x2B2A);accum[4]=U16(p,0x2B2C);
    count=(accum[0]+accum[1]+accum[2]+accum[3]+accum[4])*5>>1;
    if(count>=50){value=999;if(count<1000)value=count;}else value=50;
    tb5=g_menuLayoutBuffer;p=tb5+arg0*0x14C;
    U16(p,0x2B22)=(s16)value;
}
