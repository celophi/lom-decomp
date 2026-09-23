#include "carda_internal.h"

s32 func_80140370(s32 arg0)
{
    if (D_80165FE0 != 0)
    {
        func_8014986C();
        field_text_reset_windows();
        func_80019788(0);
        return 1;
    }
    field_text_reset_scratch();
    func_8014ADF8();
    func_80140830(arg0);
    func_8014AE34();
    field_text_upload_immediate_cache();
    D_80166004 ^= 1;
    return 0;
}

void func_801403FC(void)
{
    CardaElement *p;

    D_80165FFC = 0;
    D_80165F38 = 0;
    D_80166104 = 0;
    D_80165FF4 = 0;
    D_801660FC = 0;
    D_80166100 = (s32)D_8012271C + 0xCE0;
    if (0)
    {
        func_801425D4(0, 0, 0, 0, 0);
    }
    func_801425D4();
    D_801660F8 = 0;

    D_80165F80[0].attr.f.state = 1;
    if ((u32)(D_80166078 - 2) < 2U)
    {
        p = func_80142614();
        p->draw_handler = (void *)func_80145050;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x10;
        p->attr.f.unk0_16 = 0x4C;
        p->attr4.f.unk4_0 = 1;
        p->attr4.f.y = 0x48;
        SET_ELEM_CODE(p, 0x20);

        p = func_80142614();
        p->draw_handler = (void *)func_80141C3C;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x1C;
        p->attr.f.unk0_16 = 0x3A;
        p->attr4.f.unk4_0 = 0;
        p->attr4.f.y = 0x10;
        SET_ELEM_CODE(p, 0x80);

        p = func_80142614();
        p->draw_handler = (void *)func_80141D18;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0xA4;
        p->attr.f.unk0_16 = 0x3A;
        p->attr4.f.unk4_0 = 0;
        p->attr4.f.y = 0x10;
        SET_ELEM_CODE(p, 0x80);

        p = func_80142614();
        p->draw_handler = (void *)func_80141B50;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x20;
        p->attr.f.unk0_16 = 0x22;
        p->attr4.f.unk4_0 = 1;
        p->attr4.f.y = 0x10;
        SET_ELEM_CODE(p, 0);
    }
    else
    {
        p = func_80142614();
        p->draw_handler = (void *)func_80141250;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x0A;
        p->attr.f.unk0_16 = 0x32;
        p->attr4.f.unk4_0 = 1;
        p->attr4.f.y = 0x58;
        SET_ELEM_CODE(p, 0x2C);

        p = func_80142614();
        p->draw_handler = (void *)func_80141B50;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x68;
        p->attr.f.unk0_16 = 0x0A;
        p->attr4.f.unk4_0 = 0;
        p->attr4.f.y = 0x10;
        SET_ELEM_CODE(p, 0x70);

        p = func_80142614();
        p->draw_handler = (void *)func_80141C3C;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x1C;
        p->attr.f.unk0_16 = 0x1E;
        p->attr4.f.unk4_0 = 0;
        p->attr4.f.y = 0x10;
        SET_ELEM_CODE(p, 0x80);

        p = func_80142614();
        p->draw_handler = (void *)func_80141D18;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0xA4;
        p->attr.f.unk0_16 = 0x1E;
        p->attr4.f.unk4_0 = 0;
        p->attr4.f.y = 0x10;
        SET_ELEM_CODE(p, 0x80);

        p = func_80142614();
        p->draw_handler = (void *)func_80141DF4;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x1E;
        p->attr.f.unk0_16 = 0x8E;
        p->attr4.f.unk4_0 = 1;
        p->attr4.f.y = 0x34;
        SET_ELEM_CODE(p, 4);
    }
    D_80165F80[0].attr.f.state = 0;
}

void func_80140830(void)
{
    s32 delta;

    func_80141230();
    D_80166068 += 2;
    if ((D_80165F8C.attr.word & 0x7F) == 2)
    {
        func_80140918();
    }
    if ((u16)D_80122988 == 0xFFFF)
    {
        D_80122988 = 0;
    }
    func_80140BAC();
    if (D_80165FFC != 0)
    {
        s32 base = D_80166104;
        delta = (D_80165F38 - D_80166104) / D_80165FFC;
        D_80165FFC -= 1;
        D_80166104 += delta;
    }
    else
    {
        D_80166104 = D_80165F38;
    }
}

