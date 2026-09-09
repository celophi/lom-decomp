#include "common.h"
extern u8 *D_80123FC4;
extern u8 *D_80123FC0;
extern u8 *D_80122B78;
extern u8 *g_field_script;
void func_800BEF74(void);
void func_800BF158(void);
void func_800BF3D8(void);
void func_800BF800(void);
void func_800BF700(void);
void func_800BFF90(s32);
void func_800C015C(s32);
#define SETUP_U8(p,o) (*(u8 *)((u8 *)(p)+(o)))
#define SETUP_U16(p,o) (*(u16 *)((u8 *)(p)+(o)))
#define SETUP_U32(p,o) (*(u32 *)((u8 *)(p)+(o)))
#define SETUP_PTR(p,o) (*(u8 **)((u8 *)(p)+(o)))



extern s32 D_801227F0;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern u8 *D_80122B74;

extern u8 *func_800A9060(void);
extern void func_800BD520(s32 arg0, s32 arg1, s32 arg2);
extern void func_800BE888(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
extern void func_800BEA10(u8 *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);
extern void func_800BEC44(u8 *arg0, s32 arg1);
extern s32 *func_800C1EC8(s32 *src, s32 *dest, s32 n);

/**
 * @brief Dispatch a queued gosub result to the handler selected by its kind.
 * @param arg0 Gosub-result kind selector.
 */
void func_800BE710(s32 arg0)
{
    u8 *handle;

    D_801227F0 = 0;
    func_800C1EC8(NULL, (s32 *)D_80123FC4, 0x60);

    if (g_gosub_result_count == 0)
    {
        goto count_zero;
    }

    switch (arg0)
    {
        case 2:
            handle = func_800A9060();
            if (handle != NULL)
            {
                func_800BEA10(handle, 2, g_gosub_result_values[0], g_gosub_result_values[1],
                              g_gosub_result_values[2]);
                {
                    s32 offset = (s32)handle - 0xCE0;
                    func_800BD520(0, 0x7100, (offset - (s32)D_80122B74) >> 6);
                }
            }
            else
            {
                func_800BD520(0, 0x7100, 0xFE);
            }
            break;
        case 3:
        {
            s32 offset = (g_gosub_result_values[0] << 6) + 0xCE0;
            func_800BEC44(D_80122B74 + offset, g_gosub_result_values[1]);
            func_800BD520(0, 0x7100, g_gosub_result_values[0]);
            break;
        }
        default:
            handle = func_800A9060();
            if (handle != NULL)
            {
                func_800BE888((s32)handle, arg0, g_gosub_result_values[0], g_gosub_result_values[1]);
                {
                    s32 offset = (s32)handle - 0xCE0;
                    func_800BD520(0, 0x7100, (offset - (s32)D_80122B74) >> 6);
                }
            }
            else
            {
                func_800BD520(0, 0x7100, 0xFE);
            }
            break;
    }
    return;

count_zero:
    func_800BD520(0, 0x7100, 0xFF);
}


typedef struct
{
    s32 owner;
    u8 mode;
    u8 primary_record;
    u8 secondary_record;
    u8 tertiary_record;
} FieldSequenceConfig;



void func_800C21C0(s32 arg0);
void func_800BEF74(void);

/**
 * @brief Initialize and flush the shared field sequence configuration.
 * @param owner Owning object or handle stored in the sequence configuration.
 * @param mode Sequence mode selector.
 * @param primary_record Primary record index.
 * @param secondary_record Secondary record index.
 */
void func_800BE888(s32 owner, s32 mode, s32 primary_record, s32 secondary_record)
{
    FieldSequenceConfig *config;
    s32 i;

    func_800C21C0(secondary_record);

    config = (FieldSequenceConfig *)D_80123FC4;
    config->mode = mode;
    i = 0;
    config->owner = owner;
    ((FieldSequenceConfig *)D_80123FC4)->primary_record = primary_record;
    ((FieldSequenceConfig *)D_80123FC4)->secondary_record = secondary_record;
    ((FieldSequenceConfig *)D_80123FC4)->tertiary_record = 0xFF;

    for (; i < 8; i++)
    {
        u8 *flag_entry;
        u8 *value_entry;

        flag_entry = (u8 *)D_80123FC4;
        flag_entry += i;
        flag_entry[0x20] = (flag_entry[0x20] & 0xF0) | 4;
        value_entry = (u8 *)D_80123FC4;
        value_entry += i;
        value_entry[0x50] = 4;
    }

    for (i = 0; i < 6; i++)
    {
        u8 *entry;

        entry = (u8 *)D_80123FC4;
        entry += i;
        entry[0x28] = 0xFF;
    }

    ((u8 *)D_80123FC4)[0x2E] = ((u8 *)D_80123FC4)[5] << 4;
    ((u8 *)D_80123FC4)[0x2F] = (((u8 *)D_80123FC4)[5] << 4) + 0xC;
    ((u8 *)D_80123FC4)[0x30] = (((u8 *)D_80123FC4)[5] << 4) + 0xD;
    ((u8 *)D_80123FC4)[0x31] = (((u8 *)D_80123FC4)[5] << 4) + 0xE;
    ((u8 *)D_80123FC4)[0x32] = (((u8 *)D_80123FC4)[5] << 4) + 0xF;
    ((u8 *)D_80123FC4)[0x33] = 0xFF;

    func_800BEF74();
}

extern void func_800BF2F0(s32);
extern void func_800BFA34(void);
extern u8 *func_800C1E40(s32);
extern void func_800C21C0(s32);
/** @brief Resource table view exposing the command halfword at offset 0x244. */
typedef struct ResourceRow
{
    u8 padding[0x244];
    u16 command;
} ResourceRow;
/** @brief Effect setup header followed by packed state bytes. */
typedef struct Header
{
    u8 *owner;
    u8 mode;
    u8 rest[0x2D];
} Header;
extern u8 *D_80122B78, *D_80123FC0, *g_field_script;

/**
 * @brief Initialize effect state, run its command, and copy the resulting parameters.
 * @param arg0 Destination record receiving bytes at offsets 0x24 through 0x26.
 * @param arg1 Effect setup mode.
 * @param arg2 Resource subentry selector.
 * @param arg3 Primary resource selector.
 * @param arg4 Command selector relative to 0x40.
 */
void func_800BEA10(u8 *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    Header **state;
    u8 **resource;
    s32 command;
    u8 *clamp_base;
    s32 temp_a0;
    u8 *temp_s1;
    u8 *temp_v0_3;
    s32 var_a0;
    s32 var_a0_3;
    s32 var_a1;
    u8 temp_v1;
    s32 temp_v1_2;
    s32 temp_v1_3;
    u8 *temp_v0;
    u8 *temp_v0_2;

    func_800C21C0(arg3);
    func_800C21C0(arg4);
    ((Header *)D_80123FC4)->owner = arg0;
    ((Header *)D_80123FC4)->mode = arg1;
    ((u8 *)D_80123FC4)[0x5] = arg2;
    ((u8 *)D_80123FC4)[0x6] = (s8)arg3;
    var_a0 = 0;
    ((u8 *)D_80123FC4)[0x7] = (s8)arg4;
    do
    {
        temp_v0 = (u8 *)D_80123FC4 + var_a0;
        temp_v1 = temp_v0[0x20];
        var_a0 += 1;
        temp_v0[0x20] = (s8)((temp_v1 & 0xF0) | 4);
    } while (var_a0 < 8);
    var_a0 = 0;
    do
    {
        temp_v0_2 = (u8 *)D_80123FC4 + var_a0;
        var_a0 += 1;
        temp_v0_2[0x28] = 0xFF;
    } while (var_a0 < 6);
    resource = &D_80123FC0;
    temp_v0_3 = func_800C1E40(0xF);
    state = (Header **)&D_80123FC4;
    temp_a0 = (arg2 * 2) + (arg3 * 8);
    *resource = temp_v0_3;
    ((u8 *)(*state))[0x2F] = (s8)((temp_v0_3 + temp_a0)[0x44] & 7);
    ((u8 *)(*state))[0x30] = (s8)((u8)(*resource + temp_a0)[0x44] >> 3);
    ((u8 *)(*state))[0x31] = (u8)(*resource + temp_a0)[0x45];
    command = ((ResourceRow *)(*resource + ((arg4 - 0x40) * 2)))->command;
    temp_s1 = g_field_script;
    g_field_script = D_80122B78 + 0xD98;
    func_800BF2F0(command);
    g_field_script = temp_s1;
    func_800BFA34();
    arg0[0x24] = (u8)((u8 *)(*state))[0x2E];
    clamp_base = (u8 *)D_80123FC4;
    temp_v1_2 = clamp_base[0x2F];
    if ((s8)clamp_base[0x2F] >= 0)
    {
        var_a1 = 7;
        if (temp_v1_2 < 8U)
        {
            var_a1 = temp_v1_2 & 0xFF;
        }
    }
    else
    {
        var_a1 = 0;
    }
    clamp_base = (u8 *)D_80123FC4;
    temp_v1_3 = clamp_base[0x30];
    if ((s8)clamp_base[0x30] >= 0)
    {
        var_a0_3 = 7;
        if (temp_v1_3 < 8U)
        {
            var_a0_3 = temp_v1_3 & 0xFF;
        }
    }
    else
    {
        var_a0_3 = 0;
    }
    arg0[0x25] = (u8)(D_80123FC0 + (var_a1 + (var_a0_3 * 8)))[4];
    arg0[0x26] = (u8)((u8 *)D_80123FC4)[0x31];
}

/** @brief Expands packed record fields into the shared sequence configuration.
 * @note Initial nonmatching C recovered from assembly.
 */
void func_800BEC44(u8 *arg0, s32 arg1)
{
    u8 *var_a0;
    u8 *var_v0;
    u8 *var_v0_2;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 var_t1;
    s32 var_t1_2;
    s32 var_t1_3;
    u8 temp_v1_5;
    u8 *temp_v0_4;
    u8 *temp_v1_4;
    u8 *temp_v1_6;

    D_80123FC0 = func_800C1E40(4);
    func_800C21C0(arg1);
    SETUP_PTR(D_80123FC4, 0) = arg0;
    SETUP_U8(D_80123FC4, 0x4) = (u8) (((u32) SETUP_U32(arg0, 0x14) >> 8) & 3);
    SETUP_U8(D_80123FC4, 0x5) = (s8) (((u32) SETUP_U32(arg0, 0x14) >> 0xA) & 0x3F);
    SETUP_U8(D_80123FC4, 0x6) = (s8) (SETUP_U16(arg0, 0x16) & 0x3F);
    var_t1 = 0;
    SETUP_U8(D_80123FC4, 0x7) = arg1;
    SETUP_U32(D_80123FC4, 0x8) = (s32) (SETUP_U32(D_80123FC4, 0x8) + SETUP_U8(D_80123FC0 + (arg1 - 0x40) * 4, 0x684));
    SETUP_U8(D_80123FC4, 0xD) = (s8) (SETUP_U32(arg0, 0x18) & 0xF);
    SETUP_U8(D_80123FC4, 0xF) = (s8) (SETUP_U8(arg0, 0x18) >> 4);
    SETUP_U8(D_80123FC4, 0x11) = (s8) (((u32) SETUP_U32(arg0, 0x18) >> 8) & 0xF);
    SETUP_U8(D_80123FC4, 0x13) = (s8) (((u32) SETUP_U32(arg0, 0x18) >> 0xC) & 0xF);
    SETUP_U8(D_80123FC4, 0x15) = (s8) (SETUP_U16(arg0, 0x1A) & 0xF);
    SETUP_U8(D_80123FC4, 0x17) = (s8) (((u32) SETUP_U32(arg0, 0x18) >> 0x14) & 0xF);
    SETUP_U8(D_80123FC4, 0x19) = (s8) (SETUP_U8(arg0, 0x1B) & 0xF);
    SETUP_U8(D_80123FC4, 0x1B) = (s8) ((u32) SETUP_U32(arg0, 0x18) >> 0x1C);
    SETUP_U8(D_80123FC4, 0x1C) = (u8) SETUP_U8(arg0, 0x2E);
    temp_v1 = (SETUP_U32(D_80123FC4, 0x20) & ~0xF) | (SETUP_U32(arg0, 0x1C) & 0xF);
    SETUP_U32(D_80123FC4, 0x20) = temp_v1;
    temp_v1_2 = (temp_v1 & ~0xF00) | ((SETUP_U8(arg0, 0x1C) >> 4) << 8);
    SETUP_U32(D_80123FC4, 0x20) = temp_v1_2;
    temp_v1_3 = (temp_v1_2 & 0xFFF0FFFF) | ((((u32) SETUP_U32(arg0, 0x1C) >> 8) & 0xF) << 0x10);
    SETUP_U32(D_80123FC4, 0x20) = temp_v1_3;
    SETUP_U32(D_80123FC4, 0x20) = (s32) ((temp_v1_3 & 0xF0FFFFFF) | ((((u32) SETUP_U32(arg0, 0x1C) >> 0xC) & 0xF) << 0x18));
    temp_v0 = (SETUP_U32(D_80123FC4, 0x24) & ~0xF) | (SETUP_U16(arg0, 0x1E) & 0xF);
    SETUP_U32(D_80123FC4, 0x24) = temp_v0;
    temp_v0_2 = (temp_v0 & ~0xF00) | (((u32) SETUP_U32(arg0, 0x1C) >> 0xC) & 0xF00);
    SETUP_U32(D_80123FC4, 0x24) = temp_v0_2;
    temp_v0_3 = (temp_v0_2 & 0xFFF0FFFF) | ((SETUP_U8(arg0, 0x1F) & 0xF) << 0x10);
    SETUP_U32(D_80123FC4, 0x24) = temp_v0_3;
    SETUP_U32(D_80123FC4, 0x24) = (s32) ((temp_v0_3 & 0xF0FFFFFF) | (((u32) SETUP_U32(arg0, 0x1C) >> 0x1C) << 0x18));
    do
    {
        temp_v0_4 = D_80123FC4 + var_t1;
        var_t1 += 1;
        SETUP_U8(temp_v0_4, 0x50) = 4;
    } while (var_t1 < 8);
    var_t1_2 = 0;
    SETUP_U8(D_80123FC4, 0x28) = 0xFF;
    SETUP_U8(D_80123FC4, 0x29) = (u8) SETUP_U8(arg0, 0x23);
    var_v0 = arg0;
    do
    {
        temp_v1_4 = var_t1_2 + D_80123FC4;
        var_t1_2 += 1;
        SETUP_U8(temp_v1_4, 0x2A) = (u8) SETUP_U8(var_v0, 0x20);
        var_v0 = arg0 + var_t1_2;
    } while (var_t1_2 < 3);
    SETUP_U8(D_80123FC4, 0x2D) = 0xFF;
    var_a0 = D_80123FC4;
    temp_v1_5 = SETUP_U8(var_a0, 0x4);
    switch (temp_v1_5)
    {                            /* irregular */
    case 0:
        var_t1_3 = 0;
        var_a0 = (u8 *)&D_80123FC4;
        var_v0_2 = arg0;
        do
        {
            temp_v1_6 = D_80123FC4 + var_t1_3;
            var_t1_3 += 1;
            SETUP_U8(temp_v1_6, 0x2E) = (u8) SETUP_U8(var_v0_2, 0x26);
            var_v0_2 = arg0 + var_t1_3;
        } while (var_t1_3 < 6);
        SETUP_U8(D_80123FC4, 0x34) = 0;
        break;
    case 1:
        SETUP_U8(var_a0, 0x35) = 0;
        SETUP_U8(D_80123FC4, 0x36) = (u8) SETUP_U8(arg0, 0x2C);
        break;
    }
    func_800BEF74();
}


/** @brief Compact per-frame config block populated before a sequence flush. */
typedef struct
{
    s32 unk0;   /* 0x00 owning object pointer */
    u8 unk4;    /* 0x04 mode selector */
    u8 unk5;    /* 0x05 primary record index */
    u8 unk6;    /* 0x06 secondary record index */
    u8 unk7;    /* 0x07 tertiary record index (biased by 0x40) */
} Cfg;

/** @brief 0xC-stride record view into the D_80123FC0 table. */
typedef struct
{
    u8 pad0[4];
    u16 unk4;   /* 0x04 */
    u16 unk6;   /* 0x06 */
    u8 pad8[0xBC];
    u16 unkC4;  /* 0xC4 */
    u16 unkC6;  /* 0xC6 */
} Rec0C;

/** @brief 0x14-stride record view into the D_80123FC0 table. */
typedef struct
{
    u8 pad0[0x184];
    u16 unk184; /* 0x184 */
} Rec14;

/** @brief 4-stride record view into the D_80123FC0 table. */
typedef struct
{
    u8 pad0[0x686];
    u16 unk686; /* 0x686 */
} Rec4;



extern u8 *D_80123FC0;


/**
 * @brief Flush the staged config block through the sequence emitter chain.
 *
 * Redirects the active sequence buffer @c g_field_script to the scratch region at
 * @c D_80122B78 + 0xD98, emits the note/param records selected by the config
 * indices (using @c unk4 to pick the base vs. alternate field), restores the
 * buffer, and finally dispatches the mode-0/mode-1 finaliser.
 *
 * @see decomp.me (100%) TODO
 */
void func_800BEF74(void)
{
    s32 temp_s2;
    Rec0C *new_var;
    Rec0C *new_var2;
    u16 var_a0;
    u16 var_a0_2;

    temp_s2 = (s32)g_field_script;
    g_field_script = D_80122B78 + 0xD98;
    func_800BF158();
    if (((Cfg *)D_80123FC4)->unk4 == 0)
    {
        new_var = (Rec0C *)(D_80123FC0 + (((Cfg *)D_80123FC4)->unk5 * 3 << 2));
        var_a0 = new_var->unk4;
    }
    else
    {
        var_a0 = (new_var2 = (Rec0C *)(D_80123FC0 + (((Cfg *)D_80123FC4)->unk5 * 3 << 2)))->unkC4;
    }
    func_800BF2F0(var_a0);
    {
        Rec14 *r14 = (Rec14 *)(D_80123FC0 + (((Cfg *)D_80123FC4)->unk6 * 5 << 2));
        func_800BF2F0(r14->unk184);
    }
    func_800BF2F0(((Rec4 *)(D_80123FC0 + ((((Cfg *)D_80123FC4)->unk7 - 0x40) << 2)))->unk686);
    func_800BF3D8();
    func_800BF800();
    if (((Cfg *)D_80123FC4)->unk4 == 0)
    {
        var_a0_2 = ((Rec0C *)(D_80123FC0 + (((Cfg *)D_80123FC4)->unk5 * 3 << 2)))->unk6;
    }
    else
    {
        var_a0_2 = ((Rec0C *)(D_80123FC0 + (((Cfg *)D_80123FC4)->unk5 * 3 << 2)))->unkC6;
    }
    func_800BF2F0(var_a0_2);
    func_800BF700();
    g_field_script = (u8 *)temp_s2;
    func_800BFA34();
    switch (((Cfg *)D_80123FC4)->unk4)
    {
    case 0:
        func_800BFF90(((Cfg *)D_80123FC4)->unk0);
        return;
    case 1:
        func_800C015C(((Cfg *)D_80123FC4)->unk0);
        return;
    }
}
