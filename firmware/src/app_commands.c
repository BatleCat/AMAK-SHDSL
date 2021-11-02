/*******************************************************************************
  Sample Application

  File Name:
    app_commands.c

  Summary:
    commands for the tcp client demo app.

  Description:
    
 *******************************************************************************/


// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END
//-----------------------------------------------------------------------------
//#include "tcpip/tcpip.h"
#include "app_commands.h"
#include <stddef.h>
#include "config/default/system/command/sys_command.h"
#include "FreeRTOS.h"
#include "queue.h"

#include "system_config.h"
#include "system_definitions.h"

#include "config/default/library/tcpip/tcp.h"

#include "config/default/library/tcpip/tcpip_mac.h"
#include "config/default/library/tcpip/tcpip_manager.h"

#include "config/default/driver/ethmac/drv_ethmac.h"

#include "config/default/system/debug/sys_debug.h"
#include "config/default/system/console/sys_console.h"

#include <bsp/bsp.h>
//------------------------------------------------------------------------------
#include "AMAK_SHDSL_Queue.h"
//------------------------------------------------------------------------------
#define ENABLE_CONSOLE_MESSAGE
//------------------------------------------------------------------------------
//char APP_Hostname_Buffer[MAX_URL_SIZE];
//char APP_Port_Buffer[6];
//char APP_Message_Buffer[MAX_URL_SIZE];
static char buffer1[80];
static char buffer2[80];
static char buffer3[80];
static char buffer4[80];
static char buffer5[80];
static char buffer6[80];
static char buffer7[80];
static char buffer8[80];
static char buffer9[80];
static char buffer10[80];
static char buffer11[80];
static char buffer12[80];
bool APP_Send_Packet = false;
bool APP_Stop_Packet = false;
//-----------------------------------------------------------------------------
static void _APP_Commands_testAnswer (SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void _APP_Commands_stopUDPtest(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void _APP_Commands_SetOptions (SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void _APP_Commands_GetOptions (SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void _APP_Commands_NetUp      (SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);

static void _APP_Commands_GetNetInfo (SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void _APP_Commands_GetMACStatistics(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void _APP_Commands_GetTCPIPHeapWatermark(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void _APP_Commands_icmpEchoRequest(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
static void _APP_Commands_ARPRequest(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);
//-----------------------------------------------------------------------------
static const SYS_CMD_DESCRIPTOR    appCmdTbl[]=
{
    {"test",                        _APP_Commands_testAnswer,              ": Comand test is received"},
    {"stop",                        _APP_Commands_stopUDPtest,             ": Comand stop is received"},
    {"getudppacketoptions",         _APP_Commands_GetOptions,              ": Gets the hostname, port and message"},
    {"setudppacketoptions",         _APP_Commands_SetOptions,              ": Sets the current hostname, port, and message"},
    {"net_up",                      _APP_Commands_NetUp,                   ": Net up"},
    {"get_net_info",                _APP_Commands_GetNetInfo,              ": Get current net info"},
    {"get_mac_stat",                _APP_Commands_GetMACStatistics,        ": Get current net MAC statistics"},
    {"get_tcpip_heap_watermark",    _APP_Commands_GetTCPIPHeapWatermark,   ": Get TCPIP heap watermark"},
    {"send_icmp",                   _APP_Commands_icmpEchoRequest,         ": Send ICMP echo request"},
    {"send_arp",                    _APP_Commands_ARPRequest,              ": Send ARP request"},
};
//-----------------------------------------------------------------------------
bool APP_Commands_Init()
{
    if (!SYS_CMD_ADDGRP(appCmdTbl, sizeof(appCmdTbl)/sizeof(*appCmdTbl), "app", ": app commands"))
    {
//        SYS_DEBUG_MESSAGE(SYS_ERROR_DEBUG,"USB thread: USART init ok \r\n");
//        SYS_ERROR(SYS_ERROR_ERROR, "APP commands: Failed to create APP Commands!!!\r\n");
#ifdef ENABLE_CONSOLE_MESSAGE
        SYS_CONSOLE_MESSAGE("APP commands: Failed to create APP Commands\r\n");
#endif
        
        return false;
    }
#ifdef ENABLE_CONSOLE_MESSAGE
        SYS_CONSOLE_MESSAGE("APP commands: Init compilte\r\n");
#endif
//    strcpy(APP_Hostname_Buffer, "10.2.22.220");
//    strcpy(APP_Port_Buffer, "1500");
//    strcpy(APP_Message_Buffer, "Hello");
    APP_Send_Packet = false;
    APP_Stop_Packet = false;
    
    return true;
}
//-----------------------------------------------------------------------------
void _APP_Commands_testAnswer(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
//    SYS_DEBUG_MESSAGE(SYS_ERROR_DEBUG, "APP commands: Test \r\n");
    
//#ifdef ENABLE_CONSOLE_MESSAGE
//    SYS_CONSOLE_MESSAGE("APP commands: Test \r\n");
//#endif
    
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    
    (*pCmdIO->pCmdApi->msg)(cmdIoParam, "APP commands: Test \r\n");
    if (argc != 1)
    {
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "       Usage: test\r\n");
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "          Ex: test\r\n");
        return;
    }
//    EVENT_INFO ev;
//    ev.eventType = EVENT_TYPE_TEMP_WRITE_REQ;
//    ev.eventData = 145;
//    xQueueSend( eventQueue, (void*)&ev, portMAX_DELAY);
    APP_Send_Packet = true;
    LED1_Toggle();
    return;
}
//-----------------------------------------------------------------------------
static void _APP_Commands_stopUDPtest(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
//    SYS_DEBUG_MESSAGE(SYS_ERROR_DEBUG, "APP commands: Stop \r\n");
    
//#ifdef ENABLE_CONSOLE_MESSAGE
//    SYS_CONSOLE_MESSAGE("APP commands: Stop \r\n");
//#endif
    
    const void* cmdIoParam = pCmdIO->cmdIoParam;

    (*pCmdIO->pCmdApi->msg)(cmdIoParam, "APP commands: Stop \r\n");
    
    if (argc != 1)
    {
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "       Usage: stop\r\n");
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "          Ex: stop\r\n");
        return;
    }
    LED2_Toggle();
    APP_Stop_Packet = true;
    return;
}
//-----------------------------------------------------------------------------
static void _APP_Commands_SetOptions(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
//    SYS_DEBUG_MESSAGE(SYS_ERROR_DEBUG, "APP commands: Set options \r\n");
    
//#ifdef ENABLE_CONSOLE_MESSAGE
//    SYS_CONSOLE_MESSAGE("APP commands: Set options \r\n");
//#endif
    
    const void* cmdIoParam = pCmdIO->cmdIoParam;

    (*pCmdIO->pCmdApi->msg)(cmdIoParam, "APP commands: Set options \r\n");

    if (argc != 4)
    {
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "       Usage: setudppacketoptions <hostname> <port> <message>\r\n");
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "          Ex: setudppacketoptions 10.0.1.4 9760 Hello\r\n");
        return;
    }
    LED3_Toggle();
/*
    strcpy(APP_Hostname_Buffer, argv[1]);
    strcpy(APP_Port_Buffer, argv[2]);
    strcpy(APP_Message_Buffer, argv[3]);
*/ 
    return;
}
//char bufferArea[3][80];
//-----------------------------------------------------------------------------
void _APP_Commands_GetOptions(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
//    SYS_DEBUG_MESSAGE(SYS_ERROR_DEBUG,"APP commands: Get options \r\n");
    
//#ifdef ENABLE_CONSOLE_MESSAGE
//    SYS_CONSOLE_MESSAGE("APP commands: Get options \r\n");
//#endif
    
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    
    (*pCmdIO->pCmdApi->msg)(cmdIoParam, "APP commands: Get options \r\n");
    
    if (argc != 1)
    {
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "       Usage: getudppacketoptions\r\n");
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "          Ex: getudppacketoptions\r\n");
        return;
    }
/*
     (*pCmdIO->pCmdApi->msg)(cmdIoParam, "Current UDP Options:\r\n");
     sprintf(bufferArea[0], "\thostname: '%s'\r\n", APP_Hostname_Buffer);
     (*pCmdIO->pCmdApi->msg)(cmdIoParam, bufferArea[0]);
     sprintf(bufferArea[1], "\tport: '%s'\r\n", APP_Port_Buffer);
     (*pCmdIO->pCmdApi->msg)(cmdIoParam, bufferArea[1]);
     sprintf(bufferArea[2], "\tmessage: '%s'\r\n", APP_Message_Buffer);
     (*pCmdIO->pCmdApi->msg)(cmdIoParam, bufferArea[2]);
*/
    return;
}
//-----------------------------------------------------------------------------
static void _APP_Commands_NetUp(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
//#ifdef ENABLE_CONSOLE_MESSAGE
//    SYS_CONSOLE_MESSAGE("APP commands: net_up \r\n");
//#endif
    
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    
    (*pCmdIO->pCmdApi->msg)(cmdIoParam, "---------------------------------\r\n");
    (*pCmdIO->pCmdApi->msg)(cmdIoParam, "APP commands: net_up \r\n");
    if (argc != 1)
    {
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "       Usage: net_up \r\n");
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "          Ex: net_up \r\n");
        return;
    }

    TCPIP_NET_HANDLE netH = TCPIP_STACK_NetHandleGet("PIC32INT");

    const TCPIP_NETWORK_CONFIG __attribute__((unused))  TCPIP_HOSTS_CONFIGURATION[] =
    {
        /*** Network Configuration Index 0 ***/
        {
            .interface = TCPIP_NETWORK_DEFAULT_INTERFACE_NAME_IDX0,
            .hostName = TCPIP_NETWORK_DEFAULT_HOST_NAME_IDX0,
            .macAddr = TCPIP_NETWORK_DEFAULT_MAC_ADDR_IDX0,
            .ipAddr = TCPIP_NETWORK_DEFAULT_IP_ADDRESS_IDX0,
            .ipMask = TCPIP_NETWORK_DEFAULT_IP_MASK_IDX0,
            .gateway = TCPIP_NETWORK_DEFAULT_GATEWAY_IDX0,
            .priDNS = TCPIP_NETWORK_DEFAULT_DNS_IDX0,
            .secondDNS = TCPIP_NETWORK_DEFAULT_SECOND_DNS_IDX0,
            .powerMode = TCPIP_NETWORK_DEFAULT_POWER_MODE_IDX0,
            .startFlags = TCPIP_NETWORK_DEFAULT_INTERFACE_FLAGS_IDX0,
            .pMacObject = &TCPIP_NETWORK_DEFAULT_MAC_DRIVER_IDX0,
        },
    };
    bool is_net_up = TCPIP_STACK_NetUp(netH, TCPIP_HOSTS_CONFIGURATION);
    
    if (is_net_up)
    {
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "              Net up: Ok \r\n");
    }
    else
    {
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "              Net up: Error! \r\n");
    }

