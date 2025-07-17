/*******************************************************************************
 *
 * @file ztypes.h
 *
 * @brief Specific Type Definitions which extend standard types
 *
 ******************************************************************************
 *
 * Copyright (c) 2016-2023 ZAYA Tech Ltd - All rights reserverd.
 *
 * Unauthorised copying of this file, via any medium is strictly prohibited.
 *
 ******************************************************************************/

#ifndef __Z_TYPES_H
#define __Z_TYPES_H

/********************************* INCLUDES ***********************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/***************************** MACRO DEFINITIONS ******************************/

#ifndef PUBLIC
#define PUBLIC
#endif /* PUBLIC */

#ifndef INTERNAL
#define INTERNAL
#endif /* INTERNAL */

#ifndef PRIVATE
#define PRIVATE static
#endif /* PRIVATE */

#ifndef RESULT_SUCCESS
#define RESULT_SUCCESS              (0)
#endif /* RESULT_SUCCESS */

#ifndef RESULT_FAIL
#define RESULT_FAIL                 (-1)
#endif /* RESULT_FAIL */

#undef INLINE
#undef ALWAYS_INLINE
#undef ASSEMBLY_FUNCTION

#ifdef UNIT_TEST
#define CONST_IN_PRODUCTION
#else
#define CONST_IN_PRODUCTION const
#endif

#if defined(WIN32)

    #define INLINE                          __inline
    #define ALWAYS_INLINE                   __forceinline
    #define ASSEMBLY_FUNCTION
    #define PACKED
    #define TYPEDEF_STRUCT_PACKED           typedef struct
    #define NO_INLINE

    #define PLACE_FUNCTION_AT(addr)         
    #define PLACE_IN_SECTION(region)        

    #define GET_REGION_BASE(__region)       (uint32_t)&GET_REGION_BASE_DEF(__region)
    #define GET_REGION_LENGTH(__region)     (uint32_t)&GET_REGION_LENGTH_DEF(__region)
    #define GET_REGION_LIMIT(__region)      (uint32_t)&GET_REGION_LIMIT_DEF(__region)

    #define GET_REGION_BASE_DEF(__region)    __region##_Base
    #define GET_REGION_LENGTH_DEF(__region)  __region##_Length
    #define GET_REGION_LIMIT_DEF(__region)   __region##_Limit
    #define ALIGN_SYM_OFFSET(_n)            __attribute__((aligned(_n)))

#elif defined(__ARMCC_VERSION)

#if (__ARMCC_VERSION >= 6010050)
    // Please see "Migrate ARM Compiler 5 to ARM Compiler 6" (http://www.keil.com/appnotes/files/apnt_298.pdf)

    #define INLINE                          //__inline__
    #define ALWAYS_INLINE                   //__attribute__((always_inline))
    #define PACKED                          __attribute__((packed))
    #define TYPEDEF_STRUCT_PACKED           typedef struct PACKED
    #define NO_INLINE                       __attribute__((noinline))
    #define CREATE_AT(addr)                 __attribute__((section(".ARM.__at_" ##addr)))
    #define PLACE_FUNCTION_AT(addr)         __attribute__((section(".ARM.__at_" ##addr)))
    #define PLACE_IN_SECTION(region)        __attribute__((section(region)))
    #define ALIGN_SYM_OFFSET(_n)            __attribute__((aligned(_n)))

    #define GET_REGION_BASE_DEF(__region)   Image$$##__region##$$Base
    #define GET_REGION_LENGTH_DEF(__region) Image$$##__region##$$Length
    #define GET_REGION_LIMIT_DEF(__region)  Image$$##__region##$$Limit

    /* Extern Linker Defined Regions Information */
    #define GET_REGION_BASE(__region)       (uint32_t)&GET_REGION_BASE_DEF(__region)
    #define GET_REGION_LENGTH(__region)     (uint32_t)&GET_REGION_LENGTH_DEF(__region)
    #define GET_REGION_LIMIT(__region)      (uint32_t)&GET_REGION_LIMIT_DEF(__region)

    #define ZAYA_KERNEL_STACK_BASE          (uint32_t)&GET_REGION_BASE_DEF(ARM_LIB_STACK)
    #define ZAYA_KERNEL_STACK_LIMIT         (uint32_t)&GET_REGION_LIMIT_DEF(ARM_LIB_STACK)
#else
    #define INLINE                          //__inline
    #define ALWAYS_INLINE                   //__forceinline
    #define PACKED                          __packed
    #define TYPEDEF_STRUCT_PACKED           PACKED typedef struct
    #define NO_INLINE                       __attribute__((noinline))
    #define CREATE_AT(addr)                 __attribute__((at(addr)))
    #define PLACE_FUNCTION_AT(addr)         __attribute__((section(".ARM.__at_" ##addr)))
    #define PLACE_IN_SECTION(region)        __attribute__((section(region)))
    #define ALIGN_SYM_OFFSET(_n)            __attribute__((aligned(_n)))

    #define GET_REGION_BASE_DEF(__region)   Image$$##__region##$$Base
    #define GET_REGION_LENGTH_DEF(__region) Image$$##__region##$$Length
    #define GET_REGION_LIMIT_DEF(__region)  Image$$##__region##$$Limit

    /* Extern Linker Defined Regions Information */
    #define GET_REGION_BASE(__region)       (uint32_t)&GET_REGION_BASE_DEF(__region)
    #define GET_REGION_LENGTH(__region)     (uint32_t)&GET_REGION_LENGTH_DEF(__region)
    #define GET_REGION_LIMIT(__region)      (uint32_t)&GET_REGION_LIMIT_DEF(__region)
    
    #define ZAYA_KERNEL_STACK_BASE          (uint32_t)&GET_REGION_BASE_DEF(ARM_LIB_STACK$$ZI)
    #define ZAYA_KERNEL_STACK_LIMIT         (uint32_t)&GET_REGION_LIMIT_DEF(ARM_LIB_STACK$$ZI)
