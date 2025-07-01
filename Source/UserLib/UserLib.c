/*
 * @file
 *
 * @brief Microservice API static library source file. This source file/library
 *        runs in the caller execution.
 *
 ******************************************************************************/

/********************************* INCLUDES ***********************************/

#include USERVICE_PUBLIC_HEADER
#include USERVICE_INTERNAL_HEADER

#include "uService.h"

/***************************** MACRO DEFINITIONS ******************************/

/***************************** TYPE DEFINITIONS *******************************/
typedef struct
{
    struct
    {
        //uint32_t initialised        : 1;
    } flags;

    /*
     * The "Execution Index" is a system wide enumaration by the Microservice Runtime
     * to interact with the Microservice.
     */
    uint32_t execIndex;
} uS_UserLibSettings;

/**************************** FUNCTION PROTOTYPES *****************************/

/******************************** VARIABLES ***********************************/
PRIVATE uS_UserLibSettings userLibSettings;

PRIVATE const char usName[SYS_EXEC_NAME_MAX_LENGTH] = USERVICE_NAME;

/***************************** PRIVATE FUNCTIONS *******************************/

static SysStatus encdec(bool enc, uint32_t sessionID, uint8_t* input, uint32_t inputLen, uint8_t* output, uint32_t outputLen, uint32_t timeoutInMs, usTinyAESStatus* usStatus)
{
    SysStatus retVal;
    usTinyAESRequestPackage request;
    usTinyAESResponsePackage response;

    {
        request.header.operation = enc ? usTinyAESOp_Encrypt : usTinyAESOp_Decrypt;
        request.header.length = AES_PACKAGE_ENC_DEC_SIZE;
        request.payload.encDec.sessionID = sessionID;
        request.payload.encDec.length = inputLen;
        
        memcpy(request.payload.encDec.buffer, input, inputLen);
    }
    
    retVal = uService_RequestBlocker(userLibSettings.execIndex, (uServicePackage*)&request, (uServicePackage*)&response, timeoutInMs);
    *usStatus = response.header.status;
    
    if (retVal == SysStatus_Success && response.header.status == usTinyAESOp_Success)
    {
        memcpy(output, response.payload.encDec.buffer, response.payload.encDec.length);
    }
    
    return SysStatus_Success;
}

/***************************** PUBLIC FUNCTIONS *******************************/
#define INITIALISE_FUNCTIONEXPAND(a, b, c) a##b##c
#define INITIALISE_FUNCTION(name) INITIALISE_FUNCTIONEXPAND(us_, name, _Initialise)

SysStatus INITIALISE_FUNCTION(USERVICE_NAME_NONSTR)(void)
{
    /* Get the Microservice Index to interact with the Microservice */
    return uService_Initialise(usName, &userLibSettings.execIndex);
}

SysStatus us_tinyAES_OpenSession(usTinyAESAlg algorithm,
                                        uint8_t* key, uint32_t keyLen,
                                        uint8_t* iv, uint32_t ivLen,
                                        uint32_t timeoutInMs,
                                        uint32_t* sessionID,
                                        usTinyAESStatus* usStatus)
{
    SysStatus retVal;
    usTinyAESRequestPackage request;
    usTinyAESResponsePackage response;

    *sessionID = AES_SESSION_ID_NOT_ACTIVE;

    {
        request.header.operation = usTinyAESOp_OpenSession;
        request.header.length = AES_PACKAGE_OPENSESSION_SIZE;
        request.payload.openSession.alg = algorithm;
        request.payload.openSession.keyLen = keyLen;
        request.payload.openSession.ivLen = ivLen;

        /* In case of any memory violation, ZAYA would catch */
        memcpy(request.payload.openSession.key, key, keyLen);
        memcpy(request.payload.openSession.iv, iv, ivLen);
    }

    retVal = uService_RequestBlocker(userLibSettings.execIndex, (uServicePackage*)&request, (uServicePackage*)&response, timeoutInMs);
    *usStatus = response.header.status;

    if (retVal == SysStatus_Success && response.header.status == usTinyAESOp_Success)
    {
        *sessionID = response.payload.openSession.sessionID;
    }

    return retVal;
}

SysStatus us_tinyAES_CloseSession(uint32_t sessionID, uint32_t timeoutInMs, usTinyAESStatus* usStatus)
{
    SysStatus retVal;
    usTinyAESRequestPackage request;
    usTinyAESResponsePackage response;

    {
        request.header.operation = usTinyAESOp_CloseSession;
        request.header.length = AES_PACKAGE_CLOSESESSION_SIZE;
        request.payload.closeSession.sessionID = sessionID;
    }

    retVal = uService_RequestBlocker(userLibSettings.execIndex, (uServicePackage*)&request, (uServicePackage*)&response, timeoutInMs);
    *usStatus = response.header.status;

    return retVal;
}

SysStatus us_tinyAES_Encrypt(uint32_t sessionID, uint8_t* plainData, uint32_t plainDataLen, uint8_t* cipherData, uint32_t cipherDataLen, uint32_t timeoutInMs, usTinyAESStatus* usStatus)
{
    return encdec(true, sessionID, plainData, plainDataLen, cipherData, cipherDataLen, timeoutInMs, usStatus);
}

SysStatus us_tinyAES_Decrypt(uint32_t sessionID, uint8_t* cipherData, uint32_t cipherDataLen, uint8_t* plainData, uint32_t plainDataLen, uint32_t timeoutInMs, usTinyAESStatus* usStatus)
{
    return encdec(false, sessionID, cipherData, cipherDataLen, plainData, plainDataLen, timeoutInMs, usStatus);
}
