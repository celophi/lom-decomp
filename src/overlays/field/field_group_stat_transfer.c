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

