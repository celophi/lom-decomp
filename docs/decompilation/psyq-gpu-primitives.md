# PSY-Q GPU Primitive Macros

> Quick reference for building GPU primitives and draw commands by hand,
> and how to recognize them in decompiled code.

---

## Ordering Table (OTag) Link Pattern

Every primitive in PSX is inserted into an ordering table via this two-line idiom:

```c
// node->next = ot->next  (steal the current head's forward pointer)
*((u_long*)node) = (*((u_long*)node) & 0xFF000000) | (*((u_long*)ot) & 0x00FFFFFF);
// ot->next = node        (make the head point to node)
*((u_long*)ot)   = (*((u_long*)ot)   & 0xFF000000) | ((u_long)node   & 0x00FFFFFF);
```

Each OTag entry is a packed 32-bit word:

```
bits 31-24 : len       — number of 32-bit data words that follow the tag
bits 23-0  : next_ptr  — 24-bit address of the next entry in the chain
```

The `& 0xFF000000` guard preserves the existing `len` byte; `& 0x00FFFFFF` extracts or writes the link pointer.

---

## DR_TPAGE — Setting the Active Texture Page

`DR_TPAGE` is a 2-word primitive (tag + one GPU command word) that switches the
texture page mid-list without drawing anything. It is typically inserted **before**
the SPRT or polygon that uses that page.

### The macro

```c
setDrawTPage(DR_TPAGE* p, int dfe, int dtd, u_short tpage);
```

| Param | Meaning |
|-------|---------|
| `p` | pointer to a `DR_TPAGE`-sized slot in the primitive buffer |
| `dfe` | draw-to-display-area enable (0 = off, 1 = on) |
| `dtd` | dither enable (0 = off, 1 = on) |
| `tpage` | texture page value — use `getTPage()` to build this |

Expands to:

```c
setlen(p, 1);                                        // tag: len = 1 data word
((u_long*)(p))[1] = 0xE1000000
                  | ((dfe) ? 0x400 : 0)
                  | ((dtd) ? 0x200 : 0)
                  | ((tpage) & 0x9FF);
```

`0xE1` is the GPU "set draw mode" command byte.

### Worked example — decompiled raw code → macro

Raw decompiled form:

```c
node1[3] = 1;                                  // TAG: len = 1 word
*(u_long*)&node1[4] = 0xE10000A5UL;           // GPU draw mode cmd, tpage=0xA5
```

Equivalent macro call:

```c
setDrawTPage((DR_TPAGE*)node1, 0, 0, getTPage(1, 1, 320, 0));
```

---

## getTPage — Building the Texture Page Value

```c
u_short getTPage(int tp, int abr, int x, int y);
```

| Param | Meaning | Values |
|-------|---------|--------|
| `tp` | color depth | 0 = 4bpp, 1 = 8bpp, 2 = 15bpp |
| `abr` | semi-transparency mode | 0 = avg, 1 = add, 2 = sub, 3 = add¼ |
| `x` | VRAM X coordinate of page origin (pixels) | multiple of 64 |
| `y` | VRAM Y coordinate of page origin (pixels) | 0 or 256 |

### Bit layout of the result

```
bit:  8    7    6    5    4    3    2    1    0
      TP[1] TP[0] ABR[1] ABR[0] TY  TX[3] TX[2] TX[1] TX[0]
```

### How the macro encodes each field

```
TX = x / 64          → (x & 0x3FF) >> 6    placed at bits [3:0]
TY = y / 256         → (y & 0x100) >> 4    placed at bit  [4]
ABR                  → (abr & 0x3)  << 5   placed at bits [6:5]
TP                   → (tp  & 0x3)  << 7   placed at bits [8:7]
```

### Step-by-step: `getTPage(1, 1, 320, 0)` → `0xA5`

| Field | Calculation | Bits set |
|-------|------------|----------|
| TX    | `320 / 64 = 5` → `0x05` | `0000 0101` |
| TY    | `0 / 256 = 0` → `0x00` | `0000 0000` |
| ABR   | `1 << 5 = 0x20` | `0010 0000` |
| TP    | `1 << 7 = 0x80` | `1000 0000` |
| **OR** | `0x80 \| 0x20 \| 0x05` | **`0xA5`** |

And `getTPage(1, 1, 448, 0)`:

| Field | Calculation | Bits set |
|-------|------------|----------|
| TX    | `448 / 64 = 7` → `0x07` | `0000 0111` |
| ABR   | `1 << 5 = 0x20` | `0010 0000` |
| TP    | `1 << 7 = 0x80` | `1000 0000` |
| **OR** | `0x80 \| 0x20 \| 0x07` | **`0xA7`** |

### Step-by-step: `_get_mode(0, 0, 0xA5)` → `0xE10000A5`

```
0xE1000000   GPU command byte 0xE1 ("set draw mode")
|    0x000   dfe = 0  (draw-to-display off)
|    0x000   dtd = 0  (dither off)
|    0x0A5   tpage = 0xA5
= 0xE10000A5
```

---

## SPRT — Free-Size Sprite

```c
SPRT* p = (SPRT*)slot;
setSprt(p);                              // len=4, code=0x64
setXY0(p, x, y);                        // screen position
setWH(p, w, h);                         // pixel dimensions
setUV0(p, u, v);                        // texture coordinate
setClut(p, clut_x, clut_y);            // CLUT origin in VRAM
setRGB0(p, r, g, b);                   // color modulation (0x80 = neutral/full brightness)
```

`RGB = 0x80, 0x80, 0x80` is full-brightness (neutral). Decreasing toward 0 darkens the sprite
— used here to drive the Game Over screen fade-in/fade-out via `g_fadeLevel`.
