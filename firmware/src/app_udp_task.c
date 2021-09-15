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

/*------------------------------------------------------------------------------
  UDP Task Application Data

  Summary:
    Holds UDP task application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_UDP_TASK_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/
//------------------------------------------------------------------------------
APP_UDP_TASK_DATA app_udp_taskData;

/*------------------------------------------------------------------------------
  Event Queue for UDP Task Application

  Summary:
    This variable holds event queue for UDP task application

  Description:
    
  Remarks:
    
*/
//------------------------------------------------------------------------------
QueueHandle_t eventQueue_app_udp_task = NULL;

//------------------------------------------------------------------------------
// Section: Application Callback Functions
//------------------------------------------------------------------------------

/* TODO:  Add any necessary callback functions.
*/

//------------------------------------------------------------------------------
// Section: Application Local Functions
//------------------------------------------------------------------------------

/* TODO:  Add any necessary callback functions.
*/

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
            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: Handle error condition. Not sufficient memory to create Queue\r\n");
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
    static IPV4_ADDR    dwLastIP = {-1};
    IPV4_ADDR           ipAddr;

    /* Check the application's current state. */
    switch ( app_udp_taskData.state )
    {
        /* Application's initial state. */
        case APP_UDP_TASK_STATE_INIT:
        {
            LED3_Toggle();
            bool appUdpTaskInitialized = false;
            
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

                app_udp_taskData.state = APP_UDP_TASK_STATE_Start;
                app_udp_taskData.error = APP_UDP_TASK_ERROR_NO;
                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: Initialization complite\r\n");
                SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> APP_UDP_TASK_STATE_Start \r\n");
                #endif
            }
//            taskYIELD();
            break;
        }
        /* TODO: implement your application state machine.*/

        case APP_UDP_TASK_STATE_Start:
        {
            SYS_STATUS tcpipStat = TCPIP_STACK_Status(sysObj.tcpip);
            if(tcpipStat < 0)
            {   // some error occurred
                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: TCP/IP stack initialization failed!\r\n");
                    SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> UDP_STATE_Error\r\n");
                #endif
                app_udp_taskData.state = APP_UDP_TASK_STATE_Error;
                app_udp_taskData.error = APP_UDP_TASK_ERROR_STATE_Start;
            }
            else if(SYS_STATUS_READY == tcpipStat)
            {
                const char          *netName;
                const char          *netBiosName;
                
                app_udp_taskData.netH = TCPIP_STACK_IndexToNet(0);
                netName = TCPIP_STACK_NetNameGet(app_udp_taskData.netH);
                netBiosName = TCPIP_STACK_NetBIOSName(app_udp_taskData.netH);
                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_PRINT("    Interface %s on host %s \r\n", netName, netBiosName);
                #endif
                
                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> UDP_STATE_WAIT_FOR_IP\r\n");
                #endif
                app_udp_taskData.state = APP_UDP_TASK_STATE_WAIT_FOR_IP;
                app_udp_taskData.error = APP_UDP_TASK_ERROR_NO;
            }
//            taskYIELD();
            break;
        }
        case APP_UDP_TASK_STATE_WAIT_FOR_IP:
        {
            if( false == TCPIP_STACK_NetIsReady(app_udp_taskData.netH) )
            {
//                    vTaskDelay( 10 / portTICK_PERIOD_MS ); 
//                taskYIELD();
                return; // interface not ready yet!
            }
            ipAddr.Val = TCPIP_STACK_NetAddress(app_udp_taskData.netH);
            if(dwLastIP.Val != ipAddr.Val)
            {
                dwLastIP.Val = ipAddr.Val;
                const TCPIP_MAC_ADDR* pMACAddr = (const TCPIP_MAC_ADDR*)TCPIP_STACK_NetAddressMac(app_udp_taskData.netH);
                
                char str_ip_adr[20];
                char str_mac_adr[25];
                
                TCPIP_Helper_IPAddressToString( &ipAddr, str_ip_adr, sizeof(str_ip_adr) );
                TCPIP_Helper_MACAddressToString( pMACAddr, str_mac_adr, sizeof(str_mac_adr) );

                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_PRINT   ("----------== Net %d ==------------\r\n", 0);
                    SYS_CONSOLE_PRINT   ("    NetName:     %s \r\n", 
                                            TCPIP_STACK_NetNameGet(app_udp_taskData.netH) );
                    SYS_CONSOLE_PRINT   ("    IP  address: %d.%d.%d.%d \r\n", 
                                            ipAddr.v[0], ipAddr.v[1], ipAddr.v[2], ipAddr.v[3]);
                    SYS_CONSOLE_PRINT   ("    IP  address: %s \r\n", str_ip_adr);

                    SYS_CONSOLE_PRINT   ("    MAC address: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X \r\n", 
                                            pMACAddr->v[0], pMACAddr->v[1], pMACAddr->v[2], pMACAddr->v[3], pMACAddr->v[4], pMACAddr->v[5]);
                    SYS_CONSOLE_PRINT   ("    MAC address: %s \r\n", str_mac_adr);
                    
                    SYS_CONSOLE_MESSAGE ("---------------------------------\r\n");
                #endif
            }
            TCPIP_STACK_HEAP_TYPE heapType = TCPIP_STACK_HEAP_TYPE_INTERNAL_HEAP;
            TCPIP_STACK_HEAP_HANDLE heapH = TCPIP_STACK_HeapHandleGet( heapType, 0 );
            size_t heap_watermark = TCPIP_STACK_HEAP_HighWatermark( heapH );
            #ifdef ENABLE_CONSOLE_MESSAGE
                SYS_CONSOLE_PRINT   (" APP_UDP_TASK: heap high watermark: %d \r\n", heap_watermark);
            #endif
            
            
            TCPIP_ARP_RESULT arp_res;
            arp_res = TCPIP_ARP_Resolve(app_udp_taskData.netH, &(app_udp_taskData.dest_adr.v4Add) );
            #ifdef ENABLE_CONSOLE_MESSAGE
                SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: ARP resolve: ");
                switch (arp_res)
                {
                    case ARP_RES_OK:
                        SYS_CONSOLE_MESSAGE("operation succeeded \r\n");
                        break;
                    case ARP_RES_ENTRY_NEW:
                        SYS_CONSOLE_MESSAGE("operation succeeded and a new entry was added \r\n");
                        break;
                    case ARP_RES_ENTRY_SOLVED:
                        SYS_CONSOLE_MESSAGE("the required entry is already solved \r\n");
                        break;
                    case ARP_RES_ENTRY_QUEUED:
                        SYS_CONSOLE_MESSAGE("the required entry was already queued \r\n");
                        break;
                    case ARP_RES_ENTRY_EXIST:
                        SYS_CONSOLE_MESSAGE("the required entry was already cached \r\n");
                        break;
                    case ARP_RES_PERM_QUOTA_EXCEED:
                        SYS_CONSOLE_MESSAGE("info: the quota of permanent entries was exceeded \r\n");
                        break;
                    case ARP_RES_PROBE_OK:
                        SYS_CONSOLE_MESSAGE("requested probe sent \r\n");
                        break;
                    case ARP_RES_NO_ENTRY:
                        SYS_CONSOLE_MESSAGE("no such entry exists \r\n");
                        break;
                    case ARP_RES_CACHE_FULL:
                        SYS_CONSOLE_MESSAGE("the cache is full and no entry could be removed to make room \r\n");
                        break;
                    case ARP_RES_TX_FAILED:
                        SYS_CONSOLE_MESSAGE("failed to transmit an ARP message \r\n");
                        break;
                    case ARP_RES_BAD_INDEX:
                        SYS_CONSOLE_MESSAGE("bad query index \r\n");
                        break;
                    case ARP_RES_BAD_ADDRESS:
                        SYS_CONSOLE_MESSAGE("bad IP address specified \r\n");
                        break;
                    case ARP_RES_NO_INTERFACE:
                        SYS_CONSOLE_MESSAGE("no such interface exists \r\n");
                        break;
                    case ARP_RES_BAD_TYPE:
                        SYS_CONSOLE_MESSAGE("no such type is valid/exists \r\n");
                        break;
                    case ARP_RES_CONFIGURE_ERR:
                        SYS_CONSOLE_MESSAGE("interface is configuring now, no ARP probes \r\n");
                        break;
                    case ARP_RES_PROBE_FAILED:
                        SYS_CONSOLE_MESSAGE("requested probe failed \r\n");
                        break;
                    default:
                        SYS_CONSOLE_MESSAGE("unknown error!!! \r\n");
                        break;
                }
                #endif

                TCPIP_MAC_ADDR MACAddr;
                if ( true == TCPIP_ARP_IsResolved(app_udp_taskData.netH, &(app_udp_taskData.dest_adr.v4Add), &MACAddr) )
                {
                    #ifdef ENABLE_CONSOLE_MESSAGE
                        SYS_CONSOLE_MESSAGE (   " APP_UDP_TASK: ARP resolve MAC address: Ok  \r\n" );
                        SYS_CONSOLE_PRINT   (   "               MAC address = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X \r\n", 
                                MACAddr.v[0], MACAddr.v[1], MACAddr.v[2], MACAddr.v[3], MACAddr.v[4], MACAddr.v[5]);
                    #endif
                }
                else
                {
                    #ifdef ENABLE_CONSOLE_MESSAGE
                        SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: ARP resolve MAC address: NOT success! \r\n");
                    #endif
                }
                
                IPV4_ADDR remoteAddress = { .v = {10, 2, 22, 220} };
                uint16_t mySequenceNumber = 1;
                uint16_t myId = 0x1234;

              if ( TCPIP_ICMP_EchoRequestSend(0, &remoteAddress, mySequenceNumber, myId) == ICMP_ECHO_OK )
              {
                    #ifdef ENABLE_CONSOLE_MESSAGE
                        SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: ICMP echo request send: Ok \r\n");
                    #endif
              }
              else
              {
                    #ifdef ENABLE_CONSOLE_MESSAGE
                        SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: ICMP echo request send: Error! \r\n");
                    #endif
              }
            //----------------------------------------------------------------
//            TCPIP_NET_HANDLE netH = TCPIP_STACK_NetHandleGet("eth0");
                
            TCPIP_MAC_RX_STATISTICS rxStatistics;
            TCPIP_MAC_TX_STATISTICS txStatistics;
//            TCPIP_NET_HANDLE netH = TCPIP_STACK_NetHandleGet("PIC32INT");
            if ( TCPIP_STACK_NetMACStatisticsGet(app_udp_taskData.netH, &rxStatistics, &txStatistics) )
            {
                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_MESSAGE("---------------------------------------\r\n");
                    SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: Net MAC statistics get: \r\n");
                    SYS_CONSOLE_PRINT  ("               Rx good packets = %d \r\n", rxStatistics.nRxOkPackets);
                    SYS_CONSOLE_PRINT  ("               Rx error packets = %d \r\n", rxStatistics.nRxErrorPackets);
                    SYS_CONSOLE_PRINT  ("               Tx good packets = %d \r\n", txStatistics.nTxOkPackets);
                    SYS_CONSOLE_PRINT  ("               Tx error packets = %d \r\n", txStatistics.nTxErrorPackets);
                    SYS_CONSOLE_MESSAGE("---------------------------------------\r\n");
                #endif
            }
                
            const TCPIP_MAC_OBJECT* pEthMacObject = TCPIP_STACK_MACObjectGet(app_udp_taskData.netH);
//            pEthMacObject->TCPIP_MAC_Tasks
//                    pEthMacObject
            if(pEthMacObject != 0)
            {    // valid MAC object pointer
                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: Get MAC object: Ok \r\n");
                    SYS_CONSOLE_PRINT  (" APP_UDP_TASK: MAC name: %s \r\n", pEthMacObject->macName);
                #endif
                DRV_HANDLE hMac = (*pEthMacObject->TCPIP_MAC_Open)(pEthMacObject->macId, DRV_IO_INTENT_READWRITE);
                if(DRV_HANDLE_INVALID != hMac)
                {   
                    #ifdef ENABLE_CONSOLE_MESSAGE
                        SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: Open MAC object: Ok \r\n");
                        SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: MAC link check: ");
                        if ( true == (*pEthMacObject->TCPIP_MAC_LinkCheck)(hMac) )
                        {
                            SYS_CONSOLE_MESSAGE("up \r\n");
                        }
                        else
                        {
                            SYS_CONSOLE_MESSAGE("down \r\n");
                        }
                    #endif

                    switch ( (*pEthMacObject->TCPIP_MAC_Status)(sysObj.tcpip) )
                    {
                        case SYS_STATUS_READY:
                            #ifdef ENABLE_CONSOLE_MESSAGE
                                SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: MAC status: Ready \r\n");
                            #endif
                            break;
                        case SYS_STATUS_BUSY:
                            #ifdef ENABLE_CONSOLE_MESSAGE
                                SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: MAC status: Busy \r\n");
                            #endif
                            break;
                        case SYS_STATUS_ERROR:
                            #ifdef ENABLE_CONSOLE_MESSAGE
                                SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: MAC status: Error! \r\n");
                            #endif
                            break;
                        default:
                            #ifdef ENABLE_CONSOLE_MESSAGE
                                SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: MAC status: unknown status \r\n");
                            #endif
                            break;
                    }

                    // can use the MAC handle to access a MAC function
//                    TCPIP_MAC_RX_STATISTICS rxStatistics;
//                    TCPIP_MAC_TX_STATISTICS txStatistics;
//                    TCPIP_MAC_RES macRes = (*pEthMacObject->TCPIP_MAC_StatisticsGet)(hMac, &rxStatistics, &txStatistics);
        
                    TCPIP_MAC_PARAMETERS MacParams;
                    (*pEthMacObject->TCPIP_MAC_ParametersGet)(hMac, &MacParams);
                    #ifdef ENABLE_CONSOLE_MESSAGE
                        SYS_CONSOLE_MESSAGE (   " APP_UDP_TASK: MAC parameters get: Ok \r\n");
                        SYS_CONSOLE_PRINT   (   "               MAC address = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X \r\n", 
                                                MacParams.ifPhyAddress.v[0], 
                                                MacParams.ifPhyAddress.v[1], 
                                                MacParams.ifPhyAddress.v[2],
                                                MacParams.ifPhyAddress.v[3], 
                                                MacParams.ifPhyAddress.v[4], 
                                                MacParams.ifPhyAddress.v[5]
                                            );
                    #endif
                }
                else
                {
                    #ifdef ENABLE_CONSOLE_MESSAGE
                        SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: Open MAC object: Invalid handle. Error! \r\n");
                    #endif
                }
            }
            else
            {
                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: Get MAC object: Error! \r\n");
                #endif
            }
            //----------------------------------------------------------------
//            TCPIP_MAC_Open(const SYS_MODULE_INDEX drvIndex,  const DRV_IO_INTENT intent);
//            
            
            #ifdef ENABLE_CONSOLE_MESSAGE
                SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP_STATE_WAIT_SERVER_OPEN\r\n");
            #endif
            app_udp_taskData.state = APP_UDP_TASK_STATE_WAIT_SERVER_OPEN;
//            #ifdef ENABLE_CONSOLE_MESSAGE
//                SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> UDP_STATE_Error (for wait state)\r\n");
//            #endif
//            app_udp_taskData.state = APP_UDP_TASK_STATE_Error;
            app_udp_taskData.error = APP_UDP_TASK_ERROR_NO;
            
//            taskYIELD();
            break;
        }
        case APP_UDP_TASK_STATE_WAIT_SERVER_OPEN:
        {

            app_udp_taskData.udp_rx_socket = TCPIP_UDP_ServerOpen( IP_ADDRESS_TYPE_IPV4, app_udp_taskData.local_port, &(app_udp_taskData.local_adr) );
            if (INVALID_SOCKET != app_udp_taskData.udp_rx_socket)
            {
                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP server open: Ok \r\n");
                #endif
//                TCPIP_UDP_SourceIPAddressSet(app_udp_taskData.udp_rx_socket, IP_ADDRESS_TYPE_IPV4, &local_adr);
//                TCPIP_UDP_DestinationIPAddressSet(app_udp_taskData.udp_rx_socket, IP_ADDRESS_TYPE_IPV4, &dest_adr);
                if ( true == TCPIP_UDP_RemoteBind( app_udp_taskData.udp_rx_socket, IP_ADDRESS_TYPE_IPV4, app_udp_taskData.dest_port, &(app_udp_taskData.dest_adr) ) )
                {
                    #ifdef ENABLE_CONSOLE_MESSAGE
                        SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP remote bind: Ok \r\n");
                    #endif
                }
//                if ( true == TCPIP_UDP_Bind( app_udp_taskData.udp_rx_socket, IP_ADDRESS_TYPE_IPV4, app_udp_taskData.local_port, &(app_udp_taskData.local_adr) ) )
//                {
//                    #ifdef ENABLE_CONSOLE_MESSAGE
//                        SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP bind: Ok \r\n");
//                    #endif
//                }
                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> UDP_STATE_WAIT_CLIENT_OPEN\r\n");
                #endif
                app_udp_taskData.state = APP_UDP_TASK_STATE_WAIT_CLIENT_OPEN;
//                app_udp_taskData.state = APP_UDP_TASK_STATE_Rx;
                app_udp_taskData.error = APP_UDP_TASK_ERROR_NO;

            }
            //-----------------------------------------------------------------
            UDP_SOCKET_INFO socket_info;
            TCPIP_UDP_SocketInfoGet(app_udp_taskData.udp_rx_socket, &socket_info);
            #ifdef ENABLE_CONSOLE_MESSAGE
                const char          *netName;
                const char          *netBiosName;
                TCPIP_NET_HANDLE    netH;

                netH = socket_info.hNet;
                netName = TCPIP_STACK_NetNameGet(netH);
                netBiosName = TCPIP_STACK_NetBIOSName(netH);
                ipAddr.Val = TCPIP_STACK_NetAddress(netH);

                SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: ----------== Rx socket ==----------\r\n");

                SYS_CONSOLE_PRINT   ("      Interface %s on host %s \r\n", netName, netBiosName);
                SYS_CONSOLE_MESSAGE ("      IP Address: ");
                SYS_CONSOLE_PRINT   ("%d.%d.%d.%d \r\n", ipAddr.v[0], ipAddr.v[1], ipAddr.v[2], ipAddr.v[3]);
                        
                SYS_CONSOLE_PRINT("APP_UDP_TASK: Dest   IP %d.%d.%d.%d \r\n", socket_info.destIPaddress.v4Add.v[0], socket_info.destIPaddress.v4Add.v[1], socket_info.destIPaddress.v4Add.v[2], socket_info.destIPaddress.v4Add.v[3]);
                SYS_CONSOLE_PRINT("APP_UDP_TASK: Source IP %d.%d.%d.%d \r\n", socket_info.sourceIPaddress.v4Add.v[0], socket_info.sourceIPaddress.v4Add.v[1], socket_info.sourceIPaddress.v4Add.v[2], socket_info.sourceIPaddress.v4Add.v[3]);
                SYS_CONSOLE_PRINT("APP_UDP_TASK: Local  IP %d.%d.%d.%d \r\n", socket_info.localIPaddress.v4Add.v[0], socket_info.localIPaddress.v4Add.v[1], socket_info.localIPaddress.v4Add.v[2], socket_info.localIPaddress.v4Add.v[3]);
                SYS_CONSOLE_PRINT("APP_UDP_TASK: Remote IP %d.%d.%d.%d \r\n", socket_info.remoteIPaddress.v4Add.v[0], socket_info.remoteIPaddress.v4Add.v[1], socket_info.remoteIPaddress.v4Add.v[2], socket_info.remoteIPaddress.v4Add.v[3]);
                SYS_CONSOLE_PRINT("APP_UDP_TASK: Local  port %d \r\n", socket_info.localPort);
                SYS_CONSOLE_PRINT("APP_UDP_TASK: Remote port %d \r\n", socket_info.remotePort);
                SYS_CONSOLE_PRINT("APP_UDP_TASK: rxQueue size %d \r\n", socket_info.rxQueueSize);
                SYS_CONSOLE_PRINT("APP_UDP_TASK: tx size %d \r\n", socket_info.txSize);
            #endif
            //-----------------------------------------------------------------
//            taskYIELD();
            break;
        }
        case APP_UDP_TASK_STATE_WAIT_CLIENT_OPEN:
        {
            app_udp_taskData.udp_tx_socket = TCPIP_UDP_ClientOpen( IP_ADDRESS_TYPE_IPV4, app_udp_taskData.dest_port, &(app_udp_taskData.dest_adr) );
            if (INVALID_SOCKET != app_udp_taskData.udp_tx_socket)
            {
                if ( true == TCPIP_UDP_Bind( app_udp_taskData.udp_tx_socket, IP_ADDRESS_TYPE_IPV4, app_udp_taskData.local_port, &(app_udp_taskData.local_adr) ) )
//                if ( true == TCPIP_UDP_RemoteBind( app_udp_taskData.udp_tx_socket, IP_ADDRESS_TYPE_IPV4, app_udp_taskData.dest_port, &(app_udp_taskData.dest_adr) ) )
                {
                    #ifdef ENABLE_CONSOLE_MESSAGE
                        SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP client open\r\n");
                    #endif
                }
                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: State machine -> UDP_STATE_WAIT_FOR_CONNECTION\r\n");
                #endif

                app_udp_taskData.state = APP_UDP_TASK_STATE_WAIT_FOR_CONNECTION;
            }
//            taskYIELD();
            break;
        }
        case APP_UDP_TASK_STATE_WAIT_FOR_CONNECTION:
        {
            if ( true == TCPIP_UDP_IsConnected(app_udp_taskData.udp_tx_socket) )
//            if (TCPIP_UDP_IsConnected(app_udp_taskData.udp_rx_socket))
            {
                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: TCPIP_UDP_IsConnected\r\n");
                #endif
                app_udp_taskData.state = APP_UDP_TASK_STATE_Rx;
                //-----------------------------------------------------------------
                UDP_SOCKET_INFO socket_info;
                TCPIP_UDP_SocketInfoGet(app_udp_taskData.udp_tx_socket, &socket_info);
                #ifdef ENABLE_CONSOLE_MESSAGE
                    const char          *netName;
                    const char          *netBiosName;
                    TCPIP_NET_HANDLE    netH;

                    netH = socket_info.hNet;
                    netName = TCPIP_STACK_NetNameGet(netH);
                    netBiosName = TCPIP_STACK_NetBIOSName(netH);
                    ipAddr.Val = TCPIP_STACK_NetAddress(netH);

                    SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: --== Tx socket ==--\r\n");

                    SYS_CONSOLE_PRINT("    Interface %s on host %s \r\n", netName, netBiosName);
                    SYS_CONSOLE_MESSAGE(" IP Address: ");
                    SYS_CONSOLE_PRINT("%d.%d.%d.%d \r\n", ipAddr.v[0], ipAddr.v[1], ipAddr.v[2], ipAddr.v[3]);

                    SYS_CONSOLE_PRINT("APP_UDP_TASK: Dest   IP %d.%d.%d.%d \r\n", socket_info.destIPaddress.v4Add.v[0], socket_info.destIPaddress.v4Add.v[1], socket_info.destIPaddress.v4Add.v[2], socket_info.destIPaddress.v4Add.v[3]);
                    SYS_CONSOLE_PRINT("APP_UDP_TASK: Source IP %d.%d.%d.%d \r\n", socket_info.sourceIPaddress.v4Add.v[0], socket_info.sourceIPaddress.v4Add.v[1], socket_info.sourceIPaddress.v4Add.v[2], socket_info.sourceIPaddress.v4Add.v[3]);
                    SYS_CONSOLE_PRINT("APP_UDP_TASK: Local  IP %d.%d.%d.%d \r\n", socket_info.localIPaddress.v4Add.v[0], socket_info.localIPaddress.v4Add.v[1], socket_info.localIPaddress.v4Add.v[2], socket_info.localIPaddress.v4Add.v[3]);
                    SYS_CONSOLE_PRINT("APP_UDP_TASK: Remote IP %d.%d.%d.%d \r\n", socket_info.remoteIPaddress.v4Add.v[0], socket_info.remoteIPaddress.v4Add.v[1], socket_info.remoteIPaddress.v4Add.v[2], socket_info.remoteIPaddress.v4Add.v[3]);
                    SYS_CONSOLE_PRINT("APP_UDP_TASK: Local  port %d \r\n", socket_info.localPort);
                    SYS_CONSOLE_PRINT("APP_UDP_TASK: Remote port %d \r\n", socket_info.remotePort);
                    SYS_CONSOLE_PRINT("APP_UDP_TASK: rxQueue size %d \r\n", socket_info.rxQueueSize);
                    SYS_CONSOLE_PRINT("APP_UDP_TASK: tx size %d \r\n", socket_info.txSize);
                #endif
                //-----------------------------------------------------------------
            }
//            taskYIELD();
            break;
        }
        case APP_UDP_TASK_STATE_Rx:
        {
            app_udp_taskData.state = APP_UDP_TASK_STATE_Tx;

            //------------------------------------------------------------------
            uint16_t read_len = TCPIP_UDP_GetIsReady(app_udp_taskData.udp_rx_socket);

            if (read_len > 0)
            {
                app_udp_taskData.event_info.pData = malloc(read_len);
                configASSERT(app_udp_taskData.event_info.pData);
                
                app_udp_taskData.event_info.data_len = TCPIP_UDP_ArrayGet(app_udp_taskData.udp_rx_socket, app_udp_taskData.event_info.pData, read_len);
                app_udp_taskData.event_info.event_id = (ENUM_EVENT_TYPE)EVENT_TYPE_AMAK_UDP_POCKET;
                
                TCPIP_UDP_Discard(app_udp_taskData.udp_rx_socket);
                
                free(app_udp_taskData.event_info.pData);
                app_udp_taskData.event_info.data_len = 0;
//                xQueueSend( eventQueue_app_amak_parser_task, (void*)&( app_udp_taskData.event_info ), 0 );//portMAX_DELAY); 

              #ifdef ENABLE_CONSOLE_MESSAGE
                  SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: an UDP-packet receive\r\n");
              #endif
            }

//            taskYIELD();
            
            break;
        }
        
        case APP_UDP_TASK_STATE_Tx:
        {
            app_udp_taskData.state = APP_UDP_TASK_STATE_Rx;

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
                    TCPIP_NET_HANDLE    netH;

                    netH = socket_info.hNet;
                    netName = TCPIP_STACK_NetNameGet(netH);
                    netBiosName = TCPIP_STACK_NetBIOSName(netH);
                    ipAddr.Val = TCPIP_STACK_NetAddress(netH);

                    SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: --== Tx socket ==--\r\n");

                    SYS_CONSOLE_PRINT("    Interface %s on host %s \r\n", netName, netBiosName);
                    SYS_CONSOLE_MESSAGE(" IP Address: ");
                    SYS_CONSOLE_PRINT("%d.%d.%d.%d \r\n", ipAddr.v[0], ipAddr.v[1], ipAddr.v[2], ipAddr.v[3]);

                    SYS_CONSOLE_PRINT("APP_UDP_TASK: Dest   IP %d.%d.%d.%d \r\n", socket_info.destIPaddress.v4Add.v[0], socket_info.destIPaddress.v4Add.v[1], socket_info.destIPaddress.v4Add.v[2], socket_info.destIPaddress.v4Add.v[3]);
                    SYS_CONSOLE_PRINT("APP_UDP_TASK: Source IP %d.%d.%d.%d \r\n", socket_info.sourceIPaddress.v4Add.v[0], socket_info.sourceIPaddress.v4Add.v[1], socket_info.sourceIPaddress.v4Add.v[2], socket_info.sourceIPaddress.v4Add.v[3]);
                    SYS_CONSOLE_PRINT("APP_UDP_TASK: Local  IP %d.%d.%d.%d \r\n", socket_info.localIPaddress.v4Add.v[0], socket_info.localIPaddress.v4Add.v[1], socket_info.localIPaddress.v4Add.v[2], socket_info.localIPaddress.v4Add.v[3]);
                    SYS_CONSOLE_PRINT("APP_UDP_TASK: Remote IP %d.%d.%d.%d \r\n", socket_info.remoteIPaddress.v4Add.v[0], socket_info.remoteIPaddress.v4Add.v[1], socket_info.remoteIPaddress.v4Add.v[2], socket_info.remoteIPaddress.v4Add.v[3]);
                    SYS_CONSOLE_PRINT("APP_UDP_TASK: Local  port %d \r\n", socket_info.localPort);
                    SYS_CONSOLE_PRINT("APP_UDP_TASK: Remote port %d \r\n", socket_info.remotePort);
                    SYS_CONSOLE_PRINT("APP_UDP_TASK: rxQueue size %d \r\n", socket_info.rxQueueSize);
                    SYS_CONSOLE_PRINT("APP_UDP_TASK: tx size %d \r\n", socket_info.txSize);
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
                    TCPIP_UDP_DestinationIPAddressSet( app_udp_taskData.udp_tx_socket, IP_ADDRESS_TYPE_IPV4, &(app_udp_taskData.dest_adr) );
                    TCPIP_UDP_DestinationPortSet( app_udp_taskData.udp_tx_socket, app_udp_taskData.dest_port );
                
                    
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

//            taskYIELD();
            break;
        }
        case APP_UDP_TASK_STATE_CLOSING_CONNECTION:
        {
            break;
        }
        case APP_UDP_TASK_STATE_Error:
        {
            taskYIELD();    // ???
            
            break;
        }

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            taskYIELD();    //  ???
            
            break;
        }
    }

}
//------------------------------------------------------------------------------
// End of File
//------------------------------------------------------------------------------
