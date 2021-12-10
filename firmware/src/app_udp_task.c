/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.
 * TNG-Group

  File Name:
    app_udp_task.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

//------------------------------------------------------------------------------
// Section: Included Files
//------------------------------------------------------------------------------
#include "app_udp_task.h"
//------------------------------------------------------------------------------
#include <stddef.h>
#include <string.h>

#include "system_config.h"
#include "system_definitions.h"

#include "config/default/library/tcpip/tcp.h"
#include "config/default/library/tcpip/udp.h"
#include "config/default/library/tcpip/icmp.h"
#include "config/default/library/tcpip/tcpip_mac.h"
#include "config/default/library/tcpip/tcpip_manager.h"
#include "config/default/library/tcpip/src/tcpip_packet.h"
#include "config/default/library/tcpip/src/tcpip_manager_control.h"
#include "config/default/library/tcpip/src/ipv4_manager.h"
#include "config/default/library/tcpip/src/udp_manager.h"
#include "config/default/library/tcpip/src/oahash.h"
#include "config/default/library/tcpip/src/link_list.h"
#include "config/default/library/tcpip/src/arp_private.h"

#include "third_party/rtos/FreeRTOS/Source/include/FreeRTOS.h"
#include "third_party/rtos/FreeRTOS/Source/include/queue.h"

#include "config/default/system/debug/sys_debug.h"
#include "config/default/system/console/sys_console.h"

#include <bsp/bsp.h>
//------------------------------------------------------------------------------
#include "AMAK_SHDSL_Queue.h"

//------------------------------------------------------------------------------
#define ENABLE_CONSOLE_MESSAGE
//------------------------------------------------------------------------------
// Section: Global Data Definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="Description UDP Task Application Data">
  UDP Task Application Data

  Summary:
    Holds UDP task application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_UDP_TASK_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/
// </editor-fold>
APP_UDP_TASK_DATA app_udp_taskData;
//------------------------------------------------------------------------------

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="Description Event Queue for UDP Task Application">
  Event Queue for UDP Task Application

  Summary:
    This variable holds event queue for UDP task application

  Description:
    
  Remarks:
    
*/
// </editor-fold>
QueueHandle_t eventQueue_app_udp_task = NULL;
//------------------------------------------------------------------------------

TCPIP_UDP_PROCESS_HANDLE amak_shdsl_pktHandle;
//------------------------------------------------------------------------------
// Section: Application Callback Functions
//------------------------------------------------------------------------------
/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="Description function: amak_shdsl_mac_packet_handler()">
  Function:
    static bool amak_shdsl_mac_packet_handler(TCPIP_NET_HANDLE hNet, TCPIP_MAC_PACKET* rxPkt, uint16_t frameType, const void* hParam)

  Summary:
    

  Description:
    This function 

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    
    </code>

  Remarks:
    None.
*/
// </editor-fold>
static bool amak_shdsl_mac_packet_handler(TCPIP_NET_HANDLE hNet, TCPIP_MAC_PACKET* pRxPkt, uint16_t frameType, const void* hParam);

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="Description function: amak_shdsl_arp_packet_handler()">
 
   Function:
    bool <FunctionName> (TCPIP_NET_HANDLE hNet, struct _tag_TCPIP_MAC_PACKET* rxPkt, const void* hParam);

  Summary:
    Pointer to a function(handler) that will get called to process an incoming ARP packet.

  Description:
    Function that will be called by the amak_shdsl_mac_packet_handler()
    when a RX ARP-packet is available.

  Precondition:
    None

  Parameters:
    hNet        - network handle on which the packet has arrived
    rxPkt       - pointer to incoming packet
    hParam      - user passed parameter when handler was registered

  Returns:
    true - if the packet is processed by the external handler.
           In this case the IPv4 module will no longer process the packet
    false - the packet needs to be processed internally by the IPv4 as usual           

  Remarks:
    The packet handler is called in the IPv4 context.
    The handler should be kept as short as possible as it affects the processing of all the other
    IPv4 RX traffic.

    Before calling the external packet handler 
    - the rxPkt->pNetLayer points to an IPV4_HEADER data structure.
    - the rxPkt->pktIf points to the interface receiving the packet
    - no other checks are done! (checksum, versions, etc.)

    Important!
    When the packet handler returns true, once it's done processing the packet,
    it needs to acknowledge it, i.e. return to the owner,
    which is the MAC driver serving the network interface!
    This means that the packet acknowledge function needs to be called,
    with a proper acknowledge parameter and the QUEUED flag needs to be cleared, if needed:
    if((*rxPkt->ackFunc)(rxPkt, rxPkt->ackParam))
    {
           rxPkt->pktFlags &= ~TCPIP_MAC_PKT_FLAG_QUEUED;
    }
    Failure to do that will result in memory leaks and starvation of the MAC driver.
    See the tcpip_mac.h for details.
    
 */
//</editor-fold>
static bool amak_shdsl_arp_packet_handler(TCPIP_NET_HANDLE hNet, TCPIP_MAC_PACKET* pRxPkt, const void* hParam);

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="Description function: amak_shdsl_ip_packet_handler()">
 IPv4 packet handler Pointer

   Function:
    bool <FunctionName> (TCPIP_NET_HANDLE hNet, struct _tag_TCPIP_MAC_PACKET* rxPkt, const void* hParam);

  Summary:
    Pointer to a function(handler) that will get called to process an incoming IPv4 packet.

  Description:
    Pointer to a function that will be called by the IPv4 module
    when a RX packet is available.

  Precondition:
    None

  Parameters:
    hNet        - network handle on which the packet has arrived
    rxPkt       - pointer to incoming packet
    hParam      - user passed parameter when handler was registered

  Returns:
    true - if the packet is processed by the external handler.
           In this case the IPv4 module will no longer process the packet
    false - the packet needs to be processed internally by the IPv4 as usual           

  Remarks:
    The packet handler is called in the IPv4 context.
    The handler should be kept as short as possible as it affects the processing of all the other
    IPv4 RX traffic.

    Before calling the external packet handler 
    - the rxPkt->pNetLayer points to an IPV4_HEADER data structure.
    - the rxPkt->pktIf points to the interface receiving the packet
    - no other checks are done! (checksum, versions, etc.)

    Important!
    When the packet handler returns true, once it's done processing the packet,
    it needs to acknowledge it, i.e. return to the owner,
    which is the MAC driver serving the network interface!
    This means that the packet acknowledge function needs to be called,
    with a proper acknowledge parameter and the QUEUED flag needs to be cleared, if needed:
    if((*rxPkt->ackFunc)(rxPkt, rxPkt->ackParam))
    {
           rxPkt->pktFlags &= ~TCPIP_MAC_PKT_FLAG_QUEUED;
    }
    Failure to do that will result in memory leaks and starvation of the MAC driver.
    See the tcpip_mac.h for details.
    
 */
//</editor-fold>
static bool amak_shdsl_ip_packet_handler(TCPIP_NET_HANDLE hNet, TCPIP_MAC_PACKET* pRxPkt, const void* hParam);

/*<editor-fold defaultstate="collapsed" desc="Description function: amak_shdsl_icmp_packet_handler()">

   Function:
    bool <FunctionName> (TCPIP_NET_HANDLE hNet, struct _tag_TCPIP_MAC_PACKET* rxPkt, const void* hParam);

  Summary:
    Pointer to a function(handler) that will get called to process an incoming ICMP packet.

  Description:
    Function that will be called by the amak_shdsl_ip_packet_handler()
    when a RX ICMP-packet is available.

  Precondition:
    None

  Parameters:
    hNet        - network handle on which the packet has arrived
    rxPkt       - pointer to incoming packet
    hParam      - user passed parameter when handler was registered

  Returns:
    true - if the packet is processed by the external handler.
           In this case the IPv4 module will no longer process the packet
    false - the packet needs to be processed internally by the IPv4 as usual           

  Remarks:
    The packet handler is called in the IPv4 context.
    The handler should be kept as short as possible as it affects the processing of all the other
    IPv4 RX traffic.

    Before calling the external packet handler 
    - the rxPkt->pNetLayer points to an IPV4_HEADER data structure.
    - the rxPkt->pktIf points to the interface receiving the packet
    - no other checks are done! (checksum, versions, etc.)

    Important!
    When the packet handler returns true, once it's done processing the packet,
    it needs to acknowledge it, i.e. return to the owner,
    which is the MAC driver serving the network interface!
    This means that the packet acknowledge function needs to be called,
    with a proper acknowledge parameter and the QUEUED flag needs to be cleared, if needed:
    if((*rxPkt->ackFunc)(rxPkt, rxPkt->ackParam))
    {
           rxPkt->pktFlags &= ~TCPIP_MAC_PKT_FLAG_QUEUED;
    }
    Failure to do that will result in memory leaks and starvation of the MAC driver.
    See the tcpip_mac.h for details.
    
 */
//</editor-fold>
static bool amak_shdsl_icmp_packet_handler(TCPIP_NET_HANDLE hNet, TCPIP_MAC_PACKET* pRxPkt, const void* hParam);

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="Description function: amak_shdsl_udp_packet_handler()">
  Function:
    static bool amak_shdsl_udp_packet_handler(TCPIP_NET_HANDLE hNet, TCPIP_MAC_PACKET* pRxPkt, const void* hParam)

  Summary:
    

  Description:
    This function 

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    
    </code>

  Remarks:
    None.
*/
// </editor-fold>
static bool amak_shdsl_udp_packet_handler(TCPIP_NET_HANDLE hNet, TCPIP_MAC_PACKET* pRxPkt, const void* hParam);

//------------------------------------------------------------------------------
// Section: Application Local Functions
//------------------------------------------------------------------------------

__inline__ bool __attribute__((always_inline)) is_ipv4_hdr_len_in_packet_limits(TCPIP_MAC_PACKET* pRxPkt, uint8_t header_len);
__inline__ bool __attribute__((always_inline)) is_fragment_ipv4(IPV4_HEADER* pCHeader);
//------------------------------------------------------------------------------
__inline__ bool __attribute__((always_inline)) is_necessary_port(const UDP_HEADER* const pUDPHdr);

__inline__ void __attribute__((always_inline)) send_packet_2_amak_parser(TCPIP_MAC_PACKET* pRxPkt, const uint8_t* packet, const uint16_t len);

///*------------------------------------------------------------------------------
//<editor-fold defaultstate="collapsed" desc="Description function: wait_console_buffer_free()">
//  Function:
//    void wait_console_buffer_free ( void )
//
//  Summary:
//    
//
//  Description:
//    This function wait until Sys Console write buffer become empty.
//
//  Precondition:
//    None.
//
//  Parameters:
//    None.
//
//  Returns:
//    None.
//
//  Example:
//    <code>
//    wait_console_buffer_free();
//    </code>
//
//  Remarks:
//    None.
//*/
//// </editor-fold>
//void wait_console_buffer_free(void);

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="Description function: app_udp_task_init()">
  Function:
    APP_UDP_TASK_STATES app_udp_task_init ( void )

  Summary:
    

  Description:
    This function work in APP_UDP_TASK State Machine. 
  

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    app_udp_taskData.state = app_udp_task_init();
    </code>

  Remarks:
    None.
*/
// </editor-fold>
APP_UDP_TASK_STATES app_udp_task_init(void);

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">
  Function:
    APP_UDP_TASK_STATES app_udp_task_sart ( void )

  Summary:
    

  Description:
    This function work in APP_UDP_TASK State Machine. 
  

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    app_udp_taskData.state = app_udp_task_sart();
    </code>

  Remarks:
    None.
*/
//</editor-fold>
APP_UDP_TASK_STATES app_udp_task_start(void);

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">
  Function:
    APP_UDP_TASK_STATES app_udp_task_wait_for_ip ( void )

  Summary:
    

  Description:
    This function work in APP_UDP_TASK State Machine. 
  It is wait until IP initialization complite and IP go work.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    app_udp_taskData.state = app_udp_task_wait_for_ip();
    </code>

  Remarks:
    None.
*/
//</editor-fold>
APP_UDP_TASK_STATES app_udp_task_wait_for_ip(void);

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">
  Function:
    APP_UDP_TASK_STATES app_udp_task_register_amak_shdsl_udp_handler ( void )

  Summary:
    

  Description:
    This function work in APP_UDP_TASK State Machine. 
  It register udp handler for amak-shdsl packets.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    app_udp_taskData.state = app_udp_task_register_amak_shdsl_udp_handler();
    </code>

  Remarks:
    None.
