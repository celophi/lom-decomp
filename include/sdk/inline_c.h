#ifndef SDK_INLINE_C_BRIDGE_H
#define SDK_INLINE_C_BRIDGE_H

/* GTE data-transfer helpers used by the current Legend of Mana sources. */

#define gte_ldv0(r0)                                                                                                                                           \
    __asm__ volatile("lwc2	$0, 0( %0 );"                                                                                                                       \
                     "lwc2	$1, 4( %0 )"                                                                                                                        \
                     :                                                                                                                                         \
                     : "r"(r0))

#define gte_ldlvl(r0)                                                                                                                                          \
    __asm__ volatile("lwc2	$9, 0( %0 );"                                                                                                                       \
                     "lwc2	$10, 4( %0 );"                                                                                                                      \
                     "lwc2	$11, 8( %0 )"                                                                                                                       \
                     :                                                                                                                                         \
                     : "r"(r0))

#define gte_ldsxy3(r0, r1, r2)                                                                                                                                 \
    __asm__ volatile("mtc2	%0, $12;"                                                                                                                           \
                     "mtc2	%2, $14;"                                                                                                                           \
                     "mtc2	%1, $13"                                                                                                                            \
                     :                                                                                                                                         \
                     : "r"(r0), "r"(r1), "r"(r2))

#define gte_SetBackColor(r0, r1, r2)                                                                                                                           \
    __asm__ volatile("sll	$12, %0, 4;"                                                                                                                         \
                     "sll	$13, %1, 4;"                                                                                                                         \
                     "sll	$14, %2, 4;"                                                                                                                         \
                     "ctc2	$12, $13;"                                                                                                                          \
                     "ctc2	$13, $14;"                                                                                                                          \
                     "ctc2	$14, $15"                                                                                                                           \
                     :                                                                                                                                         \
                     : "r"(r0), "r"(r1), "r"(r2)                                                                                                               \
                     : "$12", "$13", "$14")

#define gte_SetRotMatrix(r0)                                                                                                                                   \
    __asm__ volatile("lw	$12, 0( %0 );"                                                                                                                        \
                     "lw	$13, 4( %0 );"                                                                                                                        \
                     "ctc2	$12, $0;"                                                                                                                           \
                     "ctc2	$13, $1;"                                                                                                                           \
                     "lw	$12, 8( %0 );"                                                                                                                        \
                     "lw	$13, 12( %0 );"                                                                                                                       \
                     "lw	$14, 16( %0 );"                                                                                                                       \
                     "ctc2	$12, $2;"                                                                                                                           \
                     "ctc2	$13, $3;"                                                                                                                           \
                     "ctc2	$14, $4"                                                                                                                            \
                     :                                                                                                                                         \
                     : "r"(r0)                                                                                                                                 \
                     : "$12", "$13", "$14")

#define gte_SetLightMatrix(r0)                                                                                                                                 \
    __asm__ volatile("lw	$12, 0( %0 );"                                                                                                                        \
                     "lw	$13, 4( %0 );"                                                                                                                        \
                     "ctc2	$12, $8;"                                                                                                                           \
                     "ctc2	$13, $9;"                                                                                                                           \
                     "lw	$12, 8( %0 );"                                                                                                                        \
                     "lw	$13, 12( %0 );"                                                                                                                       \
                     "lw	$14, 16( %0 );"                                                                                                                       \
                     "ctc2	$12, $10;"                                                                                                                          \
                     "ctc2	$13, $11;"                                                                                                                          \
                     "ctc2	$14, $12"                                                                                                                           \
                     :                                                                                                                                         \
                     : "r"(r0)                                                                                                                                 \
                     : "$12", "$13", "$14")

#define gte_SetColorMatrix(r0)                                                                                                                                 \
    __asm__ volatile("lw	$12, 0( %0 );"                                                                                                                        \
                     "lw	$13, 4( %0 );"                                                                                                                        \
                     "ctc2	$12, $16;"                                                                                                                          \
                     "ctc2	$13, $17;"                                                                                                                          \
                     "lw	$12, 8( %0 );"                                                                                                                        \
                     "lw	$13, 12( %0 );"                                                                                                                       \
                     "lw	$14, 16( %0 );"                                                                                                                       \
                     "ctc2	$12, $18;"                                                                                                                          \
                     "ctc2	$13, $19;"                                                                                                                          \
                     "ctc2	$14, $20"                                                                                                                           \
                     :                                                                                                                                         \
                     : "r"(r0)                                                                                                                                 \
                     : "$12", "$13", "$14")

#define gte_SetTransMatrix(r0)                                                                                                                                 \
    __asm__ volatile("lw	$12, 20( %0 );"                                                                                                                       \
                     "lw	$13, 24( %0 );"                                                                                                                       \
                     "ctc2	$12, $5;"                                                                                                                           \
                     "lw	$14, 28( %0 );"                                                                                                                       \
                     "ctc2	$13, $6;"                                                                                                                           \
                     "ctc2	$14, $7"                                                                                                                            \
                     :                                                                                                                                         \
                     : "r"(r0)                                                                                                                                 \
                     : "$12", "$13", "$14")

#define gte_stopz(r0) __asm__ volatile("swc2	$24, 0( %0 )" : : "r"(r0) : "memory")

#define gte_stlvnl(r0)                                                                                                                                         \
    __asm__ volatile("swc2	$25, 0( %0 );"                                                                                                                      \
                     "swc2	$26, 4( %0 );"                                                                                                                      \
                     "swc2	$27, 8( %0 )"                                                                                                                       \
                     :                                                                                                                                         \
                     : "r"(r0)                                                                                                                                 \
                     : "memory")

#define gte_stsv(r0)                                                                                                                                           \
    __asm__ volatile("mfc2	$12, $9;"                                                                                                                           \
                     "mfc2	$13, $10;"                                                                                                                          \
                     "mfc2	$14, $11;"                                                                                                                          \
                     "sh	$12, 0( %0 );"                                                                                                                        \
                     "sh	$13, 2( %0 );"                                                                                                                        \
                     "sh	$14, 4( %0 )"                                                                                                                         \
                     :                                                                                                                                         \
                     : "r"(r0)                                                                                                                                 \
                     : "$12", "$13", "$14", "memory")

#define gte_strgb(r0) __asm__ volatile("swc2	$22, 0( %0 )" : : "r"(r0) : "memory")

#endif
