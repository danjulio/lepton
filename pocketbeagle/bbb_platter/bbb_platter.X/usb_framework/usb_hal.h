// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright 2015 Microchip Technology Inc. (www.microchip.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

To request to license the code under the MLA license (www.microchip.com/mla_license), 
please contact mla_licensing@microchip.com
*******************************************************************************/
//DOM-IGNORE-END

#ifndef _USB_HAL_H_
#define _USB_HAL_H_

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <stdint.h>

#if defined(__18CXX) || defined(__XC8)
    #if defined(_PIC14E)
        #include "usb_hal_pic16f1.h"
    #else
        #include "usb_hal_pic18.h"
    #endif
#elif defined(__C30__) || defined __XC16__
	#if defined(__dsPIC33E__) 
            #include "usb_hal_dspic33e.h"
	#elif defined(__PIC24E__)
            #include "usb_hal_pic24e.h"
	#else
            #include "usb_hal_pic24f.h"
	#endif
#else
    #error "Silicon Platform not defined"
#endif
    
#ifdef __cplusplus  // Provide C++ Compatability
    extern "C" {
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Constants
// *****************************************************************************
// *****************************************************************************
/* USBHALControlUsbResistors flags */
#define USB_HAL_PULL_UP_D_PLUS  0x80    // Pull D+ line high
#define USB_HAL_PULL_UP_D_MINUS 0x40    // Pull D- line high
#define USB_HAL_PULL_DN_D_PLUS  0x20    // Pull D+ line low
#define USB_HAL_PULL_DN_D_MINUS 0x10    // Pull D- line low
/*
 The following are defined for convenience:
 */
#define USB_HAL_DEV_CONN_FULL_SPD       USB_HAL_PULL_UP_D_PLUS
#define USB_HAL_DEV_CONN_LOW_SPD        USB_HAL_PULL_UP_D_MINUS
#define USB_HAL_DEV_DISCONNECT          0

/* USBHALControlBusPower Commands */
#define USB_VBUS_DISCHARGE  0       // Dicharge Vbus via resistor
#define USB_VBUS_CHARGE     1       // Charge Vbus via resistor
#define USB_VBUS_POWER_ON   3       // Supply power to Vbus
#define USB_VBUS_POWER_OFF  4       // Do not supply power to Vbus

/*
 USBHALGetLastError Error Bits.
 */
#define USBHAL_PID_ERR  0x00000001  // Packet ID Error
#define USBHAL_CRC5     0x00000002  // (Host) Token CRC5 check failed
#define USBHAL_HOST_EOF 0x00000002  // (Host) EOF not reached before next SOF
#define USBHAL_CRC16    0x00000004  // Data packet CRC error
#define USBHAL_DFN8     0x00000008  // Data field size not n*8 bits
#define USBHAL_BTO_ERR  0x00000010  // Bus turn-around timeout
#define USBHAL_DMA_ERR  0x00000020  // DMA error, unable to read/write memory
#define USBHAL_BTS_ERR  0x00000080  // Bit-stuffing error
#define USBHAL_XFER_ID  0x00000100  // Unable to identify transfer EP
#define USBHAL_NO_EP    0x00000200  // Invalid endpoint number
#define USBHAL_DMA_ERR2 0x00000400  // Error starting DMA transaction

/* Flags for USBHALSetEpConfiguration */
#if defined(__18CXX) || defined(__XC8)
    #define USB_HAL_TRANSMIT    0x0400  // Enable EP for transmitting data
    #define USB_HAL_RECEIVE     0x0200  // Enable EP for receiving data
    #define USB_HAL_HANDSHAKE   0x1000  // Enable EP to give ACK/NACK (non isoch)

    #define USB_HAL_NO_INC      0x0010  // Use for DMA to another device FIFO
    #define USB_HAL_HW_KEEPS    0x0020  // Cause HW to keep EP
#else
    #define USB_HAL_TRANSMIT    0x0400  // Enable EP for transmitting data
    #define USB_HAL_RECEIVE     0x0800  // Enable EP for receiving data
    #define USB_HAL_HANDSHAKE   0x0100  // Enable EP to give ACK/NACK (non isoch)

    /* Does not work, Fix this if needed. 3/1/07 - Bud
    #define USB_HAL_NO_INC      0x0010  // Use for DMA to another device FIFO
    #define USB_HAL_HW_KEEPS    0x0020  // Cause HW to keep EP
    */
    #define USB_HAL_ALLOW_HUB   0x8000  // (host only) Enable low-spd hub support
    #define USB_HAL_NO_RETRY    0x4000  // (host only) disable auto-retry on NACK
#endif
// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
// *****************************************************************************

/*************************************************************************
    Function:
        void USBHALSetBusAddress( uint8_t addr )
        
    Description:
        This routine sets the address of the system on the USB
        when acting as a peripheral device.
        
    Preconditions:
        1. USBHALInitialize must have been called to
           initialize the USB HAL.
        2. Endpoint zero (0) must be configured as appropriate
           by calls to USBHALSetEpConfiguration.
        3. The system must have been enumerated on the USB (as
           a device).
           
    Parameters:
        addr    Desired address of this device on the USB.
        
    Return Values:
        None
        
    Side Effect:
        The bus address has been set.
        
    Remarks:
        The address is assigned by the host and is received in
        a SET_ADDRESS setup request.
                  
 *************************************************************************/
/*
 This routine is implemented as a macro to a lower-level level routine.
 */

#define USBHALSetBusAddress OTGCORE_SetDeviceAddr

void USBHALSetBusAddress( uint8_t addr );


/*************************************************************************
    Function:
        void USBHALControlUsbResistors( uint8_t flags );
        
    Description:
        This routine enables or disables the USB pull-up or
        pull-down resistors as requested.
        
    Precondition:
        USBInitialize must have been called to initialize the
        USB SW stack.
        
    Parameters:
        flags - This is a bit-mapped flags value indicating
        which resistors to enable or disable (see below).

    Return Values:
        true if successful, false if not.
   
    Side Effects:
        The resistors are enabled as requested.
        
    Remarks:
        Used for USB peripheral control to connect to or
        disconnect from the bus.  Otherwise, used for OTG
        SRP/HNP and host support.
        
 *************************************************************************/

/*
 This routine is implemented as a macro to a lower-level level routine.
 */
 #if defined(__18CXX) || defined(__XC8)
    void USBHALControlUsbResistors( uint8_t flags );
 #else
    #define USBHALControlUsbResistors OTGCORE_ControlUsbResistors
    void USBHALControlUsbResistors( uint8_t flags );
#endif

/*
 MCHP: Define a method to check for SE0 & a way to send a reset (SE0).
 */


/*************************************************************************
    Function:
       bool USBHALSessionIsValid( void )
        
    Description:
        This routine determines if there is currently a valid
        USB session or not.
        
    Precondition:
        USBInitialize must have been called to initialize the
        USB SW stack.
        
    Parameters:
        None
        
    Return Values:
        true if the session is currently valid, false if not.
        
    Remarks:
        Only used for host and OTG support.

 *************************************************************************/

bool USBHALSessionIsValid( void );


/*************************************************************************
    Function:
        USBHALControlBusPower
        
    Description:
        This routine provides a bitmap of the most recent
        error conditions to occur.
        
    Precondition:
        USBInitialize must have been called to initialize the
        USB SW stack.
        
    Parameters:
        cmd - Identifies desired command (see below).
        
    Return Values:
        true if successful, false if not.
    
    Remarks:
        Only used for host and OTG support.
                  
 *************************************************************************/

bool USBHALControlBusPower( uint8_t cmd );

/*************************************************************************
    Function:
        unsigned long USBHALGetLastError( void )
        
    Description:
        This routine provides a bitmap of the most recent
        error conditions to occur.
        
    Precondition:
        USBInitialize must have been called to initialize the
        USB SW stack.
        
    Parameters:
        None
        
    Return Values:
        Bitmap indicating the most recent error condition(s).
        
    Side Effect:
        Error record is cleared.
        
    Remarks:
        Although record of the error state is cleared, nothing
        is done to fix the condition or recover from the
        error.  The client must take appropriate steps.
                  
 *************************************************************************/

unsigned long USBHALGetLastError( void );




/*************************************************************************
    Function:
        void USBHALHandleBusEvent ( void )
        
    Description:
        This routine checks the USB for any events that may
        have occurred and handles them appropriately.  It may
        be called directly to poll the USB and handle events
        or it may be called in response to an interrupt.

    Precondition:
        USBInitialize must have been called to initialize the
        USB SW stack.

    Parameters:
        None
        
    Return Values:
        None
        
    Side Effects:
        Depend on the event that may have occurred.
        
    Remarks:
        None
              
 *************************************************************************/

void USBHALHandleBusEvent ( void );


/*************************************************************************
    Function:
        bool USBHALStallPipe( TRANSFER_FLAGS pipe )
        
    Description:
        This routine stalls the given endpoint.

    Preconditions:
        USBHALInitialize must have been called to initialize
        the USB HAL.
        
    Parameters:
        pipe -  Uses the TRANSFER_FLAGS (see USBCommon.h) format to
                identify the endpoint and direction making up the
                pipe to stall.

        Note: Only ep_num and direction fields are required.
        
    Return Values:
        true if able to stall endpoint, false if not.
        
    Side Effects:
        The endpoint will stall if additional data transfer is
        attempted.
        Given endpoint has been stalled.
  
    Remarks:
        Starting another data transfer automatically
        "un-stalls" the endpoint.
        
 *************************************************************************/
/*
 Note: This function is implemented as a macro, calling directly into
 an internal HAL routine.
 */

#define USBHALStallPipe OTGCORE_StallPipe

bool USBHALStallPipe( TRANSFER_FLAGS pipe );


/******************************************************************************
    Function:
        bool USBHALUnstallPipe( TRANSFER_FLAGS pipe )
        
    Description:
        This routine clears the stall condition for the given pipe.
        
    PreCondition:
        Assumes OTGCORE_DeviceEnable has been called and
        OTGCORE_StallPipe has been called on the given pipe.
        
    Parameters:
        pipe -  Uses the TRANSFER_FLAGS (see USBCommon.h) format to
                identify the endpoint and direction making up the
                pipe to un-stall.
                
    Return Values:
        true if able to stall the pipe, false if not.
        
    Side Effects:
        The BSTALL and UOWN bits (and all other control bits) in
        the BDT for the given pipe will be cleared.

    Remarks:
        None

 *****************************************************************************/
/*
 Note: This function is implemented as a macro, calling directly into
 an internal HAL routine.
 */

#define USBHALUnstallPipe OTGCORE_UnstallPipe

bool USBHALUnstallPipe( TRANSFER_FLAGS pipe );


/**************************************************************************
    Function:
        USBHALGetStalledEndpoints
        
    Description:
        This function returns a 16-bit bit-mapped value with a
        bit set in the position of any endpoint that is stalled
        (i.e. if endpoint 0 is stalled then bit 0 is set, if
        endpoint 1 is stalled then bit 1 is set, etc.).     
    
    Preconditions:
        USBHALInitialize must have been called to initialize
        the USB HAL.

    Parameters:
        None

    Return Values:
        Bitmap of the currently stalled endpoints (see overview).

    Remarks:
        None
 *************************************************************************/

/*
 Note: This function is implemented as a macro, calling directly into
 a HAL routine.
 */

#define USBHALGetStalledEndpoints OTGCORE_GetStalledEndpoints

uint16_t USBHALGetStalledEndpoints ( void );


/******************************************************************************
    Function:
        bool USBHALFlushPipe( TRANSFER_FLAGS pipe )
        
    Description:
        This routine clears any pending transfers on the given
        pipe.
        
    Preconditions:
        USBHALInitialize must have been called to initialize the
        USB HAL.

        The caller must ensure that there is no possible way for
        hardware to be currently accessing the pipe (see notes).

    Parameters:
        pipe -  Uses the TRANSFER_FLAGS (see USBCommon.h) format to
                identify the endpoint and direction making up the
                pipe to flush.

    Return Values:
        true if successful, false if not.

    Side Effects:
        Transfer data for this pipe has been zeroed out.

    Remarks:
        This routine ignores the normal HW protocol for ownership
        of the pipe data and flushes the pipe, even if it is in
        process.  Thus, the caller must ensure that data transfer
        cannot be in process.  This situation occurs when a
        transfer has been terminated early by the host.
 *****************************************************************************/

bool USBHALFlushPipe( TRANSFER_FLAGS pipe );


/**************************************************************************
    Function:
        USBHALTransferData
        
    Description:
        This routine prepares to transfer data on the USB.
        If the system is in device mode, the actual transfer
        will not occur until the host performs an OUT request
        to the given endpoint.  If the system is in host mode,
        the transfer will not start until the token has been
        sent on the bus.

    Preconditions:
        1. USBHALInitialize must have been called to
           initialize the USB HAL.
        2. The endpoint through which the data will be
           transferred must be configured as appropriate by a
           call to USBHALSetEpConfiguration.
        3. The bus must have been enumerated (either as a host
           or device).  Except for EP0 transfers.

    Parameters:
        flags - Flags consists of the endpoint number OR'd
                with one or more flags indicating transfer
                direction and such (see "Data Transfer
                Macros" in USBCommon.h):
    
                  7 6 5 4 3 2 1 0 - Description
                  | | | | \_____/
                  | | | |    +----- Endpoint Number
                  | | | +---------- Short or zero-size packet
                  | | +------------ Data Toggle 0/1
                  | +-------------- Force Data Toggle
                  +---------------- 1=Transmit/0=Receive

        buffer      Address of the buffer to receive data.

        size        Number of uint8_ts of data to transfer.

    Return Values:
        true if the HAL was able to successfully start the
        data transfer, false if not.
        
    Side Effects:
        The HAL has prepared to transfer the data on the USB.

    Remarks:
        The HAL will continue the data transfer, keeping track
        of the buffer address, data remaining, and ping-pong
        buffer details internally when USBHALHandleBusEvent is
        called (either polled or in response to an interrupt).
        The caller will receive notification that the transfer
        has completed when the EVT_XFER event is passed into
        the USBHALBusEventCallout call-out function.
        
 *************************************************************************/

bool USBHALTransferData ( TRANSFER_FLAGS    flags,
                          void             *buffer,
                          unsigned int      size      );


/*************************************************************************
    Function:
        USBHALSetEpConfiguration
        
    Description:
        This routine allows the caller to configure various
        options (see "Flags for USBHALSetEpConfiguration",
        below) and set the behavior for the given endpoint.
        
    Precondition:
        USBHALInitialize has been called.
        
    Parameters:
        ep_num - Number of endpoint to configure, Must be
                 (ep_num >=0) && (ep_num <= USB_DEV_HIGHEST_EP_NUMBER)
                  max_pkt_size Size of largest packet this enpoint can
                  transfer.

        flags - Configuration flags (see below)

    Return Values:
        true if successful, false if not.
        
    Side Effects:
        The endpoint has been configured as desired.

    Remarks:
        The base address and size of the buffer is not set by
        this routine.  Those features of an endpoint are
        dynamically managed by the USBHALTransferData routine.
        An endpoint can be "de-configured" by setting its max
        packet size to 0.  When doing this, you should also
        set all flags to 0.
 *************************************************************************/

bool USBHALSetEpConfiguration ( uint8_t ep_num, uint16_t max_pkt_size, uint16_t flags );

/*************************************************************************
    Function:
        USBHALInitialize
        
    Description:
        This call performs the basic initialization of the USB
        HAL.  This routine must be called before any of the
        other HAL interface routines are called.
        
    Precondition:
        The system has been initialized.
        
    Parameters:
        flags -  Initialization flags
        
    Return Values:
        true if successful, false if not.

    Side Effects:
        The USB HAL SW stack was initialized.

    Remarks:
        This routine can be called to reset the controller.
        
 *************************************************************************/

bool USBHALInitialize ( unsigned long flags );


#ifdef __cplusplus  // Provide C++ Compatibility
    }
#endif

#endif  // _USB_HAL_H_
/*************************************************************************
 * EOF
 */

