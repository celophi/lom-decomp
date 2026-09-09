#include "common.h"

/**
 * @brief Object record at D_80122C0C shared by the wrappers below.
 */
typedef struct
{
    s16 unk0; /* 0x00 */
    s16 unk2; /* 0x02 */
    u8 pad4[2];
    s16 unk6; /* 0x06 */
    s16 unk8; /* 0x08 */
    s16 unkA; /* 0x0A */
    s16 unkC; /* 0x0C */
} UnkStruct80122C0C;

void func_800B2844(s32 arg0, u8 *arg1, u8 arg2);
void func_80087F44(s16 arg0, s32 *arg1);
void func_80087D8C(s16 arg0, s32 arg1, s32 arg2, s32 arg3);

extern u8 D_800459AE;
extern s32 D_80122C00;
extern UnkStruct80122C0C D_80122C0C;
extern s16 D_80122C10;
extern u8 g_menuLayoutBuffer[];

/**
 * @brief Set D_80122C10 to 1 when D_800459AE is at least 0x28, else 0.
 */
void func_800C61D8(void)
{
    if (D_800459AE >= 0x28)
    {
        D_80122C10 = 1;
    }
    else
    {
        D_80122C10 = 0;
    }
}

/**
 * @brief Thin wrapper around func_800AD0C8.
 */
void func_800C6208(void)
{
    func_800AD0C8();
}

/**
 * @brief Dispatch each populated row of the selected menu entry and count them.
 */
void func_800C6228(void)
{
    s32 count;
    s32 i;
    s32 entry_offset;
    s32 row_offset;
    u8 *menu;
    u8 *base;

    count = 0;
    i = 0;
    menu = g_menuLayoutBuffer;
    base = menu + 0x2B58;
loop:
    row_offset = i << 6;
    entry_offset = menu[D_80122C00 + 0x29D8] * 0x14C;
    if (menu[row_offset + entry_offset + 0x2B58] != 0)
    {
        entry_offset += (s32)base;
        func_800B2844(count, (u8 *)(entry_offset + row_offset), 0xFF);
        count += 1;
    }
    i += 1;
    if (i < 4)
    {
        goto loop;
    }
    D_80122C10 = count;
}

/**
 * @brief Count the empty 0x40-byte slots and subtract them from D_80122C10, clamping at 0.
 */
void func_800C62E8(void)
{
    s32 i;
    s32 count;
    u8 *p;
    u16 v0;
    s16 *ptr;

    count = 0;
    for (i = 0; i < 0x64; i++)
    {
        p = &g_menuLayoutBuffer[i * 0x40];
        if (p[0xCE0] == 0)
        {
            count++;
        }
    }
    ptr = &D_80122C10;
    if (count >= *ptr)
    {
        v0 = 0;
    }
    else
    {
        v0 = (u16) *ptr - count;
    }
    *ptr = v0;
}

/**
 * @brief Thin stack-frame wrapper around func_800C3A00 with a fixed arg.
 */
void func_800C6344(void)
{
    func_800C3A00(0x92BC);
}

/**
 * @brief Forward the D_80122C0C object id and position pair to func_80087680.
 */
void func_800C6364(void)
{
    func_80087680(D_80122C0C.unk0, D_80122C0C.unk2, D_80122C0C.unk2, 0, 0, 0);
}

/**
 * @brief Fetch a 3D position for the current object, scale it down and re-emit it.
 *
 * Queries func_80087F44 for the object named by @c D_80122C0C.unk0 into a local
 * vector, divides each component by 256 (rounding toward zero via the +0xFF
 * negative fix-up), subtracts @c D_80122C0C.unk6 from the Y component, and passes
 * the result to func_80087D8C.
 */
void func_800C63A0(void)
{
    s32 vec[3];
    s32 x;
    s32 y;
    s32 z;
    s32 yshift;
    s32 y2;

    func_80087F44(D_80122C0C.unk0, vec);
    x = vec[0];
    if (x < 0)
    {
        x += 0xFF;
    }
    y = vec[1];
    x >>= 8;
    vec[0] = x;
    if (y < 0)
    {
        y += 0xFF;
    }
    z = vec[2];
    yshift = y >> 8;
    vec[1] = yshift;
    if (z < 0)
    {
        z += 0xFF;
    }
    z >>= 8;
    vec[2] = z;
    y2 = yshift - D_80122C0C.unk6;
    vec[1] = y2;
    func_80087D8C(D_80122C0C.unk0, x, y2, z);
}

/**
 * @brief Move the selected object toward its target position with decreasing steps.
 * @note Nonmatching m2c translation; target components are signed halfwords,
 * but negating the vertical component produces a full signed word.
 */
