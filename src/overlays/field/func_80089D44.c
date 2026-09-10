#include "common.h"

/**
 * @brief Actor record fields used by state reset and companion repositioning.
 */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 padC[4];
    s16 unk10;
    u8 pad12[0x21 - 0x12];
    u8 unk21;
    u8 pad22[2];
    u8 unk24;
    u8 unk25;
    u8 pad26;
    u8 unk27;
    u8 pad28[2];
    s16 unk2A;
    u8 pad2C[2];
    s16 unk2E;
    u8 pad30[0xA];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} FieldRecord;

/**
 * @brief Runtime actor state with packed resource and animation fields.
 */
typedef struct
{
    u32 unk0;
    u32 unk4;
    u32 unk8;
    s32 unkC;
    u8 pad10[4];
    s32 unk14;
    u8 pad18[0x174 - 0x18];
    u32 unk174;
    /** @brief Word and byte views of the packed animation state. */
    union

    {
        u32 word;
        struct

        {
            u8 byte0;
            u8 animation;
            u8 byte2;
            u8 byte3;
        } bytes;
    } flags;
    u8 pad17C[0x1AB - 0x17C];
    u8 unk1AB;
    u8 pad1AC[0x23C - 0x1AC];
} FieldState;

/** @brief Camera displacement words used by the visible-area bounds checks. */
typedef struct
{
    s32 unused0;
    s32 x;
    s32 unused8;
    s32 z;
} CameraPosition;

extern FieldRecord D_800FDF58[];
extern FieldState D_80105AE0[];
void field_start_actor_animation(s32, s32, s32);
void func_8006C3FC(FieldRecord *);
s32 func_800839F8(s32, s32);
s32 func_80083EEC(s32, s32, s32);
void func_800A3938(s32, s32);
void func_800B48B8(s32);
void func_8008A0B0(FieldRecord *, s32, s32);

/**
 * @brief Reset a matching actor's state, animation resources and companion position.
 * @param key Runtime actor key to locate among thirteen slots.
 * @param requested_state New low state bits, or -1 to use state zero.
 * @param animation Animation request, or -1 to leave animation selection unchanged.
 * @param event_id Event to dispatch at value 0x80, or -1 to skip dispatch.
 * @return Zero on success, or -1 when no actor matches the key.
 * @note Keep repeated slot-index reads and the signed scan sentinel for matching.
 * @note GCC 2.7.2 CDK matches all 219 instructions (876 bytes).
 */
s32 func_80089D44(s32 key, s32 requested_state, s32 animation, s32 event_id)
{
    FieldRecord *scan_record;
    FieldState *scan_state;
    FieldState *lookup_state;
    FieldRecord *record;
    FieldRecord *candidate;
    s32 state;
    s32 slot;
    s32 i;
    s32 unavailable;
    s32 animation_slot;
    CameraPosition *camera;
    FieldState *runtime_flags;
    FieldState *resources;
    FieldState *runtime_mode;

    camera = (CameraPosition *)0x801ED480;
    state = requested_state;
    scan_record = D_800FDF58;
    lookup_state = D_80105AE0;
    for (i = 0; i < 0xD; i++, lookup_state++, scan_record++)

    {
        if (lookup_state->unk14 == key)

        {
            goto found_it;
        }
    }
    record = (FieldRecord *)-1;
check:
    if (record != (FieldRecord *)-1)

    {
        goto body;
    }
    return (s32)record;
found_it:
    record = scan_record;
    goto check;
body:
    record->unk25 = 0;
    record->unk2A = 0;
    record->unk10 = 0;
    record->unk2E = 1;
    if (state == -1)
    {
        state = 0;
    }
    record->unk27 = 0;
    record->unk24 = 1;
    record->unk21 = (u8) ((record->unk21 & 0x80) | state);
    func_8006C3FC(record);
    if (animation != -1)
    {
        animation_slot = func_800839F8(record->unk3A, 0);
        if ((animation_slot != -1) && (func_80083EEC(record->unk3A, animation_slot, animation) != 0))
        {
            field_start_actor_animation(animation_slot, 0, 0);
            D_80105AE0[record->unk3A].flags.bytes.animation = animation_slot;
        }
    }
    if (event_id != -1)
    {
        func_800A3938(event_id, 0x80);
    }
    func_800B48B8(D_80105AE0[record->unk3A].unk14);
    D_80105AE0[record->unk3A].unkC = 0;
    runtime_flags = &D_80105AE0[record->unk3A];
    runtime_flags->flags.word = (s32) (runtime_flags->flags.word & ~0x20);
    resources = &D_80105AE0[record->unk3A];
    resources->unk8 = (s32) ((resources->unk8 & 0xFF000000) | (resources->unk0 & 0xFFFFFF));
    resources->unk4 = (s32) (resources->unk0 & 0xFFFFFF);
    D_80105AE0[record->unk3A].unk1AB = 0x3C;
    runtime_mode = &D_80105AE0[record->unk3A];
    runtime_mode->unk174 = (s32) (runtime_mode->unk174 | 0x8000);
    if (record->unk3A < 3U &&
        (record->unk0 <= -camera->x + 0xA00 ||
         record->unk0 >= -camera->x + 0x13600 ||
         record->unk8 <= -camera->z + 0xA00 ||
         record->unk8 >= -camera->z + 0x1B600))
         {
        slot = 0;
        unavailable = 0xFF;
        scan_state = D_80105AE0;
        candidate = D_800FDF58;
        for (; slot < 3; scan_state++, slot++, candidate++)

        {
            if (candidate->unk25 != unavailable && scan_state->unk4 != 0 && record->unk3A != slot)

            {
                break;
            }
        }
        if (slot != 3)
        {
            func_8008A0B0(record, slot, 1);
        }
    }
    return 0;
}
