/*******************************************************************************
 *
 * @file SysCall_Common.h
 *
 * @brief Common Defines for Systems Calls
 *
 * ******************************************************************************
 *
 * Copyright (c) 2016-2025 ZAYA Tech Ltd - All rights reserved.
 *
 * Unauthorised copying of this file, via any medium is strictly prohibited.
 *
 ******************************************************************************/

#ifndef __SYSCALL_COMMON_H
#define __SYSCALL_COMMON_H

#include "ztypes.h"

/***************************** MACRO DEFINITIONS ******************************/

/*
 * The maximum Execution UID/Name length
 */
#define SYS_EXEC_UID_LENGTH             (16)
#define SYS_EXEC_NAME_MAX_LENGTH        (16)

/*
 * Base value for Custom System Events.
 * Needs to be derived from "SysEvent" type.
 *
 * Important First SysEvent_MaxSysEventCount event is allocated for Built-in Kernel Events. Therefore,
 * custom events must be started from SysEvent_MaxSysEventCount.
 */
#define SysEvent_CustomBase     ((SysEvent)SysEvent_MaxSysEventCount)

/*
 * Base value for Custom System Resources.
 *
 * Important First 4 resources are allocated for Kernel Resources.
 * Therefore, custom resources must be started from 4.
 */
#define SysResource_CustomBase  ((SysResource)4)

/***************************** TYPE DEFINITIONS *******************************/

/*
 * System Status and System Call Return Types
 */
typedef enum
{
    /* Success */
    SysStatus_Success = 0,

    /* Operation Failed */
    SysStatus_Fail,

     /*
      * Invalid operation
      *  - Unsupported operation
      */
    SysStatus_InvalidOperation,
    
    /*
     * Undefined Operation
     * The operation is enabled but not implemented by the porting layer
     */
    SysStatus_UndefinedOperation,
    
    /* Invalid Parameter passed to System Call */
    SysStatus_InvalidParameter,

    /*
     * Execution does not have access right for System Resource.
     * It occurs when an unprivileged execution tries to access secure
     * resource (e.g. Memory, peripheral)
     */
    SysStatus_InvalidAccess,
    
    /*
     * Either the length of the input/output is not expected length or smaller
     * than required size.
     */
    SysStatus_InvalidSize,

    /*
     * Unsupported event.
     * User execution requested an unsuppported event.
     */
    SysStatus_UnsupportedEvent,

    /* The module needs to be initialised before usage. */
    SysStatus_NotInitialised,
    
    /* The entity is not found */
    SysStatus_NotFound,
    
    /* The operation not supported or not enabled */
    SysStatus_NotSupported,

    /* Timeout occurred during the operation */
    SysStatus_Timeout,
    
    /*
     * Invalid/Unaligned container size during the initialisation.
     * Use SYS_INITIALISE_<MODULE>() macro to initialise the module.
     */
    SysStatus_InvalidContainerSize,

    /* Offset must be word-aligned */
    SysStatus_InvalidOffsetAlignment,

    /* The container are already initialised before so ignored. */
    SysStatus_AlreadyInitialised,

    /* All slots are occupied */
    SysStatus_NoSlotAvailable,

    /* A thread cannot sleep while in critical section */
    SysStatus_CannotSleepInCS,

    /* Authentication Failed */
    SysStatus_AuthenticationFail,

    /* Authentication Failed */
    SysStatus_InvalidCredentials,

    /* Wait Operation Not Performed because the resource is not available */
    SysStatus_WaitNotPerformed,

    /* The operation or request is not available */
    SysStatus_NotAvailable,

    /************************* IPC Status Values ******************************/

    /* Messagebox of Destination Execution is full */
    SysStatus_IPC_MessageBoxFull = 50,

    /* There is no message in the message box */
    SysStatus_IPC_MessageBoxEmpty,

    /*
     * Invalid destination
     * - Unexisting destination ID
     * - Own destination ID (not allowed to send a message itself)
     * - Destination Execution is terminated or unauthorized.
     */
    SysStatus_IPC_InvalidDestination,

    /* Destination Execution does not have a proper message box to receive messages */
    SysStatus_IPC_DestinationNotHaveMessageBox,

    /*
     * Invalid IPC Message
     * - Zero Length
     */
    SysStatus_IPC_InvalidMessage,

    /************************ Thread Status Values *****************************/

    /* Invalid Thread */
    SysStatus_Thread_Invalid = 70,

    /* Invalid Thread Priority */
    SysStatus_Thread_InvalidPriority,

    /* Thread Already Suspended */
    SysStatus_Thread_AlreadySuspended,

    /* Thread Already Running */
    SysStatus_Thread_AlreadyRunning,

    /* Thread is waiting for a thread, cannot yield to */
    SysStatus_Thread_WaitingCannotYield,

    /* Invalid Thread Stack Size */
    SysStatus_Thread_InvalidStackSize,

    /* Invalid Thread Stack Alignment */
    SysStatus_Thread_InvalidStackAlignment,

    /************************ Thread Sync Status Values ************************/

    /* Invalid Thread */
    SysStatus_ThreadSync_Invalid,

    /************************ Upgrade Status Values ************************/

    /* No Upgrade */
    SysStatus_Upgrade_NoUpgrade = 90,

    /*  Invalid Image Type */
    SysStatus_Upgrade_InvalidEntityType,

    /* Invalid Image Offset */
    SysStatus_Upgrade_InvalidPackageOffset,

    /* Upgrade is in invalid state */
    SysStatus_Upgrade_InvalidState,

    /* Insufficint Memory for Upgrade */
    SysStatus_Upgrade_InsufficientMemory,

    /* Invalid Image Version */
    SysStatus_Upgrade_InvalidVersion,

    /* Invalid Image Size */
    SysStatus_Upgrade_InvalidSize,

    /* Invalid Image Index */
    SysStatus_Upgrade_InvalidIndex,

    /* Invalid Image Encryption */
    SysStatus_Upgrade_InvalidImageStorageType,

    /* Invalid Image Encryption */
    SysStatus_Upgrade_InvalidEncryptionType,

    /* Upgrade Low Level Error */
    SysStatus_Upgrade_LowLevelError,

    /********************** Device Lifecycle Status Values *********************/

    /* No Upgrade */
    SysStatus_LifeCycle_InvalidState = 110,

    /* No Upgrade */
    SysStatus_LifeCycle_InvalidProvisioningData,

} SysStatus;

/*
 * System Event Types
 */
typedef enum
{
    /* IPC Message Event */
    SysEvent_IPCMessage = 1,

    /* IPC Message Event */
    SysEvent_Timer,

    /* Thread Sync (Semaphore, Mutex) Event */
    SysEvent_ThreadSync,

    /* Resource Sync Event */
    SysEvent_ThreadResourceSync,

    /*
     * Maximum System Event Number
     *
     * IMPORTANT! Do not change this value because Kernel does not accept System
     * Event number bigger than 31
     */
    SysEvent_MaxSysEventCount = 8
} SysEvent;

/*
 * SysCall Event Flags.
 * Kernel notifies an execution using this flags.
 *
 * In general, Flags are set by Interrupts so flags should be "volatile" to
 * avoid compiler optimisations.
 */
typedef volatile bool SyscallEventFlag;

/*************************** FUNCTION DEFINITIONS *****************************/

#endif    /* __SYSCALL_COMMON_H */
