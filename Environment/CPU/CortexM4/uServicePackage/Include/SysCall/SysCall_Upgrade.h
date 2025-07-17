/*******************************************************************************
 *
 * @file SysCall_Upgrade.h
 *
 * @brief System Call Interface to deploy (install/upgrade) ZAYA Entities.
 *        ZAYA secure deployments are protocol-agnostic and uses the same format
 *        for all ZAYA entities, such as
 *         - FW Upgrades (OS, Microcontainer, Microservice
 *         - Lifecycle commands (Provision, Factory Reset, Decommissioning)
 *
 *        Usage:
 *          > IMPORTANT: Only "ZAYA Lifecycle Microservices" signed by "Lifecycle"
 *            keys and has "Lifecycle" access right can call this API.
 *
 *          1. SysCall_UpgradeStart() must be called first to initiate an upgrade
 *
 *          -  SysCall_UpgradeInterfaceRead() & SysCall_UpgradeInterfaceWrite()
 *             can be used to read/write from/to the communication interface.
 *
 *          -  "Lifecycle" Microservice may need to parse the received packages in
 *             specific-format(e.g. XModem) and extract the raw upgrade data.
 *
 *          - Then, the raw data shall be collected; there are two ways to
 *            collect the upgrade package
 *              1. Storing inside Kernel Space:
 *                  - Use SysCall_UpgradeGetMaxCapacity() to get maximum
 *                    capacity of storage provided in Kernel Space.
 *                  - And, Use SysCall_UpgradeAppendFrame() to append the
 *                    received deployment parts.
 *              2. Or, storing inside the Lifecycle Microservice Itself
 *                 The Container can copy the received part anywhere inside the
 *                 container RAM.
 *
 *          - Once the download is done, call the SysCall_Upgrade() to perform
 *            the deployment using the package.
 *            IMPORTANT: Pass the package offset if the package collected in
 *            Container Memory. Otherwise, pass SYSCALL_UPGRADE_OS_STORAGE value
 *            as the offset value if the package collected under Kernel Space.
 *
 *            IMPORTANT: If the Upgrade API returns fail and cancelled,
 *            The Lifecycle Microservice still needs to call the
 *            SysCall_UpgradeEnd() function to exit from the "Upgrade" State if
 *            it is running in blocking mode(see SysCall_UpgradeStart())
 *
 * ******************************************************************************
 *
 * Copyright (c) 2020-2025 ZAYA Tech Ltd - All rights reserved.
 *
 * Unauthorised copying of this file, via any medium is strictly prohibited.
 *
 ******************************************************************************/
#ifndef __SYSCALL_UPGRADE_H
#define __SYSCALL_UPGRADE_H

/********************************* INCLUDES ***********************************/

#include "SysCall_Common.h"

#include "ztypes.h"

/***************************** MACRO DEFINITIONS ******************************/

/*
 * The offset definition for the upgrade packages collected in the Kernel Space.
 *
 * If the Lifecycle Microservice collects the upgrade material in Kernel Space
 * using SysCall_UpgradeAppendFrame(), then, it shall pass this definition to
 * SysCall_Upgrade() function as offset.
 */
#define SYSCALL_UPGRADE_OS_STORAGE              ((uint8_t*)0xFFFFFFFF)

/***************************** TYPE DEFINITIONS *******************************/

/******************************** VARIABLES ***********************************/

/*************************** FUNCTION DEFINITIONS *****************************/

/**
 * @brief Starts the Upgrade Process.
 * 
 * @param blocking Block the all other executions and run only the upgrade
 *                 process, until SysCall_UpgradeEnd() called.
 *
 * @retval SysStatus_Success Success
 * @retval SysStatus_InvalidOperation Execution is not authorisated.
 * @retval SysStatus_Upgrade_InvalidState Upgrade already started before.
 *
 */
SysStatus SysCall_UpgradeStart(bool blocking);

/**
 * @brief Ends the Upgrade Process
 *
 * @param success Inform the Kernel about the status.
 *
 * @retval SysStatus_Success Success
 * @retval SysStatus_InvalidOperation Execution is not authorisated.
 * @retval SysStatus_Upgrade_InvalidState Upgrade not started before.
 *
 */
SysStatus SysCall_UpgradeEnd(bool success);

