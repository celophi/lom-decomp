#include "wmap_resource_support.h"
#include "wmap_effect_resources.h"
#include "cdrom.h"

extern s32 D_8013B294;
extern s32 D_80182E3C;
extern u8 D_800DCF18[];
extern u8 D_800DD7B4[];
extern u8 D_800DDE54[];
extern u8 D_800DDEF4[];
extern u8 D_800DDF18[];
extern u8 D_800DE39C[];
extern u8 D_800DEA88[];
extern u8 D_800DEED0[];
extern u8 D_800DEF18[];
extern u8 D_800DF378[];
extern u8 D_800DFA64[];
extern u8 D_800DFEAC[];
extern u8 D_800E0114[];
extern u8 D_800E0354[];
extern u8 D_800E0720[];
extern u8 D_800E08E4[];
extern u8 D_800E0EB4[];
extern u8 D_800E0F18[];
extern u8 D_800E0F94[];
extern u8 D_800E1F18[];
extern u8 D_800E2B1C[];
extern u8 D_800E2DD8[];
extern u8 D_800E33AC[];
extern u8 D_800E3980[];
extern u8 D_800E4F18[];
extern u8 D_800E7F64[];
extern u8 D_800E91C0[];
extern u8 D_800EA560[];
extern u8 D_800EAA54[];
extern u8 D_800EBED8[];
extern u8 D_800ECEB4[];
extern u8 D_800ECF18[];
extern u8 D_800ED310[];
extern u8 D_800EE2EC[];
extern u8 D_800F2238[];
extern u8 D_800F3214[];
extern u8 D_800F420C[];
extern u8 D_800FAE60[];
extern u8 D_8011D538[];
extern u8 D_8011F538[];
extern u8 D_80121538[];
extern u8 D_80123538[];
extern u8 D_80125538[];
extern u8 D_80127538[];
extern u8 D_80182E40[];
extern u8 D_8018B240[];
extern u8 D_80193640[];
extern u8* D_8011CF1C;
extern u8* D_8011CF24;
extern u8* D_8011CF28;
extern u8* D_8011CF2C;
extern u8* D_8011CF30;
extern u8* D_8011CF34;
extern u8* D_8011CF38;
extern u8* D_8011CF3C;
extern u8* D_8011CF40;
extern u8* D_8011CF84;
extern void (*D_800D6D5C[])(void);

static void func_800A8B38(s32 resource_index);

/**
 * @brief Wait for pending reads and dispatch a resource set when its index changes.
 * @param index Requested resource-set index; invalid indices dispatch set zero.
 */
void func_800A89DC(s32 index)
{
    cdrom_wait_queue_empty();
    if (index == D_80182E3C)
    {
        return;
    }
    D_80182E3C = index;
    if (index >= 36 || index < 0)
    {
        index = 0;
        D_8013B294 = 1;
    }
    D_800D6D5C[index]();
}

/**
 * @brief Queue a CD read of the given resource into the world-map image buffer.
 * @param resource_index CD resource index to fetch.
 */
void func_800A8AA8(s32 resource_index)
{
    cdrom_queue_read((u16)resource_index, D_80182E40);
}

/**
 * @brief Queue a CD read of the given resource into the world-map image buffer.
 * @param resource_index CD resource index to fetch.
 */
void func_800A8AF0(s32 resource_index)
{
    cdrom_queue_read((u16)resource_index, D_8018B240);
}

/**
 * @brief Queue a CD read of the given resource into the world-map image buffer.
 * @param resource_index CD resource index to fetch.
 */
static void func_800A8B38(s32 resource_index)
{
    cdrom_queue_read((u16)resource_index, D_80193640);
}

/** @brief Queue the world-map effect resource set. */
void func_800A8B80(void)
{
    D_8011CF1C = D_800E1F18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    func_800A8AA8(0x11B0);
    func_800A8AF0(0x11B1);
    func_800A8B38(0x11B2);
    cdrom_queue_read(0x11B3, D_8011F538);
    cdrom_queue_read(0x11B4, D_8011D538);
    cdrom_queue_read(0x11B5, D_80121538);
    cdrom_queue_read(0x11B6, D_80123538);
    cdrom_queue_read(0x11B7, D_8011CF24);
    cdrom_queue_read(0x11B8, D_8011CF1C);
    cdrom_queue_read(0x11B9, D_800DCF18);
}

