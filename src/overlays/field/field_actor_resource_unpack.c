/**
 * @file field_actor_resource_unpack.c
 * @brief Actor resource request queue and the unpacker for loaded actor resources.
 */
#include "cdrom.h"
#include "common.h"
#include "field_actor_palette.h"
#include "field_effect_types.h"
#include "field_mesh.h"
#include "sdk/memory.h"
#include "sdk/libgpu.h"

/**
 * @brief memcpy with a signed byte count.
 * @note sdk/memory.h leaves memcpy unprototyped; this prototype matches the original calls.
 * @param dst Destination buffer.
 * @param src Source buffer.
 * @param size Byte count.
 * @return @p dst.
 */
void *memcpy(void *dst, const void *src, s32 size);

/** @brief Proximity probe submitted to func_8005B368. */
typedef struct
{
    s32 x;      /**< World X of the probe origin. */
    s32 y;      /**< World Y of the probe origin. */
    s32 z;      /**< World Z of the probe origin. */
    u16 unkC;   /**< TODO: meaning unknown (always 8 here). */
    s16 unkE;   /**< TODO: meaning unknown (always 0x10 here). */
    u16 unk10;  /**< TODO: meaning unknown (always 5 here). */
} FieldSceneProbe;

/** @brief Byte-aligned 18-byte section copied from the packed resource stream. */
typedef struct
{
    s8 bytes[18];
} Copy18;

/** @brief Byte-aligned 10-byte section copied from the packed resource stream. */
typedef struct
{
    s8 bytes[10];
} Copy10;

/** @brief Byte-aligned 28-byte section copied from the packed resource stream. */
typedef struct
{
    s8 bytes[28];
} Copy28;

/** @brief Byte-aligned 6-byte section copied from the packed resource stream. */
typedef struct
{
    s8 bytes[6];
} Copy6;

/** @brief Six-byte link records unpacked into the actor (offset 0x4). */
#define FIELD_ACTOR_RECORDS(actor) (*(u8 **)&(actor)->pad4[0])
/** @brief Halfword record index table unpacked into the actor (offset 0x8). */
#define FIELD_ACTOR_RECORD_INDICES(actor) (*(u8 **)&(actor)->pad4[4])
/** @brief Optional resource sections @p i (0 or 1) of the actor (offset 0x1C). */
#define FIELD_ACTOR_OPTIONAL_DATA(actor, i) (((u8 **)(actor)->pad1C)[i])
/** @brief Section flags halfword at offset 0xC of an animation header. */
#define FIELD_ANIMATION_FLAGS(anim) (*(u16 *)&(anim)->pad0[0xC])

/** @brief Six-byte link record in the actor's record table. */
typedef union
{
    /** @brief Byte and halfword fields of the record. */
    struct
    {
        u8 x;
        u8 y;
        u16 h2;
        u16 h4;
    } bytes;
    u16 head; /**< First halfword; bit 15 is the record flag. */
} FieldActorLinkRecord;

/**
 * @brief Link record at byte offset @p off of the actor's record table.
 * @note The sum is written offset-first so the addu emits the offset operand first.
 */
#define FIELD_ACTOR_LINK_RECORD(a, off) ((FieldActorLinkRecord *)((off) + (u32)FIELD_ACTOR_RECORDS(a)))

extern s16 D_800FDF82;
extern s32 g_field_scene_contact_latched;
extern s32 g_field_scene_request_pending;
extern u8 *D_8010D034;
extern u8 *D_8010D038;
extern s32 D_8010D030;
extern s32 D_8010D040[];
extern s32 D_8010D080;
extern FieldMeshTexturePart D_80105798[3][9];
extern u8 D_8011BF00[];
extern s16 g_field_texture_slot_flags[];

s32 func_8005B368(FieldSceneProbe *probe);
void func_800B22F0(s32 value, s32 entry);
void *func_8009CA54(u8 *pool, s32 size, s32 tag);
void func_8009AE38(u8 *resource, s32 slot);

/**
 * @brief Probe for a nearby scene actor and raise a pending scene request.
 * @param position World-space position used as the probe origin.
 */
void func_8009A2A4(Vec3i *position)
{
    FieldSceneProbe probe;
    s32 hit;

    if (D_800FDF82 == 0)
    {
        probe.x = position->x;
        probe.y = position->y;
        probe.z = position->z;
        probe.unkC = 8;
        probe.unkE = 0x10;
        probe.unk10 = 5;
        hit = func_8005B368(&probe);
        if (hit != -1)
        {
            if (g_field_scene_request_pending == 0)
            {
                if (g_field_scene_contact_latched == 0)
                {
                    func_800B22F0(0, hit | 0x8000);
                }
                g_field_scene_contact_latched = 1;
                return;
            }
            g_field_scene_contact_latched = 0;
            return;
        }
        g_field_scene_contact_latched = 0;
    }
}

