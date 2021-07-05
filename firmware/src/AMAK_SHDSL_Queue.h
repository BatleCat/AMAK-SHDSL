/*------------------------------------------------------------------------------
 
 * File:   AMAK_SHDSL_Queue.h
 * Author: petruhin.as
 *
 * Created on 01/07/2021 , 11:36
//----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.
    TNG-Group.

  File Name:
    AMAK_SHDSL_Queue.h

  Summary:
    This header file provides prototypes and definitions for the application.
 * ?????????? ??????????? ????? ? ?????????? ??? ??????????? ?????? ????????.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.
 * ?????????? ??????????? ????? ? ?????????? ??? ??????????? ?????? ????????.

------------------------------------------------------------------------------*/
//------------------------------------------------------------------------------
#ifndef AMAK_SHDSL_QUEUE_H
#define	AMAK_SHDSL_QUEUE_H
//------------------------------------------------------------------------------
#include "third_party/rtos/FreeRTOS/Source/include/FreeRTOS.h"
#include "third_party/rtos/FreeRTOS/Source/include/queue.h"

//------------------------------------------------------------------------------
#ifdef	__cplusplus
extern "C" {
#endif
//------------------------------------------------------------------------------
/* EVENT TYPE

  Summary:
    Holds queue event type

  Description:
    This data type holds the application's queue data type:

 * EVENT_TYPE_SHDSL_DATA_POCKET   - ? ??????? ?????????? SHDSL-?????? ??????;
 * EVENT_TYPE_SHDSL_STATE_POCKET  - ? ??????? ?????????? ?????? ????????? SHDSL-??????;
 * EVENT_TYPE_UART_SERVICE_POCKET - ? ??????? ?????????? ????????? ??????? ?????????? SHDSL-??????? ????? UART-???? ? ?????? ??????
 * EVENT_TYPE_AMAK_UDP_POCKET     - ? ??????? ?????????? UDP-?????? ??????? ????

  Remarks:
*/
typedef enum
{
    EVENT_TYPE_UNKNOWN = 0,             // неизвестный тип источника события
    EVENT_TYPE_SHDSL_DATA_POCKET,       // событие от/для App_SHDSL_Task
    EVENT_TYPE_SHDSL_STATE_POCKET,      // событие запроса/оповещения состояния SHDSL-модема (сигналы: nLink, nReady)
    EVENT_TYPE_UART_SERVICE_POCKET,     // событие от/для App_Service_UART_Task (управление SHDSL-модемом по терминалу)
    EVENT_TYPE_AMAK_UDP_POCKET,         // событие от/для App_UDP_Task
} ENUM_EVENT_TYPE;
//------------------------------------------------------------------------------
typedef struct
{
    ENUM_EVENT_TYPE event_id;           // идентификатор события
    uint16_t        data_len;           // длина передаваемых данных
    uint8_t*        pData;              // указатель на передаваемые данные
} EVENT_INFO;
#define EVENT_INFO_SIZE ( sizeof(EVENT_INFO) )
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#define UDP_POCKET_SIZE        (1472 / 2)
#define SHDSL_POCKET_DATA_LEN   60
#define SHDSL_POCKETS_IN_FRAME  12  // 12*(60*2) = 1440 байт // UDP_POCKET_SIZE / SHDSL_POCKET_DATA_LEN = 12,2(6)
#define SHDSL_POCKETS_QUEUE_LEN 350 // не меньше 344 !!!
//------------------------------------------------------------------------------
typedef uint16_t                TDATA_IN_SHDSL_POCKET[SHDSL_POCKET_DATA_LEN];
typedef TDATA_IN_SHDSL_POCKET*  pDATA_IN_SHDSL_POCKET;
//------------------------------------------------------------------------------
typedef union
{
    uint16_t              udp_pocket[UDP_POCKET_SIZE];
    TDATA_IN_SHDSL_POCKET shdsl_pocket[SHDSL_POCKETS_IN_FRAME];
} TAMAK_UDP_POCKET;
typedef TAMAK_UDP_POCKET* pAMAK_UDP_POCKET;
#define AMAK_UDP_POCKET_SIZE ( sizeof(TAMAK_UDP_POCKET) )
//------------------------------------------------------------------------------
typedef enum
{
    UNKNOWN 	= 0,
	STATUS 	    = 1,	//запрос состояния
	CMD 		= 2,	//команда ВАК 32
	DATA 		= 3,	//данные ВАК 32 
	AMAK 		= 4     //работа с АМАК
} ENUM_SHDSL_POCKET_ID;
//------------------------------------------------------------------------------
typedef struct
{
    uint16_t tx_pocket_count;
    uint16_t rx_pocket_count;
    uint16_t shdsl_pocket_count :9;  //lo
    uint16_t frame_count        :3;  //mid
    uint16_t pocket_id          :4;  //hi   // допустимые варианты перечислены в ENUM_SHDSL_POCKET_ID
    uint16_t data[SHDSL_POCKET_DATA_LEN];
    uint16_t crc;
} SHDSL_POCKET;
typedef SHDSL_POCKET*     pSHDSL_POCKET;
#define SHDSL_POCKET_SIZE ( sizeof(SHDSL_POCKET) )
//------------------------------------------------------------------------------
#define APP_UDP_TASK_QUEUE_LEN          3
#define APP_AMAK_PARSER_TASK_QUEUE_LEN  15
#define APP_SHDSL_TASK_QUEUE_LEN        15
#define APP_SERVICE_UART_TASK_QUEUE_LEN 3
//------------------------------------------------------------------------------
extern QueueHandle_t eventQueue_app_udp_task;
extern QueueHandle_t eventQueue_app_amak_parser_task;
extern QueueHandle_t eventQueue_app_shdsl_task;
extern QueueHandle_t eventQueue_app_service_uart_task;
//------------------------------------------------------------------------------
#ifdef	__cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif	/* AMAK_SHDSL_QUEUE_H */
//------------------------------------------------------------------------------