/** @brief Set effect resource buffers and queue animation and texture reads. */
void func_800A8C80(void)
{
    D_8011CF1C = D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    func_800A8AA8(0x1197);
    func_800A8AF0(0x1198);
    func_800A8B38(0x1199);
    cdrom_queue_read(0x119A, D_8011D538);
    cdrom_queue_read(0x119B, D_8011F538);
    cdrom_queue_read(0x119C, D_80121538);
    cdrom_queue_read(0x119D, D_800DCF18);
    cdrom_queue_read(0x119E, D_8011CF1C);
    cdrom_queue_read(0x119F, D_8011CF24);
}

/** @brief Set effect resource buffers and queue animation and texture reads. */
void func_800A8D6C(void)
{
    func_80064F64(0x1111);
    func_800A8AA8(0x110A);
    func_800A8AF0(0x110B);
    func_800A8B38(0x110C);
    D_8011CF1C = D_800E4F18;
    D_8011CF24 = D_8011CF1C + 0xC000;
    cdrom_queue_read(0x1108, D_8011D538);
    cdrom_queue_read(0x1109, D_8011F538);
    cdrom_queue_read(0x110D, D_8011CF1C);
    cdrom_queue_read(0x110E, D_8011CF24);
    cdrom_queue_read(0x110F, D_8011CF24 + 0x4000);
    cdrom_queue_read(0x1110, D_8011CF24 + 0x5000);
    cdrom_queue_read(0x1112, D_8011CF24 + 0x6000);
}

/** @brief Queue the effect's animation and texture resources. */
void func_800A8E9C(void)
{
    func_800A8AA8(0x111B);
    func_800A8AF0(0x111C);
    func_800A8B38(0x111D);
    cdrom_queue_read(0x111E, D_8011D538);
    cdrom_queue_read(0x111F, D_8011F538);
    cdrom_queue_read(0x1120, D_80121538);
    cdrom_queue_read(0x1121, D_80123538);
    cdrom_queue_read(0x1123, D_80125538);
    cdrom_queue_read(0x1124, D_80127538);
    cdrom_queue_read(0x1122, D_800DCF18);
}

/** @brief Queue the effect's animation and texture resources. */
void func_800A8F74(void)
{
    func_800A8AA8(0x1125);
    func_800A8AF0(0x1126);
    func_800A8B38(0x1127);
    cdrom_queue_read(0x1129, D_8011D538);
    cdrom_queue_read(0x1128, D_8011F538);
    cdrom_queue_read(0x112A, D_80121538);
    cdrom_queue_read(0x112B, D_800DCF18);
}

/** @brief Set effect resource buffers and queue animation and texture reads. */
void func_800A9010(void)
{
    func_800A8AA8(0x112C);
    func_800A8AF0(0x112D);
    func_800A8B38(0x112E);
    cdrom_queue_read(0x112F, D_8011D538);
    cdrom_queue_read(0x1130, D_8011F538);
    cdrom_queue_read(0x1131, D_80121538);
    cdrom_queue_read(0x1132, D_800DCF18);
    D_8011CF1C = D_800DDEF4;
    D_8011CF24 = D_800DEA88;
    D_8011CF28 = D_800DFA64;
    D_8011CF2C = D_800E0114;
    D_8011CF30 = D_800E08E4;
    D_8011CF34 = D_800E0EB4;
}

/** @brief Divide the effect buffer and queue its animation and texture reads. */
void func_800A910C(void)
{
    D_8011CF1C = D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    D_8011CF28 = D_8011CF24 + 0x2000;
    D_8011CF2C = D_8011CF28 + 0x2000;
    D_8011CF30 = D_8011CF2C + 0x2000;
    D_8011CF34 = D_8011CF30 + 0x2000;
    func_800A8AA8(0x1133);
    func_800A8AF0(0x1134);
    func_800A8B38(0x1135);
    cdrom_queue_read(0x1136, D_8011D538);
    cdrom_queue_read(0x1137, D_8011F538);
    cdrom_queue_read(0x1138, D_800DCF18);
    cdrom_queue_read(0x1139, D_8011CF1C);
    cdrom_queue_read(0x113A, D_8011CF28);
    cdrom_queue_read(0x113B, D_8011CF2C);
    cdrom_queue_read(0x113C, D_8011CF30);
}

