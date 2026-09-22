/* Partial WMAP decompilation: 87.020836% (gcc280_g0). */
#include "common.h"

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

void cdrom_queue_read(s32, void *);
M2C_UNK cdrom_wait_queue_empty();                   /* extern */
M2C_UNK func_80064F64(M2C_UNK);                     /* extern */
M2C_UNK func_8006CAC0(void *);                   /* extern */
M2C_UNK func_8006D0F0(M2C_UNK, void *, void *); /* extern */
M2C_UNK func_800A61BC(M2C_UNK);                     /* extern */
extern s32 D_800D9224;
extern u8 D_800DCEF4;
extern u8 D_800DCEF8;
extern u8 D_800DCF00;
extern s32 D_8011CF20;
extern s32 D_8011CF44;
extern s32 D_8011CF50;
extern s32 D_8011D4F8;
extern u8 D_8011D538;
extern u8 D_80123538;
extern u8 D_80129538;
extern s32 D_80129540;
extern s32 D_8012954C;
extern s32 D_8013922C;
extern s32 D_80139238;
extern s32 D_80139248;
extern s32 D_80139834;
extern s32 D_801398C0;
extern s32 D_80139900;
extern s32 D_8013997C;
extern s32 D_8013B288;
extern s32 D_8018222C;
extern s32 D_80182DD4;
extern s32 D_801ADAF0;
extern u8 func_8009A420;
extern u8 func_8009AB20;
extern u8 func_800A7370;
extern u8 func_800A7668;
extern u8 func_800A7BE8;
extern u8 func_800A7FD0;
extern u8 func_800A83B8;
extern u8 func_800A87D4;
extern u8 func_800A88D8;
extern u8 func_800AB850;
extern u8 func_800B2080;
extern u8 func_800B45B8;
extern u8 func_800C2274;

void func_800A5DFC(void)
{
    M2C_UNK var_a0_2;
    s32 *var_a2;
    u32 var_a0;
    u8 *temp_v1;

    if (D_8011CF44 == 0)
    {
        D_8011CF50 = 1;
        D_801398C0 = 0;
        D_8013922C = 0;
        if (D_80182DD4 != 0)
        {
            D_80182DD4 = 0;
            func_8006CAC0(&func_800C2274);
        }
        else if (D_8013B288 != 0)
        {
            D_8013B288 = 0;
            func_8006CAC0(&func_8009A420);
        }
        else if (D_8013997C != 0)
        {
            cdrom_queue_read(0x1145, &D_8011D538);
            cdrom_queue_read(0x1146, (u8 *)&D_8011D538 + 0x2000);
            cdrom_wait_queue_empty();
            D_8013997C = 0;
            func_8006CAC0(&func_800A7370);
        }
        else if (D_8011CF20 != 0)
        {
            D_8011CF20 = 0;
            func_8006CAC0(&func_8009AB20);
        }
        else if (D_801ADAF0 != 0)
        {
            D_8012954C = 0;
            func_8006CAC0(&func_800B45B8);
        }
        else if (D_8012954C != 0)
        {
            D_8012954C = 0;
            func_8006CAC0(&func_800B2080);
        }
        else if (D_8011D4F8 != 0)
        {
            D_8011D4F8 = 0;
            func_8006CAC0(&func_800AB850);
        }
        else if (D_80139248 != 0)
        {
            D_80139248 = 0;
            func_8006CAC0(&func_800A7BE8);
        }
        else if (D_80129540 != 0)
        {
            D_80129540 = 0;
            func_8006CAC0(&func_800A7FD0);
        }
        else if (D_80139900 != 0)
        {
            D_80139900 = 0;
            func_8006CAC0(&func_800A83B8);
        }
        else if (D_80139238 != 0)
        {
            D_80139238 = 0;
            func_8006CAC0(&func_800A87D4);
        }
        else if (D_80139834 != 0)
        {
            D_80139834 = 0;
            func_8006CAC0(&func_800A88D8);
        }
        else
        {
            var_a0 = 0;

loop_26:
            temp_v1 = var_a0 + (u8 *)&D_80129538;
            if (*temp_v1 != 0)
            {
                *temp_v1 = 0;
                if (var_a0 < 5U)
                {
                    switch (var_a0)               
                    {
                    case 0:
                        cdrom_queue_read(0x1149, &D_80123538);
                        cdrom_wait_queue_empty();
                        func_80064F64(0x114A);
                        func_8006D0F0(4, &D_800DCEF8, &D_800DCF00);
                        D_8018222C = 1;
                        M2C_FIELD(&D_800DCEF4, s8 *, 3) = 1;
                        M2C_FIELD(&D_800DCEF4, s8 *, 2) = 1;
                        M2C_FIELD(&D_800DCEF4, s8 *, 1) = 1;
                        M2C_FIELD(&D_800DCEF4, s8 *, 0) = 1;
                        break;
                    case 1:
                        cdrom_queue_read(0x10DE, (u8 *)&D_80123538 + 0x3538);
                        cdrom_wait_queue_empty();
                        func_80064F64(0x10DF);
                        var_a0_2 = 0xB;
block_34:
                        func_800A61BC(var_a0_2);
                        break;
                    case 2:
                        cdrom_queue_read(0x10D8, (u8 *)&D_80123538 + 0x3538);
                        cdrom_wait_queue_empty();
                        func_80064F64(0x10D9);
                        var_a0_2 = 0x11;
                        goto block_34;
                    case 3:
                        cdrom_queue_read(0x10DC, (u8 *)&D_80123538 + 0x3538);
                        cdrom_wait_queue_empty();
                        func_80064F64(0x10DD);
                        var_a0_2 = 0xB;
                        goto block_34;
                    case 4:
                        cdrom_queue_read(0x10DA, (u8 *)&D_80123538 + 0x3538);
                        cdrom_wait_queue_empty();
                        func_80064F64(0x10DB);
                        var_a0_2 = 0xA;
                        goto block_34;
                    }
                }
                func_8006CAC0(&func_800A7668);
            }
            else
            {
                var_a0 += 1;

                if ((s32) var_a0 >= 8)
                {

                }
                else
                {
                    goto loop_26;
                }
            }
        }
        D_800D9224 -= 1;
    }
}
