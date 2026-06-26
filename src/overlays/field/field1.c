#include "common.h"
#include "cd_resources.h"
#include "scene_state.h"

extern void DrawSync(s32);
extern void ClearOTagR(void*, s32);
extern void field_select_object(s32, void*);
extern void field_build_render_records(void*, unsigned short);
extern void func_80054B1C(void);

extern u8 g_cdAudioEnabled;
extern unsigned int D_801ED02C;
extern s32 D_801ED000;
extern s32 D_801ED004;
extern s32 D_801ED010;
extern s32 D_801ED00C;
extern u16 D_801ED480;
extern u16 D_801ED482;
extern u32 g_field_vram_byte_count;
extern s32 D_801ED490;

typedef struct
{
    u16 unk0;
    u16 unk2;
    u8 unk4;
} RegStruct;
/**
 * @brief A field scene object: a texture/CLUT image plus its color and
 *        placement data.
 *
 * @note @c g_field_objects is a null-terminated array of pointers to these.
 *       @c field_select_object selects one by index and uploads its image;
 *       @c field_load_map clears @c flag26 on every object and registers
 *       each distinct one.
 */
typedef struct
{
    u8 _pad0[4];
    s32 unk4; /**< 0x04 VRAM address: LoadImage source; also the dedup key in field_load_map */
    u8 _pad1[0x26 - 8];
    u16 flag26; /**< 0x26 cleared at the start of each map load (field_load_map) */
    u16 unk28;  /**< 0x28 hi byte = texture height, lo byte = CLUT width */
    u16 unk2A;  /**< 0x2A pixel count passed to field_apply_pixel_lookup */
    u8 unk2C;   /**< 0x2C bit0 = has explicit color, bit1 -> RegStruct.unk4 */
    u8 unk2D;   /**< 0x2D background red   */
    u8 unk2E;   /**< 0x2E background green */
    u8 unk2F;   /**< 0x2F background blue  */
    u16 unk30;  /**< 0x30 -> RegStruct.unk0 */
    u16 unk32;  /**< 0x32 -> RegStruct.unk2 */
} FieldObject;

extern FieldObject** g_field_objects;
typedef struct
{
    s16 a;
    s16 b;
    s16 c;
    s16 d;
} Args4;

/* Node structure used in the linked list */
typedef struct Node
{
    struct Node* unk0; // offset 0x00 (next pointer)
    u8 pad[32];
    u32 unk24;
    u32 unk28;
    u32 unk2C;
    u32 unk30;
} Node;

/* Field scratch arena at 0x80180014. Built by field_build_render_records;
   walked by field_clear_node_accumulators. */
typedef struct
{
    u8 padding[8]; // offsets 0x00-0x07 (unknown/unused)
    Node* unk8;    // offset 0x08 (pointer to head of list)
} FieldScene;

typedef struct
{
  FieldScene *scene;
  u32 vram_byte_count;
} FieldSceneGlobals;

extern FieldSceneGlobals g_field_scene;
extern void func_80056A04(void); /* extern */

/**
 * @brief Initialize a field render context for a scene (no-FMV variant).
 *
 * Clears both ordering tables of the double-buffered render context, selects
 * field object @p arg1, advances the global scene cursor (D_801ED000) by 0x60,
 * and seeds the two primitive-buffer cursors.
 *
 * @param arg0 Field render context (two 0x7CC4-byte frame buffers).
 * @param arg1 Field object index, passed to field_select_object.
 * @see decomp.me (100%) https://decomp.me/scratch/m1WWc
 */
void field_init_ctx(void* arg0, unsigned short arg1)
{
    u32* mem;
    s32 zero = 0;
    DrawSync(zero);
    ClearOTagR(arg0, 0x1010);
    ClearOTagR(((char*)arg0) + 0x7CC4, 0x1010);
    field_select_object(arg1 & 0xFFFF, arg0);
    mem = (u32*)0x801ED000;
    mem[1] = mem[0];
    mem[0] = mem[0] + 0x60;
    func_80054B1C();
    *((u32*)(((char*)arg0) + 0x40B8)) = mem[3];
    *((u32*)(((char*)arg0) + 0xBD7C)) = mem[4];
}

