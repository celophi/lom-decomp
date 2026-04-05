#ifndef _CHECKPS_H
#define _CHECKPS_H

#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/memory.h"

typedef struct {
    u8  deviceState;     // 0x00 - status / mode flag
    u8  _pad1;
    u16 buttonData;      // 0x02 - raw 16-bit input (pre-remap)

    u8  _pad2[0x28];     // 0x04–0x2B - unused here

    s16 axisX;           // 0x2C - signed axis (negative/positive thresholded)
    s16 axisY;           // 0x2E - signed axis (negative/positive thresholded)
} SCDRegs;

extern s32 g_previousGameState;
extern s32 g_textBufferAddr;
extern s8 g_TextBuffer[];
extern s32 g_characterCache[256];

extern s32 D_8005D060;
extern u32 D_80052428;
extern s32 D_80061088;
extern u8 D_8005D088;
extern s32 D_8005D068[4];
extern s32 D_8005D078[3];
extern s32 D_800610A0;
extern s32 D_80061094;
extern s32 D_80061098;
extern u8  D_8005B744[];
extern u8 D_801ED600;
extern s32 D_80061090;
extern s32 D_800610A4;
extern s32 D_800610A8;
extern s32 D_8005CFE8;
extern s32 D_800894C8;
extern s32 D_800894C0;
extern s32 D_800894C4;
extern s32 D_800894CC;
extern s32 D_800894D0;
extern s32 D_800894D4;
extern u16 D_8005D030[];

typedef struct
{
  u16 sp20;
  u16 sp22;
  s16 sp24;
} Sp20Data;

typedef struct {
    union {
        s32 unk0;
        struct {
            u8 _pad0[3];
            u8 unk3;
        } byte;
    } u;
    u8  unk4;
    u8  unk5;
    u8  unk6;
    u8  unk7;
    u16 unk8;
    u16 unkA;
    s8  unkC;
    s8  unkD;
    u16 unkE;
} SomeStruct;

/**
 * Represents a single glyph's entry in the text cache, storing its ID and validity flag
 */
typedef union {
    u32 raw;
    struct {
        u16 charId; 
        struct {
            u16 isCached : 1;  // Bit 16
            u16 reserved : 15; // Bits 17-31
        } flags;
    };
} CharacterCacheEntry;

void func_80050080(void);
void func_8004FEE8(int param_1);
void func_8004FD68(int param_1);

/**
 * @brief Resets the text renderer state and buffers.
 * 
 * @details This function initializes the text rendering system by clearing the 
 * character cache (256 entries) and zeroing out the global text buffer (32KB). 
 * It also resets the image loading state by loading a minimal 1x16 rectangle 
 * at the bottom of the screen area to clear the GPU's current text-related 
 * texture state.
 * 
 * @param void No parameters.
 * @return void No return value.
 * 
 * @see decomp.me link (100%) https://decomp.me/scratch/Bdkvp
 */
void ResetTextRenderer(void);

#endif