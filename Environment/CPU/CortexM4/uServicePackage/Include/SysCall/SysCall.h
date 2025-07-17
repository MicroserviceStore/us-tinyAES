/*******************************************************************************
 *
 * @file SysCall.h
 *
 * @brief System Call Interface for Executions.
 *
 * ******************************************************************************
 *
 * Copyright (c) 2016-2025 ZAYA Tech Ltd - All rights reserved.
 *
 * Unauthorised copying of this file, via any medium is strictly prohibited.
 *
 ******************************************************************************/

#ifndef __SYSCALL_H
#define __SYSCALL_H

/********************************* INCLUDES ***********************************/

#include "SysCall_IPC.h"
#include "SysCall_Timing.h"
#include "SysCall_Thread.h"
#include "SysCall_Log.h"
#include "SysCall_Common.h"

#include "ztypes.h"

/***************************** MACRO DEFINITIONS ******************************/

/***************************** TYPE DEFINITIONS *******************************/

/**
 * User ISR Type Definition
 *
 * @param none
 * @return none
 */
typedef void (*Sys_UserISR)(void);

/*************************** FUNCTION DEFINITIONS *****************************/

/**
 * Exits the running execution.
 * An execution can use this API to terminate itself
 *
 * @param none
 * @return none
 */
void Sys_Exit(void);

/**
 * Interrupts the running thread and switches execution to next ready thread.
 *
 * @param none
 * @return none
 */
void Sys_Yield(void);

/**
 * Suspends the running thread and yields execution to a specific thread.
 * The target thread must belong to the execution.
 *
 * IMPORTANT : It is not recommended to use this function as it forces ZAYA Secure
 * Scheduler to run a specific thread.
 * This function is disabled by default, but it can be enabled to run a Guest RTOS
 * in a Microcontainer like a Virtual Machine which needs its own task selection.
 *
 * @param threadID Target thread to yield
 * @param[out] yieldTimeInMs Yield Time In Milliseconds. This value can be used for
 *             tick increments for RTOSes that runs in tickless mode. It avoids making
 *             a new system call to get the actual time.
 * @retval SysStatus
 */
SysStatus Sys_SuspendAndYieldTo(uint32_t threadID, uint64_t* yieldTimeInMs);

/**
 * Puts the active thread into critical section. This is an
 * Microcontainer/Execution specific  Critical-Section API and affects
 * only the execution that calls this API; other threads of the execution
 * are suspended and dont preempt the thread in the critical section.
 *
 * This function does not suspend neither Kernel nor other executions.
 *
 * This API supports nested critical section by the same thread.
 *
 * @param none
 * @return none
 */
void Sys_EnterCriticalSection(void);

/**
 * Exists the active thread from the critical section. This is an
 * Microcontainer/Execution specific Critical-Section API.
 * Other threads of the Microcontainer are resumed.
 *
 * @param none
 * @return none
 */
void Sys_ExitCriticalSection(void);

/**
 * Sleeps a thread for timeout value.
 *
 * @param event Timeout (in milliseconds) value for sleep
 *
 * @retval SysStatus_Success
 * @retval SysStatus_CannotSleepInCS Not allowed in critical section
 * @retval SysStatus_InvalidParameter If the timeout is zero or bigger than one
 *         day duration (86,400,000ms)
 *
 */
SysStatus Sys_Sleep(uint32_t timeoutInMs);

/**
 * Blocks a thread until an event occurs.
 *
 * @param event waited Event
 *
 * @retval SysStatus_InvalidParameter Invalid Event Number
 * @retval SysStatus_CannotSleepInCS Not allowed in critical section
 * @retval SysStatus_UnsupportedEvent Unsupported event to wait
 *
 */
SysStatus Sys_WaitForEvent(SysEvent event);

/**
 * Returns whether an event pending.
 *
 * @param event Event
 * @param clearAfterRead Clear the pending flag
 * @param[out] pending Is the event pending
 *
 * @retval SysStatus_InvalidParameter Invalid Event Number
 * @retval SysStatus_UnsupportedEvent Unsupported event to wait
 *
 */
SysStatus Sys_IsPendingEvent(SysEvent event, bool clearAfterRead, bool* pending);

/**
 * Clears the event pending flag.
 *
 * @param event Event
 *
 * @retval SysStatus_InvalidParameter Invalid Event Number
 * @retval SysStatus_UnsupportedEvent Unsupported event to wait
 *
 */