*/
//</editor-fold>
APP_UDP_TASK_STATES app_udp_task_register_amak_shdsl_udp_handler(void);

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">
  Function:
    APP_UDP_TASK_STATES app_udp_task_wait_server_open ( void )

  Summary:
    

  Description:
    This function work in APP_UDP_TASK State Machine. 
  This function open UDP server.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    app_udp_taskData.state = app_udp_task_wait_server_open();
    </code>

  Remarks:
    None.
*/
//</editor-fold>
APP_UDP_TASK_STATES app_udp_task_wait_server_open(void);

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">
  Function:
    APP_UDP_TASK_STATES app_udp_task_server_listen ( void )

  Summary:
    

  Description:
    This function work in APP_UDP_TASK State Machine. 
  This function wait until server start listen.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    app_udp_taskData.state = app_udp_task_server_listen();
    </code>

  Remarks:
    None.
*/
//</editor-fold>
APP_UDP_TASK_STATES app_udp_task_server_listen(void);

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">
  Function:
    APP_UDP_TASK_STATES app_udp_task_wait_client_open ( void )

  Summary:
    

  Description:
    This function work in APP_UDP_TASK State Machine. 
  This function open UDP client.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    app_udp_taskData.state = app_udp_task_wait_client_open();
    </code>

  Remarks:
    None.
*/
//</editor-fold>
APP_UDP_TASK_STATES app_udp_task_wait_client_open(void);

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">
  Function:
    APP_UDP_TASK_STATES app_udp_task_wait_for_connection ( void )

  Summary:
    

  Description:
    This function work in APP_UDP_TASK State Machine. 
  This function wait while UDP client connected to UDP server.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    app_udp_taskData.state = app_udp_task_wait_for_connection();
    </code>

  Remarks:
    None.
*/
//</editor-fold>
APP_UDP_TASK_STATES app_udp_task_wait_for_connection(void);

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">
  Function:
    APP_UDP_TASK_STATES app_udp_task_rx ( void )

  Summary:
    

  Description:
    This function work in APP_UDP_TASK State Machine. 
  This function receive packets.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    app_udp_taskData.state = app_udp_task_rx();
    </code>

  Remarks:
    None.
*/
//</editor-fold>
APP_UDP_TASK_STATES app_udp_task_rx(void);

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">
  Function:
    APP_UDP_TASK_STATES app_udp_task_tx ( void )

  Summary:
    

  Description:
    This function work in APP_UDP_TASK State Machine. 
  This function transmit packets.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    app_udp_taskData.state = app_udp_task_tx();
    </code>

  Remarks:
    None.
*/
//</editor-fold>
APP_UDP_TASK_STATES app_udp_task_tx(void);

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">
  Function:
    APP_UDP_TASK_STATES app_udp_task_closing_connection ( void )

  Summary:
    

  Description:
    This function work in APP_UDP_TASK State Machine. 
  This function calling when connection is close.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    app_udp_taskData.state = app_udp_task_closing_connection();
    </code>

  Remarks:
    None.
*/
//</editor-fold>
APP_UDP_TASK_STATES app_udp_task_closing_connection(void);

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">
  Function:
    APP_UDP_TASK_STATES app_udp_task_error ( void )

  Summary:
    

  Description:
    This function work in APP_UDP_TASK State Machine. 
  This function do error processing.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    app_udp_taskData.state = app_udp_task_error();
    </code>

  Remarks:
    None.
*/
//</editor-fold>
APP_UDP_TASK_STATES app_udp_task_error(void);

//------------------------------------------------------------------------------
// Section: Application Initialization and State Machine Functions
//------------------------------------------------------------------------------
/*------------------------------------------------------------------------------
  Function:
    void APP_UDP_TASK_Initialize ( void )

  Remarks:
    See prototype in app_udp_task.h.
 */
//------------------------------------------------------------------------------
void APP_UDP_TASK_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_udp_taskData.state = APP_UDP_TASK_STATE_INIT;
    app_udp_taskData.error = APP_UDP_TASK_ERROR_NO;
    
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
    LED2_Toggle();
    eventQueue_app_udp_task = xQueueCreate( APP_UDP_TASK_QUEUE_LEN, EVENT_INFO_SIZE );
    if (NULL == eventQueue_app_udp_task)
    {
        app_udp_taskData.error = APP_UDP_TASK_ERROR_Create_Queue;
        app_udp_taskData.state = APP_UDP_TASK_STATE_Error;
        /* Handle error condition. Not sufficient memory to create Queue */
        #ifdef ENABLE_CONSOLE_MESSAGE
            SYS_CONSOLE_MESSAGE(" \r\n");
            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: Handle error condition. Not sufficient memory to create Queue\r\n");
            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> APP_UDP_TASK_STATE_Error \r\n");
        #endif
    }

//    app_udp_taskData.bflag_UDP_enable    = false;
//    app_udp_taskData.bflag_UDP_send_data = false;
//    app_udp_taskData.udp_state  = UDP_STATE_INIT;
//    
//    app_udp_taskData.udp_rx_data_len = 0;
//    app_udp_taskData.udp_tx_data_len = 1472;
//    
//    int i;
//    for (i = 0; i < 1472; i++) app_udp_taskData.udp_rx_data[i] = 0;
//    for (i = 0; i < 1472; i++) app_udp_taskData.udp_tx_data[i] = 0;
    
    #ifdef ENABLE_CONSOLE_MESSAGE
        SYS_CONSOLE_MESSAGE(" \r\n");
        SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> APP_UDP_TASK_STATE_INIT \r\n");
    #endif
}
//------------------------------------------------------------------------------
/*------------------------------------------------------------------------------
  Function:
    void APP_UDP_TASK_Tasks ( void )

  Remarks:
    See prototype in app_udp_task.h.
 */
