#include "carda_internal.h"

/**
 * @brief Build and finalize the active CARDA save record.
 */
void func_80142E10(void)
{
  u8 *base;
  u8 *clear_p;
  s32 i;
  s32 idx;
  u8 *src;
  u8 *dst;
  s32 outer_n;
  u8 *outer_p;
  u8 *inner_base;
  s32 elapsed;
  s8 *p;
  s32 mins;
  int new_var;
  s32 flags;
  s32 bits;
  s32 sel;
  u8 *fill_p;
  s32 scan_i;
  u8 *scan_p;
  s32 rng;
  u8 *tail_src;
  u8 *gsp;
  unsigned long long new_var2;
  if (((u32) (D_80166078 - 2)) < 2U)
  {
    func_80146694();
    return;
  }
  base = (u8 *) D_80165FF0;
  clear_p = base + 0x1FB;
  i = 0x1FB;
  base[0] = 0x53;
  base[1] = 0x43;
  base[2] = 0x12;
  base[3] = 0x2;
  do
  {
    clear_p[4] = 0;
    i--;
    clear_p--;
  }
  while (i >= 0);
  idx = (((u8 *) D_8012271C)[0x2E4] * 9) / 26;
  i = 0;
  if (idx >= 0xA)
  {
    idx = 9;
  }
  src = (u8 *) (D_8014BF00[idx] + (((s32) (&D_8014BF00)) - 4));
  do
  {
    dst = base + i;
    dst[0x60] = *src;
    i++;
    src++;
  }
  while (i < 0x20);
  outer_n = 0;
  outer_p = base;
  do
  {
    i = 0;
    inner_base = outer_p + 0x80;
    do
    {
      do { do { do { do { do { do { do { dst = inner_base + i; } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0);
      i++;
      *dst = *src;
      src++;
    }
    while (i < 0x80);
    outer_n++;
    outer_p += 0x80;
  }
  while (outer_n < 2);
  func_800170BC(base + 4, &D_8014BECC);
  if ((((u32) (*((s32 *) (((u8 *) D_8012271C) + 0x28)))) >> 2) & 1)
  {
    base[0xC] = 0x81;
    base[0xD] = 0xF4;
  }
  elapsed = ((*((s32 *) (((u8 *) D_8012271C) + 0x30))) + func_8002054C(-1)) - D_80042FB4;
  *((s32 *) (((u8 *) D_8012271C) + 0x30)) = elapsed;
  D_80042FB4 = func_8002054C(-1);
  p = func_801471E4((s8 *) (base + 0x16), D_80166A80[D_80165FF4]);
  *((Char3 *) p) = D_8014008C;
  mins = elapsed / 216000;
  p = func_801471E4(p + 2, mins);
  *((Char3 *) p) = D_80140090;
  elapsed = (elapsed / 3600) - (mins * 0x3C);
  p += 2;
  if (elapsed < 0xA)
  {
    p = func_801471E4(p, 0);
  }
  func_8014AEC4((u8 *) func_801471E4(p, elapsed), ((u8 *) D_8012271C) + 0x5F0);
  gsp = (u8 *) D_8012271C;
  *((s32 *) (gsp + 0x18)) = ((*((s32 *) (gsp + 0x18))) & 0x01FFFFFF) | (gsp[0x608] << 0x19);
  if (gsp[0x840] != 0)
  {
    sel = gsp[0x858] & 0x7F;
    if (((u32) sel) < 2U)
    {
      flags = *((s32 *) (gsp + 0x20));
      flags &= 0xFE03FFFF;
      do
      {
        bits = sel << 0x12;
      }
      while (0);
    }
    else
    {
      flags = (*((s32 *) (gsp + 0x20))) & 0xFE03FFFF;
      bits = ((gsp[0x859] + 2) & 0x7F) << 0x12;
    }
    flags |= bits;
    *((s32 *) (gsp + 0x20)) = flags;
  }
  else
  {
    *((s32 *) (gsp + 0x20)) |= 0x01FC0000;
  }
  if (((u8 *) D_8012271C)[0xA90] != 0)
  {
    if (((*((s32 *) (((u8 *) D_8012271C) + 0xAA8))) & 0x7F) == 4)
    {
      *((s32 *) (((u8 *) D_8012271C) + 0x20)) = ((*((s32 *) (((u8 *) D_8012271C) + 0x20))) & 0x01FFFFFF) | ((((u8 *) D_8012271C)[0xAA9] + 0x4F) << 0x19);
      ((u8 *) D_8012271C)[0x1F] = ((u8 *) D_8012271C)[(((s8) ((u8 *) D_8012271C)[0x29D7]) * 0x14C) + (new_var2 = 0x2B54)];
    }
    else
    {
      *((s32 *) (((u8 *) D_8012271C) + 0x20)) = ((*((s32 *) (((u8 *) D_8012271C) + 0x20))) & 0x01FFFFFF) | ((((u8 *) D_8012271C)[0xAA9] + 0xE) << 0x19);
    }
  }
  else
  {
    *((s32 *) (((u8 *) D_8012271C) + 0x20)) |= 0xFE000000;
  }
  i = 0;
  do
  {
    fill_p = ((u8 *) D_8012271C) + i;
    i++;
    outer_n = 0x5F0;
    fill_p[0] = fill_p[outer_n];
    outer_n = 0;
  }
  while (i < 0x15);
  ((u8 *) D_8012271C)[0x15] = ((u8 *) D_8012271C)[0x610];
  scan_i = outer_n;
  ((u8 *) D_8012271C)[0x16] = ((u8 *) D_8012271C)[0x664];
  scan_p = (u8 *) D_8012271C;
  new_var = 0x3268;
  do
  {
    if (scan_p[0x3160] != 0)
    {
      outer_n++;
    }
    scan_i++;
    scan_p += 0x40;
  }
  while (scan_i < 4);
  ((u8 *) D_8012271C)[0x17] = outer_n;
  func_80016E7C(D_8012271C, base + 0x180, new_var);
  *((s32 *) (base + 0x198)) = ((*((s32 *) (base + 0x198))) & 0xFE000000) | 6;
  rng = func_80016F5C();
  *((s16 *) (base + 0x256)) = rng | (func_80016F5C() << 0xF);
  *((s32 *) (base + 0x33E0)) = func_80143414(base);
  *((s32 *) (base + 0x33E4)) = 0x414E41;
  src = base + 4;
  tail_src = &D_8014BEE4[0];
  outer_n = 0;
  do
  {
    do
    {
      outer_n++;
      *src = *tail_src;
      tail_src++;
      src++;
    }
    while (0);
  }
  while (outer_n < 0x12);
}

u8 *func_80143334(void *arg0)
{
    u8 *p = arg0;

    while ((u32)(*p - 0x30) < 10 || (u32)(*p - 0x61) < 6 || (u32)(*p - 0x41) < 6)
    {
        p++;
    }
    return p;
}

s32 func_80143380(void)
{
    if (D_80166078 == 0 && ((*(u32 *)((u8 *)D_8012271C + 0x28) >> 2) & 1))
    {
        return 1;
    }
    return 0;
}

s32 func_801433C0(u8 *base)
{
    if (*(s32 *)(base + 0x33E0) == func_80143414(base))
    {
        if (*(s32 *)(base + 0x33E4) == 0x414E41)
        {
            return 1;
        }
    }
    return 0;
}

s32 func_80143414(u8 *data)
{
    s32 sum;
    u32 i;
    u8 *p;

    p = data;
    sum = 0;
    i = 0;
    do
    {
        i += 1;
        sum += *p;
        p += 1;
    } while (i < 0x33E0U);
    return (sum * 2) + 0x0414E410;
}

s32 func_8014344C(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    s32 result;
    s32 x;
    s32 status;
    CardaElement *p;

    x = -arg2 + 0x90;
    result = func_80144F28(
        func_800A88A0(prim, ot,
                      (u8 *)&D_8014B068 + D_8014B068 - 0x30,
                      4, x, -arg3, 2),
        ot, x, 0xE - arg3);

    if ((u32)(func_80149638() - 1) < 2U)
    {
        D_80165F80[0].attr.f.state = 0;
        field_reset_input_repeat();
        func_800A3938(0x78, 0x80);
        D_80165FEC = 0xFF;
        func_80147C5C();
        if ((u32)(D_80166078 - 2) < 2U)
        {
            D_801663A0 = D_80165B70;
        }
        else
        {
            D_801663A0 = 0;
        }
    }
    else
    {
        status = D_80122988;
        if (status & 0x40)
        {
            D_80165F80[0].attr.f.state = 0;
            field_reset_input_repeat();
            func_800A3938(0x78, 0x80);
            D_801663A0 = D_80165B84;
        }
        else if (status & 0x220)
        {
            if (D_80165FF8 != 0)
            {
                D_80165F80[0].attr.f.state = 0;
                field_reset_input_repeat();
                func_800A3938(0x78, 0x80);
                D_801663A0 = D_80165B84;
            }
            else
            {
                func_800A3938(0x7E, 0x80);
                D_80166070 = 1;
                D_801663A0 = D_80165BA4;
                p = D_80165F80;
                p->draw_handler = func_8014366C;
                p->attr.f.unk0_3 = 1;
                p->attr.f.state = 1;
                p->attr.f.x = 0x10;
                p->attr.f.unk0_16 = 0x5A;
                p->attr4.f.unk4_0 = 1;
                p->attr4.f.y = 0x2C;
                SET_ELEM_CODE(p, 0x20);
            }
        }
    }
    return result;
}

s32 func_8014366C(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    u8 *base;
    u8 *resource;
    CardaElement *p;
    CardaElement *cursor;
    s32 result;
    s32 x;
    s32 i;
    s32 valid;
    s32 checksum;

    x = -arg2 + 0x90;
    result = func_800A88A0(prim, ot, (void *)((s32)&D_8014B06A - 0x32 + D_8014B06A), 4, x, -arg3, 2);
    base = (u8 *)&D_8014B06A - 0x32;
    result = func_800A88A0(result, ot, base + *(u16 *)(base + 0x1E), 4, x, 0xE - arg3, 2);
    result = func_800A88A0(result, ot, base + *(u16 *)(base + 0xB2), 4, x, 0x1C - arg3, 2);
    result = func_8014385C(result, ot);

    if (D_80166070 == 0)
    {
        resource = D_80165FF0;
        p = D_80165F80;
        p->attr.f.state = 0;
        checksum = func_80143414(resource);
        valid = 0;
        if (*(s32 *)(resource + 0x33E0) == checksum)
        {
            valid = *(s32 *)(resource + 0x33E4) == 0x414E41;
        }
        if (valid == 0)
        {
            func_801447DC(4);
            return result;
        }

        func_800A3938(0x7B, 0x80);
        func_80016E7C(resource + 0x180, D_8012271C, 0x3268);
        D_80042FB4 = func_8002054C(-1);
        func_80067F28();

        cursor = p;
        for (i = 0; i < 8; i++, cursor++)
        {
            if (cursor->attr.f.state != 0)
            {
                cursor->attr.f.state = 3;
                cursor->attr.f.unk0_3 = 8;
            }
        }
        func_80067EB4(0, 0, 0, 8);
    }

    return result;
}

typedef struct
{
    s32 unk0;
    s32 unk4;
    s16 unk8;
    s16 unkA;
    s32 unkC;
    s16 unk10;
    s16 unk12;
    s32 unk14;
    s16 unk18;
    s16 unk1A;
    s32 unk1C;
    s16 unk20;
    s16 unk22;
} CardaPolyG4Words;

/**
 * @brief Build and link CARDA's memory-card progress-bar gouraud quad.
 *
 * The progress rate is mode-dependent, then clamped to 0x100 before being
 * converted to the quad's right-edge extent.
 * @see matching: 100.00%
 */
s32 func_8014385C(s32 arg0, s32 *arg1)
{
    CardaPolyG4Words *g;
    s32 elapsed;
    s32 extent;
    s32 color;

    if (D_80166ADC != 0)
    {
        elapsed = func_8002054C(-1) - D_80166B8C;
        if ((u32)(D_80166078 - 2) < 2)
        {
            if (D_80165FEC == 0xF4)
            {
                elapsed *= 0x10;
            }
            else
            {
                elapsed /= 3;
            }
        }
        if (elapsed >= 0x101)
        {
            elapsed = 0x100;
        }
        color = 0xFFFF00;
        ((CardaPolyG4Words *)arg0)->unk4 = 0xFF;
        ((CardaPolyG4Words *)arg0)->unkC = 0xFFFF;
        ((CardaPolyG4Words *)arg0)->unk1C = 0xFF0000;
        setlen(arg0, 8);
        setcode(arg0, 0x38);
        ((CardaPolyG4Words *)arg0)->unk14 = color;
        /*
         * These single-iteration scopes preserve GCC 2.7.2's reference
         * weighting for the packet pointer and the x2 store.
         */
        do
        {
            do
            {
                g = (CardaPolyG4Words *)arg0;
                ((CardaPolyG4Words *)arg0)->unk18 = 0;
            } while (0);
            extent = elapsed * 0x120;
            ((CardaPolyG4Words *)arg0)->unk8 = 0;
            if (extent < 0)
            {
                g = (CardaPolyG4Words *)arg0;
                extent += 0xFF;
            }
            ((CardaPolyG4Words *)arg0)->unk20 = extent >> 8;
        } while (0);
        ((CardaPolyG4Words *)arg0)->unk10 = extent >> 8;
        arg0 = (s32)((u8 *)g + 0x24);
        g->unk12 = 0;
        g->unkA = 0;
        g->unk22 = 0x48;
        g->unk1A = 0x48;
        addPrim(arg1, g);
    }
    return arg0;
}

s32 func_801439B4(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    s32 result;
    s32 x;
    s32 status;
    CardaElement *p;

    x = -arg2 + 0x90;
    result = func_80144F28(
        func_800A88A0(prim, ot,
                      (u8 *)&D_8014B04E + D_8014B04E - 0x16,
                      4, x, -arg3, 2),
        ot, x, 0xE - arg3);

    if ((u32)(func_80149638() - 1) < 2U)
    {
        D_80165F80[0].attr.f.state = 0;
        field_reset_input_repeat();
        func_800A3938(0x78, 0x80);
        D_80165FEC = 0xFF;
        func_80147C5C();
        D_801663A0 = 0;
    }
    else
    {
        status = D_80122988;
        if (status & 0x40)
        {
            D_80165F80[0].attr.f.state = 0;
            field_reset_input_repeat();
            func_800A3938(0x78, 0x80);
            D_801663A0 = D_80165B84;
        }
        else if (status & 0x220)
        {
            if (D_80165FF8 != 0)
            {
                D_80165F80[0].attr.f.state = 0;
                field_reset_input_repeat();
                func_800A3938(0x78, 0x80);
                D_801663A0 = D_80165B84;
            }
            else
            {
                func_800A3938(0x7E, 0x80);
                D_80166ADC = 0;
                D_80166118 = 1;
                if ((u32)(D_80166078 - 2) < 2U)
                {
                    D_801663A0 = D_80165B91;
                }
                else
                {
                    D_801663A0 = D_80165B89;
                }

                p = D_80165F80;
                p->draw_handler = func_80143DF4;
                p->attr.f.unk0_3 = 1;
                p->attr.f.state = 1;
                p->attr.f.x = 0x10;
                p->attr.f.unk0_16 = 0x5A;
                p->attr4.f.unk4_0 = 1;
                p->attr4.f.y = 0x2C;
                SET_ELEM_CODE(p, 0x20);
            }
        }
    }
    return result;
}

s32 func_80143BD4(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    s32 result;
    s32 x;
    s32 status;
    CardaElement *p;

    x = -arg2 + 0x90;
    result = func_80144F28(
        func_800A88A0(prim, ot,
                      (u8 *)&D_8014B050 + D_8014B050 - 0x18,
                      4, x, -arg3, 2),
        ot, x, 0xE - arg3);

    if ((u32)(func_80149638() - 1) < 2U)
    {
        D_80165F80[0].attr.f.state = 0;
        field_reset_input_repeat();
        func_800A3938(0x78, 0x80);
        D_80165FEC = 0xFF;
        func_80147C5C();
        D_801663A0 = 0;
    }
    else
    {
        status = D_80122988;
        if (status & 0x40)
        {
            D_80165F80[0].attr.f.state = 0;
            field_reset_input_repeat();
            func_800A3938(0x78, 0x80);
            D_801663A0 = D_80165B84;
        }
        else if (status & 0x220)
        {
            if (D_80165FF8 != 0)
            {
                D_80165F80[0].attr.f.state = 0;
                field_reset_input_repeat();
                func_800A3938(0x78, 0x80);
                D_801663A0 = D_80165B84;
            }
            else
            {
                func_800A3938(0x7E, 0x80);
                D_80166ADC = 0;
                D_80166118 = 1;
                if ((u32)(D_80166078 - 2) < 2U)
                {
                    D_801663A0 = D_80165B90;
                }
                else
                {
                    D_801663A0 = D_80165B88;
                }

                p = D_80165F80;
                p->draw_handler = func_80143DF4;
                p->attr.f.unk0_3 = 1;
                p->attr.f.state = 1;
                p->attr.f.x = 0x10;
                p->attr.f.unk0_16 = 0x5A;
                p->attr4.f.unk4_0 = 1;
                p->attr4.f.y = 0x2C;
                SET_ELEM_CODE(p, 0x20);
            }
        }
    }
    return result;
}

s32 func_80143DF4(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    s32 x;
    s32 result;
    u8 *base;
    CardaElement *p;
    RECT pos;

    x = -arg2 + 0x90;
    result = func_800A88A0(prim, ot, (void *)((s32)&D_8014B054 - 0x1C + D_8014B054), 4, x, -arg3, 2);
    base = (u8 *)&D_8014B054 - 0x1C;
    result = func_800A88A0(result, ot, base + *(u16 *)(base + 0x1E), 4, x, 0xE - arg3, 2);
    result = func_800A88A0(result, ot, base + *(u16 *)(base + 0xB2), 4, x, 0x1C - arg3, 2);
    result = func_8014385C(result, ot);

    if (D_80166118 == 0)
    {
        func_800A3938(0x7A, 0x80);
        D_80165FEC = 0xFF;
        p = D_80165F80;
        p->draw_handler = func_80143F90;
        p->attr.f.unk0_3 = 1;
        p->attr.f.state = 1;
        p->attr.f.x = 0x10;
        p->attr.f.unk0_16 = 0x68;
        p->attr4.f.unk4_0 = 1;
        p->attr4.f.y = 0x10;
        SET_ELEM_CODE(p, 0x20);
    }
    return result;
}

s32 func_80143F90(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    s32 result;

    result = func_800A88A0(prim, ot, (void *)((s32)&D_8014B058 - 0x20 + D_8014B058), 4,
                            -arg2 + 0x90, -arg3, 2);
    if (D_80122988 & 0x260)
    {
        func_800A3938(0x7D, 0x80);
        D_80165F80[0].attr.f.state = 0;
        field_reset_input_repeat();
    }
    else if (D_80165FEC == 0xFD)
    {
        D_80165F80[0].attr.f.state = 0;
        field_reset_input_repeat();
    }
    return result;
}

s32 func_80144050(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;
    s32 x;
    s32 status;
    s32 code;
    u8 *base;
    CardaElement *p;
    s32 aa3;

    aa3 = arg3;
    if ((u32)(D_80166078 - 2) < 2U)
    {
        prim = func_800A88A0(prim, ot, (void *)((s32)&D_8014B0B4 - 0x7C + D_8014B0B4), 4, -arg2 + 0x90, -aa3, 2);
        base = (u8 *)&D_8014B0B4 - 0x7C;
        prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0x24), 4, -arg2 + 0x90, 0xE - aa3, 2);
    }
    else
    {
        prim = func_800A88A0(prim, ot, (void *)((s32)&D_8014B0EC - 0xB4 + D_8014B0EC), 4, -arg2 + 0x90, -aa3, 2);
    }
    x = -arg2 + 0x90;
    prim = func_800A88A0(prim, ot, (void *)((s32)&D_8014B05E - 0x26 + D_8014B05E), 4, x, 0x1C - aa3, 2);
    prim = func_80144F28(prim, ot, x, 0x2A - aa3);

    if ((u32)(func_80149638() - 1) < 2U)
    {
        func_800A3938(0x7D, 0x80);
        D_80165F80[0].attr.f.state = 0;
        field_reset_input_repeat();
        D_80165FEC = 0xFF;
        func_80147C5C();
        if ((u32)(D_80166078 - 2) < 2U)
        {
            D_801663A0 = D_80165B84;
        }
        else
        {
            D_801663A0 = 0;
        }
    }
    else
    {
        status = D_80122988;
        if (status & 0x40)
        {
            goto block_common;
        }
        if (!(status & 0x220))
        {
            goto done;
        }
        if (D_80165FF8 == 0)
        {
            goto draw;
        }
    block_common:
        D_801660F8 = 1;
        func_800A3938(0x7D, 0x80);
        D_80165F80[0].attr.f.state = 0;
        field_reset_input_repeat();
        D_80165FEC = 0xFF;
        func_80147C5C();
        D_80165FEC = 0xF9;
        if ((u32)(D_80166078 - 2) < 2U)
        {
            D_801663A0 = 0;
        }
        else
        {
            D_801663A0 = D_80165B84;
        }
        goto done;
    draw:
        func_800A3938(0x7E, 0x80);
        D_80165F80[0].attr.f.state = 0;
        func_80147C5C();
        D_80166074 = 0;
        p = D_80165F80;
        p->attr.f.unk0_3 = 1;
        p->attr.f.state = 1;
        if ((u32)(D_80166078 - 2) < 2U)
        {
            p->attr.f.x = 0x10;
            p->attr.f.unk0_16 = 0x4C;
            code = p->attr.word;
            code &= 0x00FFFFFF;
            code |= 0x20000000;
            p->attr.word = code;
            p->attr4.word = ((p->attr4.word | 1) & ~0x1FE) | 0x90;
        }
        else
        {
            p->attr.f.x = 0x10;
            p->attr.f.unk0_16 = 0x5A;
            code = p->attr.word;
            code &= 0x00FFFFFF;
            code |= 0x20000000;
            p->attr.word = code;
            p->attr4.word = ((p->attr4.word | 1) & ~0x1FE) | 0x58;
        }
        p->draw_handler = func_801443F0;
    }
done:
    return prim;
}

/**
 * @brief Draw the active CARDA glyphs and advance the associated state.
 * @param ot Ordering table used for the glyph primitives.
 * @param prim Current primitive packet cursor.
 * @param arg2 Horizontal placement offset.
 * @param arg3 Vertical placement offset.
 * @return Updated primitive packet cursor.
 */
s32 func_801443F0(s32 *ot, s32 prim, s32 arg2, s32 arg3)
{
    RECT pos;

    if (D_80166074 >= 0xD)
    {
        if ((u32)(D_80166078 - 2) < 2U)
        {
            s32 x;
            u8 *base;

            x = -arg2 + 0x90;
            prim = func_800A88A0(prim, ot, (void *)((s32)&D_8014B0DA - 0xA2 + D_8014B0DA), 4, x, -arg3, 2);
            base = (u8 *)&D_8014B0DA - 0xA2;
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0x7A), 4, x, 0xE - arg3, 2);
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0xB2), 4, x, 0x1C - arg3, 2);
        }
        else
        {
            s32 x;
            u8 *base;

            x = -arg2 + 0x90;
            prim = func_800A88A0(prim, ot, (void *)((s32)&D_8014B054 - 0x1C + D_8014B054), 4, x, -arg3, 2);
            base = (u8 *)&D_8014B054 - 0x1C;
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0x1E), 4, x, 0xE - arg3, 2);
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0xB2), 4, x, 0x1C - arg3, 2);
        }
    }
    else
    {
        s32 x;
        s32 mode = 2;
        u8 *base;
        u16 *anchor;

        x = -arg2 + 0x90;
        anchor = &D_8014B090;
        base = (u8 *)anchor - 0x58;
        prim = func_800A88A0(prim, ot, base + *anchor, 4, x, -arg3, mode);
        if ((u32)(D_80166078 - 2) < 2U)
        {
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0x7A), 4, x, 0xE - arg3, mode);
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0xB2), 4, x, 0x1C - arg3, mode);
        }
        else
        {
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0x1E), 4, x, 0xE - arg3, mode);
            prim = func_800A88A0(prim, ot, base + *(u16 *)(base + 0xB2), 4, x, 0x1C - arg3, mode);
        }
    }

    if (D_80166074 == 0xC)
    {
        func_80149554();
        D_801663A0 = D_80165BA8;
    }
    else if (D_80166074 >= 0xD)
    {
        D_80165FF4 = 0;
        D_80166A80[0] = 1;
        func_80142E10();
        D_80166ADC = 0;
        D_80166118 = 1;
        if ((u32)(D_80166078 - 2) < 2U)
        {
            D_80165FE8 = 0;
            D_801229B0 = 5;
            D_80165F3C = 1;
            func_80146694();
            D_801663A0 = D_80165B91;
            D_80165FEC = 0xF3;
            D_80165F80[0].attr.f.state = 0;
        }
        else
        {
            CardaElement *p;

            D_801663A0 = D_80165B89;
            p = D_80165F80;
            p->draw_handler = func_80143DF4;
            p->attr.f.unk0_3 = 1;
            p->attr.f.state = 2;
            p->attr.f.x = 0x10;
            p->attr.f.unk0_16 = 0x5A;
            p->attr4.f.unk4_0 = 1;
            p->attr4.f.y = 0x2C;
            SET_ELEM_CODE(p, 0x20);
        }
    }
    D_80166074 += 1;
    return prim;
}

