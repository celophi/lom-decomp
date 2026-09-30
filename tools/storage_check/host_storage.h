/*
 * Storage definitions of a 64-bit native build that keeps PS1 memory layouts
 * (lom-native's model), used by storage_check.py to lay the game's types out
 * the way such a build would. Clang with -fms-extensions: __ptr32 keeps a
 * stored pointer four bytes and __uptr zero-extends it when it is loaded.
 *
 * This is test scaffolding, not game source. The decomp's own defaults in
 * include/ps1_storage.h are plain C.
 */
#define PS1_STORED(pointer_type) pointer_type __ptr32 __uptr
#define PS1_LONG int
#define PS1_CODE(type)                        \
    struct __attribute__((packed, aligned(4))) \
    {                                         \
        unsigned int address;                 \
        type function_type[0];                \
    }
/* A slot holds a PS1 code address; the port maps it to its own function. */
void (*ps1_resolve_code(unsigned int address))(void);
#define PS1_CALL(slot) ((__typeof__((slot).function_type[0]))ps1_resolve_code((slot).address))