//------------------------------------------------------------------------------
void APP_UDP_TASK_Tasks ( void )
{
    switch ( app_udp_taskData.state )
    {
        //----------------------------------------------------------------------
        case APP_UDP_TASK_STATE_INIT:
        {
            app_udp_taskData.state = app_udp_task_init();
            taskYIELD();
            break;
        }
        //----------------------------------------------------------------------
        case APP_UDP_TASK_STATE_Start:
        {
            app_udp_taskData.state = app_udp_task_start();
            taskYIELD();
            break;
        }
        //----------------------------------------------------------------------
        case APP_UDP_TASK_STATE_WAIT_FOR_IP:
        {
            app_udp_taskData.state = app_udp_task_wait_for_ip();
            taskYIELD();
            break;
        }
        //----------------------------------------------------------------------
        case APP_UDP_TASK_STATE_REGISTER_AMAK_SHDSL_UDP_HANDLER:
        {
            app_udp_taskData.state = app_udp_task_register_amak_shdsl_udp_handler();
            taskYIELD();
            break;
        }
        //----------------------------------------------------------------------
        case APP_UDP_TASK_STATE_WAIT_SERVER_OPEN:
        {
            app_udp_taskData.state = app_udp_task_wait_server_open();
            taskYIELD();
            break;
        }
        //----------------------------------------------------------------------
        case APP_UDP_TASK_STATE_SERVER_LISTEN:
        {
            app_udp_taskData.state = app_udp_task_server_listen();
            taskYIELD();
            break;
        }
        //----------------------------------------------------------------------
        case APP_UDP_TASK_STATE_WAIT_CLIENT_OPEN:
        {
            app_udp_taskData.state = app_udp_task_wait_client_open();
            taskYIELD();
            break;
        }
        //----------------------------------------------------------------------
        case APP_UDP_TASK_STATE_WAIT_FOR_CONNECTION:
        {
            app_udp_taskData.state = app_udp_task_wait_for_connection();
            taskYIELD();
            break;
        }
        //----------------------------------------------------------------------
        case APP_UDP_TASK_STATE_Rx:
        {
            app_udp_taskData.state = app_udp_task_rx();
            taskYIELD();
            break;
        }
        //----------------------------------------------------------------------
        case APP_UDP_TASK_STATE_Tx:
        {
            app_udp_taskData.state = app_udp_task_tx();
            taskYIELD();
            break;
        }
        //----------------------------------------------------------------------
        case APP_UDP_TASK_STATE_CLOSING_CONNECTION:
        {
            app_udp_taskData.state = app_udp_task_closing_connection();
            break;
        }
        //----------------------------------------------------------------------
        case APP_UDP_TASK_STATE_Error:
        {
            app_udp_taskData.state = app_udp_task_error();
            taskYIELD();    // ???
            break;
        }
        //----------------------------------------------------------------------
        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            taskYIELD();    //  ???
            break;
        }
        //----------------------------------------------------------------------
    }
}
//------------------------------------------------------------------------------
void wait_console_buffer_free(void)
{
    #ifdef ENABLE_CONSOLE_MESSAGE
        ssize_t nFreeSpace;
        SYS_CONSOLE_HANDLE myConsoleHandle;
        myConsoleHandle = SYS_CONSOLE_HandleGet(SYS_CONSOLE_DEFAULT_INSTANCE);
        if (SYS_CONSOLE_HANDLE_INVALID != myConsoleHandle)
        {
            // Found a valid handle to the console instance
            // Get the number of bytes of free space available in the transmit buffer.
            nFreeSpace = SYS_CONSOLE_WriteFreeBufferCountGet(myConsoleHandle);
            // wait console bufer empty
            while ( ( (SYS_CONSOLE_PRINT_BUFFER_SIZE - 1) > nFreeSpace ) && ( -1 != nFreeSpace ) ) 
            {
                taskYIELD();
                nFreeSpace = SYS_CONSOLE_WriteFreeBufferCountGet(myConsoleHandle);

            }
        }
    #endif
}
//------------------------------------------------------------------------------
APP_UDP_TASK_STATES app_udp_task_init(void)
{
    LED3_Toggle();
    
    bool                appUdpTaskInitialized = false;
    APP_UDP_TASK_STATES result = app_udp_taskData.state;

    if (NULL != eventQueue_app_udp_task)
    {
        appUdpTaskInitialized = true;
    }

    if (appUdpTaskInitialized)
    {
        app_udp_taskData.dest_adr = (IP_MULTI_ADDRESS){ .v4Add.v = {10, 2, 22, 220} };
        app_udp_taskData.dest_port = 1500;

        app_udp_taskData.local_adr = (IP_MULTI_ADDRESS){ .v4Add.v = {10, 2, 22, 245} };
        app_udp_taskData.local_port = 1500;

        app_udp_taskData.necessary_src_port  = TCPIP_Helper_htons(NECESSARY_SRC_PORT);
        app_udp_taskData.necessary_dest_port = TCPIP_Helper_htons(NECESSARY_DEST_PORT);

        app_udp_taskData.error = APP_UDP_TASK_ERROR_NO;
        result = APP_UDP_TASK_STATE_Start;
        #ifdef ENABLE_CONSOLE_MESSAGE
            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: Initialization complite\r\n");
            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> APP_UDP_TASK_STATE_Start \r\n");
        #endif
    }
    
    return result;
}
//------------------------------------------------------------------------------
APP_UDP_TASK_STATES app_udp_task_start(void)
{
    APP_UDP_TASK_STATES result = app_udp_taskData.state;
    //------------------------------------------------------------------
    // <editor-fold defaultstate="collapsed" desc="Wait until Console Buffer become free">
    wait_console_buffer_free();
    // </editor-fold>
    //------------------------------------------------------------------
    SYS_STATUS tcpipStat = TCPIP_STACK_Status(sysObj.tcpip);
    if (0 > tcpipStat)
    {   // some error occurred
        #ifdef ENABLE_CONSOLE_MESSAGE
            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: TCP/IP stack initialization failed!\r\n");
            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> UDP_STATE_Error\r\n");
        #endif
        app_udp_taskData.error = APP_UDP_TASK_ERROR_STATE_Start;
        result = APP_UDP_TASK_STATE_Error;
    }
    else if(SYS_STATUS_READY == tcpipStat)
    {
        app_udp_taskData.netH = TCPIP_STACK_IndexToNet(0);
        #ifdef ENABLE_CONSOLE_MESSAGE
            const char *netName;
            const char *netBiosName;

            netName = TCPIP_STACK_NetNameGet(app_udp_taskData.netH);
            netBiosName = TCPIP_STACK_NetBIOSName(app_udp_taskData.netH);

            SYS_CONSOLE_PRINT("    Interface %s on host %s \r\n", netName, netBiosName);
        #endif

        #ifdef ENABLE_CONSOLE_MESSAGE
            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> UDP_STATE_WAIT_FOR_IP\r\n");
        #endif
        app_udp_taskData.error = APP_UDP_TASK_ERROR_NO;
        result = APP_UDP_TASK_STATE_WAIT_FOR_IP;
    }
    
    return result;
}
//------------------------------------------------------------------------------
APP_UDP_TASK_STATES app_udp_task_wait_for_ip(void)
{
    APP_UDP_TASK_STATES result = app_udp_taskData.state;
    static IPV4_ADDR    dwLastIP = {-1};
    IPV4_ADDR           ipAddr;
    //------------------------------------------------------------------
    // <editor-fold defaultstate="collapsed" desc="Wait until Console Buffer become free">
    wait_console_buffer_free();
    // </editor-fold>
    //------------------------------------------------------------------
    if( false == TCPIP_STACK_NetIsReady(app_udp_taskData.netH) )
    {
//        taskYIELD();
        return result; // interface not ready yet!
    }
    ipAddr.Val = TCPIP_STACK_NetAddress(app_udp_taskData.netH);
    if(dwLastIP.Val != ipAddr.Val)
    {
        dwLastIP.Val = ipAddr.Val;

        #ifdef ENABLE_CONSOLE_MESSAGE
            const TCPIP_MAC_ADDR* pMACAddr = (const TCPIP_MAC_ADDR*)TCPIP_STACK_NetAddressMac(app_udp_taskData.netH);

            char str_ip_adr[20];
            char str_mac_adr[25];

            TCPIP_Helper_IPAddressToString( &ipAddr, str_ip_adr, sizeof(str_ip_adr) );
            TCPIP_Helper_MACAddressToString( pMACAddr, str_mac_adr, sizeof(str_mac_adr) );

            SYS_CONSOLE_PRINT   ("----------== Net %d ==------------\r\n", 0);
            SYS_CONSOLE_PRINT   ("    NetName:     %s \r\n", 
                                    TCPIP_STACK_NetNameGet(app_udp_taskData.netH) );

            SYS_CONSOLE_PRINT   ("    IP  address: %s \r\n", str_ip_adr);

            SYS_CONSOLE_PRINT   ("    MAC address: %s \r\n", str_mac_adr);

            SYS_CONSOLE_MESSAGE ("---------------------------------\r\n");
        #endif
    }
    //------------------------------------------------------------------
    #ifdef ENABLE_CONSOLE_MESSAGE
        SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> UDP_STATE_REGISTER_AMAK_SHDSL_UDP_HANDLER\r\n");
    #endif
    app_udp_taskData.error = APP_UDP_TASK_ERROR_NO;
    result = APP_UDP_TASK_STATE_REGISTER_AMAK_SHDSL_UDP_HANDLER;

    //------------------------------------------------------------------
//    #ifdef ENABLE_CONSOLE_MESSAGE
//        SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> UDP_STATE_WAIT_SERVER_OPEN\r\n");
//    #endif
//    app_udp_taskData.error = APP_UDP_TASK_ERROR_NO;
//    result = APP_UDP_TASK_STATE_WAIT_SERVER_OPEN;
//
//    #ifdef ENABLE_CONSOLE_MESSAGE
//        SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> UDP_STATE_WAIT_CLIENT_OPEN\r\n");
//    #endif
//    // first - client open, second - server open... else client open error
//    app_udp_taskData.error = APP_UDP_TASK_ERROR_NO;
//    result = APP_UDP_TASK_STATE_WAIT_CLIENT_OPEN;   
    
    return result;
}
//------------------------------------------------------------------------------
//APP_UDP_TASK_STATES app_udp_task_register_amak_shdsl_ip_handler(void)
//{
//    //------------------------------------------------------------------
//    // <editor-fold defaultstate="collapsed" desc="Wait until Console Buffer become free">
//    wait_console_buffer_free();
//    // </editor-fold>
//    //------------------------------------------------------------------
//    amak_shdsl_pktHandle = TCPIP_IPV4_PacketHandlerRegister( amak_shdsl_ip_packet_handler, NULL);
//    //------------------------------------------------------------------
//    #ifdef ENABLE_CONSOLE_MESSAGE
//        SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> UDP_STATE_WAIT_SERVER_OPEN\r\n");
//    #endif
//    app_udp_taskData.error = APP_UDP_TASK_ERROR_NO;
//    
//    return APP_UDP_TASK_STATE_WAIT_SERVER_OPEN;
//}
//------------------------------------------------------------------------------
APP_UDP_TASK_STATES app_udp_task_register_amak_shdsl_udp_handler(void)
{
    //------------------------------------------------------------------
    // <editor-fold defaultstate="collapsed" desc="Wait until Console Buffer become free">
    wait_console_buffer_free();
    // </editor-fold>
    //------------------------------------------------------------------
//    amak_shdsl_pktHandle = TCPIP_UDP_PacketHandlerRegister( amak_shdsl_udp_packet_handler, NULL );
//    amak_shdsl_pktHandle = TCPIP_IPV4_PacketHandlerRegister( amak_shdsl_ip_packet_handler, NULL);
    amak_shdsl_pktHandle = TCPIP_STACK_PacketHandlerRegister(app_udp_taskData.netH, amak_shdsl_mac_packet_handler, NULL);
    //------------------------------------------------------------------
    #ifdef ENABLE_CONSOLE_MESSAGE
        SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> UDP_STATE_WAIT_SERVER_OPEN\r\n");
    #endif
    app_udp_taskData.error = APP_UDP_TASK_ERROR_NO;
    
    return APP_UDP_TASK_STATE_WAIT_SERVER_OPEN;
}
//------------------------------------------------------------------------------
APP_UDP_TASK_STATES app_udp_task_wait_server_open(void)
{
    APP_UDP_TASK_STATES result = app_udp_taskData.state;
    //------------------------------------------------------------------
    // <editor-fold defaultstate="collapsed" desc="Wait until Console Buffer become free">
    wait_console_buffer_free();
    // </editor-fold>
    //------------------------------------------------------------------
    app_udp_taskData.udp_rx_socket = TCPIP_UDP_ServerOpen( IP_ADDRESS_TYPE_IPV4, app_udp_taskData.local_port, 0 );
//    app_udp_taskData.udp_rx_socket = TCPIP_UDP_ServerOpen( IP_ADDRESS_TYPE_IPV4, app_udp_taskData.local_port, &(app_udp_taskData.local_adr) );
    if (INVALID_SOCKET != app_udp_taskData.udp_rx_socket)
    {
        #ifdef ENABLE_CONSOLE_MESSAGE
            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP server open: Ok \r\n");
        #endif
//        TCPIP_UDP_SourceIPAddressSet(app_udp_taskData.udp_rx_socket, IP_ADDRESS_TYPE_IPV4, &local_adr);
//        TCPIP_UDP_DestinationIPAddressSet(app_udp_taskData.udp_rx_socket, IP_ADDRESS_TYPE_IPV4, &dest_adr);
//
//        if ( true == TCPIP_UDP_RemoteBind( app_udp_taskData.udp_rx_socket, IP_ADDRESS_TYPE_IPV4, app_udp_taskData.dest_port, &(app_udp_taskData.dest_adr) ) )
//        {
//            #ifdef ENABLE_CONSOLE_MESSAGE
//                SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP remote bind: Ok \r\n");
//            #endif
//        }
//
//        if ( true == TCPIP_UDP_Bind( app_udp_taskData.udp_rx_socket, IP_ADDRESS_TYPE_IPV4, app_udp_taskData.local_port, &(app_udp_taskData.local_adr) ) )
//        {
//            #ifdef ENABLE_CONSOLE_MESSAGE
//                SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP bind: Ok \r\n");
//            #endif
//        }
        //--------------------------------------------------------------
        #ifdef ENABLE_CONSOLE_MESSAGE
            const char          *netName;
            const char          *netBiosName;
            UDP_SOCKET_INFO     socket_info;
            TCPIP_NET_HANDLE    netH;
            IPV4_ADDR           ipAddr;

            TCPIP_UDP_SocketInfoGet(app_udp_taskData.udp_rx_socket, &socket_info);

            netH        = socket_info.hNet;
            netName     = TCPIP_STACK_NetNameGet(netH);
            netBiosName = TCPIP_STACK_NetBIOSName(netH);
            ipAddr.Val  = TCPIP_STACK_NetAddress(netH);

            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: ----------== Rx socket ==----------\r\n");

            SYS_CONSOLE_PRINT   ("      Interface %s on host %s \r\n", netName, netBiosName);

            SYS_CONSOLE_MESSAGE ("      IP Address: ");
            SYS_CONSOLE_PRINT   ("%d.%d.%d.%d \r\n", 
                                    ipAddr.v[0], 
                                    ipAddr.v[1], 
                                    ipAddr.v[2], 
                                    ipAddr.v[3]
                                );
            SYS_CONSOLE_PRINT   ("APP_UDP_TASK: Dest   IP %d.%d.%d.%d \r\n", 
                                    socket_info.destIPaddress.v4Add.v[0], 
                                    socket_info.destIPaddress.v4Add.v[1], 
                                    socket_info.destIPaddress.v4Add.v[2],   
                                    socket_info.destIPaddress.v4Add.v[3]
                                );
            SYS_CONSOLE_PRINT   ("APP_UDP_TASK: Source IP %d.%d.%d.%d \r\n", 
                                    socket_info.sourceIPaddress.v4Add.v[0], 
                                    socket_info.sourceIPaddress.v4Add.v[1], 
                                    socket_info.sourceIPaddress.v4Add.v[2], 
                                    socket_info.sourceIPaddress.v4Add.v[3]
                                );
            SYS_CONSOLE_PRINT   ("APP_UDP_TASK: Local  IP %d.%d.%d.%d \r\n", 
                                    socket_info.localIPaddress.v4Add.v[0], 
                                    socket_info.localIPaddress.v4Add.v[1], 
                                    socket_info.localIPaddress.v4Add.v[2], 
                                    socket_info.localIPaddress.v4Add.v[3]
                                );
            SYS_CONSOLE_PRINT   ("APP_UDP_TASK: Remote IP %d.%d.%d.%d \r\n", 
                                    socket_info.remoteIPaddress.v4Add.v[0], 
                                    socket_info.remoteIPaddress.v4Add.v[1], 
                                    socket_info.remoteIPaddress.v4Add.v[2], 
                                    socket_info.remoteIPaddress.v4Add.v[3]
                                );
            SYS_CONSOLE_PRINT   ("APP_UDP_TASK: Local  port %d \r\n", 
                                    socket_info.localPort
                                );
            SYS_CONSOLE_PRINT   ("APP_UDP_TASK: Remote port %d \r\n", 
                                    socket_info.remotePort
                                );
            SYS_CONSOLE_PRINT   ("APP_UDP_TASK: rxQueue size %d \r\n", 
                                    socket_info.rxQueueSize
                                );
            SYS_CONSOLE_PRINT   ("APP_UDP_TASK: tx size %d \r\n", 
                                    socket_info.txSize
                                );
        #endif
        //--------------------------------------------------------------
        #ifdef ENABLE_CONSOLE_MESSAGE
            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> UDP_STATE_SERVER_LISTEN\r\n");
        #endif
        app_udp_taskData.error = APP_UDP_TASK_ERROR_NO;
        result = APP_UDP_TASK_STATE_SERVER_LISTEN;

//        #ifdef ENABLE_CONSOLE_MESSAGE
//            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> UDP_STATE_WAIT_CLIENT_OPEN\r\n");
//        #endif
//        app_udp_taskData.state = APP_UDP_TASK_STATE_WAIT_CLIENT_OPEN;
//        app_udp_taskData.error = APP_UDP_TASK_ERROR_NO;
//
//        #ifdef ENABLE_CONSOLE_MESSAGE
//            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> UDP_STATE_Rx\r\n");
//        #endif
//        app_udp_taskData.state = APP_UDP_TASK_STATE_Rx;
//        app_udp_taskData.error = APP_UDP_TASK_ERROR_NO;
    }
    //-----------------------------------------------------------------
    return result;
}
//------------------------------------------------------------------------------
APP_UDP_TASK_STATES app_udp_task_server_listen(void)
{
    //------------------------------------------------------------------
    uint16_t read_len;
    APP_UDP_TASK_STATES result = app_udp_taskData.state;
    //------------------------------------------------------------------
    // <editor-fold defaultstate="collapsed" desc="Wait until Console Buffer become free">
    wait_console_buffer_free();
    // </editor-fold>
    //------------------------------------------------------------------
    read_len = TCPIP_UDP_GetIsReady(app_udp_taskData.udp_rx_socket);

    if (read_len > 0)
    {
        app_udp_taskData.event_info.pData = malloc(read_len);
        configASSERT(app_udp_taskData.event_info.pData);

        app_udp_taskData.event_info.data_len = TCPIP_UDP_ArrayGet(app_udp_taskData.udp_rx_socket, app_udp_taskData.event_info.pData, read_len);
        app_udp_taskData.event_info.event_id = (ENUM_EVENT_TYPE)EVENT_TYPE_AMAK_UDP_POCKET;

        UDP_SOCKET_INFO socket_info;
        TCPIP_UDP_SocketInfoGet(app_udp_taskData.udp_rx_socket, &socket_info);

        #ifdef ENABLE_CONSOLE_MESSAGE
        {
            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: an UDP-packet receive\r\n");

            TCPIP_NET_HANDLE    netH;
            IPV4_ADDR           ipAddr;
            const char          *netName;
            const char          *netBiosName;
            char                str_ip_adr[20];

            netH        = socket_info.hNet;
            netName     = TCPIP_STACK_NetNameGet (netH);
            netBiosName = TCPIP_STACK_NetBIOSName(netH);
            ipAddr.Val  = TCPIP_STACK_NetAddress (netH);

            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: --== Rx socket ==--\r\n");

            SYS_CONSOLE_PRINT("    Interface %s on host %s \r\n", netName, netBiosName);
            TCPIP_Helper_IPAddressToString( &ipAddr, str_ip_adr, sizeof(str_ip_adr) );
            SYS_CONSOLE_PRINT(" IP Address: %s \r\n", str_ip_adr);
            TCPIP_Helper_IPAddressToString( &(socket_info.destIPaddress.v4Add), str_ip_adr, sizeof(str_ip_adr) );
            SYS_CONSOLE_PRINT("APP_UDP_TASK: Dest   IP %s \r\n", str_ip_adr);
            TCPIP_Helper_IPAddressToString( &(socket_info.sourceIPaddress.v4Add), str_ip_adr, sizeof(str_ip_adr) );
            SYS_CONSOLE_PRINT("APP_UDP_TASK: Source IP %s \r\n", str_ip_adr);
            TCPIP_Helper_IPAddressToString( &(socket_info.localIPaddress.v4Add), str_ip_adr, sizeof(str_ip_adr) );
            SYS_CONSOLE_PRINT("APP_UDP_TASK: Local  IP %s \r\n", str_ip_adr);
            TCPIP_Helper_IPAddressToString( &(socket_info.remoteIPaddress.v4Add), str_ip_adr, sizeof(str_ip_adr) );
            SYS_CONSOLE_PRINT("APP_UDP_TASK: Remote IP %s \r\n", str_ip_adr);
            SYS_CONSOLE_PRINT("APP_UDP_TASK: Local  port %d \r\n",  socket_info.localPort);
            SYS_CONSOLE_PRINT("APP_UDP_TASK: Remote port %d \r\n",  socket_info.remotePort);
            SYS_CONSOLE_PRINT("APP_UDP_TASK: rxQueue size %d \r\n", socket_info.rxQueueSize);
            SYS_CONSOLE_PRINT("APP_UDP_TASK: tx size %d \r\n", socket_info.txSize);
        }
        #endif

        app_udp_taskData.dest_port = socket_info.remotePort;
        app_udp_taskData.dest_adr.v4Add.Val = socket_info.sourceIPaddress.v4Add.Val;
//        app_udp_taskData.dest_adr.v4Add.Val = socket_info.remoteIPaddress.v4Add.Val;

        TCPIP_UDP_Discard(app_udp_taskData.udp_rx_socket);

        free(app_udp_taskData.event_info.pData);
        app_udp_taskData.event_info.data_len = 0;
//        xQueueSend( eventQueue_app_amak_parser_task, (void*)&( app_udp_taskData.event_info ), 0 );//portMAX_DELAY); 
        //--------------------------------------------------------------
        #ifdef ENABLE_CONSOLE_MESSAGE
            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> UDP_STATE_WAIT_CLIENT_OPEN\r\n");
        #endif
        app_udp_taskData.error = APP_UDP_TASK_ERROR_NO;
        result = APP_UDP_TASK_STATE_WAIT_CLIENT_OPEN;
        //--------------------------------------------------------------
    }
    //------------------------------------------------------------------
    return result;
}
//------------------------------------------------------------------------------
APP_UDP_TASK_STATES app_udp_task_wait_client_open(void)
{
    APP_UDP_TASK_STATES result = app_udp_taskData.state;
    //------------------------------------------------------------------
    // <editor-fold defaultstate="collapsed" desc="Wait until Console Buffer become free">
    wait_console_buffer_free();
    // </editor-fold>
    //------------------------------------------------------------------
    app_udp_taskData.udp_tx_socket = TCPIP_UDP_ClientOpen( IP_ADDRESS_TYPE_IPV4, app_udp_taskData.dest_port, &(app_udp_taskData.dest_adr) );
    if (INVALID_SOCKET != app_udp_taskData.udp_tx_socket)
    {
        #ifdef ENABLE_CONSOLE_MESSAGE
            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP client open: Ok \r\n");
        #endif
        #ifdef ENABLE_CONSOLE_MESSAGE
            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP bind: ");
        #endif
        int dog_time = 100;
        while ( true != TCPIP_UDP_Bind( app_udp_taskData.udp_tx_socket, IP_ADDRESS_TYPE_IPV4, app_udp_taskData.local_port, &(app_udp_taskData.local_adr) ) )
        {
            #ifdef ENABLE_CONSOLE_MESSAGE
                SYS_CONSOLE_MESSAGE(".");
            #endif
            dog_time--;
            if (0 == dog_time)
            {
                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_MESSAGE("\r\n APP_UDP_TASK: UDP bind: Error! \r\n");
                #endif
                break;
            }
            taskYIELD();
        }
        #ifdef ENABLE_CONSOLE_MESSAGE
            if (0 < dog_time) SYS_CONSOLE_MESSAGE("\r\n APP_UDP_TASK: UDP bind: Ok \r\n");
        #endif

//        if ( true == TCPIP_UDP_Bind( app_udp_taskData.udp_tx_socket, IP_ADDRESS_TYPE_IPV4, app_udp_taskData.local_port, &(app_udp_taskData.local_adr) ) )
////        if ( true == TCPIP_UDP_Bind( app_udp_taskData.udp_tx_socket, IP_ADDRESS_TYPE_IPV4, 1501, &(app_udp_taskData.local_adr) ) )
////        if ( true == TCPIP_UDP_RemoteBind( app_udp_taskData.udp_tx_socket, IP_ADDRESS_TYPE_IPV4, app_udp_taskData.dest_port, &(app_udp_taskData.dest_adr) ) )
//        {
//            #ifdef ENABLE_CONSOLE_MESSAGE
//                SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP bind: Ok \r\n");
//            #endif
//        }
//        else
//        {
//            #ifdef ENABLE_CONSOLE_MESSAGE
//                SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP bind: Error! \r\n");
//            #endif
//        }
        #ifdef ENABLE_CONSOLE_MESSAGE
            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> UDP_STATE_WAIT_FOR_CONNECTION\r\n");
        #endif

        app_udp_taskData.error = APP_UDP_TASK_ERROR_NO;
        result = APP_UDP_TASK_STATE_WAIT_FOR_CONNECTION;
    }
    return result;
}
//------------------------------------------------------------------------------
APP_UDP_TASK_STATES app_udp_task_wait_for_connection(void)
{
    APP_UDP_TASK_STATES result = app_udp_taskData.state;
    //------------------------------------------------------------------
    // <editor-fold defaultstate="collapsed" desc="Wait until Console Buffer become free">
    wait_console_buffer_free();
    // </editor-fold>
    //------------------------------------------------------------------
    if ( true == TCPIP_UDP_IsConnected(app_udp_taskData.udp_tx_socket) )
//    if (TCPIP_UDP_IsConnected(app_udp_taskData.udp_rx_socket))
    {
        #ifdef ENABLE_CONSOLE_MESSAGE
            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: TCPIP_UDP_IsConnected\r\n");
        #endif
        //-----------------------------------------------------------------
        #ifdef ENABLE_CONSOLE_MESSAGE
            const char          *netName;
            const char          *netBiosName;
            UDP_SOCKET_INFO     socket_info;
            TCPIP_NET_HANDLE    netH;
            IPV4_ADDR           ipAddr;

            TCPIP_UDP_SocketInfoGet(app_udp_taskData.udp_tx_socket, &socket_info);

            netH = socket_info.hNet;
            netName = TCPIP_STACK_NetNameGet(netH);
            netBiosName = TCPIP_STACK_NetBIOSName(netH);
            ipAddr.Val = TCPIP_STACK_NetAddress(netH);

            SYS_CONSOLE_MESSAGE (" APP_UDP_TASK: --== Tx socket ==--\r\n");
            SYS_CONSOLE_PRINT   ("    Interface %s on host %s \r\n", netName, netBiosName);
            SYS_CONSOLE_MESSAGE ("    IP Address: ");
            SYS_CONSOLE_PRINT   ("%d.%d.%d.%d \r\n", 
                                    ipAddr.v[0], 
                                    ipAddr.v[1], 
                                    ipAddr.v[2], 
                                    ipAddr.v[3]
                                );
            SYS_CONSOLE_PRINT   ("    Dest   IP %d.%d.%d.%d \r\n", 
                                    socket_info.destIPaddress.v4Add.v[0], 
                                    socket_info.destIPaddress.v4Add.v[1], 
                                    socket_info.destIPaddress.v4Add.v[2], 
                                    socket_info.destIPaddress.v4Add.v[3]
                                );
            SYS_CONSOLE_PRINT   ("    Source IP %d.%d.%d.%d \r\n", 
                                    socket_info.sourceIPaddress.v4Add.v[0], 
                                    socket_info.sourceIPaddress.v4Add.v[1], 
                                    socket_info.sourceIPaddress.v4Add.v[2], 
                                    socket_info.sourceIPaddress.v4Add.v[3]
                                );
            SYS_CONSOLE_PRINT   ("    Local  IP %d.%d.%d.%d \r\n", 
                                    socket_info.localIPaddress.v4Add.v[0], 
                                    socket_info.localIPaddress.v4Add.v[1], 
                                    socket_info.localIPaddress.v4Add.v[2], 
                                    socket_info.localIPaddress.v4Add.v[3]
                                );
            SYS_CONSOLE_PRINT   ("    Remote IP %d.%d.%d.%d \r\n", 
                                    socket_info.remoteIPaddress.v4Add.v[0], 
                                    socket_info.remoteIPaddress.v4Add.v[1], 
                                    socket_info.remoteIPaddress.v4Add.v[2], 
                                    socket_info.remoteIPaddress.v4Add.v[3]
                                );
            SYS_CONSOLE_PRINT   ("    Local  port %d \r\n", 
                                    socket_info.localPort
                                );
            SYS_CONSOLE_PRINT   ("    Remote port %d \r\n", 
                                    socket_info.remotePort
                                );
            SYS_CONSOLE_PRINT   ("    rxQueue size %d \r\n", 
                                    socket_info.rxQueueSize
                                );
            SYS_CONSOLE_PRINT   ("    tx size %d \r\n", 
                                    socket_info.txSize
                                );
            SYS_CONSOLE_PRINT   (">> ", socket_info.txSize);
        #endif
        //-----------------------------------------------------------------
        #ifdef ENABLE_CONSOLE_MESSAGE
            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> UDP_STATE_Rx\r\n");
        #endif
        app_udp_taskData.error = APP_UDP_TASK_ERROR_NO;
        result = APP_UDP_TASK_STATE_Rx;

//        #ifdef ENABLE_CONSOLE_MESSAGE
//            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> UDP_STATE_WAIT_SERVER_OPEN\r\n");
//        #endif
//        app_udp_taskData.error = APP_UDP_TASK_ERROR_NO;
//        result = APP_UDP_TASK_STATE_WAIT_SERVER_OPEN;
    }
    return result;
}
//------------------------------------------------------------------------------
APP_UDP_TASK_STATES app_udp_task_rx(void)
{
    //------------------------------------------------------------------
    uint16_t read_len;
    APP_UDP_TASK_STATES result = APP_UDP_TASK_STATE_Tx;
    //------------------------------------------------------------------
    // <editor-fold defaultstate="collapsed" desc="Wait until Console Buffer become free">
    wait_console_buffer_free();
    // </editor-fold>
    //------------------------------------------------------------------
    read_len = TCPIP_UDP_GetIsReady(app_udp_taskData.udp_rx_socket);

    if (read_len > 0)
    {
        app_udp_taskData.event_info.pData = malloc(read_len);
        configASSERT(app_udp_taskData.event_info.pData);

        app_udp_taskData.event_info.data_len = TCPIP_UDP_ArrayGet(app_udp_taskData.udp_rx_socket, app_udp_taskData.event_info.pData, read_len);
        app_udp_taskData.event_info.event_id = (ENUM_EVENT_TYPE)EVENT_TYPE_AMAK_UDP_POCKET;

        #ifdef ENABLE_CONSOLE_MESSAGE
        {
            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: an UDP-packet receive\r\n");
            UDP_SOCKET_INFO socket_info;
            TCPIP_UDP_SocketInfoGet(app_udp_taskData.udp_rx_socket, &socket_info);

            TCPIP_NET_HANDLE    netH;
            IPV4_ADDR           ipAddr;
            const char          *netName;
            const char          *netBiosName;
            char                str_ip_adr[20];

            netH = socket_info.hNet;
            netName = TCPIP_STACK_NetNameGet(netH);
            netBiosName = TCPIP_STACK_NetBIOSName(netH);
            ipAddr.Val = TCPIP_STACK_NetAddress(netH);

            SYS_CONSOLE_MESSAGE("    --== Rx socket ==--\r\n");

            SYS_CONSOLE_PRINT("    Interface %s on host %s \r\n", netName, netBiosName);
            TCPIP_Helper_IPAddressToString( &ipAddr, str_ip_adr, sizeof(str_ip_adr) );
            SYS_CONSOLE_PRINT("    IP Address: %s \r\n", str_ip_adr);
            TCPIP_Helper_IPAddressToString( &(socket_info.destIPaddress.v4Add), str_ip_adr, sizeof(str_ip_adr) );
            SYS_CONSOLE_PRINT("    Dest   IP %s \r\n", str_ip_adr);
            TCPIP_Helper_IPAddressToString( &(socket_info.sourceIPaddress.v4Add), str_ip_adr, sizeof(str_ip_adr) );
            SYS_CONSOLE_PRINT("    Source IP %s \r\n", str_ip_adr);
            TCPIP_Helper_IPAddressToString( &(socket_info.localIPaddress.v4Add), str_ip_adr, sizeof(str_ip_adr) );
            SYS_CONSOLE_PRINT("    Local  IP %s \r\n", str_ip_adr);
            TCPIP_Helper_IPAddressToString( &(socket_info.remoteIPaddress.v4Add), str_ip_adr, sizeof(str_ip_adr) );
            SYS_CONSOLE_PRINT("    Remote IP %s \r\n", str_ip_adr);
            SYS_CONSOLE_PRINT("    Local  port %d \r\n",  socket_info.localPort);
            SYS_CONSOLE_PRINT("    Remote port %d \r\n",  socket_info.remotePort);
            SYS_CONSOLE_PRINT("    rxQueue size %d \r\n", socket_info.rxQueueSize);
            SYS_CONSOLE_PRINT("    tx size %d \r\n", socket_info.txSize);

        }
        #endif

        TCPIP_UDP_Discard(app_udp_taskData.udp_rx_socket);

        free(app_udp_taskData.event_info.pData);
        app_udp_taskData.event_info.data_len = 0;
//        xQueueSend( eventQueue_app_amak_parser_task, (void*)&( app_udp_taskData.event_info ), 0 );//portMAX_DELAY); 

    }
    //------------------------------------------------------------------
    read_len = TCPIP_UDP_GetIsReady(app_udp_taskData.udp_tx_socket);

    if (read_len > 0)
    {
        app_udp_taskData.event_info.pData = malloc(read_len);
        configASSERT(app_udp_taskData.event_info.pData);

        app_udp_taskData.event_info.data_len = TCPIP_UDP_ArrayGet(app_udp_taskData.udp_tx_socket, app_udp_taskData.event_info.pData, read_len);
        app_udp_taskData.event_info.event_id = (ENUM_EVENT_TYPE)EVENT_TYPE_AMAK_UDP_POCKET;

        #ifdef ENABLE_CONSOLE_MESSAGE
        {
            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: an UDP-packet receive\r\n");
            UDP_SOCKET_INFO socket_info;
            TCPIP_UDP_SocketInfoGet(app_udp_taskData.udp_tx_socket, &socket_info);

            TCPIP_NET_HANDLE    netH;
            IPV4_ADDR           ipAddr;
            const char          *netName;
            const char          *netBiosName;
            char                str_ip_adr[20];

            netH = socket_info.hNet;
            netName = TCPIP_STACK_NetNameGet(netH);
            netBiosName = TCPIP_STACK_NetBIOSName(netH);
            ipAddr.Val = TCPIP_STACK_NetAddress(netH);

            SYS_CONSOLE_MESSAGE("    --== Tx socket ==--\r\n");

            SYS_CONSOLE_PRINT("    Interface %s on host %s \r\n", netName, netBiosName);
            TCPIP_Helper_IPAddressToString( &ipAddr, str_ip_adr, sizeof(str_ip_adr) );
            SYS_CONSOLE_PRINT("    IP Address: %s \r\n", str_ip_adr);
            TCPIP_Helper_IPAddressToString( &(socket_info.destIPaddress.v4Add), str_ip_adr, sizeof(str_ip_adr) );
            SYS_CONSOLE_PRINT("    Dest   IP %s \r\n", str_ip_adr);
            TCPIP_Helper_IPAddressToString( &(socket_info.sourceIPaddress.v4Add), str_ip_adr, sizeof(str_ip_adr) );
            SYS_CONSOLE_PRINT("    Source IP %s \r\n", str_ip_adr);
            TCPIP_Helper_IPAddressToString( &(socket_info.localIPaddress.v4Add), str_ip_adr, sizeof(str_ip_adr) );
            SYS_CONSOLE_PRINT("    Local  IP %s \r\n", str_ip_adr);
            TCPIP_Helper_IPAddressToString( &(socket_info.remoteIPaddress.v4Add), str_ip_adr, sizeof(str_ip_adr) );
            SYS_CONSOLE_PRINT("    Remote IP %s \r\n", str_ip_adr);
            SYS_CONSOLE_PRINT("    Local  port %d \r\n",  socket_info.localPort);
            SYS_CONSOLE_PRINT("    Remote port %d \r\n",  socket_info.remotePort);
            SYS_CONSOLE_PRINT("    rxQueue size %d \r\n", socket_info.rxQueueSize);
            SYS_CONSOLE_PRINT("    tx size %d \r\n", socket_info.txSize);

        }
        #endif

        TCPIP_UDP_Discard(app_udp_taskData.udp_tx_socket);

        free(app_udp_taskData.event_info.pData);
        app_udp_taskData.event_info.data_len = 0;
//                xQueueSend( eventQueue_app_amak_parser_task, (void*)&( app_udp_taskData.event_info ), 0 );//portMAX_DELAY); 
    }
    //------------------------------------------------------------------
    return result;
}
//------------------------------------------------------------------------------
APP_UDP_TASK_STATES app_udp_task_tx(void)
{
    APP_UDP_TASK_STATES result = APP_UDP_TASK_STATE_Rx;

    app_udp_taskData.event_info.event_id = (ENUM_EVENT_TYPE)EVENT_TYPE_UNKNOWN;
    //------------------------------------------------------------------
    // РџСЂРѕРІРµСЂСЏРµРј РµСЃС‚СЊ Р»Рё РІ РѕС‡РµСЂРµРґРё РґР°РЅРЅС‹Рµ РЅР° РїРµСЂРµРґР°С‡Сѓ
    //------------------------------------------------------------------
    xQueueReceive( eventQueue_app_udp_task, &app_udp_taskData.event_info, 0 ); //portMAX_DELAY );

    //------------------------------------------------------------------
    // Р•СЃР»Рё РґР°РЅРЅС‹Рµ РІ РѕС‡РµСЂРµРґРё РѕС‚ App_AMAK_Parser_Task РёР»Рё РѕС‚ App_Service_UART_Task
    //------------------------------------------------------------------
    if ( ( (ENUM_EVENT_TYPE)EVENT_TYPE_AMAK_UDP_POCKET     == app_udp_taskData.event_info.event_id ) ||
         ( (ENUM_EVENT_TYPE)EVENT_TYPE_UART_SERVICE_POCKET == app_udp_taskData.event_info.event_id ) )
    {
        //------------------------------------------------------------------
        // <editor-fold defaultstate="collapsed" desc="Wait until Console Buffer become free">
        wait_console_buffer_free();
        // </editor-fold>
        //------------------------------------------------------------------
        #ifdef ENABLE_CONSOLE_MESSAGE
            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP-packet send start\r\n");
        #endif
        //--------------------------------------------------------------
        UDP_SOCKET_INFO socket_info;
//                TCPIP_UDP_SocketInfoGet(app_udp_taskData.udp_rx_socket, &socket_info);
        TCPIP_UDP_SocketInfoGet(app_udp_taskData.udp_tx_socket, &socket_info);
        #ifdef ENABLE_CONSOLE_MESSAGE
            const char          *netName;
            const char          *netBiosName;
            char                str_ip_adr[20];
            TCPIP_NET_HANDLE    netH;
            IPV4_ADDR           ipAddr;

            netH = socket_info.hNet;
            netName = TCPIP_STACK_NetNameGet(netH);
            netBiosName = TCPIP_STACK_NetBIOSName(netH);
            ipAddr.Val = TCPIP_STACK_NetAddress(netH);

            SYS_CONSOLE_MESSAGE("    --== Tx socket ==--\r\n");

            SYS_CONSOLE_PRINT("    Interface %s on host %s \r\n", netName, netBiosName);
            TCPIP_Helper_IPAddressToString( &ipAddr, str_ip_adr, sizeof(str_ip_adr) );
            SYS_CONSOLE_PRINT("    IP Address: %s \r\n", str_ip_adr);
            TCPIP_Helper_IPAddressToString( &(socket_info.destIPaddress.v4Add), str_ip_adr, sizeof(str_ip_adr) );
            SYS_CONSOLE_PRINT("    Dest   IP %s \r\n", str_ip_adr);
            TCPIP_Helper_IPAddressToString( &(socket_info.sourceIPaddress.v4Add), str_ip_adr, sizeof(str_ip_adr) );
            SYS_CONSOLE_PRINT("    Source IP %s \r\n", str_ip_adr);
            TCPIP_Helper_IPAddressToString( &(socket_info.localIPaddress.v4Add), str_ip_adr, sizeof(str_ip_adr) );
            SYS_CONSOLE_PRINT("    Local  IP %s \r\n", str_ip_adr);
            TCPIP_Helper_IPAddressToString( &(socket_info.remoteIPaddress.v4Add), str_ip_adr, sizeof(str_ip_adr) );
            SYS_CONSOLE_PRINT("    Remote IP %s \r\n", str_ip_adr);
            SYS_CONSOLE_PRINT("    Local  port %d \r\n",  socket_info.localPort);
            SYS_CONSOLE_PRINT("    Remote port %d \r\n",  socket_info.remotePort);
            SYS_CONSOLE_PRINT("    rxQueue size %d \r\n", socket_info.rxQueueSize);
            SYS_CONSOLE_PRINT("    tx size %d \r\n", socket_info.txSize);

        #endif
        //--------------------------------------------------------------
        uint16_t udp_buf_len;
//                udp_buf_len = TCPIP_UDP_PutIsReady(app_udp_taskData.udp_rx_socket);
        udp_buf_len = TCPIP_UDP_PutIsReady(app_udp_taskData.udp_tx_socket);
        #ifdef ENABLE_CONSOLE_MESSAGE
            SYS_CONSOLE_PRINT("APP_UDP_TASK: tx_bufer size = %d \r\n", udp_buf_len);
        #endif
        if (udp_buf_len >= app_udp_taskData.event_info.data_len)
        {
//                    TCPIP_UDP_ArrayPut(app_udp_taskData.udp_rx_socket, app_udp_taskData.event_info.pData, app_udp_taskData.event_info.data_len);
//                    TCPIP_UDP_DestinationIPAddressSet( app_udp_taskData.udp_rx_socket, IP_ADDRESS_TYPE_IPV4, &(app_udp_taskData.dest_adr) );
//                    TCPIP_UDP_DestinationPortSet( app_udp_taskData.udp_rx_socket, app_udp_taskData.dest_port );

            TCPIP_UDP_ArrayPut(app_udp_taskData.udp_tx_socket, app_udp_taskData.event_info.pData, app_udp_taskData.event_info.data_len);
//                    TCPIP_UDP_DestinationIPAddressSet( app_udp_taskData.udp_tx_socket, IP_ADDRESS_TYPE_IPV4, &(app_udp_taskData.dest_adr) );
//                    TCPIP_UDP_DestinationPortSet( app_udp_taskData.udp_tx_socket, app_udp_taskData.dest_port );


//                    if ( 0 == TCPIP_UDP_Flush(app_udp_taskData.udp_rx_socket) )
            if ( 0 == TCPIP_UDP_Flush(app_udp_taskData.udp_tx_socket) )
            {
                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP-packet send not complite!\r\n");
                #endif

            }
            else
            {
                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP-packet send complite\r\n");
                #endif

            }
        }
        else
        {
            #ifdef ENABLE_CONSOLE_MESSAGE
                SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: Not enough memory for transmit data\r\n");
            #endif
        }

        free(app_udp_taskData.event_info.pData);
        app_udp_taskData.event_info.data_len = 0;
    }
    return result;
}
//------------------------------------------------------------------------------
APP_UDP_TASK_STATES app_udp_task_closing_connection(void)
{
    APP_UDP_TASK_STATES result = app_udp_taskData.state;

    return result;
}
//------------------------------------------------------------------------------
APP_UDP_TASK_STATES app_udp_task_error(void)
{
    APP_UDP_TASK_STATES result = app_udp_taskData.state;

    return result;
}
//------------------------------------------------------------------------------

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">

  Function:
    bool <FunctionName> (TCPIP_NET_HANDLE hNet, struct _tag_TCPIP_MAC_PACKET* rxPkt, uint16_t frameType, const void* hParam);

  Summary:
    Pointer to a function(handler) that will get called to process an incoming packet.

  Description:
    Pointer to a function that will be called by the TCP/IP manager
    when a RX packet is available.

  Precondition:
    None

  Parameters:
    hNet        - network handle on which the packet has arrived
    rxPkt       - pointer to incoming packet
    frameType   - type of packet being processed
                  Note: value is converted to host endianess!
                  Standard Ethernet frame value: 0x0800 - IPV4, 0x86DD - IPv6, 0x0806 - ARP, etc. 
    hParam      - user passed parameter when handler was registered

  Returns:
    true - if the packet is processed by the external handler.
           In this case the TCP/IP manager will no longer process the packet
    false - the packet needs to be processed internally by the stack as usual           

  Remarks:
    The packet handler is called in the TCP/IP stack manager context.
    The handler should be kept as short as possible as it affects the processing of all the other
    RX traffic.

    Before calling the external packet handler 
    - the rxPkt->pktIf points to the interface receiving the packet
    - no other processing/checks are done!

    Important!
    When the packet handler returns true, once it's done processing the packet,
    it needs to acknowledge it, i.e. return to the owner,
    which is the MAC driver serving the network interface!
    This means that the packet acknowledge function needs to be called,
    with a proper acknowledge parameter and the QUEUED flag needs to be cleared, if needed:
    if((*rxPkt->ackFunc)(rxPkt, rxPkt->ackParam))
    {
           rxPkt->pktFlags &= ~TCPIP_MAC_PKT_FLAG_QUEUED;
    }
    Failure to do that will result in memory leaks and starvation of the MAC driver.
    See the tcpip_mac.h for details.
    