/**
 * @brief Enqueue the default resource request via func_8009A3A0.
 * @note The call is unprototyped and passes whatever is left in $a0.
 */
void func_8009A364(void)
{
    func_8009A3A0();
}

/**
 * @brief Clear the pending resource queue count.
 */
void func_8009A384(void)
{
    D_8010D030 = 0;
}

/**
 * @brief Read the id of the resource read issued last.
 * @return Request id from D_8010D080, or -1 when the queue was empty.
 */
s32 func_8009A390(void)
{
    return D_8010D080;
}

/**
 * @brief Append a request id to the pending resource queue.
 * @param request_id Request id to enqueue.
 * @return 0 when queued, 1 when the 16-entry queue is full.
 */
s32 func_8009A3A0(s32 request_id)
{
    s32 count;

    count = D_8010D030;
    if (count < 0x10)
    {
        D_8010D040[count] = request_id;
        D_8010D030 = count + 1;
        return 0;
    }
    return 1;
}

/**
 * @brief Issue the next queued resource read and shift the queue down.
 */
void func_8009A3E8(void)
{
    s32 count;
    s32 i;

    if (D_8010D030 == 0)
    {
        D_8010D080 = -1;
        return;
    }

    if (cdrom_queue_read((u16)D_8010D040[0], D_8010D038) >= 0)
    {
        i = 0;
        D_8010D080 = D_8010D040[0];
        count = D_8010D030 - 1;
        if (count > 0)
        {
            do
            {
                D_8010D040[i] = D_8010D040[i + 1];
                i++;
            } while (i < count);
        }
        D_8010D030 -= 1;
    }
}

/**
 * @brief Kick off unpacking of the head resource buffer for the given group.
 * @param group Allocation group passed through to func_8009CB64.
 * @note func_8009CB64 is called unprototyped.
 */
void func_8009A4A0(s32 group)
{
    func_8009CB64(D_8010D034, group);
}

/**
 * @brief Unpack a loaded actor resource into allocated runtime sections.
 * @param group Allocation group; values at least three use allocation tag two.
 * @param actor Actor slot receiving resource pointers, counts, and header state.
 * @note Resource offsets, alignment, optional sections, and record remapping follow the packed format.
 */
