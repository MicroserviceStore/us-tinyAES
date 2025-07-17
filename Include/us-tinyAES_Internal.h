/*
 * @file uS_tinyAES.h
 *
 * @brief tinyAES Microservice Internal Interface
 *
 ******************************************************************************/

#ifndef __US_TINYAES_INTERNAL_H
#define __US_TINYAES_INTERNAL_H

/********************************* INCLUDES ***********************************/

#include "tiny-AES.h"

#include "uService.h"

/***************************** MACRO DEFINITIONS ******************************/

#ifndef CFG_US_TINYAES_MAX_NUM_OF_SESSION
#define CFG_US_TINYAES_MAX_NUM_OF_SESSION       1
#endif /* CFG_US_TINYAES_MAX_NUM_OF_SESSION */

#ifndef CFG_US_TINYAES_RECEIVE_BUFFER_LEN
#define CFG_US_TINYAES_RECEIVE_BUFFER_LEN       (256)
#endif /* CFG_US_TINYAES_RECEIVE_BUFFER_LEN */

#define CFG_US_TINYAES_MAX_RECEIVE_LEN          (CFG_US_TINYAES_RECEIVE_BUFFER_LEN-1)

#ifndef SUPPORT_ONLY_CBC256
#define SUPPORT_ONLY_CBC256                     1
#endif

#if SUPPORT_ONLY_CBC256

    #define MAX_KEY_BITLEN                      (256) // CBC256
    #define MAX_KEY_SIZE                        (MAX_KEY_BITLEN / 8)
    #define MAX_IV_SIZE                         (16)
    #define MAX_BLOCK_SIZE                      (MAX_KEY_BITLEN / 8)

#endif

#define AES_PACKAGE_MAX_SIZE                    sizeof(usTinyAESRequestPackage)

/***************************** TYPE DEFINITIONS *******************************/

typedef struct
{
    uint32_t alg;
    uint8_t key[MAX_KEY_SIZE];
    uint32_t keyLen;
    uint8_t iv[MAX_IV_SIZE];
    uint32_t ivLen;
} usTinyAESPayloadOpenSession;

typedef struct
{
    uint32_t sessionID;
} usTinyAESPayloadCloseSession;

typedef struct
{
    uint32_t sessionID;
    uint32_t length;
    uint8_t buffer[MAX_BLOCK_SIZE];
} usTinyAESPayloadEncDec;

typedef struct
{
    uServicePackageHeader header;

    union
    {
        #define AES_PACKAGE_OPENSESSION_SIZE        (USERVICE_PACKAGE_HEADER_SIZE + sizeof(usTinyAESPayloadOpenSession))
        usTinyAESPayloadOpenSession openSession;
        
        #define AES_PACKAGE_CLOSESESSION_SIZE       (USERVICE_PACKAGE_HEADER_SIZE + sizeof(usTinyAESPayloadCloseSession))
        usTinyAESPayloadCloseSession closeSession;
        
        #define AES_PACKAGE_ENC_DEC_SIZE            (USERVICE_PACKAGE_HEADER_SIZE + sizeof(usTinyAESPayloadEncDec))
        usTinyAESPayloadEncDec encDec;
    } payload;
} usTinyAESRequestPackage;

typedef struct
{
    uServicePackageHeader header;

    union
    {
        struct
        {
            uint32_t sessionID;
        } openSession;
        
        struct
        {
            uint8_t buffer[MAX_BLOCK_SIZE];
            uint32_t length;
        } encDec;
    } payload;
} usTinyAESResponsePackage;

typedef struct
{
    #define AES_SESSION_ID_NOT_ACTIVE               0
    uint32_t id;
    
    usTinyAESAlg alg;
    
    uint32_t blockSize;
    
    struct AES_ctx ctx;
} AESSession;

/**************************** FUNCTION PROTOTYPES *****************************/

/******************************** VARIABLES ***********************************/

/***************************** PUBLIC FUNCTIONS *******************************/

#endif /* __US_TINYAES_INTERNAL_H */