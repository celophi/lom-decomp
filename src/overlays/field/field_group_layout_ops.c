#include "common.h"

void func_800C3BD8(s32 arg0);


typedef struct
{
    u8 pad0[0x29D4];
    s32 unk29D4;
} Rec29D4;

typedef struct
{
    u8 pad0[0x29D7];
    s8 unk29D7;
} Rec29D7;

extern u8 g_menuLayoutBuffer[];
extern s32 D_80122C00;
extern s16 D_80122C06;
extern s16 D_80122C1A;
extern s8 D_800459AF;

/**
 * @brief Update or remap the active menu-layout slot state.
 * @param arg0 Dispatch selector; 0x92BC selects the slot-swap path.
 */
void func_800C3A00(s32 arg0)
{
    s32 i;
    s32 found;
    s8 ref;
    s32 idx;
    u8* p;
    u8* q;
    u8* slot_found;
    u8* slot_d;
    u8 v;
    s32 refv;

    i = 0;
    if (arg0 == 0x92BC)
    {
        idx = D_80122C00;
        if ((u32)idx < 3)
        {
            p = g_menuLayoutBuffer;
            ref = p[0x29D7];
            do
            {
                if (p[i + 0x29D8] == ref)
                {
                    found = i;
                }
                i++;
            } while (i < 3);

            q = g_menuLayoutBuffer;
            slot_d = idx + q;
            v = slot_d[0x29D8];
            slot_found = found + q;
            slot_found[0x29D8] = v;
            slot_d[0x29D8] = q[0x29D7];
            if (slot_found[0x29D8] == ((Rec29D7*)q)->unk29D7)
            {
                D_80122C06 = 3;
            }
            else
            {
                D_80122C06 = slot_found[0x29D8];
            }
            D_80122C1A = (s8)D_800459AF;
        }
    }
    else
    {
        for (; i < 0x15; i++)
        {
            g_menuLayoutBuffer[i + ((Rec29D7*)g_menuLayoutBuffer)->unk29D7 * 0x14C + 0x2B0C] = g_menuLayoutBuffer[i + 0xA90];
        }
        refv = ((Rec29D7*)g_menuLayoutBuffer)->unk29D7;
        if (refv != 3)
        {
            ((Rec29D4*)g_menuLayoutBuffer)->unk29D4 = (((Rec29D4*)g_menuLayoutBuffer)->unk29D4 & ~0xF0) | ((refv & 0xF) * 0x10);
        }
        g_menuLayoutBuffer[0x29D7] = 3;
    }
}


extern u8 g_menuLayoutBuffer[];
extern s8 D_800459AF;

extern void func_800C3BB0(void);
extern void func_800C3F18(s32 arg0, void *arg1);

void func_800C3B50(s32 arg0)
{
    if (arg0 == 3)
    {
        g_menuLayoutBuffer[0x29D7] = g_menuLayoutBuffer[0x29D4] >> 4;
    }
    else
    {
        D_800459AF = arg0;
    }

    func_800C3BB0();
    func_800C3F18((s8) g_menuLayoutBuffer[0x29D7], &g_menuLayoutBuffer[0xA90]);
}

void func_800C3BB0(void)
{
    func_800C3BD8(D_800459AF);
}


typedef union
{
    u32 raw;
    struct
    {
        unsigned mode : 2;
        unsigned pad2 : 14;
        unsigned enabled : 1;
        unsigned rest : 15;
    } bits;
} MenuWord;

extern u8 g_menuLayoutBuffer[];

