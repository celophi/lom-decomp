#include "common.h"
void akao_cmd_f1(void);                       /* extern */
void field_clear_actor_slots(void);           /* extern */
s32 field_find_active_special_attack_actor(); /* extern */
void field_initialize_actor_slots(void);      /* extern */
void field_reset_global_color_scale(void);    /* extern */
void func_8005A0D0(s32, s32, s32, s32);       /* extern */
s32 func_8005B218();                          /* extern */
void func_80067AA4(void);                     /* extern */
void func_80068028(void);                     /* extern */
void func_8006C3FC(u8 *);                     /* extern */
void func_80084240(void);                     /* extern */
s32 func_80096A00();                          /* extern */
s32 func_80096A90();                          /* extern */
void func_800A3938(s32, s32);                 /* extern */
void func_800A6204(void);                     /* extern */
extern s32 D_800F2278;
extern s32 D_800F227C;
extern s32 D_800F2280;
extern u8 D_800FD818[];
extern u8 D_800FDF58[];
extern s32 D_800FE754;
extern u8 D_80105AE0[];
extern s32 D_8010AE54;
extern s32 D_8010AE5C;
extern s32 D_8010CFD0;
extern s32 D_8010D020;
extern s32 D_801227C8;

/** @brief Position and animation fields in a 0x54-byte actor entry. */
typedef struct Entry
{
    s32 x, y, z;
    u8 padC[16];
    s32 flags;
    u8 pad20;
    u8 state;
    u8 pad22[2];
    u8 enabled, slot, pad26, unk27;
    u8 pad28[2];
    s16 anim;
    u8 pad2C[2];
    s16 timer;
    u8 tail[0x54 - 0x30];
} Entry;
/** @brief Resource, status, and overlapping flag fields in a 0x23C-byte actor record. */
typedef struct Actor
{
    u8 pad0[12];
    s32 resource;
    u8 pad10[0x2C];
    s32 value;
    u8 pad40[0x134];
    s32 flags174;
    union
    {
        s32 word;
        u8 bytes[4];
    } flags178;
    u8 tail[0x23C - 0x17C];
} Actor;
/**
 * @brief Complete a pending field reset after fifteen consecutive idle checks.
 * @note Restores global state and resets each active actor in the first three slots.
 */
void func_80096E60(void)
{
    u8 *var_s0;
    u8 *var_s1;
    u8 *var_s2;
    s32 var_v0;
    u8 *slot_base;

    if (D_8010AE54 != 0)
    {
        if ((field_find_active_special_attack_actor() == 0) && (func_80096A00() == 0) &&
            (D_801227C8 == 0) && (func_8005B218() == 0) && (func_80096A90() == 0))
        {
            D_8010CFD0 += 1;
        }
        else
        {
            D_8010CFD0 = 0;
        }
        if (D_8010CFD0 == 0xF)
        {
            field_initialize_actor_slots();
            field_clear_actor_slots();
            func_80067AA4();
            func_80084240();
            D_800F2280 = 0;
            D_800F227C = 0;
            D_800F2278 = 0;
            D_800FE754 = D_8010AE5C;
            func_80068028();
            akao_cmd_f1();
            field_reset_global_color_scale();
            func_8005A0D0(-1, 0x100, 0x100, 0x100);
            func_800A6204();
            func_800A3938(0x24, 0x80);
            var_s1 = D_80105AE0;
            var_s0 = D_800FDF58;
            slot_base = D_800FD818;
            var_s2 = slot_base;
        loop:
        {
            if (*var_s2 & 1)
            {
                ((Actor *)var_s1)->resource = 0;
                ((Actor *)var_s1)->flags178.word &= ~1;
                ((Actor *)var_s1)->flags178.word &= ~2;
                ((Actor *)var_s1)->flags178.word &= ~0x20;
                ((Actor *)var_s1)->flags178.bytes[3] = 0;
                ((Actor *)var_s1)->value = 0xFFFF;
                ((Entry *)var_s0)->y = 0;
                if ((D_8010D020 != 0) && (((Entry *)var_s0)->anim == 0x8E))
                {
                    var_v0 = (((Entry *)var_s0)->state & 0x80) + 0x31;
                }
                else
                {
                    var_v0 = (((Entry *)var_s0)->state & 0x80) + 0x13;
                }
                ((Entry *)var_s0)->state = var_v0;
                ((Entry *)var_s0)->timer = 1;
                ((Entry *)var_s0)->enabled = 1;
                ((Entry *)var_s0)->anim = 0;
                ((Entry *)var_s0)->slot = 0;
                ((Entry *)var_s0)->unk27 = 0;
                ((Entry *)var_s0)->flags = (s32)(((Entry *)var_s0)->flags & ~0x800);
                ((Actor *)var_s1)->flags174 = (s32)(((Actor *)var_s1)->flags174 & ~0x1800);
                func_8006C3FC(var_s0);
            }
            var_s1 += 0x23C;
            var_s2 += 0x268;
            var_s0 += 0x54;
        }
            if ((s32)var_s2 < (s32)(slot_base + 0x738))
            {
                goto loop;
            }
            D_8010AE54 = 0;
        }
    }
}
