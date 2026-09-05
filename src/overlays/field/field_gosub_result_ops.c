#include "common.h"

/**
 * @brief Three-word parameter block forwarded to func_800AAFEC.
 */
typedef struct
{
    s32 unk0; /* 0x00 */
    s32 unk4; /* 0x04 */
    s32 unk8; /* 0x08 */
} UnkStruct80051EC0;

typedef struct
{
    u8 unk0;
    u8 pad1[0xD];
    s16 unkE;
    u8 pad10[4];
    u16 unk14;
} UnkStruct80122C02;

typedef struct
{
    s16 unk0;
    s16 unk2;
    u16 unk4;
} UnkStruct80122C12;

void func_800AAFEC(UnkStruct80051EC0 *arg0);
void func_800B2844(s32 arg0, u8 *arg1, u8 arg2);

extern UnkStruct80051EC0 D_80051EC0;
extern UnkStruct80051EC0 D_80051ECC;
extern u8 D_80045ECC[];
extern s32 D_801227F0;
extern UnkStruct80122C02 D_80122C02;
extern UnkStruct80122C12 D_80122C12;
extern u16 D_80122C16;
extern u16 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern u8 g_menuLayoutBuffer[];

/**
 * @brief Counts active gosub-result entries whose bit 30 flag is set.
 *
 * When there are gosub results, walks the five 0x60-byte entries starting at
 * g_menuLayoutBuffer[0x2EF4]; for each entry whose leading byte is nonzero and
 * whose word at +0x44 has bit 30 set, increments the tally stored to
 * D_80122C16.
 *
 * @note Reads g_gosub_result_count as a full word here, while the other
 *       functions in this file read it as a halfword.
 */
void func_800C7168(void)
{
    s32 count;
    s32 i;
    s32 one = 1;
    u8 *p;

    if (*(s32 *)&g_gosub_result_count != 0)
    {
        count = 0;
        for (i = 0; i < 5; i++)
        {
            p = &g_menuLayoutBuffer[i * 0x60];
            if (p[0x2EF4] != 0 &&
                (((*(u32 *)(p + 0x2F38) >> 30) & 1) == one))
            {
                count++;
            }
        }
    }
    D_80122C16 = count;
}

/**
 * @brief Flag the current gosub result's menu layout entry, or trigger a song-select cue.
 * @note Calls akao_set_song_params with no prototype in scope, matching the field116.c
 *       convention; this is required to match.
 */
void func_800C71D4(void)
{
    s32 idx;
    u8 *base;
    u8 *rec;

    idx = g_gosub_result_values[0];
    if (idx < 5)
    {
        base = g_menuLayoutBuffer;
        rec = &base[idx * 0x60];
        *(u32 *)(rec + 0x2F38) |= 0x40000000;
    }
    else
    {
        akao_set_song_params(0x8002, 0x29, idx, 0);
    }
}

/**
 * @brief Copy the D_80051EC0 constant onto the stack and forward it to func_800AAFEC.
 */
void func_800C7238(void)
{
    UnkStruct80051EC0 local;

    local = D_80051EC0;
    func_800AAFEC(&local);
}

/**
 * @brief Reset D_801227F0 and latch the current gosub result index and count.
 */
void func_800C7278(void)
{
    s32 temp;

    D_801227F0 = 0;
    temp = g_gosub_result_values[0];
    D_80122C12.unk0 = temp;
    D_80122C12.unk4 = g_gosub_result_count;
}

/**
 * @brief Copy the D_80051ECC constant onto the stack and forward it to func_800AAFEC.
 */
void func_800C72A4(void)
{
    UnkStruct80051EC0 sp10;

    sp10 = D_80051ECC;
    func_800AAFEC(&sp10);
}

/**
 * @brief Record the current gosub result index and count, then emit its portrait icon.
 */
void func_800C72E4(void)
{
    s32 temp;

    D_80122C02.unkE = temp = g_gosub_result_values[0];
    D_80122C02.unk14 = g_gosub_result_count;
    func_800B2844(D_80122C02.unk0, (temp * 0x60) + D_80045ECC, 0xFF);
}
