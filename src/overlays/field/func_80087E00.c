typedef signed int s32;
typedef signed short s16;
typedef unsigned short u16;
typedef unsigned char u8;

typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x168 - 0x18];
    s32 unk168;
    u8 pad16C[0x23C - 0x16C];
} FieldActorSlot;

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u8 pad0C[0x28 - 0x0C];
    u8 unk28;
    u8 pad29;
    s16 unk2A;
    s16 unk2C;
    u8 pad2E[0x3A - 0x2E];
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} FieldActorRecord;

extern FieldActorSlot D_80105AE0[];
extern FieldActorRecord D_800FDF58[];

s32 func_80087E00(s32 key, s32 value)
{
    FieldActorRecord *scan;
    FieldActorRecord *found;
    FieldActorSlot *e;
    s32 i;
    s32 result;
    s16 state;

    scan = D_800FDF58;
    e = D_80105AE0;
    i = 0;
loop:
    i++;
    if (e->unk14 == key)
        goto found_label;
    e++;
    scan++;
    if (i < 13)
        goto loop;
    found = (FieldActorRecord *)-1;
check:
    if (found != (FieldActorRecord *)-1)
        goto body;
    result = -1;
    goto done;
found_label:
    found = scan;
    goto check;
body:
    state = found->unk2A;
    if ((u16)(state - 0x93) < 2)
    {
        result = -1;
        goto done;
    }
    if (state == 0x90 || state == 0xAE || state == 0x8E)
    {
        result = -1;
        goto done;
    }
    found->unk28 = 0xFE;
    D_80105AE0[found->unk3A].unk168 = value;
    found->unk2C = 0;
    if (found->unk2A != 0x8B && found->unk2A != 0x99)
        found->unk2A = 0;
    result = 0;
done:
    return result;
}