s32 func_80140918(void)
{
    s32 result;
    s32 repeat;
    CardaElement *p;

    if ((u32)(D_80166078 - 2) < 2) {
        repeat = 3;
        if (D_801663A0 != 0) {
            goto load_loop;
        }
        switch (D_80165FEC) {
        case 0xE9:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
        case 0xF0:
        case 0xF1:
        case 0xF6:
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFF:
            return;
        default:
            D_801663A0 = D_80165B70;
            goto set_repeat;
        }
    } else {
        repeat = 3;
        if (D_80165FEC < 0x12) {
            goto load_loop;
        }
        if (D_801663A0 != 0) {
            goto load_loop;
        }
        if (D_80165FEC == 0xF1) {
            goto load_loop;
        }
        D_801663A0 = D_80165B70;
    }
set_repeat:
    repeat = 3;
load_loop:
    do {
        result = func_80147F4C();
    } while (result == repeat);

    if (D_801660F8 != 0 && (D_80122988 & 0x220)) {
        D_80165FEC = 0xF9;
        D_801660F8 = 0;

        p = D_80165F80;
        D_80165F80[0].attr.word = (((((D_80165F80[0].attr.word & ~7U) | 1U) & ~0x78U) | 8U) & 0xFFFF007FU) | 0x800U;
        ((u8 *)p)[2] = 0x4C;
        D_80165F80[0].attr.word = (D_80165F80[0].attr.word & 0x00FFFFFFU) | 0x20000000U;
        p->attr4.word = ((p->attr4.word | 1U) & ~0x1FEU) | 0x90U;
        func_80144F18();
        D_80165F80[0].draw_handler = (void *)func_80144050;
        func_801495E4();
        return;
    }

    switch (result) {
    case 0:
        break;
    case 2:
        D_801663A0 = D_80165B84;
        break;
    case 4:
        if ((u32)(D_80166078 - 2) < 2) {
            D_801663A0 = 0;
        } else {
            D_801663A0 = D_80165B7C;
        }
        D_801660F8 = 0;
        break;
    case 5:
        if (D_80166078 == 1 || D_80166078 == 3) {
            D_80165FEC = 0xF9;
            if (D_80166078 == 1) {
                D_801663A0 = D_80165B70;
            }
        } else {
            D_80165FEC = 0xF9;
            D_801660F8 = 0;
            p = D_80165F80;
            D_80165F80[0].attr.word = (((((D_80165F80[0].attr.word & ~7U) | 1U) & ~0x78U) | 8U) & 0xFFFF007FU) | 0x800U;
            ((u8 *)p)[2] = 0x4C;
            D_80165F80[0].attr.word = (D_80165F80[0].attr.word & 0x00FFFFFFU) | 0x20000000U;
            p->attr4.word = ((p->attr4.word | 1U) & ~0x1FEU) | 0x90U;
            func_80144F18();
            D_80165F80[0].draw_handler = (void *)func_80144050;
            func_801495E4();
        }
        break;
    }
}

s32 func_80140BAC(void)
{
    s32 pending;
    s32 status;
    s32 count;
    CardaElement *p;

    if (((s32)D_80165F80[1].attr.word & 7) == 0) {
        D_80165FE0 = 1;
        return;
    }
    if (D_80165FE0 != 0) {
        return;
    }
    if (((s32)D_80165F80[1].attr.word & 7) >= 3) {
        return;
    }
    if ((D_80165F80[0].attr.word & 7) != 0) {
        return;
    }
    if ((u32)(D_80166078 - 2) < 2U) {
        return;
    }
    pending = D_80165FEC;
    if (pending == 0xFF) {
        return;
    }
    if (D_80166AE0 != 0) {
        return;
    }
    if (D_80166000 != 0) {
        return;
    }
    if ((u32)(*D_801663A0 - 6) < 2U) {
        return;
    }

    status = D_80122988;
    if (status & 0x40) {
        g_field_card_overlay_mode = 3;
        func_800A3938(0x78, 0x80);
        func_80141164();
        return;
    }
    if (status & 0xA100) {
        func_800A3938(0x7D, 0x80);
        func_801410E4();
        return;
    }
    if (pending >= 0x12) {
        return;
    }

    count = 1;
    if (status & 8) {
        D_80122988 = 0x4000;
        count = 1;
    }
    if (D_80122988 & 4) {
        D_80122988 = 0x1000;
        count = 1;
    }

    while (count != 0) {
        if (D_80122988 & 0x1000) {
            D_80165FF4 -= 1;
            if (D_80165FF4 < 0) {
                D_80165FF4 = D_80165FEC - 1;
            }
        }
        if (D_80122988 & 0x4000) {
            D_80165FF4 += 1;
            if (D_80165FF4 >= D_80165FEC) {
                D_80165FF4 = 0;
            }
        }
        count -= 1;
    }

    if (D_80122988 & 0x5000) {
        func_80149DF4();
        func_800A3938(0x7D, 0x80);
        func_801411CC();
        return;
    }

    if (D_80122988 & 0x220) {
        if (D_80166078 == 1) {
            s32 term1;
            s32 term2;
            term1 = D_801660A0 * 0x320;
            term2 = (D_80165FF4 * 0x28) + (s32)D_80166440;
            if (func_8001714C(D_800ECF7C, (void *)(term1 + term2), 0xC) == 0) {
                if (D_8003EC9C == 0xFF || D_8016636F == D_8003EC9C) {
                    p = func_80142614();
                    p->attr.f.unk0_3 = 1;
                    p->attr.f.x = 0x10;
                    p->attr.f.unk0_16 = 0x5A;
                    p->attr4.f.unk4_0 = 1;
                    p->attr4.f.y = 0x2C;
                    SET_ELEM_CODE(p, 0x20);
                    func_80142E10();
                    func_80144F18();
                    p->draw_handler = (void *)func_8014344C;
                    func_801495E4();
                    func_800A3938(0x7E, 0x80);
                    return;
                }
            }
        } else {
            {
                s32 term1;
                s32 term2;
                term1 = D_801660A0 * 0x320;
                term2 = (D_80165FF4 * 0x28) + (s32)D_80166440;
                if (func_8001714C(D_800ECFC4, (void *)(term1 + term2), 8) == 0) {
                    p = func_80142614();
                    p->attr.f.unk0_3 = 1;
                    p->attr.f.x = 0x10;
                    p->attr.f.unk0_16 = 0x5A;
                    p->attr4.f.unk4_0 = 1;
                    p->attr4.f.y = 0x2C;
                    SET_ELEM_CODE(p, 0x20);
                    func_80142E10();
                    func_80144F18();
                    p->draw_handler = (void *)func_801439B4;
                    func_801495E4();
                    func_800A3938(0x7E, 0x80);
                    return;
                }
            }
            {
                s32 term1;
                s32 term2;
                term1 = D_801660A0 * 0x320;
                term2 = (D_80165FF4 * 0x28) + (s32)D_80166440;
                if (func_8001714C(D_800ECF7C, (void *)(term1 + term2), 0xC) == 0) {
                    p = func_80142614();
                    p->attr.f.unk0_3 = 1;
                    p->attr.f.x = 0x10;
                    p->attr.f.unk0_16 = 0x5A;
                    p->attr4.f.unk4_0 = 1;
                    p->attr4.f.y = 0x2C;
                    SET_ELEM_CODE(p, 0x20);
                    func_80142E10();
                    func_80144F18();
                    p->draw_handler = (void *)func_80143BD4;
                    func_801495E4();
                    func_800A3938(0x7E, 0x80);
                    return;
                }
            }
        }
        func_800A3938(0x78, 0x80);
    }
}

