/*
 * @file
 *
 * @brief Microservice API static library source file. This source file/library
 *        runs in the caller execution.
 *
 ******************************************************************************/

/********************************* INCLUDES ***********************************/

#include "us-Template.h"
#include "us-Template_Internal.h"

#include "uService.h"

/***************************** MACRO DEFINITIONS ******************************/

#define US_OPTIONAL     0

/***************************** TYPE DEFINITIONS *******************************/
typedef struct
{
    struct
    {
        uint32_t initialised        : 1;
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

/***************************** PUBLIC FUNCTIONS *******************************/
#define INITIALISE_FUNCTIONEXPAND(a, b, c) a##b##c
#define INITIALISE_FUNCTION(name) INITIALISE_FUNCTIONEXPAND(us_, name, _Initialise)
SysStatus INITIALISE_FUNCTION(USERVICE_NAME_NONSTR)(void)
{
    /* Get the Microservice Index to interact with the Microservice */
    return uService_Initialise(usName, &userLibSettings.execIndex);
}

SysStatus us_Template_Sum(int32_t a, int32_t b, int32_t* result, usTemplateStatus* usStatus)
{
    const uint32_t timeoutInMs = 2000;
    SysStatus retVal;

    usTemplateResponsePackage response;
    usTemplateRequestPackage request;

    {
        request.header.operation = usTemplateOp_Sum;
        request.header.length = sizeof(request);
        request.payload.sum.a = a;
        request.payload.sum.b = b;
    };

    retVal = uService_RequestBlocker(userLibSettings.execIndex, (uServicePackage*)&request, (uServicePackage*)&response, timeoutInMs);
    *usStatus = response.header.status;

    if (*usStatus == usTemplate_Success)
    {
        *result = response.payload.sum.result;
    }

    return retVal;
}