//    #ifdef ENABLE_CONSOLE_MESSAGE
//        SYS_CONSOLE_MESSAGE ("---------------------------------\r\n");
//        if (is_net_up)
//        {
//            SYS_CONSOLE_MESSAGE("    Net up: Ok \r\n");
//        }
//        else
//        {
//            SYS_CONSOLE_MESSAGE("    Net up: Error! \r\n");
//        }
//    #endif

    return;
}
//-----------------------------------------------------------------------------
static void _APP_Commands_GetNetInfo(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
//#ifdef ENABLE_CONSOLE_MESSAGE
//    SYS_CONSOLE_MESSAGE("APP commands: get_net_info \r\n");
//#endif
    
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    
    (*pCmdIO->pCmdApi->msg)(cmdIoParam, "---------------------------------\r\n");
    (*pCmdIO->pCmdApi->msg)(cmdIoParam, "APP commands: get_net_info \r\n");

    if (argc != 1)
    {
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "       Usage: get_net_info \r\n");
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "          Ex: get_net_info \r\n");
        return;
    }

//    TCPIP_NET_HANDLE netH = TCPIP_STACK_IndexToNet(0);
    TCPIP_NET_HANDLE netH = TCPIP_STACK_NetHandleGet("PIC32INT");
    IPV4_ADDR        ipAddr;
    ipAddr.Val = TCPIP_STACK_NetAddress(netH);
    const TCPIP_MAC_ADDR* pMACAddr = (const TCPIP_MAC_ADDR*)TCPIP_STACK_NetAddressMac(netH);
                
    char str_ip_adr[20];
    char str_mac_adr[25];

    TCPIP_Helper_IPAddressToString( &ipAddr, str_ip_adr, sizeof(str_ip_adr) );
    TCPIP_Helper_MACAddressToString( pMACAddr, str_mac_adr, sizeof(str_mac_adr) );
