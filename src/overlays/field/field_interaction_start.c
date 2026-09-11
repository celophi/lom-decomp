#include "common.h"

/** @brief Active field context position and packed script selector fields. */
typedef struct
{
    u8 pad0[0x58];
    s32 unk58;
    u8 pad5C[0x41C - 0x5C];
    s32 unk41C;
} Context;
extern Context *D_80122B78;
/** @brief Four-word actor position returned by the position query. */
typedef struct
{
    s32 unk0, unk4, unk8, unkC;
} Position;
extern Position *D_80122B70;

extern u8 *D_80122B74;

void func_800B0BDC(void);
s32 *func_800C1EC8(s32 *src, s32 *dest, s32 n);
void func_800B0C10(void);
void func_800B0C54(void);
void func_800B0D3C(void);
void func_800B0E80(void);
void func_800B0EFC(void);
extern s32 func_800BD414(s32 arg0, s32 arg1);

/**
 * @brief Initializes the field menu/audio subsystem and flags queued song requests.
 */
void func_800B0AF8(void)
{
    func_800B0BDC();
    func_800C1EC8(0, (s32 *)((u8 *)D_80122B78 + 0x400), 0xB04);
    func_800B0C10();
    func_800B0C54();
    func_800B0D3C();
    func_800B0E80();
    func_800B0EFC();

    if (D_80122B74[0x840] != 0)
    {
        if (func_800BD414(0, 0x2F08) == 0xFF)
        {
            akao_set_song_params(0x8001, 0x320, 1, 0);
        }
    }

    if (D_80122B74[0xA90] != 0)
    {
        if (func_800BD414(0, 0x2F00) == 0xFF)
        {
            akao_set_song_params(0x8001, 0x320, 2, 0);
        }
    }
}


typedef struct
{
    u8 pad0[0x404];
    s32 unk404; /* 0x404 */
    u8 pad408[0x410 - 0x408];
    u32 unk410; /* 0x410 */
    u32 unk414; /* 0x414 */
} StructB78;

extern u8 g_menuLayoutBuffer[];
extern s32 D_80122C00;
extern u8* D_80122B74;


/** @brief Bind the shared layout, context, and position buffers. */
void func_800B0BDC(void)
{
    D_80122B74 = g_menuLayoutBuffer;
    D_80122B78 = (Context *)&D_80122C00;
    D_80122B70 = (Position *)0x801ED480;
}


/** @brief Reset pending scene and transition parameters. */
void func_800B0C10(void)
{
    StructB78 *p;
    u32 raw;
    u32 v;

    p = (StructB78 *)D_80122B78;
    p->unk414 = 0x10;
    p->unk404 = -1;
    raw = p->unk410;
    v = raw;
    v &= 0xFFFFFC00;
    v &= 0xFFF003FF;
    v &= 0xC00FFFFF;
    p->unk410 = v;
}


typedef struct
{
    u8 unk0;
    u8 _pad1[3];
    u8* unk4;
} FieldTextMacro;

extern FieldTextMacro D_80122B80[];
extern u8* D_80122B74;
extern s16 D_800EF600;

s32 func_800BD414(s32 arg0, s32 arg1);
void func_800BD520(s32 arg0, u32 arg1, s32 arg2);

/**
 * @brief Initialize the reserved field text macros and their default selection state.
 */
void func_800B0C54(void)
{
    s32 slot;
    s32 last_slot;
    FieldTextMacro* macros;
    FieldTextMacro* macro;

    slot = 0;
    macros = D_80122B80;
    last_slot = 0xF;
    do
    {
        macro = (FieldTextMacro*)(((last_slot - slot) * sizeof(*macro)) + (u32)macros);
        macro->unk0 = 0x15;
        macro->unk4 = D_80122B74 + 0x5F0 + slot * 0x250;
        slot++;
    } while (slot < 3);

    D_80122B80[0xC].unk0 = 0xFF;
    D_80122B80[0xC].unk4 = (u8*)&D_800EF600 + *(s16*)((u8*)&D_800EF600 + ((*(u16*)(D_80122B74 + 0x2E6) & 0x7F) * 2));

    if ((func_800BD414(0, 0xA02) != 0) || ((*(s32*)(D_80122B74 + 0x858) & 0x80) != 0))
    {
        func_800BD520(0, 0xA03, 1);
    }
    else
    {
        func_800BD520(0, 0xA03, 0);
    }
}

/** @brief Initialize actor IDs, event scripts, and script owner selectors. */
void func_800B0D3C(void)
{
    s32 temp_v1;
    s32 current;
    u16 invalid;
    s32 fixed0;
    s32 fixed1;
    s32 fixed2;
    s32 masked1;
    s32 masked2;
    u32 fixed_mask;
    u32 loop_mask;
    u32 mask_d;
    s32 var_a0;
    s32 var_a2;
    s32 var_a3;
    s32 var_t1_2;
    s32 var_t1;
    u8 *temp_a0;
    u8 *var_t0;
    u8 *base;

    var_t1 = 0;
    do
    {
        var_a2 = var_t1 * 0x94;
        temp_a0 = (u8 *)D_80122B78;
        temp_a0 += 1;
        temp_a0 -= 1;
        temp_a0 += var_a2;
        var_a3 = 0;
        var_a3 += 1;
        var_a3 += 1;
        var_a3 -= 2;
        (*(u8 *)((u8 *)temp_a0 + 0x430)) = var_t1;
        (*(s32 *)((u8 *)temp_a0 + 0x4C0)) |= 0x80000000;
        invalid = 0xFFFF;
        (*(s32 *)((u8 *)temp_a0 + 0x4C0)) &= 0xBFFFFFFF;
        mask_d = 0xDFFFFFFF;
        mask_d += (u32)temp_a0;
        mask_d -= (u32)temp_a0;
        (*(s32 *)((u8 *)temp_a0 + 0x4C0)) &= mask_d;
        var_a0 = var_a2;
        (*(u8 *)((u8 *)(u8 *)D_80122B78 + var_a2 + 0x431)) = (s8) (var_t1 - 0x80);
        (*(u8 *)((u8 *)(u8 *)D_80122B78 + var_a2 + 0x434)) = 0xFF;
        do
        {
            do
                    {
                        do
                                {
                                    do
                                            {
                                                do
                                                        {
                                                            do
                                                                    {
                                                                        base = (u8 *)D_80122B78;
                                                                    } while (0);
                                                        } while (0);
                                            } while (0);
                                } while (0);
                    } while (0);
        } while (0);
loop_2:
        (*(u16 *)(base + var_a0 + 0x438)) = invalid;
        var_a3 += 1;
        var_a0 += 2;
        if (var_a3 < 0x10)
        {
            goto loop_2;
        }
        var_t1 += 1;
        (*(u16 *)((u8 *)(u8 *)D_80122B78 + 0x400)) = (u16) ((*(u16 *)((u8 *)(u8 *)D_80122B78 + 0x400)) + 1);
    } while (var_t1 < 3);
    fixed_mask = 0xFFFF01FF;
    var_t1_2 = 3;
    var_a3 = 0xFF;
    var_t0 = (u8 *)D_80122B78 + 0x1BC;
    fixed0 = *(s32 *)((u8 *)D_80122B78 + 0x458);
    fixed1 = *(s32 *)((u8 *)D_80122B78 + 0x4EC);
    fixed2 = *(s32 *)((u8 *)D_80122B78 + 0x580);
    fixed0 &= fixed_mask;
    fixed0 |= 0x6000;
    masked1 = fixed1 & fixed_mask;
    masked1 |= 0x6200;
    masked2 = fixed2 & fixed_mask;
    masked2 |= 0x7000;
    *(s32 *)((u8 *)D_80122B78 + 0x458) = fixed0;
    *(s32 *)((u8 *)D_80122B78 + 0x4EC) = masked1;
    *(s32 *)((u8 *)D_80122B78 + 0x580) = masked2;
    loop_mask = 0xFFFF01FF;
loop_3:
    var_t1_2 += 1;
    temp_v1 = var_a3 & 0x7F;
    var_a3 -= 1;
    current = *(s32 *)((u8 *)var_t0 + 0x458);
    current &= loop_mask;
    current |= temp_v1 << 9;
    (*(s32 *)((u8 *)var_t0 + 0x458)) = current;
    var_t0 += 0x94;
    if (var_t1_2 < 0x10)
    {
        goto loop_3;
    }
    (*(s32 *)((u8 *)(u8 *)D_80122B78 + 0x0)) = 0x40;
}



