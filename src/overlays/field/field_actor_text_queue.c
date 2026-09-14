#include "common.h"
#include "sdk/libgpu.h"

/** @brief Text pointer and packed position/countdown state for one text slot. */
typedef struct Slot
{
    u8 *text;
    u32 flags;
} Slot;

/** @brief Fixed-point position at the start of a 0x54-byte actor entry. */
typedef struct Position
{
    s32 x, y, z;
    u8 rest[0x54 - 12];
} Position;

/** @brief Two-word view of a pending text entry used by the frame handler. */
typedef struct
{
    u32 unk0;   /* 0x0 */
    u32 unk4;   /* 0x4 */
} ArgB;

/** @brief FIELD context passed through to the text entry handler. */
typedef struct
{
    u8 pad0[0x40];
    u8 unk40;   /* 0x40 */
    u8 pad41[0x40B8 - 0x41];
    s32 unk40B8; /* 0x40B8 */
} ArgA;

extern Position D_800FDF58[];
extern u8 D_800ED064[];
extern u8 *g_pad_ctx;
extern s32 D_800F22A0, D_800F22A4, D_800F22A8;
extern s32 D_801227C8;
extern s32 D_801227DC;

extern s32 func_800AE864(u8 *);
void field_text_reset_scratch(void);
void field_text_reset_windows(void);
void func_80063194(void);
void func_800A6634(ArgA *arg0, ArgB *arg1);

s32 field_text_build_sprites(SPRT *prim, u8 *text, s32 style);
SPRT *func_800AD658(s32 *ordering_table, SPRT *sprite_cursor, s32 count);

/**
 * @brief Clear the six-bit frame countdown of the first three text entries.
 */
void func_800A6204(void)
{
    extern s32 D_801226A0[];

    D_801226A0[5] &= 0xFF81FFFF;
    D_801226A0[3] &= 0xFF81FFFF;
    D_801226A0[1] &= 0xFF81FFFF;
}

/**
 * @brief Start an inactive text slot near its actor and clamp its screen position.
 * @param arg0 Actor/text slot index; indices at least two are ignored.
 * @param arg1 Text table index, or a negative packed selector into the pad context.
 */
void func_800A623C(s32 arg0, s32 arg1)
{
    extern Slot D_801226A0[];

    s16 point[2];
    Slot *slot;
    Slot *initial;
    u8 *initial_base;
    u8 *actor_base;
    Slot *output;
    Slot *base;
    Position *actor;
    s32 offset;
    s32 xoff, x, y, width;
    s32 screen_y;
    s32 actor_screen_x;
    s32 actor_screen_y;

    s32 first;
    if (arg0 < 2)
    {
        initial_base = (u8 *)D_801226A0;
        initial = (Slot *)(arg0 * 8 + initial_base);
        if (!((initial->flags >> 17) & 0x3F))
        {
            do
            {
                first = arg1 * 2;
                if (arg1 < 0)
                {
                    s32 second;
                    s32 selector;
                    u8 **pad_context_ptr;
                    do
                    {
                        pad_context_ptr = &g_pad_ctx;
                    } while (0);
                    selector = (u32)arg1 >> 16;
                    selector &= 0xFF;
                    second = selector * 0x250 + 0x5F0;
                    first = (s32)*pad_context_ptr + second;
                    second = ((arg1 & 0xFF) << 6) + 0x150;
                    first += second;
                    initial->text = (u8 *)first;
                }
                else
                {
                    s32 second;
                    second = (u32)D_800ED064;
                    first = *(u16 *)(first + second);
                    do
                    {
                        first += second;
                        initial->text = (u8 *)first;
                    } while (0);
                }
            } while (0);
            xoff = D_800F22A0;
            base = D_801226A0;
            offset = arg0 * 8;
            slot = (Slot *)(offset + (u8 *)base);
            slot->flags |= 0x7E0000;
            if (xoff < 0)
            {
                xoff += 255;
            }
            actor_base = (u8 *)D_800FDF58;
            actor = (Position *)(arg0 * 0x54 + actor_base);
            x = actor->x;
            if (x < 0)
            {
                x += 255;
            }
            arg0 = xoff >> 8;
            xoff = D_800F22A4;
            actor_screen_x = (x >> 8) + 160;
            point[0] = arg0 + actor_screen_x;
            if (xoff < 0)
            {
                xoff += 255;
            }
            x = xoff >> 8;
            y = actor->y;
            if (y < 0)
            {
                y += 255;
            }
            xoff = actor->z;
            actor_screen_y = (y >> 8) + 112;
            screen_y = x + actor_screen_y;
            if (xoff < 0)
            {
                xoff += 511;
            }
            x = D_800F22A8;
            xoff = screen_y - (xoff >> 9);
            if (x < 0)
            {
                x += 511;
            }
            point[1] = xoff - (x >> 9);
            width = func_800AE864(slot->text) * 6;
            if (point[0] + width >= 321)
            {
                point[0] = 320 - width;
            }
            if (point[0] - width - 8 < 0)
            {
                point[0] = width + 8;
            }
            if (point[1] >= 177)
            {
                point[1] = 176;
            }
            if (point[1] < 50)
            {
                point[1] = 50;
            }
            output = (Slot *)((s32)offset + (s32)base);
            {
                s32 point_x;
                point_x = point[0];
                output->flags = (output->flags & ~0x1FF) | (point_x & 0x1FF);
            }
            output->flags = (output->flags & 0xFFFE01FF) | (((point[1] + 4) & 0xFF) << 9);
        }
    }
}