void func_801410E4(void)
{
    D_801660F8 = 0;
    D_801663A0 = 0;
    D_80165FEC = 0xFF;
    D_80165FFC = 0;
    D_80165F38 = 0;
    D_80166104 = 0;
    D_80165FF4 = 0;
    D_801660FC = 0;
    D_801660A0 ^= 1;
    func_80147C5C();
    func_8014A044();
    func_80149FEC();
}

void func_80141164(void)
{
    s32 temp_v1;
    s32 var_a1;
    s32 *var_a0;
    s32 temp;

    func_80067F28();
    var_a0 = (s32 *)D_80165F80;
    var_a1 = 0;
    do
    {
        temp_v1 = *var_a0;
        if (temp_v1 & 7)
        {
            temp = (temp_v1 & ~7) | 3;
            *var_a0 = (temp & ~0x78) | 0x40;
        }
        var_a1 += 1;
        var_a0 += 3;
    } while (var_a1 < 8);
}

void func_801411CC(void)
{
    s32 index;
    s32 temp;
    s32 base;
    s32 pos;
    s32 diff;

    index = D_80165FF4;
    temp = (index << 3) - index;
    base = D_80166104;
    pos = temp << 1;
    diff = pos - base;

    if (diff >= 0x4B)
    {
        D_80165F38 = pos - 0x46;
        D_80165FFC = 4;
    }
    if (diff < 0)
    {
        D_80165F38 = pos;
        D_80165FFC = 4;
    }
}

void func_80141230(void)
{
    func_80142668();
}

/**
 * @brief Build the primitive list for the memory-card entry browser body.
 *
 * Dispatches on the current status code @c D_80165FEC to draw a status/prompt
 * glyph, or, in the default case, renders one row per card entry (rank digits,
 * icons, protect/copy state) plus the highlight bar for the selected row.
 *
 * @param ot Ordering table the primitives are linked into.
 * @param prim GPU packet cursor to emit primitives into.
 * @param x_offset Horizontal scroll offset subtracted from each glyph x.
 * @param y_offset Vertical scroll offset subtracted from each row y.
 * @return The advanced packet cursor past the last emitted primitive.
 * @see decomp.me (100%)
 */