*/
// </editor-fold>
static bool amak_shdsl_mac_packet_handler(TCPIP_NET_HANDLE hNet, TCPIP_MAC_PACKET* pRxPkt, uint16_t frameType, const void* hParam)
{
    bool res;
    
    switch ( frameType )
    {
        case TCPIP_ETHER_TYPE_IPV4:    // IP (v4)  protocol
        {
            res = amak_shdsl_ip_packet_handler(hNet, pRxPkt, hParam);
            break;
        }
        case TCPIP_ETHER_TYPE_ARP:    // ARP protocol
        {
            res = amak_shdsl_arp_packet_handler(hNet, pRxPkt, hParam);
            break;
        }
        default:
        {
            res = false;
        }
    }
    return res;
}

/*<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">

   Function:
    bool <FunctionName> (TCPIP_NET_HANDLE hNet, struct _tag_TCPIP_MAC_PACKET* rxPkt, const void* hParam);

  Summary:
    Pointer to a function(handler) that will get called to process an incoming ARP packet.

  Description:
    Function that will be called by the amak_shdsl_mac_packet_handler()
    when a RX ARP-packet is available.

  Precondition:
    None

  Parameters:
    hNet        - network handle on which the packet has arrived
    rxPkt       - pointer to incoming packet
    hParam      - user passed parameter when handler was registered

  Returns:
    true - if the packet is processed by the external handler.
           In this case the IPv4 module will no longer process the packet
    false - the packet needs to be processed internally by the IPv4 as usual           

  Remarks:
    The packet handler is called in the IPv4 context.
    The handler should be kept as short as possible as it affects the processing of all the other
    IPv4 RX traffic.

    Before calling the external packet handler 
    - the rxPkt->pNetLayer points to an IPV4_HEADER data structure.
    - the rxPkt->pktIf points to the interface receiving the packet
    - no other checks are done! (checksum, versions, etc.)

    Important!
    When the packet handler returns true, once it's done processing the packet,
    it needs to acknowledge it, i.e. return to the owner,
    which is the MAC driver serving the network interface!
    This means that the packet acknowledge function needs to be called,
    with a proper acknowledge parameter and the QUEUED flag needs to be cleared, if needed:
    if((*rxPkt->ackFunc)(rxPkt, rxPkt->ackParam))
    {
           rxPkt->pktFlags &= ~TCPIP_MAC_PKT_FLAG_QUEUED;
    }
    Failure to do that will result in memory leaks and starvation of the MAC driver.
    See the tcpip_mac.h for details.
    
 */