/**
 * @see decomp.me (100%) TODO
 */
void func_800B0E80(void)
{
    s32 var_a0;
    s32 var_a3;
    s32 var_v1;
    s32 off;
    u8 *base;

    var_a3 = 0;
    do
    {
        off = var_a3 * 0x94;
        ((u8 *)D_80122B78 + off)[0xD70] = var_a3 - 0x80;
        var_a0 = 0;
        ((u8 *)D_80122B78 + off)[0xD71] = 0xFF;
        var_v1 = off;
        ((u8 *)D_80122B78 + off)[0x434] = 0xFF;
        base = (u8 *)D_80122B78;
    loop_2:
        *(u16 *)(base + var_v1 + 0xD78) = 0xFFFF;
        var_a0 += 1;
        var_v1 += 2;
        if (var_a0 < 0x10)
        {
            goto loop_2;
        }
        var_a3 += 1;
    } while (var_a3 < 2);
}


/** @brief Packed field-audio control values stored in the active layout. */
typedef union
{
    u32 flags;
    struct
    {
        u8 unk2E4;
        u8 unk2E5;
        u16 unk2E6;
    } fields;
} FieldAudioControl;

/** @brief Active layout view used to initialize field audio state. */
typedef struct
{
    u8 pad0[0xE4];
    s32 unkE4[8];
    u8 pad104[0x2E4 - 0x104];
    FieldAudioControl control;
    u8 pad2E8[0xC];
    u8 track_data[0x8C][0xC];
} FieldAudioState;


extern u8 D_800F0B48[];
extern u16 g_music_track_index;

s32* func_800C1EC8(s32* src, s32* dest, s32 n);
void func_800BD520(s32 arg0, u32 arg1, s32 arg2);
s32 func_800C3688(s32 arg0);
s32 rand(void);

/**
 * @brief Initialize field audio variables from the active layout and music track.
 */
void func_800B0EFC(void)
{
    s32 flags;

    flags = ((FieldAudioState *)D_80122B74)->control.flags;
    if (flags & 0x800000)
    {
        ((FieldAudioState *)D_80122B74)->control.flags = flags & 0xFF7FFFFF;
        func_800C1EC8(NULL, ((FieldAudioState *)D_80122B74)->unkE4, 0x20);
        func_800BD520(0, 0xFA, rand() & 0xFF);
    }

    if (((FieldAudioState *)D_80122B74)->control.fields.unk2E5 >= 0x12)
    {
        func_800BD520(0, 0xA00, 1);
    }

    func_800BD520(0, 0x429C, ((FieldAudioState *)D_80122B74)->control.fields.unk2E6 & 0x7F);
    func_800BD520(0, 0x4300, D_800F0B48[((FieldAudioState *)D_80122B74)->track_data[g_music_track_index][0]]);
    func_800BD520(0, 0x4304, D_800F0B48[((FieldAudioState *)D_80122B74)->track_data[g_music_track_index][1]]);
    func_800BD520(0, 0x4308, D_800F0B48[((FieldAudioState *)D_80122B74)->track_data[g_music_track_index][2]]);
    func_800BD520(0, 0x430C, D_800F0B48[((FieldAudioState *)D_80122B74)->track_data[g_music_track_index][3]]);
    func_800BD520(0, 0x4310, D_800F0B48[((FieldAudioState *)D_80122B74)->track_data[g_music_track_index][4]]);
    func_800BD520(0, 0x4314, D_800F0B48[((FieldAudioState *)D_80122B74)->track_data[g_music_track_index][5]]);
    func_800BD520(0, 0x4318, D_800F0B48[((FieldAudioState *)D_80122B74)->track_data[g_music_track_index][6]]);
    func_800BD520(0, 0x431C, D_800F0B48[((FieldAudioState *)D_80122B74)->track_data[g_music_track_index][7]]);
    func_800BD520(0, 0x5320, func_800C3688(g_music_track_index));
    func_800BD520(0, 0x5328, g_music_track_index);
}

extern u8 D_800EF84C[];
extern void func_80087F44(u32, s32 *);
extern s32 func_8008B288(u32);
extern u8 *func_800C1B60(u32, Context *);
/**
 * @brief Convert the record selector sentinel to a signed invalid value.
 * @param record Record containing the selector at byte one.
 * @return Selector value, or -1 for the 0xFF sentinel.
 */
static inline s32 resolve_byte(u8 *record)
{
    s32 value = -1;
    if (record[1] != 0xFF)
    {
        value = record[1];
    }
    return value;
}
extern void func_80087CE0(s32, s32);
extern s32 func_80087EF0(s32);
extern void func_8009C620(s32, s32, s32, s32);
extern void func_8009C77C(s32, s32, s32);
extern void func_800B2654(s32 *, s32 *, s32 *, s32 *);
extern s32 func_800B286C(s32, s32, s32);
extern s32 func_800BD414(s32, s32);
extern u8 *func_800C1B98();
extern void func_800C299C(s32);
extern s32 D_8010AE78;



#define FIELD_MENU_SLOT_UNUSED 0xFF
#define FIELD_MENU_SLOT_STRIDE 0x10
#define FIELD_MENU_RECORD_STRIDE 0x8C
#define FIELD_ACTION_ENTRY_FLAG_MASK 0xF

typedef struct
{
    u8 pad0[0x26F0];
    s32 handle;
    u8 entry_index;
    u8 entry_state[3];
} FieldMenuActionSlot;