s32 func_80141250(s32* ot, s32 prim, s32 x_offset, s32 y_offset)
{
    if ((D_80165F80[0].attr.word & 7) != 0)
    {
        if (D_80165FEC >= 0xF8)
        {
            if (D_80165FEC < 0xFE)
            {
                return prim;
            }
            if (D_80165FEC == 0xFF)
            {
                return prim;
            }
        }
    }

    switch (D_80165FEC)
    {
    case 0xF8:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B06C, 0x34), 4, -x_offset + 0x96, -y_offset, 2);
        break;
    case 0xF9:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0EC, 0xB4), 4, -x_offset + 0x96, -y_offset, 2);
        break;
    case 0xF6:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B07A, 0x42), 4, -x_offset + 0x96, -y_offset, 2);
        break;
    case 0xFF:
    {
        s32 x = -x_offset + 0x96;
        u8* base = (u8*)&D_8014B038;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0), 4, x, -y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);
        break;
    }
    case 0xFA:
    {
        s32 x = -x_offset + 0x96;
        u8* base = (u8*)&D_8014B03A;
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B03A, 2), 4, x, -y_offset, 2);
        base -= 2;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x5A), 4, x, 0x10 - y_offset, 2);
        break;
    }
    case 0xF7:
    {
        s32 x = -x_offset + 0x96;
        u8* base = (u8*)&D_8014B03A;
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B03A, 2), 4, x, -y_offset, 2);
        base -= 2;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x60), 4, x, 0x10 - y_offset, 2);
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x62), 4, x, 0x20 - y_offset, 2);
        break;
    }
    case 0xFD:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B03C, 4), 4, -x_offset + 0x96, -y_offset, 2);
        break;
    case 0xFB:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B048, 0x10), 4, -x_offset + 0x96, -y_offset, 2);
        break;
    case 0xFC:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B04A, 0x12), 4, -x_offset + 0x96, -y_offset, 2);
        break;
    case 0xFE:
        break;
    default:
    {
        s32 row_y;
        s32 i;

        if (D_80166AE0 != 0)
        {
            s32 x = -x_offset + 0x96;
            u8* base = (u8*)&D_8014B038;
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0), 4, x, -y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x1E), 4, x, 0xE - y_offset, 2);
            prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0xB2), 4, x, 0x1C - y_offset, 2);
            break;
        }

        do
        {
            i = 0;
        } while (0);
        if (D_80165FEC > 0)
        {
            s32 base_x;
            s32* flag_ptr;
            u16 misc_glyph;
            Vec2s pos;
            s32 row;
            u8* base;
            s32 misc_x;
            s32 entry_offset;
            s32 rank_offset;
            s32 rank_x;
            s32 draw_style;
            s32 label_x;

            base = (u8*)&D_8014B038;
            base_x = -x_offset;
            entry_offset = 0;
            rank_offset = 0;
            do
            {
                draw_style = 4;
                misc_x = base_x + 0xD6;
                label_x = 1 - x_offset;
                row = ((i * 14) - y_offset) - D_80166104;
                row_y = row + 1;
                if ((u32)(row + 0xE) < 0x65U)
                {
                    flag_ptr = (s32*)((u8*)D_801663A8 + rank_offset);
                    if (*flag_ptr >= 0)
                    {
                        pos.x = base_x + 0x86;
                        pos.y = row_y;
                        prim = func_800A8A78(ot, prim, *(s32*)((u8*)D_80166A80 + rank_offset), draw_style, &pos, 0);
                        prim = func_800A88A0(prim, ot, (void*)((s32)D_8014B066 + (s32)base), draw_style, base_x + 0x70, row_y, 0);
                        pos.y = row_y;
                        pos.x = misc_x;
                        if ((D_80166438 - 1) == *flag_ptr)
                        {
                            misc_glyph = *(u16*)(base + 0x36);
                            rank_x = misc_x << 16;
                            prim = func_800A88A0(prim, ot, (void*)((s32)misc_glyph + (s32)base), draw_style, rank_x >> 16, row_y, 0);
                        }
                        else if (*flag_ptr < 2)
                        {
                            misc_glyph = *(u16*)(base + 0x38);
                            rank_x = misc_x << 16;
                            prim = func_800A88A0(prim, ot, (void*)((s32)misc_glyph + (s32)base), draw_style, rank_x >> 16, row_y, 0);
                        }
                        if (*func_80143334((void*)(&((u8(*)[0x320])D_80166440)[D_801660A0][entry_offset] + 12)) == 0x2B)
                        {
                            prim = func_800A88A0(prim, ot, (void*)((s32)D_8014B0E8 + (s32)base), draw_style, 0x10C - x_offset, row_y, 1);
                        }
                    }
                    if (func_8001714C(D_800ECF7C, (void*)&((u8(*)[0x320])D_80166440)[D_801660A0][entry_offset], 0xC) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void*)((s32)D_8014B03E + (s32)base), draw_style, label_x, row_y, 0);
                    }
                    else if (func_8001714C(D_800ECF8C, (void*)&((u8(*)[0x320])D_80166440)[D_801660A0][entry_offset], 0xC) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void*)((s32)D_8014B072 + (s32)base), draw_style, label_x, row_y, 0);
                    }
                    else if (func_8001714C(D_800ECFC4, (void*)&((u8(*)[0x320])D_80166440)[D_801660A0][entry_offset], 8) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void*)((s32)D_8014B04C + (s32)base), draw_style, label_x, row_y, 0);
                    }
                    else if (func_8001714C(D_800ECFD0, (void*)&((u8(*)[0x320])D_80166440)[D_801660A0][entry_offset], 9) == 0)
                    {
                        prim = func_800A88A0(prim, ot, (void*)((s32)D_8014B09E + (s32)base), draw_style, label_x, row_y, 0);
                    }
                    else
                    {
                        prim = func_800A88A0(prim, ot, (void*)((s32)D_8014B040 + (s32)base), draw_style, label_x, row_y, 0);
                    }
                }
                entry_offset += 0x28;
                rank_offset += 4;
                i++;
            } while (i < D_80165FEC);
        }

        row_y = ((D_80165FF4 * 14) - y_offset) - D_80166104;
        if (D_80166AE0 == 0)
        {
            TILE* tile = (TILE*)prim;
            *(u32*)&tile->r0 = 0xF080F0;
            setlen(tile, 3);
            setcode(tile, 0x62);
            tile->w = 0x12C;
            setXY0(tile, 0, row_y);
            tile->h = 0xE;
            addPrim(ot, tile);
            prim += sizeof(TILE);
        }
        break;
    }
    }
    return prim;
}

/**
 * @brief Draw the mode-dependent memory-card status glyph.
 *
 * The small frame scratch preserves the original GCC 2.7.2 stack-frame
 * bucket used by this seven-argument draw call.
 * @see matching: 100.00%
 */
