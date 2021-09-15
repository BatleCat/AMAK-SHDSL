/*------------------------------------------------------------------------------

  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.
    TNG-Group

  File Name:
    app_shdsl_task.c

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
 ------------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Section: Included Files
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#include "app_shdsl_task.h"
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
    This structure should be initialized by the APP_SHDSL_TASK_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/
//------------------------------------------------------------------------------
APP_SHDSL_TASK_DATA app_shdsl_taskData;
//------------------------------------------------------------------------------

/*------------------------------------------------------------------------------
  Event Queue for SHDSL Task Application

  Summary:
    This variable holds event queue for SHDSL task application

  Description:
    
  Remarks:
    
*/
//------------------------------------------------------------------------------
QueueHandle_t eventQueue_app_shdsl_task = NULL;
//------------------------------------------------------------------------------

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
    void APP_SHDSL_TASK_Initialize ( void )

  Remarks:
    See prototype in app_shdsl_task.h.
 */
//------------------------------------------------------------------------------
void APP_SHDSL_TASK_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_shdsl_taskData.state = APP_SHDSL_TASK_STATE_INIT;

//    /* TODO: Initialize your application's state machine and other
//     * parameters.
//     */
//    //--------------------------------------------------------------------------
//    eventQueue_app_shdsl_task = xQueueCreate( APP_SHDSL_TASK_QUEUE_LEN, EVENT_INFO_SIZE );
//    if (NULL == eventQueue_app_shdsl_task)
//    {
//        /* Handle error condition. Not sufficient memory to create Queue */
//        #ifdef ENABLE_CONSOLE_MESSAGE
//            SYS_CONSOLE_MESSAGE(" APP_SHDSL_TASK: Handle error condition. Not sufficient memory to create Queue\r\n");
//        #endif
//    }
//    //--------------------------------------------------------------------------
//    app_shdsl_taskData.event_info.data_len = 0;
//    app_shdsl_taskData.event_info.pData = NULL;
//    app_shdsl_taskData.event_info.event_id = (ENUM_EVENT_TYPE)EVENT_TYPE_UNKNOWN; //ENUM_EVENT_TYPE.EVENT_TYPE_UNKNOWN;
//    //--------------------------------------------------------------------------
//    app_shdsl_taskData.rx_pocket_count = 0;
//    app_shdsl_taskData.tx_pocket_count = 0;
//    //--------------------------------------------------------------------------
}
//------------------------------------------------------------------------------


/*------------------------------------------------------------------------------
  Function:
    void APP_SHDSL_TASK_Tasks ( void )

  Remarks:
    See prototype in app_shdsl_task.h.
 */
//------------------------------------------------------------------------------
void APP_SHDSL_TASK_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( app_shdsl_taskData.state )
    {
        /* Application's initial state. */
        case APP_SHDSL_TASK_STATE_INIT:
        {
            bool appSHDSLTaskInitialized = false;
            
//            if (NULL != eventQueue_app_amak_parser_task)
//            {
//                appSHDSLTaskInitialized = true;
//            }

            if (appSHDSLTaskInitialized)
            {

                app_shdsl_taskData.state = APP_SHDSL_TASK_STATE_SERVICE_TASKS;

                #ifdef ENABLE_CONSOLE_MESSAGE
                    SYS_CONSOLE_MESSAGE(" APP_SHDSL_TASK: Initialization complite\r\n");
                #endif
            }
            break;
        }

        case APP_SHDSL_TASK_STATE_SERVICE_TASKS:
        {
            app_shdsl_taskData.event_info.event_id = (ENUM_EVENT_TYPE)EVENT_TYPE_UNKNOWN; //ENUM_EVENT_TYPE.EVENT_TYPE_UNKNOWN;
            //------------------------------------------------------------------
            // Проверяем есть ли в очереди данные на передачу
            //------------------------------------------------------------------
            xQueueReceive( eventQueue_app_shdsl_task, &app_shdsl_taskData.event_info, portMAX_DELAY );

            //------------------------------------------------------------------
            // Если данные в очереди от App_AMAK_Parser_Task
            //------------------------------------------------------------------
            if ( (ENUM_EVENT_TYPE)EVENT_TYPE_SHDSL_DATA_POCKET == app_shdsl_taskData.event_info.event_id )
            {
                //--------------------------------------------------------------
                // копирую данные из принятого события
                //--------------------------------------------------------------
                pSHDSL_POCKET shdsl_pocket = malloc(SHDSL_POCKET_SIZE);
                memset(shdsl_pocket, 0, SHDSL_POCKET_SIZE);
                memcpy(shdsl_pocket, app_shdsl_taskData.event_info.pData, app_shdsl_taskData.event_info.data_len);
                //--------------------------------------------------------------
                // освобождаю память по указателю данных из события
                //--------------------------------------------------------------
                free(app_shdsl_taskData.event_info.pData);
                
                //--------------------------------------------------------------
                //--------------------------------------------------------------
                app_shdsl_taskData.event_info.event_id = (ENUM_EVENT_TYPE)EVENT_TYPE_SHDSL_DATA_POCKET;
                app_shdsl_taskData.event_info.data_len = SHDSL_POCKET_SIZE;

                shdsl_pocket->rx_pocket_count = app_shdsl_taskData.rx_pocket_count;   // поле заполняется в app_shdsl_task, перед отправкой пакета

                //shdsl_pocket->crc                   = 0;                        // расчет CRC выполняется в app_shdsl_task, перед отправкой пакета

                //--------------------------------------------------------------
                // отправляю пакет в SHDSL-модем через SPI (DMA-операция)
                //--------------------------------------------------------------
                
                //--------------------------------------------------------------
                // принимаю пакет из SHDSL-модема
                //--------------------------------------------------------------
                app_shdsl_taskData.rx_pocket_count++;
                
                //--------------------------------------------------------------
                // проверяю CRC пакета
                //--------------------------------------------------------------
                    //----------------------------------------------------------
                    // Ощибка CRC! Может проверку делать в только App_AMAK_Parser_Task() ?!
                    // Отправить сообщение типа EVENT_TYPE_SHDSL_STATE_POCKET
                    // в App_UDP_Task с описанием ошибки для отправки оператору
                    //----------------------------------------------------------
                
                //--------------------------------------------------------------
                // отправляю принятый пакет в App_AMAK_Parser_Task
                //--------------------------------------------------------------
                xQueueSend( eventQueue_app_amak_parser_task, (void*)&( app_shdsl_taskData.event_info ), 0 );//portMAX_DELAY);
            }

            taskYIELD();

            break;
        }

        /* TODO: implement your application state machine.*/


        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            taskYIELD();

            break;
        }
    }
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// End of File
//------------------------------------------------------------------------------
