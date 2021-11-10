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
#include "app_udp_task.h"
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

TAMAK_UDP_POCKET            amak2shdsl_frame;
uint16_t                    amak2shdsl_frame_count;
TAMAK_UDP_POCKET            shdsl2amak_frame;
uint16_t                    shdsl2amak_frame_count;
uint16_t                    curent_frame_count;
uint16_t                    curent_udp_pocket_size;
bool                        bError;

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

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">
  Function:
    APP_AMAK_PARSER_TASK_STATES app_amak_parser_task_init ( void )

  Summary:
    

  Description:
    This function work in APP_AMAK_PARSER_TASK_Tasks State Machine. 

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    app_amak_parser_taskData.state = app_amak_parser_task_init();
    </code>

  Remarks:
    None.
*/
//</editor-fold>
APP_AMAK_PARSER_TASK_STATES app_amak_parser_task_init(void);

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">
  Function:
    APP_AMAK_PARSER_TASK_STATES app_amak_parser_task_service_tasks ( void )

  Summary:
    

  Description:
    This function work in APP_AMAK_PARSER_TASK_Tasks State Machine. 
  

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    app_amak_parser_taskData.state = app_amak_parser_task_service_tasks();
    </code>

  Remarks:
    None.
*/
//</editor-fold>
APP_AMAK_PARSER_TASK_STATES app_amak_parser_task_service_tasks(void);

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">
  Function:
    void app_amak_parse_shdsl_data_pocket ( void )

  Summary:
    

  Description:
    This function ...

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    app_amak_parse_shdsl_data_pocket();
    </code>

  Remarks:
    None.
*/
//</editor-fold>
void app_amak_parse_shdsl_data_pocket(void);

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">
  Function:
    void app_amak_parse_shdsl_state_pocket ( void )

  Summary:
    

  Description:
    This function ...

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    app_amak_parse_shdsl_state_pocket();
    </code>

  Remarks:
    None.
*/
//</editor-fold>
void app_amak_parse_shdsl_state_pocket(void);

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">
  Function:
    void app_amak_parse_uart_service_pocket ( void )

  Summary:
    

  Description:
    This function ...

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    app_amak_parse_uart_service_pocket();
    </code>

  Remarks:
    None.
*/
//</editor-fold>
void app_amak_parse_uart_service_pocket(void);

