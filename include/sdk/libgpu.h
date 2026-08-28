#ifndef SDK_LIBGPU_H
#define SDK_LIBGPU_H

/*
 * Legend of Mana GPU ABI contracts.
 *
 * This intentionally contains only the primitive layouts, packet helpers and
 * entry points referenced by the reconstructed game.
 */

#define setRECT(rect, x_value, y_value, width, height) \
    ((rect)->x = (x_value), (rect)->y = (y_value), \
     (rect)->w = (width), (rect)->h = (height))

#define setTPage(prim, tp, abr, x, y) \
    ((prim)->tpage = getTPage((tp), (abr), (x), (y)))
#define setClut(prim, x, y) \
    ((prim)->clut = getClut((x), (y)))

#define setRGB0(prim, red, green, blue) \
    ((prim)->r0 = (red), (prim)->g0 = (green), (prim)->b0 = (blue))
#define setXY0(prim, x, y) \
    ((prim)->x0 = (x), (prim)->y0 = (y))
#define setXY4(prim, x0v, y0v, x1v, y1v, x2v, y2v, x3v, y3v) \
    ((prim)->x0 = (x0v), (prim)->y0 = (y0v), \
     (prim)->x1 = (x1v), (prim)->y1 = (y1v), \
     (prim)->x2 = (x2v), (prim)->y2 = (y2v), \
     (prim)->x3 = (x3v), (prim)->y3 = (y3v))
#define setWH(prim, width, height) \
    ((prim)->w = (width), (prim)->h = (height))
#define setUV0(prim, u, v) \
    ((prim)->u0 = (u), (prim)->v0 = (v))
#define setUV4(prim, u0v, v0v, u1v, v1v, u2v, v2v, u3v, v3v) \
    ((prim)->u0 = (u0v), (prim)->v0 = (v0v), \
     (prim)->u1 = (u1v), (prim)->v1 = (v1v), \
     (prim)->u2 = (u2v), (prim)->v2 = (v2v), \
     (prim)->u3 = (u3v), (prim)->v3 = (v3v))

typedef struct {
    short x;
    short y;
    short w;
    short h;
} RECT;

typedef struct {
    u_long tag;
    u_long code[15];
} DR_ENV;

typedef struct {
    RECT clip;
    short ofs[2];
    RECT tw;
    u_short tpage;
    u_char dtd;
    u_char dfe;
    u_char isbg;
    u_char r0;
    u_char g0;
    u_char b0;
    DR_ENV dr_env;
} DRAWENV;

typedef struct {
    RECT disp;
    RECT screen;
    u_char isinter;
    u_char isrgb24;
    u_char pad0;
    u_char pad1;
} DISPENV;

typedef struct {
    unsigned addr : 24;
    unsigned len : 8;
    u_char r0;
    u_char g0;
    u_char b0;
    u_char code;
} P_TAG;

typedef struct {
    u_long tag;
    u_char r0, g0, b0, code;
    short x0, y0;
    short x1, y1;
    short x2, y2;
    short x3, y3;
} POLY_F4;

typedef struct {
    u_long tag;
    u_char r0, g0, b0, code;
    short x0, y0;
    u_char u0, v0;
    u_short clut;
    short x1, y1;
    u_char u1, v1;
    u_short tpage;
    short x2, y2;
    u_char u2, v2;
    u_short pad1;
    short x3, y3;
    u_char u3, v3;
    u_short pad2;
} POLY_FT4;

typedef struct {
    u_long tag;
    u_char r0, g0, b0, code;
    short x0, y0;
    u_char r1, g1, b1, pad1;
    short x1, y1;
    u_char r2, g2, b2, pad2;
    short x2, y2;
    u_char r3, g3, b3, pad3;
    short x3, y3;
} POLY_G4;

typedef struct {
    u_long tag;
    u_char r0, g0, b0, code;
    short x0, y0;
    u_char u0, v0;
    u_short clut;
    u_char r1, g1, b1, p1;
    short x1, y1;
    u_char u1, v1;
    u_short tpage;
    u_char r2, g2, b2, p2;
    short x2, y2;
    u_char u2, v2;
    u_short pad2;
    u_char r3, g3, b3, p3;
    short x3, y3;
    u_char u3, v3;
    u_short pad3;
} POLY_GT4;

typedef struct {
    u_long tag;
    u_char r0, g0, b0, code;
    short x0, y0;
    short x1, y1;
} LINE_F2;

typedef struct {
    u_long tag;
    u_char r0, g0, b0, code;
    short x0, y0;
    u_char r1, g1, b1, p1;
    short x1, y1;
} LINE_G2;

typedef struct {
    u_long tag;
    u_char r0, g0, b0, code;
    short x0, y0;
    short x1, y1;
    short x2, y2;
    short x3, y3;
    u_long pad;
} LINE_F4;

typedef struct {
    u_long tag;
    u_char r0, g0, b0, code;
    short x0, y0;
    u_char u0, v0;
    u_short clut;
    short w, h;
} SPRT;

typedef struct {
    u_long tag;
    u_char r0, g0, b0, code;
    short x0, y0;
    u_char u0, v0;
    u_short clut;
} SPRT_16;

typedef struct {
    u_long tag;
    u_char r0, g0, b0, code;
    short x0, y0;
    short w, h;
} TILE;

typedef struct {
    u_long tag;
    u_long code[2];
} DR_MODE;

typedef struct {
    u_long tag;
    u_long code[2];
} DR_TWIN;

typedef struct {
    u_long tag;
    u_long code[2];
} DR_AREA;

