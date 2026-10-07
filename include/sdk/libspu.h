#ifndef SDK_LIBSPU_H
#define SDK_LIBSPU_H

/* Only the SPU data contracts and entry points referenced by Legend of Mana. */

#define SPU_OFF 0
#define SPU_ON 1

#define SPU_TRANSFER_BY_DMA 0L
#define SPU_TRANSFER_BY_IO 1L

#define SPU_ALLCH 0xFFFFFFL

#define SPU_VOICE_LINEARIncN 1
#define SPU_VOICE_LINEARDecN 3

#define SPU_REV_MODE_CLEAR_WA 0x100
#define SPU_REV_MODE_STUDIO_C 4
typedef struct {
    short left;
    short right;
} SpuVolume;

typedef struct {
    unsigned long voice;
    unsigned long mask;
    SpuVolume volume;
    SpuVolume volmode;
    SpuVolume volumex;
    unsigned short pitch;
    unsigned short note;
    unsigned short sample_note;
    short envx;
    unsigned long addr;
    unsigned long loop_addr;
    long a_mode;
    long s_mode;
    long r_mode;
    unsigned short ar;
    unsigned short dr;
    unsigned short sr;
    unsigned short rr;
    unsigned short sl;
    unsigned short adsr1;
    unsigned short adsr2;
} SpuVoiceAttr;

typedef void (*SpuIRQCallbackProc)(void);
typedef void (*SpuTransferCallbackProc)(void);

extern void SpuInit(void);
extern void SpuStart(void);
extern void SpuQuit(void);
extern long SpuSetReverb(long on_off);
extern long SpuClearReverbWorkArea(long mode);
extern unsigned long SpuWrite(unsigned char *addr, unsigned long size);
extern unsigned long SpuRead(unsigned char *addr, unsigned long size);
extern long SpuSetTransferMode(long mode);
extern unsigned long SpuSetTransferStartAddr(unsigned long addr);
extern SpuTransferCallbackProc SpuSetTransferCallback(SpuTransferCallbackProc func);
extern long SpuSetIRQ(long on_off);
extern unsigned long SpuSetIRQAddr(unsigned long addr);
extern SpuIRQCallbackProc SpuSetIRQCallback(SpuIRQCallbackProc func);
extern long SpuInitMalloc(long count, char *top);
extern long SpuSetNoiseClock(long n_clock);
extern void SpuGetVoiceEnvelope(int vNum, short *envx);
extern void SpuSetReverbModeDepth(short depth_left, short depth_right);
extern long SpuSetReverbModeType(long mode);
extern void SpuGetReverbModeType(long *mode);

#endif