typedef struct
{
    s8 id;
    u8 pad1[0x8F];
    s32 flags;
} FieldActionEntry;

typedef struct
{
    u8 pad0[0x400];
    u16 count;
    u8 pad1[0x2E];
    FieldActionEntry entries[1];
} FieldActionTable;

typedef struct
{
    s32 flags;
    u8 pad4[8];
    s16 result_type;
} FieldActionRequest;

extern u8 *D_80122B74;


extern void func_80087FC0(s32, s32);
void func_800B0AF8(void);
void func_800B168C(s32);
s32 func_800B1894(FieldActionRequest *, FieldActionEntry **, s32, s32 *);
s32 func_800B22F0(u8, s32);
s32 func_800BD3B0(s32, s32);
void func_800C0490(u8);

/**
 * @brief Install a conditional field action and initialize its event scripts.
 * @note Base translation. Case 3 reads the prior stack slot before replacing
 * it, as the target does; unsupported action kinds also leave that slot unset.
 */
void func_800B118C(u8 *arg0, s32 arg1)
{
    s32 sp14;
    u8 *sp10;
    s32 temp_v0;
    s32 temp_v1_4;
    s32 temp_v1_6;
    s32 var_a2;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v1;
    u16 temp_v0_3;
    u16 temp_v1_2;
    u16 temp_v1_3;
    u16 temp_v1_5;
    u32 temp_v0_2;
    u32 temp_v1;
    u8 *temp_a0;
    u8 *temp_a0_2;
    u8 *temp_a0_3;
    u8 *temp_a2;
    u8 *var_a0;
    u8 *var_a0_2;
    u8 *var_a1;

    if (arg1 == 0)

    {
        func_800B0AF8();
    }
    temp_v0 = func_800BD3B0(0, (*(u16 *)((u8 *)arg0 + 0x4)) << 0x10);
    if ((temp_v0 >= (s32) (*(u8 *)((u8 *)arg0 + 0x6))) && ((s32) (*(u8 *)((u8 *)arg0 + 0x7)) >= temp_v0))
    {
        temp_v1 = arg0[0] >> 4;
        switch (temp_v1)
        {
        case 0:
            (*(s32 *)((u8 *)arg0 + 0x0)) = (s32) ((s32) (*(s32 *)((u8 *)arg0 + 0x0)) | 0x80000000);
            temp_v1_2 = (*(u16 *)((u8 *)(u8 *)D_80122B78 + 0x400));
            (*(u16 *)((u8 *)(u8 *)D_80122B78 + 0x400)) = (u16) (temp_v1_2 + 1);
            temp_a0 = (u8 *)D_80122B78 + (((temp_v1_2 & 0xFFFF) * 0x94) + 0x430);
            sp10 = temp_a0;
            (*(u8 *)((u8 *)temp_a0 + 0x0)) = (s8) (arg1 + 3);
            (*(s32 *)((u8 *)temp_a0 + 0x90)) = (s32) ((*(s32 *)((u8 *)temp_a0 + 0x90)) | 0x80000000);
            (*(s32 *)((u8 *)sp10 + 0x90)) = (s32) (((*(s32 *)((u8 *)sp10 + 0x90)) & ~0xF) | ((s32) (*(s32 *)((u8 *)arg0 + 0x0)) & 0xF));
            (*(s32 *)((u8 *)arg0 + 0x0)) = (s32) ((s32) (*(s32 *)((u8 *)arg0 + 0x0)) & ~0xF);
            (*(s32 *)((u8 *)sp10 + 0x90)) = (s32) (((*(s32 *)((u8 *)sp10 + 0x90)) & ~0x3FF0) | (((*(u16 *)((u8 *)arg0 + 0xC)) * 2) & 0x3FF0));
            sp14 = 0;
block_10:
            (*(u16 *)((u8 *)arg0 + 0xC)) = (u16) ((*(u16 *)((u8 *)arg0 + 0xC)) & 7);
            break;
        case 7:
            (*(s32 *)((u8 *)arg0 + 0x0)) = (s32) ((s32) (*(s32 *)((u8 *)arg0 + 0x0)) | 0x80000000);
            temp_v1_3 = (*(u16 *)((u8 *)(u8 *)D_80122B78 + 0x400));
            (*(u16 *)((u8 *)(u8 *)D_80122B78 + 0x400)) = (u16) (temp_v1_3 + 1);
            temp_a0_2 = (u8 *)D_80122B78 + (((temp_v1_3 & 0xFFFF) * 0x94) + 0x430);
            sp10 = temp_a0_2;
            (*(u8 *)((u8 *)temp_a0_2 + 0x0)) = (s8) (arg1 + 3);
            (*(s32 *)((u8 *)temp_a0_2 + 0x90)) = (s32) ((*(s32 *)((u8 *)temp_a0_2 + 0x90)) | 0x80000000);
            temp_v1_4 = ((*(s32 *)((u8 *)sp10 + 0x90)) & ~0xF) | ((s32) (*(s32 *)((u8 *)arg0 + 0x0)) & 0xF);
            (*(s32 *)((u8 *)sp10 + 0x90)) = temp_v1_4;
            (*(s32 *)((u8 *)sp10 + 0x90)) = (s32) ((temp_v1_4 & ~0x3FF0) | (((*(u16 *)((u8 *)arg0 + 0xC)) * 2) & 0x3FF0));
            temp_v0_2 = (s32) (*(s32 *)((u8 *)arg0 + 0x0)) & ~0xF;
            (*(s32 *)((u8 *)arg0 + 0x0)) = temp_v0_2;
            sp14 = 0;
            if (!((temp_v0_2 >> 0x1C) & 3))
            {
                (*(s32 *)((u8 *)arg0 + 0x0)) = (s32) ((temp_v0_2 & 0xCFFFFFFF) | (((((u8) (*(u8 *)((u8 *)D_80122B74 + 0x29D4)) >> 4) + 1) & 3) << 0x1C));
            }
            goto block_10;
        case 6:
            (*(s32 *)((u8 *)arg0 + 0x0)) = (s32) ((s32) (*(s32 *)((u8 *)arg0 + 0x0)) & 0x7FFFFFFF);
            temp_v1_5 = (*(u16 *)((u8 *)(u8 *)D_80122B78 + 0x400));
            (*(u16 *)((u8 *)(u8 *)D_80122B78 + 0x400)) = (u16) (temp_v1_5 + 1);
            temp_a0_3 = (u8 *)D_80122B78 + (((temp_v1_5 & 0xFFFF) * 0x94) + 0x430);
            sp10 = temp_a0_3;
            (*(u8 *)((u8 *)temp_a0_3 + 0x0)) = (s8) (arg1 + 3);
            sp14 = 0;
            (*(s32 *)((u8 *)temp_a0_3 + 0x90)) = (s32) ((*(s32 *)((u8 *)temp_a0_3 + 0x90)) | 0x80000000);
            var_a0 = temp_a0_3;
            temp_v1_6 = ((*(s32 *)((u8 *)var_a0 + 0x90)) & ~0xF) | ((s32) (*(s32 *)((u8 *)arg0 + 0x0)) & 0xF);
            (*(s32 *)((u8 *)var_a0 + 0x90)) = temp_v1_6;
            var_v0 = temp_v1_6;
            var_v1 = 0x40000000;
block_17:
            (*(s32 *)((u8 *)var_a0 + 0x90)) = (s32) (var_v0 | var_v1);
            break;
        case 2:
            sp14 = 0;
            var_v0_2 = (s32) (*(s32 *)((u8 *)arg0 + 0x0)) & 0x7FFFFFFF;
            var_a0 = (u8 *)D_80122B78 + 0x430;
block_15:
            (*(s32 *)((u8 *)arg0 + 0x0)) = var_v0_2;
            sp10 = var_a0;
block_16:
            var_v0 = (*(s32 *)((u8 *)var_a0 + 0x90));
            var_v1 = 0x80000000;
            goto block_17;
        case 3:
            sp14 = 0;
            (*(s32 *)((u8 *)arg0 + 0x0)) = (s32) ((s32) (*(s32 *)((u8 *)arg0 + 0x0)) & 0x7FFFFFFF);
            var_a0 = sp10;
            sp10 = (u8 *)D_80122B78 + 0x4C4;
            goto block_16;
        case 4:
            sp14 = 0;
            var_v0_2 = (s32) (*(s32 *)((u8 *)arg0 + 0x0)) & 0x7FFFFFFF;
            var_a0 = (u8 *)D_80122B78 + 0x558;
            goto block_15;
        case 1:
            (*(s32 *)((u8 *)arg0 + 0x0)) = (s32) ((s32) (*(s32 *)((u8 *)arg0 + 0x0)) & 0x7FFFFFFF);
            if ((u8) (*(u8 *)((u8 *)arg0 + 0x1)) < 2U)
            {
                func_800C0490((*(u8 *)((u8 *)arg0 + 0x1)));
            }
            temp_a2 = (u8 *)D_80122B78 + 0xD70;
            (*(s32 *)((u8 *)temp_a2 + 0x90)) = (s32) ((*(s32 *)((u8 *)temp_a2 + 0x90)) | 0x80000000);
            *(s32 *)((u8 *)(u8 *)D_80122B78 + 0x400) = *(s32 *)((u8 *)(u8 *)D_80122B78 + 0x400) | 0x60000;
            sp14 = 0;
            sp10 = temp_a2;
            func_800B22F0(0x80, (*(u16 *)((u8 *)arg0 + 0x2E)));
            (*(u16 *)((u8 *)arg0 + 0x2E)) = 0xFFFFU;
            func_800B168C(3);
            break;
        case 5:
            (*(s32 *)((u8 *)arg0 + 0x0)) = (s32) (((s32) (*(s32 *)((u8 *)arg0 + 0x0)) & 0x7FFFFFFF) | (func_800B1894((FieldActionRequest *)arg0, (FieldActionEntry **)&sp10, arg1, &sp14) << 0x1F));
            break;
        }
        var_a2 = 0;
        if (sp10 != NULL)
        {
            var_a1 = arg0;
            (*(u8 *)((u8 *)sp10 + 0x1)) = (u8) (*(u8 *)((u8 *)arg0 + 0x1));
            (*(u8 *)((u8 *)sp10 + 0x4)) = 0xFF;
            var_a0_2 = sp10;
            (*(u16 *)((u8 *)var_a0_2 + 0x6)) = (u16) (*(u16 *)((u8 *)arg0 + 0xE));
            do
            {
                temp_v0_3 = (*(u16 *)((u8 *)var_a1 + 0x10));
                var_a1 += 2;
                var_a2 += 1;
                (*(u16 *)((u8 *)var_a0_2 + 0x8)) = temp_v0_3;
                var_a0_2 += 2;
            } while (var_a2 < 0x10);
            (*(s32 *)((u8 *)sp10 + 0x28)) = (s32) (((*(s32 *)((u8 *)sp10 + 0x28)) & 0xFFFF01FF) | (((*(s32 *)((u8 *)(u8 *)D_80122B78 + 0x0)) & 0x7F) << 9));
            (*(s32 *)((u8 *)(u8 *)D_80122B78 + 0x0)) = (s32) ((*(s32 *)((u8 *)(u8 *)D_80122B78 + 0x0)) + (*(u8 *)((u8 *)arg0 + 0x2)));
            func_800B286C((*(u8 *)((u8 *)sp10 + 0x0)), 0xF, (u8) sp14);
        }
    }
    else
    {
        (*(s32 *)((u8 *)arg0 + 0x0)) = (s32) ((s32) (*(s32 *)((u8 *)arg0 + 0x0)) & 0x7FFFFFFF);
    }
}


