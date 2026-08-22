#include "common.h"

typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

typedef struct NikiElement {
    union {
        u32 word;
        struct {
            u32 state : 3;
            u32 unk0_3 : 4;
            u32 x : 9;
            u32 unk0_16 : 8;
        } f;
    } attr;
    u32 unk4_0 : 1;
    u32 y : 8;
    u32 unk4_9 : 23;
    void *draw_handler;
    s32 unkC;
} NikiElement;

typedef struct NikiRecord {
    u8 pad0[0x17];
    u8 unk17;
    u8 pad18[0xCF - 0x18];
    u8 unkCF;
    u8 padD0[4];
    u16 unkD4;
} NikiRecord;


extern s32 D_80164A78;
extern s32 D_80164AD0;
extern s32 D_80164AD4;
extern s32 D_80164AE4;
extern s32 D_80164AE8;
extern s32 D_80164B70;
extern s32 D_80164B74;
extern s32 D_80164B78;
extern s32 D_80164B84;
extern s32 D_80164B8C;
extern s32 D_80164B90;
extern s32 D_80164ADC;
extern s32 D_80164B7C;
extern s32 D_80164B1C;
extern s32 D_80122988;
extern s32 D_80164B88;
extern s32 D_80164AE0;
extern s32 D_80164AEC;

extern s32 D_80164B80;
extern s32 D_801606C8;
extern s32 D_801606D4;
extern s32 D_801606DC;

extern NikiElement D_80164B10;
extern s32 D_80164F18;
extern s32 D_80164A78;
extern u8 *D_80164E18;
extern s32 D_80122994;
extern s32 D_80164B70;
extern char D_800ECF7C[];
extern u8 D_80165018[];
extern NikiRecord D_80164D18;
extern NikiRecord *D_8012271C;
extern s32 D_8003EC9C;


void func_80140D2C(void);
s32 func_80140774(void);
s32 func_80140868(void);
void func_80140D4C(); 
void func_801413FC();
 void func_801414A8();
void func_80141584(); 
void func_80141660(); 
void func_801433BC();
void func_80141E84();
s32 func_80144DF8(void);

void func_800A3938();
void func_80140C60();
void func_80140BF0();
void func_80145F68();
void func_80140CC8();
s32 func_8001714C();
NikiElement *func_80141EC4();
void func_80143284();
void func_80145994();
void func_8014262C();

#define SET_ELEM_CODE(e, c) ((e)->attr.word = ((e)->attr.word & 0x00FFFFFF) | ((u32)(c) << 24))

void func_8014011C(s32 arg0, s32 arg1)
{
    RECT rect;

    D_80164AE8 = arg1;
    D_80164B78 = 0xFF;
    D_80164B70 = 0;
    func_80144BC0();
    func_80145A40();
    D_80164AD0 = 0;
    func_80067F8C();
    rect.x = 0x140;
    rect.y = 0;
    rect.w = 0x40;
    rect.h = 0x100;
    func_8001990C(&rect, 0, 0, 0);
    func_80146F34();
    D_80164B90 = 0;
    D_80164AD4 = 0;
    D_80164B84 = 0;
    D_80164A78 = 0;
    D_80164B8C = 0;
    D_80164B74 = 0;
    func_800AA02C();
    func_8014027C();
    D_80164AE4 = arg0;
}

s32 func_801401F0(s32 arg0)
{
    if (D_80164B74 != 0)
    {
        func_80145C0C();
        field_text_reset_windows();
        func_80019788(0);
        return 1;
    }
    field_text_reset_scratch();
    func_80146EB8();
    func_8014068C(arg0);
    func_80146EF4();
    func_80063194();
    D_80164B8C ^= 1;
    return 0;
}

typedef struct
{
    union
    {
        u32 word;
        struct
        {
            u32 state : 3;
            u32 unk0_3 : 4;
            u32 x : 9;
            u32 unk0_16 : 8;
        } f;
    } attr;
    u32 unk4_0 : 1;
    u32 y : 8;
    u32 unk4_9 : 23;
    void* draw_handler;
} NikiElement;

extern NikiElement D_80164B10;

NikiElement *func_80141EC4();
#define SET_ELEM_CODE(e, c) ((e)->attr.word = ((e)->attr.word & 0x00FFFFFF) | ((u32)(c) << 24))

