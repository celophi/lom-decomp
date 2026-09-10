#include "common.h"
#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/** @brief Actor record position and state selectors, with the original 0x54-byte stride. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 padc[0x25-12];
    u8 unk25;
    u8 pad26[4];
    s16 unk2a;
    u8 pad2c[11];
    s8 unk37;
    u8 pad38[2];
    u8 unk3a;
    u8 unk3b;
    u8 pad3c[0x18];
} Record;

/** @brief Runtime actor collision and animation fields, with the original 0x23C-byte stride. */
typedef struct
{
    u32 unk0;
    u32 unk4;
    u32 unk8;
    u32 unkc;
    u8 pad10[0x12C-16];
    /** @brief Word and halfword views of the packed collision dimensions. */
    union
    {
        u32 word;
        /** @brief Signed center offset and packed unsigned diameter. */
        struct
        {
            s16 offset;
            u16 diameter;
        } h;
    } collision;
    u8 pad130[0x142-0x130];
    s16 unk142;
    u8 pad144[2];
    s16 unk146;
    u8 pad148[0x178-0x148];
    u32 unk178;
    u8 pad17c[0x23C-0x17C];
} State;

/** @brief Resource classification and reaction flags, with the original 0x14-byte stride. */
typedef struct
{
    u8 pad0[8];
    u8 unk8;
    u8 pad9[7];
    u32 unk10;
} Resource;

/** @brief Actor animation request, with the original 0x1C-byte stride. */
typedef struct
{
    s32 unk0;
    u8 pad4[8];
    s32 unkc;
    u8 pad10[12];
} Request;

/** @brief Animation target list and GTE vectors at their original stack offsets. */
typedef struct
{
    s32 targets[2];
    VECTOR delta;
    VECTOR squared;
} ScanWorkspace;

extern Record D_800FDF58[];
extern Record D_800FE054[];
extern State D_80105AE0[];
extern State D_80106194[];
extern Request D_80105880[];
extern Resource g_field_resource_entries[];
extern s32 D_8010D020,D_8010D024;
s32 func_800839F8(s32,s32);
s32 func_80083EEC(s32,s32,s32);
void func_800A2DD8(s32);
void field_start_actor_animation(s32,s32,s32 *);
#define READ_S16(p,o) (*(s16 *)((u8 *)(p)+(o)))
#define READ_U16(p,o) (*(u16 *)((u8 *)(p)+(o)))
#define READ_S32(p,o) (*(s32 *)((u8 *)(p)+(o)))
#define READ_U32(p,o) (*(u32 *)((u8 *)(p)+(o)))
#define READ_S8(p,o) (*(s8 *)((u8 *)(p)+(o)))
#define READ_U8(p,o) (*(u8 *)((u8 *)(p)+(o)))

/**
 * @brief Find an overlapping actor and optionally start its contact reaction.
 * @param record Actor whose collision dimensions and resource flags are tested.
 * @param position Position to test against the selected actor group.
 * @param filter_group Nonzero to select the opposing group when filtering is enabled.
 * @return Candidate index plus 0x8000, or zero for no candidate or a handled reaction.
 * @note The SDK GTE macros preserve the original squared-distance calculation.
 */