typedef struct FieldStateB168C
{
    u8 pad0[0x400];
    u32 flags;
} FieldStateB168C;

extern s32 D_8010AE78;
extern u8 *D_80122B74;


/**
 * @brief Apply an actor-state mode to the first three field actor slots.
 * @param mode Mode controlling which active actor slots receive the state update.
 */
void func_800B168C(s32 mode)
{
    s32 i;
    s32 offset;
    u8 *slot;

    i = 0;
    offset = 0;
    do
    {
        slot = D_80122B74 + offset;
        if (slot[0x5F0] != 0)
        {
            switch (mode)
            {
            case 1:
            case 3:
                func_80087FC0(i, 2);
                break;
            case 2:
                if ((slot[0x608] >> 7) != 0)
                {
                    func_80087FC0(i, 2);
                }
                break;
            }
        }
        offset += 0x250;
        i++;
    } while (i < 3);

    D_8010AE78 = 1;
    ((FieldStateB168C *)D_80122B78)->flags = (((FieldStateB168C *)D_80122B78)->flags & 0xFFF9FFFF) | ((mode & 3) << 17);
}


typedef struct FieldStateB177C
{
    u8 pad0[0x400];
    u32 flags;
} FieldStateB177C;

extern s32 D_8010AE78;
extern u8 *D_80122B74;

extern void func_800C1D14(s32 arg0, s32 arg1);

/**
 * @brief Consume the pending actor-state mode for the first three field actor slots.
 */
void func_800B177C(void)
{
    s32 i;
    s32 mode;
    u8 *slot;

    i = 0;
    do
    {
        mode = (((FieldStateB177C *)D_80122B78)->flags >> 17) & 3;
        switch (mode)
        {
        case 1:
        case 3:
            slot = D_80122B74 + i * 0x250;
            if ((slot[0x608] >> 7) != 0)
            {
                func_80087FC0(i, 0);
                func_800C1D14(i, 0);
            }
            else
            {
                func_80087FC0(i, 1);
                func_800C1D14(i, 0);
            }
            break;
        case 2:
            slot = D_80122B74 + i * 0x250;
            if ((slot[0x608] >> 7) != 0)
            {
                func_80087FC0(i, 0);
                func_800C1D14(i, 0);
            }
            break;
        }
        i++;
    } while (i < 3);

    ((FieldStateB177C *)D_80122B78)->flags &= 0xFFF9FFFF;
    D_8010AE78 = 0;
}

