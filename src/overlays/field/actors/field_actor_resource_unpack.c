/**
 * @file field_actor_resource_unpack.c
 * @brief Actor resource read queue, the unpacker for loaded actor resources and the
 *        actor texture upload.
 *
 * An actor resource is read from CD into g_field_cd_buffer and unpacked into blocks of
 * the actor heap (g_field_actor_heap) tagged with the owner, so that all blocks of an owner can
 * be freed together. Owners 0 and 1 are the two party players; every other owner shares
 * FIELD_SHARED_OWNER.
 */
#include "main/cdrom.h"
#include "common.h"
#include "../internal/field_calls.h"
#include "../internal/field_actor_palette.h"
#include "../internal/field_effect_types.h"
#include "../internal/field_mesh.h"
#include <memory.h>
#include <libgpu.h>
#include "common/tim.h"
#include "main/display.h"
#include "main/field_entry.h"

/** @brief Capacity of the pending resource read queue. */
#define FIELD_RESOURCE_QUEUE_CAPACITY 16
/** @brief Loading resource id while no read is in flight. */
#define FIELD_NO_RESOURCE -1

/** @brief Owner index shared by every actor that is not one of the two party players. */
#define FIELD_SHARED_OWNER 2
/** @brief Number of distinct owners (two players plus the shared one). */
#define FIELD_OWNER_COUNT 3
/** @brief Texture parts reserved per owner in g_field_mesh_texture_parts. */
#define FIELD_OWNER_TEXTURE_PARTS 9
/** @brief Heap tag and table index of an owner: owners past FIELD_SHARED_OWNER share its tag. */
#define FIELD_OWNER_TAG(owner) ((owner) < FIELD_OWNER_COUNT ? (owner) : FIELD_SHARED_OWNER)
/** @brief Most meshes an actor resource can hold. */
#define FIELD_ACTOR_MAX_MESHES 3

/** @brief SVECTORs stored per face: three vertices, the normal and the offset. */
#define FIELD_MESH_VECTORS_PER_FACE 5
/** @brief Bytes of one packed mesh face. */
#define FIELD_MESH_FACE_SIZE sizeof(FieldMeshFace)

/** @brief Copy a packed three-halfword vector at @p input into the SVECTOR at @p output, advancing both. */
#define FIELD_COPY_PACKED_VECTOR(output, input)                                                                                                                \
    (*(u16*)(output) = *(u16*)(input), (output) += 2, (input) += 2, *(u16*)(output) = *(u16*)(input), (output) += 2, (input) += 2,                             \
     *(u16*)(output) = *(u16*)(input), (input) += 2, (output) += 4)

/** @brief Orientation mode of the first part from which a resource stores its curves in the current layout. */
#define FIELD_CURVE_FORMAT_CURRENT 2U

/** @brief Width in bytes of an actor texture (64 VRAM columns of 16 bits). */
#define FIELD_ACTOR_TEXTURE_ROW_BYTES 128
/** @brief Pixel data of an actor TIM: 8bpp with a 256-entry CLUT, then the image block header. */
#define FIELD_ACTOR_TIM_PIXELS (sizeof(Tim) + sizeof(TimBlock))
/** @brief Header bytes in front of a TIM block's payload (signed, so size arithmetic stays signed). */
#define FIELD_TIM_BLOCK_HEADER ((s32)sizeof(TimBlock))
/** @brief Bytes of a 256-entry CLUT. */
#define FIELD_CLUT_BYTES (CLUT_ENTRY_COUNT * 2)

/** @brief Section sizes from this value on mean "use the shared sound table" instead of inline data. */
#define FIELD_SOUND_SHARED_SIZE 0x1000
/** @brief Number of sound sections is stored in the top two bits of the part count byte. */
#define FIELD_SOUND_COUNT_SHIFT 6
/** @brief Part count bits of the part count byte. */
#define FIELD_PART_COUNT_MASK 0x3F