/** @brief Set effect resource buffers and queue animation and texture reads. */
void func_800A926C(void)
{
    func_800A8AA8(0x113D);
    func_800A8AF0(0x113E);
    func_800A8B38(0x113F);
    cdrom_queue_read(0x1140, D_8011D538);
    cdrom_queue_read(0x1141, D_8011F538);
    cdrom_queue_read(0x1142, D_80121538);
    D_8011CF1C = D_800DD7B4;
    D_8011CF24 = D_800DDE54;
    D_8011CF28 = D_800DE39C;
    D_8011CF2C = D_800DF378;
    D_8011CF30 = D_800E0354;
    cdrom_queue_read(0x1143, D_800DCF18);
}

/** @brief Queue the effect's animation and texture resources. */
void func_800A9358(void)
{
    func_800A8AA8(0x1155);
    func_800A8AF0(0x1156);
    cdrom_queue_read(0x1158, D_8011D538);
    cdrom_queue_read(0x1159, D_8011F538);
    cdrom_queue_read(0x1157, D_800DCF18);
}

/** @brief World-map step handler: kick off the batch of resource reads for this map. */
void func_800A93D4(void)
{
    D_8011CF1C = &D_800ECF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    func_800A8AA8(0x115A);
    func_800A8AF0(0x115B);
    func_800A8B38(0x115C);
    cdrom_queue_read(0x115D, &D_8011D538);
    cdrom_queue_read(0x115E, &D_8011F538);
    cdrom_queue_read(0x115F, &D_80121538);
    cdrom_queue_read(0x1160, &D_800DCF18);
    cdrom_queue_read(0x1161, D_8011CF24);
    cdrom_queue_read(0x1162, D_8011CF1C);
}

/** @brief Queue the world-map effect resource set. */
void func_800A94C0(void)
{
    D_8011CF1C = D_800ECF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    func_800A8AA8(0x1163);
    func_800A8AF0(0x1164);
    func_800A8B38(0x1165);
    cdrom_queue_read(0x1168, D_8011D538);
    cdrom_queue_read(0x1169, D_8011F538);
    cdrom_queue_read(0x116A, D_80121538);
    cdrom_queue_read(0x116B, D_80123538);
    cdrom_queue_read(0x116C, D_80125538);
    cdrom_queue_read(0x116D, D_80127538);
    cdrom_queue_read(0x1166, D_800DCF18);
    cdrom_queue_read(0x1167, D_8011CF1C);
}

/** @brief World-map step: set up load buffers and queue CD reads for a scene. */
void func_800A95D4(void)
{
    D_8011CF1C = D_800ECF18;
    D_8011CF24 = (u8*)D_8011CF1C + 0x2000;
    func_800A8AA8(0x116E);
    func_800A8AF0(0x116F);
    func_800A8B38(0x1170);
    cdrom_queue_read(0x1171, D_800DCF18);
    cdrom_queue_read(0x1172, D_8011CF1C);
    cdrom_queue_read(0x1173, D_8011D538);
    cdrom_queue_read(0x1174, D_8011F538);
    cdrom_queue_read(0x1175, D_80121538);
}

/** @brief Queue the world-map effect resource set. */
void func_800A96AC(void)
{
    D_8011CF1C = D_800ECF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    D_8011CF28 = D_8011CF24 + 0x2000;
    func_800A8AA8(0x1176);
    func_800A8AF0(0x1177);
    func_800A8B38(0x1178);
    cdrom_queue_read(0x1179, D_8011D538);
    cdrom_queue_read(0x117A, D_8011F538);
    cdrom_queue_read(0x117B, D_800DCF18);
    cdrom_queue_read(0x117C, D_8011CF1C);
    cdrom_queue_read(0x117D, D_8011CF24);
    cdrom_queue_read(0x117E, D_8011CF28);
}

/** @brief World-map step: set up load buffers and queue CD reads for a scene. */
void func_800A97B0(void)
{
    D_8011CF1C = D_800ECF18;
    D_8011CF24 = (u8*)D_8011CF1C + 0x2000;
    func_800A8AA8(0x117F);
    func_800A8AF0(0x1180);
    func_800A8B38(0x1181);
    cdrom_queue_read(0x1182, D_800DCF18);
    cdrom_queue_read(0x1183, D_8011CF1C);
    cdrom_queue_read(0x1184, D_8011D538);
    cdrom_queue_read(0x1185, D_8011F538);
    cdrom_queue_read(0x1186, D_80121538);
}