/**
 * @brief Resolve a menu action slot and append its table entry when active.
 * @param arg0 Action request containing packed slot selection and result state.
 * @param arg1 Receives the selected table entry, or NULL when no entry is active.
 * @param arg2 Value used to initialize the selected entry identifier.
 * @param arg3 Receives the selected action index when required by the action type.
 * @return -1 when an entry is appended, or 0 when no entry is selected.
 */
s32 func_800B1894(FieldActionRequest *arg0, FieldActionEntry **arg1, s32 arg2, s32 *arg3)
{
    FieldMenuActionSlot *slot;
    FieldActionEntry *entry;
    FieldActionRequest *request;
    s32 offset;
    u32 handle;
    s32 count;
    s32 index;
    s16 result_type;
    u32 record_index;
    u8 packed;

    request = arg0;
    packed = ((u8 *)request)[1];
    record_index = packed >> 7;
    packed &= 7;
    offset = packed * FIELD_MENU_SLOT_STRIDE + record_index * FIELD_MENU_RECORD_STRIDE;
    slot = (FieldMenuActionSlot *)(D_80122B74 + offset);

    if (slot->entry_index < FIELD_MENU_SLOT_UNUSED)
    {
        handle = slot->handle;
        switch (handle)
        {
        case 0:
            *arg1 = NULL;
            return 0;

        case 1:
            *arg3 = 0;
            request->result_type = 2;
            break;

        case 2:
            *arg3 = slot->entry_index - 0x30;
            result_type = (s16)((*(u32 *)&((FieldMenuActionSlot *)(D_80122B74 + offset))->entry_index >> 8) & 3);
            request->result_type = result_type;
            break;

        case 3:
            *arg3 = slot->entry_index;
            result_type = (s16)((*(u32 *)&((FieldMenuActionSlot *)(D_80122B74 + offset))->entry_index >> 8) & 3);
            request->result_type = result_type;
            break;

        default:
            break;
        }

        count = ((FieldActionTable *)D_80122B78)->count;
        ((FieldActionTable *)D_80122B78)->count = (u16)(count + 1);
        index = count & 0xFFFF;
        entry = &((FieldActionTable *)D_80122B78)->entries[index];
        *arg1 = entry;
        entry->flags |= 0x80000000;
        (*arg1)->id = (s8)(arg2 + 3);
        (*arg1)->flags = ((*arg1)->flags & ~FIELD_ACTION_ENTRY_FLAG_MASK) | (request->flags & FIELD_ACTION_ENTRY_FLAG_MASK);
        request->flags &= ~FIELD_ACTION_ENTRY_FLAG_MASK;
        return -1;
    }

    *arg1 = NULL;
    return 0;
}

void func_800B1AA8(void);
void func_800B1BBC(void);
void func_800B1D10(void);
void func_800B1F10(void);
void func_800B20B4(void);
void func_800B49C0(void);


/**
 * @brief Partial FIELD state used by the frame/update dispatcher.
 */
typedef struct
{
    u8 pad0[0xBC];
    s32 unkBC;
    u8 padC0[0x400 - 0xC0];
    s32 unk400;
    u8 pad404[0x418 - 0x404];
    s32 unk418;
} FieldStateB19FC;



/**
 * @brief Dispatch the current FIELD update path and advance its frame counter.
 *
 * A negative state value selects the reset path. Otherwise bit 30 optionally
 * runs an auxiliary update before the normal update chain. Bit 16 of unk400
 * gates an additional handler, and the per-state frame counter is incremented.
 *
 * @note 100% match with the FIELD GCC 2.8.0 G0 toolchain.
 */
void func_800B19FC(void)
{
    s32 temp_v0;

    temp_v0 = ((FieldStateB19FC *)D_80122B78)->unk418;
    if (temp_v0 < 0)
    {
        func_800B1AA8();
        return;
    }
    if (((u32)temp_v0 >> 30) & 1)
    {
        func_800B1BBC();
    }
    func_800B1D10();
    func_800B1F10();
    func_800B20B4();
    if (((FieldStateB19FC *)D_80122B78)->unk400 & 0x10000)
    {
        func_800B49C0();
    }
    ((FieldStateB19FC *)D_80122B78)->unkBC++;
}


typedef struct
{
    u8 pad0[0x404];
    s32 unk404;
    s32 unk408;
    s32 unk40C;
    u8 pad410[0x418 - 0x410];
    u16 unk418;
    u8 unk41A;
    u8 unk41B;
} FieldStateB1AA8;

void func_800BD520(s32 arg0, u32 arg1, s32 arg2);
s32 func_800BD414(s32 arg0, s32 arg1);
void field_set_scene_parameters(s32 arg0, s32 arg1, u32 arg2, s32 arg3, s32 arg4, s32 arg5);


extern s32 g_pending_game_state;
extern s32 g_layout_sub_mode;
extern s32 g_layout_option;

/**
 * @brief Apply the pending field scene state or dispatch the current scene parameters.
 */
void func_800B1AA8(void)
{
    u16 state;

    func_800BD520(0, 0xFE2, 0);
    state = ((FieldStateB1AA8 *)D_80122B78)->unk418;
    switch (state)
    {
    case 0xFFFE:
        g_pending_game_state = 4;
        ((FieldStateB1AA8 *)D_80122B78)->unk404 = 0xFFFF;
        g_layout_sub_mode = -1;
        g_layout_option = -1;
        return;
    case 0xFFFF:
        ((FieldStateB1AA8 *)D_80122B78)->unk404 = state;
        g_pending_game_state = 1;
        g_layout_sub_mode = -1;
        g_layout_option = -1;
        if (func_800BD414(0, 0xFFF) != 0)
        {
            g_pending_game_state = 0;
            ((FieldStateB1AA8 *)D_80122B78)->unk418 = 1;
            ((FieldStateB1AA8 *)D_80122B78)->unk404 = 0;
        }
        return;
    default:
        field_set_scene_parameters(((FieldStateB1AA8 *)D_80122B78)->unk418, ((FieldStateB1AA8 *)D_80122B78)->unk41A, ((FieldStateB1AA8 *)D_80122B78)->unk41B & 0x1F, ((FieldStateB1AA8 *)D_80122B78)->unk404, ((FieldStateB1AA8 *)D_80122B78)->unk408,
                                   ((FieldStateB1AA8 *)D_80122B78)->unk40C);
        break;
    }
}



extern s32 g_layout_option;

void func_80087FC0(s32 arg0, s32 arg1);
void func_8009AFBC(s32 arg0);
s32 akao_cmd_c1(s32 arg0, s32 arg1, s32 arg2);

