/*------------------------------------------------------------------------------
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.
 * TNG-Group

  File Name:
    app_udp_task.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_UDP_TASK_Initialize" and "APP_UDP_TASK_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_UDP_TASK_STATES" definition).  Both
    are defined here for convenience.
------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
#ifndef _APP_UDP_TASK_H
#define _APP_UDP_TASK_H
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Section: Included Files
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "config/default/library/tcpip/tcpip.h"
#include "config/default/library/tcpip/tcp.h"
#include "config/default/library/tcpip/udp.h"
#include "configuration.h"
//------------------------------------------------------------------------------
#include "AMAK_SHDSL_Queue.h"
//------------------------------------------------------------------------------
// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Section: Type Definitions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

/*------------------------------------------------------------------------------
 Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/
//------------------------------------------------------------------------------
typedef enum
{
    /* Application's state machine's initial state. */
    APP_UDP_TASK_STATE_INIT=0,
//    APP_UDP_TASK_STATE_SERVICE_TASKS,
    /* TODO: Define states used by the application state machine. */
    APP_UDP_TASK_STATE_Start,
    APP_UDP_TASK_STATE_WAIT_FOR_IP,
    APP_UDP_TASK_STATE_WAIT_SERVER_OPEN,
    APP_UDP_TASK_STATE_WAIT_CLIENT_OPEN,
    APP_UDP_TASK_STATE_WAIT_FOR_CONNECTION,
    APP_UDP_TASK_STATE_Rx,
    APP_UDP_TASK_STATE_Tx,
    APP_UDP_TASK_STATE_CLOSING_CONNECTION,
    APP_UDP_TASK_STATE_Error,

} APP_UDP_TASK_STATES;

/*------------------------------------------------------------------------------
 Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/
//------------------------------------------------------------------------------
typedef enum
{
    /* Application's state machine's initial state. */
    APP_UDP_TASK_ERROR_NO = 0,
//    APP_UDP_TASK_STATE_SERVICE_TASKS,
    /* TODO: Define states used by the application state machine. */
    APP_UDP_TASK_ERROR_Create_Queue,
    APP_UDP_TASK_ERROR_STATE_Start,
    APP_UDP_TASK_ERROR_STATE_WAIT_FOR_IP,
    APP_UDP_TASK_ERROR_STATE_WAIT_SERVER_OPEN,
    APP_UDP_TASK_ERROR_STATE_WAIT_CLIENT_OPEN,
    APP_UDP_TASK_ERROR_STATE_WAIT_FOR_CONNECTION,
    APP_UDP_TASK_ERROR_STATE_Rx,
    APP_UDP_TASK_ERROR_STATE_Tx,
    APP_UDP_TASK_ERROR_STATE_CLOSING_CONNECTION,

} APP_UDP_TASK_ERRORS;
/*------------------------------------------------------------------------------
 Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */
//------------------------------------------------------------------------------
typedef struct
{
    /* The application's current state */
    APP_UDP_TASK_STATES state;

    /* TODO: Define any additional data used by the application. */
    APP_UDP_TASK_ERRORS error;
    EVENT_INFO          event_info;
    UDP_SOCKET          udp_rx_socket;
    UDP_SOCKET          udp_tx_socket;
    TCPIP_NET_HANDLE    netH;
    
    IP_MULTI_ADDRESS    dest_adr;
    UDP_PORT            dest_port;

    IP_MULTI_ADDRESS    local_adr;
    UDP_PORT            local_port;
    
} APP_UDP_TASK_DATA;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Section: Application Callback Routines
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/* These routines are called by drivers when certain events occur.
*/

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Section: Application Initialization and State Machine Functions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

/*------------------------------------------------------------------------------
  Function:
    void APP_UDP_TASK_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_UDP_TASK_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_UDP_TASK_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/
//------------------------------------------------------------------------------
void APP_UDP_TASK_Initialize ( void );

/*------------------------------------------------------------------------------
  Function:
    void APP_UDP_TASK_Tasks ( void )

  Summary:
    MPLAB Harmony Demo application tasks function

  Description:
    This routine is the Harmony Demo application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_UDP_TASK_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */
//------------------------------------------------------------------------------
void APP_UDP_TASK_Tasks( void );

//------------------------------------------------------------------------------
//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END
//------------------------------------------------------------------------------

#endif /* _APP_UDP_TASK_H */
//------------------------------------------------------------------------------
/*******************************************************************************
 End of File
 */
//------------------------------------------------------------------------------