s32 func_800987DC(Record *record, Record *position, s32 filter_group)
{

    ScanWorkspace scratch;
    u8 *request_base;
    s32 record_offset;
    State *scan_state;
    u8 *scan_position;
    Record *scan_record;
    s32 record_y;
    s32 scan_y;
    s32 animation_slot;
    s32 scan_flags;
    s32 actor_index;
    s32 candidate_index;
    s32 candidate_end;
    s32 unused_result;
    s32 request_offset;
    s32 matched_request_offset;
    s8 record_height;
    s8 scan_height;
    State *record_state;
    u8 *scan_dimensions;

    if (filter_group != 0)
    {

        if (D_8010D020 == 0)
        {

            if ((u8) record->unk3a < 3U)
            {

                scan_record = D_800FE054;
                scan_state = D_80106194;
                actor_index = 3;
                goto scan_to_last;
            }
            scan_record = D_800FDF58;
            scan_state = D_80105AE0;
            actor_index = 0;
            candidate_end = 3;
        }
        else
        {
            goto scan_all;
        }
    }
    else
    {
scan_all:
        scan_record = D_800FDF58;
        scan_state = D_80105AE0;
        actor_index = 0;
scan_to_last:
        candidate_end = 0xD;
    }
    candidate_index = actor_index;
    if (record->unk37 >= 9)
    {

        return 0;
    }
    record_state = &D_80105AE0[record->unk3a];
    record_offset = record_state->collision.h.offset << 8;
    if (candidate_index < candidate_end)
    {

        scan_dimensions = (u8 *)scan_state + 0x12E;
        scan_position = (u8 *)scan_record + 8;
scan_next:
        if ((READ_U8(scan_position, 0x1D) == 0xFF) ||
        (READ_U32(scan_dimensions, -0x122) & 0x23E4) ||
        (scan_record == record) ||
        (scan_flags = READ_U32(scan_dimensions, 0x4A), ((scan_flags & 0x20) != 0)) ||
        (scan_flags & 1) ||
        (READ_S32(scan_dimensions, -2) == 0) ||
        (scan_height = READ_S8(scan_position, 0x2F), scan_y = READ_S32(scan_position, -4), record_height = record->unk37, record_y = position->unk4, (((scan_y + ((READ_S16(scan_dimensions, 0x14) + scan_height) << 8)) > (record_y + ((record_state->unk146 + record_height) << 8))) != 0)) ||
        ((scan_y + ((READ_S16(scan_dimensions, 0x18) + scan_height) << 8)) < (record_y + ((record_state->unk142 + record_height) << 8))))
        {

            goto advance_candidate;
        }
        scratch.delta.vx = (READ_S32(scan_position, 0) - position->unk8) >> 8;
        scratch.delta.vz = 0;
        scratch.delta.vy = ((scan_record->unk0 + (READ_S16(scan_dimensions, -2) << 8)) - (position->unk0 + record_offset)) >> 8;
        gte_ldlvl(&scratch.delta);
        gte_sqr0();
        gte_stlvnl(&scratch.squared);
        if (SquareRoot0(scratch.squared.vx + scratch.squared.vy) >= (((s32)(record_state->collision.h.diameter << 16) >> 17) + ((s32)(READ_U16(scan_dimensions, 0) << 16) >> 17)))
        {

advance_candidate:
            candidate_index += 1;
            scan_position += 0x54;
            scan_record += 1;
            scan_dimensions += 0x23C;
            scan_state += 1;
            if (candidate_index < candidate_end)
            {

                goto scan_next;
            }
        }
    }
    if (candidate_index == candidate_end)
    {

        D_8010D024 = 0;
        goto return_zero;
    }
    if ((&g_field_resource_entries[record->unk3b])->unk8 != 0)
    {

        if ((&g_field_resource_entries[scan_record->unk3b])->unk10 & 1)
        {

            if (scan_state->unk4 != 0)
            {

                if (!(scan_state->unkc & 0x280))
                {

                    if (!(scan_state->unk178 & 1))
                    {

                        actor_index = scan_record->unk3a;
                        if (!(((u32) (&D_80105AE0[actor_index])->unk178 >> 6) & 1))
                        {

                            request_base = (u8 *)D_80105880;
                            if (actor_index < 2U)
                            {

                                request_offset = actor_index * 0x1C;
                            }
                            else
                            {
                                request_offset = 0x38;
                            }
                            actor_index = ((Request *)(request_base + request_offset))->unkc;
                            if (actor_index == scan_record->unk3a)
                            {

                                request_base = (u8 *)D_80105880;
                                if ((u32) (actor_index & 0xFF) < 2U)
                                {

                                    matched_request_offset = actor_index * 0x1C;
                                }
                                else
                                {
                                    matched_request_offset = 0x38;
                                }

                                if (((Request *)(request_base + matched_request_offset))->unk0 == 0)
                                {

                                    goto start_reaction;
                                }
                                return 0;
                            }
                            goto start_reaction;
                        }
start_reaction:

                        if (scan_record->unk2a == 0)
                        {

                            animation_slot = func_800839F8(scan_record->unk3a, 0);
                            if (animation_slot != -1)
                            {

                                scratch.targets[0] = (s32) record->unk3a;

                                if (func_80083EEC(scan_record->unk3a, animation_slot, 0x2A) != 0)
                                {

                                    if ((u8) scan_record->unk3a < 2U)
                                    {

                                        func_800A2DD8(scan_record->unk3a);
                                    }
                                    field_start_actor_animation(animation_slot, 1, &scratch.targets[0]);
                                    return 0;
                                }
                                return 0;
                            }
return_zero:
                            return 0;
                        }
                        return 0;
                    }
                }
            }
            return 0;
        }
        goto report_candidate;
    }
report_candidate:
    D_8010D024 = candidate_index + 0x8000;
    return D_8010D024;
}