//                   "APP commands: get_net_info \r\n"
    sprintf(buffer1, "    NetName:     %s \r\n", TCPIP_STACK_NetNameGet(netH) );
    sprintf(buffer2, "    IP  address: %s \r\n", str_ip_adr);
    sprintf(buffer3, "    MAC address: %s \r\n", str_mac_adr);
    (*pCmdIO->pCmdApi->msg)(cmdIoParam, buffer1);
    (*pCmdIO->pCmdApi->msg)(cmdIoParam, buffer2);
    (*pCmdIO->pCmdApi->msg)(cmdIoParam, buffer3);
    (*pCmdIO->pCmdApi->msg)(cmdIoParam, "---------------------------------\r\n");
    
//    #ifdef ENABLE_CONSOLE_MESSAGE
//        SYS_CONSOLE_MESSAGE ("---------------------------------\r\n");
//        SYS_CONSOLE_PRINT   ("    NetName:     %s \r\n", 
//                                TCPIP_STACK_NetNameGet(netH) );
//        SYS_CONSOLE_PRINT   ("    IP  address: %s \r\n", str_ip_adr);
//        SYS_CONSOLE_PRINT   ("    MAC address: %s \r\n", str_mac_adr);
//        SYS_CONSOLE_MESSAGE ("---------------------------------\r\n");
//    #endif

    return;
}
//-----------------------------------------------------------------------------
static void _APP_Commands_GetMACStatistics(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
//#ifdef ENABLE_CONSOLE_MESSAGE
//    SYS_CONSOLE_MESSAGE("APP commands: get_mac_stat \r\n");
//#endif
    
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    
    (*pCmdIO->pCmdApi->msg)(cmdIoParam, "---------------------------------\r\n");
    (*pCmdIO->pCmdApi->msg)(cmdIoParam, "APP commands: get_mac_stat \r\n");
    
    if (argc != 1)
    {
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "       Usage: get_mac_stat \r\n");
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "          Ex: get_mac_stat \r\n");
        return;
    }
    
    TCPIP_MAC_RX_STATISTICS rxStatistics;
    TCPIP_MAC_TX_STATISTICS txStatistics;
    TCPIP_NET_HANDLE netH = TCPIP_STACK_NetHandleGet("PIC32INT");
