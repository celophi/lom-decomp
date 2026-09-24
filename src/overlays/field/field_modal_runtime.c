/** @file field_modal_runtime.c
 * @brief Immediate text, input/text session and modal runtime for FIELD.
 *
 * One translation unit covering 0x800A88A0 .. 0x800AD030 (formerly
 * field_immediate_text.c, field_input_text_session.c and field_modal_runtime.c).
 * Globals whose reconstructed type differs between the former files are
 * declared at block scope in the earlier users.
 */

#include "field_text.h"
#include "common.h"
#include "gpu_packet.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "field_modal_runtime.h"
#include "field_effect_render_state.h"
#include "cdrom.h"
#include "saved_game.h"
#include "vector.h"
#include "sdk/libetc.h"
#include "field_scene_transition.h"
#include "cd_resources.h"
#include "controller_internal.h"

/* ---- Immediate text (0x800A88A0 .. 0x800A8CFC) ---- */

/**
 * @brief Two-byte descriptor used to locate the "minus" glyph string.
 * @note Local to this TU; a distinct name avoids clashing with other files.
 */
typedef struct
{
    u8 unk0;
    u8 unk1;
} StructEC;


SPRT* func_800AD658(s32* ot, SPRT* sprite_cursor, s32 count);
s32 field_name_byte_length(u8 *arg0);
void field_copy_name(u8 *dest, u8 *src);

/**
 * @brief Build and enqueue text-glyph sprites for a line of immediate text.
 * @param sprite_cursor Sprite scratch buffer to fill and enqueue.
 * @param ot Ordering table to add primitives to.
 * @param text Glyph string to render.
 * @param text_color Text color/style selector passed to the glyph builder.
 * @param x Starting x coordinate (adjusted for center/right alignment).
 * @param y Starting y coordinate.
 * @param flags Alignment bits (0x7F) plus 0x80 post-processing flag.
 * @return Pointer just past the trailing DR_TPAGE primitive.
 * @see decomp.me (100%) TODO
 */
void* func_800A88A0(SPRT* sprite_cursor, s32* ot, u8* text, s32 text_color, s32 x, s32 y, s32 flags)
{
    s32 n, count, i, acc;
    SPRT* sprite;
    DR_TPAGE* tpage;

    if (*text == 0)
    {
        return sprite_cursor;
    }

    n = field_text_build_sprites(sprite_cursor, text, text_color);
    count = n;

    if ((flags & 0x7F) != 1)
    {
        if ((flags & 0x7F) == 2)
        {
            sprite = sprite_cursor;
            for (i = 0; i < count; i++)
            {
                x -= sprite[i].w >> 1;
            }
        }
    }
    else
    {
        sprite = sprite_cursor;
        for (i = 0; i < count; i++)
        {
            x -= sprite[i].w;
        }
    }

    acc = 0;

    if (count != 0)
    {
        do
        {
            sprite = sprite_cursor;
            SET_BGR0_PACKED(sprite, GPU_TINT_NEUTRAL);
            setSprt(sprite);
            setXY0(sprite, x + acc, y);
            acc += sprite->w;

            addPrim(ot, sprite);
            sprite_cursor++;
            count--;
        } while (count != 0);
    }

    if (flags & 0x80)
    {
        sprite_cursor = func_800AD658(ot, sprite_cursor, n);
    }

    tpage = (DR_TPAGE*)sprite_cursor;
    setDrawTPage(tpage, 0, 0, 0x1F);
    addPrim(ot, tpage);

    return tpage + 1;
}

/**
 * @brief Format a value as a narrow decimal string, then render it as text.
 * @param arg0 Ordering table for func_800A88A0.
 * @param arg1 Sprite scratch buffer for func_800A88A0.
 * @param arg2 Signed value to format into the glyph buffer.
 * @param arg3 Text color/style selector.
 * @param arg4 Pointer to a two-element x/y coordinate pair.
 * @param arg5 Alignment/post-processing flags.
 */
void func_800A8A78(void *arg0, void *arg1, s32 arg2, s32 arg3, s16 *arg4, s32 arg5)
{
    extern void func_800A8B90(void *out, s32 arg1, s32 arg2);
    u8 local[0x40];

    func_800A8B90(local, arg2, 0);
    func_800A88A0(arg1, arg0, local, arg3, arg4[0], arg4[1], arg5);
}

/**
 * @brief Format a value as a wide decimal string, then render it as text.
 * @param arg0 Ordering table for func_800A88A0.
 * @param arg1 Sprite scratch buffer for func_800A88A0.
 * @param arg2 Signed value to format into the glyph buffer.
 * @param arg3 Text color/style selector.
 * @param arg4 Pointer to a two-element x/y coordinate pair.
 * @param arg5 Alignment/post-processing flags.
 */
void func_800A8B04(void *arg0, void *arg1, s32 arg2, s32 arg3, s16 *arg4, s32 arg5)
{
    extern void func_800A8B90(void *out, s32 arg1, s32 arg2);
    u8 local[0x40];

    func_800A8B90(local, arg2, 1);
    func_800A88A0(arg1, arg0, local, arg3, arg4[0], arg4[1], arg5);
}

/**
 * @brief Format a signed decimal value into the destination glyph buffer.
 * @param buf Destination buffer.
 * @param val Signed value to format.
 */
void func_800A8B90(u8 *buf, s32 val)
{
    extern StructEC D_800EC3E4;
    u8 *dst;
    s32 value;
    s32 wide;
    u8 *minus;
    s32 low;
    s32 offset;
    s32 div;
    s32 started;
    s32 digit;

    dst = buf;
    value = val;
    wide = 0;
    if (value < 0)
    {
        value = -value;
        low = D_800EC3E4.unk0;
        offset = (D_800EC3E4.unk1 << 8) + (s32)((u8 *)&D_800EC3E4 - 0x20);
        minus = (u8 *)(low + offset);
        field_copy_name(dst, minus);
        dst += field_name_byte_length(minus);
    }
    div = 10000000;
    started = 0;
    do
    {
        digit = value / div;
        if (digit != 0)
        {
            started = 1;
        }
        if (started || div == 1)
        {
            if (wide)
            {
                *dst++ = 0x1D;
                *dst = digit;
            }
            else
            {
                *dst = digit + '0';
            }
            dst++;
            value -= (value / div) * div;
        }
        div /= 10;
    } while (div != 0);
    *dst = 0;
}

/* ---- Input repeat, saved names and inventory, actor labels (0x800A8CFC .. 0x800AA570) ---- */

#define FIELD_INVENTORY_COUNT 100
#define FIELD_INVENTORY_RECORD_SIZE 64
#define FIELD_DIALOG_ITEM_LIMIT 10
#define FIELD_PAD_REPEAT_DELAY 15
#define FIELD_PAD_REPEAT_INTERVAL 2
#define FIELD_PAD_DIRECTIONS (PADLup | PADLdown | PADLleft | PADLright)
#define FIELD_PAD_INACTIVE 0xFE
#define FIELD_ACTOR_ABSENT 0xFF
#define FIELD_SECONDARY_INPUT_ENABLED 0x80
#define FIELD_ACTOR_LABEL_HIGHLIGHT 0x80
#define FIELD_LABEL_FIRST_ACTOR 3
#define FIELD_LABEL_ACTOR_LIMIT 13
#define FIELD_LABEL_SCREEN_WIDTH 320
#define FIELD_LABEL_MIN_Y 50
#define FIELD_LABEL_MAX_Y 176
#define FIELD_LABEL_SELECTED_STYLE 4
#define FIELD_LABEL_NORMAL_STYLE 5
#define FIELD_LOW_HP_SOUND 0xA6
#define FIELD_INPUT_RESET_COMBINATION (PADh | PADselect | PADL1 | PADL2 | PADR1 | PADR2)
#define FIELD_MODAL_WORK_BUFFER ((void*)0x80170000)

/** @brief Saved inventory entry; a zero first byte marks a free slot. */
typedef struct
{
    u8 kind;
    u8 data[FIELD_INVENTORY_RECORD_SIZE - 1];
} FieldInventoryRecord;

/** @brief Saved per-player action choices and physical-button mapping. */
typedef struct
{
    u8 pad_0x0[0x608];
    u8 actions[0x30];
    u8 button_actions[8];
} FieldSavedInputMap;

/** @brief Player flags and resource kind in a 0x268-byte runtime record. */
typedef struct
{
    u8 flags;
    u8 kind;
    u8 pad_0x2[0x268 - 2];
} FieldLabelPlayer;

/** @brief Eight-byte action record whose first halfword selects label text. */
typedef struct
{
    u16 text_index;
    u8 pad_0x2[6];
} FieldLabelAction;

/** @brief Saved input settings and inventory used by this module. */
typedef struct
{
    u8 pad_0x0[0x840];
    u8 inject_enable;
    u8 pad_0x841[0x858 - 0x841];
    u32 inject_flags;
    u8 pad_0x85c[0xCE0 - 0x85C];
    FieldInventoryRecord inventory[FIELD_INVENTORY_COUNT];
} PadContext;

/** @brief Header fields of the pad context written by field_store_entry_settings. */
typedef struct
{
    u8 pad_0x0[0x18];
    u32 entry_config; /* 0x18 */
    s16 option_id;    /* 0x1C */
    s8 sub_mode;      /* 0x1E */
    u8 pad_0x1f;
    u32 music_track; /* 0x20 */
    s16 scene_mode;  /* 0x24 */
    s8 field_flags;  /* 0x26 */
    s8 layout_flags; /* 0x27 */
    u8 pad_0x28[0xCF - 0x28];
    s8 save_slot; /* 0xCF */
} FieldEntryHeader;

/** @brief Two-byte CD-error status string descriptor (field_draw_cd_error_text). */
typedef struct
{
    u8 low;
    u8 high;
} FieldTextOffset;

/** @brief Text ordering context passed to field_draw_cd_error_text. */
typedef struct
{
    u8 pad_0x0[0x3C];
    u32 ordering_table[1];
    u8 pad_0x40[0x40B8 - 0x40];
    s32 primitive_cursor;
} FieldLabelRenderContext;

/** @brief Presence byte in a scene actor record; stride 0x54. */
typedef struct
{
    u8 pad_0x0[0x25];
    u8 presence;
    u8 pad_0x26[0x54 - 0x26];
} FieldSceneActor;

/** @brief HP, group membership, and name used by actor labels; stride 0x23C. */
typedef struct
{
    s32 maximum_hp;
    s32 current_hp;
    s32 hp_display_flags;
    u32 object_flags;
    s32 group_flags;
    s32 record_id;
    u8 pad_0x18[0x4C - 0x18];
    u8 label_number;
    u8 pad_0x4d[0x64 - 0x4D];
    u8* name;
    u8 pad_0x68[0x23C - 0x68];
} FieldLabelActorState;

/** @brief Part footprint scales saved and restored by actor-label selection; stride 0x48. */
typedef struct
{
    u8 pad_0x0[0x2E];
    u8 footprint_scale_x;
    u8 pad_0x2f[0x33 - 0x2F];
    u8 footprint_scale_y;
    u8 pad_0x34[0x48 - 0x34];
} FieldLabelPart;

/** @brief Controller packet, quantized analog axes, and feedback bytes; stride 0xAE. */
typedef struct
{
    u8 device_type;
    u8 pad_0x1;
    u16 buttons;
    u8 pad_0x4[0x2C - 4];
    s16 axis_x;
    s16 axis_y;
    u8 pad_0x30[0x91 - 0x30];
    u8 feedback[2];
    u8 pad_0x93[0xAE - 0x93];
} FieldControllerSample;

/** @brief Flags, presence, and state in a 0x54-byte field actor record. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    u8 pad_0x0[0x1C - 12];
    u32 flags;
    u8 pad_0x20[5];
    u8 presence;
    u8 pad_0x26[4];
    s16 state;
    u8 tail[0x28];
} FieldInputActor;

#define PAD_HEADER ((FieldEntryHeader*)g_pad_ctx)

#define IS_DBCS_LEAD_BYTE(byte) (((byte) >= 0x19) && ((byte) <= 0x1F))
#define NAME_GLYPH_SIZE_SINGLE 1
#define NAME_GLYPH_SIZE_DOUBLE 2

#define FIELD_CONTROLLER_SAMPLES ((FieldControllerSample*)0x801ED600)

extern FieldLabelPart g_field_object_parts[];
extern u8 g_field_selected_actor_label;

/* Text and quantity pairs displayed by the field dialog. */
extern s32 g_field_dialog_item_texts[];
extern s32 g_field_dialog_item_count;
extern u8 g_field_dialog_item_quantities[];

