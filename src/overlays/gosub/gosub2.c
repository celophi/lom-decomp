typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;

typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;

void func_80142C64(); /* extern */
void func_80143B64(); /* extern */
void func_80145CEC(); /* extern */
void func_80146468(); /* extern */
void func_80146538(); /* extern */

typedef struct
{
    union
    {
        u32 word;
        struct
        {
            u32 unk0_0 : 3;
            u32 unk0_3 : 4;
            u32 x : 9;
            u32 unk0_16 : 8;
        } f;
    } attr;
    u32 unk4_0 : 1;
    u32 y : 8;
    u32 unk4_9 : 23;
    void* handler;
} GosubElement;

typedef struct
{
    u32 word;
} GosubSlot;

extern GosubElement g_gosub_fixed_element;
extern u8* D_8012271C;
extern s32 D_801227F0;
extern s32 D_801229B0[];
extern s32 D_8014F29C;
extern s32 g_gosub_cursor_row;
extern s32 g_gosub_row_count;
extern s32 g_gosub_visible_row_count;
extern u8 g_gosub_screen_sequence_index;
extern s32 D_8016B8E0;
extern s32 D_8016B8E4;
extern s32 D_8016B8E8;
extern s32 D_8016B8EC;
extern s32 D_8016B8F0;
extern u8 D_8016B8FC;
extern u8 g_gosub_required_selection_count;
extern s32 g_gosub_window_height;
extern s32 g_gosub_window_width;
extern u8 g_gosub_selection_count;
extern s32 D_8017097C;
extern s32 g_gosub_row_height;
extern s32 D_80170988;
extern s32 D_80170990;

/**
 * @brief One row of the item list built by the gosub screen builders.
 *
 * Mirrors the layout documented on GosubListEntry in gosub.c; see that file
 * for why the 0xC word and the 0x1C flag set are spelled the way they are.
 */
typedef struct
{
    u8* name;
    u8* desc;
    s16 value;
    s16 index;
    s32 unkC : 8;
    s32 unkD : 8;
    s32 unkE : 8;
    s32 kind : 4;
    u32 unkC_28 : 4;
    u16 unk10;
    u16 unk12[4];
    u16 unk1A;
    union
    {
        struct
        {
            u32 flag0 : 1;
            u32 flag1 : 1;
            u32 flag2 : 1;
            u32 unk1C_3 : 29;
        } f;
        u16 half;
        u32 word;
    } flags;
} GosubListEntry;

extern s32 D_801228F0;
extern GosubListEntry g_gosub_rows[];
extern u8 g_gosub_selected_rows[];

extern u8 D_800EC3E2[];
extern u32 D_8014F27C;
extern u32 D_8014F280;
extern u32 D_8014F288;
extern s32 D_8016B900;
extern u8 D_8016B960[];
extern u8* g_gosub_title_text;

#define GOSUB_MSG_PTR(off) ((u8*)&D_8014F29C - 0x20 + D_8014F29C + *(u16*)((u8*)&D_8014F29C + D_8014F29C + (off)))

#define GOSUB_MSG(off) func_80145CEC(GOSUB_MSG_PTR(off))

/**
 * decomp.me (99.94%) https://decomp.me/scratch/2OzmD
 */