/*------------------------------------------------------------------------------
<editor-fold defaultstate="collapsed" desc="//-----------------------------------------------------------------------------">
  Function:
    void app_amak_parse_udp_pocket( void )

  Summary:
    

  Description:
    This function ...

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    app_amak_parse_udp_pocket();
    </code>

  Remarks:
    None.
*/
//</editor-fold>
void app_amak_parse_udp_pocket(void);

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
    app_amak_parser_taskData.event_info.event_id = (ENUM_EVENT_TYPE)EVENT_TYPE_UNKNOWN; //ENUM_EVENT_TYPE.EVENT_TYPE_UNKNOWN;
    //--------------------------------------------------------------------------
    app_amak_parser_taskData.rx_pocket_count = 0;
    app_amak_parser_taskData.tx_pocket_count = 0;
    //--------------------------------------------------------------------------
    bError = false;
    //--------------------------------------------------------------------------
    curent_frame_count = 0;
    curent_udp_pocket_size = 0;
    //--------------------------------------------------------------------------
    amak2shdsl_frame_count = 0;
    shdsl2amak_frame_count = 0;
    //--------------------------------------------------------------------------
    memset((void*)&amak2shdsl_frame, 0, AMAK_UDP_POCKET_SIZE);
    memset((void*)&shdsl2amak_frame, 0, AMAK_UDP_POCKET_SIZE);
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
            app_amak_parser_taskData.state = app_amak_parser_task_init();
            taskYIELD();
            break;
        }

        case APP_AMAK_PARSER_TASK_STATE_SERVICE_TASKS:
        {
            app_amak_parser_taskData.state = app_amak_parser_task_service_tasks();
            taskYIELD();
            break;
        }

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
APP_AMAK_PARSER_TASK_STATES app_amak_parser_task_init(void)
{
    APP_AMAK_PARSER_TASK_STATES result = app_amak_parser_taskData.state;
    
    bool appAmakParserTaskInitialized = false;

    if (NULL != eventQueue_app_amak_parser_task)
    {
        appAmakParserTaskInitialized = true;
    }

    if (appAmakParserTaskInitialized)
    {

        result = APP_AMAK_PARSER_TASK_STATE_SERVICE_TASKS;

        #ifdef ENABLE_CONSOLE_MESSAGE
            SYS_CONSOLE_MESSAGE(" APP_AMAK_PARSER_TASK: Initialization complite\r\n");
        #endif
    }
    
    return result;
}
//------------------------------------------------------------------------------
APP_AMAK_PARSER_TASK_STATES app_amak_parser_task_service_tasks(void)
{
    APP_AMAK_PARSER_TASK_STATES result = APP_AMAK_PARSER_TASK_STATE_SERVICE_TASKS;
    
    app_amak_parser_taskData.event_info.event_id = (ENUM_EVENT_TYPE)EVENT_TYPE_UNKNOWN;
    //------------------------------------------------------------------
    // Проверяем есть ли в очереди данные на передачу
    //------------------------------------------------------------------
    xQueueReceive( eventQueue_app_amak_parser_task, &app_amak_parser_taskData.event_info, portMAX_DELAY );

    switch (app_amak_parser_taskData.event_info.event_id)
    {
        //----------------------------------------------------------------------
        case ( (ENUM_EVENT_TYPE)EVENT_TYPE_AMAK_UDP_POCKET ) :
            //------------------------------------------------------------------
            // Если данные в очереди от App_UDP_Task
            //------------------------------------------------------------------
            app_amak_parse_udp_pocket();
            result = APP_AMAK_PARSER_TASK_STATE_SERVICE_TASKS;
            break;
        //----------------------------------------------------------------------
        case ( (ENUM_EVENT_TYPE)EVENT_TYPE_SHDSL_DATA_POCKET ):
            //------------------------------------------------------------------
            // Если данные в очереди от App_SHDSL_Task
            //------------------------------------------------------------------
            app_amak_parse_shdsl_data_pocket();
            result = APP_AMAK_PARSER_TASK_STATE_SERVICE_TASKS;
            break;
        //----------------------------------------------------------------------
        case ( (ENUM_EVENT_TYPE)EVENT_TYPE_SHDSL_STATE_POCKET ) :      // событие запроса/оповещения состояния SHDSL-модема (сигналы: nLink, nReady)
            app_amak_parse_shdsl_state_pocket();
            result = APP_AMAK_PARSER_TASK_STATE_SERVICE_TASKS;
            break;
        //----------------------------------------------------------------------
        case ( (ENUM_EVENT_TYPE)EVENT_TYPE_UART_SERVICE_POCKET ) :     // событие от/для App_Service_UART_Task (управление SHDSL-модемом по терминалу)
            app_amak_parse_uart_service_pocket();
            result = APP_AMAK_PARSER_TASK_STATE_SERVICE_TASKS;
            break;
        //----------------------------------------------------------------------
        default:
            result = APP_AMAK_PARSER_TASK_STATE_SERVICE_TASKS;
            break;
        //----------------------------------------------------------------------
    }
    return result;
}
//------------------------------------------------------------------------------
void app_amak_parse_shdsl_data_pocket(void)
{
/*
    pSHDSL_POCKET shdsl_pocket = (pSHDSL_POCKET)app_amak_parser_taskData.event_info.pData;
    configASSERT(shdsl_pocket);

    //--------------------------------------------------------------
    // проверка CRC пакета
    //--------------------------------------------------------------
        //----------------------------------------------------------
        // Ощибка CRC!
        // Отправить сообщение типа EVENT_TYPE_SHDSL_STATE_POCKET
        // в App_UDP_Task с описанием ошибки для отправки оператору
        //----------------------------------------------------------
//                    bError = true;
    //--------------------------------------------------------------
    // проверка источник пакета АМАК ?
    //--------------------------------------------------------------
    if ( (ENUM_SHDSL_POCKET_ID)AMAK != shdsl_pocket->pocket_id )
    {
        //----------------------------------------------------------
        // Ощибка! Неизвестный тип пакета. Пакет данных не для АМАК!
        // Отправить сообщение типа EVENT_TYPE_SHDSL_STATE_POCKET
        // в App_UDP_Task с описанием ошибки для отправки оператору
        //----------------------------------------------------------
        bError = true;
        //----------------------------------------------------------
        #ifdef ENABLE_CONSOLE_MESSAGE
            SYS_CONSOLE_MESSAGE(" APP_AMAK_PARSER_TASK: Pocket.Id not AMAK\r\n");
        #endif
        //----------------------------------------------------------
    }

    //--------------------------------------------------------------
    // проверка есть ли потери пакетов
    //--------------------------------------------------------------
    if ( app_amak_parser_taskData.rx_pocket_count != shdsl_pocket->tx_pocket_count )
    {
        //----------------------------------------------------------
        // Ошибка! Потеряны пакеты... 
        // Отправить сообщение типа EVENT_TYPE_SHDSL_STATE_POCKET
        // в App_UDP_Task с описанием ошибки для отправки оператору
        //----------------------------------------------------------
        bError = true;
        //----------------------------------------------------------
        #ifdef ENABLE_CONSOLE_MESSAGE
            SYS_CONSOLE_MESSAGE(" APP_AMAK_PARSER_TASK: Pockets are loss\r\n");
        #endif
        //----------------------------------------------------------
    }
    app_amak_parser_taskData.rx_pocket_count = shdsl_pocket->tx_pocket_count;
    app_amak_parser_taskData.rx_pocket_count++;

    //--------------------------------------------------------------
    // Если принят SHDSL-пакет для нового UDP-пакета
    //--------------------------------------------------------------
    if (curent_frame_count != shdsl_pocket->frame_count)            
    {
        //----------------------------------------------------------
        // Если собранный UDP-пакет не нулевой длины 
        // и не было ошибок при его сборке из SHDSL-пакетов
        //----------------------------------------------------------
        if ( (0 != curent_udp_pocket_size) && (bError = false) )    
        {
            //--------------------------------------------------------------
            // пакет UDP собран из SHDSL-пакетов и отправляется в app_udp_task
            //--------------------------------------------------------------
//                        uint16_t curent_udp_pocket_size = AMAK_UDP_POCKET_SIZE;
            app_amak_parser_taskData.event_info.event_id = (ENUM_EVENT_TYPE)EVENT_TYPE_AMAK_UDP_POCKET;
            app_amak_parser_taskData.event_info.data_len = curent_udp_pocket_size;
            app_amak_parser_taskData.event_info.pData    = malloc(curent_udp_pocket_size);
            memcpy(app_amak_parser_taskData.event_info.pData, (void*)&shdsl2amak_frame, curent_udp_pocket_size);
            xQueueSend( eventQueue_app_udp_task, (void*)&( app_amak_parser_taskData.event_info ), 0 );//portMAX_DELAY); 
        }
        //--------------------------------------------------------------
        // сбрасываю флаг ошибки сборки UDP-пакета
        //--------------------------------------------------------------
        bError = false;
        //--------------------------------------------------------------
        // обнуляю текущую длину UDP-пакета
        //--------------------------------------------------------------
        curent_udp_pocket_size = 0;
        //--------------------------------------------------------------
        // запоминаю новый номер кадра (UDP-пакета)
        //--------------------------------------------------------------
        curent_frame_count = shdsl_pocket->frame_count;
        //--------------------------------------------------------------
        // обнуляю буфер для сборки следующего UDP-пакета
        //--------------------------------------------------------------
        memset((void*)&shdsl2amak_frame, 0, AMAK_UDP_POCKET_SIZE);
        //--------------------------------------------------------------
    }
    //--------------------------------------------------------------
    // копирую данные из SHDSL-пакета в собираемый UDP-пакет
    //--------------------------------------------------------------
    memcpy( shdsl2amak_frame.shdsl_pocket[shdsl_pocket->shdsl_pocket_count], shdsl_pocket->data, SHDSL_POCKET_DATA_LEN );
    curent_udp_pocket_size += SHDSL_POCKET_DATA_LEN;

    //--------------------------------------------------------------
    // освобождаю память по указателю данных из события
    //--------------------------------------------------------------
    free(shdsl_pocket->data);
*/
}
//------------------------------------------------------------------------------
void app_amak_parse_shdsl_state_pocket(void)
{
    
}
//------------------------------------------------------------------------------
void app_amak_parse_uart_service_pocket(void)
{
    
}
//------------------------------------------------------------------------------
void app_amak_parse_udp_pocket(void)
{
    #ifdef ENABLE_CONSOLE_MESSAGE
        SYS_CONSOLE_MESSAGE("APP_PARSER_TASK: receive message from App_UDP_Task\r\n");
        wait_console_buffer_free();
    #endif

    uint16_t rx_udp_len = app_amak_parser_taskData.event_info.data_len;

    #ifdef ENABLE_CONSOLE_MESSAGE
        SYS_CONSOLE_PRINT("    Rx UDP length = %d \r\n", rx_udp_len);
        SYS_CONSOLE_PRINT("%2X %2X %2X %2X %2X %2X \r\n",   
                app_amak_parser_taskData.event_info.pData[0], 
                app_amak_parser_taskData.event_info.pData[1], 
                app_amak_parser_taskData.event_info.pData[2],
                app_amak_parser_taskData.event_info.pData[3], 
                app_amak_parser_taskData.event_info.pData[4], 
                app_amak_parser_taskData.event_info.pData[5]);
        wait_console_buffer_free();
    #endif

    //--------------------------------------------------------------
    // копирую данные из принятого события в app_amak_parser_taskData.amak2shdsl_pocket
    //--------------------------------------------------------------
    memset((void*)&amak2shdsl_frame, 0, AMAK_UDP_POCKET_SIZE);
    memcpy((void*)&amak2shdsl_frame, app_amak_parser_taskData.event_info.pData, app_amak_parser_taskData.event_info.data_len);

    #ifdef ENABLE_CONSOLE_MESSAGE
        SYS_CONSOLE_PRINT("    Rx UDP length = %d \r\n", rx_udp_len);
        SYS_CONSOLE_PRINT("%2X %2X %2X %2X %2X %2X \r\n",   
                amak2shdsl_frame.udp_pocket[0], 
                amak2shdsl_frame.udp_pocket[1], 
                amak2shdsl_frame.udp_pocket[2],
                amak2shdsl_frame.udp_pocket[3], 
                amak2shdsl_frame.udp_pocket[4], 
                amak2shdsl_frame.udp_pocket[5]);
        wait_console_buffer_free();
    #endif

    //--------------------------------------------------------------
    // освобождаю память по указателю данных из события
    //--------------------------------------------------------------
    free(app_amak_parser_taskData.event_info.pData);

    //--------------------------------------------------------------
    //--------------------------------------------------------------
//    app_amak_parser_taskData.event_info.event_id = (ENUM_EVENT_TYPE)EVENT_TYPE_SHDSL_DATA_POCKET;
//    app_amak_parser_taskData.event_info.data_len = SHDSL_POCKET_SIZE;

    //--------------------------------------------------------------
    //--------------------------------------------------------------
    uint16_t frame_count = 0x07 & amak2shdsl_frame_count;

    #ifdef ENABLE_CONSOLE_MESSAGE
        SYS_CONSOLE_PRINT   ("    Frame count = %d \r\n", frame_count);
        wait_console_buffer_free();
    #endif
    //--------------------------------------------------------------
    // вычисляю количество SHDSL-пакетов для передачи UDP-пакета
    //--------------------------------------------------------------
    uint16_t num_pocket = rx_udp_len / SHDSL_POCKET_DATA_LEN;
    if ( 0 != (rx_udp_len % SHDSL_POCKET_DATA_LEN) )
    {
        num_pocket++;
    }

    #ifdef ENABLE_CONSOLE_MESSAGE
        SYS_CONSOLE_PRINT   ("    Number of SHDSL pockets = %d \r\n", num_pocket);
        wait_console_buffer_free();
    #endif
    //--------------------------------------------------------------
    // разбиваю UDP-пакет на SHDSL-пакеты и отправляю в App_SHDSL_Task()
    //--------------------------------------------------------------
//    uint16_t pocket_count;
//    for (pocket_count = 0; pocket_count < num_pocket; pocket_count++)
//    {
//        pSHDSL_POCKET   shdsl_pocket = malloc(SHDSL_POCKET_SIZE);
//        configASSERT(shdsl_pocket);
//        memset(shdsl_pocket, 0, SHDSL_POCKET_SIZE);
//
//        shdsl_pocket->tx_pocket_count       = app_amak_parser_taskData.tx_pocket_count;
//        //shdsl_pocket->rx_pocket_count       = 0;                        // поле заполняется в app_shdsl_task, перед отправкой пакета
//        shdsl_pocket->shdsl_pocket_count    = 0x01FF & pocket_count;
//        shdsl_pocket->frame_count           = frame_count;
//        shdsl_pocket->pocket_id             = (ENUM_SHDSL_POCKET_ID)AMAK;
//        memcpy(shdsl_pocket->data, amak2shdsl_frame.shdsl_pocket[pocket_count], SHDSL_POCKET_DATA_LEN);
//        //shdsl_pocket->crc                   = 0;                        // расчет CRC выполняется в app_shdsl_task, перед отправкой пакета
//        xQueueSend( eventQueue_app_shdsl_task, (void*)&( app_amak_parser_taskData.event_info ), 0 );//portMAX_DELAY);
//
//        app_amak_parser_taskData.tx_pocket_count++;
//    }
    //--------------------------------------------------------------
    // увеличиваю счетчик amak2shdsl_pocket_count UDP-пакетов
    //--------------------------------------------------------------
    amak2shdsl_frame_count++;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// End of File
//------------------------------------------------------------------------------
