/*
 * @file uService.h
 *
 * @brief Microservice Interface
 *
 ******************************************************************************/

#ifndef __USERVICE_H
#define __USERVICE_H

/********************************* INCLUDES ***********************************/
#include "SysCall.h"

#include "uServiceConfig.h"

/***************************** MACRO DEFINITIONS ******************************/

/***************************** TYPE DEFINITIONS *******************************/

#define USERVICE_PACKAGE_HEADER_SIZE     (sizeof(uServicePackageHeader))

typedef struct
{
    /* uService Operation ID */
    int16_t operation;
    
    /* uService Operation Status */
    int16_t status;

    /* The package length */
    uint16_t length;
    
    uint16_t _reserved;
} uServicePackageHeader;

/* A template struct to show, header + payload offset */
typedef struct
{
    uServicePackageHeader header;
    
    uint8_t payload[1];
} uServicePackage;

/**************************** FUNCTION PROTOTYPES *****************************/

/******************************** VARIABLES ***********************************/

/***************************** PUBLIC FUNCTIONS *******************************/

static ALWAYS_INLINE void uService_PrintIntro(void)
{
    LOG_PRINTF(" > " USERVICE_NAME " Microservice. Version : " USERVICE_VERSION_STR "");
}

/**
 * Initialises a Microservice.
 * 
 * Must be called to initialise a microservice and get the handle of
 * the Microservice before using a Microservice
 *
 * @param name Microservice Name
 * @param[out] uServiceHandle Handle to access to Microservice
 *
 * @return SysStatus_Success Success
 * @return SysStatus_NotFound The Microservice is not found
 *
 */
SysStatus uService_Initialise(const char* name, uint32_t* uServiceHandle);

/**
 * Make a blocker request to a Microservice
 *
 *
 * @param uServiceHandle Microservice Handle to request
 * @param request Request to the Microservice
 * @param[out] response Response from the Microservice
 * @param timeoutInMs Timeout for the blocker response
 *
 * @retval Status of the Request/Response interaction with the Microservice.
 *         Microservice interaction can be successfull but the Microservice can
 *         return a specific error using uServicePackage::header::status field.
 *
 */
SysStatus uService_RequestBlocker(uint32_t uServiceHandle, uServicePackage* request, uServicePackage* response, uint32_t timeoutInMs);

#endif /* __USERVICE_H */
