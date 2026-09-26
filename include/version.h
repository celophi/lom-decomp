#ifndef VERSION_H
#define VERSION_H

/**
 * @file version.h
 * @brief Game version selection macros.
 *
 * The Makefile defines exactly one VERSION_<NAME> macro for the release being
 * built (see mk/version.mk). Tools that compile sources outside the Makefile
 * (permuter, MCP helpers, m2c contexts) get VERSION_US by default.
 *
 * Version-specific code tests the macros directly:
 *
 * @code
 * #if defined(VERSION_JP)
 *     ...
 * #endif
 * @endcode
 */

#if !defined(VERSION_US) && !defined(VERSION_JP)
#define VERSION_US
#endif

#if defined(VERSION_US) && defined(VERSION_JP)
#error "Define only one of VERSION_US and VERSION_JP"
#endif

/**
 * @brief Splat assembly root for the selected version.
 * @note INCLUDE_ASM and INCLUDE_RODATA folder arguments are relative to this.
 */
#ifndef ASM_VERSION_DIR
#if defined(VERSION_JP)
#define ASM_VERSION_DIR "asm/jp"
#else
#define ASM_VERSION_DIR "asm/us"
#endif
#endif

#endif /* VERSION_H */