/** @brief World-map load: register buffers and queue CD reads for a set. */
void func_800A9888(void)
{
    D_8011CF1C = &D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    func_800A8AA8(0x1187);
    func_800A8AF0(0x1188);
    func_800A8B38(0x1189);
    cdrom_queue_read(0x118A, &D_8011D538);
    cdrom_queue_read(0x118B, &D_8011F538);
    cdrom_queue_read(0x118C, &D_80121538);
    cdrom_queue_read(0x118D, &D_800DCF18);
    cdrom_queue_read(0x118E, D_8011CF1C);
}

/** @brief World-map load: register buffers and queue CD reads for a set. */
void func_800A9960(void)
{
    D_8011CF1C = &D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    func_800A8AA8(0x118F);
    func_800A8AF0(0x1190);
    func_800A8B38(0x1191);
    cdrom_queue_read(0x1192, &D_8011D538);
    cdrom_queue_read(0x1193, &D_8011F538);
    cdrom_queue_read(0x1194, &D_80121538);
    cdrom_queue_read(0x1195, &D_800DCF18);
    cdrom_queue_read(0x1196, D_8011CF1C);
}

/** @brief Set effect resource buffers and queue animation and texture reads. */
void func_800A9A38(void)
{
    func_800A8AA8(0x11A0);
    func_800A8AF0(0x11A1);
    func_800A8B38(0x11A2);
    cdrom_queue_read(0x11A3, D_8011D538);
    cdrom_queue_read(0x11A4, D_8011F538);
    cdrom_queue_read(0x11A5, D_80121538);
    D_8011CF1C = D_800DDEF4;
    D_8011CF24 = D_800DEED0;
    D_8011CF28 = D_800E2B1C;
    D_8011CF2C = D_800E2DD8;
    D_8011CF30 = D_800E33AC;
    D_8011CF34 = D_800E3980;
    cdrom_queue_read(0x11A6, D_800DCF18);
}

/** @brief Set effect resource buffers and queue animation and texture reads. */
void func_800A9B34(void)
{
    D_8011CF1C = D_800E0F18;
    D_8011CF24 = D_8011CF1C + 0x4000;
    D_8011CF28 = D_8011CF24 + 0xC800;
    func_800A8AA8(0x11A7);
    func_800A8AF0(0x11A8);
    func_800A8B38(0x11A9);
    cdrom_queue_read(0x11AA, D_8011D538);
    cdrom_queue_read(0x11AB, D_8011F538);
    cdrom_queue_read(0x11AC, D_80121538);
    cdrom_queue_read(0x11AD, D_800DCF18);
    cdrom_queue_read(0x11AE, D_8011CF1C);
    cdrom_queue_read(0x11AF, D_8011CF24);
}

/** @brief World-map step handler: kick off the batch of resource reads for this map. */
void func_800A9C38(void)
{
    D_8011CF1C = &D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    func_800A8AA8(0x11BA);
    func_800A8AF0(0x11BB);
    func_800A8B38(0x11BC);
    cdrom_queue_read(0x11BD, &D_8011D538);
    cdrom_queue_read(0x11BE, &D_8011F538);
    cdrom_queue_read(0x11BF, &D_80121538);
    cdrom_queue_read(0x11C0, &D_800DCF18);
    cdrom_queue_read(0x11C1, D_8011CF1C);
    cdrom_queue_read(0x11C2, D_8011CF24);
}

/** @brief Set effect resource buffers and queue animation and texture reads. */
void func_800A9D24(void)
{
    D_8011CF1C = D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    func_800A8AA8(0x11C3);
    func_800A8AF0(0x11C4);
    func_800A8B38(0x11C5);
    cdrom_queue_read(0x11C7, D_8011D538);
    cdrom_queue_read(0x11C6, D_8011F538);
    cdrom_queue_read(0x11C8, D_80121538);
    cdrom_queue_read(0x11C9, D_80123538);
    cdrom_queue_read(0x11CA, D_800DCF18);
    cdrom_queue_read(0x11CB, D_8011CF1C);
}

