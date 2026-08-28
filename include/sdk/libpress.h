#ifndef SDK_LIBPRESS_H
#define SDK_LIBPRESS_H

/* VLC table and DCT environment layouts are ABI contracts used by LoM. */
typedef u_short DECDCTTAB[34816];

typedef struct {
    u_char iq_y[64];
    u_char iq_c[64];
    short dct[64];
} DECDCTENV;

typedef struct {
    short *src;
    short *dest;
    short *work;
    long size;
    long loop_start;
    char loop;
    char byte_swap;
    char proceed;
    char quality;
} ENCSPUENV;

extern void DecDCTReset(int mode);
extern DECDCTENV *DecDCTGetEnv(DECDCTENV *env);
extern DECDCTENV *DecDCTPutEnv(DECDCTENV *env);
extern int DecDCTvlc2(u_long *bitstream, u_long *dst, DECDCTTAB table);
extern int DecDCTvlcSize2(int size);
extern void DecDCTvlcBuild(u_short *table);
extern void DecDCTin(u_long *buf, int mode);
extern void DecDCTout(u_long *buf, int size);
extern int DecDCTinSync(int mode);
extern int DecDCToutSync(int mode);
extern int DecDCTinCallback(void (*func)());
extern int DecDCToutCallback(void (*func)());

#endif