//</editor-fold>
static bool amak_shdsl_arp_packet_handler(TCPIP_NET_HANDLE hNet, TCPIP_MAC_PACKET* pRxPkt, const void* hParam)
{
    #ifdef ENABLE_CONSOLE_MESSAGE
        SYS_CONSOLE_PRINT("   ARP packet\r\n");
    #endif

//    pRxPkt->pktFlags &= ~TCPIP_MAC_PKT_FLAG_TYPE_MASK;
//    pRxPkt->pktFlags |= TCPIP_MAC_PKT_FLAG_ARP;
    //--------------------------------------------------------------------------
    // copy from void TCPIP_ARP_Process(void) in "arp.c"
//    TCPIP_NET_IF* pInIf, *pTgtIf;
//    TCPIP_MAC_PACKET* pPkt;

    ARP_PACKET      *p_arp_pkt;
//    OA_HASH_ENTRY   *hE;
//    ARP_CACHE_DCPT  *pArpDcpt;
    TCPIP_MAC_PKT_ACK_RES ackRes;
//    TCPIP_ARP_RESULT arpReqRes;
//    IPV4_ADDR targetAdd;
//    IPV4_ADDR   algnSenderIpAddr;


//    TCPIP_PKT_FlightLogRx(pPkt, TCPIP_MODULE_ARP);
//    arpReqRes = ARP_RES_OK;

    // Obtain the incoming ARP packet and process
    p_arp_pkt = (ARP_PACKET*)pRxPkt->pNetLayer;

    // Validate the ARP packet
    if ( HW_ETHERNET            != TCPIP_Helper_ntohs(p_arp_pkt->HardwareType) ||
         sizeof(TCPIP_MAC_ADDR) != p_arp_pkt->MACAddrLen  ||
         sizeof(IPV4_ADDR)      != p_arp_pkt->ProtocolLen )
    {
        ackRes = TCPIP_MAC_PKT_ACK_STRUCT_ERR;
        _TCPIP_PKT_PacketAcknowledge(pRxPkt, ackRes, TCPIP_MODULE_ARP);
        return true;    // битый ARP-пакет, удаляем
    }
    //--------------------------------------------------------------------------
    pRxPkt->pktFlags &= ~TCPIP_MAC_PKT_FLAG_TYPE_MASK;
    pRxPkt->pktFlags |= TCPIP_MAC_PKT_FLAG_ARP;

    uint16_t mac_pkt_len = sizeof(ARP_PACKET) + sizeof(TCPIP_MAC_ETHERNET_HEADER);
    uint8_t* p_mac_pkt = pRxPkt->pMacLayer;
    
    send_packet_2_amak_parser(pRxPkt, p_mac_pkt, mac_pkt_len);
    
//    uint16_t    total_length;
//    total_length = sizeof(ARP_PACKET);
//
//    send_packet_2_amak_parser(pRxPkt, (uint8_t*)p_arp_pkt, total_length);

    return false;   // дополнительно проводим стандартную обработку ARP-запросов в стандартном обработчике
}

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">
 IPv4 packet handler Pointer

   Function:
    bool <FunctionName> (TCPIP_NET_HANDLE hNet, struct _tag_TCPIP_MAC_PACKET* rxPkt, const void* hParam);

  Summary:
    Pointer to a function(handler) that will get called to process an incoming IPv4 packet.

  Description:
    Pointer to a function that will be called by the IPv4 module
    when a RX packet is available.

  Precondition:
    None

  Parameters:
    hNet        - network handle on which the packet has arrived
    rxPkt       - pointer to incoming packet
    hParam      - user passed parameter when handler was registered

  Returns:
    true - if the packet is processed by the external handler.
           In this case the IPv4 module will no longer process the packet
    false - the packet needs to be processed internally by the IPv4 as usual           

  Remarks:
    The packet handler is called in the IPv4 context.
    The handler should be kept as short as possible as it affects the processing of all the other
    IPv4 RX traffic.

    Before calling the external packet handler 
    - the rxPkt->pNetLayer points to an IPV4_HEADER data structure.
    - the rxPkt->pktIf points to the interface receiving the packet
    - no other checks are done! (checksum, versions, etc.)

    Important!
    When the packet handler returns true, once it's done processing the packet,
    it needs to acknowledge it, i.e. return to the owner,
    which is the MAC driver serving the network interface!
    This means that the packet acknowledge function needs to be called,
    with a proper acknowledge parameter and the QUEUED flag needs to be cleared, if needed:
    if((*rxPkt->ackFunc)(rxPkt, rxPkt->ackParam))
    {
           rxPkt->pktFlags &= ~TCPIP_MAC_PKT_FLAG_QUEUED;
    }
    Failure to do that will result in memory leaks and starvation of the MAC driver.
    See the tcpip_mac.h for details.
    
 */
