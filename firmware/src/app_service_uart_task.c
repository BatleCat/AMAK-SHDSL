/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.
    TNG-Group.

  File Name:
    app_service_uart_task.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.
    ????????? ??????? ?????? SHDSL-?????? ?? UART.

  Description:
    This file contains the source code for the MPLAB Harmony application.
 *  ????????? ????????? ?????? ???????? ????????? ? ?????????  SHDSL-??????, ??-
 *  ???????? ?? ?????? App_UDP_Task() // ?? UDP-????????? //, ? ???????? ???????
 *  SHDSL-?????? ? ?????? App_UDP_Task() ??? ???????? ?? ?? UDP-????????? ?????-
 *  ????? ????????????. ????? ? SHDSL-??????? ??????? ?? UART.
 *******************************************************************************/

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Section: Included Files
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

//------------------------------------------------------------------------------
#include "app_service_uart_task.h"
//------------------------------------------------------------------------------
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "system_config.h"
#include "system_definitions.h"

#include "third_party/rtos/FreeRTOS/Source/include/FreeRTOS.h"
#include "third_party/rtos/FreeRTOS/Source/include/queue.h"
#include "config/default/system/debug/sys_debug.h"
#include "config/default/system/console/sys_console.h"
#include "driver/usart/drv_usart.h"

#include <bsp/bsp.h>

//#include "FreeRTOS.h"
//#include "queue.h"
////#include "config/default/system/debug/sys_debug.h"
//#include "config/default/system/console/sys_console.h"
//------------------------------------------------------------------------------
#include "app_commands.h"
#include "AMAK_SHDSL_Queue.h"
//------------------------------------------------------------------------------
#define ENABLE_CONSOLE_MESSAGE
//------------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Section: Global Data Definitions
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_SERVICE_UART_TASK_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

//------------------------------------------------------------------------------
APP_SERVICE_UART_TASK_DATA app_service_uart_taskData;
//------------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Section: Application Callback Functions
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/* TODO:  Add any necessary callback functions.
*/

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Section: Application Local Functions
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------


/* TODO:  Add any necessary local functions.
*/


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// Section: Application Initialization and State Machine Functions
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// <editor-fold defaultstate="collapsed" desc="Function: void APP_SERVICE_UART_TASK_Initialize ( void )">
/*------------------------------------------------------------------------------
  Function:
    void APP_SERVICE_UART_TASK_Initialize ( void )

  Remarks:
    See prototype in app_service_uart_task.h.
 */
//------------------------------------------------------------------------------
void APP_SERVICE_UART_TASK_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_service_uart_taskData.state = APP_SERVICE_UART_TASK_STATE_INIT;



    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}
//------------------------------------------------------------------------------
// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="Function: void APP_SERVICE_UART_TASK_Tasks ( void )">
/*-----------------------------------------------------------------------------
  Function:
    void APP_SERVICE_UART_TASK_Tasks ( void )

  Remarks:
    See prototype in app_service_uart_task.h.
 */
//------------------------------------------------------------------------------
void APP_SERVICE_UART_TASK_Tasks ( void )
{
    static int step = 0;
    /* Check the application's current state. */
    switch ( app_service_uart_taskData.state )
    {
        /* Application's initial state. */
        case APP_SERVICE_UART_TASK_STATE_INIT:
        {
            LED1_Toggle();
            vTaskDelay(500/portTICK_PERIOD_MS);
            
            #ifdef ENABLE_CONSOLE_MESSAGE
                   SYS_CONSOLE_MESSAGE("APP_SERVICE_UART_TASK: start init\r\n");
                   step++; 
                   SYS_CONSOLE_PRINT("%d", step);
            #endif

            bool appInitialized = APP_Commands_Init();

            if (appInitialized)
            {
                LED2_Toggle();
                #ifdef ENABLE_CONSOLE_MESSAGE
                       step++; 
                       SYS_CONSOLE_PRINT(" %d", step);
                #endif
                if ( SYS_STATUS_READY == DRV_USART_Status(sysObj.drvUsart0) )
                {
                    LED3_Toggle();
                    
                    #ifdef ENABLE_CONSOLE_MESSAGE
                           step++; 
                           SYS_CONSOLE_PRINT(" %d", step);
                    #endif

                    app_service_uart_taskData.usart_handle = DRV_USART_Open(DRV_USART_INDEX_0, DRV_IO_INTENT_READWRITE);
                    if ( DRV_HANDLE_INVALID != app_service_uart_taskData.usart_handle )
                    {
                        #ifdef ENABLE_CONSOLE_MESSAGE
                               step++; 
                               SYS_CONSOLE_PRINT(" %d\r\n", step);
                        #endif

//                        int strlen = sprintf(app_service_uart_taskData.usart_buf, "config show \r\n");
//                        strcpy(app_service_uart_taskData.usart_buf, "config show \r\n");
                        strcpy(app_service_uart_taskData.usart_buf, "get dslstate\r\n");
                        
                        DRV_USART_WriteBuffer(app_service_uart_taskData.usart_handle, app_service_uart_taskData.usart_buf, strlen(app_service_uart_taskData.usart_buf) );

                        #ifdef ENABLE_CONSOLE_MESSAGE
                                SYS_CONSOLE_PRINT("%s \r\n", app_service_uart_taskData.usart_buf);
                                SYS_CONSOLE_MESSAGE("APP_SERVICE_UART_TASK: init complite\r\n");
                        #endif
//                        app_service_uart_taskData.state = APP_SERVICE_UART_TASK_STATE_SEND_UDP;
                        app_service_uart_taskData.state = APP_SERVICE_UART_TASK_STATE_SERVICE_TASKS;
                    }
                }
                LED1_Toggle();
                LED2_Toggle();
                LED3_Toggle();
            }
            break;
        }

        case APP_SERVICE_UART_TASK_STATE_SERVICE_TASKS:
        {
            char usartData = '.';
            
//            DRV_USART_ReadBuffer(app_service_uart_taskData.usart_handle, &usartData, 1 );
//            SYS_CONSOLE_PRINT("%c", usartData);

            if (DRV_USART_ReadBuffer(app_service_uart_taskData.usart_handle, &usartData, 1 ) == true)
            {
                SYS_CONSOLE_PRINT("%c", usartData);
            }

//            if (APP_Send_Packet)
//            {
//                APP_Send_Packet = false;
//                app_service_uart_taskData.state = APP_SERVICE_UART_TASK_STATE_SEND_UDP;
//            }

//            taskYIELD();
            
            break;
        }
        
        case APP_SERVICE_UART_TASK_STATE_SEND_UDP:
        {
            app_service_uart_taskData.state = APP_SERVICE_UART_TASK_STATE_SERVICE_TASKS;
            
//            vTaskDelay( 1000 / portTICK_PERIOD_MS );
            
            app_service_uart_taskData.event_info.pData = malloc(1024);
            configASSERT(app_service_uart_taskData.event_info.pData);
            memset(app_service_uart_taskData.event_info.pData, 0xAA, 1024);

            app_service_uart_taskData.event_info.data_len = 1024;
            app_service_uart_taskData.event_info.event_id = (ENUM_EVENT_TYPE)EVENT_TYPE_UART_SERVICE_POCKET;
//            xQueueSend( eventQueue_app_udp_task, (void*)&( app_service_uart_taskData.event_info ), 0 );//portMAX_DELAY); 

            break;
        }

        /* TODO: implement your application state machine.*/


        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}
//------------------------------------------------------------------------------
// </editor-fold>

/*------------------------------------------------------------------------------
 End of File
 */
