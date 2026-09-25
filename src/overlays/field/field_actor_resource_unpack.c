/**
 * @file field_actor_resource_unpack.c
 * @brief Actor resource read queue, the unpacker for loaded actor resources and the
 *        actor texture upload.
 *
 * An actor resource is read from CD into g_field_cd_buffer and unpacked into blocks of
 * the actor heap (D_8010D034) tagged with the owner, so that all blocks of an owner can
 * be freed together. Owners 0 and 1 are the two party players; every other owner shares
 * FIELD_SHARED_OWNER.
 */
#include "cdrom.h"
#include "common.h"
#include "field_calls.h"
#include "field_actor_palette.h"
#include "field_effect_types.h"
#include "field_mesh.h"
#include "sdk/memory.h"
#include "sdk/libgpu.h"
#include "tim.h"

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
/** @brief Most meshes an actor resource can hold. */
#define FIELD_ACTOR_MAX_MESHES 3

/** @brief SVECTORs stored per face: three vertices, the normal and the offset. */
#define FIELD_MESH_VECTORS_PER_FACE 5
/** @brief Bytes of one packed mesh face. */
#define FIELD_MESH_FACE_SIZE 16

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

/** @brief Footprint query handed to func_8005B368 (same layout as in field_collision.c). */
struct FieldCollisionQuery
{
    s32 x;
    s32 y;
    s32 z;
    u16 width;
    s16 height_tolerance;
    u16 depth;
};

/*
 * Byte-aligned images of the animation sections. The resource stream is not
 * aligned, so these are copied as unaligned structs.
 */

/** @brief First 18 bytes of an animation definition (up to unknown_0x12). */
typedef struct
{
    s8 bytes[18];
} FieldAnimationBaseBytes;

/** @brief Extension fields of an animation definition (unknown_0x12 onward). */
typedef struct
{
    s8 bytes[10];
} FieldAnimationExtensionBytes;

/** @brief A whole alternate animation definition. */
typedef struct
{
    s8 bytes[sizeof(FieldActorAnimationDef)];
} FieldAnimationDefBytes;

/** @brief Three-halfword tail after the alternate definitions (FieldActorState::unknown_0x240). */
typedef struct
{
    s8 bytes[6];
} FieldAnimationTailBytes;

/** @brief Heap block holding an actor's animation definitions with every optional section. */
#define FIELD_ANIMATION_BLOCK_SIZE \
    (sizeof(FieldAnimationBaseBytes) + sizeof(FieldAnimationExtensionBytes) + 2 * sizeof(FieldAnimationDefBytes) + sizeof(FieldAnimationTailBytes))

/** @brief Six-byte link record in FieldActorState::link_records. */
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
 * @brief Link record at byte offset @p offset of @p actor's link records.
 * @note The sum is written offset-first so the addu emits the offset operand first.
 */
#define FIELD_ACTOR_LINK_RECORD(actor, offset) ((FieldActorLinkRecord*)((offset) + (u32)(actor)->link_records))

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
extern s32 g_field_scene_contact_latched;
extern s32 g_field_scene_request_pending;
extern u8* D_8010D034;
extern u8* g_field_cd_buffer;
extern s32 g_field_resource_queue_count;
extern s32 g_field_resource_queue[FIELD_RESOURCE_QUEUE_CAPACITY];
extern s32 g_field_loading_resource;
extern FieldMeshTexturePart g_field_mesh_texture_parts[FIELD_OWNER_COUNT][FIELD_OWNER_TEXTURE_PARTS];
extern u8 g_field_sound_tables[];
extern s16 g_field_texture_slot_flags[FIELD_OWNER_COUNT];

/* Local: field_contact_geometry.c calls it with a third argument, so it stays out of field_calls.h. */
s32 func_800B22F0(s32 actor_id, s32 script);
void* func_8009CA54(u8* pool, s32 size, s32 tag);
void func_8009CB64(u8* pool, s32 tag);

static s32 field_enqueue_resource_read(s32 resource_id);
static void field_upload_actor_texture(u8* tim, s32 owner);

/**
 * @brief Heap tag and table index of an owner.
 * @param owner Owner object index.
 * @return @p owner, with every owner past FIELD_SHARED_OWNER folded onto it.
 */