/** @brief Set effect resource buffers and queue animation and texture reads. */
void func_800A9E10(void)
{
    D_8011CF1C = D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    func_800A8AA8(0x11CC);
    func_800A8AF0(0x11CD);
    func_800A8B38(0x11CE);
    cdrom_queue_read(0x11CF, D_8011D538);
    cdrom_queue_read(0x11D0, D_8011F538);
    cdrom_queue_read(0x11D1, D_80121538);
    cdrom_queue_read(0x11D2, D_800DCF18);
    cdrom_queue_read(0x11D3, D_8011CF1C);
    cdrom_queue_read(0x11D4, D_8011CF24);
}

/** @brief Set effect resource buffers and queue animation and texture reads. */
void func_800A9EFC(void)
{
    D_8011CF1C = D_800E0F18;
    D_8011CF24 = D_8011CF1C + 0x4000;
    D_8011CF28 = D_8011CF24 + 0x4000;
    D_8011CF2C = D_8011CF28 + 0x4000;
    func_800A8AA8(0x11D5);
    func_800A8AF0(0x11D6);
    func_800A8B38(0x11D7);
    cdrom_queue_read(0x11D8, D_8011D538);
    cdrom_queue_read(0x11D9, D_8011F538);
    cdrom_queue_read(0x11DA, D_80121538);
    cdrom_queue_read(0x11DB, D_800DCF18);
    cdrom_queue_read(0x11DC, D_8011CF1C);
    cdrom_queue_read(0x11DD, D_8011CF24);
    cdrom_queue_read(0x11DE, D_8011CF28);
    cdrom_queue_read(0x11DF, D_8011CF2C);
}

/** @brief Divide the effect buffer and queue its animation and texture reads. */
void func_800AA040(void)
{
    D_8011CF1C = D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    D_8011CF28 = D_8011CF24 + 0x2000;
    D_8011CF2C = D_8011CF28 + 0x2000;
    func_800A8AA8(0x11E0);
    func_800A8AF0(0x11E1);
    func_800A8B38(0x11E2);
    cdrom_queue_read(0x11E3, D_800DCF18);
    cdrom_queue_read(0x11E4, D_8011CF1C);
    cdrom_queue_read(0x11E5, D_8011CF24);
    cdrom_queue_read(0x11E6, D_8011CF28);
    cdrom_queue_read(0x11E7, D_8011CF2C);
    cdrom_queue_read(0x11E8, D_8011D538);
    cdrom_queue_read(0x11E9, D_8011F538);
    cdrom_queue_read(0x11EA, D_80121538);
    cdrom_queue_read(0x11EB, D_80123538);
    cdrom_queue_read(0x11EC, D_80125538);
}

/** @brief Queue the world-map effect resource set. */
void func_800AA1AC(void)
{
    D_8011CF1C = D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    D_8011CF28 = D_8011CF24 + 0x2000;
    func_800A8AA8(0x11ED);
    func_800A8AF0(0x11EE);
    func_800A8B38(0x11EF);
    cdrom_queue_read(0x11F0, D_8011D538);
    cdrom_queue_read(0x11F1, D_8011F538);
    cdrom_queue_read(0x11F2, D_80121538);
    cdrom_queue_read(0x11F3, D_80123538);
    cdrom_queue_read(0x11F4, D_80125538);
    cdrom_queue_read(0x11F5, D_800DCF18);
    cdrom_queue_read(0x11F6, D_8011CF1C);
    cdrom_queue_read(0x11F7, D_8011CF24);
    cdrom_queue_read(0x11F8, D_8011CF28);
}

/** @brief Queue the world-map effect resource set. */
void func_800AA2EC(void)
{
    D_8011CF1C = D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    D_8011CF28 = D_8011CF24 + 0x4000;
    D_8011CF2C = D_8011CF28 + 0x4000;
    func_800A8AA8(0x11F9);
    func_800A8AF0(0x11FA);
    func_800A8B38(0x11FB);
    cdrom_queue_read(0x11FC, D_8011D538);
    cdrom_queue_read(0x11FD, D_8011F538);
    cdrom_queue_read(0x11FE, D_80121538);
    cdrom_queue_read(0x11FF, D_80123538);
    cdrom_queue_read(0x1200, D_800DCF18);
    cdrom_queue_read(0x1201, D_8011CF1C);
    cdrom_queue_read(0x1202, D_8011CF24);
    cdrom_queue_read(0x1203, D_8011CF28);
    cdrom_queue_read(0x1204, D_8011CF2C);
}

