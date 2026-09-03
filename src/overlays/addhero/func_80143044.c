#include "common.h"

typedef struct { s16 x; s16 y; s16 w; s16 h; } RECT;

typedef struct AddheroElement {
    union {
        u32 word;
        struct { u32 state:3; u32 unk0_3:4; u32 x:9; u32 unk0_16:8; } f;
    } attr;
    u32 unk4_0:1;
    u32 y:8;
    u32 unk4_9:23;
    void *draw_handler;
    s32 unkC;
} AddheroElement;

typedef struct { s32 attr; s32 flags; s32 draw; } AddheroPacket;

typedef struct {
    s32 unk0; s32 unk4; s16 unk8; s16 unkA; s32 unkC; s16 unk10; s16 unk12;
    s32 unk14; s16 unk18; s16 unk1A; s32 unk1C; s16 unk20; s16 unk22;
} AddheroPolyG4Words;

typedef struct { u8 data[0x28]; } AddheroEntry28;

extern s32 D_801609A4, D_801609A8, D_801609AC, D_801609B0, D_801609B4, D_801609B8, D_801609BC;
extern s32 D_80160928, D_8016092C, D_80160934, D_80160938, D_801609E8;
extern s32 D_80164A40, D_80164A4C, D_80164A54, D_80164A60, D_80165200, D_8016545C;
extern s32 D_80122988, D_8012298C, D_80122718;
extern AddheroElement D_80160940;
extern u8 D_801609F0[];
extern u8 *D_80165488;
extern u8 *D_8012271C;
extern char D_800ECF7C[];
extern AddheroEntry28 D_80164B60[][20];
extern u8 D_80160574, D_80160598, D_801605A1;
extern u16 D_80146FA4, D_80146FA8, D_80146FB4, D_80146FB6, D_80146FC0, D_80146FD6, D_80146FD8;
extern u16 D_8014700C, D_8014700E, D_80147012;

