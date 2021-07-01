/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.
    TNG-Group.

  File Name:
    app_amak_parser_task.h

  Summary:
    This header file provides prototypes and definitions for the application.
 *  ?????? UDP-??????? ? SHDSL-??????? ??? ??????? ????-100

  Description:
    This header file provides function prototypes and data type definitions for
    the application.
 *  ?????? UDP-??????? ????-100 ? ???????? ????? ?? SHDSL-?????? ??? ???????? ??
 *  ??????????? ?????? ??? ?????? SHDSL-??????. ?????? UDP-?????? ?? SHDSL-????-
 *  ???.  UDP-?????? ??????????? ? ???????????? ? ?????? App_UDP_Task().  SHDSL-
 *  ?????? ???????????? ? ??????????? ? ?????? App_SHDSL_Task().

*******************************************************************************/

#ifndef _APP_AMAK_PARSER_TASK_H
#define _APP_AMAK_PARSER_TASK_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
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

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/
typedef enum
{
    /* Application's state machine's initial state. */
    APP_AMAK_PARSER_TASK_STATE_INIT=0,
    APP_AMAK_PARSER_TASK_STATE_SERVICE_TASKS,
    /* TODO: Define states used by the application state machine. */

} APP_AMAK_PARSER_TASK_STATES;


// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct
{
    /* The application's current state */
    APP_AMAK_PARSER_TASK_STATES state;

    /* TODO: Define any additional data used by the application. */

} APP_AMAK_PARSER_TASK_DATA;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/* These routines are called by drivers when certain events occur.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_AMAK_PARSER_TASK_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_AMAK_PARSER_TASK_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_AMAK_PARSER_TASK_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_AMAK_PARSER_TASK_Initialize ( void );


/*******************************************************************************
  Function:
    void APP_AMAK_PARSER_TASK_Tasks ( void )

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
    APP_AMAK_PARSER_TASK_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_AMAK_PARSER_TASK_Tasks( void );

//------------------------------------------------------------------------------
//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END
//------------------------------------------------------------------------------
#endif /* _APP_AMAK_PARSER_TASK_H */
/*******************************************************************************
 End of File
 */
//------------------------------------------------------------------------------