SysStatus Sys_ClearPendingEvent(SysEvent event);

/**
 * Returns the system time since device startup in milliseconds
 *
 * @param none
 *
 * @retval System Time In Ms
 *
 */
uint64_t Sys_GetTimeInMs(void);

/**
 * Returns the EPOC time in seconds since 01/01/1970
 *
 * @param none
 *
 * @retval EPOC Time
 *
 */
uint32_t Sys_GetEPOCTime(void);

/**
 * Initialises the logical WDT for the Microcontainer which resets only the
 * owner Microcontainer; WDT timeout resets neither the device nor other
 * executables (Kernel, Other Microcontainers).
 *
 * Does not start the WDT. Sys_WDTEnable() function must be called to start the
 * WDT.
 *
 * @param timeoutInMs WDT Timeout in milliseconds
 *
 * @retval SysStatus_InvalidParameter Small Amount of Timeout
 * @retval SysStatus_Success WDT initialised
 *
 */
SysStatus Sys_WDTInit(uint32_t timeoutInMs);

/**
 * Enables/Starts the Microcontainer WDT.
 *
 * WDT must be initialised before enable using WDT_Init()
 *
 * @retval SysStatus_NotInitialised WDT is not initialised using WDT_Init() function.
 * @retval SysStatus_Succes Success.
 *
 */
SysStatus Sys_WDTEnable(void);

/**
 * Disables/Stops the Microcontainer WDT
 *
 * @retval SysStatus_NotInitialised WDT is not initialised using WDT_Init() function.
 * @retval SysStatus_Succes Success.
 *
 */
SysStatus Sys_WDTDisable(void);

/**
 * Feeds/Resets the Microcontainer WDT.
 *
 * @retval SysStatus_Succes Success.
 *
 */
SysStatus Sys_WDTFeed(void);

/**
 * Returns the Device Unique ID (Max 128 Bit)
 *
 * @param[out] uID Device Unique ID MSB Words
 * @param[out] uIDLength Device Unique ID Length. UID Length may differ on an
 *                       environment
 *
 * @retval SysStatus_Succes Success.
 *
 */
SysStatus Sys_GetDeviceUID(uint32_t uID[], uint32_t* uIDLength);

/**
 * Returns the Microcontainer Storage Size
 *
 * @param[out] storageSize Storage Size
 *
 * @retval SysStatus_Succes Success.
 *
 */
SysStatus Sys_StorageGetSize(uint32_t* storageSize);

/**
 * Read data from the Microcontainer Storage
 *
 * @param offset Storage Offset to read
 * @param length to be read length
 * @param[out] buffer Buffer to read into
 *
 * @retval SysStatus_Succes Success.
 * @retval SysStatus_InvalidParameter Invalid Parameter (out of range)
 *
 */
SysStatus Sys_StorageRead(uint32_t offset, uint32_t length, uint8_t* buffer);

/**
 * Write data to the Microcontainer Storage
 *
 * @param offset Storage Offset to write
 * @param length to be written length
 * @param buffer Buffer to write
 *
 * @retval SysStatus_Succes Success.
 * @retval SysStatus_InvalidParameter Invalid Parameter (out of range)
 *
 */
SysStatus Sys_StorageWrite(uint32_t offset, uint32_t length, uint8_t* buffer);

/**
 * Clears/Erases the whole Microcontainer Storage
 *
 * @retval SysStatus_Succes Success.
 *
 */
SysStatus Sys_StorageClear(void);

/**
 * Returns a root params items from the Vendor Root Params.
 * - It does not return the root credentials, returns only custom vendor items
 * - This API is not available for Microservices, only user application
 *   Microcontainers can use.
 *
 * @param tag Root Param Item Tag
 * @param buffer Buffer to collect the item content
 * @param bufferSize Buffer max size
 * @param[out] itemLength Custom Vendor Item Length
 *
 * @retval SysStatus_Succes Success.
 * @retval SysStatus_InvalidParameter if root credential asked or called by a 
 *         Microservice
 *
 */
SysStatus Sys_GetRootParamItem(uint32_t tag, uint8_t* buffer, uint32_t bufferSize, uint32_t* itemLength);

