/**
 * @file field_input_text_session.c
 * @brief Field controller/pad context, actor-selection text session, and input
 *        repeat handling (vram 0x800A8CFC .. 0x800AA570).
 *
 * Consolidated translation unit merged from the per-function FIELD sources.
 * Symbols whose declared type differs between the original files (g_pad_ctx,
 * D_800FE3A0, D_8011F3D2, D_800FDF58, D_80105AE0) are declared at block scope
 * inside each user with that function's original type, and are deliberately
 * kept out of file scope: GCC 2.7.2 accepts the incompatible block-scope
 * externs (warning only) and emits identical code, whereas a file-scope copy
 * that conflicts with a block-scope one is a hard error.
 */

#include "common.h"
#include "vector.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

/* ------------------------------------------------------------------------- */
/* Shared record types                                                       */
/* ------------------------------------------------------------------------- */

/** @brief Large saved-history record with a leading encoded name (from main.h). */
typedef struct
{
    u8 name[0x15];
    u8 unknown_0x15[0x44 - 0x15];
    u8 unknown_0x44; /**< Packed indices into two menu text tables. */
    u8 unknown_0x45;
    u8 unknown_0x46;
    u8 unknown_0x47;
    s32 unknown_0x48; /**< Index into a menu text table. */
    u8 unknown_0x4C[0x14C - 0x4C];
} LargeHistoryRecord;

/** @brief Compact saved-history record with a leading encoded name (from main.h). */
typedef struct
{
    u8 name[0x15];
    u8 unknown_0x15[0x60 - 0x15];
} SmallHistoryRecord;

/**
 * @brief Controller/pad context object (partial layout, from main.h).
 * @note Only fields used by the decompiled controller paths are mapped.
 */
typedef struct
{
    u8  _pad000[0x28];          /**< 0x000: not yet mapped. */
    u32 menu_option_flags;      /**< 0x028: menu audio/vibration option bits. */
    u8  _pad02C[0x840 - 0x2C]; /**< 0x02C: not yet mapped. */
    u8  inject_enable;          /**< 0x840: non-zero allows input injection. */
    u8  _pad841[0x858 - 0x841]; /**< 0x841: not yet mapped. */
    u32 inject_flags;       /**< 0x858: bit 0x80 enables input injection. */
    u8  _pad85C[0x234];          /**< 0x85C: not yet mapped. */
    u8  gname_name[0x18];        /**< 0xA90: name buffer edited by the GNAME overlay. */
    u32 unkAA8;
    u8  _padAAC[0x29D7 - 0xAAC];  /**< 0xAAC: not yet mapped. */
    s8  large_history_index;      /**< 0x29D7: slot index into @c large_history_records. */
    u8  _pad29D8[0x2B0C - 0x29D8];/**< 0x29D8: not yet mapped. */
    LargeHistoryRecord large_history_records[3];
    u32 small_history_index;      /**< 0x2EF0: slot index into @c small_history_records. */
    SmallHistoryRecord small_history_records[5];
} PadContext;

/** @brief Header fields of the pad context written by func_800A8D10. */
typedef struct
{
    u8  pad00[0x18];
    u32 unk18;   /* 0x18 */
    s16 unk1C;   /* 0x1C */
    s8  unk1E;   /* 0x1E */
    u8  pad1F;
    u32 unk20;   /* 0x20 */
    s16 unk24;   /* 0x24 */
    s8  unk26;   /* 0x26 */
    s8  unk27;   /* 0x27 */
    u8  pad28[0xCF - 0x28];
    s8  unkCF;   /* 0xCF */
} PadCtxHeader;

/** @brief Two-byte CD-error status string descriptor (func_800A92CC). */
typedef struct
{
    u8 unk0;
    u8 unk1;
} StructEC;

/** @brief Text ordering context passed to func_800A92CC. */
typedef struct
{
    u8 pad0[0x3C];
    u8 unk3C;
    u8 pad3D[0x40B8 - 0x3D];
    s32 unk40B8;
} ArgA;

/** @brief Actor sub-part presence record scanned by func_800A9198. */
typedef struct
{
    u8 pad0[0x25];
    u8 unk25;
    u8 pad26[0x54 - 0x26];
} StructFE054;

/** @brief 0x23C-byte field group record inspected by func_800A9198. */
typedef struct
{
    u8 pad0[4];
    s32 unk4;
    u8 pad8[0x10 - 8];
    s32 unk10;
    u8 pad14[0x64 - 0x14];
    s32 unk64;
    u8 pad68[0x23C - 0x68];
} Struct106194;

/** @brief 0x48-byte part record holding saved sub-part bytes (func_800A9198). */
typedef struct
{
    u8 pad0[0x2E];
    u8 unk2E;
    u8 pad2F[0x33 - 0x2F];
    u8 unk33;
    u8 pad34[0x48 - 0x34];
} RecFE3A0;

/** @brief Packed screen coordinates consumed by the sprite builders. */
typedef struct
{
    s16 x;
    s16 y;
} FieldLabelPosition;

/** @brief 0xAE-byte field resource record scanned by func_800A9D70. */
typedef struct
{
    u8 id;
    u8 pad1;
    u16 flags;
    u8 pad4[0x2C - 4];
    s16 state_a;
    s16 state_b;
    u8 pad30[0xAE - 0x30];
} FieldResourceRecord;

/** @brief Connection, raw input, and feedback bytes for the two controller ports. */
typedef struct
{
    u8 connected;
    u8 unknown1;
    u16 buttons;
    u8 pad4[0x8D];
    u8 feedback91, feedback92;
    u8 pad93[0x1B];
    u8 connected2;
    u8 padaf[0x90];
    u8 feedback13f, feedback140;
} FieldInputHardware;

