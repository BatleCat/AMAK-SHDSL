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
#include "config/default/library/tcpip/tcp.h"
#include "config/default/library/tcpip/udp.h"
#include "third_party/rtos/FreeRTOS/Source/include/FreeRTOS.h"
#include "third_party/rtos/FreeRTOS/Source/include/queue.h"
//#include "config/default/system/debug/sys_debug.h"
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
    
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
    eventQueue_app_udp_task = xQueueCreate( 3, EVENT_INFO_SIZE );
    if (NULL == eventQueue_app_udp_task)
    {
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
    static IPV4_ADDR    dwLastIP[2] = { {-1}, {-1} };
    IPV4_ADDR           ipAddr;

    /* Check the application's current state. */
    switch ( app_udp_taskData.state )
    {
        /* Application's initial state. */
        case APP_UDP_TASK_STATE_INIT:
        {
            bool appInitialized = true;
       
            if (appInitialized)
            {
                app_udp_taskData.state = APP_UDP_TASK_STATE_Start;
//                SYS_PORTS_PinClear(PORTS_ID_0, PORT_CHANNEL_D, PORTS_BIT_POS_10);
//                SYS_PORTS_PinSet(PORTS_ID_0, PORT_CHANNEL_H, PORTS_BIT_POS_0);
                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: Initialization complite\r\n");
                #endif
            }
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
                    SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP_STATE_Error\r\n");
                #endif
                app_udp_taskData.state = APP_UDP_TASK_STATE_Error;
            }
            else if(SYS_STATUS_READY == tcpipStat)
            {
                int nNets, i;
                const char *netName, *netBiosName;
                TCPIP_NET_HANDLE    netH;
                
                nNets = TCPIP_STACK_NumberOfNetworksGet();
                for(i = 0; i < nNets; i++)
                {
                    netH = TCPIP_STACK_IndexToNet(i);
                    netName = TCPIP_STACK_NetNameGet(netH);
                    netBiosName = TCPIP_STACK_NetBIOSName(netH);
                    #ifdef ENABLE_CONSOLE_MESSAGE
                        SYS_CONSOLE_PRINT("    Interface %s on host %s - NBNS disabled\r\n", netName, netBiosName);
                    #endif
                }
                
                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP_STATE_WAIT_FOR_IP\r\n");
                #endif
                app_udp_taskData.state = APP_UDP_TASK_STATE_WAIT_FOR_IP;
            }
            break;
        }
        case APP_UDP_TASK_STATE_WAIT_FOR_IP:
        {
            int nNets, i;
            TCPIP_NET_HANDLE    netH;
            
            nNets = TCPIP_STACK_NumberOfNetworksGet();
            for (i = 0; i < nNets; i++)
            {
                netH = TCPIP_STACK_IndexToNet(i);
                if(false == TCPIP_STACK_NetIsReady(netH))
                {
                    #ifdef ENABLE_CONSOLE_MESSAGE
                        SYS_CONSOLE_MESSAGE("APP_UDP_TASK: Net not ready \r\n");//(int)TCPIP_STACK_NetIsReady(netH));
                        SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP_STATE_Error\r\n");
                    #endif
                    app_udp_taskData.state = APP_UDP_TASK_STATE_Error;
                    return; // interface not ready yet!
                }
                ipAddr.Val = TCPIP_STACK_NetAddress(netH);
                if(dwLastIP[i].Val != ipAddr.Val)
                {
                    dwLastIP[i].Val = ipAddr.Val;

                    #ifdef ENABLE_CONSOLE_MESSAGE
                        SYS_CONSOLE_MESSAGE(TCPIP_STACK_NetNameGet(netH));
                        SYS_CONSOLE_MESSAGE(" IP Address: ");
                        SYS_CONSOLE_PRINT("%d.%d.%d.%d \r\n", ipAddr.v[0], ipAddr.v[1], ipAddr.v[2], ipAddr.v[3]);
                    #endif
                }
            }

            #ifdef ENABLE_CONSOLE_MESSAGE
                SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP_STATE_WAIT_SERVER_OPEN\r\n");
            #endif
            app_udp_taskData.state = APP_UDP_TASK_STATE_WAIT_SERVER_OPEN;
            
            break;
        }
        case APP_UDP_TASK_STATE_WAIT_SERVER_OPEN:
        {
//            IP_MULTI_ADDRESS dest_adr = {.v4Add.v = {10, 2, 22, 220}};
//            UDP_PORT  dest_port = 1500;

            IP_MULTI_ADDRESS local_adr = {.v4Add.v = {10, 2, 22, 245}};
            UDP_PORT  local_port = 1500;

            app_udp_taskData.udp_rx_socket = TCPIP_UDP_ServerOpen(IP_ADDRESS_TYPE_IPV4, local_port, &local_adr);
            if (INVALID_SOCKET != app_udp_taskData.udp_rx_socket)
            {
                if (true == TCPIP_UDP_Bind(app_udp_taskData.udp_rx_socket, IP_ADDRESS_TYPE_IPV4, local_port, &local_adr))
                {
                    #ifdef ENABLE_CONSOLE_MESSAGE
                        SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP server open\r\n");
                    #endif
                }
                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP_STATE_WAIT_CLIENT_OPEN\r\n");
                #endif
                app_udp_taskData.state = APP_UDP_TASK_STATE_WAIT_CLIENT_OPEN;
            }
            break;
        }
        case APP_UDP_TASK_STATE_WAIT_CLIENT_OPEN:
        {
//            IP_MULTI_ADDRESS dest_adr = {.v4Add.v = {10, 2, 22, 220}};
//            UDP_PORT  dest_port = 1500;
//
//            IP_MULTI_ADDRESS local_adr = {.v4Add.v = {10, 2, 22, 245}};
//            UDP_PORT  local_port = 1500;
//
//            app_udp_taskData.udp_tx_socket = TCPIP_UDP_ClientOpen(IP_ADDRESS_TYPE_IPV4, dest_port, 0);//&dest_adr);
//            if (INVALID_SOCKET != app_udp_taskData.udp_tx_socket)
//            {
//                if (true == TCPIP_UDP_Bind(app_udp_taskData.udp_tx_socket, IP_ADDRESS_TYPE_IPV4, local_port, &local_adr))
//                {
//                    #ifdef ENABLE_CONSOLE_MESSAGE
//                        SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP client open\r\n");
//                    #endif
//                }
//                #ifdef ENABLE_CONSOLE_MESSAGE
//                    SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP_STATE_WAIT_FOR_CONNECTION\r\n");
//                #endif
                app_udp_taskData.state = APP_UDP_TASK_STATE_WAIT_FOR_CONNECTION;
//            }
            break;
        }
        case APP_UDP_TASK_STATE_WAIT_FOR_CONNECTION:
        {
//            if (TCPIP_UDP_IsConnected(app_udp_taskData.udp_tx_socket))
            if (TCPIP_UDP_IsConnected(app_udp_taskData.udp_rx_socket))
            {
                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: TCPIP_UDP_IsConnected\r\n");
                #endif
                app_udp_taskData.state = APP_UDP_TASK_STATE_Rx;
            }
            break;
        }
        case APP_UDP_TASK_STATE_Rx:
        {
            app_udp_taskData.state = APP_UDP_TASK_STATE_Tx;

            //------------------------------------------------------------------
            // Проверяем есть ли данные для приема
            //------------------------------------------------------------------
            uint16_t read_len = TCPIP_UDP_GetIsReady(app_udp_taskData.udp_rx_socket);

            if (read_len > 0)
            {
                app_udp_taskData.event_info.pData = malloc(read_len);
                configASSERT(app_udp_taskData.event_info.pData);
                
                app_udp_taskData.event_info.data_len = TCPIP_UDP_ArrayGet(app_udp_taskData.udp_rx_socket, app_udp_taskData.event_info.pData, read_len);
                app_udp_taskData.event_info.event_id = ENUM_EVENT_TYPE.EVENT_TYPE_AMAK_UDP_POCKET;
                
                xQueueSend( eventQueue_app_amak_parser_task, (void*)&( app_udp_taskData.event_info ), 0 );//portMAX_DELAY); 

              #ifdef ENABLE_CONSOLE_MESSAGE
                  SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: an UDP-packet receive\r\n");
              #endif
            }

            taskYIELD();
            
            break;
        }
        
        case APP_UDP_TASK_STATE_Tx:
        {
            app_udp_taskData.state = APP_UDP_TASK_STATE_Rx;

            app_udp_taskData.event_info = ENUM_EVENT_TYPE.EVENT_TYPE_UNKNOWN;
            //------------------------------------------------------------------
            // Проверяем есть ли в очереди данные на передачу
            //------------------------------------------------------------------
            xQueueReceive( eventQueue_app_udp_task, &app_udp_taskData.event_info, 0 ); //portMAX_DELAY );

            //------------------------------------------------------------------
            // Если данные в очереди от App_AMAK_Parser_Task или от App_Service_UART_Task
            //------------------------------------------------------------------
            if ( ( ENUM_EVENT_TYPE.EVENT_TYPE_AMAK_UDP_POCKET == app_udp_taskData.event_info.event_id ) ||
                 ( ENUM_EVENT_TYPE.EVENT_TYPE_UART_SERVICE_POCKET == app_udp_taskData.event_info.event_id ) )
            {
//                #ifdef ENABLE_CONSOLE_MESSAGE
//                  SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP-packet send start\r\n");
//                #endif
                uint16_t udp_buf_len;
//                udp_buf_len = TCPIP_UDP_PutIsReady(app_udp_taskData.udp_tx_socket);
                udp_buf_len = TCPIP_UDP_PutIsReady(app_udp_taskData.udp_rx_socket);
                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_PRINT("APP_UDP_TASK: tx_bufer size = %d \r\n", udp_buf_len);
                #endif
                if (udp_buf_len >= app_udp_taskData.event_info.data_len)
                {
//                    TCPIP_UDP_ArrayPut(app_udp_taskData.udp_tx_socket, app_udp_taskData.udp_tx_data, app_udp_taskData.udp_tx_data_len);
//                    TCPIP_UDP_Flush(app_udp_taskData.udp_tx_socket);
                    TCPIP_UDP_ArrayPut(app_udp_taskData.udp_rx_socket, app_udp_taskData.event_info.pData, app_udp_taskData.event_info.data_len);
                    TCPIP_UDP_Flush(app_udp_taskData.udp_rx_socket);
                    #ifdef ENABLE_CONSOLE_MESSAGE
                        SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: UDP-packet send complite\r\n");
                    #endif
                }
                else
                {
//                    app_udp_taskData.bflag_UDP_enable = true;
                    #ifdef ENABLE_CONSOLE_MESSAGE
                        SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: Not enough memory for transmit data\r\n");
                    #endif
                }
                        
                free(app_udp_taskData.event_info.pData);
            }

            break;
        }
        case APP_UDP_TASK_STATE_CLOSING_CONNECTION:
        {
            break;
        }
        case APP_UDP_TASK_STATE_Error:
        {
            break;
        }

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }

}
//------------------------------------------------------------------------------
// End of File
//------------------------------------------------------------------------------
