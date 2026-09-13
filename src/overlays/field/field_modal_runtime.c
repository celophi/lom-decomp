#include "common.h"
#include "cd_resources.h"

/*
 * field_modal_runtime
 *
 * Consolidated translation unit for the FIELD overlay's modal runtime, vram
 * range 0x800AA570 .. 0x800AD030. It gathers the menu-overlay dispatch loop,
 * the field text session helpers, the party/action rebuild, the sub-overlay
 * launchers, the modal-state pump, the fade transitions and the modal
 * coordinate panels into one TU.
 *
 * D_8012269C is the field's modal-state code shared across the launchers, the
 * fade starters and the modal pump (1 shop, 2 gosub, 6/7/8 fade transitions).
 */

/* ------------------------------------------------------------------ */
/* Shared types (each name distinct; ArgA is identical in both source */
/* files, so a single copy is kept here).                             */
/* ------------------------------------------------------------------ */

/** @brief Block at 0x801ED600; only bytes 0 and 0xAE are copied here. */
typedef struct
{
    u8 unk0;
    u8 pad1[0xAE - 1];
    u8 unkAE;
} Struct_801ED600_2;

typedef struct
{
    u8 unk0;
    u8 unk1;
} D_801227B8_t;

/** @brief Party entry with overlapping flag and kind bytes, at stride 0x268. */
typedef struct
{
    union
    {
        u16 word;
        struct
        {
            u8 flags, kind;
        } bytes;
    } head;
    u8 unk2, unk3;
    u8 pad4[0x254];
    u8 unk258;
    u8 pad259[0xF];
} Party;
/** @brief Actor flags within the 0x54-byte field record. */
typedef struct
{
    u8 pad0[0x1C];
    s32 unk1C;
    u8 pad20[0x34];
} Actor;
/** @brief Runtime health fields within the 0x23C-byte actor state. */
typedef struct
{
    s32 unk0, unk4, unk8;
    u8 padC[0x230];
} State;
/** @brief Eight-byte action descriptor with byte and halfword flag access. */
typedef struct
{
    s16 unk0;
    union
    {
        u16 word;
        struct
        {
            u8 low, high;
        } byte;
    } bits;
    s16 unk4, unk6;
} Record;
/** @brief Selected item attributes at offsets 0x24 and 0x25. */
typedef struct
{
    u8 pad0[0x24];
    u8 unk24, unk25;
} Item;
/** @brief Accessed global settings and player fields within the saved context. */
typedef struct
{
    u8 pad0[0x28];
    u32 unk28;
    u8 pad2C[0x5F0 - 0x2C];
    u8 unk5F0;
    u8 pad5F1[0x17];
    u8 unk608, unk609, unk60A, unk60B, unk60C;
    u8 pad60D[7];
    u16 unk614;
    u8 pad616[0x2A];
    u8 unk640;
    u8 pad641[0x13];
    u32 unk654;
    u8 pad658[0x1E8];
    u8 unk840;
    u8 pad841[0x17];
    u32 unk858;
} Context;

typedef struct
{
    u8 unk0; /* 0x0 */
    u8 unk1; /* 0x1 */
} StructEC400;

/** @brief Caller-owned block whose draw handle word lives at 0x40B8. */
typedef struct
{
    u8 pad0[0x40B8];
    s32 unk40B8; /* 0x40B8 */
} ArgA;

/** @brief Party slot flags and action state at the original 0x268-byte stride. */
typedef struct
{
    union
    {
        u16 flags;
        struct
        {
            u16 active : 1;
            u16 selected : 1;
            u16 rest : 14;
        } bits;
    } status;
    u8 pad2;
    u8 unk3;
    u8 pad4[0x250];
    u16 unk254;
    u8 unk256;
    u8 pad257[0x11];
} Slot;
/** @brief Saved scene parameters and party-selection bit in the pad context. */
typedef struct
{
    u8 pad0[0x18];
    u32 unk18;
    s16 unk1C;
    s8 unk1E;
    u8 pad1F;
    s32 unk20;
    u16 unk24;
    u8 unk26;
    u8 unk27;
    u8 pad28[0x830];
    u8 unk858;
} PadContext;

/** @brief Two-byte little-endian offset entry of the D_800EC3C4 string table. */
typedef struct
{
    u8 unk0;
    u8 unk1;
} StructEC;

/* ------------------------------------------------------------------ */
/* Shared file-scope declarations from the already-merged member       */
/* groups (session ops, launchers, fade ops, coord panels).            */
/* ------------------------------------------------------------------ */

s32 func_800A9D70(s32);
void akao_cmd_98_9a_9c_9e(s32 arg0);
void field_text_reset_scratch(void);
void field_text_reset_windows(void);
void func_80063194(void);
void func_800A3904(s32 arg0, s32 arg1, s32 arg2);
void func_800A92CC(s32 arg0);
void func_800A939C(s32 arg0);
void func_800A9B88(void);