/**
 * Sets the Stack Memory for the ISR (Interrupt Service Routine) handled
 * in Containers.
 * 
 * All the registered ISRs using Sys_SetHWISR will be using this isolated stack.
 *
 * @param stackBase ISR Stack Base Offset
 * @param stackSize ISR Stack Size
 *
 * @retval SysStatus_Succes Success
 * @retval SysStatus_Thread_InvalidStackAlignment if stack offset + size not
 *         aligned with the architectural limitations.
 *
 */
SysStatus Sys_SetISRStack(uint8_t* stackBase, uint32_t stackSize);

/**
 * Sets a HW ISR (Interrupt Service Routine) to handle a HW interrupt in the
 * container. (User-Space Driver)
 *
 * @param irqNo HW Interrupt Number to handle
 * @param userISR to be called callback when the HW Interrupt occurred
 *
 * @retval SysStatus_Succes Success
 * @retval SysStatus_Fail Invalid parameter or Container HW ISR Handling not
 *         supported/enabled.
 *
 */
SysStatus Sys_SetHWISR(int32_t irqNo, Sys_UserISR userISR);

/**
 * Waits a resource until it gets available in blocker mode and then locks (mutex)
 * the resource.
 * 
 * The resource must be assigned to the Execution to wait a resource.
 *
 * @param resourceID ID of Resource to obtain the access
 *
 * @retval SysStatus_Succes Success
 * @retval SysStatus_InvalidAccess The execution does not have access on the
 *         resource
 * @retval SysStatus_InvalidOperation Invalid Resource ID
 *
 */
SysStatus Sys_WaitResource(uint32_t resourceID);

/**
 * Try to lock a resource in non-blocking mode, if it is available, it locks,
 * otherwise, returns with "SysStatus_WaitNotPerformed" value.
 * 
 * The resource must be assigned to the Execution to wait a resource.
 *
 * @param resourceID ID of Resource to obtain the access
 *
 * @retval SysStatus_Succes Success
 * @retval SysStatus_WaitNotPerformed The resource is not available
 * @retval SysStatus_InvalidAccess The execution does not have access on the
 *         resource
 * @retval SysStatus_InvalidOperation Invalid Resource ID
 *
 */
SysStatus Sys_TryWaitResource(uint32_t resourceID);

/**
 * Releases a resource.
 * 
 * The resource must be assigned to the Execution to wait a resource.
 *
 * @param resourceID ID of Resource to obtain the access
 *
 * @retval SysStatus_Succes Success
 * @retval SysStatus_InvalidAccess The execution does not have access on the
 *         resource
 * @retval SysStatus_InvalidOperation Invalid Resource ID
 * @retval SysStatus_ThreadSync_Invalid The resource is locked by another thread
 *
 */
SysStatus Sys_ReleaseResource(uint32_t resourceID);

/**
 * Get the Execution Index using its name.
 * Each Execution/Container has an index  this index can be used
 * for helper purposes such as faster IPC communication,
 * using system-wide index  instead of name search.
 *
 * @param name Execution Name
 * @param[out] execIndex Execution Index
 *
 * @retval SysStatus_Succes Success
 * @retval SysStatus_InvalidSize The Name length may exceed the max length
 *                               (likely no terminator char)
 * @retval SysStatus_NotFound No execution exist for the given name
 *
 */
SysStatus Sys_GetExecutionIndexByName(const char name[SYS_EXEC_NAME_MAX_LENGTH], uint32_t* execIndex);

/**
 * Get the Execution Index using its Unique-ID(UID).
 * Each Execution/Container has an index this index can be used
 * for helper purposes such as faster IPC communication,
 * using system-wide index  instead of UID search.
 *
 * @param uid Execution Unique ID
 * @param[out] execIndex Execution Index
 *
 * @retval SysStatus_Succes Success
 * @retval SysStatus_NotFound No execution exist for the given name
 *
 */
SysStatus Sys_GetExecutionIndexByUID(const uint8_t uid[SYS_EXEC_UID_LENGTH], uint32_t* execIndex);

/**
 * Get the Execution Index using its universal integer value.
 * Each Execution/Container has an index this index can be used
 * for helper purposes such as faster IPC communication,
 * using system-wide index  instead of UID search.
 *
 * @param value Universal Integer Value
 * @param[out] execIndex Execution Index
 *
 * @retval SysStatus_Succes Success
 * @retval SysStatus_NotFound No execution exist for the given name
 *
 */
SysStatus Sys_GetExecutionIndexByValue(uint32_t value, uint32_t* execIndex);

#endif    /* __SYSCALL_H */
