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

#ifndef CDC_H
#define CDC_H

/** I N C L U D E S **********************************************************/
#include "usb.h"
#include "usb_config.h"

/** D E F I N I T I O N S ****************************************************/

/* Class-Specific Requests */
#define SEND_ENCAPSULATED_COMMAND   0x00
#define GET_ENCAPSULATED_RESPONSE   0x01
#define SET_COMM_FEATURE            0x02
#define GET_COMM_FEATURE            0x03
#define CLEAR_COMM_FEATURE          0x04
#define SET_LINE_CODING             0x20
#define GET_LINE_CODING             0x21
#define SET_CONTROL_LINE_STATE      0x22
#define SEND_BREAK                  0x23

/* Notifications *
 * Note: Notifications are polled over
 * Communication Interface (Interrupt Endpoint)
 */
#define NETWORK_CONNECTION          0x00
#define RESPONSE_AVAILABLE          0x01
#define SERIAL_STATE                0x20


/* Device Class Code */
#define CDC_DEVICE                  0x02

/* Communication Interface Class Code */
#define COMM_INTF                   0x02

/* Communication Interface Class SubClass Codes */
#define ABSTRACT_CONTROL_MODEL      0x02

/* Communication Interface Class Control Protocol Codes */
#define V25TER                      0x01    // Common AT commands ("Hayes(TM)")


/* Data Interface Class Codes */
#define DATA_INTF                   0x0A

/* Data Interface Class Protocol Codes */
#define NO_PROTOCOL                 0x00    // No class specific protocol required


/* Communication Feature Selector Codes */
#define ABSTRACT_STATE              0x01
#define COUNTRY_SETTING             0x02

/* Functional Descriptors */
/* Type Values for the bDscType Field */
#define CS_INTERFACE                0x24
#define CS_ENDPOINT                 0x25

/* bDscSubType in Functional Descriptors */
#define DSC_FN_HEADER               0x00
#define DSC_FN_CALL_MGT             0x01
#define DSC_FN_ACM                  0x02    // ACM - Abstract Control Management
#define DSC_FN_DLM                  0x03    // DLM - Direct Line Managment
#define DSC_FN_TELEPHONE_RINGER     0x04
#define DSC_FN_RPT_CAPABILITIES     0x05
#define DSC_FN_UNION                0x06
#define DSC_FN_COUNTRY_SELECTION    0x07
#define DSC_FN_TEL_OP_MODES         0x08
#define DSC_FN_USB_TERMINAL         0x09
/* more.... see Table 25 in USB CDC Specification 1.1 */

/* CDC Bulk IN transfer states */
#define CDC_TX_READY                0
#define CDC_TX_BUSY                 1
#define CDC_TX_BUSY_ZLP             2       // ZLP: Zero Length Packet
#define CDC_TX_COMPLETING           3

#if defined(USB_CDC_SET_LINE_CODING_HANDLER) 
    #define LINE_CODING_TARGET &cdc_notice.SetLineCoding._byte[0]
    #define LINE_CODING_PFUNC &USB_CDC_SET_LINE_CODING_HANDLER
#else
    #define LINE_CODING_TARGET &line_coding._byte[0]
    #define LINE_CODING_PFUNC NULL
#endif

#if defined(USB_CDC_SUPPORT_HARDWARE_FLOW_CONTROL)
    #define CONFIGURE_RTS(a) UART_RTS = a;
#else
    #define CONFIGURE_RTS(a)
#endif

#if defined(USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D3)
    #error This option is not currently supported.
#else
    #define USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D3_VAL 0x00
#endif

#if defined(USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D2)
    #define USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D2_VAL 0x04
#else
    #define USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D2_VAL 0x00
#endif

#if defined(USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D1)
    #define USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D1_VAL 0x02
#else
    #define USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D1_VAL 0x00
#endif

#if defined(USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D0)
    #error This option is not currently supported.
#else
    #define USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D0_VAL 0x00
#endif

#define USB_CDC_ACM_FN_DSC_VAL  \
    USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D3_VAL |\
    USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D2_VAL |\
    USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D1_VAL |\
    USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D0_VAL