/** @brief Flags, presence, and state in a 0x54-byte field actor record. */
typedef struct
{
    u8 pad0[0x1C];
    u32 flags;
    u8 pad20[5];
    u8 presence;
    u8 pad26[4];
    s16 state;
    u8 tail[0x28];
} FieldInputActor;

/** @brief Value at offset 0x14 in a 0x23C-byte actor slot (func_800AA098). */
typedef struct
{
    u8 pad0[0x14];
    s32 value;
    u8 tail[0x224];
} FieldInputSlot;

/** @brief Partial 0x23C-byte actor slot layout used by func_800AA498. */
typedef struct
{
    s32 unk0; /* 0x00 */
    s32 unk4; /* 0x04 */
    u8 pad8[0x23C - 0x8];
} Struct_D80105AE0;

/* ------------------------------------------------------------------------- */
/* Access macros                                                             */
/* ------------------------------------------------------------------------- */

#define PAD_HEADER ((PadCtxHeader *)g_pad_ctx)
#define PAD_BYTES ((u8 *)g_pad_ctx)

#define IS_DBCS_LEAD_BYTE(byte) (((byte) >= 0x19) && ((byte) <= 0x1F))
#define NAME_GLYPH_SIZE_SINGLE 1
#define NAME_GLYPH_SIZE_DOUBLE 2

/** @brief Byte access in a partially recovered field record. */
#define U8_AT(p, o) (*(u8 *)((s32)(p) + (o)))
/** @brief Halfword access in the text-offset tables or controller sample. */
#define U16_AT(p, o) (*(u16 *)((s32)(p) + (o)))
/** @brief Word access in the render context, actor slot, or position record. */
#define S32_AT(p, o) (*(s32 *)((s32)(p) + (o)))

#define FIELD_RESOURCE_RECORDS ((FieldResourceRecord *)0x801ED600)

/* ------------------------------------------------------------------------- */
/* File-scope data (non-conflicting; conflicting symbols are block-scope)    */
/* ------------------------------------------------------------------------- */

/* Field word/amount accumulator list. */
extern s32 D_801227F8[];
extern s32 D_80122908;
extern u8 D_80122910[];

/* Selected-actor bookkeeping. */
extern u8 D_801226E0[];
extern u8 D_801227D0;
extern u8 D_801228D0[];
extern u8 D_801228E0[];

/* Field / actor tables. */
extern StructFE054 D_800FE054[];
extern Struct106194 D_80106194[];
extern s32 D_800FE754;
extern s32 D_800FDFC8;

/* Text/label offset tables (byte views). */
extern u8 D_800EC2FC[];
extern u8 D_800EC3C4[];
extern u8 D_800EC3E0[];
extern u8 D_800EC3E6[];
extern u8 D_800EC3E8[];
extern u8 D_800ED064[];
extern u8 D_800EDBE4[];
extern u8 D_800FD818[];
extern u8 D_8010A028[];
extern u8 D_8010A038[];

/* CD-error status string descriptors (func_800A92CC). */
extern StructEC D_800EC3D2;
extern StructEC D_800EC3D4;

/* Camera offsets. */
extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;

/* Input repeat state. */
extern s32 D_8012269C;
extern s32 D_801227BC;
extern s32 D_801227C0;
extern s32 D_801227C8;
extern s32 D_801227D8;
extern s32 D_801227E4;
extern s32 D_801229F8;
extern s32 D_80122984;

/* Menu-open guards / miscellaneous field state. */
extern s32 D_8010AE78;
extern s32 D_80122710;
extern s32 D_80122714;
extern s32 D_800F2298;
extern s32 D_800F229C;
extern s32 g_field_return_to_title_prompt_state;
extern s32 D_8012291C;
extern s32 D_80122980;
extern u8 D_801227B8[2];
extern u8 D_801227B9;
extern s32 g_pending_game_state;
extern s32 g_field_draw_count;
extern s32 g_frame_counter;

/* Shared main-executable globals (from main.h). g_pad_ctx is intentionally
   omitted here - it is type-conflicted and declared per-function. */
extern u8 g_menuLayoutBuffer[];
extern u16 g_music_track_index;
extern s32 g_save_slot_index;
extern s32 g_pad_input;
extern s32 g_pad_input_inject;

/* ------------------------------------------------------------------------- */
/* File-scope function prototypes (non-conflicting)                          */
/* ------------------------------------------------------------------------- */

void field_text_reset_scratch(void);
void func_80063194(void);
void akao_stop_sfx_by_id(s32 id);
void akao_cmd_99_9b_9d_9f(s32 arg0);
void akao_cmd_98_9a_9c_9e(s32 arg0);
void func_800A3904(s32 arg0, s32 arg1, s32 arg2);
void func_800A3938(s32 arg0, s32 arg1);
void field_restore_fade_target(void);
void func_8008C7A8(void);
void func_800AA858(s32 arg0);
s32 func_8006751C(s32 arg0);
s32 func_800B0850(void);
void func_800AA570(void *arg0, s32 arg1);
void func_800AA7A4(void);
s32 func_8005B218(void);
void func_800AEE28(void);
void *func_80086184(SPRT *, u32 *, s32, u32 *);
void *func_800AD208(s32 *, void *, s32, s32, u16 *, s32);
void *func_800AD524(u8 *, s32 *, s32, s32 *, s32);
s32 func_800AE864(u8 *);

/* Forward declarations for members called before their definition. */
void func_800A92CC(ArgA *arg0);
void func_800A939C(void *arg0);
void func_800AA02C(void);
s32 func_800AA498(void);