/** @brief Probe footprint used for marker contact (cells). */
#define FIELD_CONTACT_PROBE_WIDTH 8
#define FIELD_CONTACT_PROBE_HEIGHT 16
#define FIELD_CONTACT_PROBE_DEPTH 5
/** @brief Interaction id flag: run the id as a field script instead of a talk message. */
#define FIELD_INTERACTION_SCRIPT 0x8000

/*
 * Byte-aligned images of the animation sections. The resource stream is not
 * aligned, so these are copied as unaligned structs.
 */

/** @brief First 18 bytes of an animation definition (up to duration). */
typedef struct
{
    s8 bytes[18];
} FieldAnimationBaseBytes;

/** @brief Extension fields of an animation definition (duration onward). */
typedef struct
{
    s8 bytes[10];
} FieldAnimationExtensionBytes;

/** @brief A whole alternate animation definition. */
typedef struct
{
    s8 bytes[sizeof(FieldAnimationDef)];
} FieldAnimationDefBytes;

/** @brief Three-halfword tail after the alternate definitions (FieldActorSlot::part_masks). */
typedef struct
{
    s8 bytes[6];
} FieldAnimationTailBytes;

/** @brief Heap block holding an actor's animation definitions with every optional section. */
#define FIELD_ANIMATION_BLOCK_SIZE                                                                                                                             \
    (sizeof(FieldAnimationBaseBytes) + sizeof(FieldAnimationExtensionBytes) + 2 * sizeof(FieldAnimationDefBytes) + sizeof(FieldAnimationTailBytes))

/**
 * @brief FieldParameterCurve as older resources store it: the random flag is the top bit of segment_offset.
 */
typedef union
{
    struct
    {
        u8 segment_count;
        u8 segment_offset;
        u16 end_value;
        u16 start_value;
    } fields;
    /** @brief segment_count and segment_offset as one halfword; bit 15 is the random flag. */
    u16 head;
} FieldLegacyCurve;

/**
 * @brief Curve @p index of @p actor's curves, read in the legacy layout.
 */
#define FIELD_LEGACY_CURVE(actor, index) (&((FieldLegacyCurve*)(actor)->curves)[index])

/**
 * @brief memcpy with a signed byte count.
 * @note sdk/memory.h leaves memcpy unprototyped.
 * @param dst Destination buffer.
 * @param src Source buffer.
 * @param size Byte count.
 * @return @p dst.
 */
void* memcpy(void* dst, const void* src, s32 size);

/** @brief Command of the player actor (g_field_actors[0].command); 0 while idle. */
extern s16 g_field_player_command;
extern s32 g_field_resource_queue_count;
extern s32 g_field_resource_queue[FIELD_RESOURCE_QUEUE_CAPACITY];
extern s32 g_field_loading_resource;
extern FieldMeshTexturePart g_field_mesh_texture_parts[FIELD_OWNER_COUNT][FIELD_OWNER_TEXTURE_PARTS];
extern u8 g_field_sound_tables[];
extern s16 g_field_texture_slot_flags[FIELD_OWNER_COUNT];

/* Local: field_contact_geometry.c calls it with a third argument, so it stays out of field_calls.h. */
s32 field_start_interaction(s32 actor_id, s32 script);

static s32 field_enqueue_resource_read(s32 resource_id);
static void field_upload_actor_texture(u8* tim, s32 owner);

/**
 * @brief Start the interaction of a marker the player walks into.
 * @param position Player position to probe around.
 * @note The interaction starts once per contact; the latch clears when the
 *       player leaves the marker or a scene request is pending.
 */
