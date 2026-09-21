#include "field_modal_runtime.h"
/**
 * @file field_input_text_session.c
 * @brief Controller repeat, saved names and inventory, and field actor labels.
 */

#include "field_text.h"
#include "field_effect_render_state.h"
#include "cdrom.h"
#include "saved_game.h"

#include "common.h"
#include "vector.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/libetc.h"

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

extern PadContext* g_pad_ctx;
extern FieldLabelPart g_field_object_parts[];
extern FieldInputActor g_field_actors[];
extern FieldLabelActorState g_field_object_states[];
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
extern FieldLabelPlayer g_field_player_records[];
extern u8 D_8010A028[];
extern u8 g_field_resource_actions[];

/* CD-error status string descriptors (field_draw_cd_error_text). */
extern FieldTextOffset D_800EC3D2;
extern FieldTextOffset D_800EC3D4;

/* Input repeat state. */
extern s32 g_field_modal_state;
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

extern u16 g_music_track_index;
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
 */
void field_draw_actor_labels(void* context)
{
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
    s32 button_mask_or_y_offset;
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

    label_ot = (s32)((FieldLabelRenderContext*)context)->ordering_table;
    context_slot = &context;
    index = 0;
    default_label_offset = D_800EC3E0;
    label_low = D_800EC3E0;
    object_record = g_field_player_records;
    action_offset = index;
    primitive = ((FieldLabelRenderContext*)*context_slot)->primitive_cursor;
    custom_text_offset = 0x5F0;
    pad_sample = FIELD_CONTROLLER_SAMPLES;
    pad_offset = 0;
    do
    {
        if ((object_record->flags & 1) && ((u8)pad_sample->device_type < 0xFEU))
        {
            text_base = (s32)D_800EC3C4;
            button_mask_or_y_offset = 1;
            button_index = 0;
            local_pad_offset = pad_offset;
            raw_buttons = pad_sample->buttons;
            local_text_offset = custom_text_offset;
            swapped_buttons = ((raw_buttons << 8) & 0xFF00) | (raw_buttons >> 8);
            swapped_buttons = (((u32)(swapped_buttons & 0x40) >> 1) | ((swapped_buttons & 0x20) * 2) | ((u32)(swapped_buttons & 0x80) >> 3) |
                               ((swapped_buttons & 0x10) * 8) | (swapped_buttons & 0xFF0F));
            label_base = text_base;
            do
            {
                if (swapped_buttons & button_mask_or_y_offset)
                {
                    swapped_buttons = (s32)g_pad_ctx + local_pad_offset;
                    action_slot = ((FieldSavedInputMap*)swapped_buttons)->button_actions[g_field_hint_button_map[button_index]];
                    switch (action_slot)
                    {
                    case 2:
                    case 3:
                        text_high_or_offset = (s32)D_8010A028 + action_offset;
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
                            text_offset = default_label_offset[1] << 8;
                            text_high_or_offset = text_offset + label_base;
                            label_value = *label_low;
                            text_address = text_high_or_offset + label_value;
                        }
                        else
                        {
                            label_descriptor = D_800EC3E6;
                            text_high_or_offset = label_descriptor[1];
                            label_value = D_800EC3E6[0];
                            text_high_or_offset <<= 8;
                            text_high_or_offset += label_base;
                            text_address = text_high_or_offset + label_value;
                        }

                        break;
                    case 1:
                        if (((((FieldSavedInputMap*)swapped_buttons)->actions[0] & 0x7F) == 2) &&
                            ((secondary_action_alt = ((FieldSavedInputMap*)swapped_buttons)->actions[1], (secondary_action_alt == 5)) ||
                             (secondary_action_alt == 8)))
                        {
                            text_offset = default_label_offset[1] << 8;
                            text_high_or_offset = text_offset + label_base;
                            label_value = *label_low;
                            text_address = text_high_or_offset + label_value;
                        }
                        else
                        {
                            text_high_or_offset = D_800EC3E8[1];
                            text_high_or_offset <<= 8;
                            text_high_or_offset += label_base;
                            label_value = D_800EC3E8[0];
                            text_address = text_high_or_offset + label_value;
                        }
                        break;
                    default:
                        swapped_buttons = ((FieldSavedInputMap*)((u8*)g_pad_ctx + local_pad_offset))->actions[action_slot];
                        if (swapped_buttons == 0xFF)
                        {
                            text_offset = default_label_offset[1] << 8;
                            text_high_or_offset = text_offset + label_base;
                            label_value = *label_low;
                            text_address = text_high_or_offset + label_value;
                        }
                        else
                        {
                            if (swapped_buttons & 0x80)
                            {
                                text_offset = swapped_buttons & 0xFF7F;
                                label_value = (s32)g_pad_ctx + local_text_offset;
                                text_high_or_offset = (text_offset << 6) + 0x150;
                                text_address = text_high_or_offset + label_value;
                            }
                            else
                            {
                                actor_id = action_slot * sizeof(FieldLabelAction);
                                label_value = (s32)g_field_resource_actions + action_offset;
                                swapped_buttons = ((FieldLabelAction*)(actor_id + label_value))->text_index;
                                swapped_buttons &= 0x7FFF;
                                label_value = object_record->kind;
                                swapped_buttons += label_value * 0x18;

                                label_value = (s32)D_800ED064;
                                text_high_or_offset = ((u16*)label_value)[swapped_buttons];
                                text_address = text_high_or_offset + label_value;
                            }
                        }

                        break;
                    }
                    button_mask_or_y_offset = index << 5;
                    point.vx = 0x60;
                    point.vy = button_mask_or_y_offset + 0x3C;

                    primitive = (s32)func_800A88A0(field_emit_actor_portrait((SPRT*)primitive, (u32*)label_ot, index, (u32*)&point), (s32*)label_ot, (u8*)text_address, 4,
                                                   0x80, button_mask_or_y_offset + 0x40, 0x80);
                    break;
                }
                else
                {
                    button_index += 1;
                    button_mask_or_y_offset *= 2;
                }
            } while (button_index < 8);
        }
        object_record++;
        action_offset += 0x190;
        pad_sample++;
        index += 1;
        custom_text_offset += 0x250;
        pad_offset += 0x250;
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
