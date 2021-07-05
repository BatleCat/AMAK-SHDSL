/*------------------------------------------------------------------------------
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.
    TNG-Group.

  File Name:
    app_amak_parser_task.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.
 *  ?????? UDP-??????? ? SHDSL-??????? ??? ??????? ????-100

  Description:
    This file contains the source code for the MPLAB Harmony application.
 *  ?????? UDP-??????? ????-100 ? ???????? ????? ?? SHDSL-?????? ??? ???????? ??
 *  ??????????? ?????? ??? ?????? SHDSL-??????. ?????? UDP-?????? ?? SHDSL-????-
 *  ???.  UDP-?????? ??????????? ? ???????????? ? ?????? App_UDP_Task().  SHDSL-
 *  ?????? ???????????? ? ??????????? ? ?????? App_SHDSL_Task().
------------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Section: Included Files
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#include "app_amak_parser_task.h"
//------------------------------------------------------------------------------
#include <stddef.h>
#include <string.h>
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
//------------------------------------------------------------------------------
// Section: Global Data Definitions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

/*------------------------------------------------------------------------------
   Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_AMAK_PARSER_TASK_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_AMAK_PARSER_TASK_DATA app_amak_parser_taskData;

/*------------------------------------------------------------------------------
  Event Queue for AMAK Parser Task Application

  Summary:
    This variable holds event queue for AMAK parser task application

  Description:
    
  Remarks:
    
*/
//------------------------------------------------------------------------------
QueueHandle_t eventQueue_app_amak_parser_task = NULL;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Section: Application Callback Functions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

/* TODO:  Add any necessary callback functions.
*/

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Section: Application Local Functions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

/* TODO:  Add any necessary local functions.
*/


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Section: Application Initialization and State Machine Functions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

/*------------------------------------------------------------------------------
  Function:
    void APP_AMAK_PARSER_TASK_Initialize ( void )

  Remarks:
    See prototype in app_amak_parser_task.h.
 */
//------------------------------------------------------------------------------
void APP_AMAK_PARSER_TASK_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_amak_parser_taskData.state = APP_AMAK_PARSER_TASK_STATE_INIT;

    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
    //--------------------------------------------------------------------------
    eventQueue_app_amak_parser_task = xQueueCreate( APP_AMAK_PARSER_TASK_QUEUE_LEN, EVENT_INFO_SIZE );
    if (NULL == eventQueue_app_amak_parser_task)
    {
        /* Handle error condition. Not sufficient memory to create Queue */
        #ifdef ENABLE_CONSOLE_MESSAGE
            SYS_CONSOLE_MESSAGE(" APP_AMAK_PARSER_TASK: Handle error condition. Not sufficient memory to create Queue\r\n");
        #endif
    }
    //--------------------------------------------------------------------------
    app_amak_parser_taskData.event_info.data_len = 0;
    app_amak_parser_taskData.event_info.pData = NULL;
    app_amak_parser_taskData.event_info.event_id = ENUM_EVENT_TYPE.EVENT_TYPE_UNKNOWN;
    //--------------------------------------------------------------------------
    app_amak_parser_taskData.amak2shdsl_frame_count = 0;
    app_amak_parser_taskData.shdsl2amak_frame_count = 0;
    memset(app_amak_parser_taskData.amak2shdsl_frame, 0, AMAK_UDP_POCKET_SIZE);
    memset(app_amak_parser_taskData.shdsl2amak_frame, 0, AMAK_UDP_POCKET_SIZE);
    //--------------------------------------------------------------------------
}
//------------------------------------------------------------------------------


/*------------------------------------------------------------------------------
  Function:
    void APP_AMAK_PARSER_TASK_Tasks ( void )

  Remarks:
    See prototype in app_amak_parser_task.h.
 */