void field_check_marker_contact(Vec3i* position)
{
    struct FieldCollisionQuery probe;
    s32 label;

    if (g_field_player_command == 0)
    {
        probe.x = position->x;
        probe.y = position->y;
        probe.z = position->z;
        probe.width = FIELD_CONTACT_PROBE_WIDTH;
        probe.height_tolerance = FIELD_CONTACT_PROBE_HEIGHT;
        probe.depth = FIELD_CONTACT_PROBE_DEPTH;
        label = field_collision_hit_markers(&probe);

        if (label != FIELD_COLLISION_MARKER_NONE)
        {
            if (g_field_scene_request_pending == 0)
            {
                if (g_field_scene_contact_latched == 0)
                {
                    field_start_interaction(0, label | FIELD_INTERACTION_SCRIPT);
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
 * @brief Queue the read of an actor resource.
 * @param resource_id CD resource index to read.
 * @return 0 when queued, 1 when the queue is full.
 */
s32 field_request_resource_read(s32 resource_id)
{
    return field_enqueue_resource_read(resource_id);
}

/**
 * @brief Drop every pending resource read.
 */
void field_clear_resource_queue(void)
{
    g_field_resource_queue_count = 0;
}

/**
 * @brief Get the resource whose read was issued last.
 * @return Resource id, or FIELD_NO_RESOURCE when the queue ran empty.
 */
s32 field_get_loading_resource(void)
{
    return g_field_loading_resource;
}

/**
 * @brief Append a resource read to the pending queue.
 * @param resource_id CD resource index to read.
 * @return 0 when queued, 1 when the queue is full.
 */
static s32 field_enqueue_resource_read(s32 resource_id)
{
    s32 count;

    count = g_field_resource_queue_count;

    if (count < FIELD_RESOURCE_QUEUE_CAPACITY)
    {
        g_field_resource_queue[count] = resource_id;
        g_field_resource_queue_count = count + 1;
        return 0;
    }

    return 1;
}

/**
 * @brief Issue the read at the head of the queue into g_field_cd_buffer.
 * @note The head is removed only when the CD queue accepted the read.
 */
void field_issue_next_resource_read(void)
{
    s32 count;
    s32 i;

    if (g_field_resource_queue_count == 0)
    {
        g_field_loading_resource = FIELD_NO_RESOURCE;
        return;
    }

    if (cdrom_queue_read((u16)g_field_resource_queue[0], g_field_cd_buffer) >= 0)
    {
        g_field_loading_resource = g_field_resource_queue[0];
        count = g_field_resource_queue_count - 1;

        for (i = 0; i < count; i++)
        {
            g_field_resource_queue[i] = g_field_resource_queue[i + 1];
        }

        g_field_resource_queue_count -= 1;
    }
}

/**
 * @brief Free every actor heap block of an owner.
 * @param tag Owner tag of the blocks to free.
 */
void field_free_owner_resources(s32 tag)
{
    field_block_free_tag(g_field_actor_heap, tag);
}

/**
 * @brief Unpack the actor resource in g_field_cd_buffer into heap blocks of an actor.
 * @param owner Owner object index; selects the heap tag, texture slot and texture part table.
 * @param actor Actor receiving the unpacked tracks, meshes, curves, parts and animations.
 */
void field_unpack_actor_resource(s32 owner, FieldActorSlot* actor)
{
    s32 mesh_count;
    u8* resource;
    s32 texture_total;
    /** @brief Curve rewritten to the current layout (FieldParameterCurve with the random flag in the first byte). */
    struct
    {
        u32 segment_count : 7;
        u32 random : 1;
        u8 segment_offset;
        u16 end_value;
        u16 start_value;
    } converted;
    s32 allocation_size;
    s32 has_tracks;
    s32 track_size;
    s32 curve_bytes;
    s32 segment_bytes;
    s32 part_bytes;
    s32 sound_bytes;
    s32 tag;
    s32 row;
    s32 texture_height;
    s32 item;
    s32 sound_count;
    u8* cursor;
    s32 i;
    s32 column;
    u16* segments;
    FieldObjectPart* parts;
    FieldAnimationDef* animation;
    FieldParameterCurve* curves;
    u8* output;
    u16* pixel_input;
    u8* row_input;
    u16* pixel_output;
    u8* block;
    FieldMeshResource* mesh;
    FieldMeshResource* mesh_fields;
    s32 track_offset;
    s32 j;
    FieldMeshTexturePart* texture;
    s32 curve_count;
    s32 duration;
    u8* track_source;
    s32* sound_table;

    block = g_field_cd_buffer;
    cursor = block;
    resource = cursor;

    if (*(u16*)cursor != 0)
    {
        field_upload_actor_texture(cursor + *(u16*)cursor, owner);
    }

    cursor = block + 2;
    has_tracks = *(u16*)cursor;
    cursor = (u8*)ALIGN4(cursor + sizeof(u16));

    if (has_tracks != 0)
    {
        do
        {
            track_offset = *(u16*)cursor;
        } while (0);
        tag = owner;
        track_source = block + track_offset;
        track_size = *(u16*)(cursor + 2) - track_offset;

        if (tag > FIELD_SHARED_OWNER)
        {
            tag = FIELD_SHARED_OWNER;
        }

        block = field_block_alloc(g_field_actor_heap, track_size, tag);
        cursor += 2;
        memcpy(block, track_source, track_size);
        actor->track_data = block;
    }

    cursor += 2;
    mesh_count = *(u16*)cursor;
    cursor += 2;

    if (mesh_count != 0)
    {
        mesh = field_block_alloc(g_field_actor_heap, FIELD_ACTOR_MAX_MESHES * sizeof(FieldMeshResource), FIELD_OWNER_TAG(owner));
        actor->mesh_data = (u8*)mesh;
    }

    i = 0;
    texture_total = 0;

    if (mesh_count != 0)
    {
        mesh_fields = mesh;

        do
        {
            block = resource + *(u16*)cursor;
            mesh->face_count = *(u16*)block;
            block += 4;
            mesh_fields->texture_part_count = *block;
            block += 4;
            allocation_size = mesh->face_count * (FIELD_MESH_VECTORS_PER_FACE * sizeof(SVECTOR));
            output = field_block_alloc(g_field_actor_heap, allocation_size, FIELD_OWNER_TAG(owner));
            mesh_fields->vertices = (SVECTOR*)output;

            for (item = 0; item < mesh->face_count * 3; item++)
            {
                FIELD_COPY_PACKED_VECTOR(output, block);
            }

            mesh_fields->normals = (SVECTOR*)output;

            for (item = 0; item < mesh->face_count; item++)
            {
                FIELD_COPY_PACKED_VECTOR(output, block);
            }

            mesh_fields->offsets = (SVECTOR*)output;

            for (item = 0; item < mesh->face_count; item++)
            {
                FIELD_COPY_PACKED_VECTOR(output, block);
            }

            allocation_size = mesh->face_count * FIELD_MESH_FACE_SIZE;
            tag = FIELD_OWNER_TAG(owner);
            item = 0;
            output = field_block_alloc(g_field_actor_heap, allocation_size, tag);
            mesh_fields->faces = (FieldMeshFace*)output;

            for (; item < mesh->face_count; item++)
            {
                for (j = 0; j < FIELD_MESH_FACE_SIZE; j++, output++, block++)
                {
                    *output = *block;
                }
            }

            texture = mesh_fields->texture_parts = &g_field_mesh_texture_parts[FIELD_OWNER_TAG(owner)][texture_total];
            j = 0;

            if (mesh_fields->texture_part_count != 0)
            {
                do
                {
                    texture->x = *block++;
                    texture->y = *block++;
                    texture->width = *block++;
                    texture->height = *block++;
                    allocation_size = texture->width * texture->height * 2;
                    pixel_output = field_block_alloc(g_field_actor_heap, allocation_size, FIELD_OWNER_TAG(owner));
                    row = 0;
                    texture->pixels = pixel_output;
                    texture_height = texture->height;
                    row_input = resource + *(u16*)resource + FIELD_ACTOR_TIM_PIXELS;
                    row_input = row_input + texture->x * 2 + texture->y * FIELD_ACTOR_TEXTURE_ROW_BYTES;

                    if (texture_height == 0)
                    {
                        j += 1;
                        texture++;
                        continue;
                    }

                    do
                    {
                        pixel_input = (u16*)row_input;
                        column = 0;

                        if (texture->width != 0)
                        {
                            do
                            {
                                column += 1;
                                *pixel_output++ = *pixel_input++;
                            } while (column < texture->width);
                        }

                        row += 1;
                        row_input += FIELD_ACTOR_TEXTURE_ROW_BYTES;
                    } while (row < texture->height);

                    j += 1;
                    texture++;
                } while (j < mesh_fields->texture_part_count);
            }

            i += 1;
            cursor += 2;
            texture_total += mesh_fields->texture_part_count;
            mesh_fields++;
            mesh++;
        } while (i < mesh_count);
    }

    duration = *(u16*)cursor;
    cursor += 2;
    curve_count = *cursor++;
    curve_bytes = curve_count * sizeof(FieldParameterCurve);
    curves = field_block_alloc(g_field_actor_heap, curve_bytes, FIELD_OWNER_TAG(owner));
    actor->curves = curves;
    memcpy(curves, cursor, curve_bytes);
    cursor += curve_bytes;
    j = *cursor++;
    segment_bytes = j * sizeof(u16);
    segments = field_block_alloc(g_field_actor_heap, segment_bytes, FIELD_OWNER_TAG(owner));
    actor->curve_segments = segments;
    memcpy(segments, cursor, segment_bytes);
    cursor += segment_bytes;
    j = *cursor;
    sound_count = j >> FIELD_SOUND_COUNT_SHIFT;

    if (sound_count > FIELD_ACTOR_SOUND_COUNT)
    {
        sound_count = 0;
    }

    j = j & FIELD_PART_COUNT_MASK;
    cursor++;
    actor->part_count = j;
    part_bytes = j * sizeof(FieldObjectPart);
    parts = field_block_alloc(g_field_actor_heap, part_bytes, FIELD_OWNER_TAG(owner));
    actor->parts = parts;
    memcpy(parts, cursor, part_bytes);
    cursor += part_bytes;
    actor->animation_index = 0;
    animation = field_block_alloc(g_field_actor_heap, FIELD_ANIMATION_BLOCK_SIZE, FIELD_OWNER_TAG(owner));
    actor->animation = animation;
    animation->duration = duration;
    block = (u8*)actor->animation;
    actor->animations = (FieldAnimationDef*)block;
    *(FieldAnimationBaseBytes*)block = *(FieldAnimationBaseBytes*)cursor;
    block += sizeof(FieldAnimationBaseBytes);
    cursor += sizeof(FieldAnimationBaseBytes);

    if (actor->animation->flags & FIELD_ANIMATION_HAS_EXTENSION)
    {
        *(FieldAnimationExtensionBytes*)block = *(FieldAnimationExtensionBytes*)cursor;
        block += sizeof(FieldAnimationExtensionBytes);
        cursor += sizeof(FieldAnimationExtensionBytes);
    }

    /* Older resources keep the random flag in bit 7 of segment_offset; move it above segment_count. */
    if (actor->parts->orientation_flags.bits.mode < FIELD_CURVE_FORMAT_CURRENT)
    {
        for (i = 0; i < curve_count; i++)
        {
            converted.segment_count = FIELD_LEGACY_CURVE(actor, i)->fields.segment_count & FIELD_CURVE_SEGMENT_COUNT_MASK;
            converted.random = FIELD_LEGACY_CURVE(actor, i)->head >> 15;
            converted.segment_offset = (FIELD_LEGACY_CURVE(actor, i)->head >> 8) & FIELD_CURVE_SEGMENT_COUNT_MASK;
            converted.start_value = FIELD_LEGACY_CURVE(actor, i)->fields.start_value;
            converted.end_value = FIELD_LEGACY_CURVE(actor, i)->fields.end_value;
            bcopy((u8*)&converted, (u8*)&actor->curves[i], sizeof(FieldLegacyCurve));
        }
    }

    actor->duration = actor->animation->duration;

    if (actor->animations->flags & FIELD_ANIMATION_HAS_ALTERNATES)
    {
        *(FieldAnimationDefBytes*)block = *(FieldAnimationDefBytes*)cursor;
        block += sizeof(FieldAnimationDefBytes);
        cursor += sizeof(FieldAnimationDefBytes);
        *(FieldAnimationDefBytes*)block = *(FieldAnimationDefBytes*)cursor;
        cursor += sizeof(FieldAnimationDefBytes);
        block = (u8*)ALIGN2(block + sizeof(FieldAnimationDefBytes));
        actor->part_masks = (u16*)block;
        actor->animation_index = 0;
        actor->sequence_active = 0;
        *(FieldAnimationTailBytes*)block = *(FieldAnimationTailBytes*)cursor;
    }

    if (sound_count != 0)
    {
        for (i = 0; i < sound_count; i++)
        {
            cursor = (u8*)ALIGN4(cursor);
            sound_bytes = *(s32*)cursor;

            if (sound_bytes < FIELD_SOUND_SHARED_SIZE)
            {
                actor->sound_data[i] = field_block_alloc(g_field_actor_heap, sound_bytes, FIELD_OWNER_TAG(owner));
            }
            else
            {
                /* The first sound of shared table 0. */
                sound_table = (s32*)g_field_sound_tables;
                actor->sound_data[i] = (struct AkaoHeader*)((u8*)sound_table + sound_table[1]);
            }

            cursor += 4;

            if (sound_bytes < FIELD_SOUND_SHARED_SIZE)
            {
                memcpy(actor->sound_data[i], cursor, sound_bytes);
            }

            cursor += sound_bytes;
        }
    }
}

/**
 * @brief Upload an owner's actor TIM (and its CLUT) to the owner's VRAM slot.
 * @param tim Actor TIM image: 8bpp, optionally with a 256-entry CLUT.
 * @param owner Owner object index; owners from FIELD_SHARED_OWNER on share one slot.
 * @note The CLUT is also kept in the owner's palette buffer for palette animation.
 */
static void field_upload_actor_texture(u8* tim, s32 owner)
{
    u8* cursor;
    s32 clut_block_size;
    u8* clut_buffer;
    RECT rect;
    s32 width;
    s32 height;

    tim += sizeof(u32); /* skip the TIM id word */
    cursor = tim;
    g_field_texture_slot_flags[FIELD_OWNER_TAG(owner)] = *cursor;
    cursor += sizeof(u32);

    if (g_field_texture_slot_flags[FIELD_OWNER_TAG(owner)] & TIM_FLAG_HAS_CLUT)
    {
        clut_block_size = ((TimBlock*)cursor)->bnum;
        cursor += FIELD_TIM_BLOCK_HEADER;

        if (owner < FIELD_SHARED_OWNER)
        {
            setRECT(&rect, 0, FIELD_ACTOR_CLUT_VRAM_Y + owner * 2, CLUT_ENTRY_COUNT, 1);
            clut_buffer = g_field_actor_clut_buffers;
            memcpy(clut_buffer + owner * FIELD_ACTOR_CLUT_BUFFER_SIZE, cursor,
                   clut_block_size - FIELD_TIM_BLOCK_HEADER > FIELD_CLUT_BYTES ? FIELD_CLUT_BYTES : clut_block_size - FIELD_TIM_BLOCK_HEADER);
        }
        else
        {
            setRECT(&rect, 0, VRAM_CLUT_Y, CLUT_ENTRY_COUNT, 1);
            memcpy(g_field_shared_clut_buffer, cursor,
                   clut_block_size - FIELD_TIM_BLOCK_HEADER > FIELD_CLUT_BYTES ? FIELD_CLUT_BYTES : clut_block_size - FIELD_TIM_BLOCK_HEADER);
        }

        LoadImage(&rect, (u_long*)cursor);
        cursor = cursor + clut_block_size - FIELD_TIM_BLOCK_HEADER;
    }

    cursor = (u8*)&((TimBlock*)cursor)->dimensions;
    width = *(u16*)cursor;
    cursor += sizeof(u16);
    height = *(u16*)cursor;
    cursor += sizeof(u16);

    if (owner < FIELD_SHARED_OWNER)
    {
        setRECT(&rect, FIELD_ACTOR_TEXTURE_VRAM_X + owner * FIELD_ACTOR_TEXTURE_VRAM_WIDTH, FIELD_ACTOR_TEXTURE_VRAM_Y, width, height);
    }
    else
    {
        setRECT(&rect, FIELD_SHARED_TEXTURE_VRAM_X, FIELD_SHARED_TEXTURE_VRAM_Y, width, height);
    }

    LoadImage(&rect, (u_long*)cursor);
}