/**
 * @brief Select and initialize the CARDA status dialog for the requested state.
 * @param arg0 Status-dialog state index.
 */
void func_801447DC(u32 arg0)
{
    s32 state;
    CardaElement *p;
    if (D_80165FE4 != arg0 || !(D_80165F80[0].attr.word & 7) ||
        D_80165F80[0].draw_handler != (void *)func_80144A24)
    {
        D_80166118 = 0;
        D_80166070 = 0;
        D_801660FC = 0;
        D_80166000 = 0;
        func_80147C5C();
        D_80165FE4 = arg0;
        func_8001729C(D_801660A0);
        func_800A3938(0x78, 0x80);

        if ((u32)(D_80166078 - 2) < 2)
        {
            state = D_80165FE4;
            switch (state)
            {
            case 0:
                D_80165FEC = 0xF0;
                break;
            case 1:
                D_80165FEC = 0xEF;
                break;
            case 2:
                D_80165FEC = 0xEE;
                break;
            case 3:
                D_80165FEC = 0xED;
                break;
            case 4:
                D_80165FEC = 0xEC;
                break;
            case 5:
                D_80165FEC = 0xEB;
                break;
            }
            D_801663A0 = 0;
            return;
        }

        D_80165F80[0].attr.word = (((((((D_80165F80[0].attr.word & ~0x78U) | 8) & ~7U) | 1) & 0xFFFF007FU) | 0x1000) & 0xFFFFFFU);
        p = D_80165F80;
        p->attr4.word |= 1;
        if ((s32)arg0 < 2 || arg0 == 4 || arg0 == 5)
        {
            ((u8 *)p)[2] = 0x60;
            p->attr4.word = (p->attr4.word & ~0x1FEU) | 0x48;
        }
        else
        {
            p->attr4.word = (p->attr4.word & ~0x1FEU) | 0x28;
            ((u8 *)p)[2] = 0x70;
        }
        p->draw_handler = (void *)func_80144A24;
        field_reset_input_repeat();
        D_80166118 = 0;
        D_80166070 = 0;
        D_801660FC = 0;
        D_80166000 = 0;
        D_80165FEC = 0xFF;
        func_80147C5C();
        D_801663A0 = D_80165B70;
        D_80165FE4 = arg0;
        func_8001729C(D_801660A0);
    }
}