//</editor-fold>
static bool amak_shdsl_ip_packet_handler(TCPIP_NET_HANDLE hNet, TCPIP_MAC_PACKET* pRxPkt, const void* hParam)
{
    IPV4_HEADER*    p_ip_hdr;
    bool            res;
    
    #ifdef ENABLE_CONSOLE_MESSAGE
        SYS_CONSOLE_PRINT("   IP packet\r\n");
    #endif

    p_ip_hdr = (IPV4_HEADER*)pRxPkt->pNetLayer;
    
    //--------------------------------------------------------------------------
    // copy from void TCPIP_IPV4_Process(void) in "ipv4.c"
//    #define TCPIP_THIS_MODULE_ID    TCPIP_MODULE_IPV4

    // This is left shifted by 4.  Actual value is 0x04.
    #define IPv4_VERSION        (0x04)

//    TCPIP_NET_IF* pNetIf;
//    TCPIP_MAC_PACKET* pRxPkt;
    uint8_t     header_len;
    uint16_t    header_checksum;
    uint16_t    total_length;
    uint16_t    payload_len;
    IPV4_HEADER cIpv4Hdr, *pCHeader;
//    IPV4_PKT_PROC_TYPE procType;
    TCPIP_MAC_PKT_ACK_RES ackRes;

    while(true)
    {
        ackRes = TCPIP_MAC_PKT_ACK_NONE;

        // Make sure that this is an IPv4 packet.
        if ( IPv4_VERSION != (p_ip_hdr->Version) )
        {
            ackRes = TCPIP_MAC_PKT_ACK_STRUCT_ERR;
            break;
        }

        // make sure the header length is within packet limits
        header_len = p_ip_hdr->IHL << 2;
//        if ( ( header_len < sizeof(IPV4_HEADER) ) || ( (uint16_t)header_len > pRxPkt->pDSeg->segLen ) )
        if ( is_ipv4_hdr_len_in_packet_limits(pRxPkt, header_len) )
        {
            ackRes = TCPIP_MAC_PKT_ACK_STRUCT_ERR;
            break;
        }

        total_length = TCPIP_Helper_ntohs(p_ip_hdr->TotalLength);
        if (total_length < (uint16_t)header_len)
        {
            ackRes = TCPIP_MAC_PKT_ACK_STRUCT_ERR;
            break;
        }

        payload_len = TCPIP_PKT_PayloadLen(pRxPkt);
        if (total_length > payload_len)
        {
            ackRes = TCPIP_MAC_PKT_ACK_STRUCT_ERR;
            break;
        }

        // detect the proper alias interface
        //pNetIf = _TCPIPStackMapAliasInterface((TCPIP_NET_IF*)pRxPkt->pktIf, &p_ip_hdr->DestAddress);    // ?!
        //pRxPkt->pktIf = pNetIf;                                                                         // ?!

        // because it is Ethernet/SHDSL bridge
        //if ( !TCPIP_STACK_NetworkIsUp(pNetIf) )
        //{   // discard the packet
        //    ackRes = TCPIP_MAC_PKT_ACK_IP_REJECT_ERR;
        //    break;
        //}

        //----------------------------------------------------------------------
        // discard wrong source address
        // ----------------------------------------------------------------
        // if(_TCPIPStack_IsBcastAddress(pNetIf, &p_ip_hdr->SourceAddress))
        // ----------------------------------------------------------------
        // restrict broadcast check because packet send over Ethernet/SHDSL 
        // bridge and DirectedBcast check is uncorrect
        if ( _TCPIPStack_IsLimitedBcast(&p_ip_hdr->SourceAddress) ) 
        {   // net or limited bcast
            ackRes = TCPIP_MAC_PKT_ACK_SOURCE_ERR;
            break;
        }
        //----------------------------------------------------------------------

        // discard wrong destination address
        if ( 0 == p_ip_hdr->DestAddress.Val)
        {   // invalid destination
            ackRes = TCPIP_MAC_PKT_ACK_DEST_ERR;
            break;
        }

        // Validate the IP header.  If it is correct, the checksum 
        // will come out to 0x0000 (because the header contains a 
        // precomputed checksum).  A corrupt header will have a 
        // nonzero checksum.
        header_checksum = TCPIP_Helper_CalcIPChecksum((uint8_t*)p_ip_hdr, header_len, 0);

        if (header_checksum)
        {
            // Bad packet. The function caller will be notified by means of the false 
            // return value and it should discard the packet.
            ackRes = TCPIP_MAC_PKT_ACK_CHKSUM_ERR;
            break;
        }

        // Make a copy of the header for the network to host conversion
        cIpv4Hdr = *p_ip_hdr;
        pCHeader = &cIpv4Hdr;
        pCHeader->TotalLength = total_length;
        pCHeader->FragmentInfo.val = TCPIP_Helper_ntohs(pCHeader->FragmentInfo.val);

#if (_TCPIP_IPV4_FRAGMENTATION == 0)
        // Throw this packet away if it is a fragment.  
        // We don't support IPv4 fragment reconstruction.
//        if ( (pCHeader->FragmentInfo.MF != 0) || (pCHeader->FragmentInfo.fragOffset != 0) )
        if ( is_fragment_ipv4(pCHeader) )
        {   // discard the fragment
            ackRes = TCPIP_MAC_PKT_ACK_STRUCT_ERR;
            break;
        }
#endif  // (_TCPIP_IPV4_FRAGMENTATION == 0)

        //------------------------------------------------------------------
        // заглушка функции TCPIP_IPV4_CheckRxPkt() при выключеной отладке 
        // протокола TCPIP_IPV4, определена в "ipv4.c"
        //------------------------------------------------------------------
        //  TCPIP_IPV4_CheckRxPkt(pRxPkt);  
        //------------------------------------------------------------------

        //------------------------------------------------------------------------------
        // In Ethernet/SHDSL bridge we do not check the packet arrived on the proper interface and passes the filters
        //------------------------------------------------------------------------------
        //    procType = TCPIP_IPV4_VerifyPkt(pNetIf, pCHeader, pRxPkt);
        //
        //    if((procType & IPV4_PKT_DEST_HOST) == 0)
        //    {   // not processed internally; but some oter module may still need it; check the filters
        //        if(TCPIP_IPV4_VerifyPktFilters(pRxPkt, header_len))
        //        {
        //            procType = IPV4_PKT_DEST_HOST;
        //        }
        //    }
        //
        //    #if (TCPIP_IPV4_FORWARDING_ENABLE != 0)
        //        if((procType & IPV4_PKT_DEST_FWD) != 0)
        //        {   // packet to be forwarded
        //            if(TCPIP_IPV4_ProcessExtPkt(pNetIf, pRxPkt, procType))
        //            {   // we're done
        //                break;
        //            }
        //        }
        //    #endif  // (TCPIP_IPV4_FORWARDING_ENABLE != 0)
        //
        //    if((procType & (IPV4_PKT_DEST_HOST)) == 0)
        //    {   // discard
        //        ackRes = TCPIP_MAC_PKT_ACK_IP_REJECT_ERR;
        //        break;
        //    }
        //------------------------------------------------------------------------------

        // valid IPv4 packet will be processed below
        //ackRes = TCPIP_IPV4_DispatchPacket(pRxPkt);
        break;
    }

    if (TCPIP_MAC_PKT_ACK_NONE != ackRes)
    {   // something wrong; discard
//        TCPIP_PKT_PacketAcknowledge(pRxPkt, ackRes); 
        _TCPIP_PKT_PacketAcknowledge(pRxPkt, ackRes, TCPIP_MODULE_IPV4);
        
        res = true;
    }
    //--------------------------------------------------------------------------
    else    // if (ackRes != TCPIP_MAC_PKT_ACK_NONE)
    {
        switch ( (IPV4_HEADER_TYPE)(p_ip_hdr->Protocol) )
        {
            case ( IP_PROT_ICMP ):
            {
                res = amak_shdsl_icmp_packet_handler(hNet, pRxPkt, hParam);
                break;
            }
            case ( IP_PROT_UDP):
            {
                res = amak_shdsl_udp_packet_handler(hNet, pRxPkt, hParam);
                break;
            }
            default:
            {
                res = false;
                break;
            }
        }
    }
    return res;
}