s32 func_80141B50(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 frame_scratch[2];

    if ((u32)(D_80166078 - 2) < 2)
    {
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B0D4, 0x9C),
                             4, -x_offset + 0x80, -y_offset, 2);
    }
    else if (D_80166078 == 1)
    {
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B064, 0x2C),
                             4, -x_offset + 0x38, -y_offset, 2);
    }
    else
    {
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B042, 0xA),
                             4, -x_offset + 0x38, -y_offset, 2);
    }

    return prim;
}

typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} CardaRect;
typedef struct
{
    u32 tag;
    u8 r0;
    u8 g0;
    u8 b0;
    u8 code;
    s16 x0;
    s16 y0;
    s16 w;
    s16 h;
} CardaTile;

s32 func_80141C3C(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    CardaRect pos;
    CardaTile *tile;

    if (D_801660A0 != 0)
    {
        tile = (CardaTile *)prim;
        *(u32 *)&tile->r0 = 0x101010;
        setlen(tile, 3);
        setcode(tile, 0x62);
        setXY0(tile, 0, 0);
        setWH(tile, 0x80, 0x10);
        addPrim(ot, tile);
        prim += 0x10;
    }
    return func_800A88A0(prim, ot, GLYPH_SYM(D_8014B044, 0xC), 4, -arg2 + 0x40, -arg3, 2);
}

s32 func_80141D18(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    CardaRect pos;
    CardaTile *tile;

    if (D_801660A0 == 0)
    {
        tile = (CardaTile *)prim;
        *(u32 *)&tile->r0 = 0x101010;
        setlen(tile, 3);
        setcode(tile, 0x62);
        setXY0(tile, 0, 0);
        setWH(tile, 0x80, 0x10);
        addPrim(ot, tile);
        prim += 0x10;
    }
    return func_800A88A0(prim, ot, GLYPH_SYM(D_8014B046, 0xE), 4, -arg2 + 0x40, -arg3, 2);
}

typedef struct CardaFallbackTextMatch { u8 pad[0x24]; u8 text[0x20]; } CardaFallbackTextMatch;

/**
 * @brief Draw the selected memory-card entry's details.
 *
 * This is the CARDA counterpart of CLOAD's selected-entry renderer.  The
 * unused vector is retained because it is part of the original function's
 * stack layout under GCC 2.7.2 CDK.
 * @see matching: 100.00%
 */