/**
 * @brief Draw the active CARDA status dialog and handle dismissal input.
 * @param ot Ordering table used by the text renderer.
 * @param prim Current primitive-buffer cursor.
 * @param x Horizontal transition offset.
 * @param y Vertical transition offset.
 * @return Advanced primitive-buffer cursor.
 * @see matching: 100.00%
 */
s32 func_80144A24(s32 *ot, s32 prim, s32 x, s32 y)
{
    u8 *base;
    s32 frame_scratch[2];
    s32 state = D_80165FE4;

    switch (state)
    {
    case 0:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B074, 0x3C), 4, -x + 0x80, -y, 2);
        base = (u8 *)&D_8014B074 - 0x3C;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x56), 4, -x + 0x80, -y + 0x10, 2);
        break;
    case 1:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B076, 0x3E), 4, -x + 0x80, -y, 2);
        base = (u8 *)&D_8014B076 - 0x3E;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x56), 4, -x + 0x80, -y + 0x10, 2);
        break;
    case 2:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B078, 0x40), 4, -x + 0x80, -y, 2);
        break;
    case 3:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B07A, 0x42), 4, -x + 0x80, -y, 2);
        if (D_80165FEC != 0xFD)
        {
            break;
        }
        goto clear_state;
    case 4:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B076, 0x3E), 4, -x + 0x80, -y, 2);
        base = (u8 *)&D_8014B076 - 0x3E;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x5C), 4, -x + 0x80, -y + 0x10, 2);
        break;
    case 5:
        prim = func_800A88A0(prim, ot, GLYPH_SYM(D_8014B09C, 0x64), 4, -x + 0x80, -y, 2);
        base = (u8 *)&D_8014B09C - 0x64;
        prim = func_800A88A0(prim, ot, GLYPH_OFF(base, 0x56), 4, -x + 0x80, -y + 0x10, 2);
        break;
    }

    if (D_80122988 & 0x220) {
        switch (D_80166078) {
        case 2:
            g_field_card_overlay_mode = 6;
            break;
        case 3:
            g_field_card_overlay_mode = 7;
            break;
        default:
            g_field_card_overlay_mode = 3;
            break;
        }
clear_state:
        D_80165F80[0].attr.word &= ~7;
        field_reset_input_repeat();
    }
    return prim;
}

