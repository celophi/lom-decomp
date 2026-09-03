#include "common.h"
#include "sdk/libgpu.h"
extern u8 D_800C6720[];
extern s32 D_800CA8D0;
#define W32(p, o) (*(u32 *)((p) + (o)))
#define H16(p, o) (*(s16 *)((p) + (o)))
#define U16(p, o) (*(u16 *)((p) + (o)))

/**
 * @brief Emit the four shaded border quads and a trailing 0xE1000040 tag into a GPU packet.
 * @param p GPU packet cursor to write primitives into.
 * @param arg1 Ordering-table entry the primitives are linked into via addPrim.
 * @return Pointer just past the emitted primitives (p advanced by 5 * 0x10 + 8).
 * @note WIP - not yet byte-matching. Currently 99.92% (residual: two GPU-store
 *       instructions swap order at the first primitive; a scheduling coin-flip
 *       the permuter could not close over 27k iterations).
 */
void *func_80050B40(u8 *p, s32 *arg1)
{
    u8 *state;
    if (D_800CA8D0 < 0x40) D_800CA8D0 += 4;
    state = D_800C6720;
    p[3]=3; p[7]=0x60; { u8 shade; shade=D_800CA8D0; H16(p,0xC)=0x140; H16(p,0xA)=0; H16(p,8)=0; p[6]=shade; p[5]=shade; p[4]=shade; p[7]|=2; }
    H16(p,0xE)=U16(state,0x46)+0xB;
    addPrim(arg1, p); p+=0x10;
    p[7]=0x60; p[3]=3; {u8 shade=D_800CA8D0; p[6]=shade; p[5]=shade; p[4]=shade;} H16(p,8)=0; p[7]|=2;
    H16(p,0xA)=U16(state,0x46)+0x6B; H16(p,0xC)=0x140; H16(p,0xE)=0xE0-H16(p,0xA);
    addPrim(arg1, p); p+=0x10;
    p[7]=0x60; p[3]=3; {u8 shade=D_800CA8D0; p[6]=shade; p[5]=shade; p[4]=shade;} H16(p,8)=0; p[7]|=2;
    H16(p,0xA)=U16(state,0x46)+0xB; H16(p,0xC)=U16(state,0x44)+0xB; H16(p,0xE)=0x60;
    addPrim(arg1, p); p+=0x10;
    p[7]=0x60; p[3]=3; {u8 shade=D_800CA8D0; p[6]=shade; p[5]=shade; p[4]=shade;}  p[7]|=2;
    H16(p,8)=U16(state,0x44)+0x6B; H16(p,0xA)=U16(state,0x46)+0xB; H16(p,0xC)=0x140-U16(p,8); H16(p,0xE)=0x60;
    addPrim(arg1, p); p+=0x10;
    p[3]=1; W32(p,4)=0xE1000040;
    addPrim(arg1, p);
    return p+8;
}