/* ------------------------------------------------------------------------- */
/* Function definitions (ascending address order)                            */
/* ------------------------------------------------------------------------- */

/**
 * @brief Point the pad context at the menu layout buffer.
 */
void func_800A8CFC(void)
{
    extern PadContext *g_pad_ctx;

    g_pad_ctx = (PadContext *)g_menuLayoutBuffer;
}

/**
 * @brief Store the field entry parameters into the pad context header.
 * @param arg0 Stored to unk24.
 * @param arg1 Stored to unk26.
 * @param arg2 Stored to unk27.
 * @param arg3 Low 25 bits stored into unk18.
 * @param arg4 Stored to unk1C.
 * @param arg5 Stored to unk1E.
 */
void func_800A8D10(s16 arg0, s8 arg1, s8 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    extern PadContext *g_pad_ctx;

    PAD_HEADER->unk24 = arg0;
    PAD_HEADER->unk26 = arg1;
    PAD_HEADER->unk27 = arg2;
    PAD_HEADER->unk18 = (PAD_HEADER->unk18 & 0xFE000000) | (arg3 & 0x01FFFFFF);
    PAD_HEADER->unk1C = (s16)arg4;
    PAD_HEADER->unk1E = (s8)arg5;
    PAD_HEADER->unk20 = (PAD_HEADER->unk20 & 0xFFFC0000) | g_music_track_index;
    /* The target reads only the low byte of the s32 g_save_slot_index. */
    PAD_HEADER->unkCF = *(u8 *)&g_save_slot_index;
}

/**
 * @brief Append a (word, byte) pair to the ten-entry list at D_801227F8 / D_80122910.
 * @param arg0 Word stored at the next free index.
 * @param arg1 Byte stored at the same index.
 */
void func_800A8D8C(s32 arg0, u8 arg1)
{
    s32 index = D_80122908;

    if (index < 10)
    {
        D_801227F8[index] = arg0;
        D_80122910[index] = arg1;
        D_80122908++;
    }
}

/**
 * @brief Byte length of a NUL-terminated name, counting DBCS lead bytes 0x19-0x1F as two bytes.
 * @param arg0 Name string.
 * @return Byte count excluding the terminator.
 */
s32 func_800A8DDC(u8 *arg0)
{
    s32 count;
    u8 c;

    count = 0;
    c = *arg0;
    if (c != 0)
    {
        do
        {
            if ((u32)(c - 0x19) < 7)
            {
                arg0 += 2;
                count += 2;
            }
            else
            {
                arg0 += 1;
                count += 1;
            }
            c = *arg0;
        } while (c != 0);
    }
    return count;
}

/**
 * @brief Copy a NUL-terminated name, honouring DBCS glyph widths.
 * @param dest Destination buffer; receives the terminator.
 * @param src Source name.
 */
void func_800A8E28(u8 *dest, u8 *src)
{
    volatile u8 *p;
    s32 len;
    s32 i;

    p = (volatile u8 *)src;
    len = 0;
    while (*p != 0)
    {
        if ((u32)(*p - 0x19) < 7)
        {
            p += 2;
            len += 2;
        }
        else
        {
            p += 1;
            len += 1;
        }
    }
    for (i = 0; i < len; i++)
    {
        dest[i] = src[i];
    }
    dest[i] = 0;
}

/**
 * @brief Append a NUL-terminated name onto another, honouring DBCS glyph widths.
 * @param destination Existing name; the source is appended after its last glyph.
 * @param source Name to append.
 */
void func_800A8EAC(u8 *destination, const u8 *source)
{
    const u8 *scan_cursor;
    s32 destination_byte_count;
    s32 source_byte_count;
    s32 append_offset;
    s32 byte_index;

    scan_cursor = destination;
    destination_byte_count = 0;
    while (*scan_cursor)
    {
        if (IS_DBCS_LEAD_BYTE(*scan_cursor))
        {
            scan_cursor += NAME_GLYPH_SIZE_DOUBLE;
            destination_byte_count += NAME_GLYPH_SIZE_DOUBLE;
        }
        else
        {
            scan_cursor += NAME_GLYPH_SIZE_SINGLE;
            destination_byte_count += NAME_GLYPH_SIZE_SINGLE;
        }
    }

    scan_cursor = source;
    source_byte_count = 0;
    append_offset = destination_byte_count;
    while (*scan_cursor)
    {
        if (IS_DBCS_LEAD_BYTE(*scan_cursor))
        {
            scan_cursor += NAME_GLYPH_SIZE_DOUBLE;
            source_byte_count += NAME_GLYPH_SIZE_DOUBLE;
        }
        else
        {
            scan_cursor += NAME_GLYPH_SIZE_SINGLE;
            source_byte_count += NAME_GLYPH_SIZE_SINGLE;
        }
    }

    for (byte_index = 0; byte_index < source_byte_count; byte_index++)
    {
        destination[byte_index + append_offset] = source[byte_index];
    }
    destination[byte_index + append_offset] = 0;
}

/**
 * @brief Copy a fixed 0x40-byte record from src to dst.
 * @param dst Destination buffer.
 * @param src Source buffer.
 */
void func_800A8F8C(u8 *dst, u8 *src)
{
    u32 i;

    i = 0;
    do
    {
        i++;
        *dst++ = *src++;
    } while (i < 0x40);
}

/**
 * @brief Compact the active field-object table, then clear the tail.
 *
 * Walks the 0x64 fixed-size (0x40-byte) records starting at @c g_pad_ctx +
 * 0xCE0: each live record is moved down to the next free slot (via bcopy) and
 * its old copy zeroed, closing gaps. Any slots left between the compacted end
 * and @c g_pad_ctx + 0x25E0 are then zeroed.
 *
 * @see decomp.me (100%) TODO
 */