s32 func_80141DF4(s32 *ot, s32 prim, s32 x_offset, s32 y_offset)
{
    s32 result;
    DVECTOR pos;
    u8 name[0x21];
    char unused_pad[212];
    s32 slot[3];
    DVECTOR unused_pos;

    result = prim;
    if (D_801660FC == 0)
    {
        return result;
    }
    if (D_80166AE0 != 0)
    {
        return result;
    }
    if (D_801660FC != 3 && D_80165FEC != 0xFA && D_80165FEC < 0x10)
    {
        if (D_801660FC == 2)
        {
            s32 x = -x_offset;
            u8 *base;

            result = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B060, 0x28), 4, x, -y_offset, 0);
            base = (u8 *)&D_8014B060 - 0x28;
            return func_800A88A0(result, ot, GLYPH_OFF(base, 0x2A), 4, x, 0x10 - y_offset, 0);
        }
        else if (D_801660FC == 4)
        {
            return func_800A88A0(prim, ot, GLYPH_SYM(D_8014B092, 0x5A), 4, -x_offset, -y_offset, 0);
        }
        else
        {
            s32 term1 = D_801660A0 * CARDA_CARD_DIRECTORY_BYTES;
            s32 term2 = (D_80165FF4 * CARDA_DIRECTORY_ENTRY_BYTES) + (s32)D_80166440;

            if (func_8001714C(D_800ECF7C, (void *)(term1 + term2), 0xC) == 0)
            {
                if (D_8003EC9C == 0xFF || D_8016636F == D_8003EC9C || D_8016636F == 0xFF)
                {
                    u8 *base90 = D_801662A0;
                    s32 present_count;
                    s32 i;
                    s32 j;
                    s32 step;
                    s32 half_step;
                    s32 base_x;
                    s32 base_y;
                    s32 total;
                    s32 hours;
                    s32 time_val;

                    total = 0;
                    slot[0] = (u32)(*(s32 *)(base90 + 0x18)) >> 0x19;
                    slot[1] = ((u32)(*(s32 *)(base90 + 0x20)) >> 0x12) & 0x7F;
                    slot[2] = (u32)(*(s32 *)(base90 + 0x20)) >> 0x19;
                    D_8016606C = (s32)base90[0x1F];

                    present_count = 0;
                    for (i = 0; i < 3; i++)
                    {
                        if (slot[i] != 0x7F)
                        {
                            present_count += 1;
                        }
                    }

                    switch (present_count)
                    {
                    case 2:
                        step = 0x20;
                        half_step = 0x10;
                        time_val = D_80166068;
                        if (D_80166068 < 0)
                        {
                            time_val = D_80166068 + 0x1F;
                        }
                        D_80166068 -= (time_val >> 5) << 5;
                        break;
                    case 3:
                        step = 0x10;
                        half_step = 0x20;
                        D_80166068 %= 0x60;
                        break;
                    default:
                        step = 0x10;
                        half_step = 0x20;
                        D_80166068 = 0x1F;
                        break;
                    }

                    i = 0;
                    j = i;
                    for (; j < 3; j++)
                    {
                        base_y = i * half_step;
                        base_x = base_y + half_step;
                        if (slot[j] != 0x7F)
                        {
                            s32 adjust = step;
                            s32 rem;
                            s32 hi;
                            s32 delta;

                            if ((D_80166068 >= base_y && D_80166068 < base_x && (delta = D_80166068 - base_y, 1))
                                || (rem = base_x % (half_step * present_count), D_80166068 >= rem && D_80166068 < (hi = rem + half_step) && (delta = hi - D_80166068, 1)))
                            {
                                adjust += delta;
                            }
                            result = func_80144CD0(result, ot, total - x_offset, -y_offset, adjust, slot[j], i, j);
                            total += adjust;
                            i += 1;
                        }
                    }

                    {
                        u8 *base90_2 = D_801662A0;
                        s32 x = -x_offset;
                        s32 y = -y_offset;

                        base_y = *(s32 *)(base90_2 + 0x30);

                        pos.vx = (s16)(x + 0x70);
                        pos.vy = (s16)y;
                        hours = base_y / 216000;
                        result = func_800A8A78(ot, result, hours, 4, &pos, 1);
                        result = func_800A88A0(result, ot, D_800EC3F6[0] + ((s32)&D_800EC3F6 - 0x32) + (D_800EC3F6[1] << 8), 4, x + 0x6F, y, 0);
                        base_y = (base_y / 3600) - (hours * 0x3C);
                        if (base_y < 0xA)
                        {
                            pos.vx = (s16)(x + 0x7D);
                            pos.vy = (s16)y;
                            result = func_800A8A78(ot, result, 0, 4, &pos, 1);
                        }
                        pos.vx = (s16)(x + 0x85);
                        pos.vy = (s16)y;
                        result = func_800A88A0(func_800A88A0(func_800A8A78(ot, result, base_y, 4, &pos, 1), ot, base90_2, 4, x + 0x54, y + 0x10, 0), ot, GLYPH_OFF((u8 *)D_8014CA6C, (*(s32 *)(base90_2 + 0x20) & 0x3FFFF) * 2), 4, x + 0x54, y + 0x20, 0);
                    }
                }
                else
                {
                    result = func_800A88A0(result, ot, GLYPH_SYM(D_8014B08C, 0x54), 4, -x_offset, -y_offset, 0);
                }
            }
            else
            {
                s32 j;

                {
                    u8 *text_base;
                    func_80142508(&D_80166124);
                    text_base = (u8 *)&D_80166124;
                    text_base -= 4;
                    if ((u32)(text_base[0x24] - 1) >= 0x7FU)
                    {
                        do {
do {
                        for (j = 0; j < 0x20; j++)
                        {
                            name[j] = text_base[j + 4];
                        }
                        name[j] = 0;
                        result = func_8014A900(result, ot, name, -x_offset, -y_offset, 4, 0);

                        for (j = 0; j < 0x20; j++)
                        {
                            name[j] = ((CardaFallbackTextMatch *)&D_80166120)->text[j];
                        }
                        name[j] = 0;
                        result = func_8014A900(result, ot, name, -x_offset, -y_offset + 0x10, 4, 0);
                        } while (0);
} while (0);
                    }
                }
            }
        }
    }
    return result;
}

void func_80142508(void *arg0)
{
    u8 *p;
    s32 i;

    p = (u8 *)arg0;
    i = 0;
    for (;;)
    {
        if (i >= 0x40)
        {
            return;
        }
        if (*p == 0)
        {
            while (i < 0x40)
            {
                *p = 0;
                i++;
                p++;
            }
            return;
        }
        if (*p >= 0x80)
        {
            p += 2;
            i += 2;
        }
        else
        {
            p += 1;
            i += 1;
        }
    }
}

s32 func_8014256C(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;

    return func_800A88A0(prim, ot,
        (void *)((u8 *)D_800EC3D0 - 0xC + D_800EC3D0[0] + (D_800EC3D0[1] << 8)),
        5, 0x80 - arg2, -arg3, 2);
}

void func_801425D4(void)
{
    CardaElement *p;
    s32 i;

    g_menu_element_counter = 0x20;
    p = D_80165F80;
    for (i = 0; i < 8; i++)
    {
        p->attr.word &= ~7;
        p++;
    }
}

CardaElement *func_80142614(void)
{
    CardaElement *p;
    s32 i;

    p = D_80165F80;
    for (i = 0; i < 8; i++, p++)
    {
        if ((p->attr.word & 7) == 0)
        {
            p->attr.word = (p->attr.word & ~7) | 1;
            return p;
        }
    }
    return D_80165F80;
}

typedef struct { s32 unk0; s32 unk4; s16 unk8; s16 unkA; s16 unkC; u16 unkE; } CardaGpuPacket;
typedef struct { s32 unk0; u8 pad4[0x40AE]; s16 unk40B2; u8 pad40B4[4]; CardaGpuPacket *unk40B8; } CardaDrawState;
typedef CardaGpuPacket *(*CardaElemDrawFunc)();
typedef struct { union { u32 word; } attr; u32 size_flags; CardaElemDrawFunc draw_handler; } CardaElementSlot;

