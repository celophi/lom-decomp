#include "common.h"

typedef struct Unk Unk;
struct Unk {
    s32 unk0;  s32 unk1;  s32 unk2;  s32 unk4;  s32 unk6;
    s32 unk8;  s32 unk9;  s32 unkA;  s32 unkB;  s32 unkC;
    s32 unkD;  s32 unkE;  s32 unkF;  s32 unk10; s32 unk12;
    s32 unk14; s32 unk16; s32 unk18; s32 unk1A; s32 unk1C;
    s32 unk1E; s32 unk1F; s32 unk20; s32 unk21; s32 unk22;
    s32 unk24; s32 unk26; s32 unk28; s32 unk2C; s32 unk30;
    s32 unk34; s32 unk36; s32 unk38; s32 unk3A; s32 unk3C;
    s32 unk3E; s32 unk40; s32 unk42; s32 unk44; s32 unk46;
    s32 unk48; s32 unk4A;
    s32 unk1F800000;
};

typedef struct SrcObj SrcObj;
struct SrcObj {
    /* 0x00 */ SrcObj* unk0;
    /* 0x04 */ u8      unk4;
    /* 0x05 */ u8      pad05[0x13];
    /* 0x18 */ u16     unk18;
};

typedef struct  {
    /* 0x00 */ void*   unk0;
    /* 0x04 */ u8      pad04[0x04];
    /* 0x08 */ SrcObj* unk8;
    /* 0x0C */ void*   unkC;
    /* 0x10 */ u8      pad10[0x04];
    /* 0x14 */ s32     unk14;
    /* 0x18 */ s32     unk18;
    /* 0x1C */ s32     unk1C;
    /* 0x20 */ s32     unk20;
    /* 0x24 */ u8      pad24[0x02];
    /* 0x26 */ s16     unk26;
} ObjArg;

typedef struct Node30 Node30;
struct Node30 {
    /* 0x00 */ Node30* unk0;
    /* 0x04 */ void*   unk4;
    /* 0x08 */ u8      pad08[0x04];
    /* 0x0C */ u8      unkC;   /* 0x0C..0x0F also written as one s32 */
    /* 0x0D */ u8      unkD;
    /* 0x0E */ u8      unkE;
    /* 0x0F */ u8      unkF;
    /* 0x10 */ s16     unk10;
    /* 0x12 */ s16     unk12;
    /* 0x14 */ s16     unk14;
    /* 0x16 */ s16     unk16;
    /* 0x18 */ s16     unk18;
    /* 0x1A */ s16     unk1A;
    /* 0x1C */ s32     unk1C;
    /* 0x20 */ s32     unk20;
    /* 0x24 */ s32     unk24;
    /* 0x28 */ s32     unk28;
    /* 0x2C */ s32     unk2C;
};

typedef struct Node44 Node44;
struct Node44 {
    /* 0x00 */ Node44* unk0;
    /* 0x04 */ void*   unk4;
    /* 0x08 */ u8      pad08[0x08];
    /* 0x10 */ s32     unk10;
    /* 0x14 */ s32     unk14;
    /* 0x18 */ s8      unk18;
    /* 0x19 */ u8      pad19[0x03];
    /* 0x1C */ s16     unk1C;
    /* 0x1E */ s16     unk1E;
    /* 0x20 */ s16     unk20;
    /* 0x22 */ s16     unk22;
    /* 0x24 */ s32     unk24;
    /* 0x28 */ s32     unk28;
    /* 0x2C */ s32     unk2C;
    /* 0x30 */ s32     unk30;
    /* 0x34 */ s32     unk34;
    /* 0x38 */ s32     unk38;
    /* 0x3C */ s32     unk3C;
    /* 0x40 */ s32     unk40;
};

typedef struct Node38 Node38;
struct Node38 {
    /* 0x00 */ Node38* unk0;
    /* 0x04 */ void*   unk4;
    /* 0x08 */ s16     unk8;
    /* 0x0A */ s16     unkA;
    /* 0x0C */ s16     unkC;
    /* 0x0E */ s16     unkE;
    /* 0x10 */ s16     unk10;
    /* 0x12 */ s16     unk12;
    /* 0x14 */ s16     unk14;
    /* 0x16 */ s16     unk16;
    /* 0x18 */ s32     unk18;
    /* 0x1C */ s32     unk1C;
    /* 0x20 */ s32     unk20;
    /* 0x24 */ s32     unk24;
    /* 0x28 */ s32     unk28;
    /* 0x2C */ s32     unk2C;
    /* 0x30 */ s32     unk30;
    /* 0x34 */ s32     unk34;
};

typedef struct SrcObj2 SrcObj2;
struct SrcObj2 {
    /* 0x00 */ SrcObj2* unk0;
    /* 0x04 */ u16 unk4;
    /* 0x06 */ u16 unk6;
    /* 0x08 */ u16 unk8;
    /* 0x0A */ u16 unkA;
    /* 0x0C */ u16 unkC;
    /* 0x0E */ u16 unkE;
};