typedef struct {
    u_long tag;
    u_long code[2];
} DR_OFFSET;

typedef struct {
    u_long tag;
    u_long code[1];
} DR_TPAGE;

typedef struct {
    u_long tag;
    u_long code[2];
} DR_STP;

#define setlen(prim, length) \
    (((P_TAG *)(prim))->len = (u_char)(length))
#define setaddr(prim, address) \
    (((P_TAG *)(prim))->addr = (u_long)(address))
#define setcode(prim, command) \
    (((P_TAG *)(prim))->code = (u_char)(command))
#define getaddr(prim) \
    ((u_long)(((P_TAG *)(prim))->addr))
#define getcode(prim) \
    ((u_char)(((P_TAG *)(prim))->code))

#define addPrim(ordering_table, prim) \
    (setaddr((prim), getaddr(ordering_table)), setaddr((ordering_table), (prim)))
#define addPrims(ordering_table, first, last) \
    (setaddr((last), getaddr(ordering_table)), setaddr((ordering_table), (first)))

#define setSemiTrans(prim, enable) \
    ((enable) ? setcode((prim), getcode(prim) | 0x02) \
              : setcode((prim), getcode(prim) & ~0x02))

#define getTPage(tp, abr, x, y) \
    ((((tp) & 3) << 7) | (((abr) & 3) << 5) | (((y) & 0x100) >> 4) | \
     (((x) & 0x3ff) >> 6) | (((y) & 0x200) << 2))
#define getClut(x, y) \
    (((y) << 6) | (((x) >> 4) & 0x3f))

#define _get_mode(dfe, dtd, tpage) \
    (0xe1000000 | ((dtd) ? 0x0200 : 0) | ((dfe) ? 0x0400 : 0) | ((tpage) & 0x9ff))
#define setDrawTPage(prim, dfe, dtd, tpage) \
    (setlen((prim), 1), ((u_long *)(prim))[1] = _get_mode((dfe), (dtd), (tpage)))

#define _get_tw(window) \
    ((window) ? (0xe2000000 | ((((window)->y & 0xff) >> 3) << 15) | \
                  ((((window)->x & 0xff) >> 3) << 10) | \
                  (((~((window)->h - 1) & 0xff) >> 3) << 5) | \
                  ((~((window)->w - 1) & 0xff) >> 3)) : 0)
#define setTexWindow(prim, window) \
    (setlen((prim), 2), ((u_long *)(prim))[1] = _get_tw(window), ((u_long *)(prim))[2] = 0)

#define setPolyG3(prim)   (setlen((prim), 6), setcode((prim), 0x30))
#define setPolyF4(prim)   (setlen((prim), 5), setcode((prim), 0x28))
#define setPolyFT4(prim)  (setlen((prim), 9), setcode((prim), 0x2c))
#define setSprt16(prim)   (setlen((prim), 3), setcode((prim), 0x7c))
#define setSprt(prim)     (setlen((prim), 4), setcode((prim), 0x64))
#define setTile(prim)     (setlen((prim), 3), setcode((prim), 0x60))
#define setLineF2(prim)   (setlen((prim), 3), setcode((prim), 0x40))
#define setLineG2(prim)   (setlen((prim), 4), setcode((prim), 0x50))
#define setLineF4(prim)   (setlen((prim), 6), setcode((prim), 0x4c), (prim)->pad = 0x55555555)

extern DISPENV *GetDispEnv(DISPENV *env);
extern DISPENV *PutDispEnv(DISPENV *env);
extern DISPENV *SetDefDispEnv(DISPENV *env, int x, int y, int w, int h);
extern DRAWENV *GetDrawEnv(DRAWENV *env);
extern DRAWENV *PutDrawEnv(DRAWENV *env);
extern DRAWENV *SetDefDrawEnv(DRAWENV *env, int x, int y, int w, int h);
extern int ClearImage(RECT *rect, u_char r, u_char g, u_char b);
extern int ClearImage2(RECT *rect, u_char r, u_char g, u_char b);
extern int DrawSync(int mode);
extern int GetGraphDebug(void);
extern int LoadImage(RECT *rect, u_long *src);
extern int MoveImage(RECT *rect, int x, int y);
extern int ResetGraph(int mode);
extern int SetGraphDebug(int level);
extern int StoreImage(RECT *rect, u_long *dst);
extern u_long *ClearOTag(u_long *ot, int count);
extern u_long *ClearOTagR(u_long *ot, int count);
extern u_long DrawSyncCallback(void (*func)(void));
extern void DrawOTag(u_long *ot);
extern void DrawOTagEnv(u_long *ot, DRAWENV *env);
extern void DrawPrim(void *prim);
extern void SetDispMask(int mask);
extern void SetDrawArea(DR_AREA *prim, RECT *rect);
extern void SetDrawEnv(DR_ENV *packet, DRAWENV *env);
extern void SetDrawOffset(DR_OFFSET *prim, u_short *offset);
extern void SetPolyFT4(POLY_FT4 *prim);
extern void SetPolyG4(POLY_G4 *prim);
extern u_long *BreakDraw(void);
extern void ContinueDraw(u_long *insaddr, u_long *contaddr);
extern int IsIdleGPU(int max_count);
extern int GetODE(void);
extern int LoadImage2(RECT *rect, u_long *src);
extern int StoreImage2(RECT *rect, u_long *dst);
extern int MoveImage2(RECT *rect, int x, int y);
extern int DrawOTag2(u_long *ot);

#endif
