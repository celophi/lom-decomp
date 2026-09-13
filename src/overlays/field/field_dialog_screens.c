#include "common.h"
#include "vector.h"
#include "sdk/libgpu.h"

/*
 * Consolidated FIELD dialog-screen translation unit.
 *
 * Merges the per-function sources for the vram range 0x800A6EEC .. 0x800A88A0
 * into one TU. Symbols whose type varies between functions (D_800FD818,
 * D_80105AE0, D_8010D038, g_pad_ctx, bcopy, func_800ADF84, func_800A88A0,
 * func_800A8A78, func_800A838C) are declared at BLOCK scope inside each user
 * with that function's original type; GCC 2.7.2 emits identical code and only
 * warns. Do not hoist any of them to file scope.
 */

/** @brief Opaque pad-context handle; some users view it as other record types. */
typedef struct PadContext PadContext;

/** @brief Packed actor addresses and restored state in a 0x23C-byte record. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 padC[0x48 - 12];
    s16 unk48;
    u8 pad4A[0x23C - 0x4A];
} Actor;

/** @brief Saved pad-context counter used when resuming the field. */
typedef struct
{
    u8 pad0[0x315C];
    s32 unk315C;
} Pad;

/** @brief Menu display record with packed flags, state, offsets, and update callback. */
typedef struct
{
    union
    {
        u32 word;
        struct
        {
            u16 low;
            s8 priority;
            u8 high;
        } fields;
    } flags;
    union
    {
        u32 word;
        struct
        {
            u16 low;
            s16 height;
        } fields;
    } state;
    s16 scroll_offset;
    s16 scroll_target;
    s16 scroll_tick;
    s16 pad_e;
    void (*update)(void);
} FieldMenuRecord;

/** @brief 0x268-byte entry whose first halfword carries per-slot flag bits. */
typedef struct {
    union { u16 h; struct { u8 unk0; u8 unk1; } b; } u0;
    u8 pad2[0x268 - 2];
} Entry268;

/** @brief 0x54-byte record exposing the halfword at 0x2A. */
typedef struct {
    u8 pad0[0x2A];
    s16 unk2A;
    u8 pad2C[0x54 - 0x2C];
} Rec54;

/** @brief 0x23C-byte state record exposing words at 0xC and 0x178. */
typedef struct {
    u8 pad0[0xC];
    s32 unkC;
    u8 pad10[0x178 - 0x10];
    s32 unk178;
    u8 pad17C[0x23C - 0x17C];
} State23C;

/** @brief Packed prompt flags accessed as both a word and individual fields. */
typedef union
{
    u32 word;
    struct
    {
        u32 low : 3;
        u32 type : 4;
        u32 priority : 9;
        u32 value : 8;
        u32 high : 8;
    } bits;
} FieldPromptFlags;

/** @brief Packed prompt state with an enable bit and an eight-bit value. */
typedef union
{
    u32 word;
    struct
    {
        u32 enabled : 1;
        u32 value : 8;
        u32 high : 23;
    } bits;
} FieldPromptState;

/** @brief Callback record returned by func_800ADF84. */
typedef struct
{
    FieldPromptFlags flags;
    FieldPromptState state;
    u8 pad8[8];
    void (*callback)(void);
} FieldADF84Rec;

/** @brief Partial slot record exposing the halfword cleared by func_800A74E8. */
typedef struct
{
    u8 pad0[0x260];
    s16 unk260;
    u8 pad262[0x268 - 0x262];
} RecFD818;

/** @brief Actor record prefix containing the active flag. */
typedef struct
{
    u8 active;
    u8 pad1[0x268 - 1];
} FieldActorEntry;

/** @brief 0x268-byte record exposing the two count bytes at 0x25C/0x25D. */
typedef struct
{
    u8 pad0[0x25C];
    u8 unk25C;
    u8 unk25D;
    u8 pad25E[0x268 - 0x25E];
} FieldEntry268;

/** @brief Active byte and fixed-point score accessed at offsets 0x5F0 and 0x610. */
typedef struct
{
    u8 pad0[0x5F0];
    u8 active;
    u8 pad5f1[0x1F];
    u32 score;
} FieldRankStats;

/** @brief Displayed numeric fields in a 0x268-byte player record. */
typedef struct
{
    u8 pad0[0x25A];
    u8 first;
    u8 second;
    u8 tail[0xC];
} FieldRankPlayer;

/** @brief Drawing position and three sorted actor indices sharing the local work area. */
typedef struct
{
    Vec2s position;
    s32 unused;
    s32 indices[3];
} FieldRankWork;

/** @brief Two-byte relative offset into the shared FIELD string table. */
typedef struct
{
    u8 unk0;
    u8 unk1;
} StructEC;

/** @brief Packed little-endian offset into the shared header text block. */
typedef struct PackedOffset
{
    unsigned char low;
    unsigned char high;
} PackedOffset;