void func_800A8FB4(void)
{
    extern PadContext *g_pad_ctx;
    s32 var_s2;
    u8 *var_s0;
    u8 *var_s1;

    var_s2 = 0;
    var_s0 = PAD_BYTES + 0xCE0;
    var_s1 = var_s0;
    do
    {
        if (*var_s1 != 0)
        {
            if (var_s1 != var_s0)
            {
                bcopy(var_s1, var_s0, 0x40);
                *var_s1 = 0;
            }
            var_s0 += 0x40;
        }
        var_s2 += 1;
        var_s1 += 0x40;
    } while (var_s2 < 0x64);
    while ((u32)var_s0 < (u32)(PAD_BYTES + 0x25E0))
    {
        *var_s0 = 0;
        var_s0 += 0x40;
    }
}

/**
 * @brief Find the first free inventory record in g_pad_ctx's item table.
 * @return Pointer to the first record whose first byte is 0, or NULL if all
 *         0x64 records are occupied.
 */
u8 *func_800A9060(void)
{
    extern PadContext *g_pad_ctx;
    s32 count;
    u8* rec;

    rec = PAD_BYTES + 0xCE0;
    for (count = 0; count < 0x64; count++)
    {
        if (*rec == 0)
        {
            return rec;
        }
        rec += 0x40;
    }
    return NULL;
}

/**
 * @brief Reset text scratch, run one of two handlers on arg0 by D_80122984, then call func_80063194.
 * @param arg0 Forwarded to func_800A92CC or func_800A939C.
 */
void func_800A909C(void *arg0)
{
    field_text_reset_scratch();

    if (D_80122984)
    {
        func_800A92CC(arg0);
    }
    else
    {
        func_800A939C(arg0);
    }

    func_80063194();
}

/**
 * @brief Restores per-part bytes saved for each active field actor.
 *
 * Stops sound effect 0x7E, then walks the active actor-index table and copies
 * the saved bytes back to offsets 0x2E and 0x33 of each 0x48-byte part record.
 */
void func_800A90F8(void)
{
    extern u8 D_800FE3A0[];
    s32 i;
    u8 *rec;

    akao_stop_sfx_by_id(0x7E);
    for (i = 0; i < D_801227D0; i++)
    {
        rec = D_800FE3A0 + D_801226E0[i] * 0x48;
        rec[0x2E] = D_801228D0[i];
        rec[0x33] = D_801228E0[i];
    }
}

/**
 * @brief Save actor sub-part state for live actors matching the current field group.
 */
void func_800A9198(void)
{
    extern RecFE3A0 D_800FE3A0[];
    extern s8 D_8011F3D2;
    StructFE054 *t0;
    Struct106194 *a0;
    s32 a2;
    u8 *base0;

    func_800AA02C();
    akao_cmd_99_9b_9d_9f(2);
    func_800A3904(0, 0x3C, 0);
    akao_stop_sfx_by_id(0x7E);
    t0 = D_800FE054;
    a2 = 3;
    base0 = (u8 *)D_80106194;
    a0 = (Struct106194 *)base0;
    D_801227D0 = 0;
    D_8011F3D2 = 0;
    do
    {
        if (t0->unk25 != 0xFF && a0->unk4 != 0 && D_800FE754 == (a0->unk10 & 0xF) && a0->unk64 != 0)
        {
            D_801226E0[D_801227D0] = a2;
            D_801228D0[D_801227D0] = D_800FE3A0[a2].unk2E;
            D_801228E0[D_801227D0] = D_800FE3A0[a2].unk33;
            D_801227D0 += 1;
        }
        a2 += 1;
        a0 = (Struct106194 *)((u8 *)a0 + 0x23C);
        t0 = (StructFE054 *)((u8 *)t0 + 0x54);
    } while (a2 < 0xD);
}

/**
 * @brief Draw the field status text selected by the CD-ROM error state.
 * @param arg0 Render context containing the text ordering table and primitive cursor.
 */
void func_800A92CC(ArgA *arg0)
{
    extern s32 func_800A88A0(void *arg0, void *arg1, void *arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6);
    s32 handle;
    void *ordering_table;
    Vec2s text_positions[2];

    handle = arg0->unk40B8;
    ordering_table = &arg0->unk3C;
    if (cdrom_get_error_status() == 2)
    {
        s32 low;
        s32 offset;
        u8 *base;

        low = D_800EC3D2.unk0;
        offset = (D_800EC3D2.unk1 << 8) + (s32)(base = (u8 *)&D_800EC3D2 - 0xE);
        handle = func_800A88A0(handle, ordering_table, (void *)(low + offset), 4, 0xA0, 0x64, 0x82);
    }
    else
    {
        s32 low;
        s32 offset;
        u8 *base;

        low = D_800EC3D4.unk0;
        offset = (D_800EC3D4.unk1 << 8) + (s32)(base = (u8 *)&D_800EC3D4 - 0x10);
        handle = func_800A88A0(handle, ordering_table, (void *)(low + offset), 4, 0xA0, 0x64, 0x82);
    }
    arg0->unk40B8 = handle;
}

/**
 * @brief Draw controller action hints and labels for the selectable field actors.
 * @param context Rendering context with the primitive cursor at offset 0x40B8.
 * @note Action labels use the first active button among eight mapped controls.
 * @note Selected actors receive a raised ordering-table entry and highlight palette.
 * @note Access widths and record strides are retained for partially known layouts.
 */