/******************************************************************************
    Function:
        void CDCSetBaudRate(uint32_t baudRate)
        
    Summary:
        This macro is used set the baud rate reported back to the host during
        a get line coding request. (optional)

    Description:
        This macro is used set the baud rate reported back to the host during
        a get line coding request.

        Typical Usage:
        <code>
            CDCSetBaudRate(19200);
        </code>

        This function is optional for CDC devices that do not actually convert
        the USB traffic to a hardware UART.
        
    PreCondition:
        None
        
    Parameters:
        uint32_t baudRate - The desired baud rate
        
    Return Values:
        None
        
    Remarks:
        None
  
 *****************************************************************************/
#define CDCSetBaudRate(baudRate) {line_coding.dwDTERate=baudRate;}

/******************************************************************************
    Function:
        void CDCSetCharacterFormat(uint8_t charFormat)
        
    Summary:
        This macro is used manually set the character format reported back to 
        the host during a get line coding request. (optional)

    Description:
        This macro is used manually set the character format reported back to 
        the host during a get line coding request.

        Typical Usage:
        <code>
            CDCSetCharacterFormat(NUM_STOP_BITS_1);
        </code>
        
        This function is optional for CDC devices that do not actually convert
        the USB traffic to a hardware UART.

    PreCondition:
        None
        
    Parameters:
        uint8_t charFormat - number of stop bits.  Available options are:
         * NUM_STOP_BITS_1 - 1 Stop bit
         * NUM_STOP_BITS_1_5 - 1.5 Stop bits
         * NUM_STOP_BITS_2 - 2 Stop bits
        
    Return Values:
        None
        
    Remarks:
        None
  
 *****************************************************************************/
#define CDCSetCharacterFormat(charFormat) {line_coding.bCharFormat=charFormat;}
#define NUM_STOP_BITS_1     0   //1 stop bit - used by CDCSetLineCoding() and CDCSetCharacterFormat()
#define NUM_STOP_BITS_1_5   1   //1.5 stop bit - used by CDCSetLineCoding() and CDCSetCharacterFormat()
#define NUM_STOP_BITS_2     2   //2 stop bit - used by CDCSetLineCoding() and CDCSetCharacterFormat()

/******************************************************************************
    Function:
        void CDCSetParity(uint8_t parityType)
        
    Summary:
        This function is used manually set the parity format reported back to 
        the host during a get line coding request. (optional)

    Description:
        This macro is used manually set the parity format reported back to 
        the host during a get line coding request.

        Typical Usage:
        <code>
            CDCSetParity(PARITY_NONE);
        </code>
        
        This function is optional for CDC devices that do not actually convert
        the USB traffic to a hardware UART.

    PreCondition:
        None
        
    Parameters:
        uint8_t parityType - Type of parity.  The options are the following:
            * PARITY_NONE
            * PARITY_ODD
            * PARITY_EVEN
            * PARITY_MARK
            * PARITY_SPACE
        
    Return Values:
        None
        
    Remarks:
        None
  
 *****************************************************************************/
#define CDCSetParity(parityType) {line_coding.bParityType=parityType;}
#define PARITY_NONE     0 //no parity - used by CDCSetLineCoding() and CDCSetParity()
#define PARITY_ODD      1 //odd parity - used by CDCSetLineCoding() and CDCSetParity()
#define PARITY_EVEN     2 //even parity - used by CDCSetLineCoding() and CDCSetParity()
#define PARITY_MARK     3 //mark parity - used by CDCSetLineCoding() and CDCSetParity()
#define PARITY_SPACE    4 //space parity - used by CDCSetLineCoding() and CDCSetParity()

/******************************************************************************
    Function:
        void CDCSetDataSize(uint8_t dataBits)
        
    Summary:
        This function is used manually set the number of data bits reported back 
        to the host during a get line coding request. (optional)

    Description:
        This function is used manually set the number of data bits reported back 
        to the host during a get line coding request.

        Typical Usage:
        <code>
            CDCSetDataSize(8);
        </code>
        
        This function is optional for CDC devices that do not actually convert
        the USB traffic to a hardware UART.

    PreCondition:
        None
        
    Parameters:
        uint8_t dataBits - number of data bits.  The options are 5, 6, 7, 8, or 16.
        
    Return Values:
        None
        
    Remarks:
        None
  
 *****************************************************************************/
#define CDCSetDataSize(dataBits) {line_coding.bDataBits=dataBits;}

