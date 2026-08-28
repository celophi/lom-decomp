#ifndef LOM_PSYQ_COMPAT_LIBETC_H
#define LOM_PSYQ_COMPAT_LIBETC_H

#define PADLup     (1 << 12)
#define PADLdown   (1 << 14)
#define PADLleft   (1 << 15)
#define PADLright  (1 << 13)
#define PADRup     (1 << 4)
#define PADRdown   (1 << 6)
#define PADRleft   (1 << 7)
#define PADRright  (1 << 5)
#define PADi       (1 << 9)
#define PADh       (1 << 11)
#define PADL1      (1 << 2)
#define PADL2      (1 << 0)
#define PADR1      (1 << 3)
#define PADR2      (1 << 1)
#define PADselect  (1 << 8)

int CheckCallback(void);
int ResetCallback(void);
int RestartCallback(void);
int StopCallback(void);
int VSync(int mode);
int VSyncCallback(void (*func)());
long GetVideoMode(void);
long SetVideoMode(long mode);

#endif
