#include "common.h"

/**
 * @file wmap_pathfinding.c
 * @brief World-map node reachability, queued node-swap state updates, and BFS
 *        route building. This is the overlay's -O0 island (built with
 *        gcc280_g0_o0_builtin); all three functions are contiguous in ROM.
 */

#define WMAP_NODE_RECORD_SIZE 12
#define WMAP_SET_BIT(index) D_800460AC[(index) / 32] = D_800460AC[(index) / 32] | (1 << ((index) % 32))

#define WMAP_ROUTE_NODE_COUNT 64
#define WMAP_ROUTE_UNVISITED_DISTANCE 100
#define WMAP_INVALID_NODE 0xFF
#define WMAP_NODE_X(index) ((u8)(D_800432C9[(index) * WMAP_NODE_RECORD_SIZE] & 0xF))
#define WMAP_NODE_Y(index) ((u8)(D_800432C9[(index) * WMAP_NODE_RECORD_SIZE] >> 4))

/**
 * @brief Packed world-map slot record: a 32-bit word whose low bits/nibbles hold
 *        flags and grid coordinates, aliased byte-wise and as bitfields.
 */
typedef union {
    u32 word;
    struct { u8 b0; u8 b1; u8 unk2; u8 unk3; } b;
    struct {
        u32 bit0 : 1;
        u32 bit1 : 1;
        u32 bit2 : 1;
        u32 pad3 : 5;
        u32 nib8 : 4;
        u32 nibC : 4;
        u32 hi : 16;
    } f;
} WmapSlot;

typedef struct { u8 unk0; u8 unk1; u8 unk2; u8 unk3; } WmapQuad;
typedef struct { u8 unk0; u8 unk1; } WmapPair;

extern int abs(int value);

s32 func_8005D670(s32 x, s32 y);

extern s32 D_800432BC;
extern u32 D_800432C0[];
extern u8 D_800432C8[];
extern u8 D_800432C9[];
extern u8 D_800432D0[];
extern WmapSlot D_800433DC;
extern WmapSlot D_80043388;
extern WmapQuad D_8004338C;
extern WmapQuad D_800433E0;
extern WmapPair D_80043392;
extern WmapPair D_800433E6;
extern u8 D_80043390;
extern u8 D_80043391;
extern s8 D_800433E4;
extern u8 D_800433E5;
extern s32 D_80043454;
extern u32 D_800460AC[];
extern s32 D_800460EC;
extern s32 D_800460FC;
extern u16* D_800D8FF8;
extern u8* D_800D8FFC;
extern u8 D_800D01B0[];
extern u8 D_800D01F0[];
extern u16 g_scene_mode;
extern s32 g_layout_flag;
extern s16 g_music_track_index;

/**
 * @brief Rebuild the world-map reachability bitmaps from the per-node flag bytes.
 */
void func_8005DBB8(void)
{
    s32 i;

    for (i = 0; i < 0x21; i++)
    {
        if ((D_800432C8[i * WMAP_NODE_RECORD_SIZE] >> 1) & 1)
        {
            WMAP_SET_BIT(i + 0x200);
        }
    }
    if (D_80043454 & 4)
    {
        D_800460EC |= 0x01000000;
    }
    for (i = 0; i < 0x21; i++)
    {
        if (D_800432C8[i * WMAP_NODE_RECORD_SIZE] & 1)
        {
            WMAP_SET_BIT(i + 0x240);
        }
        if ((D_800432C8[i * WMAP_NODE_RECORD_SIZE] >> 1) & 1)
        {
            WMAP_SET_BIT(i + 0x280);
        }
    }
    if (D_80043454 & 4)
    {
        D_800460FC |= 0x01000000;
    }
}

/**
 * @brief Resolve a world-map node from (x, y), publish its scene/layout/music,
 *        and sweep the pending-node bitmap performing the queued node swaps.
 *
 * Looks up the node index for (arg0, arg1), derives a slot from the node's
 * packed flag byte, and stores the scene mode, layout flag and music track for
 * that slot. It then scans the 0x40-bit pending mask; for each queued node id in
 * the accepted set it clears the pending bit, republishes that node's scene
 * data, and for ids 0xC / 0xD performs the swap of the two packed slot records
 * (D_800433DC and D_80043388) plus the four-neighbour reachability update.
 *
 * @param arg0 World-map cursor X coordinate passed to func_8005D670.
 * @param arg1 World-map cursor Y coordinate passed to func_8005D670.
 * @note Built with gcc280_g0_o0_builtin (-O0); matched 100% via MCP diff.
 */