typedef struct SrcObj3 SrcObj3;
struct SrcObj3 {
    /* 0x00 */ SrcObj3* unk0;
    /* 0x04 */ u8  pad04[0x08];
    /* 0x0C */ u8  unkC;
    /* 0x0D */ u8  pad0D[0x03];
    /* 0x10 */ u16 unk10;
    /* 0x12 */ u16 unk12;
    /* 0x14 */ u16 unk14;
    /* 0x16 */ s16 unk16;
    /* 0x18 */ s16 unk18;
    /* 0x1A */ s16 unk1A;
    /* 0x1C */ u8  unk1C;   /* 0x1C..0x1F also read as one s32 */
    /* 0x1D */ u8  unk1D;
    /* 0x1E */ u8  unk1E;
    /* 0x1F */ u8  unk1F;
};

extern Unk* D_80180014;
extern s16* D_8018001C;
extern s32 D_80180010;
extern u16 D_80180008;

/**
 * decomp.me (54.84%) https://decomp.me/scratch/WaOe4
 */
void func_80052628(ObjArg* arg0, u16 arg1)
{
    s32 sp10;
    s32 sp14;
    s32 sp18;
    s8 sp20;
    Unk* sp24;
    u16 sp28;
    Unk** sp30;
    Unk* sp34;
    Node30* sp38;
    s32 sp3C;
    s32 sp50;
    Unk* sp54;
    s32 sp58;
    Unk* sp5C;
    s32 sp60;
    s32 sp64;
    Unk* sp68;
    Unk** sp6C;
    s32 sp74;
    s32 sp7C;
    s32 sp80;
    Unk* temp_a1_5;
    Unk* temp_a1_6;
    Node44* temp_s1;
    Unk* temp_s1_2;
    Unk* temp_s2;
    Node38* temp_t0;
    Unk* var_s3;
    Unk* var_s7_2;
    Unk* var_s7_3;
    Node30* var_t0_2;
    Unk* var_t0_3;
    Unk* var_t0_4;
    Node44* var_t1;
    Node38* var_t1_2;
    Unk* var_t1_5;
    Unk* var_t5;
    s16 temp_a1;
    s16 temp_a1_2;
    s16 temp_a1_3;
    s16 temp_a1_4;
    s16 temp_a2;
    s16 temp_a2_2;
    s16 temp_v0;
    s16 temp_v0_2;
    s16 temp_v0_3;
    s16 temp_v0_4;
    s16 var_a1_2;
    s16 var_a1_3;
    s16 var_t1_3;
    s16 var_v0_2;
    s16 var_v0_3;
    s16 var_v0_4;
    s16 var_v0_5;
    s16 var_v1;
    s16 var_v1_2;
    s16* var_a1;
    s16* var_a2;
    s32 temp_a0;
    s32 temp_a2_3;
    s32 temp_a2_4;
    s32 temp_t1;
    s32 temp_t1_2;
    s32 temp_t3;
    s32 temp_t3_2;
    s32 temp_t4;
    s32 temp_t4_2;
    s32 temp_t7;
    s32 temp_t7_2;
    s32 temp_v0_10;
    s32 temp_v0_11;
    s32 temp_v0_12;
    s32 temp_v0_13;
    s32 temp_v0_5;
    s32 temp_v0_6;
    s32 temp_v1_13;
    s32 temp_v1_14;
    s32 temp_v1_7;
    s32 temp_v1_8;
    s32 var_fp_2;
    s32 var_s0;
    s32 var_s0_2;
    s32 var_s0_3;
    s32 var_s0_4;
    s32 var_s0_5;
    s32 var_s0_6;
    s32 var_s0_7;
    s32 var_s0_8;
    s32 var_s1;
    s32 var_s1_3;
    s32 var_s1_4;
    s32 var_s5;
    s32 var_s5_2;
    s32 var_s6;
    s32 var_s6_2;
    s32 var_s6_3;
    s32 var_t1_4;
    s32 var_t3;
    s32 var_t4;
    s32 var_t8;
    s32 var_v0_7;
    s32 var_v1_3;
    s32 var_v1_4;
    s32 var_v1_5;
    s32 var_v1_6;
    s32 var_v1_7;
    s32 var_v1_8;
    s32* var_s3_2;
    s32* var_s3_3;
    u16 temp_v1;
    u16 temp_v1_2;
    u16 temp_v1_3;
    u16 temp_v1_4;
    u16 temp_v1_5;
    u16 temp_v1_6;
    u16 var_a0;
    u16 var_a0_2;
    u16 var_a0_3;
    u16 var_a0_4;
    u16 var_a0_5;
    u16 var_a0_6;
    u16 var_t2;
    u16 var_t2_2;
    u16* temp_a0_2;
    u16* var_t0;
    u32 temp_a0_3;
    u32 temp_a0_4;
    u32 var_v0_6;
    u32 var_v0_8;
    u8 temp_v0_7;
    u8 temp_v1_12;
    u8 temp_v1_9;
    u8 var_v0;
    Unk* var_a0_7;
    u8* var_fp;
    Unk* temp_a3;
    Unk* temp_s4;
    Unk* temp_s4_2;
    Unk* temp_t2;
    Unk* temp_v0_8;
    Unk* temp_v0_9;
    Unk* temp_v1_10;
    Unk* temp_v1_11;
    SrcObj* var_a3;
    SrcObj2* var_a3_2;
    Unk* var_s1_2;
    Unk* var_s1_5;
    Unk* var_s2;
    SrcObj3* var_t2_3;
    Unk* var_t2_4;
    Unk** var_s7;
    SrcObj3** var_t6;

    var_t3 = 0;
    var_t4 = 0;
    var_t8 = 0;
    sp30 = (Unk**)0x801ED000;
    sp80 = 0;
    sp3C = 0;
    sp24 = NULL;
    sp34 = D_80180014;
    D_80180014->unk0 = arg0;
    D_80180014->unkC = 0;
    var_a3 = arg0->unk8;
    sp24 = D_80180014 + 0x74;
    sp28 = arg1;
    var_t1 = (Node44*)((u8*)D_80180014 + 8);
    if (var_a3 != NULL)
    {
        do
        {
            temp_s1 = sp24;
            sp24 = (Unk*)((u8*)temp_s1 + 0x44);
            var_t1->unk0 = temp_s1;
            var_t1 = temp_s1;
            var_t1->unk4 = var_a3;
            var_t1->unk10 = 0;
            var_t1->unk14 = 0;
            var_t1->unk1C = 0x7FFF;
            var_t1->unk1E = 0;
            var_t1->unk20 = 0;
            var_t1->unk22 = 0x7FFF;
            var_t1->unk24 = 0;
            var_t1->unk28 = 0;
            var_t1->unk2C = 0;
            var_t1->unk30 = 0;
            var_t1->unk34 = 0;
            var_t1->unk38 = 0;
            var_t1->unk3C = 0;
            var_t1->unk40 = 0;
            var_t1->unk18 = (s8)((u8)var_a3->unk4 >> 7);
            var_s0 = var_a3->unk18 & 0x7FFF;
            var_t0 = (u16*)((u8*)var_a3 + 0x18);
            if (var_s0 != 0)
            {
                do
                {
                    var_s0_2 = var_s0 - 1;
                    var_a1 = D_8018001C + (var_t0[1] * 2);
                    if (var_s0_2 != -1)
                    {
                        var_a2 = var_a1 + 1;
                        do
                        {
                            var_a0 = (u16)*var_a1;
                            if (temp_s1->unk1C < *var_a1)
                            {
                                var_a0 = (u16)temp_s1->unk1C;
                            }
                            temp_s1->unk1C = (s16)var_a0;
                            var_a0_2 = (u16)*var_a1;
                            if (*var_a1 < temp_s1->unk1E)
                            {
                                var_a0_2 = (u16)temp_s1->unk1E;
                            }
                            temp_s1->unk1E = (s16)var_a0_2;
                            var_a0_3 = (u16)*var_a2;
                            if (*var_a2 < temp_s1->unk20)
                            {
                                var_a0_3 = (u16)temp_s1->unk20;
                            }
                            temp_s1->unk20 = (s16)var_a0_3;
                            var_a0_4 = (u16)*var_a2;
                            if (temp_s1->unk22 < *var_a2)
                            {
                                var_a0_4 = (u16)temp_s1->unk22;
                            }
                            temp_s1->unk22 = (s16)var_a0_4;
                            var_a2 += 2;
                            var_s0_2 -= 1;
                            var_a1 += 2;
                        } while (var_s0_2 != -1);
                    }
                    var_t0 += 2;
                    var_s0 = *var_t0 & 0x7FFF;
                } while (var_s0 != 0);
            }
            var_a3 = var_a3->unk0;
        } while (var_a3 != NULL);
    }
    var_t1->unk0 = NULL;
    var_a3_2 = arg0->unkC;
    var_t1_2 = (Node38*)((u8*)sp34 + 0x10);
    if (var_a3_2 != NULL)
    {
        do
        {
            temp_t0 = sp24;
            sp24 = (Unk*)((u8*)temp_t0 + 0x38);
            var_t1_2->unk0 = temp_t0;
            var_t1_2 = temp_t0;
            var_t1_2->unk4 = var_a3_2;
            var_t1_2->unk8 = (s16)(var_a3_2->unk4 + var_a3_2->unkC);
            var_t1_2->unkA = (s16)(var_a3_2->unk6 + var_a3_2->unkE);
            var_t1_2->unkC = (s16)(var_a3_2->unk8 + var_a3_2->unkC);
            var_t1_2->unkE = (s16)(var_a3_2->unkA + var_a3_2->unkE);
            var_a0_5 = var_a3_2->unk4;
            temp_a2 = (s16)var_a3_2->unk8;
            temp_v1 = var_a3_2->unk8;
            var_t2 = var_a0_5;
            if ((s16)var_a0_5 < temp_a2)
            {
                var_t2 = temp_v1;
            }
            if (temp_a2 < (s16)var_a0_5)
            {
                var_a0_5 = temp_v1;
            }
            temp_a1 = var_t1_2->unk8;
            temp_v1_2 = (u16)var_t1_2->unk8;
            if ((s16)var_t2 < temp_a1)
            {
                var_t2 = temp_v1_2;
            }
            if (temp_a1 < (s16)var_a0_5)
            {
                var_a0_5 = temp_v1_2;
            }
            temp_a1_2 = var_t1_2->unkC;
            temp_v1_3 = (u16)var_t1_2->unkC;
            if ((s16)var_t2 < temp_a1_2)
            {
                var_t2 = temp_v1_3;
            }
            if (temp_a1_2 < (s16)var_a0_5)
            {
                var_a0_5 = temp_v1_3;
            }
            var_t1_2->unk10 = var_t2;
            var_t1_2->unk12 = var_a0_5;
            var_a0_6 = var_a3_2->unk6;
            temp_a2_2 = (s16)var_a3_2->unkA;
            temp_v1_4 = var_a3_2->unkA;
            var_t2_2 = var_a0_6;
            if ((s16)var_a0_6 < temp_a2_2)
            {
                var_t2_2 = temp_v1_4;
            }
            if (temp_a2_2 < (s16)var_a0_6)
            {
                var_a0_6 = temp_v1_4;
            }
            temp_a1_3 = var_t1_2->unkA;
            temp_v1_5 = (u16)var_t1_2->unkA;
            if ((s16)var_t2_2 < temp_a1_3)
            {
                var_t2_2 = temp_v1_5;
            }
            if (temp_a1_3 < (s16)var_a0_6)
            {
                var_a0_6 = temp_v1_5;
            }
            temp_a1_4 = temp_t0->unkE;
            temp_v1_6 = (u16)temp_t0->unkE;
            if ((s16)var_t2_2 < temp_a1_4)
            {
                var_t2_2 = temp_v1_6;
            }
            if (temp_a1_4 < (s16)var_a0_6)
            {
                var_a0_6 = temp_v1_6;
            }
            temp_t0->unk14 = var_t2_2;
            temp_t0->unk16 = var_a0_6;
            temp_t0->unk18 = (s32)(temp_t0->unk8 - (s16)var_a3_2->unk4);
            temp_a2_3 = temp_t0->unk18;
            temp_v1_7 = temp_t0->unkA - (s16)var_a3_2->unk6;
            temp_t0->unk1C = temp_v1_7;
            if (temp_a2_3 != 0)
            {
                var_a1_2 = (s16)var_a3_2->unk6 - ((s32)(temp_v1_7 * (s16)var_a3_2->unk4) / temp_a2_3);
                var_v1 = (s16)var_a3_2->unkA - ((s32)(temp_v1_7 * (s16)var_a3_2->unk8) / temp_a2_3);
            }
            else
            {
                var_a1_2 = (s16)var_a3_2->unk4;
                var_v1 = (s16)var_a3_2->unk8;
            }
            if (var_v1 < var_a1_2)
            {
                temp_t0->unk2C = (s32)var_a1_2;
                temp_t0->unk28 = (s32)var_v1;
            }
            else
            {
                temp_t0->unk28 = (s32)var_a1_2;
                temp_t0->unk2C = (s32)var_v1;
            }
            temp_t0->unk20 = (s32)((s16)var_a3_2->unk8 - (s16)var_a3_2->unk4);
            temp_a2_4 = temp_t0->unk20;
            temp_v1_8 = (s16)var_a3_2->unkA - (s16)var_a3_2->unk6;
            temp_t0->unk24 = temp_v1_8;
            if (temp_a2_4 != 0)
            {
                var_a1_3 = (s16)var_a3_2->unk6 - ((s32)(temp_v1_8 * (s16)var_a3_2->unk4) / temp_a2_4);
                var_v1_2 = temp_t0->unkA - ((s32)(temp_v1_8 * temp_t0->unk8) / temp_a2_4);
            }
            else
            {
                var_a1_3 = (s16)var_a3_2->unk4;
                var_v1_2 = temp_t0->unk8;
            }
            if (var_v1_2 < var_a1_3)
            {
                temp_t0->unk34 = (s32)var_a1_3;
                temp_t0->unk30 = (s32)var_v1_2;
            }
            else
            {
                temp_t0->unk30 = (s32)var_a1_3;
                temp_t0->unk34 = (s32)var_v1_2;
            }
            var_a3_2 = var_a3_2->unk0;
        } while (var_a3_2 != NULL);
    }
    var_t1_2->unk0 = NULL;
    var_t6 = arg0->unk0;
    sp38 = (Node30*)((u8*)sp34 + 4);
    if (*var_t6 != NULL)
    {
        sp7C = 0x51EB851F;
        var_t1_3 = 1;
        do
        {
            var_t0_2 = sp24;
            var_t2_3 = *var_t6;
            sp24 = (Unk*)((u8*)var_t0_2 + 0x30);
            sp38->unk0 = var_t0_2;
            var_t0_2->unk4 = var_t2_3;
            var_t0_2->unk10 = (s16)((s32)MULT_HI((var_t2_3->unk10 << 8), sp7C) >> 5);
            var_t0_2->unk12 = (s16)((s32)MULT_HI((var_t2_3->unk12 << 8), sp7C) >> 5);
            var_t0_2->unk1A = 0x100;
            var_t0_2->unk18 = 0x100;
            var_t0_2->unk16 = 0x100;
            var_t0_2->unk14 = (s16)((s32)MULT_HI((var_t2_3->unk14 << 8), sp7C) >> 5);
            *(s32*)&var_t0_2->unkC = (s32)((*(s32*)&var_t0_2->unkC & ~1) | (var_t2_3->unkC & 1));
            var_t0_2->unkD = 0;
            var_t0_2->unk1C = (s32)(var_t2_3->unk16 << 8);
            var_t0_2->unk20 = (s32)(var_t2_3->unk18 << 8);
            sp38 = var_t0_2;
            var_t0_2->unk24 = (s32)(var_t2_3->unk1A << 8);
            if ((*(s32*)&var_t2_3->unk1C & 0xFFFF0000) == 0x100000)
            {
                var_t0_2->unkE = 0U;
            }
            else
            {
                var_t0_2->unkE = (u8)var_t2_3->unk1E;
            }
            var_t0_2->unk28 = 0;
            var_t0_2->unk2C = 0;
            var_t0_2->unkF = (u8)var_t2_3->unk1F;
            var_s7 = var_t2_3->unk0;
            var_t5 = (Unk*)((u8*)var_t0_2 + 8);
            if (*var_s7 != NULL)
            {
                do
                {
                    temp_s2 = sp24;
                    temp_s4 = *var_s7;
                    sp24 = temp_s2 + 0x4C;
                    var_t5->unk0 = temp_s2;
                    temp_s2->unk4 = temp_s4;
                    temp_s2->unk20 = (s8)(temp_s4->unk8 & 1);
                    temp_a0 = (s32)temp_s4->unk8;
                    var_t5 = temp_s2;
                    if ((temp_a0 & 0xF00) == 0x100)
                    {
                        var_v0 = (temp_a0 & 0xE) + 1;
                    }
                    else
                    {
                        var_v0 = temp_a0 & 0xE;
                    }
                    temp_s2->unk21 = var_v0;
                    temp_s2->unk22 = 0;
                    temp_s2->unk28 = (s32)(temp_s4->unkC << 8);
                    temp_s2->unk2C = (s32)(temp_s4->unkE << 8);
                    temp_s2->unk34 = 0;
                    temp_s2->unk30 = (s32)(temp_s4->unk10 << 8);
                    temp_s2->unk38 = var_t1_3;
                    temp_s2->unk3A = 0;
                    temp_s2->unk3C = 0;
                    temp_s2->unk3E = 0;
                    temp_s2->unk40 = 0x1000;
                    temp_s2->unk42 = 0x1000;
                    temp_s2->unk36 = (u16)temp_s4->unk12;
                    if ((s32)temp_s4->unk8 & 0x40)
                    {
                        temp_v0 = var_t2_3->unk1A + temp_s4->unk10 + temp_s4->unk18;
                        if (temp_v0 > 0)
                        {
                            var_v0_2 = temp_v0;
                            if (temp_v0 >= 0x800)
                            {
                                var_v0_2 = 0x7FF;
                            }
                        }
                        else
                        {
                            var_v0_2 = 0;
                        }
                        temp_s2->unk44 = var_v0_2;
                        temp_v0_2 = var_t2_3->unk1A + temp_s4->unk10 + temp_s4->unk1A;
                        if (temp_v0_2 > 0)
                        {
                            var_v0_3 = temp_v0_2;
                            if (temp_v0_2 >= 0x800)
                            {
                                var_v0_3 = 0x7FF;
                            }
                        }
                        else
                        {
                            var_v0_3 = 0;
                        }
                        temp_s2->unk46 = var_v0_3;
                        temp_v0_3 = var_t2_3->unk1A + temp_s4->unk10 + temp_s4->unk1C;
                        if (temp_v0_3 > 0)
                        {
                            var_v0_4 = temp_v0_3;
                            if (temp_v0_3 >= 0x800)
                            {
                                var_v0_4 = 0x7FF;
                            }
                        }
                        else
                        {
                            var_v0_4 = 0;
                        }
                        temp_s2->unk48 = var_v0_4;
                        temp_v0_4 = var_t2_3->unk1A + temp_s4->unk10 + temp_s4->unk1E;
                        if (temp_v0_4 > 0)
                        {
                            var_v0_5 = temp_v0_4;
                            if (temp_v0_4 >= 0x800)
                            {
                                var_v0_5 = 0x7FF;
                            }
                        }
                        else
                        {
                            var_v0_5 = 0;
                        }
                    }
                    else
                    {
                        temp_s2->unk44 = (s16)(u16)temp_s4->unk18;
                        temp_s2->unk46 = (s16)(u16)temp_s4->unk1A;
                        temp_s2->unk48 = (s16)(u16)temp_s4->unk1C;
                        var_v0_5 = (s16)(u16)temp_s4->unk1E;
                    }
                    temp_s2->unk4A = var_v0_5;
                    var_s6 = 0;
                    var_fp = temp_s4->unk0;
                    var_s5 = ((u8)temp_s2->unk21 > 0U) * 2;
                    if (var_fp != NULL)
                    {
                        sp54 = var_t0_2;
                        sp58 = (s32)var_t1_3;
                        sp5C = var_t2_3;
                        sp60 = var_t3;
                        sp64 = var_t4;
                        sp68 = var_t5;
                        sp6C = var_t6;
                        sp74 = var_t8;
                        temp_v0_5 = func_80056824(sp34, var_t0_2, temp_s2, var_fp);
                        temp_s2->unk8 = temp_v0_5;
                        if (temp_v0_5 == 0)
                        {
                            var_s0_3 = 1;
                            if (((s32)temp_s4->unk8 & 0xF00) != 0x100)
                            {
                                var_s0_3 = temp_s4->unkA * temp_s4->unkB;
                            }
                            var_v1_3 = var_s0_3 + 0x1F;
                            var_s3 = sp24;
                            temp_s2->unkC = var_s3;
                            if (var_v1_3 < 0)
                            {
                                var_v1_3 = var_s0_3 + 0x3E;
                            }
                            var_s0_4 = var_s0_3 - 1;
                            temp_v0_6 = var_v1_3 >> 5;
                            sp3C = 0;
                            temp_s2->unk14 = (s32)(temp_v0_6 * 4);
                            sp24 = &var_s3[temp_v0_6];
                            var_s1 = 1;
                            if (var_s0_4 != -1)
                            {
                                var_a0_7 = var_fp + 1;
                                do
                                {
                                    if (*var_fp & 0x80)
                                    {
                                        sp3C |= var_s1;
                                        if (var_s5 == 0)
                                        {
                                            temp_v0_7 = var_a0_7->unk0;
                                            var_s5 = 1;
                                            var_t3 = temp_v0_7 & 0xF;
                                            sp80 = (temp_v0_7 >> 4) & 3;
                                        }
                                        else if ((var_s5 == var_t1_3) &&
                                                 ((temp_v1_9 = var_a0_7->unk0, (var_t3 != (temp_v1_9 & 0xF))) ||
                                                  (sp80 != ((temp_v1_9 >> 4) & 3))))
                                        {
                                            var_s5 = 2;
                                        }
                                        if (var_s6 == 0)
                                        {
                                            var_s6 = 1;
                                            var_t4 = (s32)var_a0_7->unk2;
                                            var_t8 = ((u8)var_a0_7->unk0 >> 6) & 1;
                                        }
                                        else if ((var_s6 == var_t1_3) && ((var_t4 != var_a0_7->unk2) ||
                                                                          (var_t8 != (((u8)var_a0_7->unk0 >> 6) & 1))))
                                        {
                                            var_s6 = 2;
                                        }
                                    }
                                    var_s1 *= 2;
                                    if (var_s1 == 0)
                                    {
                                        var_s3->unk0 = (Unk*)sp3C;
                                        var_s3 += 4;
                                        var_s1 = 1;
                                        sp3C = 0;
                                    }
                                    var_a0_7 += 4;
                                    var_s0_4 -= 1;
                                    var_fp += 4;
                                } while (var_s0_4 != -1);
                            }
                            if (var_s1 != var_t1_3)
                            {
                                var_s3->unk0 = (Unk*)sp3C;
                            }
                            if (var_s5 == var_t1_3)
                            {
                                temp_s2->unk18 = (s32)(var_t3 + (sp80 * 0x10) + 1);
                            }
                            else
                            {
                                temp_s2->unk18 = 0;
                            }
                            if (var_s6 == var_t1_3)
                            {
                                temp_s2->unk1C = (s32)(var_t4 + (var_t8 << 9) + 1);
                            }
                            else
                            {
                                temp_s2->unk1C = 0;
                            }
                        }
                    }
                    var_s7 += 4;
                } while (*var_s7 != NULL);
            }
            var_t6 += 1;
            var_t5->unk0 = NULL;
        } while (*var_t6 != NULL);
    }
    sp38->unk0 = NULL;
    var_s1_2 = sp34->unk8;
    if (var_s1_2 != NULL)
    {
        do
        {
            temp_a3 = var_s1_2->unk4;
            if (((u16)D_80180008 >= 0x12U) && (temp_a3->unk8 != 0xFF))
            {
                if (temp_a3->unk9 != 0xFF)
                {
                    var_s1_2->unk8 = NULL;
                    temp_v0_8 = func_8005AB80(temp_a3->unk8, temp_a3->unk9);
                    var_s1_2->unkC = temp_v0_8;
                    if (((Unk*)temp_v0_8->unk4)->unk8 & 0xF000)
                    {
                        sp34->unkC = var_s1_2;
                    }
                    temp_v1_10 = var_s1_2->unkC;
                    temp_v1_10->unk22 = (u8)(temp_v1_10->unk22 + 1);
                }
                else
                {
                    temp_v0_9 = func_8005AB4C(temp_a3->unk8);
                    var_s1_2->unk8 = temp_v0_9;
                    temp_v0_9->unkD = (u8)(temp_v0_9->unkD + 1);
                    goto block_125;
                }
            }
            else
            {
                var_s1_2->unk8 = NULL;
            block_125:
                var_s1_2->unkC = NULL;
            }
            var_s1_2 = var_s1_2->unk0;
        } while (var_s1_2 != NULL);
    }
    func_80053880(arg0->unk14, 0);
    func_80053880(arg0->unk18, 1);
    func_80053880(arg0->unk1C, 2);
    func_80053880(arg0->unk20, 3);
    var_t0_3 = sp34->unk4;
    if (var_t0_3 != NULL)
    {
        do
        {
            temp_t2 = var_t0_3->unk4;
            sp10 = var_t0_3->unk10 << 8;
            sp14 = var_t0_3->unk12 << 8;
            sp18 = var_t0_3->unk14 << 8;
            temp_a0_2 = temp_t2->unk4;
            sp54 = var_t0_3;
            sp5C = temp_t2;
            func_8005AC50(temp_a0_2 + 4, *temp_a0_2, &sp10);
            var_t0_4 = sp54;
            var_t2_4 = sp5C;
            var_s2 = var_t0_4->unk8;
            sp20 = 0;
            if (var_s2 != NULL)
            {
                do
                {
                    sp54 = var_t0_4;
                    sp5C = var_t2_4;
                    func_8005AD20(var_s2->unk21, *(u16*)var_t2_4->unk4, &sp20);
                    temp_s4_2 = var_s2->unk4;
                    var_t0_4 = sp54;
                    var_fp_2 = temp_s4_2->unk0;
                    var_t2_4 = sp5C;
                    if (var_fp_2 != 0)
                    {
                        temp_v1_11 = var_s2->unk8;
                        if (temp_v1_11 != NULL)
                        {
                            var_s2->unkC = (s32*)temp_v1_11->unkC;
                            var_s2->unk14 = (s32)temp_v1_11->unk14;
                            var_s2->unk18 = (s32)temp_v1_11->unk18;
                            var_s2->unk1C = (s32)temp_v1_11->unk1C;
                            var_s2->unk10 = (Unk*)temp_v1_11->unk10;
                            var_s2->unk26 = (u16)temp_v1_11->unk26;
                        }
                        else
                        {
                            temp_v1_12 = var_s2->unk21;
                            var_t1_4 = 0;
                            if (temp_v1_12 != 1)
                            {
                                if ((s32)temp_v1_12 < 2)
                                {
                                    var_s5_2 = 0xC;
                                    if (temp_v1_12 != 0)
                                    {
                                        var_s2->unk26 = 0U;
                                    }
                                    else
                                    {
                                        temp_v1_13 = var_s2->unk1C;
                                        var_s7_2 = sp24;
                                        var_s2->unk10 = var_s7_2;
                                        if (temp_v1_13 != 0)
                                        {
                                            temp_t4 = temp_v1_13 - 1;
                                            var_s2->unk1C = (s32)((Unk*)((temp_t4 & 0xFF) * 4))->unk1F800000;
                                            var_s6_2 = 1;
                                            if (temp_t4 & 0x200)
                                            {
                                                var_s2->unk1F = (u8)(var_s2->unk1F | 2);
                                            }
                                            var_s5_2 = 8;
                                        }
                                        else
                                        {
                                            var_s6_2 = 0;
                                        }
                                        temp_v0_10 = var_s2->unk18;
                                        temp_t3 = temp_v0_10 - 1;
                                        if (temp_v0_10 != 0)
                                        {
                                            temp_a0_3 = temp_t3 & 0xF;
                                            temp_v0_11 = temp_t3 * 2;
                                            if (temp_a0_3 >= 0xAU)
                                            {
                                                var_v1_4 = ((temp_s4_2->unk8 * 8) & 0x180) | (temp_v0_11 & 0x60);
                                                var_v0_6 = (u32)(((temp_a0_3 << 6) - 0x80) & 0x3FF) >> 6;
                                            }
                                            else
                                            {
                                                var_v1_4 = ((temp_s4_2->unk8 * 8) & 0x180) | (temp_v0_11 & 0x60);
                                                var_v0_6 = ((u32)((temp_a0_3 << 6) + 0x140) >> 6) | 0x10;
                                            }
                                            var_s2->unk18 = (s32)(((var_v1_4 | var_v0_6) & 0x9FF) | 0xE1000400);
                                            var_s6_2 |= 2;
                                            var_s5_2 -= 4;
                                        }
                                        var_s3_2 = var_s2->unkC;
                                        var_s0_5 = (temp_s4_2->unkA * temp_s4_2->unkB) - 1;
                                        var_s1_3 = 0;
                                        if (var_s0_5 != -1)
                                        {
                                            var_v1_5 = -1;
                                            do
                                            {
                                                if (var_s1_3 == 0)
                                                {
                                                    temp_t7 = *var_s3_2;
                                                    var_s3_2 += 4;
                                                    var_s1_3 = 1;
                                                    sp3C = temp_t7;
                                                }
                                                if (sp3C & var_s1_3)
                                                {
                                                    temp_a1_5 = var_s7_2;
                                                    var_s7_2 += var_s5_2;
                                                    temp_t1 = var_t1_4 + 1;
                                                    sp50 = var_v1_5;
                                                    sp54 = var_t0_4;
                                                    sp58 = temp_t1;
                                                    sp5C = var_t2_4;
                                                    func_8005477C(var_fp_2, temp_a1_5, ((u32)temp_s4_2->unk8 >> 4) & 3,
                                                                  var_s6_2);
                                                    var_t1_4 = temp_t1;
                                                }
                                                var_s1_3 *= 2;
                                                var_s0_5 -= 1;
                                                var_fp_2 += 4;
                                            } while (var_s0_5 != var_v1_5);
                                            var_v0_7 = var_t1_4 & 0xFFFF;
                                        }
                                        else
                                        {
                                            goto block_173;
                                        }
                                        goto block_174;
                                    }
                                }
                                else
                                {
                                    var_s5_2 = 0xC;
                                    if ((s32)temp_v1_12 < 6)
                                    {
                                        temp_v1_14 = var_s2->unk1C;
                                        var_s7_3 = sp24;
                                        var_s2->unk10 = var_s7_3;
                                        if (temp_v1_14 != 0)
                                        {
                                            temp_t4_2 = temp_v1_14 - 1;
                                            var_s2->unk1C = (s32)((Unk*)((temp_t4_2 & 0xFF) * 4))->unk1F800000;
                                            var_s6_3 = 1;
                                            if (temp_t4_2 & 0x200)
                                            {
                                                var_s2->unk1F = (u8)(var_s2->unk1F | 2);
                                            }
                                            var_s5_2 = 8;
                                        }
                                        else
                                        {
                                            var_s6_3 = 0;
                                        }
                                        temp_v0_12 = var_s2->unk18;
                                        temp_t3_2 = temp_v0_12 - 1;
                                        if (temp_v0_12 != 0)
                                        {
                                            temp_a0_4 = temp_t3_2 & 0xF;
                                            temp_v0_13 = temp_t3_2 * 2;
                                            if (temp_a0_4 >= 0xAU)
                                            {
                                                var_v1_6 = ((temp_s4_2->unk8 * 8) & 0x180) | (temp_v0_13 & 0x60);
                                                var_v0_8 = (u32)(((temp_a0_4 << 6) - 0x80) & 0x3FF) >> 6;
                                            }
                                            else
                                            {
                                                var_v1_6 = ((temp_s4_2->unk8 * 8) & 0x180) | (temp_v0_13 & 0x60);
                                                var_v0_8 = ((u32)((temp_a0_4 << 6) + 0x140) >> 6) | 0x10;
                                            }
                                            var_s2->unk18 = (s32)(var_v1_6 | var_v0_8);
                                            var_s6_3 |= 2;
                                            var_s5_2 -= 4;
                                        }
                                        var_s3_3 = var_s2->unkC;
                                        var_s0_6 = (temp_s4_2->unkA * temp_s4_2->unkB) - 1;
                                        var_s1_4 = 0;
                                        if (var_s0_6 != -1)
                                        {
                                            var_v1_7 = -1;
                                            do
                                            {
                                                if (var_s1_4 == 0)
                                                {
                                                    temp_t7_2 = *var_s3_3;
                                                    var_s3_3 += 4;
                                                    var_s1_4 = 1;
                                                    sp3C = temp_t7_2;
                                                }
                                                if (sp3C & var_s1_4)
                                                {
                                                    temp_a1_6 = var_s7_3;
                                                    var_s7_3 += var_s5_2;
                                                    temp_t1_2 = var_t1_4 + 1;
                                                    sp50 = var_v1_7;
                                                    sp54 = var_t0_4;
                                                    sp58 = temp_t1_2;
                                                    sp5C = var_t2_4;
                                                    func_80054904(var_fp_2, temp_a1_6, ((u32)temp_s4_2->unk8 >> 4) & 3,
                                                                  var_s6_3);
                                                    var_t1_4 = temp_t1_2;
                                                }
                                                var_s1_4 *= 2;
                                                var_s0_6 -= 1;
                                                var_fp_2 += 4;
                                            } while (var_s0_6 != var_v1_7);
                                        }
                                    block_173:
                                        var_v0_7 = var_t1_4 & 0xFFFF;
                                    block_174:
                                        sp24 += var_v0_7 * var_s5_2;
                                    }
                                    goto block_175;
                                }
                            }
                            else
                            {
                            block_175:
                                var_s2->unk26 = (u16)var_t1_4;
                            }
                        }
                    }
                    else
                    {
                        var_s2->unk10 = NULL;
                        var_s2->unk26 = 0U;
                    }
                    var_s2 = var_s2->unk0;
                } while (var_s2 != NULL);
            }
            var_t0_3 = var_t0_4->unk0;
        } while (var_t0_3 != NULL);
    }
    sp34->unk38 = NULL;
    sp34->unk3C = 0;
    func_80053C7C(arg0->unk14, &sp24, sp34 + 0x18);
    func_80053C7C(arg0->unk18, &sp24, sp34 + 0x1C);
    func_80053C7C(arg0->unk1C, &sp24, sp34 + 0x20);
    func_80053C7C(arg0->unk20, &sp24, sp34 + 0x24);
    var_v1_8 = D_80180010;
    var_s0_7 = D_80180010 - 1;
    var_t1_5 = sp34 + 0x14;
    if (var_s0_7 != -1)
    {
        do
        {
            temp_s1_2 = sp24;
            sp24 = temp_s1_2 + 0x10;
            var_t1_5->unk0 = temp_s1_2;
            var_t1_5 = temp_s1_2;
            var_s0_7 -= 1;
            var_t1_5->unk4 = var_v1_8;
            var_v1_8 += 0xC;
            var_t1_5->unk8 = (s32)(var_t1_5->unk8 & ~3);
        } while (var_s0_7 != -1);
    }
    var_t1_5->unk0 = NULL;
    var_s1_5 = sp34->unk14;
    var_s0_8 = 0;
    if (var_s1_5 != NULL)
    {
        do
        {
            if (((Unk*)var_s1_5->unk4)->unk1 & (1 << sp28))
            {
                func_8005A744(var_s1_5, var_s0_8 & 0xFF);
            }
            var_s1_5 = var_s1_5->unk0;
            var_s0_8 += 1;
        } while (var_s1_5 != NULL);
    }
    sp34->unk34 = 0;
    if (sp34->unk38 != NULL)
    {
        sp34->unk38 = sp24;
        sp24 += 0x14C00;
        DecDCTReset(0);
        DecDCTvlcBuild(sp34->unk38);
    }
    *sp30 = sp24;
    arg0->unk26 = 1;
}