/** @brief Set effect resource buffers and queue animation and texture reads. */
void func_800AA444(void)
{
    D_8011CF1C = D_800DDEF4;
    D_8011CF24 = D_800DEED0;
    D_8011CF28 = D_800DFEAC;
    D_8011CF2C = D_800E0720;
    D_8011CF30 = D_800E0F94;
    D_8011CF34 = D_800E91C0;
    D_8011CF38 = D_800F420C;
    func_800A8AA8(0x120E);
    func_800A8AF0(0x120F);
    func_800A8B38(0x1210);
    cdrom_queue_read(0x1211, D_8011D538);
    cdrom_queue_read(0x1212, D_8011F538);
    cdrom_queue_read(0x1213, D_80121538);
    cdrom_queue_read(0x1214, D_80123538);
    cdrom_queue_read(0x1215, D_800DCF18);
}

/** @brief Queue the world-map effect resource set. */
void func_800AA564(void)
{
    D_8011CF1C = D_800E7F64;
    D_8011CF24 = D_800EA560;
    D_8011CF28 = D_800EAA54;
    D_8011CF2C = D_800EBED8;
    D_8011CF30 = D_800ECEB4;
    D_8011CF34 = D_800ED310;
    D_8011CF38 = D_800EE2EC;
    D_8011CF3C = D_800F2238;
    D_8011CF40 = D_800F3214;
    D_8011CF84 = D_800FAE60;
    func_800A8AA8(0x122C);
    func_800A8AF0(0x122D);
    func_800A8B38(0x122E);
    cdrom_queue_read(0x122F, D_8011D538);
    cdrom_queue_read(0x1230, D_8011F538);
    cdrom_queue_read(0x1231, D_80121538);
    cdrom_queue_read(0x1232, D_800DCF18);
}

/** @brief Queue the world-map effect resource set. */
void func_800AA6A0(void)
{
    D_8011CF1C = D_800DDF18;
    D_8011CF24 = D_8011CF1C + 0x1000;
    D_8011CF28 = D_8011CF24 + 0x1000;
    D_8011CF2C = D_8011CF28 + 0x10800;
    func_800A8AA8(0x1233);
    func_800A8AF0(0x1234);
    func_800A8B38(0x1235);
    cdrom_queue_read(0x1236, D_8011D538);
    cdrom_queue_read(0x1237, D_8011F538);
    cdrom_queue_read(0x1238, D_80121538);
    cdrom_queue_read(0x1239, D_800DCF18);
    cdrom_queue_read(0x123A, D_8011CF1C);
    cdrom_queue_read(0x123B, D_8011CF24);
    cdrom_queue_read(0x123C, D_8011CF28);
    cdrom_queue_read(0x123D, D_8011CF2C);
}

/** @brief Queue the resource set used by this world-map sequence. */
void func_800AA7E8(void)
{
    func_800A8AA8(0x123E);
    func_800A8AF0(0x123F);
    func_800A8B38(0x1240);
    cdrom_queue_read(0x1241, &D_80125538);
    cdrom_queue_read(0x1242, &D_80127538);
    cdrom_queue_read(0x1243, &D_80123538);
    cdrom_queue_read(0x1244, &D_80121538);
    cdrom_queue_read(0x1245, &D_800DCF18);
}

/** @brief Queue the resource set used by this world-map sequence. */
void func_800AA898(void)
{
    func_800A8AA8(0x1246);
    func_800A8AF0(0x1247);
    cdrom_queue_read(0x1248, &D_8011D538);
    cdrom_queue_read(0x1249, &D_8011F538);
    cdrom_queue_read(0x124A, &D_800DCF18);
}

/** @brief Queue the world-map effect resource set. */
void func_800AA914(void)
{
    D_8011CF1C = D_800DDF18;
    D_8011CF24 = D_8011CF1C + 0x1000;
    D_8011CF28 = D_8011CF24 + 0x6000;
    func_800A8AA8(0x124B);
    func_800A8AF0(0x124C);
    func_800A8B38(0x124D);
    cdrom_queue_read(0x124E, D_8011D538);
    cdrom_queue_read(0x124F, D_8011F538);
    cdrom_queue_read(0x1250, D_80121538);
    cdrom_queue_read(0x1251, D_800DCF18);
    cdrom_queue_read(0x1252, D_8011CF1C);
    cdrom_queue_read(0x1253, D_8011CF24);
    cdrom_queue_read(0x1254, D_8011CF28);
}
