#ifndef PS1_TYPES_H
#define PS1_TYPES_H

/**
 * @file ps1_types.h
 * @brief Scalar types and four-byte object pointers used in PS1 storage.
 *
 * Define PS1_32BIT_STORAGE when compiling this code natively on a 64-bit
 * host. Stored pointers and long then keep the PS1's four-byte size through
 * Clang's pointer-width extensions (-fms-extensions). The PS1 build leaves it
 * undefined, since its pointers are already four bytes.
 * Stored addresses must refer to mapped memory below 4 GiB. These types do
 * not translate addresses, resolve callbacks, or decode tagged GPU links.
 * Ordinary local pointers and function parameters retain the host ABI.
 */

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;

typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;

#if defined(PS1_32BIT_STORAGE)
#if !defined(__clang__)
#error PS1_32BIT_STORAGE requires Clang with -fms-extensions
#elif __is_identifier(__ptr32) || __is_identifier(__uptr)
#error PS1_32BIT_STORAGE requires -fms-extensions for four-byte pointers
#endif

/** @brief Qualifier used when defining a stored object-pointer typedef. */
#define PS1_PTR32 __ptr32 __uptr
typedef unsigned int u_long;
typedef int Ps1Long;
typedef __UINTPTR_TYPE__ host_uintptr;
#else
#define PS1_PTR32
typedef unsigned long u_long;
typedef long Ps1Long;
typedef u32 host_uintptr;
#endif

/*
 * host_uintptr is an unsigned integer as wide as an ordinary pointer: u32 on
 * the PS1, the host's pointer width on native builds. Use it where the code
 * does address arithmetic on an integer, instead of narrowing through s32.
 */

/** @brief Stored pointers; native loads zero-extend the four-byte address. */
typedef void* PS1_PTR32 void_ptr;
typedef u8* PS1_PTR32 u8_ptr;
typedef s8* PS1_PTR32 s8_ptr;
typedef u16* PS1_PTR32 u16_ptr;
typedef s16* PS1_PTR32 s16_ptr;
typedef u32* PS1_PTR32 u32_ptr;
typedef s32* PS1_PTR32 s32_ptr;
typedef u_long* PS1_PTR32 u_long_ptr;

/*
 * Stored code addresses. Some tables the game loads from the disc hold PS1
 * function addresses. PS1_CODE(type) is the type of one such slot: the
 * function pointer type on the PS1, and on native builds a four-byte PS1
 * address that means nothing to the host. Use it once, in a <Type>Slot
 * typedef next to the function type, and declare tables with that typedef,
 * the same way the <Type>Ptr aliases work. Each expansion is its own type,
 * so declare a table through one shared Slot typedef, not a fresh PS1_CODE.
 *
 * PS1_CALL(slot) calls through a slot. On the PS1 it is just the slot. A
 * native build calls ps1_resolve_code, which it must provide, to map the PS1
 * address to a host function; an unknown address should fail loudly rather
 * than be skipped. The native slot carries its function type in a zero-size
 * member, so the call is checked against the real prototype, and anything
 * that isn't a slot fails to compile.
 */
#if defined(PS1_32BIT_STORAGE)
typedef void (*Ps1CodeFunc)(void);
Ps1CodeFunc ps1_resolve_code(u32 address);
#define PS1_CODE(type)                                                                                                                                         \
    struct __attribute__((packed, aligned(4)))                                                                                                                 \
    {                                                                                                                                                          \
        u32 address;                                                                                                                                           \
        type function_type[0];                                                                                                                                 \
    }
#define PS1_CALL(slot) ((__typeof__((slot).function_type[0]))ps1_resolve_code((slot).address))
#else
#define PS1_CODE(type) type
#define PS1_CALL(slot) (slot)
#endif

#if defined(PS1_32BIT_STORAGE)
_Static_assert(sizeof(void_ptr) == 4, "PS1 address slots must remain four bytes");
_Static_assert(sizeof(u_long) == 4 && sizeof(Ps1Long) == 4, "PS1 long storage must remain four bytes");
#endif

#endif
