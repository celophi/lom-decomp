#ifndef PS1_STORAGE_H
#define PS1_STORAGE_H

/**
 * @file ps1_storage.h
 * @brief Types for the pointers, code addresses and longs the game keeps in memory.
 *
 * The game stores pointers inside its structures, tables and globals, where
 * they are four bytes, like its longs. Every such stored value is declared
 * through a typedef from here (or a <Type>Ptr / <Type>Slot typedef built the
 * same way next to its type), so a build with wider pointers can keep the PS1
 * memory layout by redefining a few hooks in one place. Locals and parameters
 * are not stored; they stay ordinary pointers.
 *
 * By default every hook is plain C and the PS1 build is unchanged. A native
 * build that keeps PS1 layouts defines the hooks before this header is read
 * (for example with a forced include):
 * - PS1_STORED(pointer_type): a stored copy of a data or function pointer type; default the type itself.
 * - PS1_PTR(type): a stored pointer to @p type; default PS1_STORED(type*).
 * - PS1_CODE(type): a stored PS1 code address read from the game data, of function
 *   pointer type @p type; default `type`. Unlike a stored function pointer that C
 *   code sets, it holds a PS1 address that a native build must translate.
 * - PS1_CALL(slot): the function a PS1_CODE slot holds, ready to call; default the slot itself.
 * - PS1_LONG: the stored long; default `long`.
 *
 * tools/storage_check checks that the game's types keep their PS1 layout
 * under such a build.
 */

#ifndef PS1_STORED
#define PS1_STORED(pointer_type) pointer_type
#endif

#ifndef PS1_PTR
#define PS1_PTR(type) PS1_STORED(type*)
#endif

#ifndef PS1_CODE
#define PS1_CODE(type) type
#endif

#ifndef PS1_CALL
#define PS1_CALL(slot) (slot)
#endif

#ifndef PS1_LONG
#define PS1_LONG long
#endif

/** @brief Psy-Q's unsigned long: four bytes in PS1 memory (GPU packet tags, SDK structures). */
typedef unsigned PS1_LONG u_long;
/** @brief A signed long as the PS1 stores it (SDK vector and matrix components). */
typedef PS1_LONG Ps1Long;

/** @brief Stored pointers to the basic types. */
typedef PS1_PTR(void) void_ptr;
typedef PS1_PTR(u8) u8_ptr;
typedef PS1_PTR(s8) s8_ptr;
typedef PS1_PTR(u16) u16_ptr;
typedef PS1_PTR(s16) s16_ptr;
typedef PS1_PTR(u32) u32_ptr;
typedef PS1_PTR(s32) s32_ptr;
typedef PS1_PTR(u_long) u_long_ptr;

#endif