//    TCPIP_NET_HANDLE netH = TCPIP_STACK_IndexToNet(0);
    
    char str_mac_adr[25];
    const TCPIP_MAC_ADDR* pMACAddr = (const TCPIP_MAC_ADDR*)TCPIP_STACK_NetAddressMac(netH);

    TCPIP_Helper_MACAddressToString( pMACAddr, str_mac_adr, sizeof(str_mac_adr) );
    sprintf(buffer1, "    MAC address: %s \r\n", str_mac_adr);
    (*pCmdIO->pCmdApi->msg)(cmdIoParam, buffer1);
    
    if ( TCPIP_STACK_NetMACStatisticsGet(netH, &rxStatistics, &txStatistics) )
    {
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "    Net MAC statistics: \r\n");
        sprintf(buffer2, "    Rx good packets = %d \r\n", rxStatistics.nRxOkPackets);
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, buffer2);
        sprintf(buffer3, "    Rx error packets = %d \r\n", rxStatistics.nRxErrorPackets);
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, buffer3);
        sprintf(buffer4, "    Tx good packets = %d \r\n", txStatistics.nTxOkPackets);
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, buffer4);
        sprintf(buffer5, "    Tx error packets = %d \r\n", txStatistics.nTxErrorPackets);
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, buffer5);