void func_8009A4CC(s32 group, FieldActorState *actor)
{
    s32 part_count;
    u8 *resource_base;
    s32 texture_total;
    s32 direct_group;
    /** @brief Six-byte remapping record with word-sized bitfield writes. */
    union
    {
        u32 word;
        /** @brief Low-byte remapping coordinate and flag. */
        struct
        {
            u32 x : 7;
            u32 flag : 1;
            u32 unused : 24;
        } bits;
        /** @brief Byte and halfword view of the remapped record. */
        struct
        {
            u8 b0;
            u8 b1;
            u16 h2;
            u16 h4;
        } h;
    } packed;
    s32 allocation_size;
    s32 has_payload;
    s32 payload_size;
    s32 record_bytes;
    s32 index_bytes;
    s32 entry_bytes;
    s32 optional_bytes;
    s32 payload_tag;
    s32 header_tag;
    s32 optional_tag;
    s32 part_tag;
    s32 vector_tag;
    s32 byte_tag;
    s32 texture_tag;
    s32 record_tag;
    s32 index_tag;
    s32 entry_tag;
    s32 row;
    s32 texture_height;
    s32 item_index;
    s32 record_offset;
    s32 optional_count;
    s32 has_optional;
    u8 *cursor;
    s32 part_index;
    s32 table_group;
    s32 column;
    u8 *indices;
    u8 *entries;
    FieldActorAnimationDef *header;
    u8 *records;
    /** @brief Output cursor shared by the vector tables and the face bytes. */
    union
    {
        u16 *half;
        u8 *byte;
    } output;
    u16 *pixel_input;
    u8 *row_input;
    u16 *pixel_output;
    u8 *header_cursor;
    FieldMeshResource *part;
    FieldMeshResource *part_fields;
    s32 payload_offset;
    u16 component;
    u16 pixel;
    s32 byte_index;
    FieldMeshTexturePart *texture_record;
    s32 record_count;
    u8 byte_value;
    u8 texture_count;
    u8 *payload_source;
    u8 **header_pool;
    u8 **optional_pool;
    u8 *shared_base;

    header_cursor = D_8010D038;
    cursor = header_cursor;
    resource_base = cursor;
    if (*(u16 *)cursor != 0)
    {
        func_8009AE38(cursor + *(u16 *)cursor, group);
    }
    cursor = header_cursor + 2;
    has_payload = *(u16 *)cursor;
    cursor = (u8 *)((u32)(cursor + 5) & ~3);
    if (has_payload != 0)
    {
        /* The block boundary keeps the group reload after this halfword load. */
        do
        {
            payload_offset = *(u16 *)cursor;
        } while (0);
        payload_tag = group;
        payload_source = header_cursor + payload_offset;
        payload_size = *(u16 *)(cursor + 2) - payload_offset;
        if (payload_tag >= 3)
        {
            payload_tag = 2;
        }
        header_cursor = func_8009CA54(D_8010D034, payload_size, payload_tag);
        cursor += 2;
        memcpy(header_cursor, payload_source, payload_size);
        actor->track_data = header_cursor;
    }
    cursor += 2;
    part_count = *(u16 *)cursor;
    cursor += 2;
    if (part_count != 0)
    {
        part_tag = group;
        if (part_tag >= 3)
        {
            part_tag = 2;
        }
        part = func_8009CA54(D_8010D034, 0x48, part_tag);
        actor->mesh_data = (u8 *)part;
    }
    part_index = 0;
    texture_total = 0;
    if (part_count != 0)
    {
        part_fields = part; /* A second cursor over the same mesh record. */
        direct_group = group < 3;
        do
        {
            header_cursor = resource_base + *(u16 *)cursor;
            part->face_count = *(u16 *)header_cursor;
            header_cursor += 4;
            part_fields->texture_part_count = *header_cursor;
            header_cursor += 4;
            allocation_size = part->face_count * 0x28;
            vector_tag = group;
            if (direct_group == 0)
            {
                vector_tag = 2;
            }
            item_index = 0;
            output.half = func_8009CA54(D_8010D034, allocation_size, vector_tag);
            part_fields->vertices = (SVECTOR *)output.half;
            if ((part->face_count * 3) != 0)
            {
                do
                {
                    component = *(u16 *)header_cursor;
                    header_cursor += 2;
                    item_index++;
                    *output.half++ = component;
                    *output.half++ = *(u16 *)header_cursor;
                    header_cursor += 2;
                    *output.half = *(u16 *)header_cursor;
                    header_cursor += 2;
                    output.half += 2;
                } while (item_index < (part->face_count * 3));
            }
            part_fields->normals = (SVECTOR *)output.half;
            item_index = 0;
            if (part->face_count != 0)
            {
                do
                {
                    component = *(u16 *)header_cursor;
                    header_cursor += 2;
                    item_index++;
                    *output.half++ = component;
                    *output.half++ = *(u16 *)header_cursor;
                    header_cursor += 2;
                    *output.half = *(u16 *)header_cursor;
                    header_cursor += 2;
                    output.half += 2;
                } while (item_index < (s32)part->face_count);
            }
            part_fields->offsets = (SVECTOR *)output.half;
            item_index = 0;
            if (part->face_count != 0)
            {
                do
                {
                    component = *(u16 *)header_cursor;
                    header_cursor += 2;
                    item_index++;
                    *output.half++ = component;
                    *output.half++ = *(u16 *)header_cursor;
                    header_cursor += 2;
                    *output.half = *(u16 *)header_cursor;
                    header_cursor += 2;
                    output.half += 2;
                } while (item_index < (s32)part->face_count);
            }
            allocation_size = part->face_count * 0x10;
            byte_tag = group;
            if (direct_group == 0)
            {
                byte_tag = 2;
            }
            item_index = 0;
            output.byte = func_8009CA54(D_8010D034, allocation_size, byte_tag);
            part_fields->faces = output.byte;
            if (part->face_count != 0)
            {
                do
                {
                    byte_index = 0;
                    do
                    {
                        byte_index += 1;
                        byte_value = *header_cursor;
                        header_cursor += 1;
                        *output.byte = byte_value;
                        output.byte += 1;
                    } while ((u32)byte_index < 0x10U);
                    item_index += 1;
                } while (item_index < (s32)part->face_count);
            }
            table_group = group;
            if (direct_group == 0)
            {
                table_group = 2;
            }
            texture_record = part_fields->texture_parts = &D_80105798[table_group][texture_total];
            byte_index = 0;
            if (part_fields->texture_part_count != 0)
            {
                do
                {
                    texture_record->x = *header_cursor++;
                    texture_record->y = *header_cursor++;
                    texture_record->width = *header_cursor++;
                    texture_record->height = *header_cursor++;
                    allocation_size = texture_record->width * texture_record->height * 2;
                    texture_tag = group;
                    if (direct_group == 0)
                    {
                        texture_tag = 2;
                    }
                    pixel_output = func_8009CA54(D_8010D034, allocation_size, texture_tag);
                    row = 0;
                    texture_record->pixels = pixel_output;
                    texture_height = texture_record->height;
                    row_input = resource_base + *(u16 *)resource_base + 0x220;
                    row_input = row_input + texture_record->x * 2 + (texture_record->y << 7);
                    do
                    {
                        if (texture_height != 0)
                        {
                            do
                            {
                                pixel_input = (u16 *)row_input;
                                column = 0;
                                if (texture_record->width != 0)
                                {
                                    do
                                    {
                                        column += 1;
                                        pixel = *pixel_input;
                                        pixel_input++;
                                        *pixel_output = pixel;
                                        pixel_output++;
                                    } while (column < (s32)texture_record->width);
                                }
                                row += 1;
                                row_input += 0x80;
                            } while (row < (s32)texture_record->height);
                        }
                        byte_index += 1;
                            texture_record++;
                    } while (0); /* Its loop note sets the s0/s1 allocation. */
                } while (byte_index < (s32)part_fields->texture_part_count);
            }
            part_index += 1;
            cursor += 2;
            texture_count = part_fields->texture_part_count;
            texture_total += texture_count;
            part_fields++;
            part++;
        } while (part_index < part_count);
    }
    header_pool = &D_8010D034;
    part_index = *(u16 *)cursor;
    cursor += 2;
    record_count = *cursor++;
    record_tag = group;
    record_bytes = record_count * 6;
    if (record_tag >= 3)
    {
        record_tag = 2;
    }
    records = func_8009CA54(*header_pool, record_bytes, record_tag);
    FIELD_ACTOR_RECORDS(actor) = records;
    memcpy(records, cursor, record_bytes);
    cursor += record_bytes;
    byte_index = *cursor++;
    index_bytes = byte_index * 2;
    index_tag = group;
    if (group >= 3)
    {
        index_tag = 2;
    }
    indices = func_8009CA54(*header_pool, index_bytes, index_tag);
    FIELD_ACTOR_RECORD_INDICES(actor) = indices;
    memcpy(indices, cursor, index_bytes);
    cursor += index_bytes;
    byte_index = *cursor;
    optional_count = byte_index >> 6;
    if (optional_count >= 3)
    {
        optional_count = 0;
    }
    byte_index = byte_index & 0x3F;
    cursor++;
    actor->part_count = byte_index;
    entry_bytes = byte_index * 0x48;
    entry_tag = group;
    if (group >= 3)
    {
        entry_tag = 2;
    }
    entries = func_8009CA54(*header_pool, entry_bytes, entry_tag);
    actor->parts = (FieldActorPartDef *)entries;
    memcpy(entries, cursor, entry_bytes);
    cursor += entry_bytes;
    actor->animation_index = 0;
    header_tag = group;
    if (group >= 3)
    {
        header_tag = 2;
    }
    header = func_8009CA54(*header_pool, 0x5A, header_tag);
    actor->animation = header;
    header->unknown_0x12 = part_index;
    header_cursor = (u8 *)actor->animation;
    actor->animations = (FieldActorAnimationDef *)header_cursor;
    *(Copy18 *)header_cursor = *(Copy18 *)cursor;
    header_cursor += 0x12;
    cursor += 0x12;
    if (FIELD_ANIMATION_FLAGS(actor->animation) & 0x8000)
    {
        *(Copy10 *)header_cursor = *(Copy10 *)cursor;
        header_cursor += 0xA;
        cursor += 0xA;
    }
    if ((actor->parts->orientation_flags.bytes.low >> 4) < 2)
    {
        part_index = 0;
        if (record_count != 0)
        {
            record_offset = 0;
            do
            {
                packed.bits.x = FIELD_ACTOR_LINK_RECORD(actor, record_offset)->bytes.x & 0x7F;
                packed.bits.flag = FIELD_ACTOR_LINK_RECORD(actor, record_offset)->head >> 15;
                packed.h.b1 = (FIELD_ACTOR_LINK_RECORD(actor, record_offset)->head >> 8) & 0x7F;
                packed.h.h4 = FIELD_ACTOR_LINK_RECORD(actor, record_offset)->bytes.h4;
                packed.h.h2 = FIELD_ACTOR_LINK_RECORD(actor, record_offset)->bytes.h2;
                bcopy((u8 *)&packed, FIELD_ACTOR_RECORDS(actor) + record_offset, 6);
                part_index += 1;
                record_offset += 6;
            } while (part_index < (s32)record_count);
        }
    }
    actor->unknown_0x222 = actor->animation->unknown_0x12;
    if (FIELD_ANIMATION_FLAGS(actor->animations) & 0x800)
    {
        *(Copy28 *)header_cursor = *(Copy28 *)cursor;
        header_cursor += 0x1C;
        cursor += 0x1C;
        *(Copy28 *)header_cursor = *(Copy28 *)cursor;
        cursor += 0x1C;
        header_cursor = (u8 *)((u32)(header_cursor + 0x1D) & ~1);
        actor->unknown_0x240 = (u16 *)header_cursor;
        actor->animation_index = 0;
        actor->sequence_active = 0;
        *(Copy6 *)header_cursor = *(Copy6 *)cursor;
    }
    if (optional_count != 0)
    {
        part_index = 0;
        has_optional = optional_count != 0;
        if (has_optional)
        {
            do
            {
                cursor = (u8 *)((u32)(cursor + 3) & ~3);
                optional_pool = &D_8010D034;
                shared_base = D_8011BF00;
                optional_bytes = *(s32 *)cursor;
                if (optional_bytes < 0x1000)
                {
                    optional_tag = group;
                    if (optional_tag >= 3)
                    {
                        optional_tag = 2;
                    }
                    FIELD_ACTOR_OPTIONAL_DATA(actor, part_index) = func_8009CA54(*optional_pool, optional_bytes, optional_tag);
                }
                else
                {
                    FIELD_ACTOR_OPTIONAL_DATA(actor, part_index) = shared_base + *(s32 *)(shared_base + 4);
                }
                cursor += 4;
                if (optional_bytes < 0x1000)
                {
                    memcpy(FIELD_ACTOR_OPTIONAL_DATA(actor, part_index), cursor, optional_bytes);
                }
                cursor += optional_bytes;
                part_index += 1;
            } while (part_index < optional_count);
        }
    }
}

