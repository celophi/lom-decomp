#include "common.h"
#include "main.h"
#include "vector.h"
/** @brief Active byte and fixed-point score accessed at offsets 0x5F0 and 0x610. */
typedef struct
{
    u8 pad0[0x5F0];
    u8 active;
    u8 pad5f1[0x1F];
    u32 score;
} FieldRankStats;
/** @brief Displayed numeric fields in a 0x268-byte player record. */
typedef struct
{
    u8 pad0[0x25A];
    u8 first;
    u8 second;
    u8 tail[0xC];
} FieldRankPlayer;
/** @brief Drawing position and three sorted actor indices sharing the local work area. */
typedef struct
{
    Vec2s position;
    s32 unused;
    s32 indices[3];
} FieldRankWork;
extern u8 D_800EC3C6[], D_800EC3DA[];
extern FieldRankPlayer D_800FD818[];
extern s32 D_801229A0[];
extern s32 func_800A88A0(s32, s32, u8 *, s32, s32, s32, s32);
extern s32 func_80086184(s32, s32, s32, Vec2s *);
extern s32 func_800A838C(s32, s32, s32, s32, s32);
extern s32 func_800A8A78(s32, s32, s32, s32, Vec2s *, s32);

/**
 * @brief Draw up to three active players in descending adjusted-score order.
 * @param arg0 Ordering table address passed to the drawing helpers.
 * @param arg1 Initial primitive-buffer address.
 * @param arg2 Horizontal drawing origin subtracted from each column position.
 * @param arg3 Vertical drawing origin subtracted from each row position.
 * @return Primitive-buffer address after the final emitted element.
 * @note Equal adjusted scores retain player order during insertion sorting.
 */
s32 func_800A7B54(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    FieldRankWork work;
    Vec2s *position;
    s32 count = 0;
    s32 i = count;
    s32 result;
    s32 *score;
    s32 *end;
    FieldRankStats *stats;
    FieldRankStats *base;
    s32 *scores;
    s32 pos;
    s32 j;
    s32 id;
    s32 *dst;
    s32 *src;
    s32 *current;
    s32 row;
    s16 y;
    u8 *text;

    result = func_800A88A0(arg1, arg0, D_800EC3C6[0] + ((D_800EC3C6[1] << 8) + (D_800EC3C6 - 2)), 4, 0x10 - arg2, -arg3, 0);
    scores = D_801229A0;
    score = scores;
    end = (s32 *)&work;
    base = (FieldRankStats *)g_pad_ctx;
    stats = base;
    do
    {
        stats = (FieldRankStats *)((u8 *)base + i * 0x250);
        if (stats->active != 0)
        {
            pos = 0;
            while (pos < count && (s32)((((FieldRankStats *)((u8 *)base + work.indices[pos] * 0x250))->score >> 8) - scores[work.indices[pos]]) >= (s32)((stats->score >> 8) - *score))
            {
                pos++;
            }
            if (pos == count)
            {
                end[2] = i;
            }
            else
            {
                j = count - 1;
                if (j >= pos)
                {
                    dst = &work.indices[j + 1];
                    src = (s32 *)&work + j;
                    do
                    {
                        *dst = src[2];
                        src--;
                        j--;
                        dst--;
                    } while (j >= pos);
                }
                work.indices[pos] = i;
            }
            end++;
            count++;
        }
        score++;
        i++;

    } while (i < 3);
    i = 0;
    if (count > 0)
    {
        text = D_800EC3DA - 0x16;
        row = i;
        position = &work.position;
        current = (s32 *)position;
        do
        {
            current = (s32 *)((s32)&work + i * 4);
            if (((FieldRankStats *)((u8 *)g_pad_ctx + current[2] * 0x250))->active != 0)
            {
                work.position.x = 0x18 - arg2;
                y = arg3 - 0x10;
                y = row - y;
                work.position.y = y;
                result = func_80086184(result, arg0, current[2], position);
                y += 8;
                work.position.y = y;
                result = func_800A838C(arg0, result, 0x38 - arg2, y - 8, 1);
                work.position.x = 0x48 - arg2;
                result = func_800A8A78(arg0, result, D_800FD818[current[2]].first, 4, position, 0);
                result = func_800A88A0(result, arg0, D_800EC3DA[0] + ((D_800EC3DA[1] << 8) + text), 4, 0x68 - arg2, work.position.y, 0);
                result = func_800A838C(arg0, result, 0x78 - arg2, work.position.y, 0);
                work.position.x = 0x88 - arg2;
                work.position.y = y;
                result = func_800A8A78(arg0, result, D_800FD818[current[2]].second, 4, position, 0);
                result = func_800A88A0(result, arg0, text[0x1A] + ((text[0x1B] << 8) + text), 4, 0xA8 - arg2, work.position.y, 0);
                work.position.x = 0xE0 - arg2;
                work.position.y = y;
                id = current[2];
                result = func_800A8A78(arg0, result, (((FieldRankStats *)((u8 *)g_pad_ctx + id * 0x250))->score >> 8) - D_801229A0[id], 4, position, 1);
            }
            row += 0x1C;
            i++;

        } while (i < count);
    }
    return result;
}
