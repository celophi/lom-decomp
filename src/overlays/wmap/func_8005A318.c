#include "common.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/** @brief World-map render context and primitive allocation cursor. */
typedef struct
{
    u8 _pad00[0x70];
    u_long ordering_table[0xB3];
    u8* packet_cursor;
} WmapRenderContext;

/** @brief World-map projection and animation state. */
typedef struct
{
    s32 x;
    s32 y;
    s32 projection_scale;
    s32 animation_offset;
    s32 texture_offset;
} WmapProjection;

/** @brief World-map cell record; remaining fields are not yet identified. */
typedef struct
{
    s32 object_id;
    u8 _pad04[0x24];
} WmapCell;

/** @brief Eight-byte world-map path entry. */
typedef struct
{
    s16 x;
    s16 y;
    u8 _pad04[4];
} WmapPath8;

/** @brief Twelve-byte world-map path entry. */
typedef struct
{
    s16 x;
    s16 y;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
} WmapPath12;

/** @brief World-map marker attributes and screen offsets. */
typedef struct
{
    u8 unk0;
    u8 _pad01;
    u8 unk2;
    u8 _pad03;
    u16 unk4;
    u16 unk6;
    u8 _pad08[8];
    s16 offset_x;
    s16 offset_y;
    s16 anchor_x;
    s16 anchor_y;
} WmapMarker;

extern POLY_FT4 D_800512E0;
extern u8 D_80051308[];
extern s16 D_80051338[3][3][2];
extern s16 D_80051384[3][3][2];
extern u8 D_800513A8[];
extern s8 D_80051B4C[];
extern s32 D_800CC130;
extern WmapMarker D_800CC164[];
extern WmapPath8 D_800CC804[];
extern WmapPath12 D_800CCCF8[];
extern s32 D_800CEFA8[];
extern WmapPath12 D_800CF0AC[];
extern s32 D_800D9168;
extern POLY_FT4 D_800D9170[];
extern s32 D_800D9214;
extern s32 D_800D9218;
extern s32 D_800D921C;
extern s32 D_800DCEC0;
extern s32 D_800DCEE0;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_800DCF08;
extern s32 D_800DCF0C;
extern s32 D_800DCF10;
extern s32 D_8011CF18;
extern s32 D_8011CF48;
extern s32 D_8011CF50;
extern s32 D_8011CF74;
extern s32 D_8011D4FC;
extern s32 D_8011D504;
extern s32 D_8011D508;
extern s32 D_8011D52C;
extern s32 D_80129558;
extern s32 D_801391E0;
extern SVECTOR D_80139278;
extern WmapCell D_80139290[][6];
extern s32 D_80139838[];
extern s32 D_8013986C;
extern s32 D_801398D0;
extern WmapRenderContext* D_801398EC;
extern WmapProjection D_80139950;
extern s32 D_8013B208;
extern s32 D_8013B230;
extern s32 D_8013B248;
extern s32 D_8013B250;
extern s32 D_80182234;
extern s32 D_8018223C;
extern VECTOR D_80182DC0;
extern s32 D_80182DDC;
extern s32 D_80182DE0;
extern s32 D_80182E24;
extern s32 D_80182E30;
extern s32 D_801ADAE0;

extern void func_8005B540(void);
extern void func_8005FF88(s32, s32, s32, s32);
extern void func_800652A8(s32, s32, s32, s32);
extern void func_8006534C(s32, s32, void*, void*);

/**
 * @brief Render world-map markers and animated selection effects.
 * @note Project object comparison: 90.633390% matching.
 * @note In-progress import; source filename reports 90.633390% matching.
 */