/******************************************************************************
    Function:
        void CDCSetLineCoding(uint32_t baud, uint8_t format, uint8_t parity, uint8_t dataSize)
        
    Summary:
        This function is used to manually set the data reported back 
        to the host during a get line coding request. (optional)

    Description:
        This function is used to manually set the data reported back 
        to the host during a get line coding request.

        Typical Usage:
        <code>
            CDCSetLineCoding(19200, NUM_STOP_BITS_1, PARITY_NONE, 8);
        </code>
        
        This function is optional for CDC devices that do not actually convert
        the USB traffic to a hardware UART.

    PreCondition:
        None
        
    Parameters:
        uint32_t baud - The desired baud rate
        uint8_t format - number of stop bits.  Available options are:
         * NUM_STOP_BITS_1 - 1 Stop bit
         * NUM_STOP_BITS_1_5 - 1.5 Stop bits
         * NUM_STOP_BITS_2 - 2 Stop bits
        uint8_t parity - Type of parity.  The options are the following:
            * PARITY_NONE
            * PARITY_ODD
            * PARITY_EVEN
            * PARITY_MARK
            * PARITY_SPACE
        uint8_t dataSize - number of data bits.  The options are 5, 6, 7, 8, or 16.
        
    Return Values:
        None
        
    Remarks:
        None
  
 *****************************************************************************/
#define CDCSetLineCoding(baud,format,parity,dataSize) {\
            CDCSetBaudRate(baud);\
            CDCSetCharacterFormat(format);\
            CDCSetParity(parity);\
            CDCSetDataSize(dataSize);\
        }

/******************************************************************************
    Function:
        bool USBUSARTIsTxTrfReady(void)
        
    Summary:
        This macro is used to check if the CDC class is ready
        to send more data.

    Description:
        This macro is used to check if the CDC class handler firmware is 
        ready to send more data to the host over the CDC bulk IN endpoint.

        Typical Usage:
        <code>
            if(USBUSARTIsTxTrfReady())
            {
                putrsUSBUSART("Hello World");
            }
        </code>
        
    PreCondition:
        The return value of this function is only valid if the device is in a
        configured state (i.e. - USBDeviceGetState() returns CONFIGURED_STATE)
        
    Parameters:
        None
        
    Return Values:
        Returns a boolean value indicating if the CDC class handler firmware
        is ready to receive new data to send to the host over the bulk data IN
        endpoint.  A return value of true indicates that the CDC handler 
        firmware is ready to receive new data, and it is therefore safe to
        call other APIs like putrsUSBUSART() and putsUSBUSART().  A return
        value of false implies that the firmware is still busy sending the
        last data, or is otherwise not ready to process any new data at
        this time.
        
    Remarks:
        Make sure the application periodically calls the CDCTxService()
        handler, or pending USB IN transfers will not be able to advance
        and complete.
  
 *****************************************************************************/
#define USBUSARTIsTxTrfReady()      (cdc_trf_state == CDC_TX_READY)

/******************************************************************************
    Function:
        void mUSBUSARTTxRam(uint8_t *pData, uint8_t len)
    
    Description:
        Deprecated in MCHPFSUSB v2.3.  This macro has been replaced by 
        USBUSARTIsTxTrfReady().
 *****************************************************************************/
#define mUSBUSARTIsTxTrfReady()     USBUSARTIsTxTrfReady()

/******************************************************************************
    Function:
        void mUSBUSARTTxRam(uint8_t *pData, uint8_t len)
        
    Description:
        Use this macro to transfer data located in data memory.
        Use this macro when:
            1. Data stream is not null-terminated
            2. Transfer length is known
        Remember: cdc_trf_state must == CDC_TX_READY
        Unlike putsUSBUSART, there is not code double checking
        the transfer state. Unexpected behavior will occur if
        this function is called when cdc_trf_state != CDC_TX_READY
 
         Typical Usage:
        <code>
            if(USBUSARTIsTxTrfReady())
            {
                mUSBUSARTTxRam(&UserDataBuffer[0], 200);
            }
        </code>
        
    PreCondition:
        cdc_trf_state must be in the CDC_TX_READY state.
        Value of 'len' must be equal to or smaller than 255 bytes.
        The USB stack should have reached the CONFIGURED_STATE prior
        to calling this API function for the first time.
        
    Parameters:
        pDdata  : Pointer to the starting location of data bytes
        len     : Number of bytes to be transferred
        
    Return Values:
        None
        
    Remarks:
        This macro only handles the setup of the transfer. The
        actual transfer is handled by CDCTxService().  This macro
        does not "double buffer" the data.  The application firmware
        should not modify the contents of the pData buffer until all
        of the data has been sent, as indicated by the 
        USBUSARTIsTxTrfReady() function returning true, subsequent to
        calling mUSBUSARTTxRam().
        
  
 *****************************************************************************/