s32 func_8014289C(s32 arg0)
{
    s32 count;
    GosubSlot* rec;
    u32 tmp1;
    u32 tmp2;
    s32 cfg;
    s32 c2;
    s32 mask;

    if (arg0 == 0 && (D_8016B8E8 & 1) == 0)
    {
        mask = ~0xFC;
        count = *(D_8012271C + 0x29D6);
        if (count < 0x28)
        {
            rec = (GosubSlot*)(D_8012271C + count * 4 + 0x29DC);
            cfg = D_8017097C;
            tmp1 = rec->word & mask;
            tmp1 = tmp1 | ((cfg & 0x3F) << 2);
            rec->word = tmp1;
            c2 = D_8016B8EC;
            arg0 = (tmp1 & ~0xF00) | ((c2 & 0xF) << 8);
            rec->word = arg0;
            tmp1 = ((arg0 & 0xFFFF0FFF) | ((D_8016B8E4 & 0xF) << 12) | 3) & 0xFFFF;
            rec->word = tmp1;
            *(D_8012271C + 0x29D6) = *(D_8012271C + 0x29D6) + 1;
            *(D_8012271C + (D_801229B0[0] << 6) + 0xCE0) = 0;
            *(D_8012271C + (D_801229B0[1] << 6) + 0xCE0) = 0;
            func_800A8FB4();
        }
        if (*(D_8012271C + 0x29D6) >= 0x28)
        {
            func_80143B64();
            D_801227F0 = 0;
            GOSUB_MSG(-4);
            return 0;
        }
        D_8016B8E0 = 0;
        D_80170990 = 0;
        D_80170988 = 0;
        g_gosub_cursor_row = 0;
        D_8016B8F0 = 0;
        func_80142C64(3);
        g_gosub_visible_row_count = 6;
        g_gosub_row_height = 0x10;
        g_gosub_window_width = 0xE8;
        g_gosub_window_height = 0x64;
        D_8016B8E4 = 0;
        D_8017097C = 0;
        D_8016B8EC = 0;
        g_gosub_required_selection_count = 2;
        D_8016B8FC = 2;
        g_gosub_selection_count = 0;
        g_gosub_fixed_element.attr.f.unk0_0 = 0;
        g_gosub_screen_sequence_index -= 1;
        if (g_gosub_row_count == 0)
        {
            D_801227F0 = 0;
            func_80067F28();
            func_80143B64();
            return 1;
        }
        return 0;
    }
    g_gosub_selection_count -= 1;
    g_gosub_screen_sequence_index -= 1;
    g_gosub_fixed_element.attr.f.unk0_0 = 0;
    return 0;
}

/**
 * decomp.me (95%) https://decomp.me/scratch/pOY6i
 */
s32 func_80142B18(void)
{
    s32 count;
    s32 var_v1;
    s32* var_a0;
    s16 temp_v0;

    if (g_gosub_selection_count == 0)
    {
        return 0;
    }

    var_v1 = 0;

    count = g_gosub_selection_count;
    D_801228F0 = count;

    if (count != 0)
    {
        var_a0 = &D_801229B0[0];

        do
        {
            temp_v0 = g_gosub_rows[g_gosub_selected_rows[var_v1]].index;
            var_v1 += 1;
            *var_a0 = (s32)temp_v0;
            var_a0 += 1;
        } while (var_v1 < count);
    }

    return 1;
}

/**
 * decomp.me (95%) https://decomp.me/scratch/FN7DQ
 */