/**
 * @brief Report whether any of the first three text entries has a pending countdown.
 * @return 1 if an entry's six-bit countdown field is nonzero, otherwise 0.
 */
s32 func_800A6490(void)
{
    typedef struct
    {
        s32 unk0;
        s32 unk4;
    } UnkStruct801226A0;
    extern UnkStruct801226A0 D_801226A0[];

    s32 i;
    UnkStruct801226A0 *p;

    i = 0;
    p = D_801226A0;
    for (; i < 3; i++, p++)
    {
        if (((u32)p->unk4 >> 17) & 0x3F)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Process pending text entries, or clear their countdowns when disabled.
 * @param arg0 FIELD context forwarded to each active entry's handler.
 * @note The previous active count controls window cleanup on the next frame.
 */
void func_800A64D0(ArgA *arg0)
{
    typedef struct
    {
        s32 unk0;
        u32 low : 17;
        u32 count : 6;
        u32 high : 9;
    } UnkStruct801226A0;
    extern UnkStruct801226A0 D_801226A0[];

    s32 i;
    s32 active_count;

    active_count = 0;
    if (D_801227C8 != 0)
    {
        i = 0;
        for (; i < 3; i++)
        {
            D_801226A0[i].count = 0;
        }
        D_801227DC = 0;
        return;
    }

    for (i = 0; i < 3; i++)
    {
        if (D_801226A0[i].count)
        {
            active_count += 1;
        }
    }

    if (active_count != 0)
    {
        field_text_reset_scratch();
        for (i = 0; i < 3; i++)
        {
            if (D_801226A0[i].count)
            {
                func_800A6634(arg0, &D_801226A0[i]);
                D_801226A0[i].count--;
            }
        }
        func_80063194();
    }
    else if (D_801227DC != 0)
    {
        field_text_reset_windows();
    }
    D_801227DC = active_count;
}

/**
 * @brief Schedule the next text draw for an active entry and store its handle.
 * @param arg0 FIELD context holding the scratch base and current draw handle.
 * @param arg1 Pending text entry supplying the packed position and text pointer.
 * @see decomp.me (100%) TODO
 */
void func_800A6634(ArgA *arg0, ArgB *arg1)
{
    s32 count;
    s32 val;
    u32 field4;
    s32 handle;
    void *p40;

    count = 0x100;
    handle = arg0->unk40B8;
    field4 = arg1->unk4;
    val = (field4 >> 0x11) & 0x3F;
    p40 = &arg0->unk40;
    if (val < 0x20)
    {
        count = val * 4;
    }
    arg0->unk40B8 = func_800A66B4(
        handle,
        p40,
        arg1->unk0,
        4,
        field4 & 0x1FF,
        (arg1->unk4 >> 9) & 0xFF,
        2,
        count);
}

/**
 * @brief Builds and links text sprite primitives into an ordering table.
 * @param sprite_cursor Primitive-buffer cursor used for generated sprites.
 * @param ordering_table Ordering-table entry that receives the generated primitives.
 * @param text Null-terminated text to render.
 * @param text_color Text style passed to the glyph builder.
 * @param x Horizontal origin used to position the rendered text.
 * @param y Vertical origin used to position the rendered text.
 * @param alignment Horizontal alignment mode for the generated glyphs.
 * @param color Sprite tint value, or 0x100 to use the neutral tint.
 * @return Primitive-buffer cursor immediately after the generated draw commands.
 */
void* func_800A66B4(SPRT* sprite_cursor, s32* ordering_table, u8* text, s32 text_color, s32 x, s32 y, s32 alignment, s32 color)
{
    s32 n, count, i, acc;
    SPRT* sprite;
    DR_TPAGE* tpage;

    if (*text == 0)
    {
        return sprite_cursor;
    }

    n = field_text_build_sprites(sprite_cursor, text, text_color);
    count = n;

    if (alignment != 1)
    {
        if (alignment == 2)
        {
            sprite = sprite_cursor;
            for (i = 0; i < count; i++)
            {
                x -= sprite[i].w >> 1;
            }
        }
    }
    else
    {
        sprite = sprite_cursor;
        for (i = 0; i < count; i++)
        {
            x -= sprite[i].w;
        }
    }

    acc = 0;

    if (count != 0)
    {
        do
        {
            setSprt(sprite_cursor);
            if (color != 0x100)
            {
                setRGB0(sprite_cursor, color, color, color);
                setSemiTrans(sprite_cursor, 1);
            }
            else
            {
                setRGB0(sprite_cursor, 0x80, 0x80, 0x80);
            }
            sprite_cursor->x0 = x + acc;
            sprite_cursor->y0 = y;
            acc += sprite_cursor->w;

            addPrim(ordering_table, sprite_cursor);
            sprite_cursor++;
            count--;
        } while (count != 0);
    }

    sprite_cursor = func_800AD658(ordering_table, sprite_cursor, n);

    tpage = (DR_TPAGE*)sprite_cursor;
    setDrawTPage(tpage, 0, 0, 0x3F);
    addPrim(ordering_table, tpage);

    return tpage + 1;
}