#define mUSBUSARTTxRam(pData,len)   \
{                                   \
    pCDCSrc.bRam = pData;           \
    cdc_tx_len = len;               \
    cdc_mem_type = USB_EP0_RAM;     \
    cdc_trf_state = CDC_TX_BUSY;    \
}

/******************************************************************************
    Function:
        void mUSBUSARTTxRom(rom uint8_t *pData, uint8_t len)
        
    Description:
        Use this macro to transfer data located in program memory.
        Use this macro when:
            1. Data stream is not null-terminated
            2. Transfer length is known
 
        Remember: cdc_trf_state must == CDC_TX_READY
        Unlike putrsUSBUSART, there is not code double checking
        the transfer state. Unexpected behavior will occur if
        this function is called when cdc_trf_state != CDC_TX_READY
 
          Typical Usage:
        <code>
            if(USBUSARTIsTxTrfReady())
            {
                mUSBUSARTTxRom(&SomeRomString[0], 200);
            }
        </code>
       
    PreCondition:
        cdc_trf_state must be in the CDC_TX_READY state.
        Value of 'len' must be equal to or smaller than 255 bytes.
        
    Parameters:
        pDdata  : Pointer to the starting location of data bytes
        len     : Number of bytes to be transferred
        
    Return Values:
        None
        
    Remarks:
        This macro only handles the setup of the transfer. The
        actual transfer is handled by CDCTxService().
                    
 *****************************************************************************/
#define mUSBUSARTTxRom(pData,len)   \
{                                   \
    pCDCSrc.bRom = pData;           \
    cdc_tx_len = len;               \
    cdc_mem_type = USB_EP0_ROM;     \
    cdc_trf_state = CDC_TX_BUSY;    \
}

/**************************************************************************
  Function:
        void CDCInitEP(void)
    
  Summary:
    This function initializes the CDC function driver. This function should
    be called after the SET_CONFIGURATION command (ex: within the context of
    the USBCBInitEP() function).
  Description:
    This function initializes the CDC function driver. This function sets
    the default line coding (baud rate, bit parity, number of data bits,
    and format). This function also enables the endpoints and prepares for
    the first transfer from the host.
    
    This function should be called after the SET_CONFIGURATION command.
    This is most simply done by calling this function from the
    USBCBInitEP() function.
    
    Typical Usage:
    <code>
        void USBCBInitEP(void)
        {
            CDCInitEP();
        }
    </code>
  Conditions:
    None
  Remarks:
    None                                                                   
  **************************************************************************/
void CDCInitEP(void);

/******************************************************************************
 	Function:
 		void USBCheckCDCRequest(void)
 
 	Description:
 		This routine checks the most recently received SETUP data packet to 
 		see if the request is specific to the CDC class.  If the request was
 		a CDC specific request, this function will take care of handling the
 		request and responding appropriately.
 		
 	PreCondition:
 		This function should only be called after a control transfer SETUP
 		packet has arrived from the host.

	Parameters:
		None
		
	Return Values:
		None
		
	Remarks:
		This function does not change status or do anything if the SETUP packet
		did not contain a CDC class specific request.		 
  *****************************************************************************/
void USBCheckCDCRequest(void);


/**************************************************************************
  Function: void CDCNotificationHandler(void)
  Summary: Checks for changes in DSR status and reports them to the USB host.
  Description: Checks for changes in DSR pin state and reports any changes
               to the USB host. 
  Conditions: CDCInitEP() must have been called previously, prior to calling
              CDCNotificationHandler() for the first time.
  Remarks:
    This function is only implemented and needed when the 
    USB_CDC_SUPPORT_DSR_REPORTING option has been enabled.  If the function is
    enabled, it should be called periodically to sample the DSR pin and feed
    the information to the USB host.  This can be done by calling 
    CDCNotificationHandler() by itself, or, by calling CDCTxService() which
    also calls CDCNotificationHandler() internally, when appropriate.
  **************************************************************************/
void CDCNotificationHandler(void);