void func_8005DF50(s32 arg0, s32 arg1)
{
    s32 sp10;
    s32 sp14;
    s32 sp18;
    s32 sp1C;
    s32 sp20;
    s32 sp24;

    func_8005DBB8();
    sp20 = func_8005D670(arg0, arg1);
    sp24 = (sp20 * 4) + ((D_800432C8[sp20 * WMAP_NODE_RECORD_SIZE] >> 4) & 3);
    g_scene_mode = D_800D8FF8[sp24];
    g_layout_flag = D_800D8FFC[sp24];
    g_music_track_index = func_8005D670(arg0, arg1);

    for (sp10 = 0; sp10 < 0x40; sp10++)
    {
        if (D_800432C0[sp10 / 32] & (1 << (sp10 % 32)))
        {
            if (sp10 == 9 || sp10 == 0xA || sp10 == 0xB || sp10 == 0xC || sp10 == 0xD || sp10 == 0xE || sp10 == 0x18 || sp10 == 0 || sp10 == 0x1B || sp10 == 0x1C)
            {
                D_800432C0[sp10 / 32] &= ~(1 << (sp10 % 32));
                g_music_track_index = D_800D01F0[sp10];
                sp20 = D_800D01B0[sp10];
                sp24 = (sp20 * 4) + ((D_800432C8[sp20 * WMAP_NODE_RECORD_SIZE] >> 4) & 3);
                g_scene_mode = D_800D8FF8[sp24];
                g_layout_flag = D_800D8FFC[sp24];
                if (sp10 == 0xC)
                {
                    D_80043390 = D_8004338C.unk0;
                    sp14 = (u8)((D_800433DC.word >> 8) & 0xF);
                    sp18 = (u8)((D_800433DC.word >> 0xC) & 0xF);
                    sp1C = func_8005D670(sp14 - 1, sp18);
                    D_80043388.f.bit0 = (u8)(D_800433DC.b.b0 & 1);
                    D_80043388.f.bit1 = (u8)((D_800433DC.word >> 1) & 1);
                    D_80043388.word = D_80043388.word | 4;
                    D_80043388.f.nib8 = (u8)((D_800433DC.word >> 8) & 0xF);
                    D_80043388.f.nibC = (u8)((D_800433DC.word >> 0xC) & 0xF);
                    D_80043388.b.unk2 = D_800433DC.b.unk2;
                    D_80043388.b.unk3 = D_800433DC.b.unk3;
                    D_8004338C.unk0 = D_800433E0.unk0;
                    D_8004338C.unk1 = D_800433E0.unk1;
                    D_8004338C.unk2 = D_800433E0.unk2;
                    D_8004338C.unk3 = D_800433E0.unk3;
                    D_80043391 = D_800433E5;
                    D_80043392.unk0 = D_800433E6.unk0;
                    D_80043392.unk1 = D_800433E6.unk1;
                    D_800433DC.word = D_800433DC.word & ~1;
                    D_800433DC.word = D_800433DC.word & ~2;
                    D_800433DC.word = D_800433DC.word & ~4;
                    D_800433DC.word = D_800433DC.word | 0xF00;
                    D_800433DC.word = D_800433DC.word | 0xF000;
                    D_800433DC.b.unk2 = 0;
                    D_800433DC.b.unk3 = 0;
                    D_800433E0.unk0 = 0;
                    D_800433E0.unk1 = 0;
                    D_800433E0.unk2 = 0;
                    D_800433E0.unk3 = 0;
                    D_800433E4 = 0;
                    D_800433E5 = 0;
                    D_800433E6.unk0 = 0;
                    D_800433E6.unk1 = 0;
                }
                if (sp10 == 0xD)
                {
                    D_800433DC.f.bit0 = (u8)(D_80043388.b.b0 & 1);
                    D_800433DC.f.bit1 = (u8)((D_80043388.word >> 1) & 1);
                    D_800433DC.word = D_800433DC.word & ~4;
                    D_800433DC.f.nib8 = (u8)((D_80043388.word >> 8) & 0xF);
                    D_800433DC.f.nibC = (u8)((D_80043388.word >> 0xC) & 0xF);
                    D_800433DC.b.unk2 = D_80043388.b.unk2;
                    D_800433DC.b.unk3 = D_80043388.b.unk3;
                    D_800433E0.unk0 = D_8004338C.unk0;
                    D_800433E0.unk1 = D_8004338C.unk1;
                    D_800433E0.unk2 = D_8004338C.unk2;
                    D_800433E0.unk3 = D_8004338C.unk3;
                    D_800433E4 = 6;
                    D_800433E5 = D_80043391;
                    D_800433E6.unk0 = D_80043392.unk0;
                    D_800433E6.unk1 = D_80043392.unk1;
                    D_8004338C.unk0 = D_80043390;
                    sp14 = (u8)((D_80043388.word >> 8) & 0xF);
                    sp18 = (u8)((D_80043388.word >> 0xC) & 0xF);
                    sp1C = func_8005D670(sp14 - 1, sp18);
                    if (sp1C != 0xFF)
                    {
                        D_8004338C.unk1 = D_800432D0[sp1C * WMAP_NODE_RECORD_SIZE];
                        if (D_800432D0[sp1C * WMAP_NODE_RECORD_SIZE] < 6)
                        {
                            D_800432D0[sp1C * WMAP_NODE_RECORD_SIZE] = 5;
                        }
                    }
                    sp1C = func_8005D670(sp14 + 1, sp18);
                    if (sp1C != 0xFF)
                    {
                        D_8004338C.unk2 = D_800432D0[sp1C * WMAP_NODE_RECORD_SIZE];
                        if (D_800432D0[sp1C * WMAP_NODE_RECORD_SIZE] < 6)
                        {
                            D_800432D0[sp1C * WMAP_NODE_RECORD_SIZE] = 5;
                        }
                    }
                    sp1C = func_8005D670(sp14, sp18 - 1);
                    if (sp1C != 0xFF)
                    {
                        D_8004338C.unk3 = D_800432D0[sp1C * WMAP_NODE_RECORD_SIZE];
                        if (D_800432D0[sp1C * WMAP_NODE_RECORD_SIZE] < 6)
                        {
                            D_800432D0[sp1C * WMAP_NODE_RECORD_SIZE] = 5;
                        }
                    }
                    sp1C = func_8005D670(sp14, sp18 + 1);
                    if (sp1C != 0xFF)
                    {
                        D_80043390 = D_800432D0[sp1C * WMAP_NODE_RECORD_SIZE];
                        if (D_800432D0[sp1C * WMAP_NODE_RECORD_SIZE] < 6)
                        {
                            D_800432D0[sp1C * WMAP_NODE_RECORD_SIZE] = 5;
                        }
                    }
                    D_80043388.word = D_80043388.word & ~1;
                    D_80043388.word = D_80043388.word & ~2;
                    D_80043388.word = D_80043388.word & ~4;
                    D_80043388.word = D_80043388.word | 0xF00;
                    D_80043388.word = D_80043388.word | 0xF000;
                    D_80043388.b.unk2 = 0;
                    D_80043388.b.unk3 = 0;
                }
            }
        }
    }
    D_800432BC |= 0x800000;
}