/**
 * @brief Advance the field transition state and update its fade and audio state.
 */
void func_800B1BBC(void)
{
    u8 *ptr = ((u8 *)D_80122B78);
    u32 val = *(u32 *)(ptr + 0x418);
    s32 i;
    u32 flags;
    u16 half;
    u32 packed;
    s32 v0;

    if ((val >> 29) & 1)
    {
        goto no_loop;
    }

    for (i = 0; i < 3; i++)
    {
        func_80087FC0(i, 2);
    }

    flags = *(u32 *)(((u8 *)D_80122B78) + 0x418) | 0x20000000;
    *(u32 *)(((u8 *)D_80122B78) + 0x418) = flags;
    if (*(s32 *)(((u8 *)D_80122B78) + 0x414) != 0xFF)
    {
        half = *(u16 *)(((u8 *)D_80122B78) + 0x418);
        if ((u16)(half + 2) >= 2)
        {
            func_8009AFBC(half & 0x7FFF);
        }

        packed = *(u32 *)(((u8 *)D_80122B78) + 0x410);
        field_set_fade_target(packed & 0x3FF, (packed >> 10) & 0x3FF, (packed >> 20) & 0x3FF, *(s32 *)(((u8 *)D_80122B78) + 0x414));

        if (*(u16 *)(((u8 *)D_80122B78) + 0x418) == 0xFFFF)
        {
            s32 shift = *(s32 *)(((u8 *)D_80122B78) + 0x414) << 2;
            g_layout_option = -1;
            akao_cmd_c1(0, shift, 0);
        }

        ptr = ((u8 *)D_80122B78);
        v0 = *(s32 *)(ptr + 0x414);
        v0 = v0 + 1;
        goto tail_write;
    }
    *(u32 *)(((u8 *)D_80122B78) + 0x418) = flags | 0x80000000;
    return;

no_loop:
    if (*(s32 *)(ptr + 0x414) <= 0)
    {
        *(u32 *)(ptr + 0x418) = val | 0x80000000;
    }

    ptr = ((u8 *)D_80122B78);
    v0 = *(s32 *)(ptr + 0x414) - 1;

tail_write:
    *(s32 *)(ptr + 0x414) = v0;
}


/** @brief Trigger bounds and command viewed relative to the table header. */
typedef struct
{
    u8 pad0[4];
    u16 unk4, unk6, unk8, unkA, unkC;
} Region;
/** @brief Field position cache and one-shot trigger table state. */
typedef struct
{
    u8 pad0[0x44];
    s32 positions[3];
    u8 pad50[4];
    s32 unk54, unk58;
    u8 pad5C[0xB8 - 0x5C];
    s32 unkB8;
    u8 padBC[0xF00 - 0xBC];
    u8 *volatile unkF00;
} TriggerContext;
/** @brief Unsigned map coordinates used for trigger bounds checks. */
typedef struct
{
    u16 x, z;
} Point;
extern Position *D_80122B70;

extern Point D_80042FC8;

extern s32 func_800B22F0(u8, s32);
extern void func_800B4410(u16);
/**
 * @brief Pack the horizontal actor coordinates into two 16-bit fields.
 * @param position Fixed-point actor position.
 * @return Packed X and Z coordinates.
 */
static inline s32 pack_position(Position *position)
{
    return ((s32)(u16)(position->unk0 >> 8) << 16) | ((position->unk8 >> 8) & 0xFFFF);
}
/**
 * @brief Refresh actor map positions and dispatch the first newly entered trigger.
 */
void func_800B1D10(void)
{
    Position positions[3];
    s32 *packed_cursor;
    Position *position_cursor;
    s32 region_offset;
    s32 used;
    Point *point;
    s32 region_bit;
    s32 index;

    s32 packed;
    u8 *table;
    Region *region;
    Region *trigger;

    index = 0;
    position_cursor = positions;
    ((TriggerContext *)D_80122B78)->unk54 = (s32)-D_80122B70->unk4;
    packed_cursor = ((TriggerContext *)D_80122B78)->positions;
    ((TriggerContext *)D_80122B78)->unk58 = (s32) - (D_80122B70->unk8 + D_80122B70->unkC);
    do
    {
        packed = ((s32 (*)(s32, Position *))func_80087F44)(index, position_cursor);
        if (packed != -1)
        {
            packed = pack_position(position_cursor);
        }
        *packed_cursor = packed;
        position_cursor++;
        index += 1;
        packed_cursor++;
    } while (index < 3);
    point = &D_80042FC8;
    point->x = (u16)(positions[0].unk0 >> 8);
    point->z = (u16)(positions[0].unk8 >> 8);
    table = ((TriggerContext *)D_80122B78)->unkF00;
    if ((table != 0) &&
        (region_bit = 1, used = ((TriggerContext *)D_80122B78)->unkB8, index = 0, ((*(u16 *)(table + 2)) != 0)))
    {
        region_offset = index;
    loop_7:
        if (!(used & region_bit))
        {
            region = (Region *)(((TriggerContext *)D_80122B78)->unkF00 + region_offset);
            if (((u16)point->x >= (u16)region->unk4) && ((u16)region->unk8 >= (u16)point->x) &&
                ((u16)point->z >= (u16)region->unk6))
            {
                if ((u16)region->unkA >= (u16)point->z)
                {
                    trigger = (Region *)(((TriggerContext *)D_80122B78)->unkF00 + region_offset);
                    ((TriggerContext *)D_80122B78)->unkB8 = (s32)(((TriggerContext *)D_80122B78)->unkB8 | region_bit);
                    if (trigger->unkC & 0x8000)
                    {
                        func_800B22F0(0, trigger->unkC);
                        return;
                    }
                    func_800B4410(trigger->unkC);
                    return;
                }
                goto block_16;
            }
        }
    block_16:
        region_bit *= 2;
        index += 1;
        region_offset += 0xC;
        if (index >= (s32)(*(u16 *)(((TriggerContext *)D_80122B78)->unkF00 + 2)))
        {
        }
        else
        {
            goto loop_7;
        }
    }
}

typedef struct
{
    u8 id;
    u8 pad[0x93];
} FieldActorRec1F10;

typedef struct
{
    u8 pad0[0x400];
    union { u16 count; u32 flags; } actors;
    u8 pad404[0x418 - 0x404];
    u32 state418;
    u8 pad41C[0x430 - 0x41C];
    FieldActorRec1F10 rec[16];
    u8 padD70[0xE98 - (0x430 + 16 * 0x94)];
    u32 script_status;
    s32 script_depth;
    u8 *script_pc;
} FieldState1F10;


extern s32 D_8010AE78;
extern void field_script_run(void *ctx);
extern s32 func_800B286C(s32, s32, s32);
extern s32 func_800BD414(s32 arg0, s32 arg1);
extern void func_800B177C(void);
extern s32 func_8006751C(s32 arg0);