s32 func_80142B98(void) {
    s32 count;
    s32 var_v1;
    s32* var_a0;
    s16 temp_v0;

    if (g_gosub_selection_count == 0) {
        return 0;
    }

    var_v1 = 0;
    
    count = g_gosub_selection_count;
    D_801228F0 = count; 
    
    if (count != 0) {
        var_a0 = &D_801229B0[0];
        
        do {
           
            temp_v0 = g_gosub_rows[g_gosub_selected_rows[var_v1]].index;
            var_v1 += 1;
            *var_a0 = (s32) temp_v0;
            var_a0 += 1; 
        } while (var_v1 < count); 
    }
    
    return 1;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/lBIH9
 */
s32 func_80142C18(s32 arg0)
{
    s32 i;
    s32 count = g_gosub_selection_count;

    for (i = 0; i < count; i++)
    {
        if (g_gosub_selected_rows[i] == arg0)
        {
            return 0;
        }
    }

    return 1;
}

/**
 * decomp.me (86.54%) https://decomp.me/scratch/CJYqj
 *
 */
void func_80142C64(u32 arg0)
{
    s32 i;
    s32 j;
    s32 count;
    u8* buf;
    u8* rec;
    u8* slot;
    GosubListEntry* e;
    u32 word;

    count = 0;
    D_8016B900 = 1;

    for (i = 0; i < 100; i++)
    {
        if (*(D_8012271C + i * 0x40 + 0xCE0) != 0)
        {
            if ((arg0 == 3) || (((*(u32*)(D_8012271C + i * 0x40 + 0xCF4) >> 8) & 3) == arg0) ||
                ((arg0 == 4) && (((*(u32*)(D_8012271C + i * 0x40 + 0xCF4) >> 8) & 3) != 2)))
            {
                e = &g_gosub_rows[count];
                buf = D_8016B960 + count * 0x50;

                e->name = D_8012271C + i * 0x40 + 0xCE0;

                func_80146538(buf, (u8*)&D_8014F27C + D_8014F280 +
                    *(u16*)((u8*)&D_8014F27C + D_8014F280 +
                        ((*(u16*)(D_8012271C + i * 0x40 + 0xCF6) & 0x3F) * 2)));
                func_80146468(buf, D_800EC3E2 - 0x1E + (D_800EC3E2[1] << 8) + D_800EC3E2[0]);

                rec = D_8012271C + i * 0x40;
                e->unkC_28 = (*(u32*)(rec + 0xCF4) >> 8) & 3;
                word = *(u32*)(rec + 0xCF4);

                switch ((word >> 8) & 3)
                {
                case 0:
                    func_80146468(buf, (u8*)&D_8014F27C + *(u32*)((u8*)&D_8014F27C + 0xC) +
                        *(u16*)((u8*)&D_8014F27C + *(u32*)((u8*)&D_8014F27C + 0xC) +
                            ((word >> 9) & 0x7E)));
                    e->unk10 = *(u16*)(D_8012271C + i * 0x40 + 0xD04);
                    break;
                case 1:
                    func_80146468(buf, (u8*)&D_8014F27C + *(u32*)((u8*)&D_8014F27C + 0xC) +
                        *(u16*)((u8*)&D_8014F27C + *(u32*)((u8*)&D_8014F27C + 0xC) +
                            ((word >> 9) & 0x7E) + 0x16));
                    slot = D_8012271C + i * 0x40 + 0xCE0;
                    for (j = 0; j < 4; j++)
                    {
                        e->unk12[j] = *(u16*)(slot + j * 2 + 0x24);
                    }
                    break;
                default:
                    rec = D_8012271C + i * 0x40;
                    slot = rec + 0xCE0;
                    e->unk10 = slot[0x26];
                    e->unk12[0] = slot[0x25] + (slot[0x24] * 14);
                    func_80146468(buf, (u8*)&D_8014F27C + D_8014F288 +
                        *(u16*)((u8*)&D_8014F288 + D_8014F288 +
                            ((*(u32*)(rec + 0xCF4) >> 9) & 0x7E) + 0x22));
                    break;
                }

                e->desc = buf;
                count++;
                e->value = -1;
                e->index = i;
                e->kind = 4;
            }
        }
    }

    g_gosub_row_count = count;
    g_gosub_visible_row_count = 8;

    switch (arg0)
    {
    case 0:
        g_gosub_title_text = GOSUB_MSG_PTR(0xC);
        break;
    case 1:
        g_gosub_title_text = GOSUB_MSG_PTR(0xE);
        break;
    case 2:
        g_gosub_title_text = GOSUB_MSG_PTR(0x10);
        break;
    case 3:
        g_gosub_title_text = GOSUB_MSG_PTR(0x16);
        break;
    case 4:
        g_gosub_visible_row_count = 7;
        g_gosub_title_text = GOSUB_MSG_PTR(0x16);
        break;
    }

    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = (g_gosub_visible_row_count * 0x10) + 4;
}