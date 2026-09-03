typedef signed int s32;
typedef unsigned int u32;

typedef struct CardaElement {
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
} CardaElement;

extern s32 D_80165FFC;
extern s32 D_80165F38;
extern s32 D_80166104;
extern s32 D_80165FF4;
extern s32 D_801660FC;
extern void *D_8012271C;
extern s32 D_80166100;
extern s32 D_801660F8;
extern s32 D_80166078;
extern CardaElement D_80165F80;

void func_801425D4();
CardaElement *func_80142614();
s32 func_80145050(s32 *, s32, s32, s32);
s32 func_80141250(s32 *, s32, s32, s32);
s32 func_80141B50(s32 *, s32, s32, s32);
s32 func_80141C3C(s32 *, s32, s32, s32);
s32 func_80141D18(s32 *, s32, s32, s32);
s32 func_80141DF4(s32 *, s32, s32, s32);

#define SET_ELEM_CODE(e,c) ((e)->attr.word = ((e)->attr.word & 0x00FFFFFF) | ((u32)(c) << 24))

void func_801403FC(void)
{
    CardaElement *p;

    D_80165FFC = 0;
    D_80165F38 = 0;
    D_80166104 = 0;
    D_80165FF4 = 0;
    D_801660FC = 0;
    D_80166100 = (s32)D_8012271C + 0xCE0;
    if (0) func_801425D4(0, 0, 0, 0, 0);
    func_801425D4();
    D_801660F8 = 0;

    D_80165F80.attr.f.state = 1;
    if ((u32)(D_80166078 - 2) < 2U)
    {
        p = func_80142614();
        p->draw_handler = (void *)func_80145050;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x10;
        p->attr.f.unk0_16 = 0x4C;
        p->unk4_0 = 1;
        p->y = 0x48;
        SET_ELEM_CODE(p, 0x20);

        p = func_80142614();
        p->draw_handler = (void *)func_80141C3C;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x1C;
        p->attr.f.unk0_16 = 0x3A;
        p->unk4_0 = 0;
        p->y = 0x10;
        SET_ELEM_CODE(p, 0x80);

        p = func_80142614();
        p->draw_handler = (void *)func_80141D18;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0xA4;
        p->attr.f.unk0_16 = 0x3A;
        p->unk4_0 = 0;
        p->y = 0x10;
        SET_ELEM_CODE(p, 0x80);

        p = func_80142614();
        p->draw_handler = (void *)func_80141B50;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x20;
        p->attr.f.unk0_16 = 0x22;
        p->unk4_0 = 1;
        p->y = 0x10;
        SET_ELEM_CODE(p, 0);
    }
    else
    {
        p = func_80142614();
        p->draw_handler = (void *)func_80141250;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x0A;
        p->attr.f.unk0_16 = 0x32;
        p->unk4_0 = 1;
        p->y = 0x58;
        SET_ELEM_CODE(p, 0x2C);

        p = func_80142614();
        p->draw_handler = (void *)func_80141B50;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x68;
        p->attr.f.unk0_16 = 0x0A;
        p->unk4_0 = 0;
        p->y = 0x10;
        SET_ELEM_CODE(p, 0x70);

        p = func_80142614();
        p->draw_handler = (void *)func_80141C3C;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x1C;
        p->attr.f.unk0_16 = 0x1E;
        p->unk4_0 = 0;
        p->y = 0x10;
        SET_ELEM_CODE(p, 0x80);

        p = func_80142614();
        p->draw_handler = (void *)func_80141D18;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0xA4;
        p->attr.f.unk0_16 = 0x1E;
        p->unk4_0 = 0;
        p->y = 0x10;
        SET_ELEM_CODE(p, 0x80);

        p = func_80142614();
        p->draw_handler = (void *)func_80141DF4;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x1E;
        p->attr.f.unk0_16 = 0x8E;
        p->unk4_0 = 1;
        p->y = 0x34;
        SET_ELEM_CODE(p, 4);
    }
    D_80165F80.attr.f.state = 0;
}