#endif


#else /* GCC */

    /*
     * TODO [IMP] When we run splint for code analysis, splint does not analyze
     * for GCC (__GNUC__) as default so we can not use following compiler switch
     * to differentiate GCC defines
     *         #if defined ( __GNUC__ )
     *         #endif
     *
     *     So let's use else case to assume all other platforms (than WIN32, ARM) are
     *     GCC for now. But we need to specify GCC for splint explicitly.
     */
    #define INLINE                          __inline__

    /*
     * Unit tests show all warnings which break tests so let's use inline
     * instead of always inline for gcc builds
     */
    #define ALWAYS_INLINE                   __inline__
    #define PACKED                          __attribute__((packed))
    #define TYPEDEF_STRUCT_PACKED           typedef struct PACKED
    #define NO_INLINE
    #define PLACE_IN_SECTION(region)        __attribute__((section(region)))
    #define ALIGN_SYM_OFFSET(_n)            __attribute__((aligned(_n)))
    #define KEEP_SYMBOL                     __attribute__((used))

#ifdef UNIT_TEST

    #define GET_REGION_BASE_DEF(__region)   __region##_Base
    #define GET_REGION_LENGTH_DEF(__region) __region##_Length
    #define GET_REGION_LIMIT_DEF(__region)  __region##_Limit

    #define GET_REGION_BASE(__region)        __region##_Base
    #define GET_REGION_LENGTH(__region)      __region##_Length
    #define GET_REGION_LIMIT(__region)       __region##_Limit
#else
    #define GET_REGION_BASE_DEF(__region)    __region##_Base
    #define GET_REGION_LENGTH_DEF(__region)  __region##_Length
    #define GET_REGION_LIMIT_DEF(__region)   __region##_Limit

    #define GET_REGION_BASE(__region)        (uint32_t)&__region##_Base
    #define GET_REGION_LENGTH(__region)      (uint32_t)&__region##_Length
    #define GET_REGION_LIMIT(__region)       (uint32_t)&__region##_Limit
#endif

    #define ZAYA_KERNEL_STACK_BASE_DEF       GET_REGION_BASE_DEF(ZAYA_SECTION_KERNEL_STACK)
    #define ZAYA_KERNEL_STACK_LIMIT_DEF      GET_REGION_LIMIT_DEF(ZAYA_SECTION_KERNEL_STACK)
    
    #define ZAYA_KERNEL_STACK_BASE           (uint32_t)&GET_REGION_BASE_DEF(ZAYA_SECTION_KERNEL_STACK)
    #define ZAYA_KERNEL_STACK_LIMIT          (uint32_t)&GET_REGION_LIMIT_DEF(ZAYA_SECTION_KERNEL_STACK)
#endif

#ifndef ENDLESS_WHILE_LOOP
#define ENDLESS_WHILE_LOOP                  for (;;)
#endif

#ifndef SUPER_LOOP
#define SUPER_LOOP ENDLESS_WHILE_LOOP
#endif

#ifndef MATH_MIN
#define MATH_MIN(x, y)                      (((x) < (y)) ? (x) : (y))
#endif

#ifndef MATH_MAX
#define MATH_MAX(x, y)                      (((x) > (y)) ? (x) : (y))
#endif

#ifndef BITN
#define BITN(n)                             ( (uint32_t)1UL << ((uint32_t)(n)) )
#endif

#ifndef BITN_64
#define BITN_64(n)                          ( (uint64_t)1UL << ((uint32_t)(n)) )
#endif

#ifndef BITMASKN
#define BITMASKN(n)                         (BITN(n) - 1)
#endif

#ifndef BITMASKN_64
#define BITMASKN_64(n)                      (BITN_64(n) - 1)
#endif

#ifndef GET_BITFIELD
#define GET_BITFIELD(__word32bit, __bitWidth, __pos) \
        ((((uint32_t)__word32bit) >> (__pos)) & BITMASKN(__bitWidth))
#endif

#ifndef GET_BITFIELD_POS_MASK
#define GET_BITFIELD_POS_MASK(__word32bit, __field) \
        ((((uint32_t)__word32bit) & (__field##_MASK)) >> (__field##_POS))
#endif

/* Checks whether a pointer WORD aligned */
#define IS_WORD_ALIGNED(__ptr)              ((((uint32_t)__ptr) % sizeof(int)) == 0)

/***************************** TYPE DEFINITIONS *******************************/

typedef volatile uint32_t reg32_t;
typedef volatile uint64_t reg64_t;

/*************************** FUNCTION DEFINITIONS *****************************/

#endif    /* __Z_TYPES_H */