/**
 * @brief Reads from the "Upgrade" Communication Interface provided by the
 *        custom target.
 *
 * @param buffer Buffer to store the read data
 * @param bufferSize Buffer maximum size
 * @param readSize Read Data Size
 *
 * @retval SysStatus_Success Success
 *
 */
SysStatus SysCall_UpgradeInterfaceRead(uint8_t* buffer, uint32_t bufferSize, uint32_t* readSize);

/**
 * @brief Writes to the "Upgrade" Communication Interface provided by the custom
 *        target.
 *
 * @param buffer to be written data
 * @param bufferSize Write Size
 *
 * @retval SysStatus_Success Success
 *
 */
SysStatus SysCall_UpgradeInterfaceWrite(uint8_t* buffer, uint32_t bufferSize);

/**
 * @brief Gets the capacity of the space provided by Kernel to collect the
 *        upgrade packages.
 *        If Lifecycle Microservice collect the upgrade data in the Microservice
 *        space, no need to use this function.
 *
 * @param fwUpgrageCapacity Capacity of the space provided by Kernel
 *
 * @retval SysStatus_Success Success
 *
 */
SysStatus SysCall_UpgradeGetMaxCapacity(uint32_t* fwUpgrageCapacity);

/**
 * @brief Appends RAW data to the internal RAM Upgrade Memory.
 *        If Lifecycle Microservice collect the upgrade data in the Microservice
 *        space, no need to use this function.
 *
 * @param data Data to append
 * @param size Data Size
 *
 * @retval SysStatus_Success Success
 * @retval SysStatus_Upgrade_InsufficientMemory if exceeds the internal memory
 *
 */
SysStatus SysCall_UpgradeAppendFrame(uint8_t* data, uint32_t size);

/**
 * @brief Performs the upgrade operation using the given package, once the data
 *        collection ended.
 *
 * @param package Upgrade Package offset. If the package is collected in Kernel
 *                space using SysCall_UpgradeAppendFrame(),
 *                SYSCALL_UPGRADE_OS_STORAGE value shall be passed.
 * @param length  The Upgrade Package Length. Ignored if the offset is
 *                SYSCALL_UPGRADE_OS_STORAGE.
 *
 * @retval SysStatus_Success if success
 *
 * @retval SysStatus_AuthenticationFail Package Authentication Fails
 *
 * @retval SysStatus_Upgrade_InvalidPackageOffset Invalid Package Offset
 * @retval SysStatus_Upgrade_InvalidSize Invalid Package Size
 * @retval SysStatus_Upgrade_InvalidEntityType Invalid Entity Type
 * @retval SysStatus_Upgrade_InvalidState Upgrade not started yet.
 * @retval SysStatus_Upgrade_InvalidIndex Invalid Entity Index.
 * @retval SysStatus_Upgrade_InvalidVersion Invalid Entity Version.
 *
 * @retval SysStatus_LifeCycle_InvalidState Invalid Lifecycle State.
 * @retval SysStatus_LifeCycle_InvalidState Invalid Lifecycle State.
 *
 * @retval SysStatus_Fail Low Level Error
 */
SysStatus SysCall_Upgrade(uint8_t* packageOffset, uint32_t length);

/**
 * Returns the product details.
 *
 * Optionally, the Provisioning data can have Vendor Product ID and Product
 * Class ID. These details would be use the differentiate the product groups
 * for a vendor.
 *
 * @param[out] productVendorID Product Vendor ID
 * @param[out] productID Product ID
 * @param[out] productClassID Product Class ID to define sub-groups under the product
 *
 * @retval SysStatus_Succes Success
 * @retval SysStatus_NotFound The Details not found
 *
 */
SysStatus SysCall_LifecycleGetProductDetails(uint32_t* productVendorID, uint32_t* productID, uint32_t* productClassID);

/**
 * Returns the lifecycle server details.
 *
 * Optionally, the Provisioning data can have these details for device to
 * connect to the lifecycle service.
 *
 * @param[out] url Lifecycle Server URL
 * @param urlSize URL Buffer Size to check if the url fits into
 * @param[out] port Lifecycle Server Port
 *
 * @retval SysStatus_Succes Success
 * @retval SysStatus_NotFound Details not found in the Root Params
 *
 */
SysStatus SysCall_LifecycleGetServerDetails(char* url, uint32_t urlSize, uint16_t* port);

#endif /* __SYSCALL_UPGRADE_H */