/**
 * @brief Reset the field scene-state block at 0x801ED480 and counter D_801ED02C.
 * @see decomp.me (100%) https://decomp.me/scratch/S4vVP
 */
void field_scene_reset(void)
{
    S_801ED480* ptr = (S_801ED480*)0x801ED480;
    ptr->unk0 = 0;
    ptr->unk2 = 0;
    ptr->unk10 = 0;
    D_801ED02C = 0;
    func_800642D4();
}

/**
 * @brief Build and draw one field frame, servicing streamed video when active.
 *
 * Runs the field draw helpers against the render context and pumps
 * movie_service_video_ops (func_80140D48) whenever CD audio is playing.
 *
 * @param unused Unused first parameter.
 * @param base   Field render context base address.
 * @param arg2   Primitive/draw count; forced to 2 when @p arg3 is non-zero.
 * @param arg3   Non-zero selects the fixed 2-primitive path.
 * @see decomp.me (100%) https://decomp.me/scratch/lg9gw
 */
void field_draw_frame(s32 unused, s32 base, s32 arg2, s32 arg3)
{

    u8* struct_ptr;
    if (arg3 != 0)
    {
        func_80054CA8(base + 0x40B8, base + 0x40, 2);
    }
    else
    {
        func_80054CA8(base + 0x40B8, base + 0x40, arg2);
    }
    struct_ptr = (u8*)0x801ED800;
    func_80059C44();
    if (g_cdAudioEnabled != 0)
    {

        func_80140D48();
    }
    func_80064C28(base + 0x40B8, base, arg2);
    if (struct_ptr[4] != 0)
    {
        func_80140D48();
    }
}

/**
 * @brief Zero the four per-node accumulators across the g_allocInfo list.
 *
 * Walks the linked list from g_allocInfo->unk8, clearing unk24/unk28/unk2C/unk30
 * on every node. When both arguments are zero, also calls func_80056A04.
 *
 * @param arg0 TODO: meaning unknown; both args zero triggers func_80056A04.
 * @param arg1 TODO: meaning unknown.
 * @see decomp.me (100%) https://decomp.me/scratch/KyLZb
 */
void field_clear_node_accumulators(s32 arg0, s32 arg1)
{
    Node* var_v0;

    var_v0 = g_field_scene.scene->unk8;
    if (var_v0 != 0)
    {
        do
        {
            var_v0->unk24 = 0;
            var_v0->unk28 = 0;
            var_v0->unk2C = 0;
            var_v0->unk30 = 0;
            var_v0 = var_v0->unk0;
        } while (var_v0 != 0);
    }
    if ((arg0 == 0) && (arg1 == 0))
    {
        func_80056A04();
    }
}

/**
 * @brief Initialize a field scene and its FMV using a caller-supplied context.
 *
 * Streams MOVIE.BIN into 0x80140000 and starts playback, loads the map
 * graphics for map id D_801ED480, then initializes render context @p arg1 for
 * field object D_801ED482.
 *
 * @param unused Unused first parameter.
 * @param arg1   Field render context to initialize.
 * @see decomp.me (100%) https://decomp.me/scratch/EXpXm
 */
void field_init_with_fmv(void* unused, void* arg1)
{
    u16 temp_s1;
    u16 first_val;
    u8* addr = (u8*)0x801ED480;

    DrawSync(0);
    cdrom_stream(CD_RES_MOVIE_BIN, (void*)0x80140000);
    func_80140018(0); /* MOVIE.BIN entry point (offset 0x18) */

    // Read two 16-bit values from fixed address 0x801ED480
    first_val = *(u16*)addr;
    field_load_map(first_val);
    temp_s1 = *(u16*)(addr + 2);

    DrawSync(0);
    ClearOTagR(arg1, 0x1010);
    ClearOTagR((void*)((char*)arg1 + 0x7CC4), 0x1010);
    field_select_object(temp_s1 & 0xFFFF, arg1);

    D_801ED004 = D_801ED000;
    D_801ED000 += 0x60;

    func_80054B1C();

    // Write to offsets 0x40B8 and 0xBD7C of the structure pointed by arg1
    *(s32*)((char*)arg1 + 0x40B8) = D_801ED00C;
    *(s32*)((char*)arg1 + 0xBD7C) = D_801ED010;

    func_800643E0();
}

