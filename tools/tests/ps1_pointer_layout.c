/** @file ps1_pointer_layout.c
 * @brief Compile the real shared layouts for each supported native target.
 */
#include "akao_driver.h"
#include "field_animation.h"
#if defined(TEST_ACTOR_TABLES)
#include "field_actor_tables.h"
#else
#include "field_effect_types.h"
#endif
#include "field_runtime.h"
#include "field_script.h"
#include "field_text.h"
#include "field_records.h"
#include "field_scene_internal.h"
#include "wmap_frame_render.h"
#include "wmap_land_layout.h"
#include "wmap_map_display.h"
#include "wmap_sprite_render.h"
#include "wsel.h"

#define CHECK_SIZE(type, bytes) _Static_assert(sizeof(type) == (bytes), #type " size")
#define CHECK_OFFSET(type, member, bytes) _Static_assert(__builtin_offsetof(type, member) == (bytes), #type "." #member " offset")

CHECK_SIZE(void_ptr, 4);
CHECK_SIZE(u8_ptr, 4);
CHECK_SIZE(s8_ptr, 4);
CHECK_SIZE(u16_ptr, 4);
CHECK_SIZE(s16_ptr, 4);
CHECK_SIZE(u32_ptr, 4);
CHECK_SIZE(s32_ptr, 4);
CHECK_SIZE(u_long_ptr, 4);
CHECK_SIZE(void*, 8);
CHECK_SIZE(g_field_script, 4);
CHECK_SIZE(g_field_text_saved_configs, 4);
CHECK_SIZE(g_wmap_current_frame, 4);
CHECK_SIZE(g_akao_pending_channels, 4);
CHECK_SIZE(g_akao_seq_channel0, 4);
CHECK_SIZE(Ps1Long, 4);
CHECK_SIZE(u_long, 4);
CHECK_SIZE(MATRIX, 32);
CHECK_OFFSET(MATRIX, t, 20);
CHECK_SIZE(VECTOR, 16);
CHECK_SIZE(AkaoCommandParam, 4);
CHECK_SIZE(AkaoChannelState, 0x118);
CHECK_OFFSET(AkaoChannelState, return_cursor, 0x14);
CHECK_SIZE(AkaoXaTracker, 0x5C);
CHECK_OFFSET(AkaoXaTracker, source, 0x2C);
CHECK_SIZE(FieldImageReq, 0x10);
CHECK_SIZE(FieldTextMacro, 8);
CHECK_SIZE(FieldScriptFrame, 0xC);
CHECK_SIZE(FieldScriptRecord, 0xC);
CHECK_SIZE(FieldScriptRecordState, 0x14);
CHECK_SIZE(FieldScriptContext, 0xC);
CHECK_OFFSET(FieldScriptRecordState, flags, 0xC);
CHECK_SIZE(FieldScriptState, 0x68);
CHECK_SIZE(FieldActorRecord, 0x94);
CHECK_SIZE(FieldItemStaging, 0x60);
CHECK_SIZE(FieldRuntimeContext, 0xF04);
CHECK_OFFSET(FieldRuntimeContext, actors, 0x430);
CHECK_OFFSET(FieldRuntimeContext, events, 0xD70);
CHECK_OFFSET(FieldRuntimeContext, trigger_table, 0xF00);
#if defined(TEST_ACTOR_TABLES)
CHECK_SIZE(FieldActorSlot, 0x244);
CHECK_OFFSET(FieldActorSlot, part_masks, 0x240);
CHECK_SIZE(FieldResourceEntry, 0x14);
#else
CHECK_SIZE(FieldActorState, 0x244);
#endif
CHECK_SIZE(FieldAnimDef, 0x18);
CHECK_OFFSET(FieldRenderHalf, primitive_cursor, 0x40B8);
CHECK_OFFSET(FieldAnimDef, data, 0x14);
CHECK_SIZE(FieldPartDefPtr, 4);
CHECK_SIZE(FieldPartDefTablePtr, 4);
CHECK_SIZE(FieldObjDefPtr, 4);
CHECK_SIZE(FieldMapObjectPtr, 4);
CHECK_SIZE(FieldMapObjectTablePtr, 4);
CHECK_SIZE(WmapFramePtr, 4);
CHECK_SIZE(WmapLandDisplay, 0x2C);
CHECK_SIZE(WmapCacheEntry, 0x14);
CHECK_SIZE(WmapSpriteActor, 0x2C);
CHECK_OFFSET(WselRenderBuffer, prim_cursor, 0x80B8);

/**
 * @brief Check the public text API accepts the stored render cursor's address.
 * @param frame Render buffer containing the four-byte cursor slot.
 * @param tags Ordering-table entries used by the text renderer.
 */
void ps1_pointer_cursor_call(FieldRenderHalf* frame, FieldOrderingTags* tags)
{
    field_text_update(&frame->primitive_cursor, tags, 1);
}

/**
 * @brief Exercise code generation for ordinary stored-pointer expressions.
 * @param script Script frame containing a four-byte bytecode cursor.
 * @param objects Table of four-byte object addresses.
 * @return The consumed opcode plus the selected object's width.
 */
u32 ps1_pointer_codegen(FieldScriptFrame* script, FieldMapObjectTablePtr objects)
{
    u8* cursor = script->pc;
    u32 opcode = *cursor++;
    script->pc = cursor;
    objects++;
    return opcode + objects[0]->width;
}
