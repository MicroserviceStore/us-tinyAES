/*
 * @file main.c
 *
 * @brief "tiny AES Microservice" Example
 *
 ******************************************************************************/

/********************************* INCLUDES ***********************************/

#include "SysCall.h"
#include "tiny-AES.h"

#include "uS-tinyAES.h"
#include "uS-tinyAES_Internal.h"

/***************************** MACRO DEFINITIONS ******************************/

/***************************** TYPE DEFINITIONS *******************************/

/**************************** FUNCTION PROTOTYPES *****************************/

PRIVATE void startAESService(void);

/******************************** VARIABLES ***********************************/

PRIVATE AESSession aesSession;

PRIVATE usTinyAESRequestPackage aesRequest;

/**************************** PRIVATE FUNCTIONS ******************************/

PRIVATE ALWAYS_INLINE void sendError(uint8_t receiverID, uint8_t operation, uint8_t status)
{
    uint32_t sequenceNo;
    (void)sequenceNo;
    usTinyAESResponsePackage response = 
    {
        .header.operation = operation,
        .header.status = status
    };
    
    (void)Sys_SendMessage(receiverID, (uint8_t*)&response, sizeof(response), &sequenceNo);
}

PRIVATE ALWAYS_INLINE uint32_t getSessionID(uint8_t receiverID)
{
    static uint32_t lastValue = 0x00A1B2C3;
    lastValue = lastValue * lastValue;
    
    #define SESSION_ID_RECEIVER_ID_MASK     ((uint32_t)0x000000FF)
    #define SESSION_ID_RANDOM_MASK          ((uint32_t)(~SESSION_ID_RECEIVER_ID_MASK))
    
    return (lastValue & SESSION_ID_RANDOM_MASK) | receiverID;
}

PRIVATE ALWAYS_INLINE bool isValidAlgorithm(usTinyAESRequestPackage* request, uint32_t* blockSize)
{
#if SUPPORT_ONLY_CBC256
    if (request->payload.openSession.alg != usTinyAESAlg_AES_CBC_256)
    {
        return false;
    }
    
    *blockSize = MAX_KEY_SIZE;
    
    return true;
#else
    #error "Unsupported yet"
#endif
}

PRIVATE ALWAYS_INLINE bool isValidKeyAndIV(usTinyAESRequestPackage* request)
{
#if SUPPORT_ONLY_CBC256    
    if (request->payload.openSession.keyLen != MAX_KEY_SIZE ||
        request->payload.openSession.ivLen != MAX_IV_SIZE)
    {
        return false;
    }
    return true;
#else
    #error "Unsupported yet"
#endif
}

PRIVATE ALWAYS_INLINE void processRequest(uint8_t receiverID, usTinyAESRequestPackage* request)
{
    usTinyAESResponsePackage response;

    switch (aesRequest.header.operation)
    {
        case usTinyAESOp_OpenSession:
            {
                uint32_t blockSize;

                if (aesSession.id != AES_SESSION_ID_NOT_ACTIVE)
                {
                    sendError(receiverID, request->header.operation, usTinyAESOp_NoSessionSlotAvailable);
                    return;
                }

                if (!isValidAlgorithm(request, &blockSize))
                {
                    sendError(receiverID, request->header.operation, usTinyAESOp_UnsupportedOperation);
                    return;
                }

                if (!isValidKeyAndIV(request))
                {
                    sendError(receiverID, request->header.operation, usTinyAESOp_InvalidParam_Key);
                    return;
                }

                /* Initialise the AES Context */
                AES_init_ctx_iv(&aesSession.ctx, request->payload.openSession.key, request->payload.openSession.iv);
                
                aesSession.id = getSessionID(receiverID);
                aesSession.blockSize = blockSize;

                /* Send the response */
                {
                    uint32_t sequenceNo;
                    (void)sequenceNo;

                    response.header.operation = request->header.operation;
                    response.header.status = usTinyAESOp_Success;
                    response.payload.openSession.sessionID = aesSession.id;
                    (void)Sys_SendMessage(receiverID, (uint8_t*)&response, sizeof(usTinyAESResponsePackage), &sequenceNo);
                }
            }
            break;
        case usTinyAESOp_CloseSession:
            {
                uint8_t ownerID;
                ownerID = request->payload.closeSession.sessionID & SESSION_ID_RECEIVER_ID_MASK;
                if (ownerID != receiverID || aesSession.id != request->payload.closeSession.sessionID)
                {
                    sendError(receiverID, request->header.operation, usTinyAESOp_InvalidSession);
                    return;
                }

                aesSession.id = AES_SESSION_ID_NOT_ACTIVE;
                
                /* Just return success even no session active */
                sendError(receiverID, request->header.operation, usTinyAESOp_Success);
                
            }
            break;
        case usTinyAESOp_Encrypt:
        case usTinyAESOp_Decrypt:
            {
                uint8_t ownerID;
                if (aesSession.id == AES_SESSION_ID_NOT_ACTIVE)
                {
                    sendError(receiverID, request->header.operation, usTinyAESOp_NoSession);
                    return;
                }
                
                ownerID = request->payload.encDec.sessionID & SESSION_ID_RECEIVER_ID_MASK;
                if (ownerID != receiverID || aesSession.id != request->payload.encDec.sessionID)
                {
                    sendError(receiverID, request->header.operation, usTinyAESOp_InvalidSession);
                    return;
                }

                if (request->payload.encDec.length > MAX_BLOCK_SIZE)
                {
                    sendError(receiverID, request->header.operation, usTinyAESOp_InvalidParam_SizeExceedAllowed);
                    return;
                }

                if (request->header.operation == usTinyAESOp_Encrypt)
                {
                    AES_CBC_encrypt_buffer(&aesSession.ctx, (uint8_t*)request->payload.encDec.buffer, request->payload.encDec.length);
                }
                else
                {
                    AES_CBC_decrypt_buffer(&aesSession.ctx, (uint8_t*)request->payload.encDec.buffer, request->payload.encDec.length);
                }
                
                response.payload.encDec.length = aesSession.blockSize;                
                memcpy(response.payload.encDec.buffer, request->payload.encDec.buffer, aesSession.blockSize);

                /* Send the response */
                {
                    uint32_t sequenceNo;
                    (void)sequenceNo;

                    response.header.operation = request->header.operation;
                    response.header.status = usTinyAESOp_Success;
                    (void)Sys_SendMessage(receiverID, (uint8_t*)&response, sizeof(response), &sequenceNo);
                }
            }
            break;
        default:
            sendError(receiverID, aesRequest.header.operation, usTinyAESOp_InvalidOperation);
            break;
    }
}