/*<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">

   Function:
    bool <FunctionName> (TCPIP_NET_HANDLE hNet, struct _tag_TCPIP_MAC_PACKET* rxPkt, const void* hParam);

  Summary:
    Pointer to a function(handler) that will get called to process an incoming ICMP packet.

  Description:
    Function that will be called by the amak_shdsl_ip_packet_handler()
    when a RX ICMP-packet is available.

  Precondition:
    None

  Parameters:
    hNet        - network handle on which the packet has arrived
    rxPkt       - pointer to incoming packet
    hParam      - user passed parameter when handler was registered

  Returns:
    true - if the packet is processed by the external handler.
           In this case the IPv4 module will no longer process the packet
    false - the packet needs to be processed internally by the IPv4 as usual           

  Remarks:
    The packet handler is called in the IPv4 context.
    The handler should be kept as short as possible as it affects the processing of all the other
    IPv4 RX traffic.

    Before calling the external packet handler 
    - the rxPkt->pNetLayer points to an IPV4_HEADER data structure.
    - the rxPkt->pktIf points to the interface receiving the packet
    - no other checks are done! (checksum, versions, etc.)

    Important!
    When the packet handler returns true, once it's done processing the packet,
    it needs to acknowledge it, i.e. return to the owner,
    which is the MAC driver serving the network interface!
    This means that the packet acknowledge function needs to be called,
    with a proper acknowledge parameter and the QUEUED flag needs to be cleared, if needed:
    if((*rxPkt->ackFunc)(rxPkt, rxPkt->ackParam))
    {
           rxPkt->pktFlags &= ~TCPIP_MAC_PKT_FLAG_QUEUED;
    }
    Failure to do that will result in memory leaks and starvation of the MAC driver.
    See the tcpip_mac.h for details.
    
 */
//</editor-fold>
static bool amak_shdsl_icmp_packet_handler(TCPIP_NET_HANDLE hNet, TCPIP_MAC_PACKET* pRxPkt, const void* hParam)
{
    #ifdef ENABLE_CONSOLE_MESSAGE
        SYS_CONSOLE_PRINT("   ICMP packet\r\n");
    #endif
    
    IPV4_HEADER*    p_ip_hdr;
    uint8_t         header_len;

//    pRxPkt->pktFlags &= ~TCPIP_MAC_PKT_FLAG_TYPE_MASK;
//    pRxPkt->pktFlags |= TCPIP_MAC_PKT_FLAG_IPV4;
//    pRxPkt->pktFlags |= TCPIP_MAC_PKT_FLAG_ICMPV4;
    
//    p_ip_hdr = (IPV4_HEADER*)pRxPkt->pNetLayer;
//    
//    ip_total_length = TCPIP_Helper_ntohs(p_ip_hdr->TotalLength);
//
//    send_packet_2_amak_parser(pRxPkt, (uint8_t*)p_ip_hdr, ip_total_length);
//    //----------------------------------------------------------------------
//    return true;
//    //----------------------------------------------------------------------
//    return false;
    
    //----------------------------------------------------------------------
    // copy from TCPIP_ICMP_Process() in "icmp.c"
    //----------------------------------------------------------------------
    // ICMP Packet Structure
    typedef struct
    {
        uint8_t vType;
        uint8_t vCode;
        uint16_t wChecksum;
        uint16_t wIdentifier;
        uint16_t wSequenceNumber;
        uint32_t wData[];      // payload
    } ICMP_PACKET;

//    TCPIP_MAC_PACKET        *pRxPkt;
//    IPV4_HEADER*            pIpv4Header;
//    uint32_t                srcAdd;
    ICMP_PACKET*            pRxHdr;
    uint16_t                icmpTotLength;
    uint16_t                checksum;
    TCPIP_MAC_PKT_ACK_RES   ackRes;

    TCPIP_PKT_FlightLogRx(pRxPkt, TCPIP_MODULE_ICMP);
    pRxHdr = (ICMP_PACKET*)pRxPkt->pTransportLayer;
    
    ackRes = TCPIP_MAC_PKT_ACK_RX_OK;

    p_ip_hdr = (IPV4_HEADER*)pRxPkt->pNetLayer;
//    srcAdd   =  p_ip_hdr->SourceAddress.Val;

    header_len = p_ip_hdr->IHL << 2;
    pRxPkt->totTransportLen = TCPIP_Helper_ntohs(p_ip_hdr->TotalLength) - header_len;   //?!

    icmpTotLength = pRxPkt->totTransportLen;    // length of the 1st segment (if fragmented)

    if (icmpTotLength < sizeof(*pRxHdr))
    {
        ackRes = TCPIP_MAC_PKT_ACK_STRUCT_ERR;
        _TCPIP_PKT_PacketAcknowledge(pRxPkt, ackRes, TCPIP_MODULE_ICMP);
        return true;
    }

    // Validate the checksum
    // The checksum data includes the precomputed checksum in the header
    // so a valid packet will always have a checksum of 0x0000
    // do it across all fragment segments
#if (_TCPIP_IPV4_FRAGMENTATION != 0)
    TCPIP_MAC_PACKET* pFragPkt;
    checksum = 0;
    for(pFragPkt = pRxPkt; pFragPkt != 0; pFragPkt = pFragPkt->pkt_next)
    {
        checksum = ~TCPIP_Helper_PacketChecksum(pFragPkt, pFragPkt->pTransportLayer, pFragPkt->totTransportLen, checksum);
    }
    checksum = ~checksum;
#else
    checksum = TCPIP_Helper_PacketChecksum(pRxPkt, (uint8_t*)pRxHdr, icmpTotLength, 0);
#endif  // (_TCPIP_IPV4_FRAGMENTATION != 0)

    if(0 != checksum)
    {
        ackRes = TCPIP_MAC_PKT_ACK_CHKSUM_ERR;
        _TCPIP_PKT_PacketAcknowledge(pRxPkt, ackRes, TCPIP_MODULE_ICMP);
        return true;
    }
    //----------------------------------------------------------------------
    pRxPkt->pktFlags &= ~TCPIP_MAC_PKT_FLAG_TYPE_MASK;
    pRxPkt->pktFlags |= TCPIP_MAC_PKT_FLAG_IPV4;
    pRxPkt->pktFlags |= TCPIP_MAC_PKT_FLAG_ICMPV4;

    uint16_t mac_pkt_len = TCPIP_Helper_ntohs(p_ip_hdr->TotalLength) + sizeof(TCPIP_MAC_ETHERNET_HEADER);
    uint8_t* p_mac_pkt = pRxPkt->pMacLayer;
    
    send_packet_2_amak_parser(pRxPkt, p_mac_pkt, mac_pkt_len);
    
//    uint16_t        ip_total_length;
//    ip_total_length = TCPIP_Helper_ntohs(p_ip_hdr->TotalLength);
//
//    send_packet_2_amak_parser(pRxPkt, (uint8_t*)p_ip_hdr, ip_total_length);
    //----------------------------------------------------------------------
    return false;
}