void func_8005A318(void)
{
    SVECTOR position;
    MATRIX matrix;
    DVECTOR screen;
    DVECTOR* screen_ptr;
    WmapPath8* path8;
    WmapPath12* path12_case3;
    WmapPath12* path12_case4;
    WmapMarker* anchor_marker;
    WmapMarker* sprite_marker;
    POLY_FT4* packet2;
    s32 counter;
    s32 ot_offset;
    s32 cell_x;
    s32 cell_y;
    s32 object_id;
    s32 screen_y;
    s32 screen_x;
    s32 value;
    s16 sound_id;
    s32 phase;
    s32 temp;

    func_8005B540();
    ot_offset = 0;

    if ((D_8013B208 == 0) || (D_801ADAE0 == 0))
    {
        counter = D_8013B230 + 1;
        D_8013B230 = counter;

        if (D_80182DE0 == 0)
        {
            phase = D_80051B4C[(counter & 0xFF) + 0x20];
            if (phase < 0)
            {
                phase += 0x1F;
            }
            phase >>= 5;
            D_8011CF48 = phase;
        }
        else
        {
            if (D_8011CF48 > 0)
            {
                D_8011CF48--;
            }
            if (D_8011CF48 < 0)
            {
                D_8011CF48++;
            }
        }

        if (D_8011CF18 >= 2)
        {
            D_8011CF48 = 0;
        }

        if (D_801398D0 == 2)
        {
            RotMatrix(&D_80139278, &matrix);
            TransMatrix(&matrix, &D_80182DC0);
            SetRotMatrix(&matrix);
            SetTransMatrix(&matrix);

            position.vz = 0;
            position.vx = ((D_80139950.x * 0x14000) / D_80139950.projection_scale * 0x6000) / D_80139950.projection_scale;
            position.vy = ((D_80139950.y * 0x14000) / D_80139950.projection_scale * 0x6000) / D_80139950.projection_scale;

            gte_ldv0(&position);
            gte_rtps();
            screen_ptr = &screen;
            gte_stsxy(screen_ptr);

            D_8013B248 = D_80182234 - screen.vx;
            D_8013B250 = D_8018223C - (s16)(u16)screen_ptr->vy;
        }
        else
        {
            D_8013B250 = 0;
            D_8013B248 = 0;
        }

        switch (D_8011CF18)
        {
            case 0:
                D_800DCF08 = D_80051338[D_800DCEF0][D_800DCEEC][0];
                D_800DCF10 = D_80051338[D_800DCEF0][D_800DCEEC][1];
                break;

            case 1:
                break;

            case 2:
                path8 = D_800CC804;
                path8 += D_80182DDC;
                D_800DCF08 = path8->x;
                D_800DCF10 = path8->y;
                ot_offset = 7;
                break;

            case 3:
                temp = D_80182DDC;
                if (D_800D9218 == temp)
                {
                    D_800DCF08 = D_800CC804[temp].x;
                    D_800DCF10 = D_800CC804[temp].y;

                    if (D_8011D4FC == -1)
                    {
                        D_8011CF18 = 2;
                        D_8011CF50 = 0;
                        D_800D9168 = D_8011D4FC;
                        D_80182E30 = 0;
                        D_800DCEE0 = 0;
                    }
                    else if (D_800D9168 == -1)
                    {
                        D_800D9168 = D_800CEFA8[D_8011D4FC];
                        D_800DCF0C = D_800CEFA8[D_8011D4FC + 1] - 1;
                    }
                    else if (D_800D9168 == D_800DCF0C)
                    {
                        func_800652A8(D_800CF0AC[D_800DCF0C].unk8, 0x98, D_800DCF0C, D_8011D4FC);
                        D_8011CF18 = 2;
                        D_8011CF50 = 0;
                        D_80182E30 = 0;
                        D_800D9168 = -1;
                        D_800DCEE0 = 0;
                        D_8011D4FC = -1;
                    }
                    else
                    {
                        path12_case3 = D_800CF0AC;
                        path12_case3 += D_800D9168;
                        D_800DCF10 = path12_case3->y;
                        counter = D_800D9168 + 1;
                        D_800D9168 = counter;
                        D_800DCF08 = path12_case3->x;
                        D_800DCEE0 = path12_case3->unk4;
                        D_80182E30 = path12_case3->unk6;
                        D_800D9214 = path12_case3->unkA;

                        if (counter == D_800DCF0C)
                        {
                            s32* route_slots;
                            value = D_800CC130;
                            route_slots = D_80139838;
                            if (value < 0)
                            {
                                value += 3;
                            }
                            D_80182E24 = 0;
                            route_slots[((value >> 2) + 0x2B) % 12] = D_8011D4FC;
                        }
                    }
                }
                else
                {
                    D_80182DDC = temp + 1;
                    D_800DCF08 = D_800CC804[temp].x;
                    D_800DCF10 = D_800CC804[temp].y;
                }
                break;

            case 4:
                path8 = D_800CC804;
                path8 += D_80182DDC;
                D_800DCF08 = path8->x;
                D_800DCF10 = path8->y;

                if (D_8011D4FC == -1)
                {
                    D_80182E30 = 0;
                    D_80182E24 = 0;
                }

                if (D_800D9218 == D_80182DDC)
                {
                    D_8011CF18 = 0;
                    D_8011CF50 = 0;
                }
                else if (D_800D9168 == -1)
                {
                    D_80182E24 = 1;
                    D_80182DDC--;
                    D_800DCF08 = path8->x;
                    D_800DCF10 = path8->y;
                    if (D_8011D4FC != -1)
                    {
                        s32* route_slots;
                        value = D_800CC130;
                        route_slots = D_80139838;
                        if (value < 0)
                        {
                            value += 3;
                        }
                        route_slots[((value >> 2) + 0x2B) % 12] = -1;
                    }
                }
                else
                {
                    path12_case4 = D_800CCCF8;
                    path12_case4 += D_800D9168;
                    D_800DCF10 = path12_case4->y;
                    D_800DCF08 = path12_case4->x;
                    D_800DCEE0 = path12_case4->unk4;
                    D_80182E30 = path12_case4->unk6;
                    sound_id = path12_case4->unk8;
                    D_800D9214 = path12_case4->unkA;
                    if (sound_id != -1)
                    {
                        func_800652A8(sound_id, 0x8F, D_80182DDC, D_8011D4FC);
                    }
                    ot_offset = 7;
                    counter = D_800D9168 + 1;
                    D_800D9168 = counter;
                    if (D_800DCF0C == counter)
                    {
                        D_800D9168 = -1;
                    }
                }
                break;
        }

        D_80129558 = D_800DCF08;
        D_801391E0 = D_800DCF10;

        if ((D_8013B208 == 0) && (D_8013986C == 0))
        {
            POLY_FT4* packet;
            u8 x0_offset;
            u8 y0_offset;
            u8 x1_offset;
            u8 y1_offset;
            u8 x2_offset;
            u8 y2_offset;
            u8 x3_offset;
            u8 y3_offset;

            packet = (POLY_FT4*)D_801398EC->packet_cursor;
            *packet = D_800512E0;

            if (D_8011D4FC == -1)
            {
                screen_x = D_80129558;
                screen_y = D_801391E0;
            }
            else
            {
                screen_x = D_80129558 + D_800CC164[D_8011D4FC].offset_x;
                screen_y = D_801391E0 + D_800CC164[D_8011D4FC].offset_y;
            }

            screen_y += D_80182DE0 + D_8011CF48;
            screen_x += D_8013B248;
            screen_y += D_8013B250;

            x0_offset = D_800513A8[D_800DCEE0 * 8 + 0];
            y0_offset = D_800513A8[D_800DCEE0 * 8 + 1];
            x1_offset = D_800513A8[D_800DCEE0 * 8 + 2];
            y1_offset = D_800513A8[D_800DCEE0 * 8 + 3];
            x2_offset = D_800513A8[D_800DCEE0 * 8 + 4];
            y2_offset = D_800513A8[D_800DCEE0 * 8 + 5];
            x3_offset = D_800513A8[D_800DCEE0 * 8 + 6];
            y3_offset = D_800513A8[D_800DCEE0 * 8 + 7];

            packet->x1 = (s8)x1_offset + screen_x;
            packet->x0 = (s8)x0_offset + screen_x;
            packet->y0 = (s8)y0_offset + screen_y;

            *(u16*)&packet->u0 = *(u16*)&D_80051308[D_80182E30 * 8];
            packet->u1 = packet->u0 + D_80051308[(D_80182E30 * 8) | 2];
            packet->v1 = packet->v0;
            packet->u2 = packet->u0;
            packet->v2 = packet->v0 + D_80051308[(D_80182E30 * 8) | 3];

            packet->y1 = (s8)y1_offset + screen_y;
            packet->x2 = (s8)x2_offset + screen_x;
            packet->y2 = (s8)y2_offset + screen_y;
            packet->x3 = (s8)x3_offset + screen_x;
            packet->y3 = (s8)y3_offset + screen_y;

            packet->u3 = packet->u0 + D_80051308[(D_80182E30 * 8) | 2];
            packet->v3 = packet->v0 + D_80051308[(D_80182E30 * 8) | 3];

            if (D_80182E30 != 0)
            {
                if (D_800D9214 != 0)
                {
                    addPrim(&D_801398EC->ordering_table[5], packet);
                }
                else
                {
                    addPrim(&D_801398EC->ordering_table[6 + ot_offset], packet);
                }

                D_800D9170[(D_8011CF74 & 1) * 2] = *packet;

                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += sizeof(POLY_FT4);
                    D_801398EC->packet_cursor += sizeof(POLY_FT4);
                }

                packet2 = (POLY_FT4*)D_801398EC->packet_cursor;
                *packet2 = *packet;
                packet2->v0 += 0x20;
                packet2->v1 += 0x20;
                packet2->v2 += 0x20;
                packet2->v3 += 0x20;
                addPrim(&D_801398EC->ordering_table[4 + ot_offset], packet2);
                D_800D9170[(D_8011CF74 & 1) * 2 + 1] = *packet2;

                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += sizeof(POLY_FT4);
                    D_801398EC->packet_cursor += sizeof(POLY_FT4);
                }
            }
            else
            {
                addPrim(&D_801398EC->ordering_table[4 + ot_offset], packet);
                D_800D9170[(D_8011CF74 & 1) * 2] = *packet;
                D_800D9170[(D_8011CF74 & 1) * 2 + 1] = *packet;

                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += sizeof(POLY_FT4);
                    D_801398EC->packet_cursor += sizeof(POLY_FT4);
                }
            }
        }

        if ((D_801ADAE0 == 0) && (D_8013986C == 0))
        {
            if (((D_8011D4FC != -1) && (D_80182E24 != 0)) || (D_8011D52C != 0))
            {
                SPRT* sprite;

                sprite = (SPRT*)D_801398EC->packet_cursor;
                if ((D_8011D52C != 0) && (D_801398D0 != 2))
                {
                    sprite->x0 = D_8011D504;
                    sprite->y0 = D_8011D508;
                }
                else
                {
                    anchor_marker = &D_800CC164[D_8011D4FC];
                    sprite->x0 = (u16)D_8013B248 + ((u16)D_80129558 + anchor_marker->anchor_x);
                    D_8011D504 = (s16)sprite->x0;
                    sprite->y0 = (u16)D_8013B250 + ((u16)D_8011CF48 + ((u16)D_80182DE0 + ((u16)D_801391E0 + anchor_marker->anchor_y)));
                    D_8011D508 = (s16)sprite->y0;
                }

                sprite_marker = &D_800CC164[D_8011D4FC];
                sprite->w = sprite_marker->unk4;
                sprite->h = sprite_marker->unk6;
                sprite->u0 = sprite_marker->unk0;
                sprite->v0 = D_800CC164[D_8011D4FC].unk2;
                sprite->clut = 0x7FEC;
                *(u32*)&sprite->r0 = 0x80808080;
                setSprt(sprite);
                setSemiTrans(sprite, 1);
                addPrim(&D_801398EC->ordering_table[5], sprite);

                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += sizeof(SPRT);
                    D_801398EC->packet_cursor += sizeof(SPRT);
                }
                func_8006534C(0xAE, 5, D_801398EC, sprite);
            }

            cell_x = (D_80139950.x / 48) + D_800DCEEC;
            cell_y = (D_80139950.y / 48) + D_800DCEF0;
            if ((D_8011CF18 == 0) && (D_800DCEC0 != 0))
            {
                object_id = D_80139290[cell_x][cell_y].object_id;
                if (object_id == 0xFF)
                {
                    object_id = -1;
                }
                func_8005FF88(object_id, D_80139950.y >> 31, D_80139950.y / 6, D_80139950.x / 6);
            }

            if ((D_80139290[cell_x][cell_y].object_id == 0xFF) && (D_8011CF18 == 0) && (D_801398D0 != 2) && (D_8013B208 == 0))
            {
                POLY_FT4* packet;
                packet = (POLY_FT4*)D_801398EC->packet_cursor;
                *(u32*)&packet->r0 = 0x00808080;
                packet->clut = 0x7F2D;
                packet->u3 = 0x20;
                packet->u1 = 0x20;
                packet->v1 = 0xC0;
                packet->v0 = 0xC0;
                packet->v3 = 0xE0;
                packet->v2 = 0xE0;
                packet->u2 = 0;
                packet->u0 = 0;
                packet->tpage = 0x5B;
                setPolyFT4(packet);
                setSemiTrans(packet, 1);
                *(u32*)&packet->x0 = *(u32*)&D_80051384[D_800DCEF0][D_800DCEEC][0];
                packet->x2 = packet->x0;
                packet->x3 = packet->x0 + 0x20;
                packet->x1 = packet->x3;
                packet->y1 = packet->y0;
                packet->y3 = packet->y0 + 0x20;
                packet->y2 = packet->y3;
                addPrim(&D_801398EC->ordering_table[0xAD], packet);

                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += sizeof(POLY_FT4);
                    D_801398EC->packet_cursor += sizeof(POLY_FT4);
                }
            }
        }
    }
}