void func_800A939C(void *context)
{
    extern s32 func_800A88A0(s32, s32, void *, s32, s32, s32, s32);
    extern s32 g_pad_ctx;
    extern u8 D_800FDF58[];
    extern u8 D_800FE3A0[];
    extern u8 D_80105AE0[];
    extern u8 D_8011F3D2;
    s32 pad_offset;
    s32 custom_text_offset;
    FieldLabelPosition point;
    u8 *object_record;
    s32 text_style;
    s32 swapped_buttons;
    s32 buttons;
    s32 text_address;
    s32 text_base;
    s32 local_pad_offset;
    s32 local_text_offset;
    s32 screen_y;
    s32 camera_x_pixels;
    s32 label_ot;
    s32 label_half_width;
    s32 text_index;
    s32 actor_x;
    s32 camera_height;
    s32 number_ot;
    s32 camera_x;
    s32 camera_y;
    s32 text_ot;
    s32 left_glyph_ot;
    s32 right_glyph_ot;
    s32 button_index;
    s32 button_mask_or_y_offset;
    s32 index;
    s32 primitive;
    s32 action_offset;
    s32 actor_y;
    s32 actor_height;
    u16 raw_buttons;
    u8 *default_label_offset;
    s32 action_code;
    s32 actor_id;
    s32 action_slot;
    s32 secondary_action;
    s32 secondary_action_alt;
    s32 text_low_or_base;
    s32 pad_record;
    void *actor_slot;
    void *highlight_motion;
    void *actor_position;
    void *normal_motion;
    void *pad_sample;
    s32 text_high_or_offset;

    label_ot = (s32)context + 0x3C;
    index = 0;
    default_label_offset = D_800EC3E0;
    object_record = D_800FD818;
    action_offset = index;
    primitive = S32_AT(context, 0x40B8);
    pad_sample = (void *)0x801ED600;
    custom_text_offset = 0x5F0;
    pad_offset = 0;
    do
    {
        if ((U8_AT(object_record, 0x0) & 1) && ((u8)U8_AT(pad_sample, 0x0) < 0xFEU))
        {
            text_base = (s32)D_800EC3C4;
            button_mask_or_y_offset = 1;
            button_index = 0;
            local_pad_offset = pad_offset;
            raw_buttons = U16_AT(pad_sample, 0x2);
            local_text_offset = custom_text_offset;
            swapped_buttons = ((raw_buttons << 8) & 0xFF00) | (raw_buttons >> 8);
            buttons = (((u32)(swapped_buttons & 0x40) >> 1) | ((swapped_buttons & 0x20) * 2) |
                       ((u32)(swapped_buttons & 0x80) >> 3) | ((swapped_buttons & 0x10) * 8) |
                       (swapped_buttons & 0xFF0F));
        scan_buttons:
            if (buttons & button_mask_or_y_offset)
            {
                pad_record = g_pad_ctx + local_pad_offset;
                action_slot = U8_AT(pad_record + D_800EC2FC[button_index], 0x638);
                do
                {
                    switch (action_slot)
                    {
                    case 2:
                    case 3:
                        text_low_or_base = (s32)D_800EDBE4;
                        text_index = U16_AT(D_8010A028, action_offset + action_slot * 8) & 0x7FFF;
                        goto read_text_offset;

                    case 0:
                        if (((U8_AT(pad_record, 0x608) & 0x7F) == 2) &&
                            ((secondary_action = U8_AT(pad_record, 0x609),
                              (secondary_action == 5)) ||
                             (secondary_action == 8)))
                        {
                            text_high_or_offset = (default_label_offset[1] << 8) + text_base;
                            text_low_or_base = D_800EC3E0[0];
                        }
                        else
                        {
                            text_high_or_offset = (D_800EC3E6[1] << 8) + text_base;
                            text_low_or_base = D_800EC3E6[0];
                        }

                        break;
                    case 1:
                        if (((U8_AT(pad_record, 0x608) & 0x7F) == 2) &&
                            ((secondary_action_alt = U8_AT(pad_record, 0x609),
                              (secondary_action_alt == 5)) ||
                             (secondary_action_alt == 8)))
                        {
                            text_high_or_offset = (default_label_offset[1] << 8) + text_base;
                            text_low_or_base = D_800EC3E0[0];
                        }
                        else
                        {
                            text_high_or_offset = (D_800EC3E8[1] << 8) + text_base;
                            text_low_or_base = D_800EC3E8[0];
                        }
                        break;
                    default:
                        action_code = U8_AT(g_pad_ctx + local_pad_offset + action_slot, 0x608);
                        if (action_code == 0xFF)
                        {
                            text_high_or_offset = (default_label_offset[1] << 8) + text_base;
                            text_low_or_base = D_800EC3E0[0];
                        }
                        else if (action_code & 0x80)
                        {
                            text_low_or_base = g_pad_ctx + local_text_offset;
                            text_high_or_offset = ((action_code & 0xFF7F) << 6) + 0x150;
                        }
                        else
                        {
                            text_index =
                                (U16_AT(D_8010A038, action_offset + action_slot * 8) & 0x7FFF) +
                                (U8_AT(object_record, 0x1) * 0x18);
                            text_low_or_base = (s32)D_800ED064;
                        read_text_offset:
                            text_high_or_offset = U16_AT(text_low_or_base, text_index * 2);
                        }

                        break;
                    }
                    text_address = text_high_or_offset + text_low_or_base;
                } while (0);
                button_mask_or_y_offset = index << 5;
                point.x = 0x60;
                point.y = button_mask_or_y_offset + 0x3C;

                primitive = func_800A88A0(
                    (s32)func_80086184((SPRT *)primitive, (u32 *)label_ot, index, (u32 *)&point),
                    label_ot, (void *)text_address, 4, 0x80, button_mask_or_y_offset + 0x40, 0x80);
            }
            else
            {
                button_index += 1;
                button_mask_or_y_offset *= 2;
                if (button_index < 8)
                {
                    goto scan_buttons;
                }
            }
        }
        object_record += 0x268;
        action_offset += 0x190;
        pad_sample += 0xAE;
        index += 1;
        custom_text_offset += 0x250;
        pad_offset += 0x250;
    } while (index < 2);
    index = 0;
    if (D_801227D0 != 0)
    {
        do
        {
            actor_id = D_801226E0[index];
            camera_x = D_800F22A0;
            actor_slot = (void *)((actor_id * 0x23C) + (s32)D_80105AE0);
            actor_position = (void *)((actor_id * 0x54) + (s32)D_800FDF58);
            if (camera_x < 0)
            {
                camera_x += 0xFF;
            }
            actor_x = S32_AT(actor_position, 0x0);
            camera_x_pixels = camera_x >> 8;
            if (actor_x < 0)
            {
                actor_x += 0xFF;
            }
            camera_y = D_800F22A4;
            point.x = camera_x_pixels + ((actor_x >> 8) + 0xA0);
            if (camera_y < 0)
            {
                camera_y += 0xFF;
            }
            actor_y = S32_AT(actor_position, 0x4);
            if (actor_y < 0)
            {
                actor_y += 0xFF;
            }
            actor_height = S32_AT(actor_position, 0x8);
            screen_y = (camera_y >> 8) + ((actor_y >> 8) + 0x70);
            if (actor_height < 0)
            {
                actor_height += 0x1FF;
            }
            camera_height = D_800F22A8;
            if (camera_height < 0)
            {
                camera_height += 0x1FF;
            }
            point.y = (screen_y - (actor_height >> 9)) - (camera_height >> 9);
            label_half_width = func_800AE864((u8 *)S32_AT(actor_slot, 0x64)) * 6;
            if ((point.x + label_half_width) >= 0x141)
            {
                point.x = 0x140 - label_half_width;
            }
            if (((point.x - label_half_width) - 8) < 0)
            {
                point.x = label_half_width + 8;
            }
            if (point.y >= 0xB1)
            {
                point.y = 0xB0;
            }
            if (point.y < 0x32)
            {
                point.y = 0x32;
            }
            text_ot = label_ot;
            if (D_8011F3D2 == index)
            {
                text_ot = label_ot - 4;
            }
            text_style = 5;
            if (D_8011F3D2 == index)
            {
                text_style = 4;
            }
            primitive = func_800A88A0(primitive, text_ot, (void *)S32_AT(actor_slot, 0x64),
                                      text_style, (s32)point.x, (s32)point.y, 0x82);
            left_glyph_ot = label_ot;
            point.y = (u16)point.y - 8;
            if (D_8011F3D2 == index)
            {
                left_glyph_ot = label_ot - 4;
            }
            primitive = (s32)func_800AD524((u8 *)primitive, (s32 *)left_glyph_ot, 0xC,
                                           (s32 *)&point, D_8011F3D2 == index ? 0x81 : 0x82);
            right_glyph_ot = label_ot;
            point.x = (u16)point.x + 8;
            if (D_8011F3D2 == index)
            {
                right_glyph_ot = label_ot - 4;
            }
            primitive = (s32)func_800AD524((u8 *)primitive, (s32 *)right_glyph_ot, 0xD,
                                           (s32 *)&point, D_8011F3D2 == index ? 0x81 : 0x82);
            number_ot = label_ot;
            point.x = (u16)point.x + 8;
            if (D_8011F3D2 == index)
            {
                number_ot -= 4;
            }
            if (D_8011F3D2 == index)
            {
            }
            primitive = (s32)func_800AD208((s32 *)number_ot, (void *)primitive,
                                           (u8)U8_AT(actor_slot, 0x4C) >> 1, 2, (u16 *)&point,
                                           D_8011F3D2 == index ? 0x81 : 0x82);
            if ((D_8011F3D2 == index) && (S32_AT(actor_slot, 0x8) >= 0))
            {
                highlight_motion = (void *)((actor_id * 0x48) + (s32)D_800FE3A0);
                U8_AT(highlight_motion, 0x2E) = 0x80;
                U8_AT(highlight_motion, 0x33) = 0x80;
            }
            else
            {
                normal_motion = (void *)((actor_id * 0x48) + (s32)D_800FE3A0);
                U8_AT(normal_motion, 0x2E) = (u8)D_801228D0[index];
                U8_AT(normal_motion, 0x33) = (u8)D_801228E0[index];
            }
            index += 1;
        } while (index < (s32)D_801227D0);
    }
    S32_AT(context, 0x40B8) = primitive;
}

