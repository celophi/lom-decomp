/* Partial WMAP decompilation: 75.426994% (gcc280_g0). */
#include "common.h"

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

#include "sdk/libgte.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"
M2C_UNK func_8006432C(void *, void *, void *, void *, void *, void *); /* extern */
extern s32 D_800D921C;
extern u8 D_80139870;
extern u8 D_80139888;
extern void *D_801398EC;
extern u8 D_8013B238;
extern u8 D_8013B240;
extern u8 D_801B2478;
extern u8 D_801B2490;
extern u8 D_801B24A8;

void func_8006454C(void)
{
    VECTOR sp88;
    VECTOR sp78;
    VECTOR sp68;
    VECTOR sp58;
    VECTOR sp48;
    VECTOR sp38;
    MATRIX sp18;
    void *temp_a0;
    void *temp_a0_2;

    M2C_FIELD(&D_8013B238, u16 *, 4) = (u16) (M2C_FIELD(&D_8013B238, u16 *, 4) + M2C_FIELD(&D_801B24A8, u16 *, 4));
    M2C_FIELD(&D_8013B238, u16 *, 2) = (u16) (M2C_FIELD(&D_8013B238, u16 *, 2) + M2C_FIELD(&D_801B24A8, u16 *, 2));
    M2C_FIELD(&D_8013B238, u16 *, 0) = (u16) (M2C_FIELD(&D_8013B238, u16 *, 0) + M2C_FIELD(&D_801B24A8, u16 *, 0));
    M2C_FIELD(&D_80139870, s32 *, 8) = (s32) (M2C_FIELD(&D_80139870, s32 *, 8) + M2C_FIELD(&D_801B2478, s32 *, 8));
    M2C_FIELD(&D_80139870, s32 *, 4) = (s32) (M2C_FIELD(&D_80139870, s32 *, 4) + M2C_FIELD(&D_801B2478, s32 *, 4));
    M2C_FIELD(&D_80139870, s32 *, 0) = (s32) (M2C_FIELD(&D_80139870, s32 *, 0) + M2C_FIELD(&D_801B2478, s32 *, 0));
    RotMatrix(&D_8013B238, &sp18);
    TransMatrix(&sp18, &D_80139870);
    SetRotMatrix(&sp18);
    SetTransMatrix(&sp18);
    M2C_FIELD(&D_801B2490, s16 *, 0) = -0xA0;
    M2C_FIELD(&D_801B2490, s16 *, 2) = -0x78;
    M2C_FIELD(&D_801B2490, s16 *, 4) = 0;
    gte_ldv0(&D_801B2490);
    gte_rtv0tr();
    M2C_FIELD(&D_801B2490, s16 *, 0) = 0x5F;
    M2C_FIELD(&D_801B2490, s16 *, 2) = -0x78;
    M2C_FIELD(&D_801B2490, s16 *, 4) = 0;
    gte_stlvnl(&sp38);
    gte_ldv0(&D_801B2490);
    gte_rtv0tr();
    M2C_FIELD(&D_801B2490, s16 *, 0) = -0xA0;
    M2C_FIELD(&D_801B2490, s16 *, 2) = 0x78;
    M2C_FIELD(&D_801B2490, s16 *, 4) = 0;
    gte_stlvnl(&sp48);
    gte_ldv0(&D_801B2490);
    gte_rtv0tr();
    M2C_FIELD(&D_801B2490, s16 *, 0) = 0x5F;
    M2C_FIELD(&D_801B2490, s16 *, 2) = 0x78;
    M2C_FIELD(&D_801B2490, s16 *, 4) = 0;
    gte_stlvnl(&sp58);
    gte_ldv0(&D_801B2490);
    gte_rtv0tr();
    M2C_FIELD(&D_801B2490, s16 *, 0) = 0xA0;
    M2C_FIELD(&D_801B2490, s16 *, 2) = -0x78;
    M2C_FIELD(&D_801B2490, s16 *, 4) = 0;
    gte_stlvnl(&sp68);
    gte_ldv0(&D_801B2490);
    gte_rtv0tr();
    M2C_FIELD(&D_801B2490, s16 *, 0) = 0xA0;
    M2C_FIELD(&D_801B2490, s16 *, 2) = 0x78;
    M2C_FIELD(&D_801B2490, s16 *, 4) = 0;
    gte_stlvnl(&sp78);
    gte_ldv0(&D_801B2490);
    gte_rtv0tr();
    gte_stlvnl(&sp88);
    func_8006432C(&sp38, &sp48, &sp58, &sp68, &sp78, &sp88);
    M2C_FIELD(&D_8013B240, u16 *, 4) = (u16) (M2C_FIELD(&D_8013B240, u16 *, 4) - M2C_FIELD(&D_801B24A8, u16 *, 4));
    M2C_FIELD(&D_8013B240, u16 *, 2) = (u16) (M2C_FIELD(&D_8013B240, u16 *, 2) - M2C_FIELD(&D_801B24A8, u16 *, 2));
    M2C_FIELD(&D_8013B240, u16 *, 0) = (u16) (M2C_FIELD(&D_8013B240, u16 *, 0) - M2C_FIELD(&D_801B24A8, u16 *, 0));
    M2C_FIELD(&D_80139888, s32 *, 8) = (s32) (M2C_FIELD(&D_80139888, s32 *, 8) - M2C_FIELD(&D_801B2478, s32 *, 8));
    M2C_FIELD(&D_80139888, s32 *, 4) = (s32) (M2C_FIELD(&D_80139888, s32 *, 4) - M2C_FIELD(&D_801B2478, s32 *, 4));
    M2C_FIELD(&D_80139888, s32 *, 0) = (s32) (M2C_FIELD(&D_80139888, s32 *, 0) - M2C_FIELD(&D_801B2478, s32 *, 0));
    RotMatrix(&D_8013B240, &sp18);
    TransMatrix(&sp18, &D_80139888);
    SetRotMatrix(&sp18);
    SetTransMatrix(&sp18);
    M2C_FIELD(&D_801B2490, s16 *, 0) = -0xA0;
    M2C_FIELD(&D_801B2490, s16 *, 2) = -0x78;
    M2C_FIELD(&D_801B2490, s16 *, 4) = 0;
    gte_ldv0(&D_801B2490);
    gte_rtv0tr();
    M2C_FIELD(&D_801B2490, s16 *, 0) = 0x5F;
    M2C_FIELD(&D_801B2490, s16 *, 2) = -0x78;
    M2C_FIELD(&D_801B2490, s16 *, 4) = 0;
    gte_stlvnl(&sp38);
    gte_ldv0(&D_801B2490);
    gte_rtv0tr();
    M2C_FIELD(&D_801B2490, s16 *, 0) = -0xA0;
    M2C_FIELD(&D_801B2490, s16 *, 2) = 0x78;
    M2C_FIELD(&D_801B2490, s16 *, 4) = 0;
    gte_stlvnl(&sp48);
    gte_ldv0(&D_801B2490);
    gte_rtv0tr();
    M2C_FIELD(&D_801B2490, s16 *, 0) = 0x5F;
    M2C_FIELD(&D_801B2490, s16 *, 2) = 0x78;
    M2C_FIELD(&D_801B2490, s16 *, 4) = 0;
    gte_stlvnl(&sp58);
    gte_ldv0(&D_801B2490);
    gte_rtv0tr();
    M2C_FIELD(&D_801B2490, s16 *, 0) = 0xA0;
    M2C_FIELD(&D_801B2490, s16 *, 2) = -0x78;
    M2C_FIELD(&D_801B2490, s16 *, 4) = 0;
    gte_stlvnl(&sp68);
    gte_ldv0(&D_801B2490);
    gte_rtv0tr();
    M2C_FIELD(&D_801B2490, s16 *, 0) = 0xA0;
    M2C_FIELD(&D_801B2490, s16 *, 2) = 0x78;
    M2C_FIELD(&D_801B2490, s16 *, 4) = 0;
    gte_stlvnl(&sp78);
    gte_ldv0(&D_801B2490);
    gte_rtv0tr();
    gte_stlvnl(&sp88);
    func_8006432C(&sp38, &sp48, &sp58, &sp68, &sp78, &sp88);
    temp_a0 = M2C_FIELD(D_801398EC, void **, 0x33C);
    M2C_FIELD(temp_a0, s16 *, 0x14) = 0x140;
    M2C_FIELD(temp_a0, s16 *, 0xC) = 0x140;
    M2C_FIELD(temp_a0, s16 *, 0x16) = 0xF0;
    M2C_FIELD(temp_a0, s16 *, 0x12) = 0xF0;
    M2C_FIELD(temp_a0, s8 *, 3) = 5;
    M2C_FIELD(temp_a0, s32 *, 4) = 0x101010;
    M2C_FIELD(temp_a0, s8 *, 7) = 0x2A;
    M2C_FIELD(temp_a0, s16 *, 0xE) = 0;
    M2C_FIELD(temp_a0, s16 *, 0xA) = 0;
    M2C_FIELD(temp_a0, s16 *, 0x10) = 0;
    M2C_FIELD(temp_a0, s16 *, 8) = 0;
    M2C_FIELD(temp_a0, s32 *, 0) = (s32) ((M2C_FIELD(temp_a0, s32 *, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x74) & 0xFFFFFF));
    M2C_FIELD(D_801398EC, s32 *, 0x74) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x74) & 0xFF000000) | ((s32) temp_a0 & 0xFFFFFF));
    if (D_800D921C < 0x7D00)
    {
        D_800D921C += 0x18;
        M2C_FIELD(D_801398EC, void **, 0x33C) = (void *) (M2C_FIELD(D_801398EC, void **, 0x33C) + 0x18);
    }
    temp_a0_2 = M2C_FIELD(D_801398EC, void **, 0x33C);
    M2C_FIELD(temp_a0_2, s8 *, 3) = 7;
    M2C_FIELD(temp_a0_2, s8 *, 7) = 0x24;
    M2C_FIELD(temp_a0_2, s32 *, 0x18) = 0x190;
    M2C_FIELD(temp_a0_2, s32 *, 0x10) = 0x190;
    M2C_FIELD(temp_a0_2, s32 *, 8) = 0x190;
    M2C_FIELD(temp_a0_2, s16 *, 0x16) = 0x140;
    M2C_FIELD(temp_a0_2, s32 *, 0) = (s32) ((M2C_FIELD(temp_a0_2, s32 *, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32 *, 0x74) & 0xFFFFFF));
    M2C_FIELD(D_801398EC, s32 *, 0x74) = (s32) ((M2C_FIELD(D_801398EC, s32 *, 0x74) & 0xFF000000) | ((s32) temp_a0_2 & 0xFFFFFF));
    if (D_800D921C < 0x7D00)
    {
        D_800D921C += 0x20;
        M2C_FIELD(D_801398EC, void **, 0x33C) = (void *) (M2C_FIELD(D_801398EC, void **, 0x33C) + 0x20);
    }
}