/** @brief Text resource header with offsets to both item-name tables. */
typedef struct TextResource
{
    s32 unused;
    s32 normal;
    s32 special;
} TextResource;

/** @brief Eight-word animation lookup table copied to local storage. */
typedef struct
{
    s32 words[8];
} FieldQuadAnimationTable;

/* --- Shared (non-conflicting) extern data --- */
extern u8 D_8011F430[];
extern u8 D_800EC3D6[];
extern u8 D_800EC3C6[], D_800EC3DA[];
extern u8 D_80122910[];
extern unsigned char D_800EC3C4[];
extern void *D_801227F8[];
extern s32 D_801229A0[];
extern Rec54 D_800FDF58[];
extern StructEC D_800EC3D8;
extern PackedOffset D_800EC3CC;
extern PackedOffset D_800EC3CE;
extern FieldQuadAnimationTable D_800513E8, D_80051408, D_80051428, D_80051448;
extern s32 D_801227EC;
extern s32 D_801227C8;
extern s32 D_80122908;
extern u16 D_80122998;
extern u16 D_80122920;
extern s32 D_800F229C;
extern s32 D_801226D8;
extern s32 D_80122828;
extern s32 D_8011F420;
extern s32 g_pad_input;
extern s32 g_pending_game_state;
extern s32 g_field_return_to_title_prompt_state;
extern s32 g_field_return_to_title_prompt_delay;
extern s32 g_frame_counter;
extern s32 D_80115888, D_80115898, D_8011589C, D_801178B0, D_801178BC, D_801178C0;

/* --- Shared (non-conflicting) extern function prototypes --- */
extern void func_800ADEB0(void);
extern void func_800AA02C(void);
extern s32 func_800ADEEC(void);
extern void func_800ADF34(void);
extern void func_800A3938(s32 sound_id, s32 pan);
extern void func_800AA90C(s32);
extern void field_set_scene_parameters(s32, s32, s32, s32, s32, s32);
extern void field_begin_return_to_title_prompt_close(void);
extern void cdrom_queue_read(s32, s32);
extern void cdrom_wait_queue_empty(void);
extern void func_8006809C(void);
extern void func_800AE8A8(void);
extern void func_800AED20(void);
extern void func_800B661C(s32 arg0, FieldADF84Rec *arg1);
extern void akao_cmd_f1(void);
extern void func_800AE9E0(void);
extern s32 func_800B0888(void *arg0);
extern s32 func_80086184(s32, s32, s32, Vec2s *);