/**
 * @brief Merge duplicate entries in the field word/amount list and compact removed slots.
 */
void func_800A9A5C(void)
{
    s32 i;
    s32 j;
    u8 amount;
    s32 k;

    i = 0;
    if (D_80122908 > 0)
    {
        do
        {
            if (D_80122910[i] != 0)
            {
                for (j = 0; j < i; j++)
                {
                    amount = D_80122910[j];
                    if ((amount != 0) && (D_801227F8[i] == D_801227F8[j]))
                    {
                        k = i;
                        D_80122910[j] = amount + D_80122910[i];
                        while (k < (D_80122908 - 1))
                        {
                            D_80122910[k] = D_80122910[k + 1];
                            D_801227F8[k] = D_801227F8[k + 1];
                            k += 1;
                        }
                        i -= 1;
                        D_80122908 -= 1;
                        break;
                    }
                }
            }
            i += 1;
        } while (i < D_80122908);
    }
}

/**
 * @brief Update field actor selection or close the text session and restore saved part bytes.
 */
void func_800A9B88(void)
{
    extern void *g_pad_ctx;
    extern u8 D_800FE3A0[];
    extern u8 D_8011F3D2;
    extern s32 cdrom_get_error_status(void);
    s32 actor_index;
    u8 *part;
    if ((D_80122984 && !cdrom_get_error_status()) ||
        (!D_80122984 &&
         (g_pad_input == 0x800 || ((*(s32 *)((u8 *)g_pad_ctx + 0x858) & 0x80) &&
                                   *((u8 *)g_pad_ctx + 0x840) && g_pad_input_inject == 0x800))))
    {
        g_field_draw_count = 0;
        D_801227C8 = 0;
        field_restore_fade_target();
        akao_stop_sfx_by_id(0x7E);
        for (actor_index = 0; actor_index < D_801227D0; actor_index++)
        {
            part = D_800FE3A0 + D_801226E0[actor_index] * 0x48;
            part[0x2E] = D_801228D0[actor_index];
            part[0x33] = D_801228E0[actor_index];
        }
        func_800AA02C();
    }
    else
    {
        g_field_draw_count = 1;
        if (D_801227D0 >= 2U)
        {
            g_pad_input |= g_pad_input_inject;
            if (g_pad_input & 0x9000)
            {
                D_8011F3D2 = D_8011F3D2 ? D_8011F3D2 - 1 : D_801227D0 - 1;
                akao_stop_sfx_by_id(0x7D);
            }
            else if (g_pad_input & 0x6000)
            {
                D_8011F3D2 = D_8011F3D2 == D_801227D0 - 1 ? 0 : D_8011F3D2 + 1;
                akao_stop_sfx_by_id(0x7D);
            }
        }
    }
}