//        #ifdef ENABLE_CONSOLE_MESSAGE
//            SYS_CONSOLE_MESSAGE("---------------------------------------\r\n");
//            SYS_CONSOLE_MESSAGE("   Net MAC statistics: \r\n");
//            SYS_CONSOLE_PRINT  ("               Rx good packets = %d \r\n", rxStatistics.nRxOkPackets);
//            SYS_CONSOLE_PRINT  ("               Rx error packets = %d \r\n", rxStatistics.nRxErrorPackets);
//            SYS_CONSOLE_PRINT  ("               Tx good packets = %d \r\n", txStatistics.nTxOkPackets);
//            SYS_CONSOLE_PRINT  ("               Tx error packets = %d \r\n", txStatistics.nTxErrorPackets);
//            SYS_CONSOLE_MESSAGE("---------------------------------------\r\n");
//        #endif
    }

    const TCPIP_MAC_OBJECT* pEthMacObject = TCPIP_STACK_MACObjectGet(netH);
    if(0 != pEthMacObject)
    {    // valid MAC object pointer
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "    Get MAC object: Ok \r\n");
        sprintf(buffer6, "    MAC name: %s \r\n", pEthMacObject->macName);
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, buffer6);

        //  DRV_ETHMAC_CLIENTS_NUMBER has to be > 1
        DRV_HANDLE hMac = (*pEthMacObject->TCPIP_MAC_Open)(pEthMacObject->macId, DRV_IO_INTENT_READWRITE);
        if(hMac != DRV_HANDLE_INVALID)
        {   // can use the MAC handle to access a MAC function
            TCPIP_MAC_RES macRes;
            TCPIP_MAC_PARAMETERS macParams;
            
            (*pCmdIO->pCmdApi->msg)(cmdIoParam, "    TCPIP_MAC_Open: Success \r\n");

            macRes = (*pEthMacObject->TCPIP_MAC_ParametersGet)(hMac, &macParams);
            TCPIP_Helper_MACAddressToString( &(macParams.ifPhyAddress), str_mac_adr, sizeof(str_mac_adr) );
            sprintf(buffer7, "    MAC address: %s \r\n", str_mac_adr);
            (*pCmdIO->pCmdApi->msg)(cmdIoParam, buffer7);
            
            macRes = (*pEthMacObject->TCPIP_MAC_StatisticsGet)(hMac, &rxStatistics, &txStatistics);

            sprintf(buffer8, "    TCPIP_MAC_StatisticsGet result = %d \r\n", macRes);
            (*pCmdIO->pCmdApi->msg)(cmdIoParam, buffer8);
            
            (*pCmdIO->pCmdApi->msg)(cmdIoParam, "    Net MAC statistics: \r\n");
            sprintf(buffer9, "    Rx good packets = %d \r\n", rxStatistics.nRxOkPackets);
            (*pCmdIO->pCmdApi->msg)(cmdIoParam, buffer9);
            sprintf(buffer10, "    Rx error packets = %d \r\n", rxStatistics.nRxErrorPackets);
            (*pCmdIO->pCmdApi->msg)(cmdIoParam, buffer10);
            sprintf(buffer11, "    Tx good packets = %d \r\n", txStatistics.nTxOkPackets);
            (*pCmdIO->pCmdApi->msg)(cmdIoParam, buffer11);
            sprintf(buffer12, "    Tx error packets = %d \r\n", txStatistics.nTxErrorPackets);
            (*pCmdIO->pCmdApi->msg)(cmdIoParam, buffer12);
        }
        else
        {
            (*pCmdIO->pCmdApi->msg)(cmdIoParam, "    TCPIP_MAC_Open: Error! \r\n");
        }
    }
    else
    {
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "    Get MAC object: Error! \r\n");
    }

    switch (DRV_MIIM_Status(sysObj.drvMiim))
    {
        case (SYS_STATUS)SYS_STATUS_READY:
                (*pCmdIO->pCmdApi->msg)(cmdIoParam, "    Driver MIIM status: ready \r\n");
            break;
        case (SYS_STATUS)SYS_STATUS_BUSY:
                (*pCmdIO->pCmdApi->msg)(cmdIoParam, "    Driver MIIM status: busy \r\n");
            break;
        case (SYS_STATUS)SYS_STATUS_ERROR:
                (*pCmdIO->pCmdApi->msg)(cmdIoParam, "    Driver MIIM status: error! \r\n");
            break;
        default:
            break;
    }
    (*pCmdIO->pCmdApi->msg)(cmdIoParam, "---------------------------------------\r\n");
    return;
}
//-----------------------------------------------------------------------------
static void _APP_Commands_GetTCPIPHeapWatermark(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
    const void* cmdIoParam = pCmdIO->cmdIoParam;

    (*pCmdIO->pCmdApi->msg)(cmdIoParam, "-------------------------------------- \r\n");
    (*pCmdIO->pCmdApi->msg)(cmdIoParam, "APP commands: get_tcpip_heap_watermark \r\n");
    
    if (argc != 1)
    {
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "       Usage: get_tcpip_heap_watermark \r\n");
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "       Ex: get_tcpip_heap_watermark \r\n");
        return;
    }
    