/**********************************************************************************
  Function:
    bool USBCDCEventHandler(USB_EVENT event, void *pdata, uint16_t size)
    
  Summary:
    Handles events from the USB stack, which may have an effect on the CDC 
    endpoint(s).

  Description:
    Handles events from the USB stack.  This function should be called when 
    there is a USB event that needs to be processed by the CDC driver.
    
  Conditions:
    Value of input argument 'len' should be smaller than the maximum
    endpoint size responsible for receiving bulk data from USB host for CDC
    class. Input argument 'buffer' should point to a buffer area that is
    bigger or equal to the size specified by 'len'.
  Input:
    event - the type of event that occurred
    pdata - pointer to the data that caused the event
    size - the size of the data that is pointed to by pdata
                                                                                   
  **********************************************************************************/
bool USBCDCEventHandler(USB_EVENT event, void *pdata, uint16_t size);


/**********************************************************************************
  Function:
        uint8_t getsUSBUSART(char *buffer, uint8_t len)
    
  Summary:
    getsUSBUSART copies a string of BYTEs received through USB CDC Bulk OUT
    endpoint to a user's specified location. It is a non-blocking function.
    It does not wait for data if there is no data available. Instead it
    returns '0' to notify the caller that there is no data available.

  Description:
    getsUSBUSART copies a string of BYTEs received through USB CDC Bulk OUT
    endpoint to a user's specified location. It is a non-blocking function.
    It does not wait for data if there is no data available. Instead it
    returns '0' to notify the caller that there is no data available.
    
    Typical Usage:
    <code>
        uint8_t numBytes;
        uint8_t buffer[64]
    
        numBytes = getsUSBUSART(buffer,sizeof(buffer)); //until the buffer is free.
        if(numBytes \> 0)
        {
            //we received numBytes bytes of data and they are copied into
            //  the "buffer" variable.  We can do something with the data
            //  here.
        }
    </code>
  Conditions:
    Value of input argument 'len' should be smaller than the maximum
    endpoint size responsible for receiving bulk data from USB host for CDC
    class. Input argument 'buffer' should point to a buffer area that is
    bigger or equal to the size specified by 'len'.
  Input:
    buffer -  Pointer to where received BYTEs are to be stored
    len -     The number of BYTEs expected.
  Output:
    uint8_t -    Returns a byte indicating the total number of bytes that were actually
              received and copied into the specified buffer.  The returned value
              can be anything from 0 up to the len input value.  A return value of 0
              indicates that no new CDC bulk OUT endpoint data was available.
                                                                                   
  **********************************************************************************/
uint8_t getsUSBUSART(uint8_t *buffer, uint8_t len);

/******************************************************************************
  Function:
	void putUSBUSART(char *data, uint8_t length)
		
  Summary:
    putUSBUSART writes an array of data to the USB. Use this version, is
    capable of transferring 0x00 (what is typically a NULL character in any of
    the string transfer functions).

  Description:
    putUSBUSART writes an array of data to the USB. Use this version, is
    capable of transferring 0x00 (what is typically a NULL character in any of
    the string transfer functions).
    
    Typical Usage:
    <code>
        if(USBUSARTIsTxTrfReady())
        {
            char data[] = {0x00, 0x01, 0x02, 0x03, 0x04};
            putUSBUSART(data,5);
        }
    </code>
    
    The transfer mechanism for device-to-host(put) is more flexible than
    host-to-device(get). It can handle a string of data larger than the
    maximum size of bulk IN endpoint. A state machine is used to transfer a
    \long string of data over multiple USB transactions. CDCTxService()
    must be called periodically to keep sending blocks of data to the host.

  Conditions:
    USBUSARTIsTxTrfReady() must return true. This indicates that the last
    transfer is complete and is ready to receive a new block of data. The
    string of characters pointed to by 'data' must equal to or smaller than
    255 BYTEs.

  Input:
    char *data - pointer to a RAM array of data to be transfered to the host
    uint8_t length - the number of bytes to be transfered (must be less than 255).
		
 *****************************************************************************/
void putUSBUSART(uint8_t *data, uint8_t Length);