/**
 * @brief Build the status flags for one field resource record.
 * @param index Record index to inspect.
 * @return Encoded status flags, or zero when the record is inactive.
 */
s32 func_800A9D70(s32 index)
{
    FieldResourceRecord *base;
    FieldResourceRecord *record;
    u16 flags;
    s32 value;
    s16 state;
    s32 offset;

    base = FIELD_RESOURCE_RECORDS;
    record = &base[index];
    if (record->id >= 0xFE)
    {
        return 0;
    }

    flags = record->flags;
    value = (flags >> 8) | ((flags & 0xFF) << 8);
    value = ((u32)(value & 0x40) >> 1) | ((value & 0x20) << 1) | ((u32)(value & 0x80) >> 3) | ((value & 0x10) << 3) | (value & 0xFF0F);

    if (record->id != 0)
    {
        state = record->state_a;
        if (state < -1)
        {
            value |= 0x8000;
        }
        else if (state >= 2)
        {
            value |= 0x2000;
        }

        offset = index * sizeof(*base);
        state = ((FieldResourceRecord *)((u8 *)base + offset))->state_b;
        if (state < -1)
        {
            value |= 0x1000;
        }
        else if (state >= 2)
        {
            value |= 0x4000;
        }
    }

    return value;
}

/**
 * @brief Apply initial-delay and repeat timing to both field controller inputs.
 * @note Held input waits fifteen ticks initially, then repeats every three ticks.
 * @note The direction filter is enabled by D_8012269C.
 * @note 100% match with GCC 2.7.2 CDK: 109 instructions, 436 bytes.
 */
void func_800A9E78(void)
{
    s32 directions;
    s32 buttons;

    buttons = func_800A9D70(0);
    g_pad_input = 0;
    D_801229F8 = 0;
    if ((buttons == D_801227BC) || ((D_801227BC != 0) && (buttons & (D_801227BC | 0xB6F))))
    {
        directions = buttons & 0xF000;
        if (buttons != 0)
        {
            if ((directions != 0) && (D_8012269C != 0))
            {
                buttons = directions;
            }
            if (D_801227C0 == 0)
            {
                g_pad_input = buttons;
                D_801227C0 = 2;
            }
            else
            {
                D_801227C0 -= 1;
                g_pad_input = 0;
            }
        }
        else
        {
            goto clear_primary;
        }
    }
    else if (buttons == 0)
    {
    clear_primary:
        D_801227C0 = 0;
        D_801227BC = 0;
    }
    else
    {
        g_pad_input = buttons;
        D_801227BC = buttons;
        D_801227C0 = 0xF;
    }
    buttons = func_800A9D70(1);
    g_pad_input_inject = 0;
    if ((buttons == D_801227D8) || ((D_801227D8 != 0) && (buttons & (D_801227D8 | 0xB6F))))
    {
        directions = buttons & 0xF000;
        if (buttons != 0)
        {
            if ((directions != 0) && (D_8012269C != 0))
            {
                buttons = directions;
            }
            if (D_801227E4 == 0)
            {
                g_pad_input_inject = buttons;
                D_801227E4 = 2;
            }
            else
            {
                D_801227E4 -= 1;
                g_pad_input_inject = 0;
            }
        }
        else
        {
            goto clear_injected;
        }
    }
    else if (buttons == 0)
    {
    clear_injected:
        D_801227E4 = 0;
        D_801227D8 = 0;
    }
    else
    {
        g_pad_input_inject = buttons;
        D_801227D8 = buttons;
        D_801227E4 = 0xF;
    }
    D_801229F8 = g_pad_input;
}

/**
 * @brief Resets field menu input and its two control handles.
 *
 * Clears the live and injected pad state, acquires control handles zero and
 * one, initializes their masks to 0xF, and clears the pending input state.
 */
void func_800AA02C(void)
{
    g_pad_input = 0;
    D_801227BC = func_800A9D70(0);
    D_801227C0 = 0xF;
    g_pad_input_inject = 0;
    D_801227D8 = func_800A9D70(1);
    D_801227E4 = 0xF;
    D_801229F8 = 0;
}