/* --- Forward prototypes for members defined later (real signatures) --- */
void func_800A7384(void);
void func_800A764C(void);
void func_800A7724(void);
void func_800A788C(void *ot, void *cursor, s32 x_offset, s32 y_offset);
s32 func_800A7B54(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
s32 func_800A7FB4(s32 *ot, s32 prim, s32 arg2, s32 arg3);
s32 func_800A8128(s32 ordering_table, s32 cursor, s32 scroll_x, s32 scroll_y, s32 viewport_height);
void *func_800A8524(s32 *ordering_table, POLY_FT4 *prim, s32 x, s32 y);
void *func_800A8660(s32 *ordering_table, POLY_FT4 *prim, s32 x, s32 y);

/**
 * @brief Snapshot the pad context into the field save buffer.
 */
void func_800A6EEC(void)
{
    extern void *bcopy(const void *, void *, int);
    extern PadContext *g_pad_ctx;

    bcopy(g_pad_ctx, D_8011F430, 0x3268);
}

/**
 * @brief Process the return-to-title choice or restore the saved field scene.
 */
void func_800A6F1C(void)
{
    extern void bcopy(void *, void *, s32);
    extern Pad *g_pad_ctx;
    extern u8 D_800FD818[];
    extern Actor D_80105AE0[];

    volatile Actor *actor;
    s32 prompt_state;
    s32 resume_count;
    s32 actor_index;
    s32 packed, source, copied;
    unsigned low_mask, high_mask;
    s16 sentinel;
    u8 *actor_flags;

    u8 *state = (u8 *)0x801ED600;

    if (!(D_80122828 & 7))
    {
        prompt_state = g_field_return_to_title_prompt_state - 1;
        g_field_return_to_title_prompt_state = prompt_state;
        if (prompt_state == 0)
        {
            if (D_801226D8 != 0)
            {
                g_field_return_to_title_prompt_state = 1;
                g_pending_game_state = 2;
                return;
            }
            bcopy(D_8011F430, g_pad_ctx, 0x3268);
            resume_count = g_pad_ctx->unk315C;
            if (resume_count != -1)
            {
                g_pad_ctx->unk315C = (s32)(resume_count + 1);
            }
            state[0x13F] = 0;
            state[0x91] = 0;
            state[0x140] = 0;
            state[0x92] = 0;
            func_800AA90C(0);
            field_set_scene_parameters(D_801178B0, D_801178BC, D_80115888, D_80115898, D_801178C0,
                                       D_8011589C);
            actor_index = 0;
            low_mask = 0xFFFFFF;
            high_mask = 0xFF000000;
            sentinel = 0xFF;
            actor_flags = D_800FD818;
            actor = D_80105AE0;
            do
            {
                /* Preserve both address reads before updating the packed fields. */
                packed = actor->unk8;
                source = *(volatile s32 *)&actor->unk0;
                copied = *(volatile s32 *)&actor->unk0;
                actor->unk8 = (packed & high_mask) | (source & low_mask);
                actor->unk4 = copied & low_mask;
                if (*actor_flags & 1)
                {
                    actor->unk48 = sentinel;
                }
                actor_flags += 0x268;
                actor_index += 1;
                actor++;
            } while (actor_index < 3);
        }
    }
    else if (func_800ADEEC() == 0)
    {
        if (g_pad_input & 0xA20)
        {
            func_800A3938(0x7E, 0x80);
            func_800ADF34();
            field_begin_return_to_title_prompt_close();
            return;
        }
        if (g_pad_input & 0xF100)
        {
            func_800A3938(0x7D, 0x80);
            D_801226D8 ^= 1;
        }
    }
}

/**
 * @brief Route a confirm/cancel pad press to the active field sub-dialog.
 *
 * When no modal is blocking (func_800ADEEC returns 0) and a confirm or cancel
 * button is held (@c g_pad_input & 0x220), dispatches on the current dialog
 * mode @c D_800F229C: mode 1 advances via func_800A7384 (gated by
 * @c D_801227EC); mode 3 backs out via func_800A764C when @c D_80122908 is set,
 * else falls through to the mode-2 handler func_800A7724.
 *
 * @see decomp.me (100%) TODO
 */
void func_800A710C(void)
{
    if ((func_800ADEEC() == 0) && (g_pad_input & 0x220))
    {
        switch (D_800F229C)
        {
        case 1:
            if (D_801227EC == 0)
            {
                func_800A7384();
                return;
            }
            break;
        case 3:
            if (D_80122908 != 0)
            {
                func_800A764C();
                return;
            }
            /* fallthrough */
        case 2:
            func_800A7724();
            break;
        }
    }
}

/**
 * @brief Load menu data and size its display record for both entry categories.
 */
void func_800A71CC(void)
{
    FieldMenuRecord *func_800ADF84(void);
    extern s32 D_8010D038;

    s16 raw_height;
    s32 height;
    u16 entry_count;
    s32 entry_limit;
    s32 padding;
    s32 index;
    s32 saw_special;
    s32 saw_normal;
    u16 *entry;
    u32 large_state;
    u32 state;
    u32 small_state;
    FieldMenuRecord *record;

    cdrom_queue_read(0x5DF, D_8010D038);
    cdrom_wait_queue_empty();
    func_800A3938(0xA1, 0x80);
    D_800F229C = 3;
    func_800ADF34();
    record = func_800ADF84();
    record->flags.word = (s32)((((record->flags.word & ~0x78) | 8) & 0xFFFF007F) | 0x1000);
    saw_normal = 0;
    saw_special = 0;
    index = 0;
    padding = 0;
    entry_count = D_80122998;
    if (entry_count != 0)
    {
        entry_limit = entry_count;
        entry = &D_80122920;
        do
        {
            if (*entry & 0x8000)
            {
                if (saw_special == 0)
                {
                    saw_special = 1;
                    padding += 0x10;
                }
            }
            else if (saw_normal == 0)
            {
                saw_normal = 1;
                padding += 0x10;
            }
            index += 1;
            entry++;
        } while (index < entry_limit);
    }

    record->update = &func_800A8128;
    record->flags.word = (s32)(record->flags.word & 0xFFFFFF);
    state = record->state.word;
    state |= 1;
    record->state.word = state;
    raw_height = (D_80122998 * 0x10) + padding;
    record->state.word = state | 0x200;
    record->state.fields.height = raw_height;
    height = raw_height;
    if (height < 0xA1)
    {
        small_state = (record->state.word & ~0x1FE) | ((height & 0xFF) * 2);
        record->state.word = small_state;
        record->flags.fields.priority = (s8)(0x70 - ((small_state >> 2) & 0x7F));
        return;
    }

    record->scroll_offset = 0;
    record->scroll_target = 0;
    record->scroll_tick = 0;
    large_state = (((record->state.word & ~0xC00) | 0x400) & ~0x1FE) | 0x140;
    record->state.word = large_state;
    record->flags.fields.priority = (s8)(0x70 - ((large_state >> 2) & 0x50));
}

/**
 * @brief Clear pending selection flags across the three actor slots.
 */
void func_800A7384(void)
{
    extern Entry268 D_800FD818[];
    extern State23C D_80105AE0[];

    s32 i;

    D_801227C8 = 0;
    i = 0;
    do {
        if ((D_800FD818[i].u0.b.unk0 & 1) && D_800FDF58[i].unk2A == 0x8E) {
            D_800FDF58[i].unk2A = 0;
        }
        i++;
    } while (i < 3);

    func_8006809C();

    i = 0;
    do {
        D_80105AE0[i].unkC = 0;
        D_80105AE0[i].unk178 &= ~0x20;
        i++;
    } while (i < 3);
    D_80122908 = 0;
}

/**
 * @brief Reset field sub-state and dispatch to one of three handlers.
 *
 * Runs the shared reset (func_800ADEB0, then latches @c D_801227EC to 4 and
 * calls func_800AA02C), then selects a handler by state: func_800A71CC when
 * @c D_80122998 is set, else func_800A764C when @c D_80122908 is set, else
 * func_800A7724. Finishes with func_800B0A08(0).
 *
 * @see decomp.me (100%) TODO
 */
void func_800A7434(void)
{
    extern void func_800B0A08(s32 arg0);

    func_800ADEB0();
    D_801227EC = 4;
    func_800AA02C();
    if (D_80122998 != 0)
    {
        func_800A71CC();
    }
    else if (D_80122908 != 0)
    {
        func_800A764C();
    }
    else
    {
        func_800A7724();
    }
    func_800B0A08(0);
}

/**
 * @brief Finish a field sub-dialog and return to the base field loop.
 */
void func_800A74B8(void)
{
    func_800AA02C();
    func_800B0A08(0);
    func_800AB774();
}

/**
 * @brief Configure the return-to-title prompt and reset its three slot values.
 * @note The mixed word and bitfield updates preserve the original store widths.
 */
void func_800A74E8(void)
{
    extern FieldADF84Rec *func_800ADF84(void);
    extern RecFD818 D_800FD818[];

    FieldADF84Rec *rec;
    u32 state;
    s32 i;

    func_800ADEB0();
    func_800A3938(0xB9, 0x80);

    rec = func_800ADF84();
    rec->callback = func_800AE8A8;
    rec->flags.bits.type = 1;
    rec->flags.bits.priority = 0x20;
    state = (rec->state.word | 1) & ~0x1FE;
    rec->flags.bits.value = 0x30;
    state |= 0x60;
    rec->state.word = state;
    rec->flags.word &= 0xFFFFFF;

    rec = func_800ADF84();
    rec->callback = func_800AED20;
    rec->flags.bits.type = 1;
    rec->flags.bits.priority = 0x50;
    rec->state.bits.enabled = 0;
    rec->state.bits.value = 0x22;
    rec->flags.bits.value = 0x80;

    rec->flags.word = (rec->flags.word & 0xFFFFFF) | 0xA0000000;

    /* Keep the shared -2 value in the target's argument register. */
    do
    {
        func_800B661C(-2, rec);
    } while (0);
    akao_cmd_f1();

    g_field_return_to_title_prompt_delay = 0x3C;
    g_field_return_to_title_prompt_state = 3;
    D_801226D8 = 0;
    func_800AE9E0();

    for (i = 2; i >= 0; i--)
    {
        D_800FD818[i].unk260 = 0;
    }
}

/**
 * @brief Configure a field prompt record for the current selection state.
 */
void func_800A764C(void)
{
    extern FieldADF84Rec *func_800ADF84(void);

    FieldADF84Rec *rec;
    u32 state;

    D_800F229C = 2;
    func_800A3938(0xB9, 0x80);
    func_800ADF34();

    rec = func_800ADF84();
    rec->flags.bits.type = 1;
    rec->flags.bits.priority = 0x40;

    state = rec->state.word & ~0x1FE;
    state |= (((D_80122908 << 4) + 0x10) & 0xFF) << 1;
    rec->flags.bits.value = 0x70 - ((state >> 2) & 0x78);

    rec->callback = func_800A7FB4;
    rec->state.word = state;
    rec->flags.word = (rec->flags.word & 0xFFFFFF) | 0xC0000000;
    rec->state.bits.enabled = 0;
}

/**
 * @brief Initialize two field processes and position the second from the active actor count.
 */
void func_800A7724(void)
{
    extern FieldADF84Rec *func_800ADF84(void);
    extern FieldActorEntry D_800FD818[];

    FieldADF84Rec *record;
    s32 actor_index;
    s32 active_count;

    func_800ADF34();
    D_800F229C = 1;
    func_800A3938(0xB9, 0x80);

    active_count = 0;
    record = func_800ADF84();
    actor_index = active_count;
    record->callback = func_800A788C;
    record->flags.bits.type = 1;
    record->flags.bits.priority = 0x20;
    record->flags.bits.value = 0x30;
    record->state.bits.enabled = 1;
    record->state.bits.value = 0x20;
    record->flags.word &= 0xFFFFFF;

    for (actor_index = 0; actor_index < 3; actor_index++)
    {
        if (D_800FD818[actor_index].active & 1)
        {
            active_count++;
        }
    }

    record = func_800ADF84();
    record->callback = func_800A7B54;
    record->flags.bits.type = 1;
    record->flags.bits.priority = 0x20;
    record->flags.bits.value = 0x58;
    record->state.bits.enabled = 1;
    record->state.bits.value = ((active_count * 28) + 16) & 0xFF;
    record->flags.word &= 0xFFFFFF;
}

/**
 * @brief Draw the field process/actor summary row.
 * @param ot Ordering-table address used by the drawing helpers.
 * @param cursor Current primitive-buffer cursor.
 * @param x_offset Horizontal drawing origin subtracted from each column.
 * @param y_offset Vertical drawing origin subtracted from each row.
 */
void func_800A788C(void *ot, void *cursor, s32 x_offset, s32 y_offset)
{
    extern u8 *g_pad_ctx;
    extern FieldEntry268 D_800FD818[];
    void *func_800A88A0(void *sprite_cursor, void *ot, u8 *text, s32 color, s32 x, s32 y, s32 flags);
    void *func_800A8A78(void *ot, void *cursor, s32 value, s32 color, s16 *position, s32 flags);

    s32 i;
    s32 total_x;
    s32 total_y;
    volatile u8 *entry;
    u8 *pad;
    volatile u8 *text_base;
    void *handle;
    void *first_cursor;
    s16 position[2];
    s16 y;

    if (D_801227EC != 0)
    {
        first_cursor = cursor;
        if ((x_offset | y_offset) != 0)
        {
            goto loop_setup;
        }
        D_801227EC--;
        if (D_801227EC != 0)
        {
            goto loop_setup;
        }
        if (func_800B0888(first_cursor) != 0)
        {
            D_801227EC = 1;
        }
    }

    first_cursor = cursor;
loop_setup:
    i = 0;
    total_x = i;
    total_y = i;
    entry = (u8 *)D_800FD818;
    pad = g_pad_ctx;
    do
    {
        if (pad[0x5F0] != 0)
        {
            total_x += entry[0x25C];
            total_y += entry[0x25D];
        }
        entry += 0x268;
        i++;
        pad += 0x250;
    } while (i < 3);

    text_base = D_800EC3D6 - 0x12;
    handle = func_800A88A0(first_cursor, ot,
        D_800EC3D6[0] + ((D_800EC3D6[1] << 8) + text_base),
        4, 0x10 - x_offset, -y_offset, 0);

    y = 0x10 - y_offset;
    position[1] = y;
    handle = func_800A8660(ot, handle, 0x18 - x_offset, y - 3);

    position[0] = 0x28 - x_offset;
    handle = func_800A8A78(ot, handle, total_x, 4, position, 0);
    handle = func_800A88A0(handle, ot,
        text_base[0x16] + ((text_base[0x17] << 8) + text_base),
        4, 0x40 - x_offset, position[1], 0);
    handle = func_800A8524(ot, handle, 0x50 - x_offset, position[1] + 2);

    position[0] = 0x60 - x_offset;
    handle = func_800A8A78(ot, handle, total_y, 4, position, 0);
    handle = func_800A88A0(handle, ot,
        text_base[0x1A] + ((text_base[0x1B] << 8) + text_base),
        4, 0x80 - x_offset, position[1], 0);

    position[0] = 0xD0 - x_offset;
    handle = func_800A8A78(ot, handle, *(s32 *)(g_pad_ctx + 0x2C) - D_8011F420, 4, position, 1);
    func_800A88A0(handle, ot,
        text_base[0] + ((text_base[1] << 8) + text_base),
        4, 0xF0 - x_offset, position[1], 1);
}

/**
 * @brief Draw up to three active players in descending adjusted-score order.
 * @param arg0 Ordering table address passed to the drawing helpers.
 * @param arg1 Initial primitive-buffer address.
 * @param arg2 Horizontal drawing origin subtracted from each column position.
 * @param arg3 Vertical drawing origin subtracted from each row position.
 * @return Primitive-buffer address after the final emitted element.
 * @note Equal adjusted scores retain player order during insertion sorting.
 */
s32 func_800A7B54(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    extern PadContext *g_pad_ctx;
    extern FieldRankPlayer D_800FD818[];
    extern s32 func_800A88A0(s32, s32, u8 *, s32, s32, s32, s32);
    extern s32 func_800A838C(s32, s32, s32, s32, s32);
    extern s32 func_800A8A78(s32, s32, s32, s32, Vec2s *, s32);

    FieldRankWork work;
    Vec2s *position;
    s32 count = 0;
    s32 i = count;
    s32 result;
    s32 *score;
    s32 *end;
    FieldRankStats *stats;
    FieldRankStats *base;
    s32 *scores;
    s32 pos;
    s32 j;
    s32 id;
    s32 *dst;
    s32 *src;
    s32 *current;
    s32 row;
    s16 y;
    u8 *text;

    result = func_800A88A0(arg1, arg0, D_800EC3C6[0] + ((D_800EC3C6[1] << 8) + (D_800EC3C6 - 2)), 4, 0x10 - arg2, -arg3, 0);
    scores = D_801229A0;
    score = scores;
    end = (s32 *)&work;
    base = (FieldRankStats *)g_pad_ctx;
    stats = base;
    do
    {
        stats = (FieldRankStats *)((u8 *)base + i * 0x250);
        if (stats->active != 0)
        {
            pos = 0;
            while (pos < count && (s32)((((FieldRankStats *)((u8 *)base + work.indices[pos] * 0x250))->score >> 8) - scores[work.indices[pos]]) >= (s32)((stats->score >> 8) - *score))
            {
                pos++;
            }
            if (pos == count)
            {
                end[2] = i;
            }
            else
            {
                j = count - 1;
                if (j >= pos)
                {
                    dst = &work.indices[j + 1];
                    src = (s32 *)&work + j;
                    do
                    {
                        *dst = src[2];
                        src--;
                        j--;
                        dst--;
                    } while (j >= pos);
                }
                work.indices[pos] = i;
            }
            end++;
            count++;
        }
        score++;
        i++;

    } while (i < 3);
    i = 0;
    if (count > 0)
    {
        text = D_800EC3DA - 0x16;
        row = i;
        position = &work.position;
        current = (s32 *)position;
        do
        {
            current = (s32 *)((s32)&work + i * 4);
            if (((FieldRankStats *)((u8 *)g_pad_ctx + current[2] * 0x250))->active != 0)
            {
                work.position.x = 0x18 - arg2;
                y = arg3 - 0x10;
                y = row - y;
                work.position.y = y;
                result = func_80086184(result, arg0, current[2], position);
                y += 8;
                work.position.y = y;
                result = func_800A838C(arg0, result, 0x38 - arg2, y - 8, 1);
                work.position.x = 0x48 - arg2;
                result = func_800A8A78(arg0, result, D_800FD818[current[2]].first, 4, position, 0);
                result = func_800A88A0(result, arg0, D_800EC3DA[0] + ((D_800EC3DA[1] << 8) + text), 4, 0x68 - arg2, work.position.y, 0);
                result = func_800A838C(arg0, result, 0x78 - arg2, work.position.y, 0);
                work.position.x = 0x88 - arg2;
                work.position.y = y;
                result = func_800A8A78(arg0, result, D_800FD818[current[2]].second, 4, position, 0);
                result = func_800A88A0(result, arg0, text[0x1A] + ((text[0x1B] << 8) + text), 4, 0xA8 - arg2, work.position.y, 0);
                work.position.x = 0xE0 - arg2;
                work.position.y = y;
                id = current[2];
                result = func_800A8A78(arg0, result, (((FieldRankStats *)((u8 *)g_pad_ctx + id * 0x250))->score >> 8) - D_801229A0[id], 4, position, 1);
            }
            row += 0x1C;
            i++;

        } while (i < count);
    }
    return result;
}

/**
 * @brief Draw a FIELD text list and the nonzero amount beside each entry.
 * @param ot Ordering table receiving the generated primitives.
 * @param prim Current primitive-chain handle.
 * @param arg2 Horizontal origin adjustment, also used by the amount row position.
 * @param arg3 Vertical origin adjustment for the list text.
 * @return Updated primitive-chain handle after drawing the list.
 */
s32 func_800A7FB4(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    extern s32 func_800A88A0(s32 prim, s32 *ot, void *tex, s32 arg3, s32 x, s32 y, s32 z);
    extern s32 func_800A8A78(s32 *ot, s32 prim, u32 val, s32 arg3, Vec2s *pos, s32 arg5);

    s32 i;
    Vec2s pos;
    u8 pad[0x84];
    s32 row;
    s32 low;
    s32 offset;
    u8 *tex;

    low = D_800EC3D8.unk0;
    offset = (D_800EC3D8.unk1 << 8) + (s32)((u8 *)&D_800EC3D8 - 0x14);
    tex = (u8 *)(low + offset);
    prim = func_800A88A0(prim, ot, tex, 4, 0x20 - arg2, -arg3, 0);
    i = 0;
    if (D_80122908 > 0)
    {
        do
        {
            row = i * 0x10;
            prim = func_800A88A0(prim, ot, D_801227F8[i], 4, 0x10 - arg2, (row + 0x10) - arg3, 0);
            pos.x = 0xB0 - arg2;
            pos.y = (row + 0x10) - arg2;
            if (D_80122910[i] != 0)
            {
                prim = func_800A8A78(ot, prim, D_80122910[i], 4, &pos, 1);
            }
            i += 1;
        } while (i < D_80122908);
    }
    return prim;
}

/**
 * @brief Draw the visible rows of the two-group item list, inserting each group header once.
 *
 * @param ordering_table Ordering-table address used by the text renderer.
 * @param cursor Current primitive buffer cursor.
 * @param scroll_x Horizontal scroll offset.
 * @param scroll_y Vertical scroll offset.
 * @param viewport_height Bottom clipping boundary.
 * @return Primitive buffer cursor after drawing the visible text.
 * @note Partial assembly match; probe evidence is retained in working/func_800A8128.
 */
s32 func_800A8128(s32 ordering_table, s32 cursor, s32 scroll_x, s32 scroll_y, s32 viewport_height)
{
    s32 func_800A88A0(s32, s32, void *, s32, s32, s32, s32); /* extern */
    extern TextResource *D_8010D038;

    s32 header_x;
    s32 item_x;
    void *special_names;
    void *normal_names;
    s32 normal_header_y;
    s32 special_header_y;
    s32 draw_cursor;
    s32 next_cursor;
    s32 special_header_drawn;
    s32 row;
    s32 index;
    s32 normal_header_drawn;
    s32 special_row_y;
    s32 normal_row_y;
    s32 item_y;
    u16 *entry;
    u16 name_index;
    void *name_table;

    next_cursor = cursor;
    special_header_drawn = 0;
    row = 0;
    normal_header_drawn = 0;
    index = 0;
    normal_names = (unsigned char *)D_8010D038 + D_8010D038->normal;
    special_names = (unsigned char *)D_8010D038 + D_8010D038->special;
    if (D_80122998 != 0)
    {
        header_x = 0x20 - scroll_x;
        item_x = 0x80 - scroll_x;
        entry = &D_80122920;
        do
        {
            if (*entry & 0x8000)
            {
                special_row_y = row * 0x10;
                if (special_header_drawn == 0)
                {
                    special_header_y = special_row_y - scroll_y;
                    if ((special_header_y >= -0xF) && (special_header_y < viewport_height))
                    {
                        next_cursor =
                            func_800A88A0(next_cursor, ordering_table,
                                          D_800EC3CE.low + ((D_800EC3CE.high << 8) +
                                                            (unsigned char *)&D_800EC3CE - 10),
                                          4, header_x, special_header_y, 0);
                    }
                    special_header_drawn = 1;
                    row += 1;
                    special_row_y = row * 0x10;
                }
                item_y = special_row_y - scroll_y;
                if (item_y >= -0xF)
                {
                    draw_cursor = next_cursor;
                    if (item_y < viewport_height)
                    {
                        name_table = special_names;
                        name_index = *entry & 0x7FFF;
                        next_cursor = func_800A88A0(draw_cursor, ordering_table,
                                                    (unsigned char *)name_table +
                                                        ((u16 *)name_table)[name_index],
                                                    4, item_x, item_y, 2);
                    }
                }
            }
            else
            {
                normal_row_y = row * 0x10;
                if (normal_header_drawn == 0)
                {
                    normal_header_y = normal_row_y - scroll_y;
                    if ((normal_header_y >= -0xF) && (normal_header_y < viewport_height))
                    {
                        next_cursor =
                            func_800A88A0(next_cursor, ordering_table,
                                          D_800EC3CC.low + ((D_800EC3CC.high << 8) +
                                                            (unsigned char *)&D_800EC3CC - 8),
                                          4, header_x, normal_header_y, 0);
                    }
                    normal_header_drawn = 1;
                    row += 1;
                    normal_row_y = row * 0x10;
                }
                item_y = normal_row_y - scroll_y;
                if (item_y >= -0xF)
                {
                    draw_cursor = next_cursor;
                    if (item_y < viewport_height)
                    {
                        name_index = *entry;
                        name_table = normal_names;
                        next_cursor = func_800A88A0(draw_cursor, ordering_table,
                                                    (unsigned char *)name_table +
                                                        ((u16 *)name_table)[name_index],
                                                    4, item_x, item_y, 2);
                    }
                }
            }
            row += 1;
            entry += 1;
        } while (++index < (s32)D_80122998);
    }
    return next_cursor;
}

/**
 * @brief Build and link an animated textured quad primitive.
 * @param ordering_table Ordering-table tag to link the primitive into.
 * @param prim Primitive buffer slot to populate.
 * @param x Left screen coordinate.
 * @param y Top screen coordinate.
 * @param wide Selects the larger geometry and texture region when nonzero.
 * @return Pointer just past the emitted primitive.
 */
POLY_FT4* func_800A838C(u32* ordering_table, POLY_FT4* prim, s16 x, s16 y, s32 wide)
{
    s32 phase_table[8] = {0, 1, 2, 3, 2, 1, 0, 0};
    s32 phase;

    phase = phase_table[((g_frame_counter >> 2) + 1) & 7];

    *(u32*)&prim->r0 = 0x808080;
    setlen(prim, 9);
    setcode(prim, 0x2C);
    prim->x2 = x;
    prim->x0 = x;
    prim->y1 = y;
    prim->y0 = y;

    if (wide != 0)
    {
        prim->x3 = x + 12;
        prim->x1 = x + 12;
        prim->y3 = y + 24;
        prim->y2 = y + 24;
        prim->u2 = phase * 12 + 0x20;
        prim->u0 = phase * 12 + 0x20;
        prim->u3 = phase * 12 + 0x2C;
        prim->u1 = phase * 12 + 0x2C;
        prim->v1 = 0x60;
        prim->v0 = 0x60;
        prim->v3 = 0x78;
        prim->v2 = 0x78;
    }
    else
    {
        prim->x3 = x + 8;
        prim->x1 = x + 8;
        prim->y3 = y + 16;
        prim->y2 = y + 16;
        prim->u2 = phase * 8 - 0x48;
        prim->u0 = phase * 8 - 0x48;
        prim->u3 = phase * 8 - 0x40;
        prim->u1 = phase * 8 - 0x40;
        prim->v1 = 0x40;
        prim->v0 = 0x40;
        prim->v3 = 0x50;
        prim->v2 = 0x50;
    }

    prim->clut = 0x7A87;
    prim->tpage = 0x26;
    addPrim(ordering_table, prim);
    return prim + 1;
}

/**
 * @brief Build and link an animated eight-pixel textured quad.
 * @param ordering_table Ordering-table tag receiving the primitive.
 * @param prim Primitive buffer to populate.
 * @param x Left screen coordinate.
 * @param y Top screen coordinate.
 * @return Buffer address immediately after the emitted primitive.
 */
void *func_800A8524(s32 *ordering_table, POLY_FT4 *prim, s32 x, s32 y)
{
    FieldQuadAnimationTable table;
    s32 phase_index;
    s32 frame;
    u32 color;
    s32 phase;
    u16 left_u;
    u16 right_u;

    table = D_800513E8;
    color = 0x808080;
    frame = g_frame_counter;
    phase_index = ((frame >> 2) + 2) & 7;
    phase = table.words[phase_index];

    *(u32 *)&prim->r0 = color;
    setlen(prim, 9);
    setcode(prim, 0x2C);
    prim->x2 = x;
    prim->x0 = x;
    prim->x3 = x + 8;
    prim->x1 = x + 8;
    prim->y1 = y;
    prim->y0 = y;
    prim->y3 = y + 8;
    prim->y2 = y + 8;
    prim->v1 = 0x78;
    prim->v0 = 0x78;
    prim->v3 = 0x80;
    prim->v2 = 0x80;
    prim->clut = 0x7A87;
    prim->tpage = 0x26;

    phase *= 8;
    left_u = phase + 0x20;
    right_u = phase + 0x28;
    prim->u3 = right_u;
    prim->u1 = right_u;
    prim->u2 = left_u;
    prim->u0 = left_u;

    prim->tag = (prim->tag & 0xFF000000) | (*ordering_table & 0xFFFFFF);
    *ordering_table = (*ordering_table & 0xFF000000) | ((s32)prim & 0xFFFFFF);
    return (u8 *)prim + 0x28;
}

/**
 * @brief Append an animated textured quad to an ordering table.
 * @param ordering_table Ordering-table entry receiving the primitive.
 * @param prim Writable primitive buffer.
 * @param x Horizontal origin.
 * @param y Vertical origin.
 * @return Buffer address immediately after the emitted primitive.
 */
void *func_800A8660(s32 *ordering_table, POLY_FT4 *prim, s32 x, s32 y)
{
    s32 frame;
    u32 color;
    FieldQuadAnimationTable tables[4];
    u16 left_x;
    u16 right_x;
    u16 width;
    u8 texture_u;
    s32 phase_index;

    tables[0] = D_800513E8;
    tables[1] = D_80051408;
    tables[2] = D_80051428;
    tables[3] = D_80051448;
    color = 0x808080;
    frame = g_frame_counter;
    *(u32 *)&prim->r0 = color;
    setlen(prim, 9);
    setcode(prim, 0x2C);
    phase_index = ((frame >> 2) + 3) & 7;
    left_x = *(u16 *)&tables[1].words[phase_index] + x;
    prim->x2 = left_x;
    prim->x0 = left_x;
    width = *(u16 *)&tables[3].words[phase_index];
    prim->y1 = y;
    prim->y0 = y;
    y += 0x10;
    prim->y3 = y;
    prim->y2 = y;
    right_x = left_x + width;
    prim->x3 = right_x;
    prim->x1 = right_x;
    texture_u = *(u8 *)&tables[2].words[phase_index];
    prim->u2 = texture_u;
    prim->u0 = texture_u;
    texture_u += *(u8 *)&tables[3].words[phase_index];
    prim->u3 = texture_u;
    prim->u1 = texture_u;
    prim->v1 = 0x80;
    prim->v0 = 0x80;
    prim->v3 = 0x90;
    prim->v2 = 0x90;
    prim->clut = 0x7A87;
    prim->tpage = 0x26;
    addPrim(ordering_table, prim);
    return prim + 1;
}

/**
 * @brief Thin wrapper delegating to the field menu-window handler.
 */
void func_800A8880(void)
{
    func_800AE008();
}
