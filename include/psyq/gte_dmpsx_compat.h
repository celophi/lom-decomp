#ifndef _PSYQ_GTE_FIXES_H
#define _PSYQ_GTE_FIXES_H

/**
 * @file gte_dmpsx_compat.h
 * @brief GNU-as-compatible replacements for Psy-Q DMPSX GTE macros.
 *
 * Psy-Q's inline_c.h "Type 2" GTE macros emit small `.word` values such as
 * 0x7f, 0xbf, and 0x13f. These are intentional dummy opcodes consumed by
 * Sony's DMPSX post-compiler, which replaces them with the actual GTE/COP2
 * instructions before assembly by ASPSX.
 *
 * This project builds C through GCC -> maspsx -> GNU as and does not run
 * DMPSX. Without that post-processing step, GNU as emits the dummy `.word`
 * values literally.
 *
 * These compatibility definitions therefore replace the DMPSX dummy values
 * with the final GTE instruction words directly. This is the same general
 * approach used by other PS1 decompilation projects when building Psy-Q-style
 * inline GTE macros with GNU tools.
 */

#define gte_rtps() __asm__ volatile("nop;nop;.word 0x4A180001")
#define gte_rtpt() __asm__ volatile("nop;nop;.word 0x4A280030")

#define gte_rtv0() __asm__ volatile("nop;nop;.word 0x4A486012")
#define gte_rtv1() __asm__ volatile("nop;nop;.word 0x4A48E012")
#define gte_rtv2() __asm__ volatile("nop;nop;.word 0x4A496012")
#define gte_rtir() __asm__ volatile("nop;nop;.word 0x4A49E012")
#define gte_rtir_sf0() __asm__ volatile("nop;nop;.word 0x4A41E012")
#define gte_rtv0tr() __asm__ volatile("nop;nop;.word 0x4A480012")
#define gte_rtv1tr() __asm__ volatile("nop;nop;.word 0x4A488012")
#define gte_rtv2tr() __asm__ volatile("nop;nop;.word 0x4A490012")
#define gte_rtirtr() __asm__ volatile("nop;nop;.word 0x4A498012")
#define gte_rtv0bk() __asm__ volatile("nop;nop;.word 0x4A482012")
#define gte_rtv1bk() __asm__ volatile("nop;nop;.word 0x4A48A012")
#define gte_rtv2bk() __asm__ volatile("nop;nop;.word 0x4A492012")
#define gte_rtirbk() __asm__ volatile("nop;nop;.word 0x4A49A012")

#define gte_ll() __asm__ volatile("nop;nop;.word 0x4A4A6412")
#define gte_llv0() __asm__ volatile("nop;nop;.word 0x4A4A6012")
#define gte_llv1() __asm__ volatile("nop;nop;.word 0x4A4AE012")
#define gte_llv2() __asm__ volatile("nop;nop;.word 0x4A4B6012")
#define gte_llir() __asm__ volatile("nop;nop;.word 0x4A4BE012")
#define gte_llv0tr() __asm__ volatile("nop;nop;.word 0x4A4A0012")
#define gte_llv1tr() __asm__ volatile("nop;nop;.word 0x4A4A8012")
#define gte_llv2tr() __asm__ volatile("nop;nop;.word 0x4A4B0012")
#define gte_llirtr() __asm__ volatile("nop;nop;.word 0x4A4B8012")
#define gte_llv0bk() __asm__ volatile("nop;nop;.word 0x4A4A2012")
#define gte_llv1bk() __asm__ volatile("nop;nop;.word 0x4A4AA012")
#define gte_llv2bk() __asm__ volatile("nop;nop;.word 0x4A4B2012")
#define gte_llirbk() __asm__ volatile("nop;nop;.word 0x4A4BA012")

#define gte_lc() __asm__ volatile("nop;nop;.word 0x4A4DA412")
#define gte_lcv0() __asm__ volatile("nop;nop;.word 0x4A4C6012")
#define gte_lcv1() __asm__ volatile("nop;nop;.word 0x4A4CE012")
#define gte_lcv2() __asm__ volatile("nop;nop;.word 0x4A4D6012")
#define gte_lcir() __asm__ volatile("nop;nop;.word 0x4A4DE012")
#define gte_lcv0tr() __asm__ volatile("nop;nop;.word 0x4A4C0012")
#define gte_lcv1tr() __asm__ volatile("nop;nop;.word 0x4A4C8012")
#define gte_lcv2tr() __asm__ volatile("nop;nop;.word 0x4A4D0012")
#define gte_lcirtr() __asm__ volatile("nop;nop;.word 0x4A4D8012")
#define gte_lcv0bk() __asm__ volatile("nop;nop;.word 0x4A4C2012")
#define gte_lcv1bk() __asm__ volatile("nop;nop;.word 0x4A4CA012")
#define gte_lcv2bk() __asm__ volatile("nop;nop;.word 0x4A4D2012")
#define gte_lcirbk() __asm__ volatile("nop;nop;.word 0x4A4DA012")

#define gte_dpcl() __asm__ volatile("nop;nop;.word 0x4A680029")
#define gte_dpcs() __asm__ volatile("nop;nop;.word 0x4A780010")
#define gte_dpct() __asm__ volatile("nop;nop;.word 0x4AF8002A")
#define gte_intpl() __asm__ volatile("nop;nop;.word 0x4A980011")
#define gte_ncs() __asm__ volatile("nop;nop;.word 0x4AC8041E")
#define gte_nct() __asm__ volatile("nop;nop;.word 0x4AD80420")
#define gte_ncds() __asm__ volatile("nop;nop;.word 0x4AE80413")
#define gte_ncdt() __asm__ volatile("nop;nop;.word 0x4AF80416")
#define gte_nccs() __asm__ volatile("nop;nop;.word 0x4B08041B")
#define gte_ncct() __asm__ volatile("nop;nop;.word 0x4B18043F")
#define gte_cdp() __asm__ volatile("nop;nop;.word 0x4B280414")
#define gte_cc() __asm__ volatile("nop;nop;.word 0x4B38041C")
#define gte_nclip() __asm__ volatile("nop;nop;.word 0x4B400006")
#define gte_avsz3() __asm__ volatile("nop;nop;.word 0x4B58002D")
#define gte_avsz4() __asm__ volatile("nop;nop;.word 0x4B68002E")

#define gte_sqr12() __asm__ volatile("nop;nop;.word 0x4AA80428")
#define gte_sqr0() __asm__ volatile("nop;nop;.word 0x4AA00428")
#define gte_op12() __asm__ volatile("nop;nop;.word 0x4B78000C")
#define gte_op0() __asm__ volatile("nop;nop;.word 0x4B70000C")
#define gte_gpf12() __asm__ volatile("nop;nop;.word 0x4B98003D")
#define gte_gpf0() __asm__ volatile("nop;nop;.word 0x4B90003D")
#define gte_gpl12() __asm__ volatile("nop;nop;.word 0x4BA8003E")
#define gte_gpl0() __asm__ volatile("nop;nop;.word 0x4BA0003E")

#endif