void func_80142668(CardaDrawState *arg0)
{
    CardaGpuPacket *var_s0;
    CardaDrawState *var_s5;
    volatile u32 *var_s3;
    s32 temp_s1;
    s32 temp_s2;
    s32 var_s6;
    s32 sp20[24];
    u32 temp_a0_2;
    s32 temp_v1_2;
    u32 temp_a1;
    u32 temp_a2;
    s32 temp_a0_3;
    s32 var_v1;
    s32 temp_a3_2;
    s32 var_v0;
    s32 temp_a3_3;
    u32 temp_v0_3;
    u32 temp_a0_4;
    s32 temp_a0_5;
    s32 var_v1_2;
    s32 temp_a3_5;
    s32 var_v0_2;
    s32 temp_a3_6;
    u32 temp_v0_5;
    u32 temp_v1_3;
    s32 count;

    var_s0 = arg0->unk40B8;
    var_s5 = arg0;

    {
        CardaElementSlot *pool;
        CardaElemDrawFunc handler;

        pool = (CardaElementSlot *)D_80165F80;
        handler = pool[1].draw_handler;
        if (handler == (CardaElemDrawFunc)func_80141250)
        {
            count = D_80165FEC;
            if ((count < 0x10) && ((pool[1].attr.word & 7) == 2))
            {
                count *= 0xE;
                if ((D_80166104 + 0x58) < count)
                {
                    var_s0 = (CardaGpuPacket *)func_800AE76C(var_s0, var_s5, 0x12E, 0x82, 0);
                }
                if (D_80166104 != 0)
                {
                    var_s0 = (CardaGpuPacket *)func_800AE76C(var_s0, var_s5, 0x12E, 0x3A, 1);
                }
            }
        }
        else if ((handler == (CardaElemDrawFunc)func_80146EDC) && ((pool[1].attr.word & 7) == 2))
        {
            if (((D_80165FE8 * 0xE) - D_80166104) >= 0x8D)
            {
                var_s0 = (CardaGpuPacket *)func_800AE76C(var_s0, var_s5, 0x118, 0xBE, 0);
            }
            if (D_80166104 != 0)
            {
                var_s0 = (CardaGpuPacket *)func_800AE76C(var_s0, var_s5, 0x118, 0x3E, 1);
            }
        }
    }

    if (arg0->unk40B2 != 0)
    {
        func_8001C56C(sp20, 0, 0xF0, 0x140, 0xE0);
    }
    else
    {
        func_8001C56C(sp20, 0, 8, 0x140, 0xE0);
    }

    var_s3 = (volatile u32 *)D_80165F80;
    var_s6 = 0;

    for (; var_s6 < 8; var_s6++, var_s3 += 3)
    {
        if (*var_s3 & 7)
        {
            func_8001A5D4((s32)var_s0, sp20);

            addPrim(var_s5, var_s0);

            temp_a0_2 = *var_s3;
            temp_v1_2 = temp_a0_2 & 7;

            var_s0 = (CardaGpuPacket *)((u8 *)var_s0 + 0x40);

            switch (temp_v1_2)
            {
            case 1:
                temp_v0_3 = *var_s3;
                temp_a1 = *(u32 *)((u8 *)var_s3 + 4);
                temp_a0_4 = temp_v0_3 >> 24;
                temp_a2 = ((temp_a1 & 1) << 8) | temp_a0_4;
                temp_a0_3 = (temp_v0_3 >> 3) & 0xF;
                var_v1 = temp_a2 * temp_a0_3;
                D_80122988 = 0;
                if (var_v1 < 0)
                {
                    var_v1 += 7;
                }
                temp_a3_2 = (temp_a1 >> 1) & 0xFF;
                var_v0 = temp_a3_2 * temp_a0_3;
                temp_s1 = var_v1 >> 3;
                if (var_v0 < 0)
                {
                    var_v0 += 7;
                }
                temp_s2 = var_v0 >> 3;
                temp_a3_3 = (s32)(temp_a3_2 - temp_s2);

                var_s0 = (*(CardaElemDrawFunc *)((u8 *)var_s3 + 8))(var_s5, var_s0, (s32)(temp_a2 - temp_s1) / 2, temp_a3_3 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *var_s3;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    var_s0 = (CardaGpuPacket *)func_800AD850(var_s0, var_s5,
                                           field + (s32)((((*(u32 *)((u8 *)var_s3 + 4) & 1) << 8) | high) - temp_s1) / 2,
                                           (*((u8 *)var_s3 + 2)) + ((s32)((*(u32 *)((u8 *)var_s3 + 4) >> 1) & 0xFF) - temp_s2) / 2,
                                           temp_s1, temp_s2, arg0->unk40B2, var_s6 == 0);
                }
                {
                    u32 old_word;
                    u32 new_word;
                    old_word = *var_s3;
                    new_word = (old_word & ~0x78) | (((((old_word >> 3) & 0xF) + 1) & 0xF) * 8);
                    *(u32 *)var_s3 = new_word;
                    if (((new_word >> 3) & 0xF) == 8)
                    {
                        field_reset_input_repeat();
                        *(u32 *)var_s3 = (*var_s3 & ~7) | 2;
                    }
                }
                break;

            case 2:
                var_s0 = (*(CardaElemDrawFunc *)((u8 *)var_s3 + 8))(var_s5, var_s0, 0, 0);
                {
                    u32 case_word;
                    u32 high;
                    case_word = *var_s3;
                    high = case_word >> 24;
                    var_s0 = (CardaGpuPacket *)func_800AD850(var_s0, var_s5,
                                           (case_word >> 7) & 0x1FF, *((u8 *)var_s3 + 2),
                                           ((*(u32 *)((u8 *)var_s3 + 4) & 1) << 8) | high,
                                           (*(u32 *)((u8 *)var_s3 + 4) >> 1) & 0xFF, arg0->unk40B2, var_s6 == 0);
                }
                temp_v1_3 = *var_s3;
                if (((temp_v1_3 >> 3) & 0xF) != 0)
                {
                    *(u32 *)var_s3 = (temp_v1_3 & ~0x78) | (((((temp_v1_3 >> 3) & 0xF) - 1) & 0xF) * 8);
                }
                break;

            case 3:
                temp_a0_5 = *var_s3;
                temp_a1 = *(u32 *)((u8 *)var_s3 + 4);
                var_v1_2 = (u32)temp_a0_5 >> 24;
                temp_a2 = ((temp_a1 & 1) << 8) | var_v1_2;
                temp_a0_5 = (u32)temp_a0_5 >> 3;
                temp_a0_5 &= 0xF;
                var_v1_2 = temp_a2 * temp_a0_5;
                D_80122988 = 0;
                if (var_v1_2 < 0)
                {
                    var_v1_2 += 7;
                }
                temp_a3_5 = (temp_a1 >> 1) & 0xFF;
                var_v0_2 = temp_a3_5 * temp_a0_5;
                temp_s1 = var_v1_2 >> 3;
                if (var_v0_2 < 0)
                {
                    var_v0_2 += 7;
                }
                temp_s2 = var_v0_2 >> 3;
                temp_a3_6 = (s32)(temp_a3_5 - temp_s2);

                var_s0 = (*(CardaElemDrawFunc *)((u8 *)var_s3 + 8))(var_s5, var_s0, (s32)(temp_a2 - temp_s1) / 2, temp_a3_6 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *var_s3;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    var_s0 = (CardaGpuPacket *)func_800AD850(var_s0, var_s5,
                                           field + (s32)((((*(u32 *)((u8 *)var_s3 + 4) & 1) << 8) | high) - temp_s1) / 2,
                                           (*((u8 *)var_s3 + 2)) + ((s32)((*(u32 *)((u8 *)var_s3 + 4) >> 1) & 0xFF) - temp_s2) / 2,
                                           temp_s1, temp_s2, arg0->unk40B2, var_s6 == 0);
                }
                {
                    u32 old_word;
                    old_word = *var_s3;
                    var_v1_2 = old_word & ~0x78;
                    old_word >>= 3;
                    old_word &= 0xF;
                    old_word--;
                    old_word &= 0xF;
                    old_word <<= 3;
                    var_v1_2 |= old_word;
                    *(u32 *)var_s3 = var_v1_2;
                    if (!(((u32)var_v1_2 >> 3) & 0xF))
                    {
                        *(u32 *)var_s3 = ((((u32)var_v1_2 & ~0x78) | 0x18) & ~7) | 4;
                    }
                }
                break;

            case 4:
                temp_v0_5 = *(u32 *)var_s3;
                D_80122988 = 0;
                temp_v1_3 = (temp_v0_5 & ~0x78) | (((((temp_v0_5 >> 3) & 0xF) - 1) & 0xF) * 8);
                *(u32 *)var_s3 = temp_v1_3;
                if (!((temp_v1_3 >> 3) & 0xF))
                {
                    *(u32 *)var_s3 = temp_v1_3 & ~7;
                }
                break;
            }
        }
    }

    arg0->unk40B8 = var_s0;
}