/* Selected-actor bookkeeping. */
extern u8 g_field_label_actor_indices[];
extern u8 g_field_label_actor_count;
extern u8 g_field_label_saved_scale_x[];
extern u8 g_field_label_saved_scale_y[];

/* Field / actor tables. */
extern FieldSceneActor g_field_scene_actors[];
extern FieldLabelActorState g_field_scene_object_states[];
extern s32 g_field_active_group;
extern s32 D_800FDFC8;

/* Text/label offset tables (byte views). */
extern u8 g_field_hint_button_map[];
extern u8 D_800EC3C4[];
extern u8 D_800EC3E0[];
extern u8 D_800EC3E6[];
extern u8 D_800EC3E8[];
extern u8 D_800ED064[];
extern u8 D_800EDBE4[];
extern u8 D_8010A028[];

/* CD-error status string descriptors (field_draw_cd_error_text). */
extern FieldTextOffset D_800EC3D2;
extern FieldTextOffset D_800EC3D4;

/* Input repeat state. */
extern s32 g_field_primary_held_buttons;
extern s32 g_field_primary_repeat_delay;
extern s32 g_field_text_session_active;
extern s32 g_field_secondary_held_buttons;
extern s32 g_field_secondary_repeat_delay;
extern s32 g_field_buffered_input;
extern s32 g_field_text_session_cd_error;

/* Menu-open guards / miscellaneous field state. */
extern s32 D_8010AE78;
extern s32 D_80122710;
extern s32 D_80122714;
extern s32 D_800F2298;
extern s32 D_800F229C;
extern s32 g_field_return_to_title_prompt_state;
extern s32 D_8012291C;
extern s32 D_80122980;
extern u8 g_field_menu_controller_types[2];
extern u8 D_801227B9;
extern s32 g_pending_game_state;
extern s32 g_field_draw_count;
extern s32 g_frame_counter;

extern s32 g_save_slot_index;
extern s32 g_pad_input;
extern s32 g_pad_input_inject;

void akao_stop_sfx_by_id(s32 id);
void akao_cmd_99_9b_9d_9f(s32 arg0);
void akao_cmd_98_9a_9c_9e(s32 arg0);
void func_800A3904(s32 arg0, s32 arg1, s32 arg2);
void func_800A3938(s32 arg0, s32 arg1);
void field_restore_fade_target(void);
void field_refresh_party_routes(void);


s32 func_800B0850(void);

s32 func_8005B218(void);
void func_800AEE28(void);
void* func_800A88A0(SPRT* cursor, s32* ordering_table, u8* text, s32 color, s32 x, s32 y, s32 flags);
void* field_emit_actor_portrait(SPRT*, u32*, s32, u32*);
void* func_800AD208(s32*, void*, s32, s32, u16*, s32);
void* func_800AD524(u8*, s32*, s32, s32*, s32);
s32 func_800AE864(u8*);

/* Forward declarations for members called before their definition. */
void field_draw_cd_error_text(FieldLabelRenderContext* arg0);
void field_draw_actor_labels(void* arg0);
void field_reset_input_repeat(void);
s32 field_play_low_hp_warning(void);

/**
 * @brief Bind the field input and inventory context to the loaded saved game.
 */
void field_bind_saved_game_context(void)
{
    extern PadContext* g_pad_ctx;
    g_pad_ctx = (PadContext*)g_saved_game.bytes;
}

/**
 * @brief Save the field entry settings, music track, and current save slot.
 * @param scene_mode Scene entry mode.
 * @param field_flags Field entry flags.
 * @param layout_flags Field layout flags.
 * @param entry_config Packed entry configuration; only the low 25 bits are replaced.
 * @param option_id Entry option identifier.
 * @param sub_mode Entry submode.
 */
void field_store_entry_settings(s16 scene_mode, s8 field_flags, s8 layout_flags, s32 entry_config, s32 option_id, s32 sub_mode)
{
    extern PadContext* g_pad_ctx;
    extern u16 g_music_track_index;
    PAD_HEADER->scene_mode = scene_mode;
    PAD_HEADER->field_flags = field_flags;
    PAD_HEADER->layout_flags = layout_flags;
    PAD_HEADER->entry_config = (PAD_HEADER->entry_config & 0xFE000000) | (entry_config & 0x01FFFFFF);
    PAD_HEADER->option_id = (s16)option_id;
    PAD_HEADER->sub_mode = (s8)sub_mode;
    PAD_HEADER->music_track = (PAD_HEADER->music_track & 0xFFFC0000) | g_music_track_index;
    /* The saved header stores a single-byte slot number. */
    PAD_HEADER->save_slot = *(u8*)&g_save_slot_index;
}

/**
 * @brief Append text and its quantity to the field dialog item list.
 * @param text Address of the encoded item text.
 * @param quantity Quantity displayed alongside the text.
 */
void field_append_dialog_item(s32 text, u8 quantity)
{
    s32 index = g_field_dialog_item_count;

    if (index < FIELD_DIALOG_ITEM_LIMIT)
    {
        g_field_dialog_item_texts[index] = text;
        g_field_dialog_item_quantities[index] = quantity;
        g_field_dialog_item_count++;
    }
}

/**
 * @brief Count encoded name bytes, excluding the terminator.
 * @param name NUL-terminated name; lead bytes 0x19 through 0x1F begin two-byte glyphs.
 * @return Number of bytes before the terminator.
 */
s32 field_name_byte_length(u8* name)
{
    s32 count;
    u8 character;

    count = 0;
    character = *name;
    if (character != 0)
    {
        do
        {
            if ((u32)(character - 0x19) < 7)
            {
                name += 2;
                count += 2;
            }
            else
            {
                name += 1;
                count += 1;
            }
            character = *name;
        } while (character != 0);
    }
    return count;
}

/**
 * @brief Copy an encoded name, including its terminator.
 * @param destination Buffer receiving the name.
 * @param source NUL-terminated encoded name.
 */
void field_copy_name(u8* destination, u8* source)
{
    volatile u8* cursor;
    s32 byte_count;
    s32 byte_index;

    cursor = (volatile u8*)source;
    byte_count = 0;
    while (*cursor != 0)
    {
        if ((u32)(*cursor - 0x19) < 7)
        {
            cursor += 2;
            byte_count += 2;
        }
        else
        {
            cursor += 1;
            byte_count += 1;
        }
    }
    for (byte_index = 0; byte_index < byte_count; byte_index++)
    {
        destination[byte_index] = source[byte_index];
    }
    destination[byte_index] = 0;
}

/**
 * @brief Append a NUL-terminated name onto another, honouring DBCS glyph widths.
 * @param destination Existing name; the source is appended after its last glyph.
 * @param source Name to append.
 */
