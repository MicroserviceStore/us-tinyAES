/*******************************************************************************
 *
 * @file SysCall_IPC.h
 *
 * @brief Inter-Processor Communication Interface
 *
 * ******************************************************************************
 *
 * Copyright (c) 2016-2025 ZAYA Tech Ltd - All rights reserved.
 *
 * Unauthorised copying of this file, via any medium is strictly prohibited.
 *
 ******************************************************************************/

#ifndef __SYSCALL_IPC_H
#define __SYSCALL_IPC_H

/********************************* INCLUDES ***********************************/
#include "SysCall_Common.h"

#include "ztypes.h"

/***************************** MACRO DEFINITIONS ******************************/

/*
 * IPC Message Size.
 * Kernel needs 12 bytes for message.
 */
#define SYS_IPC_MESSAGE_SIZE                                (12)

/*
 * The maximum name length when IPC by name
 */
#define SYS_IPC_MAX_NAME_LENGTH                             (SYS_EXEC_NAME_MAX_LENGTH)

/* Macro to calculate required container size for Message Box */
#define SYS_IPC_MESSAGEBOX_CONTAINER_SIZE(_messageCount) \
            ((_messageCount) * SYS_IPC_MESSAGE_SIZE)

/**
 * Initialises IPC MessageBox
 * It can be called in a function (like Microcontainer init) which is
 * recommended to reduce the scope. In this way, you can avoid mistaken accesses.
 *
 * @param[out] retVal Return value after initialisation. Microcontainer needs to
 *                    check it before messagebox usage.
 *
 *                  - SysStatus_Success Success
 *                  - SysStatus_IPC_AlreadyInitialised A already initialised before.
 *                  - SysStatus_InvalidContainerSize Invalid container size.
 *                      Not aligned with SYS_IPC_MESSAGE_SIZE.
 *
 * @param _messageMessageCount Maximum accepted Message Count.
 */
#define SYS_INITIALISE_IPC_MESSAGEBOX(_retVal, _messageBoxCapacity) \
{ \
    static volatile uint32_t messageBoxContainer[SYS_IPC_MESSAGEBOX_CONTAINER_SIZE(_messageBoxCapacity)/sizeof(uint32_t)]; \
    _retVal = Sys_InitialiseMessageBox((volatile uint8_t*)messageBoxContainer, SYS_IPC_MESSAGEBOX_CONTAINER_SIZE(_messageBoxCapacity)); \
}

/*
 * IPC API can be used to interact with Kernel, and Kernel has a dedicated ID
 */
#define SYS_IPC_KERNELID                    (0xFF)

/***************************** TYPE DEFINITIONS *******************************/

/*************************** FUNCTION DEFINITIONS *****************************/

/**
 * Initialises Microcontainer IPC MessageBox
 *
 * If a Microcontainer needs to use messagebox, it needs to initialise a
 * messagebox first by providing a memory space.
 *
 * IMP : Please use SYS_INITIALISE_IPC_MESSAGEBOX() macro instead of calling this
 * function directly.
 *
 * Please see SYS_INITIALISE_IPC_MESSAGEBOX() for detailed description.
 *
 * @param messageBoxContainer Container (Memory Area) for message box.
 * @param containerSize Container Size
 *
 * @retval SysStatus_Success Success
 * @retval SysStatus_AlreadyInitialised MessageBox already initialised before.
 * @retval SysStatus_InvalidContainerSize Invalid container size.
 *          - Not aligned with SYS_IPC_MESSAGE_SIZE
 */
SysStatus Sys_InitialiseMessageBox(volatile uint8_t* messageBoxContainer, uint32_t containerSize);

/**
 * Sends a message to a Microcontainer.
 *
 * @param[in] destinationID Destination of message
 * @param[in] message to be sent message.
 * @param[in] len to be sent message length
 * @param[out] sequenceNo Message sequence no
 *
 * @retval SysStatus_Success Message Sent
 * @retval SysStatus_IPCInvalidDestination Invalid Destination ID.
 *         Destination does not exist or terminated by kernel before.
 * @retval SysStatus_IPC_DestinationNotHaveMessageBox The destination does not
 *         have a message box to receive messages.
 * @retval SysStatus_IPC_InvalidMessage Invalid Message. Zero Length or longer than allowed
 * @retval SysStatus_IPCMessageBoxFull Message queue of destination is full.
 *
 */
SysStatus Sys_SendMessage(uint8_t destinationID, uint8_t* message, uint32_t len, uint32_t* sequenceNo);
SysStatus Sys_SendMessageByName(char destinationName[SYS_IPC_MAX_NAME_LENGTH], uint8_t* message, uint32_t len, uint32_t* sequenceNo);

/**
 * Receives a message from Message Box.
 *
 * This function works iteratively. Use with Sys_IsMessageReceived() function to
 * get received length and receive a message from the message box with its
 * length.
 *
 * Microcontainer can receive message partially if receiver buffer size is
 * smaller than received message length.
 *
 * Example Code for partial read :
 *
 * ---------------------------------------------------------------------------
 *
 *      SysStatus retVal;
 *      bool messageReceived;
 *      uint32_t messageLen;
 *      uint8_t senderID;
 *      uint32_t sequenceNo;
 *
 *      retVal = Sys_IsMessageReceived(&messageReceived, &messageLen, &sequenceNo);
 *
 *      if (retVal == SysStatus_Success && messageReceived)
 *      {
 *          uint8_t buffer[BUFFER_SIZE];
 *          while (messageLen > 0)
 *          {
 *              retVal = Sys_ReceiveMessage(&senderID, buffer, BUFFER_SIZE, &sequenceNo);
 *              if (retVal == SysStatus_Success)
 *              {
 *                  // Process buffer
 *                  messageLen -= BUFFER_SIZE;
 *              }
 *          }
 * 
 *          // Release buffers with less "sequenceNo" if required
 *      }
 *
 * ---------------------------------------------------------------------------
 *
 * IMP : Sys_ReceiveMessage() function returns next message (if exists) after
 * previous message is finished but it does not mean the remaining part of
 * previous message and next message are mixed; Messages are reported seperately.
 * Each message has a sequence no as out parameter, and when the next message
 * started after the partial read, the sequence no changes.
 * SenderID is used to seperate message from different Microcontainer.
 *
 * @param[out] senderID Message Sender ID.
 * @param[out] message Received message.
 * @param[in] len Received message length
 * @param[out] sequenceNo Received message sequence no
 *
 * @retval SysStatus_Success Message is sent
 * @retval SysStatus_IPC_NotInitialised IPC Messagebox is not initialised before.
 * @retval SysStatus_IPC_MessageBoxEmpty MessageBox is empty.
 *
 */
SysStatus Sys_ReceiveMessage(uint8_t* senderID, uint8_t* message, uint32_t len, uint32_t* sequenceNo);
SysStatus Sys_ReceiveMessageByName(char senderIDName[SYS_IPC_MAX_NAME_LENGTH], uint8_t* message, uint32_t len, uint32_t* sequenceNo);

/**
 * Returns whether a new message is received.
 *
 * @param[out] isReceived Is a new nessage received.
 * @param[out] messageLen received message length
 * @param[out] sequenceNo message sequence no
 *
 * @retval SysStatus_Success Status read successfully
 * @retval SysStatus_NotInitialised MessageBox not initialised
 *
 */
SysStatus Sys_IsMessageReceived(bool* isReceived, uint32_t* messageLen, uint32_t* sequenceNo);

#endif    /* __SYSCALL_IPC_H */
