/* field_record_setup_ops */
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

/** @brief Eight four-bit values packed into one record word. */
typedef struct
{
    u32 n0 : 4;
    u32 n1 : 4;
    u32 n2 : 4;
    u32 n3 : 4;
    u32 n4 : 4;
    u32 n5 : 4;
    u32 n6 : 4;
    u32 n7 : 4;
} SetupNibbles;

/** @brief Packed 0x40-byte record expanded into the shared sequence configuration. */
typedef struct
{
    u8 pad0[0x14];
    u32 : 8;
    u32 mode : 2;
    u32 primary_index : 6;
    u32 secondary_index : 6;
    u32 : 10;
    SetupNibbles values18;
    SetupNibbles values1c;
    u8 values20[3];
    u8 value23;
    u8 pad24[2];
    u8 values26[6];
    u8 value2c;
    u8 pad2d;
    u8 value2e;
    u8 pad2f[0x11];
} SetupSourceRecord;

extern s32 D_801227F0;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern u8 *D_80122B74;

extern u8 *func_800A9060(void);
extern void func_800BD520(s32 arg0, s32 arg1, s32 arg2);
extern void func_800BE888(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
extern void func_800BEA10(u8 *arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);
extern void func_800BEC44(SetupSourceRecord *record, s32 resource_index);
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
            func_800BEC44((SetupSourceRecord *)(D_80122B74 + offset), g_gosub_result_values[1]);
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
typedef struct FieldResourceCommandEntry
{
    u8 padding[0x244];
    u16 command;
} FieldResourceCommandEntry;
/** @brief Effect setup header followed by packed state bytes. */
typedef struct FieldSequenceHeader
{
    u8 *owner;
    u8 mode;
    u8 rest[0x2D];
} FieldSequenceHeader;
extern u8 *D_80122B78, *D_80123FC0, *g_field_script;

/**
 * @brief Initialize effect state, run its command, and copy the resulting parameters.
 * @param destination Destination record receiving bytes at offsets 0x24 through 0x26.
 * @param mode Effect setup mode.
 * @param subentry Resource subentry selector.
 * @param resource_index Primary resource selector.
 * @param command_selector Command selector relative to 0x40.
 */
void func_800BEA10(u8 *destination, s32 mode, s32 subentry, s32 resource_index, s32 command_selector)
{
    FieldSequenceHeader **config_slot;
    u8 **resource_slot;
    s32 command;
    u8 *command_entry;
    s32 resource_offset;
    u8 *saved_script;
    u8 *loaded_resource;
    s32 index;
    s32 row;
    s32 column;
    u8 flags;
    u8 *flag_entry;
    u8 *reset_entry;

    func_800C21C0(resource_index);
    func_800C21C0(command_selector);
    ((FieldSequenceHeader *)D_80123FC4)->owner = destination;
    ((FieldSequenceHeader *)D_80123FC4)->mode = mode;
    ((u8 *)D_80123FC4)[0x5] = subentry;
    ((u8 *)D_80123FC4)[0x6] = (s8)resource_index;
    index = 0;
    ((u8 *)D_80123FC4)[0x7] = (s8)command_selector;
    do
    {
        flag_entry = (u8 *)D_80123FC4 + index;
        flags = flag_entry[0x20];
        index += 1;
        flag_entry[0x20] = (s8)((flags & 0xF0) | 4);
    } while (index < 8);
    index = 0;
    do
    {
        reset_entry = (u8 *)D_80123FC4 + index;
        index += 1;
        reset_entry[0x28] = 0xFF;
    } while (index < 6);
    resource_slot = &D_80123FC0;
    loaded_resource = func_800C1E40(0xF);
    config_slot = (FieldSequenceHeader **)&D_80123FC4;
    resource_offset = (subentry * 2) + (resource_index * 8);
    *resource_slot = loaded_resource;
    ((u8 *)(*config_slot))[0x2F] = (s8)((loaded_resource + resource_offset)[0x44] & 7);
    ((u8 *)(*config_slot))[0x30] = (s8)((u8)(*resource_slot + resource_offset)[0x44] >> 3);
    ((u8 *)(*config_slot))[0x31] = (u8)(*resource_slot + resource_offset)[0x45];
    command_entry = *resource_slot;
    command_entry += (command_selector - 0x40) * 2;
    command = ((FieldResourceCommandEntry *)command_entry)->command;
    saved_script = g_field_script;
    g_field_script = D_80122B78 + 0xD98;
    func_800BF2F0(command);
    g_field_script = saved_script;
    func_800BFA34();
    destination[0x24] = (u8)((u8 *)(*config_slot))[0x2E];
    {
        u8 *clamp_base = (u8 *)D_80123FC4;

        if ((s8)clamp_base[0x2F] >= 0)
        {
            column = 7;
            if (clamp_base[0x2F] < 8U)
            {
                column = clamp_base[0x2F] & 0xFF;
            }
        }
        else
        {
            column = 0;
        }
    }
    {
        u8 *clamp_base = (u8 *)D_80123FC4;

        if ((s8)clamp_base[0x30] >= 0)
        {
            row = 7;
            if (clamp_base[0x30] < 8U)
            {
                row = clamp_base[0x30] & 0xFF;
            }
        }
        else
        {
            row = 0;
        }
    }
    {
        u8 *resource_base = D_80123FC0;
        destination[0x25] = (resource_base + (column + (row << 3)))[4];
    }
    destination[0x26] = (u8)((u8 *)D_80123FC4)[0x31];
}

/**
 * @brief Expand packed record fields into the shared sequence configuration.
 * @param record Packed source record.
 * @param resource_index Resource-table selector stored in the staged configuration.
 */
void func_800BEC44(SetupSourceRecord *record, s32 resource_index)
{
    u8 *config_ptr;
    s32 config_word24;
    s32 config_word24_2;
    s32 config_word24_3;
    s32 config_word20;
    s32 config_word20_2;
    s32 config_word20_3;
    s32 i;
    u8 mode;
    u8 *config_entry;

    D_80123FC0 = func_800C1E40(4);
    func_800C21C0(resource_index);
    {
        u8 *config = D_80123FC4;
        SETUP_PTR(config, 0) = (u8 *)record;
        SETUP_U8(config, 0x4) = (u8)record->mode;
    }
    SETUP_U8(D_80123FC4, 0x5) = (s8)record->primary_index;
    SETUP_U8(D_80123FC4, 0x6) = (s8)record->secondary_index;
    SETUP_U8(D_80123FC4, 0x7) = resource_index;
    resource_index -= 0x40;
    resource_index *= 4;
    SETUP_U32(D_80123FC4, 0x8) = (s32)(SETUP_U32(D_80123FC4, 0x8) + SETUP_U8(D_80123FC0 + resource_index, 0x684));
    SETUP_U8(D_80123FC4, 0xD) = (s8)record->values18.n0;
    SETUP_U8(D_80123FC4, 0xF) = (s8)record->values18.n1;
    SETUP_U8(D_80123FC4, 0x11) = (s8)record->values18.n2;
    SETUP_U8(D_80123FC4, 0x13) = (s8)record->values18.n3;
    SETUP_U8(D_80123FC4, 0x15) = (s8)record->values18.n4;
    SETUP_U8(D_80123FC4, 0x17) = (s8)record->values18.n5;
    SETUP_U8(D_80123FC4, 0x19) = (s8)record->values18.n6;
    SETUP_U8(D_80123FC4, 0x1B) = (s8)record->values18.n7;
    SETUP_U8(D_80123FC4, 0x1C) = record->value2e;
    config_word20 = (SETUP_U32(D_80123FC4, 0x20) & ~0xF) | record->values1c.n0;
    SETUP_U32(D_80123FC4, 0x20) = config_word20;
    config_word20_2 = (config_word20 & ~0xF00) | (record->values1c.n1 << 8);
    SETUP_U32(D_80123FC4, 0x20) = config_word20_2;
    config_word20_3 = (config_word20_2 & 0xFFF0FFFF) | (record->values1c.n2 << 0x10);
    SETUP_U32(D_80123FC4, 0x20) = config_word20_3;
    SETUP_U32(D_80123FC4, 0x20) = (s32)((config_word20_3 & 0xF0FFFFFF) | (record->values1c.n3 << 0x18));
    i = 0;
    config_word24 = (SETUP_U32(D_80123FC4, 0x24) & ~0xF) | record->values1c.n4;
    SETUP_U32(D_80123FC4, 0x24) = config_word24;
    config_word24_2 = (config_word24 & ~0xF00) | (record->values1c.n5 << 8);
    SETUP_U32(D_80123FC4, 0x24) = config_word24_2;
    config_word24_3 = (config_word24_2 & 0xFFF0FFFF) | (record->values1c.n6 << 0x10);
    SETUP_U32(D_80123FC4, 0x24) = config_word24_3;
    SETUP_U32(D_80123FC4, 0x24) = (s32)((config_word24_3 & 0xF0FFFFFF) | (record->values1c.n7 << 0x18));
    do
    {
        config_entry = D_80123FC4 + i;
        i += 1;
        SETUP_U8(config_entry, 0x50) = 4;
    } while (i < 8);
    i = 0;
    SETUP_U8(D_80123FC4, 0x28) = 0xFF;
    SETUP_U8(D_80123FC4, 0x29) = record->value23;
    do
    {
        SETUP_U8((u8 *)((s32)i + (s32)D_80123FC4), 0x2A) = record->values20[i];
        i += 1;
    } while (i < 3);
    SETUP_U8(D_80123FC4, 0x2D) = 0xFF;
    config_ptr = D_80123FC4;
    mode = SETUP_U8(config_ptr, 0x4);
    switch (mode)
    {
    case 0:
        i = 0;
        do
        {
            SETUP_U8(D_80123FC4 + i, 0x2E) = record->values26[i];
            i += 1;
        } while (i < 6);
        SETUP_U8(D_80123FC4, 0x34) = 0;
        break;
    case 1:
        SETUP_U8(config_ptr, 0x35) = 0;
        SETUP_U8(D_80123FC4, 0x36) = record->value2c;
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


/* func_800BF158 */
#include "common.h"

extern u8 *func_800C1E40(s32);
extern u8 *D_80123FC0;

typedef struct
{
    u8 bytes[0x58];
    unsigned int flags : 4;
    unsigned int other_flags : 28;
} FieldRecordState;

typedef struct
{
    u8 value;
    u8 padding;
} FieldBytePair;

typedef struct
{
    u8 prefix[0xC];
    FieldBytePair pairs[8];
} FieldPairState;



/**
 * @brief Copy the selected resource record into the shared field state.
 */
void func_800BF158(void)
{
    s32 i;
    u8 *record;
    FieldRecordState *state;

    record = func_800C1E40(4);
    do
    {
        state = ((FieldRecordState *)D_80123FC4);
    } while (0);
    D_80123FC0 = record;
    i = 0;
    record += state->bytes[6] * 20;
    *(u16 *)(state->bytes + 0x3A) = *(u16 *)(record + 0x186);
    do
    {
        s32 source_offset;
        u8 *dest;

        source_offset = i + ((FieldRecordState *)D_80123FC4)->bytes[6] * 20;
        dest = ((FieldRecordState *)D_80123FC4)->bytes + i;
        dest[0x3C] = *(D_80123FC0 + source_offset + 0x188);
        i++;
    } while (i < 4);
    i = 0;
    do
    {
        s32 source_offset;
        u8 *dest;

        source_offset = i + ((FieldRecordState *)D_80123FC4)->bytes[6] * 20;
        dest = ((FieldRecordState *)D_80123FC4)->bytes + i;
        dest[0x40] = *(D_80123FC0 + source_offset + 0x18C);
        i++;
    } while (i < 4);
    i = 0;
    do
    {
        s32 source_offset;
        u8 *clear_dest;

        source_offset = i + ((FieldRecordState *)D_80123FC4)->bytes[6] * 20;
        ((FieldPairState *)((FieldRecordState *)D_80123FC4))->pairs[i].value = *(D_80123FC0 + source_offset + 0x190);
        clear_dest = ((FieldRecordState *)D_80123FC4)->bytes + i;
        clear_dest[0x44] = 0;
        i++;
    } while (i < 8);
    i = 1;
    ((FieldRecordState *)D_80123FC4)->flags = 0xF;
    do
    {
        u8 *entry;

        entry = ((FieldRecordState *)D_80123FC4)->bytes + i;
        if (entry[0x28] < 0x10U)
        {
            ((FieldRecordState *)D_80123FC4)->flags = entry[0x28];
        }
        i++;
    } while (i < 5);
}