static inline s32 field_owner_tag(s32 owner)
{
    if (owner > FIELD_SHARED_OWNER)
    {
        owner = FIELD_SHARED_OWNER;
    }
    return owner;
}

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
        /* Called as returning int: the original uses the s16 result without extending it. */
        label = ((s32(*)(struct FieldCollisionQuery*))func_8005B368)(&probe);
        if (label != -1)
        {
            if (g_field_scene_request_pending == 0)
            {
                if (g_field_scene_contact_latched == 0)
                {
                    func_800B22F0(0, label | FIELD_INTERACTION_SCRIPT);
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
        i = 0;
        g_field_loading_resource = g_field_resource_queue[0];
        count = g_field_resource_queue_count - 1;
        for (; i < count; i++)
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
    func_8009CB64(D_8010D034, tag);
}

/**
 * @brief Unpack the actor resource in g_field_cd_buffer into heap blocks of an actor.
 * @param owner Owner object index; selects the heap tag, texture slot and texture part table.
 * @param actor Actor receiving the unpacked tracks, meshes, records, parts and animations.
 */
void field_unpack_actor_resource(s32 owner, FieldActorState* actor)
{
    s32 mesh_count;
    u8* resource;
    s32 texture_total;
    /** @brief Link record rewritten to the current layout. */
    union
    {
        u32 word;
        /** @brief Low byte: record x and flag. */
        struct
        {
            u32 x : 7;
            u32 flag : 1;
            u32 unused : 24;
        } bits;
        /** @brief Byte and halfword view of the record. */
        struct
        {
            u8 b0;
            u8 b1;
            u16 h2;
            u16 h4;
        } h;
    } packed;
    s32 allocation_size;
    s32 has_tracks;
    s32 track_size;
    s32 record_bytes;
    s32 index_bytes;
    s32 part_bytes;
    s32 sound_bytes;
    s32 track_tag;
    s32 animation_tag;
    s32 mesh_tag;
    s32 vector_tag;
    s32 face_tag;
    s32 pixel_tag;
    s32 record_tag;
    s32 index_tag;
    s32 part_tag;
    s32 row;
    s32 texture_height;
    s32 item;
    s32 record_offset;
    s32 sound_count;
    u8* cursor;
    s32 i;
    s32 texture_owner;
    s32 column;
    u8* indices;
    u8* parts;
    FieldActorAnimationDef* animation;
    u8* records;
    /** @brief Output cursor shared by the vector tables and the face bytes. */
    union
    {
        u16* half;
        u8* byte;
    } output;
    u16* pixel_input;
    u8* row_input;
    u16* pixel_output;
    u8* block;
    FieldMeshResource* mesh;
    FieldMeshResource* mesh_fields;
    s32 track_offset;
    u16 component;
    u16 pixel;
    s32 j;
    FieldMeshTexturePart* texture;
    s32 record_count;
    s32 fallback_value;
    u8 face_byte;
    u8 texture_count;
    u8* track_source;
    u8** heap;
    u8** sound_heap;
    u8* sound_tables;

    block = g_field_cd_buffer;
    cursor = block;
    resource = cursor;
    if (*(u16*)cursor != 0)
    {
        field_upload_actor_texture(cursor + *(u16*)cursor, owner);
    }
    cursor = block + 2;
    has_tracks = *(u16*)cursor;
    cursor = (u8*)((u32)(cursor + 5) & ~3);
    if (has_tracks != 0)
    {
        /* The block boundary keeps the owner reload after this halfword load. */
        do
        {
            track_offset = *(u16*)cursor;
        } while (0);
        track_tag = owner;
        track_source = block + track_offset;
        track_size = *(u16*)(cursor + 2) - track_offset;
        if (track_tag > FIELD_SHARED_OWNER)
        {
            track_tag = FIELD_SHARED_OWNER;
        }
        block = func_8009CA54(D_8010D034, track_size, track_tag);
        cursor += 2;
        memcpy(block, track_source, track_size);
        actor->track_data = block;
    }
    cursor += 2;
    mesh_count = *(u16*)cursor;
    cursor += 2;
    if (mesh_count != 0)
    {
        mesh_tag = field_owner_tag(owner);
        mesh = func_8009CA54(D_8010D034, FIELD_ACTOR_MAX_MESHES * sizeof(FieldMeshResource), mesh_tag);
        actor->mesh_data = (u8*)mesh;
    }
    i = 0;
    texture_total = 0;
    if (mesh_count != 0)
    {
        mesh_fields = mesh; /* A second cursor over the same mesh record. */
        do
        {
            block = resource + *(u16*)cursor;
            mesh->face_count = *(u16*)block;
            block += 4;
            mesh_fields->texture_part_count = *block;
            block += 4;
            allocation_size = mesh->face_count * (FIELD_MESH_VECTORS_PER_FACE * sizeof(SVECTOR));
            vector_tag = field_owner_tag(owner);
            item = 0;
            output.half = func_8009CA54(D_8010D034, allocation_size, vector_tag);
            mesh_fields->vertices = (SVECTOR*)output.half;
            if ((mesh->face_count * 3) != 0)
            {
                do
                {
                    component = *(u16*)block;
                    block += 2;
                    item++;
                    *output.half++ = component;
                    *output.half++ = *(u16*)block;
                    block += 2;
                    *output.half = *(u16*)block;
                    block += 2;
                    output.half += 2;
                } while (item < (mesh->face_count * 3));
            }
            mesh_fields->normals = (SVECTOR*)output.half;
            item = 0;
            if (mesh->face_count != 0)
            {
                do
                {
                    component = *(u16*)block;
                    block += 2;
                    item++;
                    *output.half++ = component;
                    *output.half++ = *(u16*)block;
                    block += 2;
                    *output.half = *(u16*)block;
                    block += 2;
                    output.half += 2;
                } while (item < (s32)mesh->face_count);
            }
            mesh_fields->offsets = (SVECTOR*)output.half;
            item = 0;
            if (mesh->face_count != 0)
            {
                do
                {
                    component = *(u16*)block;
                    block += 2;
                    item++;
                    *output.half++ = component;
                    *output.half++ = *(u16*)block;
                    block += 2;
                    *output.half = *(u16*)block;
                    block += 2;
                    output.half += 2;
                } while (item < (s32)mesh->face_count);
            }
            allocation_size = mesh->face_count * FIELD_MESH_FACE_SIZE;
            face_tag = field_owner_tag(owner);
            item = 0;
            output.byte = func_8009CA54(D_8010D034, allocation_size, face_tag);
            mesh_fields->faces = output.byte;
            if (mesh->face_count != 0)
            {
                do
                {
                    j = 0;
                    do
                    {
                        j += 1;
                        face_byte = *block;
                        block += 1;
                        *output.byte = face_byte;
                        output.byte += 1;
                    } while ((u32)j < FIELD_MESH_FACE_SIZE);
                    item += 1;
                } while (item < (s32)mesh->face_count);
            }
            texture_owner = field_owner_tag(owner);
            texture = mesh_fields->texture_parts = &g_field_mesh_texture_parts[texture_owner][texture_total];
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
                    pixel_tag = field_owner_tag(owner);
                    pixel_output = func_8009CA54(D_8010D034, allocation_size, pixel_tag);
                    row = 0;
                    texture->pixels = pixel_output;
                    texture_height = texture->height;
                    row_input = resource + *(u16*)resource + FIELD_ACTOR_TIM_PIXELS;
                    row_input = row_input + texture->x * 2 + texture->y * FIELD_ACTOR_TEXTURE_ROW_BYTES;
                    do
                    {
                        if (texture_height != 0)
                        {
                            do
                            {
                                pixel_input = (u16*)row_input;
                                column = 0;
                                if (texture->width != 0)
                                {
                                    do
                                    {
                                        column += 1;
                                        pixel = *pixel_input;
                                        pixel_input++;
                                        *pixel_output = pixel;
                                        pixel_output++;
                                    } while (column < (s32)texture->width);
                                }
                                row += 1;
                                row_input += FIELD_ACTOR_TEXTURE_ROW_BYTES;
                            } while (row < (s32)texture->height);
                        }
                        j += 1;
                        texture++;
                    } while (0); /* Its loop depth sets the s0/s1 allocation. */
                } while (j < (s32)mesh_fields->texture_part_count);
            }
            i += 1;
            cursor += 2;
            texture_count = mesh_fields->texture_part_count;
            texture_total += texture_count;
            mesh_fields++;
            mesh++;
        } while (i < mesh_count);
    }
    heap = &D_8010D034;
    fallback_value = *(u16*)cursor; /* unknown_0x12 unless the extension overrides it */
    cursor += 2;
    record_count = *cursor++;
    record_bytes = record_count * sizeof(FieldActorLinkRecord);
    record_tag = field_owner_tag(owner);
    records = func_8009CA54(*heap, record_bytes, record_tag);
    actor->link_records = records;
    memcpy(records, cursor, record_bytes);
    cursor += record_bytes;
    j = *cursor++;
    index_bytes = j * sizeof(u16);
    index_tag = field_owner_tag(owner);
    indices = func_8009CA54(*heap, index_bytes, index_tag);
    actor->link_record_indices = indices;
    memcpy(indices, cursor, index_bytes);
    cursor += index_bytes;
    j = *cursor;
    sound_count = j >> FIELD_SOUND_COUNT_SHIFT;
    if (sound_count >= 3)
    {
        sound_count = 0;
    }
    j = j & FIELD_PART_COUNT_MASK;
    cursor++;
    actor->part_count = j;
    part_bytes = j * sizeof(FieldActorPartDef);
    part_tag = field_owner_tag(owner);
    parts = func_8009CA54(*heap, part_bytes, part_tag);
    actor->parts = (FieldActorPartDef*)parts;
    memcpy(parts, cursor, part_bytes);
    cursor += part_bytes;
    actor->animation_index = 0;
    animation_tag = field_owner_tag(owner);
    animation = func_8009CA54(*heap, FIELD_ANIMATION_BLOCK_SIZE, animation_tag);
    actor->animation = animation;
    animation->unknown_0x12 = fallback_value;
    block = (u8*)actor->animation;
    actor->animations = (FieldActorAnimationDef*)block;
    *(FieldAnimationBaseBytes*)block = *(FieldAnimationBaseBytes*)cursor;
    block += sizeof(FieldAnimationBaseBytes);
    cursor += sizeof(FieldAnimationBaseBytes);
    if (actor->animation->flags & FIELD_ANIMATION_HAS_EXTENSION)
    {
        *(FieldAnimationExtensionBytes*)block = *(FieldAnimationExtensionBytes*)cursor;
        block += sizeof(FieldAnimationExtensionBytes);
        cursor += sizeof(FieldAnimationExtensionBytes);
    }
    /* Older resources keep the record flag in bit 7 of the second byte; move it to the first. */
    if ((actor->parts->orientation_flags.bytes.low >> 4) < 2)
    {
        i = 0;
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
                bcopy((u8*)&packed, actor->link_records + record_offset, sizeof(FieldActorLinkRecord));
                i += 1;
                record_offset += sizeof(FieldActorLinkRecord);
            } while (i < (s32)record_count);
        }
    }
    actor->unknown_0x222 = actor->animation->unknown_0x12;
    if (actor->animations->flags & FIELD_ANIMATION_HAS_ALTERNATES)
    {
        *(FieldAnimationDefBytes*)block = *(FieldAnimationDefBytes*)cursor;
        block += sizeof(FieldAnimationDefBytes);
        cursor += sizeof(FieldAnimationDefBytes);
        *(FieldAnimationDefBytes*)block = *(FieldAnimationDefBytes*)cursor;
        cursor += sizeof(FieldAnimationDefBytes);
        block = (u8*)((u32)(block + sizeof(FieldAnimationDefBytes) + 1) & ~1);
        actor->unknown_0x240 = (u16*)block;
        actor->animation_index = 0;
        actor->sequence_active = 0;
        *(FieldAnimationTailBytes*)block = *(FieldAnimationTailBytes*)cursor;
    }
    if (sound_count != 0)
    {
        for (i = 0; i < sound_count; i++)
        {
            cursor = (u8*)((u32)(cursor + 3) & ~3);
            sound_heap = &D_8010D034;
            sound_tables = g_field_sound_tables;
            sound_bytes = *(s32*)cursor;
            if (sound_bytes < FIELD_SOUND_SHARED_SIZE)
            {
                actor->sound_data[i] = func_8009CA54(*sound_heap, sound_bytes, field_owner_tag(owner));
            }
            else
            {
                /* The first sound of shared table 0. */
                actor->sound_data[i] = sound_tables + ((s32*)sound_tables)[1];
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
    g_field_texture_slot_flags[field_owner_tag(owner)] = *cursor;
    cursor += sizeof(u32);
    if (g_field_texture_slot_flags[field_owner_tag(owner)] & TIM_FLAG_HAS_CLUT)
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
            setRECT(&rect, 0, FIELD_SHARED_CLUT_VRAM_Y, CLUT_ENTRY_COUNT, 1);
            memcpy(g_field_shared_clut_buffer, cursor,
                   clut_block_size - FIELD_TIM_BLOCK_HEADER > FIELD_CLUT_BYTES ? FIELD_CLUT_BYTES : clut_block_size - FIELD_TIM_BLOCK_HEADER);
        }
        LoadImage(&rect, (u_long*)cursor);
        cursor = cursor + clut_block_size - FIELD_TIM_BLOCK_HEADER;
    }
    cursor += FIELD_TIM_BLOCK_HEADER - sizeof(TimDimensions);
    width = *(u16*)cursor;
    cursor += 2;
    height = *(u16*)cursor;
    cursor += 2;
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