void func_800C642C(void)
{
    s32 pos[3];
    s32 temp_a1;
    s32 temp_a0;
    s32 temp_a0_2;
    s32 temp_a0_3;
    s32 temp_a0_4;
    s32 temp_a2;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v1;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 var_v1_4;

    func_80087F44(D_80122C0C.unk0, pos);
    var_v0 = pos[0];
    if (var_v0 < 0)
    {
        var_v0 += 0xFF;
    }
    var_v1 = pos[1];
    temp_a2 = var_v0 >> 8;
    pos[0] = temp_a2;
    if (var_v1 < 0)
    {
        var_v1 += 0xFF;
    }
    var_v0_2 = pos[2];
    temp_a0 = var_v1 >> 8;
    pos[1] = temp_a0;
    if (var_v0_2 < 0)
    {
        var_v0_2 += 0xFF;
    }
    pos[2] = var_v0_2 >> 8;
    temp_a1 = -D_80122C0C.unkC;
    if ((temp_a0 == temp_a1) || (temp_a0 < temp_a1))
    {
        temp_a0_2 = temp_a2 - D_80122C0C.unk8;
        var_v1_2 = temp_a0_2;
        if (temp_a0_2 < 0)
        {
            var_v1_2 = -var_v1_2;
        }
        if ((var_v1_2 * 2) >= 4)
        {
            pos[0] = D_80122C0C.unk8 + ((temp_a0_2 * 2) / 3);
        }
    else if (var_v1_2 > 0)
    {
            pos[0] = temp_a2 - (temp_a0_2 / var_v1_2);
        }
    else
    {
            pos[0] = (s32) D_80122C0C.unk8;
        }
    }
    if ((pos[1] == temp_a1) || (pos[1] < temp_a1))
    {
        temp_a0_3 = pos[2] - D_80122C0C.unkA;
        var_v1_3 = temp_a0_3;
        if (temp_a0_3 < 0)
        {
            var_v1_3 = -var_v1_3;
        }
        if ((var_v1_3 * 2) >= 4)
        {
            pos[2] = D_80122C0C.unkA + ((temp_a0_3 * 2) / 3);
        }
    else if (var_v1_3 > 0)
    {
            pos[2] -= temp_a0_3 / var_v1_3;
        }
    else
    {
            pos[2] = (s32) D_80122C0C.unkA;
        }
    }
    if (((pos[0] == D_80122C0C.unk8) && (pos[2] == D_80122C0C.unkA)) || (pos[1] >= temp_a1))
    {
        temp_a0_4 = pos[1] - temp_a1;
        var_v1_4 = temp_a0_4;
        if (temp_a0_4 < 0)
        {
            var_v1_4 = -var_v1_4;
        }
        if ((var_v1_4 * 2) >= 4)
        {
            pos[1] = temp_a1 + ((temp_a0_4 * 2) / 3);
        }
    else if (var_v1_4 > 0)
    {
            pos[1] -= temp_a0_4 / var_v1_4;
        }
    else
    {
            pos[1] = (s32) temp_a1;
        }
    }
    func_80087D8C(D_80122C0C.unk0, pos[0], pos[1], pos[2]);
}


typedef struct
{
    u8 raw[0x25];
} CopyBuf;

extern CopyBuf D_80051CBC;
extern u8 g_menuLayoutBuffer[];
extern s32 D_80122C00;
extern s32 g_gosub_result_values[];
extern s32 D_80122C08;

void func_800A54D0(void);

/**
 * @brief Translate the selected resource id to a layout palette index and upload it.
 * @note Nonmatching C. The 37-byte stack copy retains the target table size;
 * valid selection ids address that copy after subtracting 0x60.
 */
void func_800C66DC(void)
{
    CopyBuf tmp;
    s32 offset;
    u16 half_val;
    s32 raw_val;
    s32 clamped;
    u8 *base;
    u8 idx;

    tmp = D_80051CBC;

    offset = g_gosub_result_values[0];
    half_val = *(u16 *)g_gosub_result_values;
    raw_val = *((u8 *)&tmp + offset - 0x60);
    *(u16 *)&D_80122C08 = half_val;

    if (raw_val >= 0x20)
    {
        raw_val = 0;
    }

    if (raw_val < 0)
    {
        clamped = 0;
    }
    else if (raw_val >= 0x20)
    {
        clamped = 0x1F;
    }
    else
    {
        clamped = raw_val;
    }

    base = g_menuLayoutBuffer;
    idx = base[D_80122C00 + 0x29D8];
    *(s32 *)(base + idx * 332 + 0x2B54) = clamped;
    func_800A54D0();
}


extern s16 D_80122C06;
extern s16 g_akao_song_cmd_arg0;
extern u8 D_800459AF;

/**
 * @brief Copy the selected command parameter to the audio and layout state.
 * @see decomp.me (100%) N/A -- trivial 7-instruction leaf function, no scratch needed.
 */
void func_800C6834(void)
{
    s32 temp = D_80122C06;
    g_akao_song_cmd_arg0 = temp;
    D_800459AF = temp;
}


extern s32 func_800A4744(void);
extern s32 func_800A4778(void);
extern u16 D_80122C16;
extern s32 D_800F19CC;

/** @brief Store the selected result, using the alternate result on failure. */
void func_800C6850(void)
{
    s32 result = func_800A4744();

    if (result < 0)
    {
        D_80122C16 = 1;
        *(&D_80122C16 - 1) = func_800A4778();
    }
    else
    {
        D_80122C16 = 0;
        *(&D_80122C16 - 1) = result;
    }
}

/**
 * @brief Thin stack-frame wrapper around field_open_gosub_screen_sequence passing &D_800F19CC.
 */
void func_800C68A4(void)
{
    field_open_gosub_screen_sequence(&D_800F19CC);
}