/**
 * @brief Initialize a field scene and its FMV, allocating the render context.
 *
 * Same flow as field_init_with_fmv, but obtains the render context from
 * FUN_80015c28 instead of receiving it as a parameter.
 * @see decomp.me (100%) https://decomp.me/scratch/KMYoZ
 */
void field_init_with_fmv_alloc(void)
{
    u16 temp_s1;
    void* temp_s0;
    char* ptr;

    temp_s0 = FUN_80015c28();
    DrawSync(0);
    cdrom_stream(CD_RES_MOVIE_BIN, (void*)0x80140000);
    func_80140018(0); /* MOVIE.BIN entry point (offset 0x18) */
    field_load_map(D_801ED480);
    temp_s1 = D_801ED482;
    DrawSync(0);
    ClearOTagR(temp_s0, 0x1010);
    ClearOTagR((void*)((char*)temp_s0 + 0x7CC4), 0x1010);
    field_select_object(temp_s1 & 0xFFFF, temp_s0);
    D_801ED004 = D_801ED000;
    D_801ED000 += 0x60;
    func_80054B1C();

    ptr = (char*)temp_s0;
    *(s32*)(ptr + 0x40B8) = D_801ED00C;
    ptr += 0x8000;
    *(s32*)(ptr + 0x3D7C) = D_801ED010;

    func_800643E0();
}

/**
 * @brief Load a field map's graphics and register its scene objects.
 *
 * Reads map resource (0xB4 + @p arg0) into 0x80180000, uploads its texture
 * pages to VRAM via LoadImage, clears flag26 on every object in g_field_objects,
 * and (when D_801ED490 is set) registers each distinct object through
 * field_apply_pixel_lookup.
 *
 * @param arg0 Map id; values below 15 use a blocking CD read, others stream.
 * @see decomp.me (97.33%) https://decomp.me/scratch/V1GlO
 */
void field_load_map(s32 arg0)
{
    u16 sp[24];
    s32 seen_count;
    s32 remaining;
    s32* seen;
    s32 var_s0;
    u32 var_s1;
    FieldObject* obj;
    FieldObject* var_v0;
    s32 vram_addr;
    FieldObject** obj_iter;
    u16 map_id;
    DrawSync(0);
    map_id = arg0;
    if (map_id < 0xFU)
    {
        cdrom_queue_read((arg0 + 0xB4) & 0xFFFF, 0x80180000);
        cdrom_wait_queue_empty();
    }
    else
    {
        cdrom_stream((arg0 + 0xB4) & 0xFFFF, 0x80180000);
    }
    var_s1 = (u32)g_field_scene.scene;
    sp[0] = 0x140;
    sp[1] = 0x100;
    var_s0 = g_field_scene.vram_byte_count >> 9;
    sp[3] = 0x100;
    while (var_s0 != 0)
    {
        sp[2] = (s16)((var_s0 < 0x11) ? (var_s0) : (0x10));
        LoadImage(sp, var_s1);
        var_s1 += 0x2000;
        var_s0 -= (s16)sp[2];
        sp[0] += 0x10;
    }

    DrawSync(0);
    obj_iter = g_field_objects;
    var_v0 = *obj_iter;
    if (var_v0 != 0)
    {
        obj_iter++;
        do
        {
            var_v0->flag26 = 0;
            var_v0 = *obj_iter;
            obj_iter++;
        } while (var_v0 != 0);
    }
    if (D_801ED490 != 0)
    {
        obj_iter = g_field_objects;
        seen_count = 0;
        if ((*obj_iter) != 0)
        {
            seen = &sp[4];
            do
            {
                obj = *obj_iter;
                vram_addr = obj->unk4;
                remaining = seen_count;
                if (seen_count != 0)
                {
                loop_13:
                    if ((*seen) != vram_addr)
                    {
                        remaining -= 1;
                        var_s1 = (u32)g_field_scene.scene;
                        seen += 1;
                        if (remaining != 0)
                        {
                            goto loop_13;
                        }
                    }

                    if (remaining == 0)
                    {
                        goto block_16;
                    }
                }
                else
                {
                block_16:
                    seen_count += 1;

                    *seen = vram_addr;
                    field_apply_pixel_lookup(vram_addr, obj->unk2A, D_801ED490 - 1, obj);
                }
                obj_iter += 1;
                seen = &sp[4];
            } while ((*obj_iter) != 0);
        }
    }
}