typedef struct {
    s32 unk0; s32 unk4; s16 unk8; s16 unkA; u8 unkC; u8 unkD; s16 unkE;
    s16 unk10; s16 unk12; u8 unk14; u8 unk15; s16 unk16; s16 unk18;
    s16 unk1A; u8 unk1C; u8 unk1D; u8 pad1E[2]; s16 unk20; s16 unk22;
    u8 unk24; u8 unk25; u8 pad26[2];
} GlyphPrim;

s32 func_80144CD0(s32 result, s32 *ot, s32 x, s32 y, s32 adjust, s32 slot, s32 i, s32 j)
{
    RECT rect;
    s32 temp;
    s8 shade;

    if (slot == 0x7F)
    {
        return result;
    }
    setRECT(&rect, i * 0x10, 0x1F2, 0x10, 1);
    if ((j == 1) && (slot < 2)) {
        func_800A5638(D_80166080, slot);
        func_80019A34(&rect, D_80166080);
        func_80019788(0);
    } else if (slot >= 0x4F) {
        func_800A55E4(D_80166080, D_8016606C);
        func_80019A34(&rect, D_80166080);
        func_80019788(0);
    } else {
        func_80019A34(&rect, (void *)((u8 *)&D_8014CC54 - 4 + D_8014CC54[slot]));
    }
    temp = i * 3;
    setRECT(&rect, temp * 4 + 0x140, 0xD0, 0xC, 0x30);
    func_80019A34(&rect, (void *)((u8 *)&D_8014CC54 + 0x1C + D_8014CC54[slot]));
    ((GlyphPrim *)result)->unk4 = 0x808080;
    setPolyFT4(result);
    ((GlyphPrim *)result)->unk18 = x;
    ((GlyphPrim *)result)->unk8 = x;
    ((GlyphPrim *)result)->unk12 = y;
    ((GlyphPrim *)result)->unkA = y;
    ((GlyphPrim *)result)->unk20 = x + adjust;
    shade = temp * 0x10;
    ((GlyphPrim *)result)->unk1C = shade;
    ((GlyphPrim *)result)->unkC = shade;
    shade += 0x2F;
    ((GlyphPrim *)result)->unk24 = shade;
    ((GlyphPrim *)result)->unk14 = shade;
    ((GlyphPrim *)result)->unk15 = 0xD0;
    ((GlyphPrim *)result)->unkD = 0xD0;
    ((GlyphPrim *)result)->unk10 = x + adjust;
    ((GlyphPrim *)result)->unk22 = y + 0x2F;
    ((GlyphPrim *)result)->unk1A = y + 0x2F;
    ((GlyphPrim *)result)->unk25 = 0xFF;
    ((GlyphPrim *)result)->unk1D = 0xFF;
    ((GlyphPrim *)result)->unkE = (i & 0x3F) | 0x7C80;
    ((GlyphPrim *)result)->unk16 = 5;
    addPrim(ot, result);
    return result + 0x28;
}

void func_80144F18(void)
{
    D_80165FF8 = 1;
}

s32 func_80144F28(s32 prim, s32 *ot, s32 x, s32 y)
{
    u8 *p;
    u8 *base;
    s32 g1;
    s32 g2;
    s32 hi;
    s32 a3;

    p = (u8 *)&D_800EC3FA;
    hi = p[1] << 8;
    base = p - 0x36;
    a3 = 4;
    g1 = p[0] + (hi + (s32)base);
    if (D_80165FF8 != 0)
    {
        a3 = 5;
    }
    prim = func_800A88A0(prim, ot, (void *)g1, a3, x - 0x10, y, 1);
    a3 = 4;
    g2 = base[0x38] + ((base[0x39] << 8) + (s32)base);
    if (D_80165FF8 == 0)
    {
        a3 = 5;
    }
    prim = func_800A88A0(prim, ot, (void *)g2, a3, x + 8, y, 0);
    if (D_80122988 & 0xA000)
    {
        D_80165FF8 ^= 1;
        func_800A3938(0x7D, 0x80);
        D_80122988 = 0;
    }
    return prim;
}