//------------------------------------------------------------------------------
void APP_AMAK_PARSER_TASK_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( app_amak_parser_taskData.state )
    {
        /* Application's initial state. */
        case APP_AMAK_PARSER_TASK_STATE_INIT:
        {
            bool appAmakParserTaskInitialized = false;
            
            if (NULL != eventQueue_app_amak_parser_task)
            {
                appAmakParserTaskInitialized = true;
            }

            if (appAmakParserTaskInitialized)
            {

                app_amak_parser_taskData.state = APP_AMAK_PARSER_TASK_STATE_SERVICE_TASKS;

                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_MESSAGE(" APP_AMAK_PARSER_TASK: Initialization complite\r\n");
                #endif
            }
            break;
        }

        case APP_AMAK_PARSER_TASK_STATE_SERVICE_TASKS:
        {

            app_amak_parser_taskData.event_info = ENUM_EVENT_TYPE.EVENT_TYPE_UNKNOWN;
            //------------------------------------------------------------------
            // Проверяем есть ли в очереди данные на передачу
            //------------------------------------------------------------------
            xQueueReceive( eventQueue_app_amak_parser_task, &app_amak_parser_taskData.event_info, portMAX_DELAY );

            //------------------------------------------------------------------
            // Если данные в очереди от App_UDP_Task
            //------------------------------------------------------------------
            if ( ENUM_EVENT_TYPE.EVENT_TYPE_AMAK_UDP_POCKET == app_amak_parser_taskData.event_info.event_id )
            {
                // копирую данные из принятого события в app_amak_parser_taskData.amak2shdsl_pocket
                memset(app_amak_parser_taskData.amak2shdsl_frame, 0, AMAK_UDP_POCKET_SIZE);
                memcpy(app_amak_parser_taskData.amak2shdsl_frame, app_amak_parser_taskData.event_info.pData, app_amak_parser_taskData.event_info.data_len);
                // освобождаю память по указателю данных из события
                free(app_amak_parser_taskData.event_info.pData);
                
                uint16_t rx_udp_len = app_amak_parser_taskData.event_info.data_len;
                app_amak_parser_taskData.event_info.event_id = EVENT_TYPE_SHDSL_DATA_POCKET;

                uint16_t        k = 0;
                uint16_t        count = 0;
                pSHDSL_POCKET   shdsl_pocket = malloc(SHDSL_POCKET_SIZE);
                shdsl_pocket->frame_count = app_amak_parser_taskData.amak2shdsl_frame_count;
                
                shdsl_pocket->tx_pocket_count = app_amak_parser_taskData.amak2shdsl_frame_count;
                shdsl_pocket->rx_pocket_count = app_amak_parser_taskData.shdsl2amak_frame_count;
                shdsl_pocket->shdsl_pocket_count :9;  //lo
                shdsl_pocket->frame_count        :3;  //mid
                shdsl_pocket->pocket_id = ENUM_SHDSL_POCKET_ID.AMAK;
                shdsl_pocket->data[SHDSL_POCKET_DATA_LEN];
                shdsl_pocket->crc;
                
                uint16_t i;
                for (i = 0; i < rx_udp_len; i++)
                {
                    
                }
                
                configASSERT(app_amak_parser_taskData.event_info.pData);
                
                app_amak_parser_taskData.event_info.data_len = pocket_len;
                app_amak_parser_taskData.event_info.event_id = ENUM_EVENT_TYPE.EVENT_TYPE_SHDSL_DATA_POCKET;
                
                xQueueSend( eventQueue_app_shdsl_task, (void*)&( app_amak_parser_taskData.event_info ), 0 );//portMAX_DELAY); 

                // увеличиваю счетчик amak2shdsl_pocket_count UDP-пакетов
                app_amak_parser_taskData.amak2shdsl_frame_count++;
                app_amak_parser_taskData.amak2shdsl_frame_count &= 0x01FF;
            }
            //------------------------------------------------------------------
            // Если данные в очереди от App_SHDSL_Task
            //------------------------------------------------------------------
            if ( ENUM_EVENT_TYPE.EVENT_TYPE_SHDSL_DATA_POCKET == app_amak_parser_taskData.event_info.event_id )
            {
                
                configASSERT(app_amak_parser_taskData.event_info.pData);
                
                app_amak_parser_taskData.event_info.data_len = pocket_len;
                app_amak_parser_taskData.event_info.event_id = ENUM_EVENT_TYPE.EVENT_TYPE_AMAK_UDP_POCKET;
                
                xQueueSend( eventQueue_app_udp_task, (void*)&( app_amak_parser_taskData.event_info ), 0 );//portMAX_DELAY); 
            }
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

//------------------------------------------------------------------------------
// End of File
//------------------------------------------------------------------------------
