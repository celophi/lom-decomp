#ifndef _PSYQ_GTE_FIXES_H
#define _PSYQ_GTE_FIXES_H

/**
 * @file gte_fixes.h
 * @brief Corrected replacements for psyq/inline_c.h's "Type 2" (no-operand)
 *        GTE macros.
 *
 * Every no-operand GTE macro in the vendored inline_c.h (gte_rtv0(),
 * gte_rtps(), gte_nclip(), and friends) hardcodes a small placeholder
 * `.word` value (0x7f, 0xbf, 0x13f, 0x117f, ...) instead of the real COP2
 * instruction word. This is not a maspsx/GNU-`as` translation issue - a
 * `.word` directive is never interpreted or rewritten by any assembler,
 * PSY-Q's own `aspsx` included, so the SDK header is simply wrong/a stub for
 * this entire macro family, on real PSY-Q too. The Castlevania: Symphony of
 * the Night decompilation (github.com/Xeeynamo/sotn-decomp,
 * include/psxsdk/libgte.h) hit the same bug independently and fixes it the
 * same way: hardcode the real opcode word directly. Their corrected values
 * agree exactly with ours for every op both projects use.
 *
 * `#include` this file after "psyq/inline_c.h" in any source that calls one
 * of the corrected macros below; the redefinition warning GCC prints
 * ("`gte_rtv0' redefined" / "this is the location of the previous
 * definition") is expected and harmless.
 *
 * Values are the standard PS1 GTE COP2 opcode encoding (bit 25 set marks a
 * GTE compute op; the remaining 25 bits are the per-instruction cofun
 * field), cross-checked against this project's own include/gte_macros.inc
 * (the GAS-macro equivalent used when reassembling splat output) and, for
 * rtv0/rtps/rtpt/nclip/avsz3/avsz4, against sotn-decomp's independently
 * derived values. gte_rtv0() (func_8007AA2C, field13.c) and gte_sqr0()
 * (func_8006D79C, field7.c, WIP) are build/diff-verified against an actual
 * matched function; the rest are transcribed from the same verified opcode
 * table but not yet exercised by a match. gte_rt(), gte_mvlvtr(), gte_nop(),
 * gte_FlipRotMatrixX(), and gte_FlipTRX() are intentionally NOT corrected
 * here - they're composite/non-arithmetic helpers with no single-opcode
 * equivalent to cross-check against. Verify by diff before trusting any one
 * of these in a new match.
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