/**
 * @brief Advance the active field script state and dispatch pending actor commands.
 */
void func_800B1F10(void)
{
    s32 i;

    {
        u8 *script_record;
        script_record = (u8 *)D_80122B78;
        script_record += ((FieldState1F10 *)D_80122B78)->script_depth * 12;
        if (*(u32 *)(script_record + 0xEA0) != 0)
        {
            field_script_run((u8 *)D_80122B78 + 0xE98);
            return;
        }
    }

    if (D_8010AE78 != 0)
    {
        i = 0;
        if (((FieldState1F10 *)D_80122B78)->actors.count != 0)
        {
            do
            {
                func_800B286C(((FieldState1F10 *)D_80122B78)->rec[i].id, 0xD, 0x82);
                i++;
            } while (i < (s32)((FieldState1F10 *)D_80122B78)->actors.count);
        }
        if ((((((FieldState1F10 *)D_80122B78)->state418 >> 30) & 1) == 0) && (func_800BD414(0, 0xFE2) == 0))
        {
            func_800B177C();
        }
    }
    else if ((((FieldState1F10 *)D_80122B78)->actors.flags & 0x80000) && (func_8006751C(0) == -1))
    {
        i = 0;
        if (((FieldState1F10 *)D_80122B78)->actors.count != 0)
        {
            do
            {
                func_800B286C(((FieldState1F10 *)D_80122B78)->rec[i].id, 0xD, 0x85);
                i++;
            } while (i < (s32)((FieldState1F10 *)D_80122B78)->actors.count);
        }
        ((FieldState1F10 *)D_80122B78)->actors.flags &= 0xFFF7FFFF;
    }
}


typedef struct
{
    s32 active;
    u8 pad4[8];
} FieldEventEntryB20B4;

typedef struct
{
    u8 pad0[0x2C];
    s32 active_event;
    FieldEventEntryB20B4 events[8];
    u8 pad90[4];
} FieldRecordB20B4;

typedef struct
{
    u8 pad0[0xD70];
    FieldRecordB20B4 records[2];
} FieldStateB20B4;


extern void func_800B28E0(s32 arg0, s32 arg1, s32 arg2);
extern void field_script_run(void* script);

/**
 * @brief Advance the two field event records and re-arm their event flags.
 */
void func_800B20B4(void)
{
    s32 i;
    s32 event_index;
    s32 script_offset;
    u8* record;

    i = 0;
    do
    {
        script_offset = i * 0x94 + 0xD70;
        event_index = ((FieldStateB20B4 *)D_80122B78)->records[i].active_event;
        if (((FieldStateB20B4 *)D_80122B78)->records[i].events[event_index].active != 0)
        {
            field_script_run((u8*)D_80122B78 + script_offset + 0x28);
        }
        record = (u8*)D_80122B78 + i * 0x94;
        func_800B28E0(i + 0x80, record[0xD74], record[0xD75]);
        record = (u8*)D_80122B78 + i * 0x94;
        record[0xD74] = 0xFF;
        func_800B28E0(0x80, 0xE, 0);
        record = (u8*)D_80122B78 + i * 0x94;
        record[0xD74] = 0xFF;
        i += 1;
    } while (i < 2);
}


typedef struct
{
    s32 unk0;
    u8 pad4[8];
} FieldEventEntry;

typedef struct
{
    u8 unk0;
    u8 pad1[3];
    u8 unk4;
    u8 unk5;
    u8 pad6[0x28 - 6];
    s32 unk28;
    s32 unk2C;
    FieldEventEntry entries[8];
    s32 unk90;
} FieldRecordB2198;

typedef struct
{
    u32 x;
    u32 y;
    u32 z;
} FieldPositionB2198;

typedef struct
{
    u8 pad0[0x54];
    u32 x;
    u32 z;
} FieldBoundsB2198;




void func_800B28E0(s32 arg0, s32 arg1, s32 arg2);

void field_script_run(void *script);

/**
 * @brief Update an active field record and dispatch its pending event or script.
 * @param arg0 Record id forwarded to the record lookup and position query.
 * @param arg1 Unused actor-table argument supplied by the caller.
 */
void func_800B2198(s32 arg0, void *arg1)
{
    FieldRecordB2198 *record;
    FieldPositionB2198 position;

    record = (FieldRecordB2198 *)func_800C1B98();
    if ((record != NULL) && (record->unk90 < 0))
    {
        if (record->unk4 != 0xFF)
        {
            func_800B28E0(record->unk0, record->unk4, record->unk5);
            record->unk4 = 0xFF;
        }
        if (!(((u32)record->unk90 >> 30) & 1))
        {
            func_80087F44(arg0, (s32 *)&position);
            if ((position.x > ((FieldBoundsB2198 *)D_80122B78)->x) &&
                (position.z > ((FieldBoundsB2198 *)D_80122B78)->z) &&
                (position.x < ((FieldBoundsB2198 *)D_80122B78)->x + 0x140) &&
                (position.z < ((FieldBoundsB2198 *)D_80122B78)->z + 0x1C0))
            {
                func_800B28E0(record->unk0, 2, 0);
            }
            else
            {
                func_800B28E0(record->unk0, 3, 0);
            }
            func_800B28E0(record->unk0, 0xE, 0);
            if (record->entries[record->unk2C].unk0 == 0)
            {
                func_800B28E0(record->unk0, 8, 0);
                return;
            }
            field_script_run(&record->unk28);
        }
    }
}

