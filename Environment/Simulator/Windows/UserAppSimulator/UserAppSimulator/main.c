/*
 * @file main.c
 *
 ******************************************************************************/

 /********************************* INCLUDES ***********************************/
#define LOG_INFO_ENABLED            1
#define LOG_WARNING_ENABLED         1
#define LOG_ERROR_ENABLED           1

#include "SysCall.h"

#include "us-tinyAES.h"

/***************************** MACRO DEFINITIONS ******************************/

/***************************** TYPE DEFINITIONS *******************************/

/**************************** FUNCTION PROTOTYPES *****************************/

/******************************** VARIABLES ***********************************/

/***************************** PRIVATE FUNCTIONS ******************************/

void tinyAESTest(void)
{
#define CHECK_AES_ERR(sysStatus, usStatus) \
                if (sysStatus != SysStatus_Success || usStatus != usTinyAESOp_Success) \
                { LOG_PRINTF(" > AES Test Failed. Line %d. Sys Status %d | usAES Status %d", __LINE__, sysStatus, usStatus); return; }
    uint8_t key[32];
    uint8_t iv[16];
    uint8_t plainData[32];
    uint8_t encData[32];
    uint8_t decData[32];

    uint32_t sessionID;
    usTinyAESStatus usStatus;
    uint32_t timeoutInMs = 10000;
    SysStatus retVal;

    memset(plainData, 0, sizeof plainData);
    memcpy(plainData, "ZAYA", sizeof("ZAYA"));

    memset(encData, 0, sizeof encData);
    memset(decData, 0, sizeof decData);

    retVal = us_TINYAES_Initialise();
    CHECK_AES_ERR(retVal, 0);

    /* Encrypt */
    {
        retVal = us_tinyAES_OpenSession(usTinyAESAlg_AES_CBC_256,
            key, sizeof(key),
            iv, sizeof(iv),
            timeoutInMs, &sessionID, &usStatus);
        CHECK_AES_ERR(retVal, usStatus);

        retVal = us_tinyAES_Encrypt(sessionID, plainData, sizeof(plainData), encData, sizeof(encData), timeoutInMs, &usStatus);
        CHECK_AES_ERR(retVal, usStatus);

        retVal = us_tinyAES_CloseSession(sessionID, timeoutInMs, &usStatus);
        CHECK_AES_ERR(retVal, usStatus);
    }

    /* Decrypt */
    {
        retVal = us_tinyAES_OpenSession(usTinyAESAlg_AES_CBC_256,
            key, sizeof(key),
            iv, sizeof(iv), timeoutInMs, &sessionID, &usStatus);
        CHECK_AES_ERR(retVal, usStatus);

        retVal = us_tinyAES_Decrypt(sessionID, encData, sizeof(encData), decData, sizeof(decData), timeoutInMs, &usStatus);
        CHECK_AES_ERR(retVal, usStatus);

        retVal = us_tinyAES_CloseSession(sessionID, timeoutInMs, &usStatus);
        CHECK_AES_ERR(retVal, usStatus);
    }

    LOG_PRINTF(" > tinyAES Encryption Test %s", memcmp(plainData, decData, sizeof(plainData)) == 0 ? "Success" : "Failed");
}

/***************************** PUBLIC FUNCTIONS *******************************/

int main(void)
{
    SysStatus retVal; (void)retVal;

    LOG_PRINTF(" > Container : Microservice Test User App");

    SYS_INITIALISE_IPC_MESSAGEBOX(retVal, 4);

    tinyAESTest();

    Sys_Exit();
}