/* UDP packet handler Pointer
<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">

  Function:
    bool <FunctionName> (TCPIP_NET_HANDLE hNet, struct _tag_TCPIP_MAC_PACKET* rxPkt, const void* hParam);

  Summary:
    Pointer to a function(handler) that will get called to process an incoming UDP packet.

  Description:
    Pointer to a function that will be called by the UDP module
    when a RX packet is available.

  Precondition:
    None

  Parameters:
    hNet        - network handle on which the packet has arrived
    rxPkt       - pointer to incoming packet
    hParam      - user passed parameter when handler was registered

  Returns:
    true - if the packet is processed by the external handler.
           In this case the UDP module will no longer process the packet
    false - the packet needs to be processed internally by the UDP as usual           

  Remarks:
    The packet handler is called in the UDP context.
    The handler should be kept as short as possible as it affects the processing of all the other
    UDP RX traffic.

    Before calling the external packet handler 
    - the rxPkt->pktFlags has the bit 9 (value 0x0200) set for an IPv6 packet, cleared for IPv4
    - the rxPkt->pTransportLayer points to an UDP_HEADER data structure.
    - the rxPkt->pNetLayer points to an IPV4_HEADER/IPV6_HEADER data structure.
    - the rxPkt->pktIf points to the interface receiving the packet
    - the first data segment segLen is adjusted to store the size of the UDP data 

    Important!
    When the packet handler returns true, once it's done processing the packet,
    it needs to acknowledge it, i.e. return to the owner,
    which is the MAC driver serving the network interface!
    This means that the packet acknowledge function needs to be called,
    with a proper acknowledge parameter and the QUEUED flag needs to be cleared, if needed:
    if((*rxPkt->ackFunc)(rxPkt, rxPkt->ackParam))
    {
           rxPkt->pktFlags &= ~TCPIP_MAC_PKT_FLAG_QUEUED;
    }
    Failure to do that will result in memory leaks and starvation of the MAC driver.
    See the tcpip_mac.h for details.
    
 */
//</editor-fold>
static bool amak_shdsl_udp_packet_handler(TCPIP_NET_HANDLE hNet, TCPIP_MAC_PACKET* pRxPkt, const void* hParam)
{
//    #ifdef ENABLE_CONSOLE_MESSAGE
//        SYS_CONSOLE_PRINT("   UDP packet\r\n");
//    #endif

    IPV4_HEADER*    p_ip_hdr;
    UDP_HEADER*     p_udp_hdr;
    uint16_t        udp_tot_length;
    uint8_t         header_len;
    const IPV4_ADDR* pPktSrcAdd;
    const IPV4_ADDR* pPktDstAdd;
    
    p_ip_hdr = (IPV4_HEADER*)pRxPkt->pNetLayer;

    header_len = p_ip_hdr->IHL << 2;
    p_udp_hdr  = (UDP_HEADER*)(pRxPkt->pNetLayer + header_len);
    
    pRxPkt->pTransportLayer = (uint8_t*)p_udp_hdr;
    
//    udp_tot_length = TCPIP_Helper_ntohs( p_udp_hdr->Length );
//    pRxPkt->totTransportLen = udp_tot_length;                       //?!
    pRxPkt->totTransportLen = TCPIP_Helper_ntohs(p_ip_hdr->TotalLength) - header_len;   //?!

//    p_udp_hdr = (UDP_HEADER*)pRxPkt->pTransportLayer;

    if ( is_necessary_port(p_udp_hdr) )
    {
        #ifdef ENABLE_CONSOLE_MESSAGE
            SYS_CONSOLE_PRINT( "   (src port) = %d (dest port) = %d\r\n", TCPIP_Helper_ntohs(p_udp_hdr->SourcePort), TCPIP_Helper_ntohs(p_udp_hdr->DestinationPort) );
        #endif
        //----------------------------------------------------------------------
        // copy from TCPIP_UDP_Process() in "udp.c"
        //----------------------------------------------------------------------
        TCPIP_MAC_PKT_ACK_RES ackRes;
        
        ackRes = TCPIP_MAC_PKT_ACK_PROTO_DEST_ERR;
        if ( pRxPkt->totTransportLen < sizeof(UDP_HEADER) )
        {
            ackRes = TCPIP_MAC_PKT_ACK_STRUCT_ERR;
            _TCPIP_PKT_PacketAcknowledge(pRxPkt, ackRes, TCPIP_MODULE_UDP);
            return true;
        }
        //----------------------------------------------------------------------
        // cut from TCPIP_UDP_ProcessIPv4() in "udp.c"
        //----------------------------------------------------------------------
        udp_tot_length = TCPIP_Helper_ntohs( p_udp_hdr->Length );
        //----------------------------------------------------------------------
        #if (_TCPIP_IPV4_FRAGMENTATION == 0)
            if (udp_tot_length != pRxPkt->totTransportLen)
            {   // discard suspect packet
                ackRes = TCPIP_MAC_PKT_ACK_STRUCT_ERR;
                _TCPIP_PKT_PacketAcknowledge(pRxPkt, ackRes, TCPIP_MODULE_UDP);
                return true;
            }
        #endif  // (_TCPIP_IPV4_FRAGMENTATION != 0)
        //----------------------------------------------------------------------
        pPktSrcAdd = TCPIP_IPV4_PacketGetSourceAddress(pRxPkt);
        pPktDstAdd = TCPIP_IPV4_PacketGetDestAddress(pRxPkt);
        //----------------------------------------------------------------------
        // See if we need to validate the checksum field (0x0000 is disabled)
        #ifdef TCPIP_UDP_USE_RX_CHECKSUM
            if (0 != p_udp_hdr->Checksum)
            {
                IPV4_PSEUDO_HEADER  pseudoHdr;
                uint16_t            calcChkSum;
                
                // Calculate IP pseudoheader checksum.
                pseudoHdr.Zero	   = 0;
                pseudoHdr.Length   = p_udp_hdr->Length;
                pseudoHdr.Protocol = IP_PROT_UDP;
                pseudoHdr.DestAddress.Val   = pPktDstAdd->Val;
                pseudoHdr.SourceAddress.Val = pPktSrcAdd->Val;
                //--------------------------------------------------------------
                calcChkSum = ~TCPIP_Helper_CalcIPChecksum((uint8_t*)&pseudoHdr, sizeof(pseudoHdr), 0);
                //--------------------------------------------------------------
                
            #if (0 != _TCPIP_IPV4_FRAGMENTATION)
                TCPIP_MAC_PACKET* pFragPkt;
                uint16_t totCalcUdpLen = 0;
                //--------------------------------------------------------------
                for(pFragPkt = pRxPkt; pFragPkt != 0; pFragPkt = pFragPkt->pkt_next)
                {
                    calcChkSum = ~TCPIP_Helper_PacketChecksum(pFragPkt, pFragPkt->pTransportLayer, pFragPkt->totTransportLen, calcChkSum);
                    totCalcUdpLen += pFragPkt->totTransportLen;
                }
                //--------------------------------------------------------------
                calcChkSum = ~calcChkSum;
                //--------------------------------------------------------------
                if (udp_tot_length != totCalcUdpLen)
                {   // discard suspect packet
                    ackRes = TCPIP_MAC_PKT_ACK_STRUCT_ERR;
                    _TCPIP_PKT_PacketAcknowledge(pRxPkt, ackRes, TCPIP_MODULE_UDP);
                    return true;
                }
                //--------------------------------------------------------------
               
            #else
                if ( 0 != (pRxPkt->pktFlags & TCPIP_MAC_PKT_FLAG_SPLIT) )
                {
                    calcChkSum = TCPIP_Helper_PacketChecksum(pRxPkt, (uint8_t*)p_udp_hdr, udp_tot_length, calcChkSum);
                }
                else
                {
                    calcChkSum = TCPIP_Helper_CalcIPChecksum((uint8_t*)p_udp_hdr, udp_tot_length, calcChkSum);
                }
            #endif  // (_TCPIP_IPV4_FRAGMENTATION != 0)
                //--------------------------------------------------------------
                if (0 != calcChkSum)
                {   // discard packet
                    ackRes = TCPIP_MAC_PKT_ACK_CHKSUM_ERR;
                    _TCPIP_PKT_PacketAcknowledge(pRxPkt, ackRes, TCPIP_MODULE_UDP);
                    return true;
                }
                //--------------------------------------------------------------
            }
        #endif // TCPIP_UDP_USE_RX_CHECKSUM
        //----------------------------------------------------------------------
//        pUDPHdr      = (UDP_HEADER*)pRxPkt->pTransportLayer;
//        udpTotLength = TCPIP_Helper_ntohs(pUDPHdr->Length);

        pRxPkt->pktFlags &= ~TCPIP_MAC_PKT_FLAG_TYPE_MASK;
        pRxPkt->pktFlags |= TCPIP_MAC_PKT_FLAG_IPV4;
        pRxPkt->pktFlags |= TCPIP_MAC_PKT_FLAG_UDP;
        
        uint16_t mac_pkt_len = TCPIP_Helper_ntohs(p_ip_hdr->TotalLength) + sizeof(TCPIP_MAC_ETHERNET_HEADER);
        uint8_t* p_mac_pkt = pRxPkt->pMacLayer;

        send_packet_2_amak_parser(pRxPkt, p_mac_pkt, mac_pkt_len);
        
            
//        send_packet_2_amak_parser(pRxPkt, (uint8_t*)p_udp_hdr, udp_tot_length);
        
//        uint16_t ipv4_tot_length = TCPIP_Helper_ntohs(p_ip_hdr->TotalLength);
//        send_packet_2_amak_parser(pRxPkt, (uint8_t*)p_ip_hdr, ipv4_tot_length);
        //----------------------------------------------------------------------
        return true;
        //----------------------------------------------------------------------
    }
    
    return false;
}
//------------------------------------------------------------------------------
__inline__ bool __attribute__((always_inline)) is_ipv4_hdr_len_in_packet_limits(TCPIP_MAC_PACKET* pRxPkt, uint8_t header_len)
{
    return  (  (           header_len < sizeof(IPV4_HEADER)   ) 
            || ( (uint16_t)header_len > pRxPkt->pDSeg->segLen ) 
            );
}
//------------------------------------------------------------------------------
__inline__ bool __attribute__((always_inline)) is_fragment_ipv4(IPV4_HEADER* pCHeader)
{
    return  (  ( 0 != pCHeader->FragmentInfo.MF         ) 
            || ( 0 != pCHeader->FragmentInfo.fragOffset )
            );
}
//------------------------------------------------------------------------------
__inline__ bool __attribute__((always_inline)) is_necessary_port(const UDP_HEADER* const pUDPHdr)
{
    return ( (app_udp_taskData.necessary_src_port  == pUDPHdr->SourcePort) 
          && (app_udp_taskData.necessary_dest_port == pUDPHdr->DestinationPort) 
           );
}
//------------------------------------------------------------------------------
__inline__ void __attribute__((always_inline)) send_packet_2_amak_parser(TCPIP_MAC_PACKET* pRxPkt, const uint8_t* packet, const uint16_t len)
{
    //----------------------------------------------------------------------
    app_udp_taskData.event_info.data_len = len;
    //----------------------------------------------------------------------
    app_udp_taskData.event_info.pData = malloc(len);
    configASSERT(app_udp_taskData.event_info.pData);
    memcpy((uint8_t*)app_udp_taskData.event_info.pData, (uint8_t*)packet, len);
    //----------------------------------------------------------------------
    app_udp_taskData.event_info.event_id = (ENUM_EVENT_TYPE)EVENT_TYPE_AMAK_UDP_POCKET;
    //----------------------------------------------------------------------
    xQueueSend( eventQueue_app_amak_parser_task, (void*)&( app_udp_taskData.event_info ), 0 );//portMAX_DELAY); 
    //----------------------------------------------------------------------
    if( (*pRxPkt->ackFunc)(pRxPkt, pRxPkt->ackParam) )
    {
           pRxPkt->pktFlags &= ~TCPIP_MAC_PKT_FLAG_QUEUED;
    }
    //----------------------------------------------------------------------
}

//------------------------------------------------------------------------------
// End of File
//------------------------------------------------------------------------------