//    #ifdef ENABLE_CONSOLE_MESSAGE
//        SYS_CONSOLE_MESSAGE("APP commands: get_tcpip_heap_watermark \r\n");

    TCPIP_STACK_HEAP_TYPE   heapType = TCPIP_STACK_HEAP_TYPE_INTERNAL_HEAP;
    TCPIP_STACK_HEAP_HANDLE heapH    = TCPIP_STACK_HeapHandleGet( heapType, 0 );
    size_t heap_watermark = TCPIP_STACK_HEAP_HighWatermark( heapH );

    sprintf(buffer1, "    heap high watermark: %d \r\n", heap_watermark);
    (*pCmdIO->pCmdApi->msg)(cmdIoParam, buffer1);
     
//        SYS_CONSOLE_PRINT   ("               heap high watermark: %d \r\n", heap_watermark);
//    #endif
}
//-----------------------------------------------------------------------------
static void _APP_Commands_icmpEchoRequest(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
    //--------------------------------------------------------------------------
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    
    (*pCmdIO->pCmdApi->msg)(cmdIoParam, "-------------------------------------- \r\n");
    (*pCmdIO->pCmdApi->msg)(cmdIoParam, "APP commands: send_icmp \r\n");

    if (argc != 2)
    {
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "       Usage: send_icmp <ip address> \r\n");
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "          Ex: send_icmp 192.168.1.1 \r\n");
        return;
    }
    //--------------------------------------------------------------------------
    IPV4_ADDR remoteAddress = { .v = {10, 2, 22, 220} };
    uint16_t  mySequenceNumber = 1;
    uint16_t  myId = 0x1234;

    if ( true == TCPIP_Helper_StringToIPAddress(argv[1], &remoteAddress) )
    {
        if ( TCPIP_ICMP_EchoRequestSend(0, &remoteAddress, mySequenceNumber, myId) == ICMP_ECHO_OK )
        {
            (*pCmdIO->pCmdApi->msg)(cmdIoParam, "    ICMP echo request send: Ok \r\n");
        }
        else
        {
            (*pCmdIO->pCmdApi->msg)(cmdIoParam, "    ICMP echo request send: Error! \r\n");
        }
    }
    else
    {
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "    Error!!! Bad IP address! \r\n");
    }