s32 func_80142CE8(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_80144018(s32 prim, s32 *ot, s32 x, s32 y);
void func_80144008(void);
s32 func_80144140(u8 *base);
s32 func_80144194(u8 *base);
void func_801449F0(void);
void func_80145E14(void);

#define GLYPH_SYM(sym, off) ((void *)(((u8 *)&(sym) - (off)) + (sym)))
#define GLYPH_OFF(base, off) ((void *)((base) + *(u16 *)((base) + (off))))
#define SET_ELEM_CODE(e,c) ((e)->attr.word = ((e)->attr.word & 0x00FFFFFF) | ((u32)(c) << 24))

s32 func_80143044(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    switch (D_801609A4)
    {
    case 0xF8:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FD8, 0x34), 4, -arg2 + 0x90, -arg3, 2);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FD8, 0x34), 4, -arg2 + 0x90, -arg3, 2);
        break;
    case 0xFF:
        {
            s32 x; u8 *base;
            x = -arg2 + 0x90;
            base = (u8 *)&D_80146FA4;
            prim = func_800A88A0(prim, ot, base + D_80146FA4, 4, x, -arg3, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - arg3, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - arg3, 2);
        }
        break;
    case 0xFA:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FD8, 0x34), 4, -arg2 + 0x90, -arg3, 2);
        break;
    case 0xFD:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FA8, 4), 4, -arg2 + 0x90, -arg3, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB4, 0x10), 4, -arg2 + 0x90, -arg3, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80146FB6, 0x12), 4, -arg2 + 0x90, -arg3, 2);
        break;
    case 0xF7:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014700C, 0x68), 4, -arg2 + 0x90, -arg3, 2);
        break;
    case 0xF6:
        {
            s32 x; u8 *base; AddheroPolyG4Words *g; s32 next, elapsed, extent, color, finalmode;
            x = -arg2 + 0x90;
            prim = func_800A88A0(prim, ot, (void *)((s32)&D_80146FD6 - 0x32 + D_80146FD6), 4, x, -arg3, 2);
            base = (u8 *)&D_80146FD6 - 0x32;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - arg3, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - arg3, 2);
            next = prim; g = (AddheroPolyG4Words *)prim;
            if (D_80164A4C != 0)
            {
                elapsed = func_8002054C(-1) - D_80164A54;
                if (elapsed >= 0x101) elapsed = 0x100;
                color = 0xFFFF00; extent = elapsed * 0x120;
                g->unk4=0xFF; g->unkC=0xFFFF; g->unk1C=0xFF0000; ((u8 *)g)[3]=8;
                g->unk14=color; ((u8 *)g)[7]=0x38; g->unk18=0; g->unk8=0;
                if (extent < 0) extent += 0xFF;
                g->unk20=extent>>8; g->unk10=extent>>8; g->unk12=0; g->unkA=0; g->unk22=0x2C; g->unk1A=0x2C;
                g->unk0=(g->unk0 & 0xFF000000)|(*ot & 0xFFFFFF);
                *ot=(*ot & 0xFF000000)|(prim & 0xFFFFFF); next=prim+0x24;
            }
            prim = next;
            if (D_80160934 == 0)
            {
                if (func_80144140(D_801609F0) == 0)
                {
                    func_800A3938(0x78, 0x80);
                    D_80160940.draw_handler=(void *)func_80142CE8;
                    D_80160940.attr.f.unk0_3=1; D_80160940.attr.f.state=1; D_80160940.attr.f.x=0x20; D_80160940.attr.f.unk0_16=0x70;
                    D_80160940.unk4_0=1; D_80160940.y=0x14; SET_ELEM_CODE(&D_80160940,0);
                    func_800AA02C();
                    D_80165200=0; D_801609B8=0; D_80164A40=0; D_80160934=0; D_801609A4=0xFF;
                    func_801449F0(); finalmode=4; D_80165488=0; D_801609E8=finalmode; return prim;
                }
                func_800A3938(0x7B,0x80); D_801609A4=0xF4; func_80144008(); func_800AA02C();
            }
        }
        break;
    case 0xF3:
        {
            s32 x; AddheroPacket *packet; s32 i;
            x = -arg2 + 0x90;
            prim = func_800A88A0(prim, ot, GLYPH_SYM(D_80147012,0x6E), 4, x, -arg3, 2);
            prim = func_80144018(prim, ot, x, 0xE -arg3);
            if (D_80122988 & 0x40)
            {
                func_800A3938(0x78, 0x80);
                func_80144008();
                D_801609A4 = 0xF4;
                func_800AA02C();
            }
            else if (D_80122988 & 0x220)
            {
                if (D_801609B0 != 0)
                {
                    func_800A3938(0x78, 0x80);
                    func_80144008();
                    D_801609A4 = 0xF4;
                    func_800AA02C();
                }
                else
                {
                    func_800A3938(0x7D, 0x80);
                    D_8016092C = 3;
                    D_8012298C = 0x20;
                    packet = (AddheroPacket *)&D_80160940;
                    for (i = 0; i < 8; i++, packet++)
                    {
                        packet->flags &= ~0x200;
                        packet->attr &= ~7;
                    }
                    func_80067F5C(8);
                    func_800AA02C();
                }
            }
        }
        break;
    case 0xF4:
        {
            s32 x; u8 *base; s32 temp;
            x=-arg2+0x90;
            prim=func_800A88A0(prim,ot,(void *)((s32)&D_8014700E-0x6A+D_8014700E),4,x,-arg3,2);
            base=(u8 *)&D_8014700E-0x6A;
            prim=func_800A88A0(prim,ot,GLYPH_OFF(base,0x6C),4,x,0xE -arg3,2);
            prim=func_80144018(prim,ot,x,0x1C-arg3);
            if (D_80122988 & 0x40)
            {
                func_80144008();
                D_801609A4 = 0xF3;
                func_800A3938(0x78, 0x80);
                func_800AA02C();
            }
            else if (D_80122988 & 0x220)
            {
                if (D_801609B0 != 0)
                {
                    func_80144008();
                    D_801609A4 = 0xF3;
                    func_800A3938(0x78, 0x80);
                    func_800AA02C();
                }
                else
                {
                    func_800A3938(0x7E,0x80);
                    base = D_801609F0;
                    func_80016E7C(D_8012271C + 0x840, base + 0x770, 0x250);
                    *(s32 *)(base + 0x788) |= 0x80;
                    temp = func_80144194(base);
                    *(s32 *)(base + 0x33E4) = 0x414E41;
                    *(s32 *)(base + 0x33E0) = temp;
                    D_80165200 = 1;
                    D_80165488 = &D_801605A1;
                    D_801609A4 = 0xF5;
                }
            }
        }
        break;
    case 0xF5:
        {
            s32 x; u8 *base; AddheroPolyG4Words *g; s32 next,elapsed,extent,color; AddheroPacket *packet; s32 i;
            x=-arg2+0x90;
            prim=func_800A88A0(prim,ot,(void *)((s32)&D_80146FC0-0x1C+D_80146FC0),4,x,-arg3,2);
            base=(u8 *)&D_80146FC0-0x1C;
            prim=func_800A88A0(prim,ot,GLYPH_OFF(base,0x1E),4,x,0xE -arg3,2);
            prim=func_800A88A0(prim,ot,GLYPH_OFF(base,0xB2),4,x,0x1C-arg3,2);
            next=prim; g=(AddheroPolyG4Words *)prim;
            if(D_80164A4C!=0){
                elapsed=func_8002054C(-1)-D_80164A54; if(elapsed>=0x101)elapsed=0x100; color=0xFFFF00; extent=elapsed*0x120;
                g->unk4=0xFF;g->unkC=0xFFFF;g->unk1C=0xFF0000;((u8*)g)[3]=8;g->unk14=color;((u8*)g)[7]=0x38;g->unk18=0;g->unk8=0;
                if(extent<0)extent+=0xFF;g->unk20=extent>>8;g->unk10=extent>>8;g->unk12=0;g->unkA=0;g->unk22=0x2C;g->unk1A=0x2C;
                g->unk0=(g->unk0&0xFF000000)|(*ot&0xFFFFFF);*ot=(*ot&0xFF000000)|(prim&0xFFFFFF);next=prim+0x24;
            }
            prim=next;
            if(D_80165200==0){
                D_8012271C[0x840]=0; func_800A3938(0x7A,0x80); D_8012298C=0x20;
                packet=(AddheroPacket *)&D_80160940;
                for(i=0;i<8;i++,packet++){ packet->flags &= ~0x200; packet->attr &= ~7; }
                func_80067F5C(8); D_8016092C=2;
            }
        }
        break;
    default:
        {
            s32 x,posv,diff; u8 *base;
            x=-arg2+0x90; base=(u8 *)&D_80146FA4;
            prim=func_800A88A0(prim,ot,base+D_80146FA4,4,x,-arg3,2);
            prim=func_800A88A0(prim,ot,GLYPH_OFF(base,0x1E),4,x,0xE -arg3,2);
            prim=func_800A88A0(prim,ot,GLYPH_OFF(base,0xB2),4,x,0x1C-arg3,2);
            if(D_80164A60==0){
                if(D_80164A40!=0)return prim;
                if((u32)(*D_80165488-6)<2U)return prim;
                if((func_8001714C(D_800ECF7C,&D_80164B60[D_801609A8][D_801609AC],0xC)!=0) ||
                   (D_8016545C != *(s32 *)(D_8012271C+0xD8))) {
                    D_801609AC++;
                    if(D_801609AC>=D_801609A4){ if(D_801609A4!=0)D_801609A4=0xF7; else D_801609A4=0xF8; }
                    else { func_80145E14(); posv=D_801609AC*0xE; diff=posv-D_80160928;
                        if(diff>=0x4B){D_80160938=posv-0x46;D_801609BC=4;} if(diff<0){D_80160938=posv;D_801609BC=4;}
                    }
                } else { D_80164A54=func_8002054C(-1);D_80160934=1;D_80165488=&D_80160598;D_801609A4=0xF6; }
            }
        }
        break;
    case 0xFE:
        break;
    }
    if(D_80164A40!=0)return prim;
    if(D_801609A4==0xF6)return prim; if(D_801609A4==0xF5)return prim; if(D_801609A4==0xF4)return prim; if(D_801609A4==0xF3)return prim;
    if(D_80122988&0x40){
        s32 *p; s32 i,word; D_80122718=3;func_800A3938(0x78,0x80);func_80067F28();p=(s32 *)&D_80160940;i=0;
        do{word=*p;if(word&7)*p=(((word&~7)|3)&~0x78)|0x40;i++;p+=3;}while(i<8);return prim;
    }
    if((D_80122988&0xA100)&&(D_801609A4!=0xFF)){
        func_800A3938(0x7D,0x80);D_801609B4=0;D_80165488=0;D_801609BC=0;D_80160938=0;D_80160928=0;D_801609AC=0;D_801609A4=0xFF;D_801609B8=0;
        D_801609A8^=1;func_801449F0();func_800AA02C();D_80164A4C=0;D_80122988=0;D_80165488=&D_80160574;
    }
    return prim;
}
