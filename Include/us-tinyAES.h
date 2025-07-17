/*
 * @file uS_tinyAES.h
 *
 * @brief tinyAES Microservice Interface
 *
 ******************************************************************************/

#ifndef __US_TINYAES_H
#define __US_TINYAES_H

/********************************* INCLUDES ***********************************/
#include "SysCall.h"

/***************************** MACRO DEFINITIONS ******************************/

/***************************** TYPE DEFINITIONS *******************************/

typedef enum
{
    usTinyAESOp_Success = 0,
    usTinyAESOp_InvalidOperation,
    usTinyAESOp_NoSessionSlotAvailable,
    usTinyAESOp_NoSession,
    usTinyAESOp_InvalidSession,
    
    usTinyAESOp_Timeout,

    usTinyAESOp_UnsupportedOperation,
    
    usTinyAESOp_InvalidParam_UnsufficientSize,
    usTinyAESOp_InvalidParam_SizeExceedAllowed,
    
    usTinyAESOp_InvalidParam_Key,
} usTinyAESStatus;

typedef enum
{
    usTinyAESOp_None,
    usTinyAESOp_OpenSession,
    usTinyAESOp_CloseSession,
    usTinyAESOp_Encrypt,
    usTinyAESOp_Decrypt,
} usTinyAESOp;

typedef enum
{
    usTinyAESAlg_None = 0,
    usTinyAESAlg_AES_CBC_256 = 1,
} usTinyAESAlg;


/**************************** FUNCTION PROTOTYPES *****************************/

/******************************** VARIABLES ***********************************/

/***************************** PUBLIC FUNCTIONS *******************************/

/*
 * Initialise tiny AES Microservice
 *
 * @param none
 *
 * @retval SysStatus_Success Success
 * @retval SysStatus_NotFound The Microservice not found on the device.
 */
SysStatus us_TINYAES_Initialise(void);

/*
 * Opens an AES Session
 *
 * @param algorithm AES Algorithm See usTinyAESAlg
 * @param key AES Key
 * @param iv AES Initialisation Vector
 * @param timeoutInMs Timeout for the blocker operation
 * @param[out] sessionID Session Handle to use in AES operations during this session
 * @param[out] usStatus tinyAES Specific Status/Error
 *
 * @retval SysStatus_Success Success
 * @retval SysStatus_NotFound The Microservice not found on the device.
 */
SysStatus us_tinyAES_OpenSession(usTinyAESAlg algorithm,
                                        uint8_t* key, uint32_t keyLen,
                                        uint8_t* iv, uint32_t ivLen,
                                        uint32_t timeoutInMs,
                                        uint32_t* sessionID,
                                        usTinyAESStatus* usStatus);

/*
 * Closes an AES Session
 *
 * @param sessionID Session Handle to use in AES operations during this session
 * @param timeoutInMs Timeout for the blocker operation
 * @param[out] usStatus tinyAES Specific Status/Error
 *
 * @return SysStatus
 */
SysStatus us_tinyAES_CloseSession(uint32_t sessionID, uint32_t timeoutInMs, usTinyAESStatus* usStatus);

/*
 * AES Encryption
 *
 * @param sessionID AES Session ID
 * @param plainData Plaindata to encrypt
 * @param[out] cipherData Encrypted Output
 * @param timeoutInMs Timeout for the blocker operation
 * @param[out] usStatus tinyAES Specific Status/Error
 *
 * @return SysStatus
 */
SysStatus us_tinyAES_Encrypt(uint32_t sessionID, uint8_t* plainData, uint32_t plainDataLen, uint8_t* cipherData, uint32_t cipherDataLen, uint32_t timeoutInMs, usTinyAESStatus* usStatus);

/*
 * AES Decryption
 *
 * @param sessionID AES Session ID
 * @param plainData Plaindata to encrypt
 * @param[out] cipherData Encrypted Output
 * @param timeoutInMs Timeout for the blocker operation
 * @param[out] usStatus tinyAES Specific Status/Error
 *
 * @return SysStatus
 */
SysStatus us_tinyAES_Decrypt(uint32_t sessionID, uint8_t* cipherData, uint32_t cipherDataLen, uint8_t* plainData, uint32_t plainDataLen, uint32_t timeoutInMs, usTinyAESStatus* usStatus);

#endif /* __US_TINYAES_H */