//    #ifdef ENABLE_CONSOLE_MESSAGE
//        IPV4_ADDR remoteAddress = { .v = {10, 2, 22, 220} };
//        uint16_t mySequenceNumber = 1;
//        uint16_t myId = 0x1234;
//        
//        bool TCPIP_Helper_StringToIPAddress(const char* str, IPV4_ADDR* IPAddress);
//        
//        if ( TCPIP_ICMP_EchoRequestSend(0, &remoteAddress, mySequenceNumber, myId) == ICMP_ECHO_OK )
//        {
//            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: ICMP echo request send: Ok \r\n");
//        }
//        else
//        {
//            SYS_CONSOLE_MESSAGE(" APP_UDP_TASK: ICMP echo request send: Error! \r\n");
//        }
//    #endif
}
//-----------------------------------------------------------------------------
static void _APP_Commands_ARPRequest(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv)
{
    //--------------------------------------------------------------------------
    const void* cmdIoParam = pCmdIO->cmdIoParam;
    
    (*pCmdIO->pCmdApi->msg)(cmdIoParam, "-------------------------------------- \r\n");
    (*pCmdIO->pCmdApi->msg)(cmdIoParam, "APP commands: send_arp \r\n");

    if (argc != 2)
    {
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "       Usage: send_arp <ip address> \r\n");
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "          Ex: send_arp 192.168.1.1 \r\n");
        return;
    }
    //--------------------------------------------------------------------------
    IPV4_ADDR remoteAddress = { .v = {10, 2, 22, 220} };

    if ( true == TCPIP_Helper_StringToIPAddress(argv[1], &remoteAddress) )
    {
        //------------------------------------------------------------------
//        TCPIP_NET_HANDLE netH = TCPIP_STACK_IndexToNet(0);
        TCPIP_NET_HANDLE netH = TCPIP_STACK_NetHandleGet("PIC32INT");
        TCPIP_ARP_RESULT arp_res;
        
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "    ARP resolve: ");
        //----------------------------------------------------------------------
        arp_res = TCPIP_ARP_Resolve(netH, &(remoteAddress) );

        switch (arp_res)
        {
            case ARP_RES_OK:
                (*pCmdIO->pCmdApi->msg)(cmdIoParam, "operation succeeded \r\n");
                break;
            case ARP_RES_ENTRY_NEW:
                (*pCmdIO->pCmdApi->msg)(cmdIoParam, "operation succeeded and a new entry was added \r\n");
                break;
            case ARP_RES_ENTRY_SOLVED:
                (*pCmdIO->pCmdApi->msg)(cmdIoParam, "the required entry is already solved \r\n");
                break;
            case ARP_RES_ENTRY_QUEUED:
                (*pCmdIO->pCmdApi->msg)(cmdIoParam, "the required entry was already queued \r\n");
                break;
            case ARP_RES_ENTRY_EXIST:
                (*pCmdIO->pCmdApi->msg)(cmdIoParam, "the required entry was already cached \r\n");
                break;
            case ARP_RES_PERM_QUOTA_EXCEED:
                (*pCmdIO->pCmdApi->msg)(cmdIoParam, "info: the quota of permanent entries was exceeded \r\n");
                break;
            case ARP_RES_PROBE_OK:
                (*pCmdIO->pCmdApi->msg)(cmdIoParam, "requested probe sent \r\n");
                break;
            case ARP_RES_NO_ENTRY:
                (*pCmdIO->pCmdApi->msg)(cmdIoParam, "no such entry exists \r\n");
                break;
            case ARP_RES_CACHE_FULL:
                (*pCmdIO->pCmdApi->msg)(cmdIoParam, "the cache is full and no entry could be removed to make room \r\n");
                break;
            case ARP_RES_TX_FAILED:
                (*pCmdIO->pCmdApi->msg)(cmdIoParam, "failed to transmit an ARP message \r\n");
                break;
            case ARP_RES_BAD_INDEX:
                (*pCmdIO->pCmdApi->msg)(cmdIoParam, "bad query index \r\n");
                break;
            case ARP_RES_BAD_ADDRESS:
                (*pCmdIO->pCmdApi->msg)(cmdIoParam, "bad IP address specified \r\n");
                break;
            case ARP_RES_NO_INTERFACE:
                (*pCmdIO->pCmdApi->msg)(cmdIoParam, "no such interface exists \r\n");
                break;
            case ARP_RES_BAD_TYPE:
                (*pCmdIO->pCmdApi->msg)(cmdIoParam, "no such type is valid/exists \r\n");
                break;
            case ARP_RES_CONFIGURE_ERR:
                (*pCmdIO->pCmdApi->msg)(cmdIoParam, "interface is configuring now, no ARP probes \r\n");
                break;
            case ARP_RES_PROBE_FAILED:
                (*pCmdIO->pCmdApi->msg)(cmdIoParam, "requested probe failed \r\n");
                break;
            default:
                (*pCmdIO->pCmdApi->msg)(cmdIoParam, "unknown error!!! \r\n");
                break;
        }
        //----------------------------------------------------------------------
        TCPIP_MAC_ADDR MACAddr;
        int dog_time = 100;
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "    ARP resolve MAC address: ");
        while ( true != TCPIP_ARP_IsResolved(netH, &(remoteAddress), &MACAddr) )
        {
            (*pCmdIO->pCmdApi->msg)(cmdIoParam, ".");
            dog_time--;
            if (0 == dog_time)
            {
                (*pCmdIO->pCmdApi->msg)(cmdIoParam, "\r\n    ARP resolve MAC address: Time out! \r\n ");
                break;
            }
            taskYIELD();
        }
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, " \r\n");
        if ( true == TCPIP_ARP_IsResolved(netH, &(remoteAddress), &MACAddr) )
        {
            (*pCmdIO->pCmdApi->msg)(cmdIoParam, "    ARP resolve MAC address: Ok  \r\n" );
            sprintf(buffer1, "    MAC address = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X \r\n", 
                    MACAddr.v[0], MACAddr.v[1], MACAddr.v[2], MACAddr.v[3], MACAddr.v[4], MACAddr.v[5]);
            (*pCmdIO->pCmdApi->msg)(cmdIoParam, buffer1);
        }
        else
        {
            (*pCmdIO->pCmdApi->msg)(cmdIoParam, "    ARP resolve MAC address: NOT success! \r\n");
        }
        //----------------------------------------------------------------------
    }
    else
    {
        (*pCmdIO->pCmdApi->msg)(cmdIoParam, "    Error!!! Bad IP address! \r\n");
    }
}
//-----------------------------------------------------------------------------
#undef ENABLE_CONSOLE_MESSAGE
//------------------------------------------------------------------------------
