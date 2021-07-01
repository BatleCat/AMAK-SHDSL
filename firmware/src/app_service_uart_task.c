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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app_service_uart_task.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_SERVICE_UART_TASK_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_SERVICE_UART_TASK_DATA app_service_uart_taskData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_SERVICE_UART_TASK_Initialize ( void )

  Remarks:
    See prototype in app_service_uart_task.h.
 */

void APP_SERVICE_UART_TASK_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_service_uart_taskData.state = APP_SERVICE_UART_TASK_STATE_INIT;



    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APP_SERVICE_UART_TASK_Tasks ( void )

  Remarks:
    See prototype in app_service_uart_task.h.
 */

void APP_SERVICE_UART_TASK_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( app_service_uart_taskData.state )
    {
        /* Application's initial state. */
        case APP_SERVICE_UART_TASK_STATE_INIT:
        {
            bool appInitialized = true;


            if (appInitialized)
            {

                app_service_uart_taskData.state = APP_SERVICE_UART_TASK_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_SERVICE_UART_TASK_STATE_SERVICE_TASKS:
        {

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


/*******************************************************************************
 End of File
 */
