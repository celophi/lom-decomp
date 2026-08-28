#ifndef SDK_KERNEL_H
#define SDK_KERNEL_H

#define LOM_PSYQ_DESC_HW 0xf0000000
#define LOM_PSYQ_DESC_SW 0xf4000000
#define HwCARD            (LOM_PSYQ_DESC_HW | 0x11)
#define SwCARD            (LOM_PSYQ_DESC_SW | 0x01)

#define EvSpIOE           0x0004
#define EvSpTIMOUT        0x0100
#define EvSpNEW           0x2000
#define EvSpERROR         0x8000
#define EvMdNOINTR        0x2000

#ifndef NULL
#define NULL 0
#endif

struct EXEC {
    unsigned long pc0;
    unsigned long gp0;
    unsigned long t_addr;
    unsigned long t_size;
    unsigned long d_addr;
    unsigned long d_size;
    unsigned long b_addr;
    unsigned long b_size;
    unsigned long s_addr;
    unsigned long s_size;
    unsigned long sp;
    unsigned long fp;
    unsigned long gp;
    unsigned long ret;
    unsigned long base;
};

struct DIRENTRY {
    char name[20];
    long attr;
    long size;
    struct DIRENTRY *next;
    long head;
    char system[4];
};

#if defined(_LANGUAGE_C) || defined(LANGUAGE_C)
#define delete erase
#endif

#endif