/**
 * @brief Process field controller status, reset input, and guarded menu actions.
 * @param arg0 Value forwarded to the alternate input handler when it is active.
 * @note Reset input and CD errors clear controller feedback bytes.
 * @note The first and final actor scans share their counter to preserve codegen.
 */
void func_800AA098(s32 arg0)
{
    extern PadContext *g_pad_ctx;
    extern FieldInputActor D_800FDF58[];
    extern FieldInputSlot D_80105AE0[];
    extern s32 cdrom_get_error_status(void);
    FieldInputHardware *pad = (FieldInputHardware *)0x801ED600;
    u32 buttons;
    FieldInputActor *actor;
    FieldInputActor *actor2;
    FieldInputSlot *slot;
    s32 i;
    s32 count;
    s32 index;
    s32 mask;
    s32 absent;
    s32 injected;

    buttons = pad->buttons;
    buttons = (buttons >> 8) | ((buttons & 0xFF) << 8);
    buttons = ((buttons & 0x40) >> 1) | ((buttons & 0x20) << 1) |
              ((buttons & 0x80) >> 3) | ((buttons & 0x10) << 3) | (buttons & 0xFF0F);
    if (D_8012269C != 0)
    {
        return;
    }
    if (buttons == 0x90F)
    {
        g_pending_game_state = 4;
        pad->feedback91 = 0;
        pad->feedback92 = 0;
        pad->feedback13f = 0;
        pad->feedback140 = 0;
        akao_cmd_98_9a_9c_9e(0);
        func_800A3904(0, 0x3C, 0x7F);
        return;
    }
    if (D_8010AE78 != 0)
    {
        D_80122710 = 1;
        return;
    }
    if (D_80122710 != 0)
    {
        D_80122710 = 0;
        func_8008C7A8();
    }
    if (D_801227C8 != 0)
    {
        func_800AA858(arg0);
        return;
    }
    actor = D_800FDF58;
    i = 0;
    if (!(actor->flags & 0x1FF))
    {
        do
        {
            if (D_800FDF58[i].presence != 0xFF && D_800FDF58[i].state == 0x9A)
            {
                return;
            }
            i++;
            actor++;
        } while (i < 3);
        if (func_8006751C(0) == -1 && D_800F2298 == 0 && D_800F229C == 0 &&
            g_field_return_to_title_prompt_state == 0 && D_80122714 == 0 && func_800B0850() == 0)
        {
            if (D_801227B8[0] != 0xFF && pad->connected == 0xFF)
            {
                func_800AA570((void *)0x80170000, 0);
            }
            if (D_801227B9 != 0xFF && pad->connected2 == 0xFF)
            {
                func_800AA570((void *)0x80170000, 1);
            }
            if (cdrom_get_error_status() != 0)
            {
                D_80122984 = 1;
                pad->feedback91 = 0;
                pad->feedback92 = 0;
                pad->feedback13f = 0;
                pad->feedback140 = 0;
                func_800AA7A4();
                return;
            }
            D_801227B8[0] = pad->connected;
            D_801227B8[1] = pad->connected2;
            if (D_8012291C != 0)
            {
                if (func_8005B218() == 0)
                {
                    mask = 0x800;
                    if (g_pad_input == 0x800)
                    {
                        goto open_menu;
                    }
                    if ((g_pad_ctx->inject_flags & 0x80) && g_pad_ctx->inject_enable != 0)
                    {
                        injected = g_pad_input_inject;
                        goto check_injected;
                    }
                }
            }
            else
            {
                mask = 0x10;
                if (g_pad_input == 0x800 || g_pad_input == 0x10)
                {
                    goto open_menu;
                }
                if ((g_pad_ctx->inject_flags & 0x80) && g_pad_ctx->inject_enable != 0)
                {
                    injected = g_pad_input_inject;
                    if (injected == 0x800)
                    {
                        goto open_menu;
                    }
check_injected:
                    if (injected == mask)
                    {
open_menu:
                        func_800AA570((void *)0x80170000, 0);
                    }
                }
            }
            func_800AA498();
            if (g_pad_input & 0x80)
            {
                i = 0;
                if (D_80122980 != 0)
                {
                    index = i;
                    absent = 0xFF;
                    slot = D_80105AE0;
                    actor2 = D_800FDF58;
                    do
                    {
                        if (actor2->presence != absent && slot->value >= 0x14)
                        {
                            i++;
                        }
                        slot++;
                        index++;
                        actor2++;
                    } while (index < 13);
                    if (i < 5)
                    {
                        func_800AEE28();
                        return;
                    }
                    func_800A3938(0x78, 0x80);
                }
            }
        }
    }
}

/**
 * @brief Check two frame-phased sound queues and submit their guarded sound command.
 * @return Result is unspecified; no value is explicitly returned.
 * @see decomp.me (100%) TODO
 */
s32 func_800AA498(void)
{
    extern Struct_D80105AE0 D_80105AE0[];

    if (D_800FE754 != 0)
    {
        if (!(g_frame_counter & 0x1F))
        {
            if ((D_80105AE0[0].unk4 != 0) && ((u32) (D_80105AE0[0].unk4 * 4) < (u32) D_80105AE0[0].unk0))
            {
                func_800A3938(0xA6, 0x80);
            }
        }

        if (!((g_frame_counter + 0x10) & 0x1F) && !(D_800FDFC8 & 0x1FF) &&
            (D_80105AE0[1].unk4 != 0) && ((u32) (D_80105AE0[1].unk4 * 4) < (u32) D_80105AE0[1].unk0))
        {
            func_800A3938(0xA6, 0x80);
        }
    }
}