void field_append_name(u8* destination, const u8* source)
{
    const u8* scan_cursor;
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
 * @brief Copy one saved inventory record.
 * @param destination Buffer receiving the record.
 * @param source Record to copy.
 */
void field_copy_inventory_record(u8* destination, u8* source)
{
    u32 byte_index;

    byte_index = 0;
    do
    {
        byte_index++;
        *destination++ = *source++;
    } while (byte_index < FIELD_INVENTORY_RECORD_SIZE);
}

/**
 * @brief Close gaps in the saved inventory and mark the remaining slots free.
 * @note Record data beyond the occupancy byte is retained in free slots.
 * @see decomp.me (100%) TODO
 */
void field_compact_inventory(void)
{
    extern PadContext* g_pad_ctx;
    s32 record_index;
    FieldInventoryRecord* write_record;
    FieldInventoryRecord* read_record;

    record_index = 0;
    write_record = g_pad_ctx->inventory;
    read_record = write_record;
    do
    {
        if (read_record->kind != 0)
        {
            if (read_record != write_record)
            {
                bcopy(read_record, write_record, sizeof(*read_record));
                read_record->kind = 0;
            }
            write_record++;
        }
        record_index += 1;
        read_record++;
    } while (record_index < FIELD_INVENTORY_COUNT);
    while (write_record < &g_pad_ctx->inventory[FIELD_INVENTORY_COUNT])
    {
        write_record->kind = 0;
        write_record++;
    }
}

/**
 * @brief Find a free slot in the saved inventory.
 * @return Address of the first free record, or NULL when the inventory is full.
 */
u8* field_find_free_inventory_record(void)
{
    extern PadContext* g_pad_ctx;
    s32 record_index;
    FieldInventoryRecord* record;

    record = g_pad_ctx->inventory;
    for (record_index = 0; record_index < FIELD_INVENTORY_COUNT; record_index++)
    {
        if (record->kind == 0)
        {
            return (u8*)record;
        }
        record++;
    }
    return NULL;
}

/**
 * @brief Draw actor labels or CD-error text and upload the text cache.
 * @param context Render context receiving the text primitives.
 */
void field_draw_text_session(void* context)
{
    field_text_reset_scratch();

    if (g_field_text_session_cd_error)
    {
        field_draw_cd_error_text(context);
    }
    else
    {
        field_draw_actor_labels(context);
    }

    field_text_upload_immediate_cache();
}

/**
 * @brief Restore the part scales saved before displaying actor labels.
 */
void field_restore_label_actor_parts(void)
{
    s32 index;
    FieldLabelPart* part;

    akao_stop_sfx_by_id(0x7E);
    for (index = 0; index < g_field_label_actor_count; index++)
    {
        part = &g_field_object_parts[g_field_label_actor_indices[index]];
        part->footprint_scale_x = g_field_label_saved_scale_x[index];
        part->footprint_scale_y = g_field_label_saved_scale_y[index];
    }
}

/**
 * @brief Collect living named actors in the active group and save their part scales.
 */
void field_init_actor_labels(void)
{
    FieldSceneActor* actor;
    FieldLabelActorState* state;
    s32 actor_index;

    field_reset_input_repeat();
    akao_cmd_99_9b_9d_9f(2);
    func_800A3904(0, 0x3C, 0);
    akao_stop_sfx_by_id(0x7E);
    actor = g_field_scene_actors;
    actor_index = FIELD_LABEL_FIRST_ACTOR;
    state = g_field_scene_object_states;
    g_field_label_actor_count = 0;
    g_field_selected_actor_label = 0;
    do
    {
        if (actor->presence != FIELD_ACTOR_ABSENT && state->current_hp != 0 && g_field_active_group == (state->group_flags & 0xF) && state->name != 0)
        {
            g_field_label_actor_indices[g_field_label_actor_count] = actor_index;
            g_field_label_saved_scale_x[g_field_label_actor_count] = g_field_object_parts[actor_index].footprint_scale_x;
            g_field_label_saved_scale_y[g_field_label_actor_count] = g_field_object_parts[actor_index].footprint_scale_y;
            g_field_label_actor_count += 1;
        }
        actor_index += 1;
        state++;
        actor++;
    } while (actor_index < FIELD_LABEL_ACTOR_LIMIT);
}

/**
 * @brief Draw the status text selected by the current CD-ROM error.
 * @param context Render context containing the ordering table and primitive cursor.
 */
void field_draw_cd_error_text(FieldLabelRenderContext* context)
{
    s32 primitive;
    void* ordering_table;
    Vec2s text_positions[2];

    primitive = context->primitive_cursor;
    ordering_table = &context->ordering_table;
    if (cdrom_get_error_status() == 2)
    {
        s32 low;
        s32 offset;
        u8* base;

        low = D_800EC3D2.low;
        offset = (D_800EC3D2.high << 8) + (s32)(base = (u8*)&D_800EC3D2 - 0xE);
        primitive = (s32)func_800A88A0((SPRT*)primitive, ordering_table, (u8*)(low + offset), 4, 0xA0, 0x64, 0x82);
    }
    else
    {
        s32 low;
        s32 offset;
        u8* base;

        low = D_800EC3D4.low;
        offset = (D_800EC3D4.high << 8) + (s32)(base = (u8*)&D_800EC3D4 - 0x10);
        primitive = (s32)func_800A88A0((SPRT*)primitive, ordering_table, (u8*)(low + offset), 4, 0xA0, 0x64, 0x82);
    }
    context->primitive_cursor = primitive;
}

/**
 * @brief Draw controller action hints and labels for the selectable field actors.
 * @param context Render context receiving the labels.
 * @note Each controller displays the first active button among eight mapped controls.
 * @note TODO: 98.47% (419/432 exact). Remaining: button_index/g_pad_ctx a1-a2 swap,
 *       D_800ED064 high half in t3, and label y +0x40 scheduling; the resource-path
 *       (index << 4) * 25 spelling is a loop-invariant lever. See
 *       working/field_draw_actor_labels/status.md.
 */
void field_draw_actor_labels(void* context)
{
    extern u8 g_field_resource_actions[];
    extern FieldLabelPlayer g_field_player_records[];
    extern FieldLabelActorState g_field_object_states[];
    extern FieldInputActor g_field_actors[];
    extern PadContext* g_pad_ctx;
    s32 custom_text_offset;
    s32 pad_offset;
    DVECTOR point;
    FieldLabelPlayer* object_record;
    u8* label_low;
    u8* label_descriptor;
    s32 text_style;
    u16 text_offset;
    s32 swapped_buttons;
    s32 text_address;
    void** context_slot;
    s32 text_base;
    s32 label_base;
    s32 local_pad_offset;
    s32 local_text_offset;
    s32 screen_y;
    s32 projected_y;
    s32 camera_x_pixels;
    s32 actor_screen_x;
    s32 label_ot;
    s32 label_half_width;
    s32 actor_x;
    s32 camera_height;
    s32 number_ot;
    s32 camera_x;
    s32 camera_y;
    s32 camera_y_pixels;
    s32 text_ot;
    s32 left_glyph_ot;
    s32 right_glyph_ot;
    s32 button_index;
    s32 index;
    s32 primitive;
    s32 action_offset;
    s32 actor_y;
    s32 actor_height;
    u16 raw_buttons;
    u8* default_label_offset;
    s32 actor_id;
    s32 action_slot;
    s32 secondary_action;
    s32 secondary_action_alt;
    s32 label_value;
    FieldLabelPart* highlight_part;
    FieldInputActor* actor_position;
    FieldLabelPart* normal_part;
    FieldControllerSample* pad_sample;
    s32 text_high_or_offset;
    s32 pad_base;
    s32 name_offset;

    primitive = ((FieldLabelRenderContext*)context)->primitive_cursor;
    label_ot = (s32)((FieldLabelRenderContext*)context)->ordering_table;
    index = 0;
    pad_base = 0x801ED600;
    do
    {
        if ((g_field_player_records[index].flags & 1) && ((pad_sample = (FieldControllerSample*)(pad_base + index * 0xAE))->device_type < 0xFEU))
        {
            actor_id = 1;
            button_index = 0;
            local_pad_offset = index * 0x250;
            raw_buttons = pad_sample->buttons;
            name_offset = index * 0x250 + 0x5F0;
            swapped_buttons = ((raw_buttons << 8) & 0xFF00) | (raw_buttons >> 8);
            swapped_buttons = (((u32)(swapped_buttons & 0x40) >> 1) | ((swapped_buttons & 0x20) * 2) | ((u32)(swapped_buttons & 0x80) >> 3) |
                               ((swapped_buttons & 0x10) * 8) | (swapped_buttons & 0xFF0F));
            do
            {
                if (swapped_buttons & actor_id)
                {
                    swapped_buttons = (s32)g_pad_ctx + local_pad_offset;
                    action_slot = ((FieldSavedInputMap*)swapped_buttons)->button_actions[g_field_hint_button_map[button_index]];
                    switch (action_slot)
                    {
                    case 2:
                    case 3:
                        text_high_or_offset = (s32)D_8010A028 + index * 0x190;
                        action_slot *= sizeof(FieldLabelAction);
                        swapped_buttons = ((FieldLabelAction*)(text_high_or_offset + action_slot))->text_index & 0x7FFF;
                        label_value = (s32)D_800EDBE4;
                        text_high_or_offset = ((u16*)label_value)[swapped_buttons];
                        text_address = text_high_or_offset + label_value;
                        break;

                    case 0:
                        if (((((FieldSavedInputMap*)swapped_buttons)->actions[0] & 0x7F) == 2) &&
                            ((secondary_action = ((FieldSavedInputMap*)swapped_buttons)->actions[1], (secondary_action == 5)) || (secondary_action == 8)))
                        {
                            text_offset = D_800EC3E0[1] << 8;
                            text_high_or_offset = text_offset + (s32)D_800EC3C4;
                            label_value = D_800EC3E0[0];
                            text_address = text_high_or_offset + label_value;
                        }
                        else
                        {
                            label_descriptor = D_800EC3E6;
                            text_high_or_offset = label_descriptor[1];
                            label_value = D_800EC3E6[0];
                            text_high_or_offset <<= 8;
                            text_high_or_offset += (s32)D_800EC3C4;
                            text_address = text_high_or_offset + label_value;
                        }

                        break;
                    case 1:
                        if (((((FieldSavedInputMap*)swapped_buttons)->actions[0] & 0x7F) == 2) &&
                            ((secondary_action_alt = ((FieldSavedInputMap*)swapped_buttons)->actions[1], (secondary_action_alt == 5)) ||
                             (secondary_action_alt == 8)))
                        {
                            text_offset = D_800EC3E0[1] << 8;
                            text_high_or_offset = text_offset + (s32)D_800EC3C4;
                            label_value = D_800EC3E0[0];
                            text_address = text_high_or_offset + label_value;
                        }
                        else
                        {
                            text_high_or_offset = D_800EC3E8[1];
                            text_high_or_offset <<= 8;
                            text_high_or_offset += (s32)D_800EC3C4;
                            label_value = D_800EC3E8[0];
                            text_address = text_high_or_offset + label_value;
                        }
                        break;
                    default:
                        swapped_buttons = ((FieldSavedInputMap*)((u8*)g_pad_ctx + local_pad_offset))->actions[action_slot];
                        if (swapped_buttons == 0xFF)
                        {
                            text_offset = D_800EC3E0[1] << 8;
                            text_high_or_offset = text_offset + (s32)D_800EC3C4;
                            label_value = D_800EC3E0[0];
                            text_address = text_high_or_offset + label_value;
                        }
                        else
                        {
                            if (swapped_buttons & 0x80)
                            {
                                text_offset = swapped_buttons & 0xFF7F;
                                label_value = (s32)g_pad_ctx + name_offset;
                                text_high_or_offset = (text_offset << 6) + 0x150;
                                text_address = text_high_or_offset + label_value;
                            }
                            else
                            {
                                action_slot = (s32)&((FieldLabelAction*)g_field_resource_actions)[action_slot];
                                swapped_buttons = ((FieldLabelAction*)(((index << 4) * 25) + action_slot))->text_index;
                                swapped_buttons &= 0x7FFF;
                                label_value = g_field_player_records[index].kind;
                                swapped_buttons += label_value * 0x18;

                                label_value = (s32)D_800ED064;
                                text_high_or_offset = ((u16*)label_value)[swapped_buttons];
                                text_address = text_high_or_offset + label_value;
                            }
                        }

                        break;
                    }
                    actor_id = index << 5;
                    point.vx = 0x60;
                    point.vy = actor_id + 0x3C;

                    primitive = (s32)field_emit_actor_portrait((SPRT*)primitive, (u32*)label_ot, index, (u32*)&point);
                    actor_id += 0x40;
                    primitive = (s32)func_800A88A0((SPRT*)primitive, (s32*)label_ot, (u8*)text_address, 4, 0x80, actor_id, 0x80);
                    break;
                }
                else
                {
                    button_index += 1;
                    actor_id *= 2;
                }
            } while (button_index < 8);
        }
        index += 1;
    } while (index < 2);
    index = 0;
    if (g_field_label_actor_count != 0)
    {
        do
        {
            actor_id = g_field_label_actor_indices[index];
            camera_x = g_field_view_offset_x;
            text_address = (s32)&g_field_object_states[actor_id];
            actor_position = &g_field_actors[actor_id];
            if (camera_x < 0)
            {
                camera_x += 0xFF;
            }
            actor_x = actor_position->x;
            camera_x_pixels = camera_x >> 8;
            if (actor_x < 0)
            {
                actor_x += 0xFF;
            }
            camera_y = g_field_view_offset_y;
            actor_screen_x = (actor_x >> 8) + 0xA0;
            point.vx = camera_x_pixels + actor_screen_x;
            if (camera_y < 0)
            {
                camera_y += 0xFF;
            }
            camera_y_pixels = camera_y >> 8;
            actor_y = actor_position->y;
            if (actor_y < 0)
            {
                actor_y += 0xFF;
            }
            actor_height = actor_position->z;
            actor_y = (actor_y >> 8) + 0x70;
            screen_y = camera_y_pixels + actor_y;
            if (actor_height < 0)
            {
                actor_height += 0x1FF;
            }
            camera_height = g_field_view_offset_z;
            projected_y = screen_y - (actor_height >> 9);
            if (camera_height < 0)
            {
                camera_height += 0x1FF;
            }
            point.vy = projected_y - (camera_height >> 9);
            label_half_width = func_800AE864(((FieldLabelActorState*)text_address)->name) * 6;
            if ((point.vx + label_half_width) > FIELD_LABEL_SCREEN_WIDTH)
            {
                point.vx = FIELD_LABEL_SCREEN_WIDTH - label_half_width;
            }
            if (((point.vx - label_half_width) - 8) < 0)
            {
                point.vx = label_half_width + 8;
            }
            if (point.vy > FIELD_LABEL_MAX_Y)
            {
                point.vy = FIELD_LABEL_MAX_Y;
            }
            if (point.vy < FIELD_LABEL_MIN_Y)
            {
                point.vy = FIELD_LABEL_MIN_Y;
            }
            camera_x_pixels = point.vx;
            text_ot = label_ot;
            if (g_field_selected_actor_label == index)
            {
                text_ot = label_ot - 4;
            }
            text_style = FIELD_LABEL_NORMAL_STYLE;
            if (g_field_selected_actor_label == index)
            {
                text_style = FIELD_LABEL_SELECTED_STYLE;
            }
            primitive = (s32)func_800A88A0((SPRT*)primitive, (s32*)text_ot, ((FieldLabelActorState*)text_address)->name, text_style, camera_x_pixels,
                                           (s32)point.vy, 0x82);
            left_glyph_ot = label_ot;
            point.vy = (u16)point.vy - 8;
            if (g_field_selected_actor_label == index)
            {
                left_glyph_ot = label_ot - 4;
            }
            primitive = (s32)func_800AD524((u8*)primitive, (s32*)left_glyph_ot, 0xC, (s32*)&point, g_field_selected_actor_label == index ? 0x81 : 0x82);
            right_glyph_ot = label_ot;
            point.vx = (u16)point.vx + 8;
            if (g_field_selected_actor_label == index)
            {
                right_glyph_ot = label_ot - 4;
            }
            primitive = (s32)func_800AD524((u8*)primitive, (s32*)right_glyph_ot, 0xD, (s32*)&point, g_field_selected_actor_label == index ? 0x81 : 0x82);
            number_ot = label_ot;
            point.vx = (u16)point.vx + 8;
            if (g_field_selected_actor_label == index)
            {
                number_ot -= 4;
            }
            primitive = (s32)func_800AD208((s32*)number_ot, (void*)primitive, ((FieldLabelActorState*)text_address)->label_number >> 1, 2, (u16*)&point,
                                           g_field_selected_actor_label == index ? 0x81 : 0x82);
            if ((g_field_selected_actor_label == index) && (((FieldLabelActorState*)text_address)->hp_display_flags >= 0))
            {
                highlight_part = &g_field_object_parts[actor_id];
                highlight_part->footprint_scale_x = FIELD_ACTOR_LABEL_HIGHLIGHT;
                highlight_part->footprint_scale_y = FIELD_ACTOR_LABEL_HIGHLIGHT;
            }
            else
            {
                normal_part = &g_field_object_parts[actor_id];
                normal_part->footprint_scale_x = (u8)g_field_label_saved_scale_x[index];
                normal_part->footprint_scale_y = (u8)g_field_label_saved_scale_y[index];
            }
            index += 1;
        } while (index < (s32)g_field_label_actor_count);
    }
    ((FieldLabelRenderContext*)context)->primitive_cursor = primitive;
}

/**
 * @brief Combine quantities for repeated dialog text entries and close the gaps.
 */
void field_merge_dialog_items(void)
{
    s32 index;
    s32 previous;
    u8 amount;
    s32 shift_index;

    index = 0;
    if (g_field_dialog_item_count > 0)
    {
        do
        {
            if (g_field_dialog_item_quantities[index] != 0)
            {
                for (previous = 0; previous < index; previous++)
                {
                    amount = g_field_dialog_item_quantities[previous];
                    if ((amount != 0) && (g_field_dialog_item_texts[index] == g_field_dialog_item_texts[previous]))
                    {
                        shift_index = index;
                        g_field_dialog_item_quantities[previous] = amount + g_field_dialog_item_quantities[index];
                        while (shift_index < (g_field_dialog_item_count - 1))
                        {
                            g_field_dialog_item_quantities[shift_index] = g_field_dialog_item_quantities[shift_index + 1];
                            g_field_dialog_item_texts[shift_index] = g_field_dialog_item_texts[shift_index + 1];
                            shift_index += 1;
                        }
                        index -= 1;
                        g_field_dialog_item_count -= 1;
                        break;
                    }
                }
            }
            index += 1;
        } while (index < g_field_dialog_item_count);
    }
}

/**
 * @brief Move the actor-label selection or close the active text session.
 */
void field_update_text_session(void)
{
    extern PadContext* g_pad_ctx;
    s32 actor_index;
    FieldLabelPart* part;
    if ((g_field_text_session_cd_error && !cdrom_get_error_status()) ||
        (!g_field_text_session_cd_error &&
         (g_pad_input == PADh || ((g_pad_ctx->inject_flags & FIELD_SECONDARY_INPUT_ENABLED) && g_pad_ctx->inject_enable && g_pad_input_inject == PADh))))
    {
        g_field_draw_count = 0;
        g_field_text_session_active = 0;
        field_restore_fade_target();
        akao_stop_sfx_by_id(0x7E);
        for (actor_index = 0; actor_index < g_field_label_actor_count; actor_index++)
        {
            part = &g_field_object_parts[g_field_label_actor_indices[actor_index]];
            part->footprint_scale_x = g_field_label_saved_scale_x[actor_index];
            part->footprint_scale_y = g_field_label_saved_scale_y[actor_index];
        }
        field_reset_input_repeat();
    }
    else
    {
        g_field_draw_count = 1;
        if (g_field_label_actor_count >= 2U)
        {
            g_pad_input |= g_pad_input_inject;
            if (g_pad_input & (PADLup | PADLleft))
            {
                g_field_selected_actor_label = g_field_selected_actor_label ? g_field_selected_actor_label - 1 : g_field_label_actor_count - 1;
                akao_stop_sfx_by_id(0x7D);
            }
            else if (g_pad_input & (PADLdown | PADLright))
            {
                g_field_selected_actor_label = g_field_selected_actor_label == g_field_label_actor_count - 1 ? 0 : g_field_selected_actor_label + 1;
                akao_stop_sfx_by_id(0x7D);
            }
        }
    }
}

/**
 * @brief Read field buttons, including directional input from the analog axes.
 * @param index Controller port index.
 * @return Field button mask, or zero if the controller is unavailable.
 * @note Face-button bits are reordered from the hardware packet layout.
 */
s32 field_read_controller_buttons(s32 index)
{
    FieldControllerSample* base;
    FieldControllerSample* record;
    u16 flags;
    s32 value;
    s16 state;
    s32 sample_offset;

    base = FIELD_CONTROLLER_SAMPLES;
    record = &base[index];
    if (record->device_type >= FIELD_PAD_INACTIVE)
    {
        return 0;
    }

    flags = record->buttons;
    value = (flags >> 8) | ((flags & 0xFF) << 8);
    value = ((u32)(value & 0x40) >> 1) | ((value & 0x20) << 1) | ((u32)(value & 0x80) >> 3) | ((value & 0x10) << 3) | (value & 0xFF0F);

    if (record->device_type != 0)
    {
        state = record->axis_x;
        if (state < -1)
        {
            value |= PADLleft;
        }
        else if (state >= 2)
        {
            value |= PADLright;
        }

        sample_offset = index * sizeof(*base);
        state = ((FieldControllerSample*)((u8*)base + sample_offset))->axis_y;
        if (state < -1)
        {
            value |= PADLup;
        }
        else if (state >= 2)
        {
            value |= PADLdown;
        }
    }

    return value;
}

/**
 * @brief Apply initial-delay and repeat timing to both field controller inputs.
 * @note Held input waits fifteen ticks initially, then repeats every three ticks.
 * @note Modal input gives held direction buttons priority.
 * @see 100% match with GCC 2.7.2 CDK: 109 instructions, 436 bytes.
 */
void field_update_input_repeat(void)
{
    extern s32 g_field_modal_state;
    s32 directions;
    s32 buttons;

    buttons = field_read_controller_buttons(0);
    g_pad_input = 0;
    g_field_buffered_input = 0;
    if (((buttons == g_field_primary_held_buttons) || ((g_field_primary_held_buttons != 0) && (buttons & (g_field_primary_held_buttons | 0xB6F)))) &&
        buttons != 0)
    {
        directions = buttons & FIELD_PAD_DIRECTIONS;

        if ((directions != 0) && (g_field_modal_state != 0))
        {
            buttons = directions;
        }
        if (g_field_primary_repeat_delay == 0)
        {
            g_pad_input = buttons;
            g_field_primary_repeat_delay = FIELD_PAD_REPEAT_INTERVAL;
        }
        else
        {
            g_field_primary_repeat_delay -= 1;
            g_pad_input = 0;
        }
    }
    else if (buttons == 0)
    {
        g_field_primary_repeat_delay = 0;
        g_field_primary_held_buttons = 0;
    }
    else
    {
        g_pad_input = buttons;
        g_field_primary_held_buttons = buttons;
        g_field_primary_repeat_delay = FIELD_PAD_REPEAT_DELAY;
    }
    buttons = field_read_controller_buttons(1);
    g_pad_input_inject = 0;
    if (((buttons == g_field_secondary_held_buttons) || ((g_field_secondary_held_buttons != 0) && (buttons & (g_field_secondary_held_buttons | 0xB6F)))) &&
        buttons != 0)
    {
        directions = buttons & FIELD_PAD_DIRECTIONS;

        if ((directions != 0) && (g_field_modal_state != 0))
        {
            buttons = directions;
        }
        if (g_field_secondary_repeat_delay == 0)
        {
            g_pad_input_inject = buttons;
            g_field_secondary_repeat_delay = FIELD_PAD_REPEAT_INTERVAL;
        }
        else
        {
            g_field_secondary_repeat_delay -= 1;
            g_pad_input_inject = 0;
        }
    }
    else if (buttons == 0)
    {
        g_field_secondary_repeat_delay = 0;
        g_field_secondary_held_buttons = 0;
    }
    else
    {
        g_pad_input_inject = buttons;
        g_field_secondary_held_buttons = buttons;
        g_field_secondary_repeat_delay = FIELD_PAD_REPEAT_DELAY;
    }
    g_field_buffered_input = g_pad_input;
}

/**
 * @brief Clear delivered input and restart both controllers' held-button delays.
 */
void field_reset_input_repeat(void)
{
    g_pad_input = 0;
    g_field_primary_held_buttons = field_read_controller_buttons(0);
    g_field_primary_repeat_delay = FIELD_PAD_REPEAT_DELAY;
    g_pad_input_inject = 0;
    g_field_secondary_held_buttons = field_read_controller_buttons(1);
    g_field_secondary_repeat_delay = FIELD_PAD_REPEAT_DELAY;
    g_field_buffered_input = 0;
}

/**
 * @brief Handle reset, controller removal, CD errors, and guarded field-menu input.
 * @param context Value forwarded to the active text-session handler.
 */
void field_process_input(s32 context)
{
    extern s32 g_field_modal_state;
    extern FieldLabelActorState g_field_object_states[];
    extern FieldInputActor g_field_actors[];
    extern PadContext* g_pad_ctx;
    FieldControllerSample* pad = FIELD_CONTROLLER_SAMPLES;
    u32 buttons;
    FieldInputActor* actor;
    FieldInputActor* candidate;
    FieldLabelActorState* state;
    s32 actor_count;
    s32 actor_index;
    s32 absent_actor;

    buttons = pad[0].buttons;
    buttons = (buttons >> 8) | ((buttons & 0xFF) << 8);
    buttons = ((buttons & 0x40) >> 1) | ((buttons & 0x20) << 1) | ((buttons & 0x80) >> 3) | ((buttons & 0x10) << 3) | (buttons & 0xFF0F);
    if (g_field_modal_state != 0)
    {
        return;
    }
    if (buttons == FIELD_INPUT_RESET_COMBINATION)
    {
        g_pending_game_state = 4;
        pad[0].feedback[0] = 0;
        pad[0].feedback[1] = 0;
        pad[1].feedback[0] = 0;
        pad[1].feedback[1] = 0;
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
        field_refresh_party_routes();
    }
    if (g_field_text_session_active != 0)
    {
        field_update_modal_text_session(context);
        return;
    }
    actor = g_field_actors;
    actor_count = 0;
    if (!(actor->flags & 0x1FF))
    {
        do
        {
            if (g_field_actors[actor_count].presence != FIELD_ACTOR_ABSENT && g_field_actors[actor_count].state == 0x9A)
            {
                return;
            }
            actor_count++;
            actor++;
        } while (actor_count < 3);
        if (field_text_get_status(0) == -1 && D_800F2298 == 0 && D_800F229C == 0 && g_field_return_to_title_prompt_state == 0 && D_80122714 == 0 &&
            func_800B0850() == 0)
        {
            if (g_field_menu_controller_types[0] != 0xFF && pad[0].device_type == 0xFF)
            {
                field_run_menu(FIELD_MODAL_WORK_BUFFER, 0);
            }
            if (D_801227B9 != 0xFF && pad[1].device_type == 0xFF)
            {
                field_run_menu(FIELD_MODAL_WORK_BUFFER, 1);
            }
            if (cdrom_get_error_status() != 0)
            {
                g_field_text_session_cd_error = 1;
                pad[0].feedback[0] = 0;
                pad[0].feedback[1] = 0;
                pad[1].feedback[0] = 0;
                pad[1].feedback[1] = 0;
                field_begin_text_session();
                return;
            }
            g_field_menu_controller_types[0] = pad[0].device_type;
            g_field_menu_controller_types[1] = pad[1].device_type;
            if (D_8012291C != 0)
            {
                if (func_8005B218() == 0)
                {
                    if (g_pad_input == PADh ||
                        ((g_pad_ctx->inject_flags & FIELD_SECONDARY_INPUT_ENABLED) && g_pad_ctx->inject_enable && g_pad_input_inject == PADh))
                    {
                        field_run_menu(FIELD_MODAL_WORK_BUFFER, 0);
                    }
                }
            }
            else
            {
                if (g_pad_input == PADh || g_pad_input == PADRup ||
                    ((g_pad_ctx->inject_flags & FIELD_SECONDARY_INPUT_ENABLED) && g_pad_ctx->inject_enable &&
                     (g_pad_input_inject == PADh || g_pad_input_inject == PADRup)))
                {
                    field_run_menu(FIELD_MODAL_WORK_BUFFER, 0);
                }
            }
            field_play_low_hp_warning();
            if (g_pad_input & 0x80)
            {
                actor_count = 0;
                if (D_80122980 != 0)
                {
                    actor_index = actor_count;
                    absent_actor = 0xFF;
                    state = g_field_object_states;
                    candidate = g_field_actors;
                    do
                    {
                        if (candidate->presence != absent_actor && state->record_id >= 0x14)
                        {
                            actor_count++;
                        }
                        state++;
                        actor_index++;
                        candidate++;
                    } while (actor_index < FIELD_LABEL_ACTOR_LIMIT);
                    if (actor_count < 5)
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
 * @brief Play a staggered warning for living players below one-quarter HP.
 * @return Unspecified; callers ignore the return value.
 * @see decomp.me (100%) TODO
 */
s32 field_play_low_hp_warning(void)
{
    extern FieldLabelActorState g_field_object_states[];
    if (g_field_active_group != 0)
    {
        if (!(g_frame_counter & 0x1F))
        {
            if ((g_field_object_states[0].current_hp != 0) && ((u32)(g_field_object_states[0].current_hp * 4) < (u32)g_field_object_states[0].maximum_hp))
            {
                func_800A3938(FIELD_LOW_HP_SOUND, 0x80);
            }
        }

        if (!((g_frame_counter + 0x10) & 0x1F) && !(D_800FDFC8 & 0x1FF) && (g_field_object_states[1].current_hp != 0) &&
            ((u32)(g_field_object_states[1].current_hp * 4) < (u32)g_field_object_states[1].maximum_hp))
        {
            func_800A3938(FIELD_LOW_HP_SOUND, 0x80);
        }
    }
}

/* ---- Menu dispatch, modal overlays and duel panels (0x800AA570 .. 0x800AD030) ---- */

#define FIELD_SUBOVERLAY_ADDRESS ((void*)0x80140000)
#define FIELD_SAVED_CHARACTER_STRIDE 0x250
#define FIELD_INPUT_REPEAT_DELAY 15
#define FIELD_DUEL_PANEL_HOLD_FRAMES 90
#define FIELD_DUEL_PANEL_START_OFFSET 500

/** @brief Animation stages shared by the duel introduction and winner panels. */
typedef enum FieldDuelPanelPhase
{
    FIELD_DUEL_SLIDE_IN,
    FIELD_DUEL_HOLD,
    FIELD_DUEL_SLIDE_OUT,
    FIELD_DUEL_FINISHED
} FieldDuelPanelPhase;

/** @brief Party flags, weapon category, and action state in one 0x268-byte slot. */
typedef struct
{
    union
    {
        u16 word;
        struct
        {
            u8 flags, weapon_type;
        } bytes;
        struct
        {
            u16 active : 1;
            u16 selected : 1;
            u16 rest : 14;
        } bits;
    } head;
    u8 companion_id, companion_type;
    u8 unknown_0x004[0x250];
    u16 action_state;
    u8 action_index;
    u8 unknown_0x257;
    u8 weapon_flag;
    u8 unknown_0x259[0xF];
} FieldModalPartySlot;
/** @brief Actor flags within the 0x54-byte field record. */
typedef struct
{
    u8 unknown_0x000[0x1C];
    s32 flags;
    u8 unknown_0x020[0x34];
} FieldModalActor;
/** @brief Runtime health fields within the 0x23C-byte actor state. */
typedef struct
{
    s32 max_hp, hp, display_hp;
    u8 unknown_0x00c[0x230];
} FieldModalActorHealth;
/** @brief Eight-byte action descriptor with byte and halfword flag access. */
typedef struct
{
    s16 action_id;
    union
    {
        u16 word;
        struct
        {
            u8 low, high;
        } byte;
    } bits;
    s16 icon_id, texture_id;
} FieldModalAction;
/** @brief Selected item attributes at offsets 0x24 and 0x25. */
typedef struct
{
    u8 unknown_0x000[0x24];
    u8 instrument_type, spell;
} FieldModalInstrument;
/** @brief Accessed global settings and player fields within the saved context. */
typedef struct
{
    u8 unknown_0x000[0x18];
    u32 scene_config;
    s16 scene_option;
    s8 scene_sub_mode;
    u8 unknown_0x01f;
    s32 music_track;
    u16 scene_mode;
    u8 scene_flags;
    u8 layout_flags;
    u32 option_flags;
    u8 unknown_0x02c[0x5F0 - 0x2C];
    u8 name[24];
    u8 character_kind, character_id;
    u8 bound_actions[2];
    u8 equipped_abilities[4];
    u8 unknown_0x610[4];
    u16 max_hp;
    u8 unknown_0x616[0x1E];
    u16 duel_wins;
    u16 duel_losses;
    u8 unknown_0x638[8];
    u8 equipment_present_at_slot;
    u8 unknown_0x641[0x13];
    u32 equipment_flags;
    u8 unknown_0x658[0x1E8];
    u8 second_name[24];
    union
    {
        u32 word;
        u8 bytes[4];
    } second_character;
    u8 unknown_0x85c[0x884 - 0x85C];
    u16 second_duel_wins;
    u16 second_duel_losses;
    u8 unknown_0x888[0xCE0 - 0x888];
    u8 equipment_present;
    u8 unknown_0xce1[0x25E0 - 0xCE1];
    u8 item_counts[256];
} FieldModalSaveView;

/** @brief Unaligned little-endian offset into the field UI string table. */
typedef struct
{
    u8 low;
    u8 high;
} FieldModalStringOffset;

/** @brief Caller-owned block whose packet cursor lives at 0x40B8. */
typedef struct
{
    u8 unknown_0x000[0x40B8];
    s32 packet_cursor;
} FieldModalDrawContext;

s32 field_read_controller_buttons(s32);
void akao_cmd_98_9a_9c_9e(s32 context);

void func_800A3904(s32 context, s32 arg1, s32 arg2);
void field_update_text_session(void);

void field_set_fade_target_only(s16 red, s16 green, s16 blue, s16 duration);
void func_800A3938(s32 sound_id, s32 pan);
void func_800AE9E0(void);
void* func_800A88A0(SPRT* cursor, s32* ot, u8* text, s32 color, s32 x, s32 y, s32 flags);
s32 field_name_byte_length(u8* context);
void field_copy_name(u8* dest, u8* src);
s32 func_800AEAC0(s32 packet_cursor, void* arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
s32 func_800AF950(s32 packet_cursor, void* arg1, u8* str, s32 arg3, s32 x, s32 y, s32 arg6, s32 arg7, s32 arg8, s32 arg9, s32 arg10, s32 arg11);

void field_draw_empty_shop_notice(FieldModalDrawContext* context);
s32 field_draw_duel_intro(FieldModalDrawContext* context);
s32 field_draw_duel_result(FieldModalDrawContext* context);

extern u8 g_field_menu_controller_types[CONTROLLER_PORT_COUNT];
extern s32 g_field_primary_held_buttons;
extern s32 g_field_primary_repeat_delay;
extern s32 g_field_text_session_active;
extern s32 g_field_secondary_held_buttons;
extern s32 g_field_secondary_repeat_delay;
extern s32 D_8012291C;
extern s32 g_field_text_session_cd_error;
extern s32 g_field_buffered_input;
extern s32 g_pad_input;
extern s32 g_pad_input_inject;

extern s32 g_field_gosub_phase;
extern s32 D_801227F0;
extern s32 g_field_shop_active;
extern s32 g_gosub_result_count;

extern FieldModalStringOffset D_800EC400;
extern s32 g_field_duel_panel_phase;
extern s32 g_field_shop_notice_hidden;
extern s32 g_field_duel_panel_offset;
extern s32 g_field_duel_panel_hold_frames;

extern FieldModalStringOffset D_800EC3E4;
extern FieldModalStringOffset D_800EC406;
extern FieldModalStringOffset D_800EC408;
extern FieldModalStringOffset D_800EC40A;
extern FieldModalStringOffset D_800EC40C;
extern s32 g_field_duel_winner;

extern void func_80140024(u32, s32);
extern s32 func_801405B0(s32);
extern s32 g_field_actor_bindings[];
extern u8 D_801226B8[], D_801226F0[];
extern s32 D_8011F424, D_801227D4, D_801229F4, g_active_script, g_script_repeat_count;
extern void akao_set_paused(s32);
extern void field_refresh_party_routes(void);
extern void func_80091438(s32);
extern void func_800A3D44(s32, u8);
extern void func_800A5174(s32, s32);
extern void func_800A54D0(void);
extern u8 g_field_action_animation_parameters[];
extern u8 D_800EB24C[];
extern FieldModalPartySlot g_field_player_records[];
extern FieldModalActor g_field_actors[];
extern FieldModalActorHealth g_field_object_states[];
extern FieldModalAction g_field_resource_actions[];
extern s32 g_field_scene_mode_bit;
extern u8* g_pad_ctx;
extern u32 g_field_modal_state;
extern void field_restore_fade_target(void);
extern void field_release_actor_resource_slot(s32);
extern s32 field_activate_actor_resource_slot(s32, s32, s32);
extern void func_80084240(void);
extern void func_800A39A8(s32, s32, s32, s32);
extern void func_800A7384(void);

extern s32 func_800B0888(void);
extern s32 func_801400C4(s32);
extern s32 func_801400D4(s32);
extern s32 func_801401F0(s32);
extern s32 func_801401F8(s32);
extern s32 func_80140370(s32);
extern s32 g_field_niki_addhero_state;
extern s32 g_field_card_overlay_mode;
extern s32 g_gosub_result_values;
extern s16 g_music_track_index;
extern u8 D_800FDF79;
/* GNAME and SHOP share this entry address but have different parameter lists. */
extern void func_80140004();

/** @brief True for a DBCS lead byte (0x19-0x1F), which owns the following byte. */
#define IS_DBCS(c) ((u32)((c) - 0x19) < 7)

/** @brief Write a signed decimal into @p record_text (minus-sign glyph from the string table). */
#define FORMAT_SIGNED(record_text, val)                                                                                                                        \
    {                                                                                                                                                          \
        u8* dst;                                                                                                                                               \
        s32 value;                                                                                                                                             \
        s32 double_byte_digits;                                                                                                                                \
        u8* minus;                                                                                                                                             \
        s32 low;                                                                                                                                               \
        s32 offset;                                                                                                                                            \
        s32 div;                                                                                                                                               \
        s32 started;                                                                                                                                           \
        s32 digit;                                                                                                                                             \
        dst = record_text;                                                                                                                                     \
        value = val;                                                                                                                                           \
        double_byte_digits = 0;                                                                                                                                \
        if (value < 0)                                                                                                                                         \
        {                                                                                                                                                      \
            value = -value;                                                                                                                                    \
            low = D_800EC3E4.low;                                                                                                                              \
            offset = (D_800EC3E4.high << 8) + (s32)((u8*)&D_800EC3E4 - 0x20);                                                                                  \
            minus = (u8*)(low + offset);                                                                                                                       \
            field_copy_name(dst, minus);                                                                                                                       \
            dst += field_name_byte_length(minus);                                                                                                              \
        }                                                                                                                                                      \
        div = 10000000;                                                                                                                                        \
        started = 0;                                                                                                                                           \
        do                                                                                                                                                     \
        {                                                                                                                                                      \
            digit = value / div;                                                                                                                               \
            if (digit != 0)                                                                                                                                    \
            {                                                                                                                                                  \
                started = 1;                                                                                                                                   \
            }                                                                                                                                                  \
            if (started || div == 1)                                                                                                                           \
            {                                                                                                                                                  \
                if (double_byte_digits)                                                                                                                        \
                {                                                                                                                                              \
                    *dst++ = 0x1D;                                                                                                                             \
                    *dst = digit;                                                                                                                              \
                }                                                                                                                                              \
                else                                                                                                                                           \
                {                                                                                                                                              \
                    *dst = digit + '0';                                                                                                                        \
                }                                                                                                                                              \
                dst++;                                                                                                                                         \
                value -= (value / div) * div;                                                                                                                  \
            }                                                                                                                                                  \
            div /= 10;                                                                                                                                         \
        } while (div != 0);                                                                                                                                    \
        *dst = 0;                                                                                                                                              \
    }

/** @brief Advance @p p to the terminator, accumulating the DBCS-aware byte length. */
#define STR_LEN_LOOP(p, len)                                                                                                                                   \
    while (*p != 0)                                                                                                                                            \
    {                                                                                                                                                          \
        if (IS_DBCS(*p))                                                                                                                                       \
        {                                                                                                                                                      \
            p += 2;                                                                                                                                            \
            len += 2;                                                                                                                                          \
        }                                                                                                                                                      \
        else                                                                                                                                                   \
        {                                                                                                                                                      \
            p += 1;                                                                                                                                            \
            len += 1;                                                                                                                                          \
        }                                                                                                                                                      \
    }

/** @brief Shared strcat body: append @p s_ after the last glyph of @p d_. */
#define STR_CAT_BODY(d_, s_, qsrc)                                                                                                                             \
    {                                                                                                                                                          \
        volatile u8* p = d_;                                                                                                                                   \
        s32 len_d = 0;                                                                                                                                         \
        volatile u8* q;                                                                                                                                        \
        s32 len_s;                                                                                                                                             \
        s32 append;                                                                                                                                            \
        s32 i;                                                                                                                                                 \
        STR_LEN_LOOP(p, len_d)                                                                                                                                 \
        q = qsrc;                                                                                                                                              \
        len_s = 0;                                                                                                                                             \
        append = len_d;                                                                                                                                        \
        STR_LEN_LOOP(q, len_s)                                                                                                                                 \
        for (i = 0; i < len_s; i++)                                                                                                                            \
        {                                                                                                                                                      \
            d_[i + append] = s_[i];                                                                                                                            \
        }                                                                                                                                                      \
        d_[i + append] = 0;                                                                                                                                    \
    }

/** @brief Append buffer @p s onto @p d. */
#define STR_CAT(d, s)                                                                                                                                          \
    {                                                                                                                                                          \
        u8* d_ = (d);                                                                                                                                          \
        u8* s_ = (s);                                                                                                                                          \
        STR_CAT_BODY(d_, s_, (s))                                                                                                                              \
    }

/** @brief Append string-table entry @p sym onto @p d. */
#define STR_CAT_ENTRY(d, sym, off)                                                                                                                             \
    {                                                                                                                                                          \
        u8* d_ = (d);                                                                                                                                          \
        u8* s_ = (u8*)(((sym).high << 8) + (sym).low);                                                                                                         \
        s_ += (s32)((u8*)&(sym) - (off));                                                                                                                      \
        STR_CAT_BODY(d_, s_, s_)                                                                                                                               \
    }

/** @brief Panel contents are drawn only while the panel is entering or leaving. */
#define DRAW_FLAG (g_field_duel_panel_phase == FIELD_DUEL_SLIDE_IN || g_field_duel_panel_phase == FIELD_DUEL_SLIDE_OUT)

/**
 * @brief Run the menu overlay and dispatch its requested follow-up screens.
 * @param render_buffers Pair of MENU render buffers.
 * @param input_source Caller input-source selector; unused by this routine.
 * @note Fixed entry addresses are reused by the overlays loaded before each call.
 */
void field_run_menu(void* render_buffers, s32 input_source)
{
    s32 menu_locked;
    ControllerState* controllers;
    s32 screen_id;

    controllers = CONTROLLER_STATE;
    menu_locked = D_8012291C;
    g_field_text_session_cd_error = 0;
    controllers->ports[0].small_motor_command = 0;
    controllers->ports[0].actuator_control.fields.large_motor_command = 0;
    controllers->ports[1].small_motor_command = 0;
    controllers->ports[1].actuator_control.fields.large_motor_command = 0;
    if (menu_locked != 0 || g_field_actor_bindings[0] != 0 || g_field_actor_bindings[7] != 0 || g_field_actor_bindings[14] != 0)
    {
        field_begin_text_session();
        return;
    }
    func_800A3938(0x80, 0x80);
    func_80084240();

    g_active_script = 0;

    for (;;)
    {
        cdrom_stream(CD_RES_MENU_BIN, FIELD_SUBOVERLAY_ADDRESS);
        cdrom_wait_queue_empty();
        screen_id = func_801405B0((s32)render_buffers);
        if (screen_id == 0)
        {
            field_rebuild_party_actions(1);
            break;
        }
        if (screen_id == 0xA)
        {
            field_rebuild_party_actions(1);
            cdrom_stream(CD_RES_GOLEM_BIN, FIELD_SUBOVERLAY_ADDRESS);
            cdrom_wait_queue_empty();
            func_80140024(0x80150000, 1);
            func_800C3BB0();
            func_80084240();
            field_text_reset_windows();
            g_active_script = screen_id;
            g_script_repeat_count = 0;
        }
        else
        {
            field_rebuild_party_actions(1);
            cdrom_stream(CD_RES_GNAME_BIN, FIELD_SUBOVERLAY_ADDRESS);
            cdrom_wait_queue_empty();
            if ((u32)(screen_id - 0xB) < 2U)
            {
                func_80140004((void*)0x80160000, D_801226F0, D_801227D4, 1, D_801229F4, D_801226B8, 0);
            }
            else
            {
                func_80140004((void*)0x80160000, D_801226F0, D_801227D4, screen_id, D_801229F4, D_801226B8, 0);
            }
            field_text_reset_windows();
            g_script_repeat_count = D_801229F4;
            if ((u32)(screen_id - 0xB) < 2U)
            {
                g_script_repeat_count = 0;
                g_active_script = screen_id;
            }
            else
            {
                g_active_script = D_8011F424 + 1;
            }
        }
    }
    func_80084240();
}

/**
 * @brief Open the field text session and reset both controller repeat timers.
 * @see decomp.me (100%) TODO
 */
void field_begin_text_session(void)
{
    g_field_text_session_active = 1;
    field_set_cd_error_fade_target();
    g_pad_input = 0;
    g_field_primary_held_buttons = field_read_controller_buttons(0);
    g_field_primary_repeat_delay = FIELD_INPUT_REPEAT_DELAY;
    g_pad_input_inject = 0;
    g_field_secondary_held_buttons = field_read_controller_buttons(1);
    g_field_secondary_repeat_delay = FIELD_INPUT_REPEAT_DELAY;
    g_field_buffered_input = 0;
    field_init_actor_labels();
}

/**
 * @brief Clear text-session flags and remember both controller device types.
 */
void field_reset_text_session(void)
{
    ControllerState* controllers = CONTROLLER_STATE;

    g_field_text_session_active = 0;
    D_8012291C = 0;
    g_field_menu_controller_types[0] = controllers->ports[0].published_sample.device_type;
    g_field_menu_controller_types[1] = controllers->ports[1].published_sample.device_type;
}

/**
 * @brief Update the active text session, draw its contents, and handle closing.
 * @param context Render context forwarded to the text renderer.
 * @see decomp.me (100%) TODO
 */
void field_update_modal_text_session(s32 context)
{
    if (g_field_text_session_active != 0)
    {
        field_update_text_session();
        if (g_field_text_session_active != 0)
        {
            field_text_reset_scratch();
            if (g_field_text_session_cd_error != 0)
            {
                field_draw_cd_error_text(context);
            }
            else
            {
                field_draw_actor_labels(context);
            }
            field_text_upload_immediate_cache();
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
 * @note TODO: 99.304% (369/381 exact). first_absent is a matching scaffold: an early 0xFF copied into absent
 *       fixes the ability-loop register rotation; its original source is unknown. Remaining: g_pad_ctx/record_base
 *       s5-s6 swap. See working/field_rebuild_party_actions/status.md.
 */
void field_rebuild_party_actions(s32 refresh_only)
{
    FieldModalPartySlot* initial_party;
    FieldModalPartySlot* party;
    FieldModalPartySlot* equipment_owner;
    s32 texture_value;
    s16 technique_action_id;
    u8* item_context;
    s32 context_offset;
    s32 record_offset;
    s32 absent;
    s32 ability_empty;
    s32 first_absent;
    FieldModalAction* record_base;
    u8* pair_first;
    u8* pair_second;
    FieldModalSaveView* input_base;
    s32 character_kind;
    s32 controller_or_player_test;
    s32 equipment_offset;
    s32 equipment_index;
    s32 button_index;
    s32 action_offset;
    s32 saved_offset;
    s32 actor_stride_words;
    s32 player_index;
    s32 context_stride;
    s32 record_stride;
    s32 state_stride;
    s32 item_record_offset;
    u16 max_hp;
    u32 equipment_flags;
    u8 spell;
    u8 weapon_type;
    u8 ability_id;
    FieldModalActor* actor;
    FieldModalActor* actor_base;
    FieldModalAction* button_action;
    FieldModalInstrument* instrument;
    FieldModalAction* technique_action;
    FieldModalSaveView* saved_player;
    FieldModalAction* instrument_action;
    FieldModalSaveView* ability_start;
    FieldModalAction* empty_action;
    FieldModalSaveView* saved_member;
    FieldModalActorHealth* health;
    FieldModalSaveView* equipment;
    FieldModalSaveView* button;
    FieldModalSaveView* item_cursor;

    akao_set_paused((((u32)((FieldModalSaveView*)g_pad_ctx)->option_flags >> 1) & 1) ^ 1);
    cdrom_set_audio_volume(0x7F, ((u32)((FieldModalSaveView*)g_pad_ctx)->option_flags >> 1) & 1);
    controller_or_player_test = (s32)CONTROLLER_STATE;
    ((ControllerState*)controller_or_player_test)->ports[0].actuators_enabled = (s8)(*(volatile u32*)&((FieldModalSaveView*)g_pad_ctx)->option_flags & 1);
    if ((((FieldModalSaveView*)g_pad_ctx)->second_character.word & 0x80) && (((FieldModalSaveView*)g_pad_ctx)->second_name[0] != 0))
    {
        ((ControllerState*)controller_or_player_test)->ports[1].actuators_enabled = (s8)(*(volatile u32*)&((FieldModalSaveView*)g_pad_ctx)->option_flags & 1);
    }
    else
    {
        ((ControllerState*)controller_or_player_test)->ports[1].actuators_enabled = 0;
    }
    if (refresh_only == 0)
    {
        player_index = 0;
        do
        {
            saved_offset = player_index * FIELD_SAVED_CHARACTER_STRIDE;
            initial_party = &g_field_player_records[player_index];

            if (((FieldModalSaveView*)(g_pad_ctx + saved_offset))->name[0] != 0)
            {
                initial_party->head.bytes.weapon_type = 0xFF;
                saved_member = (FieldModalSaveView*)(g_pad_ctx + saved_offset);
                initial_party->head.bits.active = 1;
                button_index = saved_member->character_kind & 0x7F;
                character_kind = button_index;
                if (character_kind < 2)
                {
                    initial_party->head.bits.selected = character_kind;
                    initial_party->companion_type = 0;
                }
                else if (character_kind == 2)
                {
                    initial_party->head.bits.selected = 0;
                    initial_party->companion_id = saved_member->character_id;
                    initial_party->companion_type = 1;
                }
                else if (character_kind == 3)
                {
                    initial_party->head.bits.selected = 0;
                    initial_party->companion_id = saved_member->character_id;
                    initial_party->companion_type = 2;
                }
                else if (character_kind == 4)
                {
                    initial_party->head.bits.selected = 0;
                    initial_party->companion_id = saved_member->character_id + 0x41;
                    initial_party->companion_type = 2;
                }
            }
            else
            {
                initial_party->companion_type = 1;
                initial_party->companion_id = 0U;
                initial_party->head.bits.active = 0;
            }
            player_index += 1;

        } while (player_index < 3);
        func_800A54D0();
    }
    player_index = 0;
    record_base = g_field_resource_actions;
    do
    {
        party = &g_field_player_records[player_index];
        actor_stride_words = player_index * 0x14;
        record_stride = player_index * 0x190;
        context_stride = player_index * FIELD_SAVED_CHARACTER_STRIDE;
        state_stride = player_index * 0x23C;

        if (party->head.bytes.flags & 1)
        {
            actor_base = g_field_actors;
            actor = (FieldModalActor*)((actor_stride_words + player_index) * 4 + (u8*)actor_base);
            saved_player = (FieldModalSaveView*)(g_pad_ctx + context_stride);
            first_absent = 0xFF;
            actor->flags = (s32)((actor->flags & ~0x1FF) | (((u8)saved_player->character_kind >> 7) ^ 1));
            weapon_type = ((u32)saved_player->equipment_flags >> 0xA) & 0x3F;
            controller_or_player_test = player_index < 2;
            if (party->head.bytes.weapon_type != weapon_type)
            {
                party->head.bytes.weapon_type = weapon_type;
                if (controller_or_player_test != 0)
                {
                    func_80091438(player_index);
                }
                if (refresh_only == 0)
                {
                    health = (FieldModalActorHealth*)(state_stride + (u8*)g_field_object_states);
                    max_hp = ((FieldModalSaveView*)(g_pad_ctx + context_stride))->max_hp;
                    health->display_hp = (s32)((health->display_hp & 0xFF000000) | max_hp);
                    health->max_hp = (s32)max_hp;
                    health->hp = (s32)max_hp;
                }
                if ((g_field_scene_mode_bit != 0) && (refresh_only != 0) && (controller_or_player_test != 0))
                {
                    func_800A3D44(player_index, party->head.bytes.weapon_type);
                }
            }
            party->weapon_flag = 0;
            if ((u32)(party->head.bytes.weapon_type - 1) < 2U)
            {
                party->weapon_flag = 1;
                equipment_index = 1;
                equipment_owner = party;
                equipment_offset = context_stride + 0x40;
                do
                {
                    equipment = (FieldModalSaveView*)(g_pad_ctx + equipment_offset);
                    if (equipment->equipment_present_at_slot != 0)
                    {
                        equipment_flags = equipment->equipment_flags;
                        if ((((equipment_flags >> 8) & 3) == 1) && !((equipment_flags >> 0xA) & 0x3F))
                        {
                            equipment_owner->weapon_flag = 0;
                        }
                    }
                    equipment_index += 1;
                    equipment_offset += 0x40;
                } while (equipment_index < 4);
            }
            if (player_index == 2 && g_field_player_records[2].companion_type == player_index)
            {
                func_800A5174(2, g_field_player_records[2].companion_id + 0xA9B);
            }
            else
            {
                button_index = 0;
                pair_first = g_field_action_animation_parameters;
                pair_second = g_field_action_animation_parameters + 1;
                input_base = (FieldModalSaveView*)(g_pad_ctx + context_stride);
                action_offset = record_stride;
                do
                {
                    button = (FieldModalSaveView*)((u8*)input_base + button_index);
                    button_action = (FieldModalAction*)(action_offset + (u32)record_base);
                    button_action->action_id = (s16)button->bound_actions[0];
                    button_action->icon_id = (s16) * ((button->bound_actions[0] * 2) + pair_first);
                    button_action->texture_id = (s16) * ((button->bound_actions[0] * 2) + pair_second);
                    action_offset += 8;
                    button_index += 1;
                } while (button_index < 2);
                context_offset = context_stride;
                absent = first_absent;
                record_offset = record_stride;
                item_context = g_pad_ctx;
                ability_start = (FieldModalSaveView*)(item_context + context_offset);
                item_record_offset = 0x20;
                item_cursor = ability_start;
            rebuild_equipped_ability:

                ability_empty = item_cursor->equipped_abilities[0] == absent;
                if (ability_empty)
                {
                    empty_action = (FieldModalAction*)(item_record_offset + record_offset + (u32)record_base);
                    empty_action->bits.word = (u16)(empty_action->bits.word & 0xFBFF);
                    empty_action->bits.byte.low = absent;
                    empty_action->action_id = 0;
                    empty_action->icon_id = 0;
                    empty_action->texture_id = 0;
                    empty_action->bits.word = (u16)(empty_action->bits.word & 0xFCFF);
                }
                else
                {
                    ability_id = item_cursor->equipped_abilities[0];
                    if (ability_id & 0x80)
                    {
                        instrument_action = (FieldModalAction*)(item_record_offset + record_offset + (u32)record_base);
                        instrument_action->action_id = 0;
                        instrument_action->bits.word = (u16)(instrument_action->bits.word | 0x400);
                        instrument = (FieldModalInstrument*)(item_context + (context_offset + 0x5F0) + (((ability_id & 0x7F) << 6) + 0x150));
                        instrument_action->bits.byte.low = (s8)((u8)instrument->spell >> 1);
                        instrument_action->icon_id = (s16) * (instrument->instrument_type + D_800EB24C);
                        spell = instrument->spell;
                        texture_value = 0x8018;
                        texture_value += spell;
                        texture_value += instrument->instrument_type * 0xE;
                        if (!(spell & 1))
                        {
                            texture_value += 0x800;
                        }
                        instrument_action->texture_id = texture_value;
                        instrument_action->bits.word = (u16)(instrument_action->bits.word & 0xFCFF);
                    }
                    else
                    {
                        technique_action = (FieldModalAction*)(item_record_offset + record_offset + (u32)record_base);
                        technique_action->bits.word = (u16)(technique_action->bits.word & 0xFBFF);
                        ability_id = item_cursor->equipped_abilities[0];
                        technique_action_id = (s16)(ability_id | 0x8000);
                        technique_action->bits.byte.low = absent;
                        technique_action->icon_id = 2;
                        technique_action->action_id = technique_action_id;
                        ability_id = item_cursor->equipped_abilities[0];
                        weapon_type = g_field_player_records[player_index].head.bytes.weapon_type;
                        technique_action->bits.word = (u16)(technique_action->bits.word & 0xFCFF);
                        technique_action->texture_id = (s16)(((ability_id + 0x88) | ~0x7FFF) + (weapon_type * 0x18));
                    }
                }
                item_cursor = (FieldModalSaveView*)((u8*)item_cursor + 1);
                item_record_offset += 8;

                if ((s32)item_cursor < (s32)((u8*)ability_start + 4))
                {
                    goto rebuild_equipped_ability;
                }
            }
        }
        player_index += 1;

    } while (player_index < 3);
    field_refresh_party_routes();
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
    cdrom_stream(CD_RES_GNAME_BIN, FIELD_SUBOVERLAY_ADDRESS);
    cdrom_wait_queue_empty();
    func_80140004((void*)0x80160000, initial_name, active_name, source_mode, history_index, custom_name, 0);
    field_text_reset_windows();
    func_80084240();
}

/**
 * @brief Load ZUKAN.BIN and run its entry point at 0x80140E00.
 * @param context Forwarded to the ZUKAN entry point; meaning not yet established.
 */
void field_run_zukan(s32 context)
{
    func_80084240();
    cdrom_stream(CD_RES_ZUKAN_BIN, FIELD_SUBOVERLAY_ADDRESS);
    cdrom_wait_queue_empty();
    func_80140E00((void*)0x80160000, context);
    func_80084240();
}

/**
 * @brief Load GOSUB.BIN and open a screen sequence, unless a sub-overlay is already active.
 *
 * Sets g_field_modal_state to 2 for the duration and clears the gosub result count.
 *
 * @param screen_sequence Terminated s32 array passed to gosub_open_screen_sequence.
 * @see decomp.me (100%) TODO
 */
void field_open_gosub_screen_sequence(void* screen_sequence)
{
    if (g_field_modal_state == FIELD_MODAL_NONE)
    {
        D_801227F0 = 1;
        g_gosub_result_count = 0;
        func_80084240();
        cdrom_stream(CD_RES_GOSUB_BIN, FIELD_SUBOVERLAY_ADDRESS);
        cdrom_wait_queue_empty();
        g_field_modal_state = FIELD_MODAL_GOSUB;
        g_field_gosub_phase = 2;
        func_80140080((void*)0x80175000, screen_sequence);
    }
}

/**
 * @brief Open the selling screen when inventory is present, otherwise show a notice.
 * @param shop_options Value forwarded to the shop; meaning not yet established.
 * @note The equipment loop repeatedly checks the first record, as in the original.
 * @see decomp.me (100%) TODO
 */
void field_open_shop_mode_0(s32 shop_options)
{
    s32 count;
    s32 i;

    if (g_field_modal_state == FIELD_MODAL_NONE)
    {
        count = 0;
        for (i = 0; i < 0x64; i++)
        {
            if (((FieldModalSaveView*)g_pad_ctx)->equipment_present != 0)
            {
                count++;
                break;
            }
        }
        for (i = 0; i < 0x100; i++)
        {
            if (((FieldModalSaveView*)g_pad_ctx)->item_counts[i] != 0)
            {
                count++;
                break;
            }
        }
        if (count == 0)
        {
            field_begin_empty_shop_notice(0);
        }
        else
        {
            func_80084240();
            cdrom_stream(CD_RES_SHOP_BIN, FIELD_SUBOVERLAY_ADDRESS);
            cdrom_wait_queue_empty();
            g_field_shop_active = 1;
            g_field_modal_state = FIELD_MODAL_SHOP;
            func_80140004((void*)0x80150000, 0, 0, 0, 0, shop_options);
        }
    }
}

/**
 * @brief Open the shop with a caller-supplied entry list.
 * @param entry_count Number of entries in the supplied list.
 * @param entries Address of the eight-byte shop entry array.
 * @param list_options Additional list data forwarded to the shop; meaning unresolved.
 * @param shop_options Value forwarded as the final shop argument.
 * @see decomp.me (100%) TODO
 */
void field_open_shop_mode_1(s32 entry_count, s32 entries, s32 list_options, s32 shop_options)
{
    if (g_field_modal_state == FIELD_MODAL_NONE)
    {
        func_80084240();
        cdrom_stream(CD_RES_SHOP_BIN, FIELD_SUBOVERLAY_ADDRESS);
        cdrom_wait_queue_empty();
        g_field_shop_active = 1;
        g_field_modal_state = FIELD_MODAL_SHOP;
        func_80140004((void*)0x80150000, 1, entry_count, entries, list_options, shop_options);
    }
}

/**
 * @brief Advance the active modal overlay and restore field input on completion.
 * @param context_or_delay Render context address, reused for the input repeat delay.
 */
void field_update_modal(s32 context_or_delay)
{
    FieldModalPartySlot* slot;
    s32 result;
    s32 index_or_zero;

    switch (g_field_modal_state)
    {
    case FIELD_MODAL_SHOP:
        if ((g_field_shop_active != 0) && (func_801400D4(context_or_delay) != 0))
        {
            g_field_shop_active = 0;
            g_field_modal_state = FIELD_MODAL_NONE;
            func_80084240();
            return;
        }
    case FIELD_MODAL_NONE:
        return;
    case FIELD_MODAL_GOSUB:
        if (g_field_gosub_phase != 0)
        {
            if (g_field_gosub_phase >= 2)
            {
                if (func_801400C4(context_or_delay) != 0)
                {
                    DrawSync(0);
                    g_field_gosub_phase = 1;
                    func_80084240();
                    return;
                }
            }
            else
            {
                DrawSync(0);
                field_text_reset_windows();
                D_801227F0 = 2;
                g_field_modal_state = FIELD_MODAL_NONE;
                g_field_gosub_phase = 0;
                return;
            }
        }
        return;
    case FIELD_MODAL_CARDA:
        if ((g_field_card_overlay_mode != 0) && (func_80140370(context_or_delay) != 0))
        {
            func_80084240();
            switch (g_field_card_overlay_mode)
            {
            case 2:
                field_rebuild_party_actions(0);
                index_or_zero = 0;
                slot = g_field_player_records;
                do
                {
                    slot = &g_field_player_records[index_or_zero];
                    slot->action_state = 0;
                    slot->action_index = 0xFF;
                    index_or_zero += 1;
                } while (index_or_zero < 3);
                g_music_track_index = (s16) * (volatile s32*)&((FieldModalSaveView*)g_pad_ctx)->music_track;
                field_set_scene_parameters(((FieldModalSaveView*)g_pad_ctx)->scene_mode, ((FieldModalSaveView*)g_pad_ctx)->scene_flags,
                                           ((FieldModalSaveView*)g_pad_ctx)->scene_config & 0x01FFFFFF, ((FieldModalSaveView*)g_pad_ctx)->layout_flags,
                                           (s32)((FieldModalSaveView*)g_pad_ctx)->scene_option, (s32)((FieldModalSaveView*)g_pad_ctx)->scene_sub_mode);
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
            g_field_card_overlay_mode = 0;
            g_field_modal_state = FIELD_MODAL_NONE;
            return;
        }
        return;
    case FIELD_MODAL_NIKI:
        if ((g_field_niki_addhero_state != 0) && (func_801401F0(context_or_delay) != 0))
        {
            func_80084240();
            g_field_niki_addhero_state = 0;
            g_field_modal_state = FIELD_MODAL_NONE;
            return;
        }
        return;
    case FIELD_MODAL_ADDHERO:
        if (g_field_niki_addhero_state != 0)
        {
            result = func_801401F8(context_or_delay);
            switch (result)
            {
            case 1:
                g_field_player_records[1].head.bits.active = 0;
                g_field_player_records[1].head.bits.selected = ((FieldModalSaveView*)g_pad_ctx)->second_character.bytes[0] & 1;
                g_field_player_records[1].companion_type = 0;
                field_activate_actor_resource_slot(-2, 0, 0);
                func_80084240();
                g_field_niki_addhero_state = 0;
                g_field_modal_state = FIELD_MODAL_NONE;
                return;
            case 3:
                func_80084240();
                g_field_niki_addhero_state = 0;
                g_field_modal_state = FIELD_MODAL_NONE;
                return;
            case 2:
                field_release_actor_resource_slot(0);
                func_80084240();
                g_field_niki_addhero_state = 0;
                g_field_modal_state = FIELD_MODAL_NONE;
                return;
            }
        }
        return;
    case FIELD_MODAL_EMPTY_SHOP:
        if (g_pad_input & (PADRright | PADi))
        {
            field_text_reset_windows();
            index_or_zero = 0;
            g_field_modal_state = FIELD_MODAL_NONE;
            g_pad_input = 0;
            break;
        }
        field_text_reset_scratch();
        field_draw_empty_shop_notice((void*)context_or_delay);
        field_text_upload_immediate_cache();
        return;
    case FIELD_MODAL_DUEL_INTRO:
        if (field_draw_duel_intro((void*)context_or_delay) == 0)
        {
            return;
        }
        func_800A39A8(0, 0x80, 0, 3);
        field_text_reset_windows();
        index_or_zero = 0;
        g_field_modal_state = FIELD_MODAL_NONE;
        g_pad_input = 0;
        break;
    case FIELD_MODAL_DUEL_RESULT:
        if (field_draw_duel_result((void*)context_or_delay) == 0 || func_800B0888() != 0)
        {
            return;
        }
        func_800A7384();
        field_text_reset_windows();
        index_or_zero = 0;
        g_field_modal_state = FIELD_MODAL_NONE;
        g_pad_input = 0;
        break;
    default:
        return;
    }
    g_field_primary_held_buttons = field_read_controller_buttons(index_or_zero);
    context_or_delay = FIELD_INPUT_REPEAT_DELAY;
    g_field_primary_repeat_delay = context_or_delay;
    g_pad_input_inject = 0;
    g_field_secondary_held_buttons = field_read_controller_buttons(1);
    g_field_secondary_repeat_delay = context_or_delay;
    g_field_buffered_input = 0;
    field_restore_fade_target();
}

/**
 * @brief Open the empty-inventory notice with a red fade.
 * @param hidden Nonzero suppresses the notice text.
 */
void field_begin_empty_shop_notice(s32 hidden)
{
    g_field_modal_state = FIELD_MODAL_EMPTY_SHOP;
    field_set_fade_target_only(0xC0, 0x80, 0x80, 8);
    func_800A3938(0xC7, 0x80);
    g_field_shop_notice_hidden = hidden;
}

/**
 * @brief Draw the empty-inventory notice unless its text is suppressed.
 * @param context Field drawing context and packet cursor.
 */
void field_draw_empty_shop_notice(FieldModalDrawContext* context)
{
    s32 packet_cursor;
    s32 low;
    s32 offset;
    u8* base;

    packet_cursor = context->packet_cursor;
    if (g_field_shop_notice_hidden == 0)
    {
        low = D_800EC400.low;
        offset = (D_800EC400.high << 8) + (s32)(base = (u8*)&D_800EC400 - 0x3C);
        packet_cursor = (s32)func_800A88A0((SPRT*)packet_cursor, (s32*)context, (u8*)(low + offset), 4, 0xA0, 0x68, 2);
    }
    context->packet_cursor = packet_cursor;
}

/**
 * @brief Start the duel introduction panels sliding in from off-screen.
 */
void field_begin_duel_intro(void)
{
    g_field_modal_state = FIELD_MODAL_DUEL_INTRO;
    field_set_fade_target_only(0xC0, 0xC0, 0xC0, 8);
    func_800AE9E0();
    func_800A3938(0x125, 0x80);
    g_field_duel_panel_phase = FIELD_DUEL_SLIDE_IN;
    g_field_duel_panel_hold_frames = 0;
    g_field_duel_panel_offset = FIELD_DUEL_PANEL_START_OFFSET;
}

/**
 * @brief Select the duel winner, update win/loss records, and start the result panel.
 */
void field_begin_duel_result(void)
{
    g_field_modal_state = FIELD_MODAL_DUEL_RESULT;
    field_set_fade_target_only(0xC0, 0xC0, 0xC0, 8);
    func_800AE9E0();
    func_800A3938(0x126, 0x80);
    g_field_duel_panel_phase = FIELD_DUEL_SLIDE_IN;
    g_field_duel_panel_hold_frames = 0;
    g_field_duel_panel_offset = FIELD_DUEL_PANEL_START_OFFSET;

    if ((D_800FDF79 & 0x7F) == 0x1D)
    {
        g_field_duel_winner = 1;
        ((FieldModalSaveView*)g_pad_ctx)->duel_losses = ((FieldModalSaveView*)g_pad_ctx)->duel_losses + 1;
        if ((u32)(((FieldModalSaveView*)g_pad_ctx)->second_character.bytes[0] & 0x7F) < 2)
        {
            ((FieldModalSaveView*)g_pad_ctx)->second_duel_wins = ((FieldModalSaveView*)g_pad_ctx)->second_duel_wins + 1;
        }
    }
    else
    {
        g_field_duel_winner = 0;
        ((FieldModalSaveView*)g_pad_ctx)->duel_wins = ((FieldModalSaveView*)g_pad_ctx)->duel_wins + 1;
        if ((u32)(((FieldModalSaveView*)g_pad_ctx)->second_character.bytes[0] & 0x7F) < 2)
        {
            ((FieldModalSaveView*)g_pad_ctx)->second_duel_losses = ((FieldModalSaveView*)g_pad_ctx)->second_duel_losses + 1;
        }
    }
}

/**
 * @brief Animate and draw the opposing players and their duel records.
 * @param context Field drawing context and packet cursor.
 * @return Nonzero after the panels have slid out.
 */
s32 field_draw_duel_intro(FieldModalDrawContext* context)
{
    u8 record_text[0x38];
    u8 loss_text[0x38];
    s32 packet_cursor;
    void* draw_context;

    packet_cursor = context->packet_cursor;
    draw_context = context;
    switch (g_field_duel_panel_phase)
    {
    case FIELD_DUEL_SLIDE_IN:
        g_field_duel_panel_offset /= 2;
        if (g_field_duel_panel_offset == 0)
        {
            g_field_duel_panel_phase = FIELD_DUEL_HOLD;
            g_field_duel_panel_hold_frames = FIELD_DUEL_PANEL_HOLD_FRAMES;
        }
        break;
    case FIELD_DUEL_HOLD:
        g_field_duel_panel_hold_frames--;
        if (g_field_duel_panel_hold_frames == 0)
        {
            func_800A3938(0x127, 0x80);
            g_field_duel_panel_phase = FIELD_DUEL_SLIDE_OUT;
            g_field_duel_panel_offset = -1;
        }
        break;
    case FIELD_DUEL_SLIDE_OUT:
        g_field_duel_panel_offset *= 2;
        if (g_field_duel_panel_offset < -0x64)
        {
            g_field_duel_panel_phase = FIELD_DUEL_FINISHED;
        }
        break;
    case FIELD_DUEL_FINISHED:
        return 1;
    }

    packet_cursor = func_800AEAC0(packet_cursor, draw_context, 0, g_field_duel_panel_offset + 0x32, 0x22, 1);
    packet_cursor = func_800AF950(packet_cursor, draw_context, &((FieldModalSaveView*)g_pad_ctx)->name[0], 4, g_field_duel_panel_offset + 0x6C, 0x32, 0, 5,
                                  0x180, 0x180, -4, DRAW_FLAG);

    FORMAT_SIGNED(record_text, ((FieldModalSaveView*)g_pad_ctx)->duel_wins);
    STR_CAT_ENTRY(record_text, D_800EC408, 0x44);
    FORMAT_SIGNED(loss_text, ((FieldModalSaveView*)g_pad_ctx)->duel_losses);
    STR_CAT(record_text, loss_text);
    STR_CAT_ENTRY(record_text, D_800EC40A, 0x46);

    packet_cursor = func_800AF950(packet_cursor, draw_context, record_text, 4, g_field_duel_panel_offset + 0x6C, 0x42, 0, 6, 0x180, 0x180, -4, DRAW_FLAG);
    {
        s32 low = D_800EC406.low;
        s32 offset = (D_800EC406.high << 8) + (s32)((u8*)&D_800EC406 - 0x42);
        packet_cursor = func_800AF950(packet_cursor, draw_context, (u8*)(low + offset), 4, 0xA0, 0x64, 2, 7, 0x180, 0x180, -4, DRAW_FLAG);
    }
    packet_cursor = func_800AEAC0(packet_cursor, draw_context, 1, 0xDE - g_field_duel_panel_offset, 0x86, 0);
    packet_cursor = func_800AF950(packet_cursor, draw_context, &((FieldModalSaveView*)g_pad_ctx)->second_name[0], 4, 0xD4 - g_field_duel_panel_offset, 0x96, 1,
                                  8, 0x180, 0x180, -4, DRAW_FLAG);

    if ((u32)(((FieldModalSaveView*)g_pad_ctx)->second_character.bytes[0] & 0x7F) < 2)
    {
        FORMAT_SIGNED(record_text, ((FieldModalSaveView*)g_pad_ctx)->second_duel_wins);
        STR_CAT_ENTRY(record_text, D_800EC408, 0x44);
        FORMAT_SIGNED(loss_text, ((FieldModalSaveView*)g_pad_ctx)->second_duel_losses);
        STR_CAT(record_text, loss_text);
        STR_CAT_ENTRY(record_text, D_800EC40A, 0x46);
        packet_cursor = func_800AF950(packet_cursor, draw_context, record_text, 4, 0xD4 - g_field_duel_panel_offset, 0xA6, 1, 9, 0x180, 0x180, -4, DRAW_FLAG);
    }

    context->packet_cursor = packet_cursor;
    return 0;
}

/**
 * @brief Animate and draw the winner and their duel record.
 * @param context Field drawing context and packet cursor.
 * @return Nonzero after the panel has slid out.
 */
s32 field_draw_duel_result(FieldModalDrawContext* context)
{
    u8 record_text[0x38];
    u8 loss_text[0x38];
    s32 packet_cursor;
    void* draw_context;
    FieldModalSaveView* winner;

    packet_cursor = context->packet_cursor;
    draw_context = context;
    switch (g_field_duel_panel_phase)
    {
    case FIELD_DUEL_SLIDE_IN:
        g_field_duel_panel_offset /= 2;
        if (g_field_duel_panel_offset == 0)
        {
            g_field_duel_panel_phase = FIELD_DUEL_HOLD;
            g_field_duel_panel_hold_frames = FIELD_DUEL_PANEL_HOLD_FRAMES;
        }
        break;
    case FIELD_DUEL_HOLD:
        g_field_duel_panel_hold_frames--;
        if (g_field_duel_panel_hold_frames == 0)
        {
            func_800A3938(0x127, 0x80);
            g_field_duel_panel_phase = FIELD_DUEL_SLIDE_OUT;
            g_field_duel_panel_offset = -1;
        }
        break;
    case FIELD_DUEL_SLIDE_OUT:
        g_field_duel_panel_offset *= 2;
        if (g_field_duel_panel_offset < -0x64)
        {
            g_field_duel_panel_phase = FIELD_DUEL_FINISHED;
        }
        break;
    case FIELD_DUEL_FINISHED:
        return 1;
    }

    {
        s32 low = D_800EC40C.low;
        s32 offset = (D_800EC40C.high << 8) + (s32)((u8*)&D_800EC40C - 0x48);
        packet_cursor = func_800AF950(packet_cursor, draw_context, (u8*)(low + offset), 4, 0xA0, 0x34, 2, 5, 0x200, 0x200, -4, DRAW_FLAG);
    }
    packet_cursor = func_800AEAC0(packet_cursor, draw_context, g_field_duel_winner, g_field_duel_panel_offset + 0x32, 0x54, 1);
    packet_cursor = func_800AF950(packet_cursor, draw_context, g_pad_ctx + (g_field_duel_winner * FIELD_SAVED_CHARACTER_STRIDE + 0x5F0), 4,
                                  g_field_duel_panel_offset + 0x6C, 0x64, 0, 6, 0x1C0, 0x1C0, -4, DRAW_FLAG);

    winner = (FieldModalSaveView*)(g_pad_ctx + g_field_duel_winner * FIELD_SAVED_CHARACTER_STRIDE);
    if ((u32)(winner->character_kind & 0x7F) < 2)
    {
        FieldModalSaveView* winner_record;

        FORMAT_SIGNED(record_text, winner->duel_wins);
        STR_CAT_ENTRY(record_text, D_800EC408, 0x44);
        winner_record = (FieldModalSaveView*)(g_pad_ctx + g_field_duel_winner * FIELD_SAVED_CHARACTER_STRIDE);
        FORMAT_SIGNED(loss_text, winner_record->duel_losses);
        STR_CAT(record_text, loss_text);
        STR_CAT_ENTRY(record_text, D_800EC40A, 0x46);
        packet_cursor = func_800AF950(packet_cursor, draw_context, record_text, 4, g_field_duel_panel_offset + 0x8C, 0x84, 0, 7, 0x180, 0x180, -4, DRAW_FLAG);
    }

    context->packet_cursor = packet_cursor;
    return 0;
}