/**
 * @brief Upload a field texture and its optional palette to the selected slot.
 * @param resource Pointer to the image resource header and payload blocks.
 * @param slot Upload slot; values of three or more share the third flags entry.
 */
void func_8009AE38(u8 *resource, s32 slot)
{
    s32 original_slot;
    s32 flags_slot;
    u8 *cursor;
    s32 block_size;
    s32 copy_size;
    u8 *palette_dst;
    u8 *palette_base;
    RECT rect;
    s32 width;
    s32 height;

    /* The do/while(0) blocks and palette_base keep the CLUT arms from cross-jumping. */
    original_slot = slot;
    resource += 4;
    cursor = resource;
    if (original_slot >= 3)
    {
        slot = 2;
    }
    g_field_texture_slot_flags[slot] = *cursor;
    cursor += 4;
    flags_slot = original_slot;
    if (original_slot >= 3)
    {
        flags_slot = 2;
    }
    if (g_field_texture_slot_flags[flags_slot] & 8)
    {
        block_size = *(s32 *)cursor;
        cursor += 0xC;
        if (original_slot < 2)
        {
            copy_size = 0x200;
            do
            {
                rect.y = (original_slot * 2) + 0x1EE;
                rect.w = 0x100;
                rect.h = 1;
            } while (0);
            palette_base = g_field_actor_clut_buffers;
            palette_dst = palette_base + (original_slot << 10);

            slot = block_size - 0xC;
            rect.x = 0;
            if (slot < 0x201)
            {
                copy_size = slot;
            }
        }
        else
        {
            copy_size = 0x200;
            do
            {
                rect.y = 0x1F2;
                rect.w = 0x100;
                rect.h = 1;
            } while (0);
            palette_dst = g_field_shared_clut_buffer;
            rect.x = 0;
            if (block_size - 0xC < 0x201)
            {
                copy_size = block_size - 0xC;
            }
        }
        do
        {
            do
            {
                do
                {
                    memcpy(palette_dst, cursor, copy_size);
                } while (0);
            } while (0);
        } while (0);
        LoadImage(&rect, (u_long *)cursor);
        cursor = cursor + block_size - 0xC;
    }
    cursor += 8;
    do
    {
        width = *(u16 *)cursor;
    } while (0);
    cursor += 2;
    height = *(u16 *)cursor;
    cursor += 2;
    if (original_slot < 2)
    {
        rect.x = (original_slot << 6) + 0x340;
        rect.y = 0x100;
    }
    else
    {
        rect.x = 0x140;
        rect.y = 0;
    }
    do
    {
        rect.w = width;
        rect.h = height;
    } while (0);
    LoadImage(&rect, (u_long *)cursor);
}