/**
 * @brief Build a world-map route between two grid coordinates.
 * @param start_x Starting world-map grid X coordinate.
 * @param start_y Starting world-map grid Y coordinate.
 * @param end_x Destination world-map grid X coordinate.
 * @param end_y Destination world-map grid Y coordinate.
 * @param out_x Output array receiving route X coordinates.
 * @param out_y Output array receiving route Y coordinates.
 */
void func_8005EB68(s32 start_x, s32 start_y, s32 end_x, s32 end_y, s32 *out_x, s32 *out_y)
{
    s32 i;
    s32 path_found;
    s32 current_x;
    s32 current_y;
    s32 distance;
    s32 node_index;
    s32 distances[WMAP_ROUTE_NODE_COUNT];
    s32 route[WMAP_ROUTE_NODE_COUNT];

    if (func_8005D670(start_x, start_y) == WMAP_INVALID_NODE)
    {
        current_x = start_x;
        current_y = start_y;
        i = 0;
        while (current_x != end_x || current_y != end_y)
        {
            out_x[i] = current_x;
            out_y[i] = current_y;

            if (abs(end_x - current_x) > abs(end_y - current_y))
            {
                current_x += (end_x - current_x) / abs(end_x - current_x);
            }
            else
            {
                current_y += (end_y - current_y) / abs(end_y - current_y);
            }

            i++;
        }

        out_x[i] = end_x;
        out_y[i] = end_y;
        return;
    }

    i = 0;
    while (i < WMAP_ROUTE_NODE_COUNT)
    {
        distances[i] = WMAP_ROUTE_UNVISITED_DISTANCE;
        i++;
    }

    distances[func_8005D670(start_x, start_y)] = 0;
    distance = 0;
    path_found = 0;

    while (path_found == 0)
    {
        i = 0;
        while (i < WMAP_ROUTE_NODE_COUNT)
        {
            if (distances[i] == distance)
            {
                node_index = func_8005D670(WMAP_NODE_X(i) - 1, WMAP_NODE_Y(i));
                if (node_index != WMAP_INVALID_NODE)
                {
                    if (distances[node_index] > distance + 1)
                    {
                        distances[node_index] = distance + 1;
                    }
                    if (node_index == func_8005D670(end_x, end_y))
                    {
                        i = WMAP_ROUTE_NODE_COUNT;
                        path_found = 1;
                    }
                }

                node_index = func_8005D670(WMAP_NODE_X(i) + 1, WMAP_NODE_Y(i));
                if (node_index != WMAP_INVALID_NODE)
                {
                    if (distances[node_index] > distance + 1)
                    {
                        distances[node_index] = distance + 1;
                    }
                    if (node_index == func_8005D670(end_x, end_y))
                    {
                        i = WMAP_ROUTE_NODE_COUNT;
                        path_found = 1;
                    }
                }

                node_index = func_8005D670(WMAP_NODE_X(i), WMAP_NODE_Y(i) - 1);
                if (node_index != WMAP_INVALID_NODE)
                {
                    if (distances[node_index] > distance + 1)
                    {
                        distances[node_index] = distance + 1;
                    }
                    if (node_index == func_8005D670(end_x, end_y))
                    {
                        i = WMAP_ROUTE_NODE_COUNT;
                        path_found = 1;
                    }
                }

                node_index = func_8005D670(WMAP_NODE_X(i), WMAP_NODE_Y(i) + 1);
                if (node_index != WMAP_INVALID_NODE)
                {
                    if (distances[node_index] > distance + 1)
                    {
                        distances[node_index] = distance + 1;
                    }
                    if (node_index == func_8005D670(end_x, end_y))
                    {
                        i = WMAP_ROUTE_NODE_COUNT;
                        path_found = 1;
                    }
                }
            }
            i++;
        }
        distance++;
    }

    distance = distances[func_8005D670(end_x, end_y)];
    route[distance] = func_8005D670(end_x, end_y);
    node_index = func_8005D670(end_x, end_y);

    while (distance >= 0)
    {
        i = 0;
        while (i < WMAP_ROUTE_NODE_COUNT)
        {
            if (distances[i] == distance - 1)
            {
                if ((WMAP_NODE_X(i) == WMAP_NODE_X(node_index) - 1) &&
                    (WMAP_NODE_Y(i) == WMAP_NODE_Y(node_index)))
                {
                    route[distance - 1] = i;
                    break;
                }
                if ((WMAP_NODE_X(i) == WMAP_NODE_X(node_index) + 1) &&
                    (WMAP_NODE_Y(i) == WMAP_NODE_Y(node_index)))
                {
                    route[distance - 1] = i;
                    break;
                }
                if ((WMAP_NODE_X(i) == WMAP_NODE_X(node_index)) &&
                    (WMAP_NODE_Y(i) == WMAP_NODE_Y(node_index) - 1))
                {
                    route[distance - 1] = i;
                    break;
                }
                if ((WMAP_NODE_X(i) == WMAP_NODE_X(node_index)) &&
                    (WMAP_NODE_Y(i) == WMAP_NODE_Y(node_index) + 1))
                {
                    route[distance - 1] = i;
                    break;
                }
            }
            i++;
        }
        distance--;
        node_index = route[distance];
    }

    i = 0;
    while (i <= distances[func_8005D670(end_x, end_y)])
    {
        out_x[i] = WMAP_NODE_X(route[i]);
        out_y[i] = WMAP_NODE_Y(route[i]);
        i++;
    }
}