void field_set_fade_target_only(s16 red, s16 green, s16 blue, s16 duration);
void func_800A3938(s32 sound_id, s32 pan);
void func_800AE9E0(void);
s32 func_800A88A0(void *arg0, void *arg1, void *arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6);
s32 func_800A8DDC(u8 *arg0);
void func_800A8E28(u8 *dest, u8 *src);
s32 func_800AEAC0(s32 handle, void *arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
s32 func_800AF950(s32 handle, void *arg1, u8 *str, s32 arg3, s32 x, s32 y, s32 arg6, s32 arg7, s32 arg8, s32 arg9, s32 arg10, s32 arg11);

/* Forward prototypes for member panel/fade functions defined later in this
 * TU and called by func_800AB214. Real (ArgA *) signatures; the caller passes
 * a (void *) argument, which differs only by pointer type (codegen-identical). */
void func_800AB690(ArgA *arg0);
s32 func_800AB86C(ArgA *arg0);
s32 func_800AC768(ArgA *arg0);

extern D_801227B8_t D_801227B8;
extern s32 D_801227BC;
extern s32 D_801227C0;
extern s32 D_801227C8;
extern s32 D_801227D8;
extern s32 D_801227E4;
extern s32 D_8012291C;
extern s32 D_80122984;
extern s32 D_801229F8;
extern s32 g_pad_input;
extern s32 g_pad_input_inject;

extern s32 D_8011F41C;
extern s32 D_801227F0;
extern s32 D_801229AC;
extern s32 g_gosub_result_count;

/*
 * D_8012269C and g_pad_ctx are read with conflicting types across functions
 * (D_8012269C: s32 vs u32; g_pad_ctx: u8 * vs PadContext *). They are declared
 * at block scope inside each using function with that function's original type;
 * GCC 2.7.2 accepts the incompatible block-scope externs (warning only) and
 * emits identical code. A file-scope copy would turn those warnings into
 * hard "conflicting types" errors, so none is kept here.
 */

extern StructEC400 D_800EC400;
extern s32 D_8011F3D4;
extern s32 D_801227E0;
extern s32 D_80122990;
extern s32 D_80122B08;

extern StructEC D_800EC3E4;
extern StructEC D_800EC406;
extern StructEC D_800EC408;
extern StructEC D_800EC40A;
extern StructEC D_800EC40C;
extern s32 D_80122820;

/* ------------------------------------------------------------------ */
/* Coordinate-panel formatting macros (used by func_800AB86C /         */
/* func_800AC768).                                                     */
/* ------------------------------------------------------------------ */

/** @brief True for a DBCS lead byte (0x19-0x1F), which owns the following byte. */
#define IS_DBCS(c) ((u32)((c) - 0x19) < 7)

/** @brief Write a signed decimal into @p buf (minus-sign glyph from the string table). */
#define FORMAT_SIGNED(buf, val)                                       \
    {                                                                 \
    u8 *dst;                                                          \
    s32 value;                                                        \
    s32 wide;                                                         \
    u8 *minus;                                                        \
    s32 low;                                                          \
    s32 offset;                                                       \
    s32 div;                                                          \
    s32 started;                                                      \
    s32 digit;                                                        \
    dst = buf;                                                        \
    value = val;                                                      \
    wide = 0;                                                         \
    if (value < 0)                                                    \
    {                                                                 \
        value = -value;                                               \
        low = D_800EC3E4.unk0;                                        \
        offset = (D_800EC3E4.unk1 << 8) + (s32)((u8 *)&D_800EC3E4 - 0x20); \
        minus = (u8 *)(low + offset);                                 \
        func_800A8E28(dst, minus);                                    \
        dst += func_800A8DDC(minus);                                  \
    }                                                                 \
    div = 10000000;                                                   \
    started = 0;                                                      \
    do                                                                \
    {                                                                 \
        digit = value / div;                                          \
        if (digit != 0)                                               \
        {                                                             \
            started = 1;                                              \
        }                                                             \
        if (started || div == 1)                                      \
        {                                                             \
            if (wide)                                                 \
            {                                                         \
                *dst++ = 0x1D;                                        \
                *dst = digit;                                         \
            }                                                         \
            else                                                      \
            {                                                         \
                *dst = digit + '0';                                   \
            }                                                         \
            dst++;                                                    \
            value -= (value / div) * div;                             \
        }                                                             \
        div /= 10;                                                    \
    } while (div != 0);                                               \
    *dst = 0;                                                         \
    }

/** @brief Advance @p p to the terminator, accumulating the DBCS-aware byte length. */
#define STR_LEN_LOOP(p, len)                                          \
    while (*p != 0)                                                   \
    {                                                                 \
        if (IS_DBCS(*p))                                              \
        {                                                             \
            p += 2;                                                   \
            len += 2;                                                 \
        }                                                             \
        else                                                          \
        {                                                             \
            p += 1;                                                   \
            len += 1;                                                 \
        }                                                             \
    }

/** @brief Shared strcat body: append @p s_ after the last glyph of @p d_. */
#define STR_CAT_BODY(d_, s_, qsrc)                                    \
    {                                                                 \
        volatile u8 *p = d_;                                          \
        s32 len_d = 0;                                                \
        volatile u8 *q;                                               \
        s32 len_s;                                                    \
        s32 append;                                                   \
        s32 i;                                                        \
        STR_LEN_LOOP(p, len_d)                                        \
        q = qsrc;                                                     \
        len_s = 0;                                                    \
        append = len_d;                                               \
        STR_LEN_LOOP(q, len_s)                                        \
        for (i = 0; i < len_s; i++)                                   \
        {                                                             \
            d_[i + append] = s_[i];                                   \
        }                                                             \
        d_[i + append] = 0;                                           \
    }

/** @brief Append buffer @p s onto @p d. */
#define STR_CAT(d, s)                                                 \
    {                                                                 \
        u8 *d_ = (d);                                                 \
        u8 *s_ = (s);                                                 \
        STR_CAT_BODY(d_, s_, (s))                                     \
    }

/** @brief Append string-table entry @p sym onto @p d. */
#define STR_CAT_ENTRY(d, sym, off)                                    \
    {                                                                 \
        u8 *d_ = (d);                                                 \
        u8 *s_ = (u8 *)(((sym).unk1 << 8) + (sym).unk0);             \
        s_ += (s32)((u8 *)&(sym) - (off));                            \
        STR_CAT_BODY(d_, s_, s_)                                      \
    }

/** @brief Panel contents are drawn only while the fade state is 0 or 2. */
#define DRAW_FLAG (D_8011F3D4 == 0 || D_8011F3D4 == 2)

/* ================================================================== */
/* Functions, in ascending vram order.                                */
/* ================================================================== */

/**
 * @brief Run the menu overlay and dispatch its requested follow-up screens.
 * @param render_buffer_addr Address of the pair of MENU render buffers.
 * @note Fixed entry addresses are reused by the overlays loaded before each call.
 */
void func_800AA570(s32 render_buffer_addr)
{
    extern void cdrom_stream(s32, u32), cdrom_wait_queue_empty(void), field_text_reset_windows(void),
        func_80084240(void), func_800A3938(s32, s32), func_800AA7A4(void), func_800AA90C(s32),
        func_800C3BB0(void);
    extern void func_80140004(u32, void *, s32, s32, s32, void *, s32), func_80140024(u32, s32);
    extern s32 func_801405B0(s32);
    extern s32 D_80105880[];
    extern u8 D_801226B8[], D_801226F0[];
    extern s32 D_8011F424, D_801227D4, D_8012291C, D_80122984, D_801229F4, g_active_script,
        g_script_repeat_count;

    s32 var_a0;
    u8 *screen;
    s32 *repeat, *active, *name_id, *history;
    u8 *initial, *custom;
    s32 temp_v0;
    s32 var_a3;

    screen = (u8 *)0x801ED600;
    var_a0 = D_8012291C;
    D_80122984 = 0;
    screen[0x91] = 0;
    screen[0x92] = 0;
    screen[0x13F] = 0;
    screen[0x140] = 0;
    if (var_a0 != 0 || D_80105880[0] != 0 || D_80105880[7] != 0 || D_80105880[14] != 0)
    {
        func_800AA7A4();
        return;
    }
    goto start;
finished:
    func_800AA90C(1);
    goto cleanup;
start:
    func_800A3938(0x80, 0x80);
    func_80084240();

    g_active_script = 0;

    for (;;)
    {
        cdrom_stream(6, 0x80140000);
        cdrom_wait_queue_empty();
        temp_v0 = func_801405B0(render_buffer_addr);
        if (temp_v0 == 0)
        {
            goto finished;
        }
        {
            if (temp_v0 == 0xA)
            {
                func_800AA90C(1);
                cdrom_stream(9, 0x80140000);
                cdrom_wait_queue_empty();
                func_80140024(0x80150000, 1);
                func_800C3BB0();
                func_80084240();
                field_text_reset_windows();
                g_active_script = temp_v0;
                g_script_repeat_count = 0;
            }
            else
            {
                func_800AA90C(1);
                cdrom_stream(5, 0x80140000);
                cdrom_wait_queue_empty();
                if ((u32)(temp_v0 - 0xB) < 2U)
                {
                    func_80140004(0x80160000, D_801226F0, D_801227D4, 1, D_801229F4, D_801226B8, 0);
                }
                else
                {
                    func_80140004(0x80160000, D_801226F0, D_801227D4, temp_v0, D_801229F4,
                                  D_801226B8, 0);
                }
                field_text_reset_windows();
                g_script_repeat_count = D_801229F4;
                if ((u32)(temp_v0 - 0xB) < 2U)
                {
                    g_script_repeat_count = 0;
                    g_active_script = temp_v0;
                }
                else
                {
                    g_active_script = D_8011F424 + 1;
                }
            }
            continue;
        }
        break;
    }
cleanup:
    func_80084240();
}

/**
 * @brief Open the text session: set the active flag, start the CD-error fade, clear pad input, seed the two func_800A9D70 handles with 0xF counters, and run func_800A9198.
 * @see decomp.me (100%) TODO
 */
void func_800AA7A4(void)
{
    D_801227C8 = 1;
    field_set_cd_error_fade_target();
    g_pad_input = 0;
    D_801227BC = func_800A9D70(0);
    D_801227C0 = 0xF;
    g_pad_input_inject = 0;
    D_801227D8 = func_800A9D70(1);
    D_801227E4 = 0xF;
    D_801229F8 = 0;
    func_800A9198();
}

/**
 * @brief Clear the session flags and copy bytes 0 and 0xAE of the 0x801ED600 block into D_801227B8.
 */
void func_800AA824(void)
{
    Struct_801ED600_2 *ptr = (Struct_801ED600_2 *)0x801ED600;

    D_801227C8 = 0;
    D_8012291C = 0;
    D_801227B8.unk0 = ptr->unk0;
    D_801227B8.unk1 = ptr->unkAE;
}

/**
 * @brief Tear down or advance the active field text window on a gated event.
 *
 * When the text subsystem is active (@c D_801227C8), runs func_800A9B88 and, if
 * still active, resets the scratch buffer and dispatches the per-mode advance
 * (@c func_800A92CC / @c func_800A939C selected by @c D_80122984). If the pass
 * cleared the active flag it instead resets the windows, stops the sequence
 * (@c akao_cmd_98_9a_9c_9e), and kicks off the close fade.
 *
 * @param arg0 Mode parameter forwarded to the advance dispatch.
 * @see decomp.me (100%) TODO
 */
void func_800AA858(s32 arg0)
{
    if (D_801227C8 != 0)
    {
        func_800A9B88();
        if (D_801227C8 != 0)
        {
            field_text_reset_scratch();
            if (D_80122984 != 0)
            {
                func_800A92CC(arg0);
            }
            else
            {
                func_800A939C(arg0);
            }
            func_80063194();
            return;
        }
        field_text_reset_windows();
        akao_cmd_98_9a_9c_9e(2);
        func_800A3904(0, 0x3C, 0x7F);
    }
}

/**
 * @brief Apply saved player settings and rebuild the party action descriptors.
 * @param refresh_only Nonzero preserves the current party membership and health values.
 * @note Zero also reloads active-party data and palettes before refreshing actions.
 * @note Packed descriptor byte writes preserve the other flag byte.
 */
void func_800AA90C(s32 refresh_only)
{
    extern void akao_set_paused(s32);
    extern void cdrom_set_audio_volume(u8, s32);
    extern void func_8008C7A8(void);
    extern void func_80091438(s32);
    extern void func_800A3D44(s32, u8);
    extern void func_800A5174(s32, s32);
    extern void func_800A54D0(void);
    extern u8 D_800EB114[];
    extern u8 D_800EB24C[];
    extern Party D_800FD818[];
    extern Actor D_800FDF58[];
    extern State D_80105AE0[];
    extern Record D_8010A038[];
    extern s32 D_801158A0;
    extern u8 *g_pad_ctx;

    Party *initial_party;
    Party *party;
    s16 texture_value;
    u8 *controller;
    u8 *item_context;
    s32 context_offset;
    s32 record_offset;
    s32 absent;
    u8 selected;
    s32 first_one;
    s32 first_two;
    Record *record_base;
    u8 **context_pointer;
    u8 *pair_first;
    u8 *pair_second;
    Context *input_base;
    s32 temp_a1;
    s32 controller_or_player_test;
    s32 var_a0_2;
    s32 var_a1;
    s32 button_index;
    s32 var_a2;
    s32 var_a3;
    s32 actor_stride_words;
    s32 player_index;
    s32 context_stride;
    s32 record_stride;
    s32 state_stride;
    s32 item_record_offset;
    u16 temp_a0_2;
    u16 temp_a2;
    u32 temp_v1_4;
    u8 temp_a1_3;
    u8 temp_v0;
    u8 temp_v1_6;
    Actor *temp_a0;
    Record *temp_a0_3;
    Item *temp_a0_4;
    Record *temp_a0_5;
    Context *temp_a1_2;
    Record *temp_a3;
    Context *temp_t4;
    Record *temp_v0_2;
    Context *temp_v1;
    State *temp_v1_2;
    Context *temp_v1_3;
    Context *temp_v1_5;
    Context *item_cursor;

    akao_set_paused((((u32)((Context *)g_pad_ctx)->unk28 >> 1) & 1) ^ 1);
    cdrom_set_audio_volume(0x7F, ((u32)((Context *)g_pad_ctx)->unk28 >> 1) & 1);
    controller_or_player_test = (s32)0x801ED600;
    ((u8 *)controller_or_player_test)[0x90] = (s8)(*(volatile u32 *)&((Context *)g_pad_ctx)->unk28 & 1);
    if ((((Context *)g_pad_ctx)->unk858 & 0x80) && (((Context *)g_pad_ctx)->unk840 != 0))
    {
        ((u8 *)controller_or_player_test)[0x13E] = (s8)(*(volatile u32 *)&((Context *)g_pad_ctx)->unk28 & 1);
    }
    else
    {
        ((u8 *)controller_or_player_test)[0x13E] = 0;
    }
    player_index = 0;
    if (refresh_only == 0)
    {
        first_two = 2;
        first_one = 1;
        initial_party = D_800FD818;
        var_a3 = player_index;
    first_party:
    {
        if (((Context *)(g_pad_ctx + var_a3))->unk5F0 != 0)
        {
            initial_party->head.bytes.kind = 0xFF;
            temp_a2 = initial_party->head.word | 1;
            temp_v1 = (Context *)(g_pad_ctx + var_a3);
            initial_party->head.word = temp_a2;
            temp_a1 = temp_v1->unk608 & 0x7F;
            if (temp_a1 < 2)
            {
                initial_party->head.word = (u16)((temp_a2 & 0xFFFD) | ((temp_a1 & 1) * 2));
                initial_party->unk3 = 0;
            }
            else if (temp_a1 == first_two)
            {
                initial_party->head.word = (u16)(temp_a2 & 0xFFFD);
                selected = temp_v1->unk609;
                initial_party->unk3 = first_one;
                initial_party->unk2 = selected;
            }
            else if (temp_a1 == 3)
            {
                initial_party->head.word = (u16)(temp_a2 & 0xFFFD);
                selected = temp_v1->unk609;
                initial_party->unk3 = first_two;
                initial_party->unk2 = selected;
            }
            else if (temp_a1 == 4)
            {
                initial_party->head.word = (u16)(temp_a2 & 0xFFFD);
                selected = temp_v1->unk609;
                initial_party->unk3 = first_two;
                initial_party->unk2 = selected + 0x41;
            }
        }
        else
        {
            initial_party->unk3 = first_one;
            initial_party->unk2 = 0U;
            initial_party->head.word = (u16)(initial_party->head.word & 0xFFFE);
        }
        initial_party++;
        player_index += 1;
        var_a3 += 0x250;
    }
        if (player_index < 3)
        {
            goto first_party;
        }
        player_index = 0;
        func_800A54D0();
    }
    context_pointer = &g_pad_ctx;
    record_base = D_8010A038;
    party = D_800FD818;
    actor_stride_words = player_index;
    record_stride = player_index;
    context_stride = player_index;
    state_stride = player_index;
party_loop:
{
    if (party->head.bytes.flags & 1)
    {
        temp_a0 = (Actor *)((actor_stride_words + player_index) * 4 + (u8 *)D_800FDF58);
        temp_a1_2 = (Context *)((*context_pointer) + context_stride);
        temp_a0->unk1C = (s32)((temp_a0->unk1C & ~0x1FF) | (((u8)temp_a1_2->unk608 >> 7) ^ 1));
        temp_v0 = ((u32)temp_a1_2->unk654 >> 0xA) & 0x3F;
        controller_or_player_test = player_index < 2;
        if (party->head.bytes.kind != temp_v0)
        {
            party->head.bytes.kind = temp_v0;
            if (controller_or_player_test != 0)
            {
                func_80091438(player_index);
            }
            if (refresh_only == 0)
            {
                temp_v1_2 = (State *)(state_stride + (u8 *)D_80105AE0);
                temp_a0_2 = ((Context *)((*context_pointer) + context_stride))->unk614;
                temp_v1_2->unk8 = (s32)((temp_v1_2->unk8 & 0xFF000000) | temp_a0_2);
                temp_v1_2->unk0 = (s32)temp_a0_2;
                temp_v1_2->unk4 = (s32)temp_a0_2;
            }
            if ((D_801158A0 != 0) && (refresh_only != 0) && (controller_or_player_test != 0))
            {
                func_800A3D44(player_index, party->head.bytes.kind);
            }
        }
        party->unk258 = 0;
        if ((u32)(party->head.bytes.kind - 1) < 2U)
        {
            party->unk258 = 1;
            var_a1 = 1;
            var_a0_2 = context_stride + 0x40;
            do
            {
                temp_v1_3 = (Context *)((*context_pointer) + var_a0_2);
                if (temp_v1_3->unk640 != 0)
                {
                    temp_v1_4 = temp_v1_3->unk654;
                    if ((((temp_v1_4 >> 8) & 3) == 1) && !((temp_v1_4 >> 0xA) & 0x3F))
                    {
                        party->unk258 = 0;
                    }
                }
                var_a1 += 1;
                var_a0_2 += 0x40;
            } while (var_a1 < 4);
        }
        if (player_index == 2)
        {
            button_index = 0;
            if (D_800FD818[2].unk3 == player_index)
            {
                func_800A5174(2, D_800FD818[2].unk2 + 0xA9B);
                party++;
            }
            else
            {
                goto block_40;
            }
        }
        else
        {
            button_index = 0;
        block_40:
            pair_first = D_800EB114;
            pair_second = D_800EB114 + 1;
            input_base = (Context *)((*context_pointer) + context_stride);
            var_a2 = record_stride;
            do
            {
                temp_v1_5 = (Context *)((u8 *)input_base + button_index);
                temp_a0_3 = (Record *)(var_a2 + (u8 *)record_base);
                temp_a0_3->unk0 = (s16)temp_v1_5->unk60A;
                temp_a0_3->unk4 = (s16) * ((temp_v1_5->unk60A * 2) + pair_first);
                button_index += 1;
                temp_a0_3->unk6 = (s16) * ((temp_v1_5->unk60A * 2) + pair_second);
                var_a2 += 8;
            } while (button_index < 2);
            context_offset = context_stride;
            absent = 0xFF;
            record_offset = record_stride;
            item_context = *context_pointer;
            item_record_offset = 0x20;
            temp_t4 = (Context *)(item_context + context_offset);
            item_cursor = temp_t4;
        item_loop:
        {
            if (item_cursor->unk60C == absent)
            {
                temp_v0_2 = (Record *)(item_record_offset + record_offset + (u8 *)record_base);
                temp_v0_2->bits.word = (u16)(temp_v0_2->bits.word & 0xFBFF);
                temp_v0_2->bits.byte.low = absent;
                temp_v0_2->unk0 = 0;
                temp_v0_2->unk4 = 0;
                temp_v0_2->unk6 = 0;
                temp_v0_2->bits.word = (u16)(temp_v0_2->bits.word & 0xFCFF);
            }
            else
            {
                temp_v1_6 = item_cursor->unk60C;
                if (temp_v1_6 & 0x80)
                {
                    temp_a3 = (Record *)(item_record_offset + record_offset + (u8 *)record_base);
                    temp_a3->unk0 = 0;
                    temp_a3->bits.word = (u16)(temp_a3->bits.word | 0x400);
                    temp_a0_4 = (Item *)(item_context + (context_offset + 0x5F0) + (((temp_v1_6 & 0x7F) << 6) + 0x150));
                    temp_a3->bits.byte.low = (s8)((u8)temp_a0_4->unk25 >> 1);
                    temp_a3->unk4 = (s16) * (temp_a0_4->unk24 + D_800EB24C);
                    temp_a1_3 = temp_a0_4->unk25;
                    texture_value = temp_a1_3 + 0x8018 + (temp_a0_4->unk24 * 0xE);
                    if (!(temp_a1_3 & 1))
                    {
                        texture_value += 0x800;
                    }
                    temp_a3->unk6 = texture_value;
                    temp_a3->bits.word = (u16)(temp_a3->bits.word & 0xFCFF);
                }
                else
                {
                    temp_a0_5 = (Record *)(item_record_offset + record_offset + (u8 *)record_base);
                    temp_a0_5->bits.word = (u16)(temp_a0_5->bits.word & 0xFBFF);
                    temp_a0_5->bits.byte.low = absent;
                    temp_a0_5->unk4 = 2;
                    temp_a0_5->unk0 = (s16)(item_cursor->unk60C | 0x8000);
                    temp_a0_5->bits.word = (u16)(temp_a0_5->bits.word & 0xFCFF);
                    temp_a0_5->unk6 = (s16)(((item_cursor->unk60C + 0x88) | ~0x7FFF) + (party->head.bytes.kind * 0x18));
                }
            }
            item_cursor = (Context *)((u8 *)item_cursor + 1);
            item_record_offset += 8;
        }
            if ((s32)item_cursor < (s32)((u8 *)temp_t4 + 4))
            {
                goto item_loop;
            }
            goto block_51;
        }
    }
    else
    {
    block_51:
        party++;
    }
    actor_stride_words += 0x14;
    record_stride += 0x190;
    context_stride += 0x250;
    player_index += 1;
    state_stride += 0x23C;
}
    if (player_index < 3)
    {
        goto party_loop;
    }
    func_8008C7A8();
}

/**
 * @brief Load GNAME.BIN and run the name-entry screen.
 *
 * Argument order follows gname_run, which receives the 0x80160000 render
 * buffers first and allow_empty_cancel = 0 last.
 *
 * @param initial_name Name shown when the screen opens.
 * @param active_name Name buffer edited by the UI.
 * @param source_mode Random-name source selector.
 * @param history_index History-list entry selector.
 * @param custom_name Custom random-name source.
 * @see decomp.me (100%) TODO
 */
void field_run_name_entry(s32 initial_name, s32 active_name, s32 source_mode, s32 history_index, s32 custom_name)
{
    func_80084240();
    cdrom_stream(CD_RES_GNAME_BIN, (void *)0x80140000);
    cdrom_wait_queue_empty();
    func_80140004((void *)0x80160000, initial_name, active_name, source_mode, history_index, custom_name, 0);
    field_text_reset_windows();
    func_80084240();
}

/**
 * @brief Load ZUKAN.BIN and run its entry point at 0x80140E00.
 * @param arg0 Forwarded to the ZUKAN entry point; meaning not yet established.
 */
void field_run_zukan(s32 arg0)
{
    func_80084240();
    cdrom_stream(CD_RES_ZUKAN_BIN, (void *)0x80140000);
    cdrom_wait_queue_empty();
    func_80140E00((void *)0x80160000, arg0);
    func_80084240();
}

/**
 * @brief Load GOSUB.BIN and open a screen sequence, unless a sub-overlay is already active.
 *
 * Sets D_8012269C to 2 for the duration and clears the gosub result count.
 *
 * @param screen_sequence Terminated s32 array passed to gosub_open_screen_sequence.
 * @see decomp.me (100%) TODO
 */
void field_open_gosub_screen_sequence(void *screen_sequence)
{
    extern s32 D_8012269C;

    if (D_8012269C == 0)
    {
        D_801227F0 = 1;
        g_gosub_result_count = 0;
        func_80084240();
        cdrom_stream(CD_RES_GOSUB_BIN, (void *)0x80140000);
        cdrom_wait_queue_empty();
        D_8012269C = 2;
        D_8011F41C = 2;
        func_80140080((void *)0x80175000, screen_sequence);
    }
}

/**
 * @brief Load SHOP.BIN in mode 0 when the party holds anything, else start the func_800AB638 fade.
 *
 * Scans the equipment records at 0xCE0 and the item counts at 0x25E0 of the
 * pad context. With nothing held, hands 0 to func_800AB638 instead of
 * opening the shop.
 *
 * @param arg0 Forwarded as shop_init's last argument; meaning not yet established.
 * @see decomp.me (100%) TODO
 */
void field_open_shop_mode_0(s32 arg0)
{
    extern s32 D_8012269C;
    extern u8 *g_pad_ctx;
    s32 count;
    s32 i;

    if (D_8012269C == 0)
    {
        count = 0;
        for (i = 0; i < 0x64; i++)
        {
            if (g_pad_ctx[0xCE0] != 0)
            {
                count++;
                break;
            }
        }
        for (i = 0; i < 0x100; i++)
        {
            if ((g_pad_ctx + i)[0x25E0] != 0)
            {
                count++;
                break;
            }
        }
        if (count == 0)
        {
            func_800AB638(0);
        }
        else
        {
            func_80084240();
            cdrom_stream(CD_RES_SHOP_BIN, (void *)0x80140000);
            cdrom_wait_queue_empty();
            D_801229AC = 1;
            D_8012269C = 1;
            func_80140004((void *)0x80150000, 0, 0, 0, 0, arg0);
        }
    }
}

/**
 * @brief Load SHOP.BIN in mode 1 with four arguments, unless a sub-overlay is already active.
 * @param arg0 Forwarded as shop_init's second argument; meaning not yet established.
 * @param arg1 Forwarded as shop_init's third argument.
 * @param arg2 Forwarded as shop_init's fourth argument.
 * @param arg3 Forwarded as shop_init's fifth argument.
 * @see decomp.me (100%) TODO
 */
void field_open_shop_mode_1(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    extern s32 D_8012269C;

    if (D_8012269C == 0)
    {
        func_80084240();
        cdrom_stream(CD_RES_SHOP_BIN, (void *)0x80140000);
        cdrom_wait_queue_empty();
        D_801229AC = 1;
        D_8012269C = 1;
        func_80140004((void *)0x80150000, 1, arg0, arg1, arg2, arg3);
    }
}

/**
 * @brief Advance the active field modal state and restore field input on completion.
 * @param context_or_delay Opaque context forwarded to the active modal handler.
 * @note The argument carrier is reused for the two fifteen-frame input delays.
 * @note Party slot and scene data remain owned by their existing external tables.
 */
void func_800AB214(s32 context_or_delay)
{
    extern s32 DrawSync(s32);
    extern void field_restore_fade_target(void);
    extern void field_set_fade_target_only(s16, s16, s16, s16);
    extern void field_set_scene_parameters(s32, s32, u32, s32, s32, s32);
    extern void field_text_reset_scratch(void);
    extern void field_text_reset_windows(void);
    extern void func_80063194(void);
    extern void func_8006AB38(s32);
    extern s32 func_8006AD04(s32, s32, s32);
    extern void func_80084240(void);
    extern void func_800A39A8(s32, s32, s32, s32);
    extern void func_800A7384(void);
    extern s32 func_800A9D70(s32);
    extern void func_800AA90C(s32);
    extern s32 func_800B0888(void);
    extern s32 func_801400C4(s32);
    extern s32 func_801400D4(s32);
    extern s32 func_801401F0(s32);
    extern s32 func_801401F8(s32);
    extern s32 func_80140370(s32);
    extern Slot D_800FD818[];
    extern s32 D_8011F41C;
    extern u32 D_8012269C;
    extern s32 D_801227BC;
    extern s32 D_801227C0;
    extern s32 D_801227D8;
    extern s32 D_801227E4;
    extern s32 D_801227F0;
    extern s32 D_80122994;
    extern s32 D_8012299C;
    extern s32 D_801229AC;
    extern s32 D_801229F8;
    extern s32 g_gosub_result_count;
    extern s32 g_gosub_result_values;
    extern s16 g_music_track_index;
    extern PadContext *g_pad_ctx;
    extern s32 g_pad_input;
    extern s32 g_pad_input_inject;

    s32 slot_address;
    s32 result;
    s32 index_or_zero;
    u16 temp_v1;

    switch (D_8012269C)
    {
    case 1:
        if ((D_801229AC != 0) && (func_801400D4(context_or_delay) != 0))
        {
            D_801229AC = 0;
            D_8012269C = 0;
            func_80084240();
            return;
        }
    case 0:
        return;
    case 2:
        if (D_8011F41C != 0)
        {
            if (D_8011F41C >= 2)
            {
                if (func_801400C4(context_or_delay) != 0)
                {
                    DrawSync(0);
                    D_8011F41C = 1;
                    func_80084240();
                    return;
                }
            }
            else
            {
                DrawSync(0);
                field_text_reset_windows();
                D_801227F0 = 2;
                D_8012269C = 0;
                D_8011F41C = 0;
                return;
            }
        }
        break;
    case 3:
        if ((D_8012299C != 0) && (func_80140370(context_or_delay) != 0))
        {
            func_80084240();
            switch (D_8012299C)
            {
            case 2:
                func_800AA90C(0);
                index_or_zero = 0;
                slot_address = (s32)D_800FD818;
                do
                {
                    slot_address = (s32)&D_800FD818[index_or_zero];
                    *(u16 *)(slot_address + 0x254) = 0;
                    *(u8 *)(slot_address + 0x256) = 0xFF;
                    index_or_zero += 1;
                } while (index_or_zero < 3);
                g_music_track_index = (s16) * (volatile s32 *)&g_pad_ctx->unk20;
                field_set_scene_parameters(g_pad_ctx->unk24, g_pad_ctx->unk26, g_pad_ctx->unk18 & 0x01FFFFFF,
                                           g_pad_ctx->unk27, (s32)g_pad_ctx->unk1C, (s32)g_pad_ctx->unk1E);
                field_set_fade_target_only(0x100, 0x100, 0x100, 8);
                break;
            case 6:
            case 7:
                g_gosub_result_values = 6;
                /* fall through */
            default:
            case 3:
            case 4:
            case 5:
                break;
            }
            g_gosub_result_count = 1;
            D_801227F0 = 2;
            D_8012299C = 0;
            D_8012269C = 0;
            return;
        }
        break;
    case 4:
        if ((D_80122994 != 0) && (func_801401F0(context_or_delay) != 0))
        {
            func_80084240();
            D_80122994 = 0;
            D_8012269C = 0;
            return;
        }
        break;
    case 5:
        if (D_80122994 != 0)
        {
            result = func_801401F8(context_or_delay);
            switch (result)
            {
            case 1:
                D_800FD818[1].status.bits.active = 0;
                D_800FD818[1].status.bits.selected = g_pad_ctx->unk858 & 1;
                D_800FD818[1].unk3 = 0;
                func_8006AD04(-2, 0, 0);
                func_80084240();
                D_80122994 = 0;
                D_8012269C = 0;
                return;
            case 3:
                func_80084240();
                D_80122994 = 0;
                D_8012269C = 0;
                return;
            case 2:
                func_8006AB38(0);
                func_80084240();
                D_80122994 = 0;
                D_8012269C = 0;
                return;
            }
        }
        break;
    case 6:
        if (g_pad_input & 0x220)
        {
            field_text_reset_windows();
            index_or_zero = 0;
            D_8012269C = 0;
            g_pad_input = 0;
            goto block_42;
        }
        else
        {
            field_text_reset_scratch();
            func_800AB690((void *)context_or_delay);
            func_80063194();
            return;
        }
        break;
    case 7:
        if (func_800AB86C((void *)context_or_delay) != 0)
        {
            func_800A39A8(0, 0x80, 0, 3);
            goto block_41;
        }
        break;
    case 8:
        if ((func_800AC768((void *)context_or_delay) != 0) && (func_800B0888() == 0))
        {
            func_800A7384();
        block_41:
            field_text_reset_windows();
            index_or_zero = 0;
            D_8012269C = 0;
            g_pad_input = 0;
        block_42:
            D_801227BC = func_800A9D70(index_or_zero);
            context_or_delay = 0xF;
            D_801227C0 = context_or_delay;
            g_pad_input_inject = 0;
            D_801227D8 = func_800A9D70(1);
            D_801227E4 = context_or_delay;
            D_801229F8 = 0;
            field_restore_fade_target();
        }
        break;
    }
}

/**
 * @brief Enter modal state 6: fade toward a red tint, play sound 0xC7, and latch arg0.
 * @param arg0 Stored to D_801227E0; func_800AB690 skips its refresh while this is nonzero.
 */
void func_800AB638(s32 arg0)
{
    extern s32 D_8012269C;

    D_8012269C = 6;
    field_set_fade_target_only(0xC0, 0x80, 0x80, 8);
    func_800A3938(0xC7, 0x80);
    D_801227E0 = arg0;
}

/**
 * @brief Refresh a handle at arg0->unk40B8 unless the global gate is set.
 *
 * Builds the D_800EC400 address from its own first two bytes (low byte plus
 * a high byte shifted by 8) offset by -0x3C, then hands it to func_800A88A0.
 *
 * @param arg0 Block holding the handle at 0x40B8.
 */
void func_800AB690(ArgA *arg0)
{
    s32 handle;
    s32 low;
    s32 offset;
    u8 *base;

    handle = arg0->unk40B8;
    if (D_801227E0 == 0)
    {
        low = D_800EC400.unk0;
        offset = (D_800EC400.unk1 << 8) + (s32)(base = (u8 *)&D_800EC400 - 0x3C);
        handle = func_800A88A0(
            handle,
            arg0,
            (void *)(low + offset),
            4,
            0xA0,
            0x68,
            2);
    }
    arg0->unk40B8 = handle;
}

/**
 * @brief Enter modal state 7: fade toward white, play sound 0x125, and arm a 500-frame counter.
 */
void func_800AB710(void)
{
    extern s32 D_8012269C;

    D_8012269C = 7;
    field_set_fade_target_only(0xC0, 0xC0, 0xC0, 8);
    func_800AE9E0();
    func_800A3938(0x125, 0x80);
    D_8011F3D4 = 0;
    D_80122B08 = 0;
    D_80122990 = 0x1F4;
}

/**
 * @brief Enter modal state 8, arm the 500-frame slide-out counter, and bump
 *        the visit counter for whichever pad-context record the "hidden
 *        controller test" byte selects.
 *
 * D_800FDF79 (masked to 7 bits) selects record 1 when it equals 0x1D,
 * otherwise record 0; D_80122820 is left pointing at the chosen record.
 * The chosen record's own-position visit counter is always incremented; its
 * paired counter is incremented too while the record's mode byte (masked to
 * 7 bits) is below 2.
 */
void func_800AB774(void)
{
    extern void field_set_fade_target_only(s16 red, s16 green, s16 blue, s16 duration);
    extern void func_800A3938(s32 sound_id, s32 pan);
    extern void func_800AE9E0(void);
    extern s32 D_8011F3D4;
    extern s32 D_80122820;
    extern s32 D_80122990;
    extern s32 D_80122B08;
    extern s32 D_8012269C;
    extern u8 D_800FDF79;
    extern u8 *g_pad_ctx;

    D_8012269C = 8;
    field_set_fade_target_only(0xC0, 0xC0, 0xC0, 8);
    func_800AE9E0();
    func_800A3938(0x126, 0x80);
    D_8011F3D4 = 0;
    D_80122B08 = 0;
    D_80122990 = 0x1F4;

    if ((D_800FDF79 & 0x7F) == 0x1D)
    {
        D_80122820 = 1;
        *(u16 *)(g_pad_ctx + 0x636) = *(u16 *)(g_pad_ctx + 0x636) + 1;
        if ((u32)(g_pad_ctx[0x858] & 0x7F) < 2)
        {
            *(u16 *)(g_pad_ctx + 0x884) = *(u16 *)(g_pad_ctx + 0x884) + 1;
        }
    }
    else
    {
        D_80122820 = 0;
        *(u16 *)(g_pad_ctx + 0x634) = *(u16 *)(g_pad_ctx + 0x634) + 1;
        if ((u32)(g_pad_ctx[0x858] & 0x7F) < 2)
        {
            *(u16 *)(g_pad_ctx + 0x886) = *(u16 *)(g_pad_ctx + 0x886) + 1;
        }
    }
}

/**
 * @brief Step the modal result panel and queue its draw primitives.
 *
 * Advances the D_8011F3D4 fade state machine (halve the slide offset, hold for
 * 0x5A frames, then slide back out), then builds and queues the panel frame,
 * the position label, the formatted "x,y" coordinate string and, while the
 * pad-context mode byte at 0x858 is below 2, a second coordinate string.
 *
 * @param arg0 Block holding the draw handle at 0x40B8.
 * @return 1 once the panel has fully slid out (state 3), otherwise 0.
 */
s32 func_800AB86C(ArgA *arg0)
{
    extern u8 *g_pad_ctx;
    u8 buf[0x38];
    u8 buf2[0x38];
    s32 handle;
    void *ctx;

    handle = arg0->unk40B8;
    ctx = arg0;
    switch (D_8011F3D4)
    {
    case 0:
        D_80122990 /= 2;
        if (D_80122990 == 0)
        {
            D_8011F3D4 = 1;
            D_80122B08 = 0x5A;
        }
        break;
    case 1:
        D_80122B08--;
        if (D_80122B08 == 0)
        {
            func_800A3938(0x127, 0x80);
            D_8011F3D4 = 2;
            D_80122990 = -1;
        }
        break;
    case 2:
        D_80122990 *= 2;
        if (D_80122990 < -0x64)
        {
            D_8011F3D4 = 3;
        }
        break;
    case 3:
        return 1;
    }

    handle = func_800AEAC0(handle, ctx, 0, D_80122990 + 0x32, 0x22, 1);
    handle = func_800AF950(handle, ctx, g_pad_ctx + 0x5F0, 4, D_80122990 + 0x6C, 0x32, 0, 5, 0x180, 0x180, -4, DRAW_FLAG);

    FORMAT_SIGNED(buf, *(u16 *)(g_pad_ctx + 0x634));
    STR_CAT_ENTRY(buf, D_800EC408, 0x44);
    FORMAT_SIGNED(buf2, *(u16 *)(g_pad_ctx + 0x636));
    STR_CAT(buf, buf2);
    STR_CAT_ENTRY(buf, D_800EC40A, 0x46);

    handle = func_800AF950(handle, ctx, buf, 4, D_80122990 + 0x6C, 0x42, 0, 6, 0x180, 0x180, -4, DRAW_FLAG);
    {
        s32 low = D_800EC406.unk0;
        s32 offset = (D_800EC406.unk1 << 8) + (s32)((u8 *)&D_800EC406 - 0x42);
        handle = func_800AF950(handle, ctx, (u8 *)(low + offset), 4, 0xA0, 0x64, 2, 7, 0x180, 0x180, -4, DRAW_FLAG);
    }
    handle = func_800AEAC0(handle, ctx, 1, 0xDE - D_80122990, 0x86, 0);
    handle = func_800AF950(handle, ctx, g_pad_ctx + 0x840, 4, 0xD4 - D_80122990, 0x96, 1, 8, 0x180, 0x180, -4, DRAW_FLAG);

    if ((u32)(g_pad_ctx[0x858] & 0x7F) < 2)
    {
        FORMAT_SIGNED(buf, *(u16 *)(g_pad_ctx + 0x884));
        STR_CAT_ENTRY(buf, D_800EC408, 0x44);
        FORMAT_SIGNED(buf2, *(u16 *)(g_pad_ctx + 0x886));
        STR_CAT(buf, buf2);
        STR_CAT_ENTRY(buf, D_800EC40A, 0x46);
        handle = func_800AF950(handle, ctx, buf, 4, 0xD4 - D_80122990, 0xA6, 1, 9, 0x180, 0x180, -4, DRAW_FLAG);
    }

    arg0->unk40B8 = handle;
    return 0;
}

/**
 * @brief Step the selected-slot coordinate panel and queue its draw primitives.
 *
 * Same fade state machine and layout as func_800AB86C, but the pad-context
 * record is chosen by D_80122820 (stride 0x250) and the panel title comes from
 * string-table entry 0x48.
 *
 * @param arg0 Block holding the draw handle at 0x40B8.
 * @return 1 once the panel has fully slid out (state 3), otherwise 0.
 */
s32 func_800AC768(ArgA *arg0)
{
    extern u8 *g_pad_ctx;
    u8 buf[0x38];
    u8 buf2[0x38];
    s32 handle;
    void *ctx;
    u8 *rec;

    handle = arg0->unk40B8;
    ctx = arg0;
    switch (D_8011F3D4)
    {
    case 0:
        D_80122990 /= 2;
        if (D_80122990 == 0)
        {
            D_8011F3D4 = 1;
            D_80122B08 = 0x5A;
        }
        break;
    case 1:
        D_80122B08--;
        if (D_80122B08 == 0)
        {
            func_800A3938(0x127, 0x80);
            D_8011F3D4 = 2;
            D_80122990 = -1;
        }
        break;
    case 2:
        D_80122990 *= 2;
        if (D_80122990 < -0x64)
        {
            D_8011F3D4 = 3;
        }
        break;
    case 3:
        return 1;
    }

    {
        s32 low = D_800EC40C.unk0;
        s32 offset = (D_800EC40C.unk1 << 8) + (s32)((u8 *)&D_800EC40C - 0x48);
        handle = func_800AF950(handle, ctx, (u8 *)(low + offset), 4, 0xA0, 0x34, 2, 5, 0x200, 0x200, -4, DRAW_FLAG);
    }
    handle = func_800AEAC0(handle, ctx, D_80122820, D_80122990 + 0x32, 0x54, 1);
    handle = func_800AF950(handle, ctx, g_pad_ctx + (D_80122820 * 0x250 + 0x5F0), 4, D_80122990 + 0x6C, 0x64, 0, 6, 0x1C0, 0x1C0, -4, DRAW_FLAG);

    rec = g_pad_ctx + D_80122820 * 0x250;
    if ((u32)(rec[0x608] & 0x7F) < 2)
    {
        u8 *rec2;

        FORMAT_SIGNED(buf, *(u16 *)(rec + 0x634));
        STR_CAT_ENTRY(buf, D_800EC408, 0x44);
        rec2 = g_pad_ctx + D_80122820 * 0x250;
        FORMAT_SIGNED(buf2, *(u16 *)(rec2 + 0x636));
        STR_CAT(buf, buf2);
        STR_CAT_ENTRY(buf, D_800EC40A, 0x46);
        handle = func_800AF950(handle, ctx, buf, 4, D_80122990 + 0x8C, 0x84, 0, 7, 0x180, 0x180, -4, DRAW_FLAG);
    }

    arg0->unk40B8 = handle;
    return 0;
}
