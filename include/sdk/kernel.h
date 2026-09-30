#ifndef SDK_KERNEL_H
#define SDK_KERNEL_H

#define LOM_PSYQ_DESC_HW 0xf0000000
#define DescRC            0xf2000000
#define LOM_PSYQ_DESC_SW 0xf4000000
#define HwCARD            (LOM_PSYQ_DESC_HW | 0x11)
#define SwCARD            (LOM_PSYQ_DESC_SW | 0x01)

#define RCntCNT2          (DescRC | 0x02)
#define RCntMdINTR        0x1000

#define EvSpINT           0x0002
#define EvSpIOE           0x0004
#define EvSpTIMOUT        0x0100
#define EvSpNEW           0x2000
#define EvSpERROR         0x8000
#define EvMdINTR          0x1000
#define EvMdNOINTR        0x2000

#ifndef NULL
#define NULL 0
#endif

struct EXEC {
    u_long pc0;
    u_long gp0;
    u_long t_addr;
    u_long t_size;
    u_long d_addr;
    u_long d_size;
    u_long b_addr;
    u_long b_size;
    u_long s_addr;
    u_long s_size;
    u_long sp;
    u_long fp;
    u_long gp;
    u_long ret;
    u_long base;
};

typedef PS1_PTR(struct DIRENTRY) DIRENTRYPtr;
struct DIRENTRY {
    char name[20];
    Ps1Long attr;
    Ps1Long size;
    DIRENTRYPtr next;
    Ps1Long head;
    char system[4];
};

#if defined(_LANGUAGE_C) || defined(LANGUAGE_C)
#define delete erase
#endif

#endif