PRIVATE void startAESService(void)
{
    SysStatus retVal;
    bool dataReceived;
    uint32_t receivedLen;
    uint8_t senderID;
    bool responseStatus;
    uint32_t sequenceNo;
    (void)sequenceNo;

    while (1)
    {
        dataReceived = false;
        receivedLen = 0;
        responseStatus = usTinyAESOp_Success;

        (void)Sys_IsMessageReceived(&dataReceived, &receivedLen, &sequenceNo);       
        if (!dataReceived || receivedLen == 0)
        {
            /* Sleep until receive an IPC message */
            Sys_WaitForEvent(SysEvent_IPCMessage);
            
            continue;
        }

        if (receivedLen <= USERVICE_PACKAGE_HEADER_SIZE)
        {
            responseStatus = usTinyAESOp_InvalidParam_UnsufficientSize;
            LOG_PRINTF(" > Unsufficint Mandatory Received Length (%d)/(%d)",
                       receivedLen, USERVICE_PACKAGE_HEADER_SIZE);
        }
        
        if (responseStatus == usTinyAESOp_Success &&
            receivedLen > AES_PACKAGE_MAX_SIZE)
        {
            responseStatus = usTinyAESOp_InvalidParam_SizeExceedAllowed;

            LOG_PRINTF(" > Received Length (%d) exceed than allowed length(%d)",
                       receivedLen, AES_PACKAGE_MAX_SIZE);
            
            /* Let us just get the header, as not need for the payload */
            receivedLen = USERVICE_PACKAGE_HEADER_SIZE;
        }

        /* Get the message */
        (void)Sys_ReceiveMessage(&senderID, (uint8_t*)&aesRequest, receivedLen, &sequenceNo);

        /* Do not process the message if there was an error */
        if (responseStatus != usTinyAESOp_Success)
        {
            sendError(senderID, aesRequest.header.operation, responseStatus);
            continue;
        }

        /* Process the request */
        processRequest(senderID, &aesRequest);        
    }
}

/***************************** PUBLIC FUNCTIONS *******************************/

int main(void)
{
    SysStatus retVal;

    uService_PrintIntro();

#if CFG_US_TINYAES_MAX_NUM_OF_SESSION == 1
    SYS_INITIALISE_IPC_MESSAGEBOX(retVal, CFG_US_TINYAES_MAX_NUM_OF_SESSION);
    if (retVal != SysStatus_Success)
    {
        LOG_ERROR("IPC Messagebox Init Fails! %d", retVal);
        Sys_Exit();
    }
#else
    #error "Multiple request not supported yet!"
#endif

    startAESService();
    
    Sys_Exit();
}