void func_80142CA4(void)
{
    D_80165F80[0].attr.word &= ~7;
}

void func_80142CBC(u8 *arg0, u8 *arg1)
{
    s32 temp_s0;
    s32 temp_v0;
    s32 i;

    temp_s0 = func_80142D40(arg0);
    temp_v0 = func_80142D40(arg1);
    for (i = 0; i < temp_v0; i++)
    {
        arg0[temp_s0 + i] = arg1[i];
    }
    arg0[temp_s0 + i] = 0;
}

s32 func_80142D40(u8 *arg0)
{
    u8 *p;
    u8 c;
    s32 len;

    p = arg0;
    c = *p;
    len = 0;
    while (c != 0)
    {
        if ((u32)(c - 0x19) < 7)
        {
            p += 2;
            len += 2;
        }
        else
        {
            p += 1;
            len += 1;
        }
        c = *p;
    }
    return len;
}

void func_80142D8C(u8 *arg0, u8 *arg1)
{
    u8 *p;
    u8 c;
    s32 len;
    s32 i;

    p = arg1;
    len = 0;
    while (*p != 0)
    {
        c = *(volatile u8 *)p;
        if ((u32)(c - 0x19) < 7)
        {
            p += 2;
            len += 2;
        }
        else
        {
            p += 1;
            len += 1;
        }
    }
    for (i = 0; i < len; i++)
    {
        arg0[i] = arg1[i];
    }
    arg0[i] = 0;
}
