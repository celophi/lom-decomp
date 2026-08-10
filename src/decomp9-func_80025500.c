typedef int         s32;
typedef unsigned int    u32;
typedef unsigned char   u8;
typedef signed char     s8;
typedef unsigned short  u16;
typedef signed short    s16;

#define NULL 0

extern s16* g_field_node_angle_table;
extern long ratan2(long y, long x);

typedef struct Move_EdgeRun {
    u16 count;
    u16 index;
} Move_EdgeRun;

typedef struct Move_UnkNode2 {
    u8 pad0[4];
    s32 unk4;
    u8 pad8[2];
    u16 unkA;
    u16 unkC;
    u16 unkE;
    s16 unk10;
    s16 unk12;
    s16 unk14;
    u8 pad16[2];
    Move_EdgeRun runs[1];
} Move_UnkNode2;

s32 func_8005E1A8(Move_UnkNode2* def, s32 edge, s32 dir, s32 best)
{
    s16* tbl;
    s16* prev;
    s16* ep;
    Move_EdgeRun* run;
    s32 scratch;
    s32 angle;
    s32 dx;
    s32 dy;
    s32 d2;
    s32 mid;
    s32 wrap;

    if (best == -1)
    {
        return -1;
    }
    prev = NULL;
    if (edge < 0x7E)
    {
        tbl = g_field_node_angle_table;
        for (run = def->runs; (scratch = run->count & 0x7FFF) != 0; run++)
        {
            ep = &tbl[run->index * 2];
            while (--scratch != -1)
            {
                mid = prev != NULL;
                if (mid)
                {
                    if (edge == 0)
                    {
                        goto found;
                    }
                    edge--;
                }
                prev = ep;
                ep += 2;
            }
        }
        ep = &tbl[def->runs[0].index * 2];
    found:
        dx = ep[0] - prev[0];
        dy = ep[1] - prev[1];
        if (dx == 0)
        {
            angle = 0x400;
        }
        else
        {
            angle = ratan2(dy, dx) & 0x7FF;
        }
    }
    else
    {
        angle = (edge == 0x7E) << 10;
    }

    if (dir < angle)
    {
        scratch = angle - dir;
    }
    else
    {
        scratch = dir - angle;
        if (scratch > 0x800)
        {
            wrap = dir - 0x1000;
            scratch = angle - wrap;
        }
    }
    mid = angle + 0x800;
    if (dir < mid)
    {
        d2 = mid - dir;
        if (d2 > 0x800)
        {
            wrap = angle - 0x800;
            d2 = dir - wrap;
        }
    }
    else
    {
        d2 = dir - mid;
    }
    if (scratch == d2)
    {
        return -1;
    }
    if (d2 < scratch)
    {
        angle += 0x800;
    }
    if (best == -2)
    {
        return angle;
    }

    scratch = angle - dir;
    if (scratch > 0x800)
    {
        scratch -= 0x1000;
    }
    else if (scratch < -0x800)
    {
        scratch += 0x1000;
    }
    d2 = best - dir;
    if (d2 > 0x800)
    {
        d2 -= 0x1000;
    }
    else if (d2 < -0x800)
    {
        d2 += 0x1000;
    }

    if ((scratch >= 0) && (d2 >= 0))
    {
        if (scratch < d2)
        {
            angle = best;
        }
        else if (scratch >= 0x472)
        {
            angle = -1;
        }
    }
    else if ((scratch <= 0) && (d2 <= 0))
    {
        if (d2 < scratch)
        {
            angle = best;
        }
        else if (scratch < -0x471)
        {
            angle = -1;
        }
    }
    else
    {
        angle = -1;
    }
    return angle;
}