/******************************************************************************
	Function:
		void putsUSBUSART(char *data)
		
  Summary:
    putsUSBUSART writes a string of data to the USB including the null
    character. Use this version, 'puts', to transfer data from a RAM buffer.

  Description:
    putsUSBUSART writes a string of data to the USB including the null
    character. Use this version, 'puts', to transfer data from a RAM buffer.
    
    Typical Usage:
    <code>
        if(USBUSARTIsTxTrfReady())
        {
            char data[] = "Hello World";
            putsUSBUSART(data);
        }
    </code>
    
    The transfer mechanism for device-to-host(put) is more flexible than
    host-to-device(get). It can handle a string of data larger than the
    maximum size of bulk IN endpoint. A state machine is used to transfer a
    \long string of data over multiple USB transactions. CDCTxService()
    must be called periodically to keep sending blocks of data to the host.

  Conditions:
    USBUSARTIsTxTrfReady() must return true. This indicates that the last
    transfer is complete and is ready to receive a new block of data. The
    string of characters pointed to by 'data' must equal to or smaller than
    255 BYTEs.

  Input:
    char *data -  null\-terminated string of constant data. If a
                            null character is not found, 255 BYTEs of data
                            will be transferred to the host.
		
 *****************************************************************************/
void putsUSBUSART(char *data);


/**************************************************************************
  Function:
        void putrsUSBUSART(const const char *data)
    
  Summary:
    putrsUSBUSART writes a string of data to the USB including the null
    character. Use this version, 'putrs', to transfer data literals and
    data located in program memory.

  Description:
    putrsUSBUSART writes a string of data to the USB including the null
    character. Use this version, 'putrs', to transfer data literals and
    data located in program memory.
    
    Typical Usage:
    <code>
        if(USBUSARTIsTxTrfReady())
        {
            putrsUSBUSART("Hello World");
        }
    </code>
    
    The transfer mechanism for device-to-host(put) is more flexible than
    host-to-device(get). It can handle a string of data larger than the
    maximum size of bulk IN endpoint. A state machine is used to transfer a
    \long string of data over multiple USB transactions. CDCTxService()
    must be called periodically to keep sending blocks of data to the host.

  Conditions:
    USBUSARTIsTxTrfReady() must return true. This indicates that the last
    transfer is complete and is ready to receive a new block of data. The
    string of characters pointed to by 'data' must equal to or smaller than
    255 BYTEs.

  Input:
    const const char *data -  null\-terminated string of constant data. If a
                            null character is not found, 255 BYTEs of data
                            will be transferred to the host.
                                                                           
  **************************************************************************/
void putrsUSBUSART(const const char *data);

/************************************************************************
  Function:
        void CDCTxService(void)
    
  Summary:
    CDCTxService handles device-to-host transaction(s). This function
    should be called once per Main Program loop after the device reaches
    the configured state.
  Description:
    CDCTxService handles device-to-host transaction(s). This function
    should be called once per Main Program loop after the device reaches
    the configured state (after the CDCIniEP() function has already executed).
    This function is needed, in order to advance the internal software state 
    machine that takes care of sending multiple transactions worth of IN USB
    data to the host, associated with CDC serial data.  Failure to call 
    CDCTxService() periodically will prevent data from being sent to the
    USB host, over the CDC serial data interface.
    
    Typical Usage:
    <code>
    void main(void)
    {
        USBDeviceInit();
        while(1)
        {
            USBDeviceTasks();
            if((USBGetDeviceState() \< CONFIGURED_STATE) ||
               (USBIsDeviceSuspended() == true))
            {
                //Either the device is not configured or we are suspended
                //  so we don't want to do execute any application code
                continue;   //go back to the top of the while loop
            }
            else
            {
                //Keep trying to send data to the PC as required
                CDCTxService();
    
                //Run application code.
                UserApplication();
            }
        }
    }
    </code>
  Conditions:
    CDCIniEP() function should have already exectuted/the device should be
    in the CONFIGURED_STATE.
  Remarks:
    None                                                                 
  ************************************************************************/
void CDCTxService(void);


/** S T R U C T U R E S ******************************************************/

/* Line Coding Structure */
#define LINE_CODING_LENGTH          0x07

typedef union _LINE_CODING
{
    struct
    {
        uint8_t _byte[LINE_CODING_LENGTH];
    };
    struct
    {
        uint32_t   dwDTERate;
        uint8_t    bCharFormat;
        uint8_t    bParityType;
        uint8_t    bDataBits;
    };
} LINE_CODING;

