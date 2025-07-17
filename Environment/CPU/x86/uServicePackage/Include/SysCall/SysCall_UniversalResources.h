/*******************************************************************************
 *
 * @file SysCall_UniversalResources.h
 *
 * @brief Universal Resources for System Call APIs
 *
 * ******************************************************************************
 *
 * Copyright (c) 2024-2025 ZAYA Tech Ltd - All rights reserved.
 *
 * Unauthorised copying of this file, via any medium is strictly prohibited.
 *
 ******************************************************************************/

#include <stdint.h>

#ifndef __SYSCALL_UNIVERSAL_RES_H
#define __SYSCALL_UNIVERSAL_RES_H

/* Universal Resource Versioning for Backward Compatibility Checks */
#define SYSCALL_UNIVERSAL_RESOURCES_VER_MAJOR       0
#define SYSCALL_UNIVERSAL_RESOURCES_VER_MINOR       9
#define SYSCALL_UNIVERSAL_RESOURCES_VER_MICRO       0

/********************************* INCLUDES ***********************************/

/************************ BUILT-IN UNIVERSAL RESOURCES ************************/

/* Universal Resource Flag (Bit 31) */
#define SYSCALL_UNIVERSAL_RESOURCE_MASK            (0x80000000UL)
/* Custom Universal Resource Flag (Bit 31 + 30) */
#define SYSCALL_UNIVERSAL_CUSTOM_RESOURCE_MASK     (0xC0000000UL)

/* Macro to check whether resource is a built-int universal resource */
#define SYSCALL_IS_UNIVERSAL_RESOURCE(__res)        \
            (((__res) & SYSCALL_UNIVERSAL_CUSTOM_RESOURCE_MASK) == SYSCALL_UNIVERSAL_RESOURCE_MASK)

/* Macro to check whether resource is a custom universal resource */
#define SYSCALL_IS_UNIVERSAL_CUSTOM_RESOURCE(__res) \
            (((__res) & SYSCALL_UNIVERSAL_CUSTOM_RESOURCE_MASK) == SYSCALL_UNIVERSAL_CUSTOM_RESOURCE_MASK)

/************************ BUILT-IN UNIVERSAL RESOURCES ************************/
#define SysResource_Uni_Test                        (SYSCALL_UNIVERSAL_RESOURCE_MASK)

/* Connectivity over Internet */
#define SysResource_Uni_Internet                    (SYSCALL_UNIVERSAL_RESOURCE_MASK | 0x100)
#define SysResource_Uni_Internet_TCP                (SYSCALL_UNIVERSAL_RESOURCE_MASK | 0x101)
#define SysResource_Uni_Internet_UDP                (SYSCALL_UNIVERSAL_RESOURCE_MASK | 0x102)

#define SysResource_Uni_Internet_TLS                (SYSCALL_UNIVERSAL_RESOURCE_MASK | 0x140)

/* Security&Crypto */
#define SysResource_Uni_Crypto                      (SYSCALL_UNIVERSAL_RESOURCE_MASK | 0x200)
#define SysResource_Uni_Crypto_PSA                  (SYSCALL_UNIVERSAL_RESOURCE_MASK | 0x201)

/***************************** MACRO DEFINITIONS ******************************/
#define SYS_UNIVERSAL_RES_TLS_INVALID_SOCKET        ((int32_t)-1)

/***************************** TYPE DEFINITIONS *******************************/

/*
 * Data Structure to share/pass credentials to a Universal Resource API
 */
typedef struct
{
    struct
    {
        #define SYS_UNIVERSAL_RES_CREDENTIAL_TYPE_NONE              0
        /*
         * Credential without content but with a handle, such as Key Secure Storage Handle.
         * Please use SYS_UNIVERSAL_RES_CREDENTIAL_TYPE_REFERENCE instead of HANDLE for
         * authentication
         */
        #define SYS_UNIVERSAL_RES_CREDENTIAL_TYPE_HANDLE            1
        /*
         * Credential in RAW/Plain Format. Not Secure. 
         * Please use SYS_UNIVERSAL_RES_CREDENTIAL_TYPE_REFERENCE
         */
        #define SYS_UNIVERSAL_RES_CREDENTIAL_TYPE_RAW               2
        /*
         * Credential as a reference. Includes the key handle with a
         * authentication token.
         */
        #define SYS_UNIVERSAL_RES_CREDENTIAL_TYPE_REFERENCE         3

        /* Credential Type, see  SYS_UNIVERSAL_RES_CREDENTIAL_TYPE_<> above */
        uint32_t type                   : 4;
    } flags;
    
    /* Credential context depending the flags.type */
    union
    {
        uint32_t handle;
        struct
        {
            /* RAW Key content */
            const uint8_t* key;
            uint32_t keyLen;
        } raw;
        struct
        {
            /* Authenticate token length : SHA-256 */
            #define SYS_UNI_RES_AUTH_TOKEN_LEN          32
            const uint8_t* authToken;
            uint32_t authTokenLength;
            /* Reference key handle  */
            uint32_t handle;
        } reference;
    } context;
} SysUniversalResourceCredentials;

/*************************** FUNCTION DEFINITIONS *****************************/

SysStatus Sys_Test_Get(uint32_t* testValue);

/* TLS Resources */
SysStatus Sys_TLSGetSocket(int32_t* socket, int32_t* status);
SysStatus Sys_TLSConnect(int32_t socket,
                         SysUniversalResourceCredentials* rootCert,
                         SysUniversalResourceCredentials* deviceCert,
                         SysUniversalResourceCredentials* privateKey,
                         const char* url, uint16_t urlLen, uint16_t port, uint32_t timeout,
                         int32_t* status);
SysStatus Sys_TLSClose(int32_t socket, int32_t* status);
SysStatus Sys_TLSWrite(int32_t socket, const uint8_t* buffer, uint32_t len, uint32_t* writtenLen, int32_t* status);
SysStatus Sys_TLSRead(int32_t socket, uint8_t* buffer, uint32_t len, uint32_t* readLen, int32_t* status);
SysStatus Sys_TLSPeriodic(void);

#endif    /* __SYSCALL_UNIVERSAL_RES_H */