/** @brief Start an actor interaction through its script or presentation path. */
s32 func_800B22F0(u8 arg0, s32 arg1)
{
    s32 sp1C;
    s32 sp18;
    s32 sp14;
    s32 sp10;
    s32 var_a2;
    s32 var_a2_2;
    s32 temp_a0;
    s32 var_s0;
    s32 var_s0_2;
    s32 var_s1;
    s32 var_s1_2;
    s32 var_v0;
    s32 var_v0_2;
    u32 temp_v1;
    u8 var_a0;
    u8 var_a0_2;
    u8 *temp_a1;
    u8 *temp_v0;
    u8 *temp_v0_2;

    if ((arg1 & 0xFFFF) != 0xFFFF)

    {
        var_v0 = 0;
        if (!(((u32) *(u32 *)(((u8 *)D_80122B78) + 0x418) >> 0x1E) & 1))
        {
            var_v0 = 0;
            if (func_800BD414(0, 0xFE1) == 0)
            {
                temp_v0 = func_800C1B98(arg0);
                var_v0 = 0;
                if (temp_v0 != NULL)
                {
                    temp_v1 = *(u32 *)(temp_v0 + 0x90);
                    if ((temp_v1 >> 0x1E) & 1)
                    {
                        goto block_5;
                    }
                    var_v0 = 0;
                    if (!((temp_v1 >> 0x1D) & 1))
                    {
                        temp_a0 = (temp_v1 >> 4) & 0x3FF;
                        var_v0_2 = arg1 & 0x8000;
                        if (temp_a0 != 0)
                        {
                            func_800C299C(temp_a0);
                            var_v0_2 = arg1 & 0x8000;
                        }
                        if (var_v0_2 != 0)
                        {
                            var_v0 = 0;
                            if (*(u32 *)((((u8 *)D_80122B78) + (*(s32 *)(((u8 *)D_80122B78) + 0xE9C) * 0xC)) + 0xEA0) == 0)
                            {
                                var_s0 = 0;
                                if (*(u16 *)(((u8 *)D_80122B78) + 0x400) != 0)
                                {
                                    var_s1 = 0;
                                    do
                                    {
                                        if (var_s0 < 3)
                                        {
                                            func_80087CE0(var_s0, 0);
                                        }
                                        else
                                        {
                                            var_a0 = *(u8 *)((((u8 *)D_80122B78) + var_s1) + 0x430);
                                            if (var_a0 == arg0)
                                            {
                                                var_a0 = arg0;
                                                var_a2 = 0x80;
                                            }
                                            else
                                            {
                                                var_a2 = 0x81;
                                            }
                                            func_800B286C(var_a0, 0xD, var_a2);
                                        }
                                        var_s0 += 1;
                                        var_s1 += 0x94;
                                    } while (var_s0 < (s32) *(u16 *)(((u8 *)D_80122B78) + 0x400));
                                }
                                D_8010AE78 = 1;
                                ((u8 *)D_80122B78)[0xE98] = arg0;
                                *(u32 *)(((u8 *)D_80122B78) + 0xE98) = (s32) (((s32) *(u32 *)(((u8 *)D_80122B78) + 0xE98) & 0xFFFF01FF) | (*(u32 *)(temp_v0 + 0x28) & 0xFE00));
                                *(u32 *)((((u8 *)D_80122B78) + (*(s32 *)(((u8 *)D_80122B78) + 0xE9C) * 0xC)) + 0xEA0) = func_80087EF0(arg1 & 0x7FFF);
                                temp_v0_2 = ((u8 *)D_80122B78) + (*(s32 *)(((u8 *)D_80122B78) + 0xE9C) * 0xC);
                                *(u32 *)(temp_v0_2 + 0xEA8) = (s32) (*(u32 *)(temp_v0_2 + 0xEA8) & ~1);
                                temp_a1 = ((u8 *)D_80122B78) + (*(s32 *)(((u8 *)D_80122B78) + 0xE9C) * 0xC);
                                *(u32 *)(temp_a1 + 0xEA8) = (s32) (*(u32 *)(temp_a1 + 0xEA8) & 1);
                                return -1;
                            }

                            return var_v0;
                        }
                        var_s0_2 = 0;
                        if (*(u16 *)(((u8 *)D_80122B78) + 0x400) != 0)
                        {
                            var_s1_2 = 0;
                            do
                            {
                                if (var_s0_2 < 3)
                                {
                                    func_80087CE0(var_s0_2, 0);
                                }
                                else
                                {
                                    var_a0_2 = *(u8 *)((((u8 *)D_80122B78) + var_s1_2) + 0x430);
                                    if (var_a0_2 == arg0)
                                    {
                                        var_a0_2 = arg0;
                                        var_a2_2 = 0x83;
                                    }
                                    else
                                    {
                                        var_a2_2 = 0x84;
                                    }
                                    func_800B286C(var_a0_2, 0xD, var_a2_2);
                                }
                                var_s0_2 += 1;
                                var_s1_2 += 0x94;
                            } while (var_s0_2 < (s32) *(u16 *)(((u8 *)D_80122B78) + 0x400));
                        }
                        sp10 = (s32) arg0;
                        sp14 = 0xFF;
                        sp18 = 0xFE;
                        sp1C = 0xFF;
                        *(u32 *)(((u8 *)D_80122B78) + 0x400) = (s32) (*(u32 *)(((u8 *)D_80122B78) + 0x400) | 0x80000);
                        func_800B2654(&sp10, &sp14, &sp18, &sp1C);
                        func_8009C620(sp14, sp1C, sp10, sp18);
                        func_8009C77C(sp14, arg1 & 0xFFFF, 1);
                        var_v0 = -1;

                        return var_v0;
                    }

                    return var_v0;
                }
            }
        }
        return var_v0;
    }
block_5:
    return 0;
}

/**
 * @brief Resolve script actor, plane, effect, and selector operands in place.
 * @param actor_id Actor identifier; invalid identifiers are replaced with zero.
 * @param plane Plane selector or automatic-selection sentinel.
 * @param effect Effect selector, updated with facing flags or -1 when unavailable.
 * @param selector Selector index or current-context sentinel.
 */
void func_800B2654(s32 *actor_id, s32 *plane, s32 *effect, s32 *selector)
{
    s32 position[3];
    s32 plane_value;
    s32 effect_value;
    s32 facing_flag;
    s32 selector_value;
    u32 original_actor_id;

    original_actor_id = *actor_id;
    if (original_actor_id < 0x80U)
    {
        func_80087F44(original_actor_id, position);
    }
    else
    {
        *actor_id = 0;
    }
    plane_value = *plane;
    if (plane_value != 0xFF)
    {
        if (plane_value & 0x80)
        {
            facing_flag = 0;
        }
        else if (plane_value & 0x40)
        {
            facing_flag = 0x40;
        }
        else
        {
            facing_flag = ((u32)(func_8008B288(*actor_id) - 0x41) < 0x80U) << 6;
        }
        *plane &= 3;
    }
    else
    {
        if ((position[2] - D_80122B78->unk58) <= 0xBFFF)
        {
            *plane = 0;
        }
        else
        {
            *plane = 1;
        }
        facing_flag = ((u32)(func_8008B288(*actor_id) - 0x41) < 0x80U) << 6;
    }
    D_80122B78->unk41C = (s32)((D_80122B78->unk41C & ~0x300) | ((*plane & 3) << 8));
    effect_value = *effect;
    switch (effect_value)
    { /* irregular */
    case 0xFE:
        *effect = resolve_byte(func_800C1B60(original_actor_id, D_80122B78));
        break;
    case 0xFF:
        *effect = -1;
        break;
    }
    *effect |= facing_flag;
    selector_value = *selector;
    if (selector_value == 0xFF)
    {
        selector_value = (s32)(u8)D_80122B78->unk41C;
    }
    *selector = selector_value;
    if (!(((s32)D_800EF84C[selector_value] >> *plane) & 1))
    {
        *effect = -1;
    }
}

/**
 * @brief Set a field text macro's type and backing record.
 * @param arg0 Macro slot index, checked against the upper bound only.
 * @param arg1 Backing record pointer.
 * @param arg2 Macro type.
 */
void func_800B2844(s32 arg0, u8* arg1, u8 arg2)
{
    if (arg0 < 0x10)
    {
        D_80122B80[arg0].unk0 = arg2;
        D_80122B80[arg0].unk4 = arg1;
    }
}