typedef union _CONTROL_SIGNAL_BITMAP
{
    uint8_t _byte;
    struct
    {
        unsigned DTE_PRESENT:1;       // [0] Not Present  [1] Present
        unsigned CARRIER_CONTROL:1;   // [0] Deactivate   [1] Activate
    };
} CONTROL_SIGNAL_BITMAP;


/* Functional Descriptor Structure - See CDC Specification 1.1 for details */

/* Header Functional Descriptor */
typedef struct __attribute__((packed)) _USB_CDC_HEADER_FN_DSC
{
    uint8_t bFNLength;
    uint8_t bDscType;
    uint8_t bDscSubType;
    uint16_t bcdCDC;
} USB_CDC_HEADER_FN_DSC;

/* Abstract Control Management Functional Descriptor */
typedef struct __attribute__((packed)) _USB_CDC_ACM_FN_DSC
{
    uint8_t bFNLength;
    uint8_t bDscType;
    uint8_t bDscSubType;
    uint8_t bmCapabilities;
} USB_CDC_ACM_FN_DSC;

/* Union Functional Descriptor */
typedef struct __attribute__((packed)) _USB_CDC_UNION_FN_DSC
{
    uint8_t bFNLength;
    uint8_t bDscType;
    uint8_t bDscSubType;
    uint8_t bMasterIntf;
    uint8_t bSaveIntf0;
} USB_CDC_UNION_FN_DSC;

/* Call Management Functional Descriptor */
typedef struct __attribute__((packed)) _USB_CDC_CALL_MGT_FN_DSC
{
    uint8_t bFNLength;
    uint8_t bDscType;
    uint8_t bDscSubType;
    uint8_t bmCapabilities;
    uint8_t bDataInterface;
} USB_CDC_CALL_MGT_FN_DSC;

typedef union __attribute__((packed)) _CDC_NOTICE
{
    LINE_CODING GetLineCoding;
    LINE_CODING SetLineCoding;
    unsigned char packet[CDC_COMM_IN_EP_SIZE];
} CDC_NOTICE, *PCDC_NOTICE;

/* Bit structure definition for the SerialState notification byte */
typedef union
{
    uint8_t byte;
    struct
    {
        uint8_t    DCD             :1;
        uint8_t    DSR             :1;
        uint8_t    BreakState      :1;
        uint8_t    RingDetect      :1;
        uint8_t    FramingError    :1;
        uint8_t    ParityError     :1;
        uint8_t    Overrun         :1;
        uint8_t    Reserved        :1;
    }bits;    
}BM_SERIAL_STATE;  

/* Serial State Notification Packet Structure */
typedef struct
{
    uint8_t    bmRequestType;  //Always 0xA1 for serial state notification packets
    uint8_t    bNotification;  //Always SERIAL_STATE (0x20) for serial state notification packets
    uint16_t  wValue;     //Always 0 for serial state notification packets
    uint16_t  wIndex;     //Interface number
    uint16_t  wLength;    //Should always be 2 for serial state notification packets
    BM_SERIAL_STATE SerialState;
    uint8_t    Reserved;
}SERIAL_STATE_NOTIFICATION;   

//DOM-IGNORE-BEGIN
/** E X T E R N S ************************************************************/
extern uint8_t cdc_rx_len;
extern USB_HANDLE lastTransmission;

extern uint8_t cdc_trf_state;
extern POINTER pCDCSrc;
extern uint8_t cdc_tx_len;
extern uint8_t cdc_mem_type;

extern CDC_NOTICE cdc_notice;
extern LINE_CODING line_coding;

extern volatile CTRL_TRF_SETUP SetupPkt;
extern const uint8_t configDescriptor1[];

/** Public Prototypes *************************************************/
//------------------------------------------------------------------------------
//This is the list of public API functions provided by usb_function_cdc.c.
//This list is commented out, since the actual prototypes are declared above
//with associated inline documentation.
//------------------------------------------------------------------------------
//void USBCheckCDCRequest(void);
//void CDCInitEP(void);
//bool USBCDCEventHandler(USB_EVENT event, void *pdata, uint16_t size);
//uint8_t getsUSBUSART(char *buffer, uint8_t len);
//void putUSBUSART(char *data, uint8_t Length);
//void putsUSBUSART(char *data);
//void putrsUSBUSART(const const char *data);
//void CDCTxService(void);
//void CDCNotificationHandler(void);
//------------------------------------------------------------------------------
//DOM-IGNORE-END


#endif //CDC_H