/**
 * @brief Select a field object and apply its image and background color.
 *
 * Walks g_field_objects to object @p arg0, copies its unk30/unk32 into the register
 * block at 0x801ED400, sets the DRAWENV background color in both frame buffers
 * of @p arg1, and uploads the object's texture and CLUT to VRAM.
 *
 * @param arg0 Field object index into g_field_objects.
 * @param arg1 Field render context; when NULL the DRAWENV update is skipped.
 * @see decomp.me (100%) https://decomp.me/scratch/vjiqR
 */
void field_select_object(unsigned short arg0, void* arg1)
{
    u8* ptr = (u8*)g_field_objects;
    RegStruct* hw = (RegStruct*)0x801ED400;
    short counter = arg0 - 1;
    FieldObject* temp_s0;
    s32 var_s2;
    u16 temp_s1;
    u8 temp_v0_3;
    Args4 args;
    while ((counter & 0xFFFF) != 0xFFFF)
    {
        if ((*((u32*)(ptr + 4))) == 0)
        {
            break;
        }
        counter--;
        ptr += 4;
    }

    temp_s0 = *((FieldObject**)ptr);
    hw->unk0 = temp_s0->unk30;
    hw->unk2 = temp_s0->unk32;
    hw->unk4 = ((*((u32*)(&temp_s0->unk2C))) >> 1) & 1;
    if (arg1 != 0)
    {
        u32 packed;
        u8* base = (u8*)arg1;
        u8* base2 = base + 0x7CC4;
        base2[0x406C] = 1;
        base[0x406C] = 1;
        packed = *((u32*)(&temp_s0->unk2C));
        if (packed & 1)
        {
            base[0x406D] = temp_s0->unk2D;
            base[0x406E] = temp_s0->unk2E;
            base[0x406F] = temp_s0->unk2F;
            base[0xBD31] = temp_s0->unk2D;
            base[0xBD32] = temp_s0->unk2E;
            base[0xBD33] = temp_s0->unk2F;
        }
        else
        {
            base[0x406D] = 0;
            base[0x406E] = 0;
            base[0x406F] = 0;
            base[0xBD31] = 0;
            base[0xBD32] = 0;
            base[0xBD33] = 0;
        }
    }
    var_s2 = temp_s0->unk4;
    args.a = 0;
    args.b = 0x1D8;
    temp_s1 = temp_s0->unk28 >> 8;
    if (temp_s1 != 0)
    {
        s32 s3 = temp_s1;
        args.c = 0x100;
        args.d = (s16)temp_s1;
        LoadImage(&args, var_s2);
        var_s2 += s3 << 9;
        args.b += temp_s1;
    }
    temp_v0_3 = (u8)temp_s0->unk28;
    if (temp_v0_3 != 0)
    {
        args.c = (s16)temp_v0_3;
        args.d = 1;
        LoadImage(&args, var_s2);
    }
    field_build_render_records(temp_s0, arg0 & 0xFFFF);
    func_8006312C();
}