void func_8014027C(void)
{
    NikiElement *p;
    D_80164B88 = 0;
    D_80164AEC = 0;
    D_80164AE0 = 0;
    D_80164B7C = 0;
    D_80164B84 = 0;
    D_80164ADC = D_8012271C + 0xCE0;
    if (0) func_80141E84(0,0,0,0,0);
    func_80141E84();
    D_80164B80 = 0;
    if (D_80164AE8 != 0)
    {
        D_80164B10.attr.f.state = 1;
        p = func_80141EC4();
        p->draw_handler = (void *)func_801433BC;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x10;
        p->attr.f.unk0_16 = 0x61;
        p->unk4_0 = 1;
        p->y = 0x2C;
        SET_ELEM_CODE(p, 0x20);

        p = func_80141EC4();
        p->draw_handler = (void *)func_801414A8;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x18;
        p->attr.f.unk0_16 = 0x4D;
        p->unk4_0 = 0;
        p->y = 0x10;
        SET_ELEM_CODE(p, 0x80);

        p = func_80141EC4();
        p->draw_handler = (void *)func_80141584;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0xA0;
        p->attr.f.unk0_16 = 0x4D;
        p->unk4_0 = 0;
        p->y = 0x10;
        SET_ELEM_CODE(p, 0x80);
        D_80164B10.attr.f.state = 0;
        return;
    }

    D_80164B10.attr.f.state = 1;
    p = func_80141EC4();
    p->draw_handler = (void *)func_80140D4C;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0x32;
    p->unk4_0 = 1;
    p->y = 0x58;
    SET_ELEM_CODE(p, 8);

    p = func_80141EC4();
    p->draw_handler = (void *)func_801413FC;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x24;
    p->attr.f.unk0_16 = 0x0A;
    p->unk4_0 = 0;
    p->y = 0x10;
    SET_ELEM_CODE(p, 0xF0);

    p = func_80141EC4();
    p->draw_handler = (void *)func_801414A8;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x18;
    p->attr.f.unk0_16 = 0x1E;
    p->unk4_0 = 0;
    p->y = 0x10;
    SET_ELEM_CODE(p, 0x80);

    p = func_80141EC4();
    p->draw_handler = (void *)func_80141584;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0xA0;
    p->attr.f.unk0_16 = 0x1E;
    p->unk4_0 = 0;
    p->y = 0x10;
    SET_ELEM_CODE(p, 0x80);

    p = func_80141EC4();
    p->draw_handler = (void *)func_80141660;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1E;
    p->attr.f.unk0_16 = 0x8E;
    p->unk4_0 = 1;
    p->y = 0x34;
    SET_ELEM_CODE(p, 4);
    D_80164B10.attr.f.state = 0;
}

void func_8014068C(void)
{
    s32 delta;

    func_80140D2C();
    D_80164AD0 += 2;
    if ((D_80164B1C & 0x7F) == 2)
    {
        func_80140774();
    }
    if ((u16)D_80122988 == 0xFFFF)
    {
        D_80122988 = 0;
    }
    func_80140868();
    if (D_80164B88 != 0)
    {
        s32 base = D_80164AE0;
        delta = (D_80164AEC - D_80164AE0) / D_80164B88;
        D_80164B88 -= 1;
        D_80164AE0 += delta;
    }
    else
    {
        D_80164AE0 = D_80164AEC;
    }
}

s32 func_80140774(void)
{
    s32 result;

    if (D_80164B78 >= 0x10)
    {
        if (D_80164E18 == NULL)
        {
            D_80164E18 = &D_801606C8;
        }
    }

    do
    {
        result = func_80144DF8();
    } while (result == 3);

    if ((D_80164B80 != 0) && (D_80122988 & 0x220))
    {
        D_80164B78 = 0xF8;
        D_80164E18 = &D_801606DC;
    }
    else
    {
        switch (result)
        {
        case 0:
            break;
        case 4:
            D_80164E18 = &D_801606D4;
            D_80164B80 = 0;
            break;
        case 5:
            D_80164B78 = 0xF8;
            /* fallthrough */
        case 2:
            D_80164E18 = &D_801606DC;
            break;
        }
    }
}

s32 func_80140868(void)
{
    s32 pending;
    s32 status;
    s32 count;
    s32 term1;
    s32 term2;
    NikiElement *p;

    if ((D_80164B10.unkC & 7) == 0) {
        D_80164B74 = 1;
        return;
    }
    if (D_80164B74 != 0) {
        return;
    }
    if ((D_80164B10.unkC & 7) >= 3) {
        return;
    }
    if ((D_80164B10.attr.word & 7) != 0) {
        return;
    }
    pending = D_80164B78;
    if (pending == 0xFF) {
        return;
    }
    if (D_80164F18 != 0) {
        return;
    }
    if (D_80164A78 != 0) {
        return;
    }
    if ((u32)(*D_80164E18 - 6) < 2U) {
        return;
    }
    if (D_80164AE8 != 0) {
        return;
    }

    status = D_80122988;
    if (status & 0x40) {
        D_80122994 = 3;
        func_800A3938(0x78, 0x80);
        func_80140C60();
        return;
    }
    if (status & 0xA100) {
        func_800A3938(0x7D, 0x80);
        func_80140BF0();
        return;
    }
    if (pending >= 0x10) {
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
            D_80164B7C -= 1;
            if (D_80164B7C < 0) {
                D_80164B7C = D_80164B78 - 1;
            }
        }
        if (D_80122988 & 0x4000) {
            D_80164B7C += 1;
            if (D_80164B7C >= D_80164B78) {
                D_80164B7C = 0;
            }
        }
        count -= 1;
    }

    if (D_80122988 & 0x5000) {
        func_80145F68();
        func_800A3938(0x7D, 0x80);
        func_80140CC8();
        return;
    }

    if (D_80122988 & 0x220) {
        if (D_80164AE8 != 0) {
            return;
        }
        term1 = D_80164B70 * 0x320;
        term2 = (D_80164B7C * 0x28) + (s32)D_80165018;
        if (func_8001714C(D_800ECF7C, (char *)(term1 + term2), 0xC) == 0) {
            if ((D_80164D18.unkD4 != D_8012271C->unkD4) &&
                (D_80164D18.unk17 != 0) &&
                ((D_8003EC9C == 0xFF) || (D_80164D18.unkCF == D_8003EC9C))) {
                p = func_80141EC4();
                p->attr.f.unk0_3 = 1;
                p->attr.f.x = 0x10;
                p->attr.f.unk0_16 = 0x61;
                p->unk4_0 = 1;
                p->y = 0x1E;
                SET_ELEM_CODE(p, 0x20);
                func_80143284();
                p->draw_handler = func_8014262C;
                func_80145994();
                func_800A3938(0x7E, 0x80);
                return;
            }
        }
        func_800A3938(0x78, 0x80);
    }
}
