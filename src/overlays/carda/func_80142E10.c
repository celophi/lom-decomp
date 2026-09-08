#include "common.h"

typedef struct
{
  s8 b0;
  s8 b1;
  s8 b2;
} Char3;
extern s32 D_80042FB4;
extern void *D_8012271C;
extern Char3 D_8014008C;
extern Char3 D_80140090;
extern u8 D_8014BECC[];
extern u8 D_8014BEE4[];
extern s32 D_8014BF00[];
extern void *D_80165FF0;
extern s32 D_80165FF4;
extern s32 D_80166078;
extern s32 D_80166A80[];
extern void func_80146694(void);
extern s32 func_800170BC(void *, void *, ...);
extern s32 func_8002054C(s32);
extern s8 *func_801471E4(s8 *out, s32 value);
extern void func_8014AEC4(u8 *out, u8 *in);
extern void func_80016E7C();
extern s32 func_80016F5C();
extern s32 func_80143414(u8 *data);
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
