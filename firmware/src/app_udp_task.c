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
#include "config/default/library/tcpip/src/tcpip_manager_control.h"
#include "config/default/library/tcpip/src/ipv4_manager.h"
#include "config/default/library/tcpip/tcpip_manager.h"
#include "config/default/library/tcpip/src/udp_manager.h"
#include "config/default/library/tcpip/icmp.h"
#include "config/default/library/tcpip/tcpip_mac.h"
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

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="Description function: wait_console_buffer_free()">
  Function:
    void wait_console_buffer_free ( void )

  Summary:
    

  Description:
    This function wait until Sys Console write buffer become empty.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    wait_console_buffer_free();
    </code>

  Remarks:
    None.
*/
// </editor-fold>
void wait_console_buffer_free(void);

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
            while ( ( nFreeSpace < (SYS_CONSOLE_PRINT_BUFFER_SIZE - 1) ) && ( nFreeSpace != -1 ) ) 
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
    if(tcpipStat < 0)
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
APP_UDP_TASK_STATES app_udp_task_register_amak_shdsl_udp_handler(void)
{
    //------------------------------------------------------------------
    // <editor-fold defaultstate="collapsed" desc="Wait until Console Buffer become free">
    wait_console_buffer_free();
    // </editor-fold>
    //------------------------------------------------------------------
    amak_shdsl_pktHandle = TCPIP_UDP_PacketHandlerRegister( amak_shdsl_udp_packet_handler, NULL );
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
    UDP_HEADER*		 pUDPHdr;
//    UDP_SOCKET_DCPT* pSkt;
    uint16_t         udpTotLength;
    const IPV4_ADDR* pPktSrcAdd;
    const IPV4_ADDR* pPktDstAdd;
//    TCPIP_UDP_SIGNAL_FUNCTION sigHandler;
//    const void*      sigParam;
//    TCPIP_MAC_PKT_ACK_RES ackRes;
    
    pUDPHdr = (UDP_HEADER*)pRxPkt->pTransportLayer;
    udpTotLength = TCPIP_Helper_ntohs(pUDPHdr->Length);

#if (_TCPIP_IPV4_FRAGMENTATION == 0)
    if(udpTotLength != pRxPkt->totTransportLen)
    {   // discard suspect packet
        return false; //TCPIP_MAC_PKT_ACK_STRUCT_ERR;
    }
#endif  // (_TCPIP_IPV4_FRAGMENTATION != 0)

    pPktSrcAdd = TCPIP_IPV4_PacketGetSourceAddress(pRxPkt);
    pPktDstAdd = TCPIP_IPV4_PacketGetDestAddress(pRxPkt);
	// See if we need to validate the checksum field (0x0000 is disabled)
#ifdef TCPIP_UDP_USE_RX_CHECKSUM
	if((pUDPHdr->Checksum != 0))
	{
        IPV4_PSEUDO_HEADER  pseudoHdr;
        uint16_t            calcChkSum;
	    // Calculate IP pseudoheader checksum.
	    pseudoHdr.SourceAddress.Val = pPktSrcAdd->Val;
	    pseudoHdr.DestAddress.Val = pPktDstAdd->Val;
	    pseudoHdr.Zero	= 0;
	    pseudoHdr.Protocol = IP_PROT_UDP;
	    pseudoHdr.Length = pUDPHdr->Length;

	    calcChkSum = ~TCPIP_Helper_CalcIPChecksum((uint8_t*)&pseudoHdr, sizeof(pseudoHdr), 0);
#if (_TCPIP_IPV4_FRAGMENTATION != 0)
        TCPIP_MAC_PACKET* pFragPkt;
        uint16_t totCalcUdpLen = 0;
        for(pFragPkt = pRxPkt; pFragPkt != 0; pFragPkt = pFragPkt->pkt_next)
        {
            calcChkSum = ~TCPIP_Helper_PacketChecksum(pFragPkt, pFragPkt->pTransportLayer, pFragPkt->totTransportLen, calcChkSum);
            totCalcUdpLen += pFragPkt->totTransportLen;
        }
        calcChkSum = ~calcChkSum;

        if(udpTotLength != totCalcUdpLen)
        {   // discard suspect packet
            return false; //TCPIP_MAC_PKT_ACK_STRUCT_ERR;
        }
#else
        if((pRxPkt->pktFlags & TCPIP_MAC_PKT_FLAG_SPLIT) != 0)
        {
            calcChkSum = TCPIP_Helper_PacketChecksum(pRxPkt, (uint8_t*)pUDPHdr, udpTotLength, calcChkSum);
        }
        else
        {
            calcChkSum = TCPIP_Helper_CalcIPChecksum((uint8_t*)pUDPHdr, udpTotLength, calcChkSum);
        }
#endif  // (_TCPIP_IPV4_FRAGMENTATION != 0)

        if(calcChkSum != 0)
        {   // discard packet
            return false; //TCPIP_MAC_PKT_ACK_CHKSUM_ERR;
        }
	}
#endif // TCPIP_UDP_USE_RX_CHECKSUM

    pUDPHdr->SourcePort = TCPIP_Helper_ntohs(pUDPHdr->SourcePort);
    pUDPHdr->DestinationPort = TCPIP_Helper_ntohs(pUDPHdr->DestinationPort);
    
    if ( (1500 == pUDPHdr->SourcePort) && (1500 == pUDPHdr->DestinationPort) )
    {
        app_udp_taskData.event_info.data_len = udpTotLength;
        app_udp_taskData.event_info.pData = (uint8_t*)pRxPkt->pTransportLayer;
        app_udp_taskData.event_info.event_id = (ENUM_EVENT_TYPE)EVENT_TYPE_AMAK_UDP_POCKET;
        
        xQueueSend( eventQueue_app_amak_parser_task, (void*)&( app_udp_taskData.event_info ), 0 );//portMAX_DELAY); 
        
        return true;
    }
    
    return false;
/*
    pUDPHdr->Length = udpTotLength - sizeof(UDP_HEADER);    

    while(true)
    {
        pSkt = _UDPFindMatchingSocket(pRxPkt, pUDPHdr, IP_ADDRESS_TYPE_IPV4);
        if(pSkt == 0)
        {
            // If there is no matching socket, There is no one to handle
            // this data.  Discard it.
            ackRes = TCPIP_MAC_PKT_ACK_PROTO_DEST_ERR;
            break;
        }

#if defined(TCPIP_STACK_USE_IGMP)    
        if(pSkt->extFlags.mcastSkipCheck == 0)
        {   // don't skip check multicast traffic
            if(TCPIP_Helper_IsMcastAddress(pPktDstAdd))
            {   // need to check
                if(!TCPIP_IGMP_IsMcastEnabled(pSkt->sktIx, pRxPkt->pktIf, *pPktDstAdd, *pPktSrcAdd))
                {   // don't let it through
                    ackRes = TCPIP_MAC_PKT_ACK_PROTO_DEST_ERR;
                    break;
                }
            }
        }
#endif  // defined(TCPIP_STACK_USE_IGMP)    
    
        if(pSkt->extFlags.mcastOnly != 0)
        {   // let through multicast traffic only
            if(!TCPIP_Helper_IsMcastAddress(pPktDstAdd))
            {   // don't let it through
                ackRes = TCPIP_MAC_PKT_ACK_PROTO_DEST_ERR;
                break;
            }
        }

        // insert valid packet in the RX queue
        sigHandler = _RxSktQueueAddLocked(pSkt, pRxPkt, &sigParam);
        if(sigHandler)
        {   // notify socket user
            (*sigHandler)(pSkt->sktIx, pRxPkt->pktIf, TCPIP_UDP_SIGNAL_RX_DATA, sigParam);
        }

        // everything OK, pass to user
        ackRes = TCPIP_MAC_PKT_ACK_NONE;
        break;
    }


    // log 
#if (TCPIP_PACKET_LOG_ENABLE)
    uint32_t logPort = (pSkt != 0) ? ((uint32_t)pSkt->localPort << 16) | pSkt->remotePort : ((uint32_t)pUDPHdr->DestinationPort << 16) | pUDPHdr->SourcePort;
    TCPIP_PKT_FlightLogRxSkt(pRxPkt, TCPIP_MODULE_LAYER3, logPort, pSkt != 0 ? pSkt->sktIx: 0xffff);
#endif  // (TCPIP_PACKET_LOG_ENABLE)

    return ackRes;
*/
}

//------------------------------------------------------------------------------
// End of File
//------------------------------------------------------------------------------