void func_800C3CB4(void);
void func_800C3D38(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/**
 * @brief Clear menu layout state and process enabled entries for the requested mode.
 * @param arg0 Menu entry mode to process.
 */
void func_800C3BD8(s32 arg0)
{
    s32 i;
    u8 *v1;
    u8 *s1;
    u8 *s2;
    u32 a3;
    MenuWord word;

    i = 0;
    do
    {
        v1 = g_menuLayoutBuffer + i * 4;
        v1[0x2A7C] = 0;
        v1[0x2A7D] = 0;
        i += 1;
    } while (i < 0x24);

    if (arg0 != 3)
    {
        i = 0;
        if (g_menuLayoutBuffer[0x29D6] != 0)
        {
            s2 = g_menuLayoutBuffer;
            s1 = s2;
        loop:
            a3 = *(u32 *)(s1 + 0x29DC);
            word.raw = a3;
            if ((word.bits.enabled == 1) && (word.bits.mode == arg0))
            {
                func_800C3D38(i, (a3 >> 0x11) & 3, (s32)(a3 << 8) >> 0x1B, (s32)(a3 << 3) >> 0x1B);
            }
            s1 += 4;
            i += 1;
            if (i < s2[0x29D6])
            {
                goto loop;
            }
        }
        func_800C3CB4();
    }
}

/** @brief Marks grid cells whose lower neighbor belongs to another block. */
extern u8 g_menuLayoutBuffer[];
void func_800C3CB4(void)
{
    s32 i;
    s32 offset;
    s8 value;
    s32 sentinel;
    u8 *base;
    u8 *initial_base;
    u8 *p;
    u8 a;
    u8 b;

    value = 0x63;
    i = 0x23;
    initial_base = g_menuLayoutBuffer;

    do

    {
        p = initial_base + i * 4;
        p[0x2A7E] = value;
        i--;

    } while (i >= 0);

    i = 0;
    sentinel = 0x63;
    offset = 0x18;
    base = g_menuLayoutBuffer;
    do
    {
        p = base + i * 4;
        a = p[0x2A7F];
        if (a != sentinel)
        {
            b = ((u8 *)((u32)offset + (u32)base))[0x2A7F];
            value = i + 6;
            if (b != sentinel)
            {
                if (a != b)
                {
                    p = base + i * 4;
                    p[0x2A7E] = value;
                }
            }
        }
        offset += 4;
        i++;
    } while (i < 0x1E);
}


/** @brief Byte-aligned shape record with count and signed coordinate access fields. */
typedef struct
{
    u8 count;
    u8 pad[11];
    s8 x, y;
    u8 tail[74];
} Shape;
/** @brief Eleven shape records copied locally before updating layout cells. */
typedef struct
{
    Shape shapes[11];
} ShapeTable;
extern ShapeTable D_80051888;
extern u8 g_menuLayoutBuffer[];
/**
 * @brief Populate layout cells for the selected shape and rotation.
 * @param index Menu entry whose packed flags select the shape and cell metadata.
 * @param rotation Orientation selecting a twenty-byte coordinate block.
 * @param x Horizontal cell origin.
 * @param y Vertical cell origin in the six-column grid.
 */
void func_800C3D38(s32 index, s32 rotation, s32 x, s32 y)
{
    ShapeTable table;
    s32 offset, i, step;
    u8 *layout, *entry, *cell, *loop_layout;
    Shape *point;
    table = D_80051888;
    offset = index * 4;
    layout = g_menuLayoutBuffer;
    i = 0;
    if (table.shapes[(*(u32 *)(layout + offset + 0x29DC) >> 12) & 0xF].count != 0)
    {
        loop_layout = layout;
        step = rotation * 20;
        entry = loop_layout + 0x29DC + offset;
        do
        {
            point = (Shape *)((u8 *)&table + (step + ((*(u32 *)(entry) >> 12) & 0xF) * 88));
            cell = loop_layout + (x + point->x + (y + point->y) * 6) * 4;
            cell[0x2A7F] = index;
            cell[0x2A7C] = entry[0] >> 2;
            cell[0x2A7D] = (*(u32 *)(entry) >> 8) & 0xF;
            i++;
            step += 4;
        } while (i < table.shapes[(*(u32 *)(entry) >> 12) & 0xF].count);
    }
}
