/*
 * Copyright (c) 2015-2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== empty.c ========
 */

/* For usleep() */
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>


/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/Timer.h>
#include <ti/drivers/net/wifi/simplelink.h>
#include <ti/drivers/Power.h>

/* Board Header file */
#include "Board.h"
#include "semaphore.h"
#include "pthread.h"
#include "uart_term.h"
#include "platform.h"

/* #define section =========================================================================================*/

#define SL_SOCKET_ERROR             ("Socket error, please refer \"SOCKET ERRORS CODES\" section in errors.h")
#define BSD_SOCKET_ERROR            ("BSD Socket error, please refer \"BSD SOCKET ERRORS CODES\" section in errors.h")
#define WLAN_EVENT_TOUT             (6000)
#define TCP_PROTOCOL_FLAGS          0
#define TIMEOUT_SEM                 (-1)
#define TASK_STACK_SIZE             (2048)
#define SPAWN_TASK_PRIORITY         (9)
#define MAX_BUF_SIZE                (1400)
#define MAX_TEXT_PAD_SIZE           (256)
#define WLAN_SCAN_COUNT             (20)
#define MAX_SERVICE_NAME_LENGTH     (63)
#define CHANNEL_MASK_ALL            (0x1FFF)
#define RSSI_TH_MAX                 (-95)
#define CMD_BUFFER_LEN              (256)
#define DEVICE_ERROR                ("Device error, please refer \"DEVICE ERRORS CODES\" section in errors.h")
#define NETAPP_ERROR                ("Netapp error, please refer \"NETAPP ERRORS CODES\" section in errors.h")
#define SL_STOP_TIMEOUT             (200)
#define SHOW_WARNING(ret, errortype)        UART_PRINT("\n\r[line:%d, error code:%d] %s\n\r", __LINE__, ret, errortype);
#define IS_CONNECTED(status_variable)       \
                GET_STATUS_BIT(status_variable, STATUS_BIT_CONNECTION)
#define WLAN_ERROR              ("WLAN error, please refer \"WLAN ERRORS CODES\" section in errors.h")
#define ASSERT_ON_ERROR(ret, errortype)\
        {\
            if(ret < 0)\
            {\
                SHOW_WARNING(ret, errortype);\
                UART_PRINT("Assert on error error\n\r");\
                return -1;\
            }\
        }
#define IS_PING_RUNNING(status_variable)    \
                GET_STATUS_BIT(status_variable, STATUS_BIT_PING_STARTED)

#define LAPTOP_IP "192.168.137.65"
#define PORTNUM 1111
#define RGB_ON 1
#define RGB_OFF 0
#define THRESHOLD 450000
#define WIFI_SSID "PC"
#define WIFI_KEY "12345678"
#define BUFF_SIZE 100
#define ParkingLot "EastRemote"
//#define ParkingLot "NorthRemote"
#define ParkingSpace "2"
#define OPEN_SPACE "open"
#define BUSY_SPACE "busy"
#define CLOSED_SPACE "closed"
#define BROKEN_SPACE "broken"
#define ILLEGAL_SPACE "illegal"
#define CHANGEDBIT_0 "0"
#define CHANGEDBIT_1 "1"
#define ACK_SPACE "ACK"
#define OK_SPACE "ok"
#define NOT_ACTIVE_DURATION_MSEC 3000 // 3 sec

#define RED 1
#define GREEN 2
#define BLUE 3
#define YELLOW 4
#define PURPLE 5
#define MARINE 6
#define ALL 7
#define NONE 8

#define SET_STATUS_BIT(status_variable, bit) status_variable |= (1<<(bit))
#define CLR_STATUS_BIT(status_variable, bit) status_variable &= ~(1<<(bit))
#define GET_STATUS_BIT(status_variable, bit)    \
                                (0 != (status_variable & (1<<(bit))))

char cmdPromptStr[]             = "user:";
char lineBreak[]                = "\n\r";

typedef enum{

    STATUS_BIT_NWP_INIT = 0,          /* This bit is set: Network Processor is powered up */

    STATUS_BIT_CONNECTION,            /* This bit is set: the device is connected
                                         to the AP or client is connected to device (AP) */
    STATUS_BIT_IP_LEASED,             /* This bit is set: the device has leased IP to
                                         any connected client */
    STATUS_BIT_IP_ACQUIRED,           /* This bit is set: the device has acquired an IP */

    STATUS_BIT_P2P_DEV_FOUND,         /* If this bit is set: the device (P2P mode)
                                         found any p2p-device in scan */
    STATUS_BIT_P2P_REQ_RECEIVED,      /* If this bit is set: the device (P2P mode)
                                         found any p2p-negotiation request */
    STATUS_BIT_CONNECTION_FAILED,     /* If this bit is set: the device(P2P mode)
                                         connection to client(or reverse way) is failed */
    STATUS_BIT_PING_STARTED,          /* This bit is set: device is undergoing ping operation */

    STATUS_BIT_SCAN_RUNNING,          /* This bit is set: Scan is running is background */

    STATUS_BIT_IPV6_ACQUIRED,         /* If this bit is set: the device has acquired
                                         an IPv6 address */
    STATUS_BIT_IPV6_GLOBAL_ACQUIRED,  /* If this bit is set: the device has acquired
                                         an IPv6 address */
    STATUS_BIT_IPV6_LOCAL_ACQUIRED,   /* If this bit is set: the device has acquired
                                        an IPv6 address */
    STATUS_BIT_AUTHENTICATION_FAILED, /* If this bit is set: Authentication with ENT AP failed. */

    STATUS_BIT_RESET_REQUIRED,

    STATUS_BIT_TX_STARED

}e_StatusBits;

typedef union{

    SlSockAddrIn6_t     in6;   /* Socket info for Ipv6 */
    SlSockAddrIn_t      in4;   /* Socket info for Ipv4 */
}sockAddr_t;

typedef union
{
    uint8_t                    nwData[MAX_BUF_SIZE];
    int8_t                     textPad[MAX_TEXT_PAD_SIZE];
    SlWlanNetworkEntry_t       netEntries[WLAN_SCAN_COUNT];
}gDataBuffer_t;

typedef struct cmdAction
{
    char        *cmd;
    int32_t    (*callback)(void *);
    int32_t    (*printusagecallback)(void *);
}cmdAction_t;

typedef struct p2p_ControlBlock_t
{
    uint8_t            p2pPeerDeviceName[32];
    uint8_t            p2pDeviceName[32];
    sem_t              DeviceFound;
    sem_t              RcvNegReq;
    sem_t              RcvConReq;
    SlWlanSecParams_t P2PsecParams;
}p2p_CB;

typedef struct connectionControlBlock_t
{
    sem_t     connectEventSyncObj;
    sem_t     ip4acquireEventSyncObj;
    sem_t     ip6acquireEventSyncObj;
    sem_t     eventCompletedSyncObj;
    uint32_t GatewayIP;
    uint8_t  ConnectionSSID[SL_WLAN_SSID_MAX_LENGTH +1];
    uint8_t  ConnectionBSSID[SL_WLAN_BSSID_LENGTH];
    uint32_t DestinationIp;
    uint32_t IpAddr;
    uint32_t StaIp;
    uint32_t Ipv6Addr[4];
}connection_CB;

typedef struct parkingControlBlock_t
{
    /* Status Variables */
    uint32_t        Status;             /* This bit-wise status variable shows the state of the NWP */
    uint32_t        Role;               /* This field keeps the device's role (STA, P2P or AP) */
    uint32_t        Exit;               /* This flag lets the application to exit */
    uint32_t        PingAttempts;       /* Sets the number of Ping attempts to send */
    /* Data & Network entry Union */
    gDataBuffer_t     gDataBuffer;
    /* P2P mode CB */
    p2p_CB            P2P_CB;
    /* STA/AP mode CB */
    connection_CB    CON_CB;
    /* Cmd Prompt buffer */
    uint8_t         CmdBuffer[CMD_BUFFER_LEN];
    /* WoWLAN semaphore */
    sem_t            WowlanSleepSem;
}parkingControlBlock;

typedef struct ConnectCmd
{
    uint8_t                 *ssid;              /* Ap's SSID */
    uint8_t                 *ip;                /* Static IP address (for static configuration) */
    uint8_t                 *gw;                /* Default gateway IP address (for static configuration) */
    uint8_t                 *dns;               /* Dns IP address (for static configuration) */
    uint8_t                 *entUserName;       /* Enterprise user name */
    SlDateTime_t            dateTime;           /* Device Date and Time - IMPORTANT: Date and time should match the certificate expiration date */
    SlWlanSecParams_t       secParams;          /* Security parameters - Security Type and Password */
    SlWlanSecParamsExt_t    secParamsEnt;       /* Enterprise parameters - Security Type and Password */
}ConnectCmd_t;

typedef union
{
    uint32_t    ipv4;       /* Ipv4 Address */
    uint8_t     ipv6[16];   /* Ipv6 Address */
}ip_t;

typedef struct SendCmd
{
    int32_t          numberOfPackets;       /* Number of packets to send */
    uint8_t          *ip;                   /* IP address */
    int16_t          portNumber;            /* Server's port address*/
    uint8_t          udpFlag;               /* Decides the type of transport protocol */
    uint8_t          server;                /* Send as server or client flag */
    uint8_t          nb;                    /* Blocking or non-blocking on socket */
    uint8_t          ipv6;                  /* IPV4 or IPv6 flag. By default IPV4 enable */
    ip_t             ipAddr;                /* IP in binary format */
}SendCmd_t;

/* Declaring functions ============================================== */
/* Callback used for toggling the LED. */
void timerCallback(Timer_Handle myHandle);
void distCallback(Timer_Handle myHandle);
void RGBTest(Timer_Handle myHandle);
void inputTest(Timer_Handle myHandle);
void BT_Test(Timer_Handle myHandle);
void StopWatch_Test(Timer_Handle myHandle);
void TRIG_PULSE(Timer_Handle myHandle);

/* Other functions for sensor */
void setPulse(useconds_t time);
void DoNothing(void);
int GetDistance(uint32_t distime);
void exPulse(useconds_t time);
void Wifi_Connect(void);
void Socket_Connect(void);
void retError(int32_t val, char* message);
void FreeConnectCmd(ConnectCmd_t *ConnectParams);
int32_t initAppVariables();
void SendStatusMessage();
void TX_Send();
void RX_Recieve();
void Socket_Reconnect();
void socket_Test();
void Work();
void Check_Response(char *message);
void RGB_TestFunc();
void RGB_Color(int color);
void Safety_Response();


char Recv_Message[BUFF_SIZE];
char Status_Message[BUFF_SIZE];
char ChangedBitSt[10];
char ParkingLotSt[10];
char ParkingSpaceSt[10];
char StatusSt[10];
int ZeroHour = 0;
uint_fast8_t Echo_Echo = 0;
uint_fast8_t Echo_1 = 0;
uint32_t DistTime = 0;
uint32_t Echo_Beg = 0;
uint32_t Echo_End = 0;
int ans = 0;
int BTcount = 0;
int break_it = 0;
int i = 0;
int msgFlag = 0;
int32_t sock = 0;
uint8_t nb = 0;
SlSockAddr_t        *sa;
int32_t             addrSize;
sockAddr_t          sAddr;
int sendFlag = 0;
int powerFlag = 0;
int greenCount = 0;
int two_runs = 0;
int safety_mode = 0;

/* Initialize timer handlers and parameters */
Timer_Handle sens_timer;
Timer_Params sens_params;

Timer_Handle dist_timer;
Timer_Params dist_params;

pthread_t           gSpawn_thread = (pthread_t)NULL;
parkingControlBlock     parking_CB;

/* Event Handler Functions */
/*!
    \brief          SimpleLinkWlanEventHandler

    This handler gets called whenever a WLAN event is reported
    by the host driver / NWP. Here user can implement he's own logic
    for any of these events. This handler is used by 'network_terminal'
    application to show case the following scenarios:

    1. Handling connection / Disconnection.
    2. Handling Addition of station / removal.
    3. RX filter match handler.
    4. P2P connection establishment.

    \param          pWlanEvent       -   pointer to Wlan event data.

    \return         void

    \note           For more information, please refer to: user.h in the porting
                    folder of the host driver and the  CC3120/CC3220 NWP programmer's
                    guide (SWRU455) sections 4.3.4, 4.4.5 and 4.5.5.

    \sa             cmdWlanConnectCallback, cmdEnableFilterCallback, cmdWlanDisconnectCallback,
                    cmdP2PModecallback.

*/
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)
{
    if(!pWlanEvent)
    {
        return;
    }

    switch(pWlanEvent->Id)
    {
        case SL_WLAN_EVENT_CONNECT:
        {
            SET_STATUS_BIT(parking_CB.Status, STATUS_BIT_CONNECTION);

            /* Copy new connection SSID and BSSID to global parameters */
            memcpy(parking_CB.CON_CB.ConnectionSSID, pWlanEvent->Data.Connect.SsidName, pWlanEvent->Data.Connect.SsidLen);
            memcpy(parking_CB.CON_CB.ConnectionBSSID, pWlanEvent->Data.Connect.Bssid, SL_WLAN_BSSID_LENGTH);

            UART_PRINT("\n\r[WLAN EVENT] STA Connected to the AP: %s , "
                "BSSID: %x:%x:%x:%x:%x:%x\n\r",
                      parking_CB.CON_CB.ConnectionSSID, parking_CB.CON_CB.ConnectionBSSID[0],
                      parking_CB.CON_CB.ConnectionBSSID[1],parking_CB.CON_CB.ConnectionBSSID[2],
                      parking_CB.CON_CB.ConnectionBSSID[3],parking_CB.CON_CB.ConnectionBSSID[4],
                      parking_CB.CON_CB.ConnectionBSSID[5]);

            sem_post(&parking_CB.CON_CB.connectEventSyncObj);
        }
        break;

        case SL_WLAN_EVENT_DISCONNECT:
        {
            SlWlanEventDisconnect_t  *pEventData = NULL;

            CLR_STATUS_BIT(parking_CB.Status, STATUS_BIT_CONNECTION);
            CLR_STATUS_BIT(parking_CB.Status, STATUS_BIT_IP_ACQUIRED);
            CLR_STATUS_BIT(parking_CB.Status, STATUS_BIT_IPV6_ACQUIRED);

            /* If ping operation is running, release it. */
            if(IS_PING_RUNNING(parking_CB.Status))
            {
                sem_post(&parking_CB.CON_CB.eventCompletedSyncObj);
                UART_PRINT("\n\rPing failed, since device is no longer connected.\n\r");
            }

            pEventData = &pWlanEvent->Data.Disconnect;

            /* If the user has initiated 'Disconnect' request,
              'reason_code' is SL_WLAN_DISCONNECT_USER_INITIATED */
            if(SL_WLAN_DISCONNECT_USER_INITIATED == pEventData->ReasonCode)
            {
                UART_PRINT("\n\r[WLAN EVENT] Device disconnected from the AP: %s,\n\r"
                "BSSID: %x:%x:%x:%x:%x:%x on application's request \n\r",
                  parking_CB.CON_CB.ConnectionSSID, parking_CB.CON_CB.ConnectionBSSID[0],
                  parking_CB.CON_CB.ConnectionBSSID[1],parking_CB.CON_CB.ConnectionBSSID[2],
                  parking_CB.CON_CB.ConnectionBSSID[3],parking_CB.CON_CB.ConnectionBSSID[4],
                  parking_CB.CON_CB.ConnectionBSSID[5]);
            }
            else
            {
                UART_PRINT("\n\r[WLAN ERROR] Device disconnected from the AP: %s,\n\r"
                "BSSID: %x:%x:%x:%x:%x:%x\n\r",
                  parking_CB.CON_CB.ConnectionSSID, parking_CB.CON_CB.ConnectionBSSID[0],
                  parking_CB.CON_CB.ConnectionBSSID[1],parking_CB.CON_CB.ConnectionBSSID[2],
                  parking_CB.CON_CB.ConnectionBSSID[3],parking_CB.CON_CB.ConnectionBSSID[4],
                  parking_CB.CON_CB.ConnectionBSSID[5]);
            }
            memset(&(parking_CB.CON_CB.ConnectionSSID), 0x0, sizeof(parking_CB.CON_CB.ConnectionSSID));
            memset(&(parking_CB.CON_CB.ConnectionBSSID), 0x0, sizeof(parking_CB.CON_CB.ConnectionBSSID));

            Wifi_Connect();
        }
        break;

        case SL_WLAN_EVENT_PROVISIONING_STATUS:
        {
            /* Do nothing, this suppress provisioning event is because simplelink is configured to default state. */
        }
        break;

        case SL_WLAN_EVENT_STA_ADDED:
        {
            memcpy(&(parking_CB.CON_CB.ConnectionBSSID), pWlanEvent->Data.STAAdded.Mac, SL_WLAN_BSSID_LENGTH);
            UART_PRINT("\n\r[WLAN EVENT] STA was added to AP: BSSID: %x:%x:%x:%x:%x:%x\n\r",
                    parking_CB.CON_CB.ConnectionBSSID[0],parking_CB.CON_CB.ConnectionBSSID[1],
                    parking_CB.CON_CB.ConnectionBSSID[2],parking_CB.CON_CB.ConnectionBSSID[3],
                    parking_CB.CON_CB.ConnectionBSSID[4],parking_CB.CON_CB.ConnectionBSSID[5]);
        }
        break;

        case SL_WLAN_EVENT_STA_REMOVED:
        {
            memcpy(&(parking_CB.CON_CB.ConnectionBSSID), pWlanEvent->Data.STAAdded.Mac, SL_WLAN_BSSID_LENGTH);
            UART_PRINT("\n\r[WLAN EVENT] STA was removed from AP: BSSID: %x:%x:%x:%x:%x:%x\n\r",
                    parking_CB.CON_CB.ConnectionBSSID[0],parking_CB.CON_CB.ConnectionBSSID[1],
                    parking_CB.CON_CB.ConnectionBSSID[2],parking_CB.CON_CB.ConnectionBSSID[3],
                    parking_CB.CON_CB.ConnectionBSSID[4],parking_CB.CON_CB.ConnectionBSSID[5]);

            memset(&(parking_CB.CON_CB.ConnectionBSSID), 0x0, sizeof(parking_CB.CON_CB.ConnectionBSSID));
        }
        break;

        case SL_WLAN_EVENT_RXFILTER:
        {
            SlWlanEventRxFilterInfo_t  *triggred_filter = NULL;

            triggred_filter = &(pWlanEvent->Data.RxFilterInfo) ;

            UART_PRINT("\n\r[WLAN EVENT] Rx filter match triggered. Set filters in filter bitmap :0x%x.\n\r", triggred_filter->UserActionIdBitmap);

            /*
             *     User can write he's / her's rx filter match handler here.
             *     Be advised, you can use the 'triggred_filter' structure info to determine which filter
             *     has received a match. (Bit X is set if user action id X was passed to a filter that matched a packet.)
             */
        }
        break;

        case SL_WLAN_EVENT_P2P_DEVFOUND:
        {
            UART_PRINT("\n\r[WLAN EVENT] P2P Remote device found\n\r");
            sem_post(&(parking_CB.P2P_CB.DeviceFound));
        }
        break;

        case SL_WLAN_EVENT_P2P_REQUEST:
        {
            UART_PRINT("\n\r[WLAN EVENT] P2P Negotiation request received\n\r");

            /* This information is needed to create connection*/
            memset(&(parking_CB.P2P_CB.p2pPeerDeviceName), '\0', sizeof(parking_CB.P2P_CB.p2pPeerDeviceName));
            memcpy(&parking_CB.P2P_CB.p2pPeerDeviceName,
                    pWlanEvent->Data.P2PRequest.GoDeviceName,
                    pWlanEvent->Data.P2PRequest.GoDeviceNameLen);

            sem_post(&parking_CB.P2P_CB.RcvNegReq);
        }
        break;

        case SL_WLAN_EVENT_P2P_CONNECT:
        {
            UART_PRINT("n\r[WLAN EVENT] P2P connection was successfully completed as CLIENT\n\r");
            UART_PRINT("n\rBSSID is %02x:%02x:%02x:%02x:%02x:%02x\n\r",
                                            pWlanEvent->Data.STAAdded.Mac[0],
                                            pWlanEvent->Data.STAAdded.Mac[1],
                                            pWlanEvent->Data.STAAdded.Mac[2],
                                            pWlanEvent->Data.STAAdded.Mac[3],
                                            pWlanEvent->Data.STAAdded.Mac[4],
                                            pWlanEvent->Data.STAAdded.Mac[5]);

            sem_post(&parking_CB.P2P_CB.RcvConReq);
        }
        break;

        case SL_WLAN_EVENT_P2P_CLIENT_ADDED:
        {
            UART_PRINT("n\r[WLAN EVENT] P2P connection was successfully completed as GO\n\r");
            UART_PRINT("n\rBSSID is %02x:%02x:%02x:%02x:%02x:%02x\n\r",
                                            pWlanEvent->Data.P2PClientAdded.Mac[0],
                                            pWlanEvent->Data.P2PClientAdded.Mac[1],
                                            pWlanEvent->Data.P2PClientAdded.Mac[2],
                                            pWlanEvent->Data.P2PClientAdded.Mac[3],
                                            pWlanEvent->Data.P2PClientAdded.Mac[4],
                                            pWlanEvent->Data.P2PClientAdded.Mac[5]);

            sem_post(&parking_CB.P2P_CB.RcvConReq);
        }
        break;

        case SL_WLAN_EVENT_P2P_DISCONNECT:
        {
            UART_PRINT("\n\r[WLAN EVENT] STA disconnected from device.\n\r");
            CLR_STATUS_BIT(parking_CB.Status ,STATUS_BIT_CONNECTION);
        }
        break;

        default:
        {
            UART_PRINT("\n\r[WLAN EVENT] Unexpected event [0x%x]\n\r", pWlanEvent->Id);
        }
        break;
    }
}

/*!
    \brief          SimpleLinkNetAppEventHandler

    This handler gets called whenever a Netapp event is reported
    by the host driver / NWP. Here user can implement he's own logic
    for any of these events. This handler is used by 'network_terminal'
    application to show case the following scenarios:

    1. Handling IPv4 / IPv6 IP address acquisition.
    2. Handling IPv4 / IPv6 IP address Dropping.

    \param          pNetAppEvent     -   pointer to Netapp event data.

    \return         void

    \note           For more information, please refer to: user.h in the porting
                    folder of the host driver and the  CC3120/CC3220 NWP programmer's
                    guide (SWRU455) section 5.7

*/
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
    if(!pNetAppEvent)
    {
        return;
    }

    switch(pNetAppEvent->Id)
    {
        case SL_NETAPP_EVENT_IPV4_ACQUIRED:
        {
            SlIpV4AcquiredAsync_t *pEventData = NULL;

            SET_STATUS_BIT(parking_CB.Status, STATUS_BIT_IP_ACQUIRED);

            /* Ip Acquired Event Data */
            pEventData = &pNetAppEvent->Data.IpAcquiredV4;
            parking_CB.CON_CB.IpAddr = pEventData->Ip ;

            /* Gateway IP address */
            parking_CB.CON_CB.GatewayIP = pEventData->Gateway;

            UART_PRINT("\n\r[NETAPP EVENT] IP set to: IPv4=%d.%d.%d.%d , "
                    "Gateway=%d.%d.%d.%d\n\r",

                    SL_IPV4_BYTE(parking_CB.CON_CB.IpAddr,3),
                    SL_IPV4_BYTE(parking_CB.CON_CB.IpAddr,2),
                    SL_IPV4_BYTE(parking_CB.CON_CB.IpAddr,1),
                    SL_IPV4_BYTE(parking_CB.CON_CB.IpAddr,0),

                    SL_IPV4_BYTE(parking_CB.CON_CB.GatewayIP,3),
                    SL_IPV4_BYTE(parking_CB.CON_CB.GatewayIP,2),
                    SL_IPV4_BYTE(parking_CB.CON_CB.GatewayIP,1),
                    SL_IPV4_BYTE(parking_CB.CON_CB.GatewayIP,0));

            sem_post(&(parking_CB.CON_CB.ip4acquireEventSyncObj));
        }
        break;

        case SL_NETAPP_EVENT_IPV6_ACQUIRED:
        {
            uint32_t i = 0;

            SET_STATUS_BIT(parking_CB.Status, STATUS_BIT_IPV6_ACQUIRED);

            for(i = 0 ; i < 4 ; i++)
            {
                parking_CB.CON_CB.Ipv6Addr[i] = pNetAppEvent->Data.IpAcquiredV6.Ip[i];
            }

            UART_PRINT("\n\r[NETAPP EVENT] IP Acquired: IPv6=");

            for(i = 0; i < 3 ; i++)
            {
                UART_PRINT("%04x:%04x:", ((parking_CB.CON_CB.Ipv6Addr[i]>>16) & 0xffff), parking_CB.CON_CB.Ipv6Addr[i] & 0xffff);
            }

            UART_PRINT("%04x:%04x", ((parking_CB.CON_CB.Ipv6Addr[3]>>16) & 0xffff), parking_CB.CON_CB.Ipv6Addr[3] & 0xffff);
            UART_PRINT(lineBreak);
            sem_post(&parking_CB.CON_CB.ip6acquireEventSyncObj);
        }
        break;

        case SL_NETAPP_EVENT_DHCPV4_LEASED:
        {
            SET_STATUS_BIT(parking_CB.Status, STATUS_BIT_IP_LEASED);
            SET_STATUS_BIT(parking_CB.Status, STATUS_BIT_IP_ACQUIRED);

            parking_CB.CON_CB.StaIp = pNetAppEvent->Data.IpLeased.IpAddress;
            UART_PRINT("\n\r[NETAPP EVENT] IP Leased to Client: IP=%d.%d.%d.%d \n\r",
                        SL_IPV4_BYTE(parking_CB.CON_CB.StaIp ,3), SL_IPV4_BYTE(parking_CB.CON_CB.StaIp ,2),
                        SL_IPV4_BYTE(parking_CB.CON_CB.StaIp ,1), SL_IPV4_BYTE(parking_CB.CON_CB.StaIp ,0));

            sem_post(&(parking_CB.CON_CB.ip4acquireEventSyncObj));
        }
        break;

        case SL_NETAPP_EVENT_DHCPV4_RELEASED:
        {
            UART_PRINT("\n\r[NETAPP EVENT] IP is released.\n\r");
        }
        break;

        default:
        {
            UART_PRINT("\n\r[NETAPP EVENT] Unexpected event [0x%x] \n\r", pNetAppEvent->Id);
        }
        break;
    }
}

/*!
    \brief          SimpleLinkHttpServerEventHandler

    This handler gets called whenever a HTTP event is reported
    by the NWP internal HTTP server.

    \param          pHttpEvent       -   pointer to http event data.

    \param          pHttpEvent       -   pointer to http response.

    \return         void

    \note           For more information, please refer to: user.h in the porting
                    folder of the host driver and the  CC3120/CC3220 NWP programmer's
                    guide (SWRU455) chapter 9.

*/
void SimpleLinkHttpServerEventHandler(SlNetAppHttpServerEvent_t *pHttpEvent,
                                      SlNetAppHttpServerResponse_t *pHttpResponse)
{
    /* Unused in this application */
}

/*!
    \brief          SimpleLinkGeneralEventHandler

    This handler gets called whenever a general error is reported
    by the NWP / Host driver. Since these errors are not fatal,
    application can handle them.

    \param          pDevEvent    -   pointer to device error event.

    \return         void

    \note           For more information, please refer to: user.h in the porting
                    folder of the host driver and the  CC3120/CC3220 NWP programmer's
                    guide (SWRU455) section 17.9.

*/
void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent)
{
    if(!pDevEvent)
    {
        return;
    }
    /*
      Most of the general errors are not FATAL are are to be handled
      appropriately by the application
    */
    UART_PRINT("\n\r[GENERAL EVENT] - ID=[%d] Sender=[%d]\n\n",
               pDevEvent->Data.Error.Code,
               pDevEvent->Data.Error.Source);
}

/*!
    \brief          SimpleLinkSockEventHandler

    This handler gets called whenever a socket event is reported
    by the NWP / Host driver.

    \param          SlSockEvent_t    -   pointer to socket event data.

    \return         void

    \note           For more information, please refer to: user.h in the porting
                    folder of the host driver and the  CC3120/CC3220 NWP programmer's
                    guide (SWRU455) section 7.6.

*/
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
    /* Unused in this application */
}

/*!
    \brief          SimpleLinkFatalErrorEventHandler

    This handler gets called whenever a socket event is reported
    by the NWP / Host driver. After this routine is called, the user's
    application must restart the device in order to recover.

    \param          slFatalErrorEvent    -   pointer to fatal error event.

    \return         void

    \note           For more information, please refer to: user.h in the porting
                    folder of the host driver and the  CC3120/CC3220 NWP programmer's
                    guide (SWRU455) section 17.9.

*/
void SimpleLinkFatalErrorEventHandler(SlDeviceFatal_t *slFatalErrorEvent)
{

    switch (slFatalErrorEvent->Id)
    {
        case SL_DEVICE_EVENT_FATAL_DEVICE_ABORT:
        {
            UART_PRINT("\n\r[ERROR] - FATAL ERROR: Abort NWP event detected: "
                        "AbortType=%d, AbortData=0x%x\n\r",
                        slFatalErrorEvent->Data.DeviceAssert.Code,
                        slFatalErrorEvent->Data.DeviceAssert.Value);
        }
        break;

        case SL_DEVICE_EVENT_FATAL_DRIVER_ABORT:
        {
            UART_PRINT("\n\r[ERROR] - FATAL ERROR: Driver Abort detected. \n\r");
        }
        break;

        case SL_DEVICE_EVENT_FATAL_NO_CMD_ACK:
        {
            UART_PRINT("\n\r[ERROR] - FATAL ERROR: No Cmd Ack detected "
                        "[cmd opcode = 0x%x] \n\r",
                                        slFatalErrorEvent->Data.NoCmdAck.Code);
        }
        break;

        case SL_DEVICE_EVENT_FATAL_SYNC_LOSS:
        {
            UART_PRINT("\n\r[ERROR] - FATAL ERROR: Sync loss detected n\r");
        }
        break;

        case SL_DEVICE_EVENT_FATAL_CMD_TIMEOUT:
        {
            UART_PRINT("\n\r[ERROR] - FATAL ERROR: Async event timeout detected "
                        "[event opcode =0x%x]  \n\r",
                                    slFatalErrorEvent->Data.CmdTimeout.Code);
        }
        break;

        default:
            UART_PRINT("\n\r[ERROR] - FATAL ERROR: Unspecified error detected \n\r");
        break;
    }
}

/*!
    \brief          SimpleLinkNetAppRequestEventHandler

    This handler gets called whenever a NetApp event is reported
    by the NWP / Host driver. User can write he's logic to handle
    the event here.

    \param          pNetAppRequest     -   Pointer to NetApp request structure.

    \param          pNetAppResponse    -   Pointer to NetApp request Response.

    \note           For more information, please refer to: user.h in the porting
                    folder of the host driver and the  CC3120/CC3220 NWP programmer's
                    guide (SWRU455) section 17.9.

    \return         void

*/
void SimpleLinkNetAppRequestEventHandler(SlNetAppRequest_t *pNetAppRequest, SlNetAppResponse_t *pNetAppResponse)
{
    /* Unused in this application */
}

/*!
    \brief          SimpleLinkNetAppRequestMemFreeEventHandler

    This handler gets called whenever the NWP is done handling with
    the buffer used in a NetApp request. This allows the use of
    dynamic memory with these requests.

    \param          pNetAppRequest     -   Pointer to NetApp request structure.

    \param          pNetAppResponse    -   Pointer to NetApp request Response.

    \note           For more information, please refer to: user.h in the porting
                    folder of the host driver and the  CC3120/CC3220 NWP programmer's
                    guide (SWRU455) section 17.9.

    \return         void

*/
void SimpleLinkNetAppRequestMemFreeEventHandler(uint8_t *buffer)
{
    /* Unused in this application */
}

// End Event handlers

/*!
    \brief          Configure SimpleLink to default state.

    This routine configures the device to a default state.
    It's important to note that this is one example for a 'restore to default state'
    function, which meet the needs of this application, 'Network Terminal'. User who
    wish to incorporate this function into he's app, must adjust the implementation
    and make sure it meets he's needs.

    \return         Upon successful completion, the function shall return 0.
                    In case of failure, this function would return -1.

*/
int32_t ConfigureSimpleLinkToDefaultState()
{
     uint8_t                              ucConfigOpt;
     uint8_t                              ucPower;
     int32_t                              RetVal = -1;
     int32_t                              Mode = -1;
     uint32_t                             IfBitmap = 0;
     SlWlanScanParamCommand_t             ScanDefault = {0};
     SlWlanRxFilterOperationCommandBuff_t RxFilterIdMask = {{0}};

     /* Turn NWP on */
     Mode = sl_Start(0, 0, 0);
     retError(Mode, "sl_Start ");
     ASSERT_ON_ERROR(Mode, DEVICE_ERROR);

     if(Mode != ROLE_STA)
     {
           /* Set NWP role as STA */
           Mode = sl_WlanSetMode(ROLE_STA);
           retError(Mode, "Set mode ");
           ASSERT_ON_ERROR(Mode, WLAN_ERROR);

         /* For changes to take affect, we restart the NWP */
         RetVal = sl_Stop(SL_STOP_TIMEOUT);
         ASSERT_ON_ERROR(RetVal, DEVICE_ERROR);

         Mode = sl_Start(0, 0, 0);
         ASSERT_ON_ERROR(Mode, DEVICE_ERROR);
     }

     if(Mode != ROLE_STA)
     {
         UART_PRINT("Failed to configure device to it's default state");
         return -1;
     }

     /* Set policy to auto only */
     RetVal = sl_WlanPolicySet(SL_WLAN_POLICY_CONNECTION, SL_WLAN_CONNECTION_POLICY(1,0,0,0), NULL ,0);
     ASSERT_ON_ERROR(RetVal, WLAN_ERROR);

     /* Disable Auto Provisioning */
     RetVal = sl_WlanProvisioning(SL_WLAN_PROVISIONING_CMD_STOP, 0xFF, 0, NULL, 0x0);
     ASSERT_ON_ERROR(RetVal, WLAN_ERROR);

     /* Delete existing profiles */
     RetVal = sl_WlanProfileDel(0xFF);
     ASSERT_ON_ERROR(RetVal, WLAN_ERROR);

     /* enable DHCP client */
     RetVal = sl_NetCfgSet(SL_NETCFG_IPV4_STA_ADDR_MODE, SL_NETCFG_ADDR_DHCP, 0, 0);
     ASSERT_ON_ERROR(RetVal, NETAPP_ERROR);

     /* Disable ipv6 */
     IfBitmap = !(SL_NETCFG_IF_IPV6_STA_LOCAL | SL_NETCFG_IF_IPV6_STA_GLOBAL);
     RetVal = sl_NetCfgSet(SL_NETCFG_IF, SL_NETCFG_IF_STATE, sizeof(IfBitmap),(const unsigned char *)&IfBitmap);
     ASSERT_ON_ERROR(RetVal, NETAPP_ERROR);

     /* Configure scan parameters to default */
     ScanDefault.ChannelsMask = CHANNEL_MASK_ALL;
     ScanDefault.RssiThreshold = RSSI_TH_MAX;

     RetVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, SL_WLAN_GENERAL_PARAM_OPT_SCAN_PARAMS, sizeof(ScanDefault), (uint8_t *)&ScanDefault);
     ASSERT_ON_ERROR(RetVal, WLAN_ERROR);

     /* Disable scans */
     ucConfigOpt = SL_WLAN_SCAN_POLICY(0, 0);
     RetVal = sl_WlanPolicySet(SL_WLAN_POLICY_SCAN , ucConfigOpt, NULL, 0);
     ASSERT_ON_ERROR(RetVal, WLAN_ERROR);

     /* Set TX power lvl to max */
     ucPower = 0;
     RetVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, SL_WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1, (uint8_t *)&ucPower);
     ASSERT_ON_ERROR(RetVal, WLAN_ERROR);

     /* Set NWP Power policy to 'normal' */
     RetVal = sl_WlanPolicySet(SL_WLAN_POLICY_PM, SL_WLAN_NORMAL_POLICY, NULL, 0);
     ASSERT_ON_ERROR(RetVal, WLAN_ERROR);

     /* Unregister mDNS services */
     RetVal = sl_NetAppMDNSUnRegisterService(0, 0, 0);
     ASSERT_ON_ERROR(RetVal, NETAPP_ERROR);

     /* Remove all 64 RX filters (8*8) */
     memset(RxFilterIdMask.FilterBitmap , 0xFF, 8);

     RetVal = sl_WlanSet(SL_WLAN_RX_FILTERS_ID, SL_WLAN_RX_FILTER_REMOVE, sizeof(SlWlanRxFilterOperationCommandBuff_t),(uint8_t *)&RxFilterIdMask);
     ASSERT_ON_ERROR(RetVal, WLAN_ERROR);

     /* Set NWP role as STA */
     RetVal = sl_WlanSetMode(ROLE_STA);
     ASSERT_ON_ERROR(RetVal, WLAN_ERROR);

     /* For changes to take affect, we restart the NWP */
     RetVal = sl_Stop(SL_STOP_TIMEOUT);
     ASSERT_ON_ERROR(RetVal, DEVICE_ERROR);

     Mode = sl_Start(0, 0, 0);
     ASSERT_ON_ERROR(Mode, DEVICE_ERROR);

     if(ROLE_STA != Mode)
     {
         UART_PRINT("Failed to configure device to it's default state");
         return -1 ;
     }
     else
     {
         parking_CB.Role = ROLE_STA;
         SET_STATUS_BIT(parking_CB.Status, STATUS_BIT_NWP_INIT);
     }

     return 0;
}

void PrintIPAddress(unsigned char ipv6, void *ip)
{
    uint32_t        *pIPv4;
    uint8_t         *pIPv6;
    int32_t          i=0;

    if(!ip)
    {
        return;
    }

    if(ipv6)
    {
        pIPv6 = (uint8_t*) ip;

        for(i = 0; i < 14; i+=2)
        {
            UART_PRINT("%02x%02x:", pIPv6[i], pIPv6[i+1]);
        }

        UART_PRINT("%02x%02x", pIPv6[i], pIPv6[i+1]);
    }
    else
    {
        pIPv4 = (uint32_t*)ip;
        UART_PRINT("%d.%d.%d.%d", SL_IPV4_BYTE(*pIPv4,3), SL_IPV4_BYTE(*pIPv4,2),
                                  SL_IPV4_BYTE(*pIPv4,1), SL_IPV4_BYTE(*pIPv4,0));
    }
    return;
}

int32_t ipv4AddressParse(char *str, uint32_t *ipv4ip)
{
    volatile int32_t i = 0;
    uint32_t         n;
    uint32_t         ipv4Address = 0;
    char             *token;

    token = strtok(str, ".");
    if (token)
    {
        n = (int)strtoul(token, 0, 10);
    }
    else
    {
        return -1;
    }

    while(i < 4)
    {
       /* Check Whether IP is valid */
       if((token != NULL) && (n < 256))
       {
           ipv4Address |= n;
           if(i < 3)
           {
               ipv4Address = ipv4Address << 8;
           }
           token=strtok(NULL,".");
           if (token)
           {
               n = (int)strtoul(token, 0, 10);
           }
           i++;
       }
       else
       {
           return -1;
       }
    }

    *ipv4ip = ipv4Address;

    //UART_PRINT("From parse function ")
    return 0;
}

int32_t sem_wait_timeout(sem_t *sem, uint32_t Timeout)
{
    struct timespec abstime;
    abstime.tv_nsec = 0 ;
    abstime.tv_sec = 0 ;

    /* Since POSIX timeout are relative and not absolute,
     * take the current timestamp. */
    clock_gettime(CLOCK_REALTIME, &abstime);
    if ( abstime.tv_nsec < 0 )
    {
        abstime.tv_sec = Timeout;
        return (sem_timedwait(sem , &abstime));
    }

    /* Add the amount of time to wait */
    abstime.tv_sec += Timeout / 1000 ;
    abstime.tv_nsec += (Timeout % 1000)*1000000;

    abstime.tv_sec += (abstime.tv_nsec / 1000000000);
    abstime.tv_nsec = abstime.tv_nsec % 1000000000;

    /* Call the semaphore wait API */
    return sem_timedwait(sem , &abstime);
}

/*
 * Interrupt callback for the echo pin from the sensor
 * called whenever the echo pin changes from low to high or high to low
 * gets the time for the pulse and sends it to GetDistance()
 * If the sensor is blocked for a certain time, then this will turn on the beacon
 * If the sensor is freed, then it will turn off the beacon
 * updates the status message
 */

void Echo_NoTimer_Callback(uint_least8_t index)
{
    Echo_Echo = GPIO_read(Board_GPIO_Echo);
    if(Echo_Echo == 1)
    {
        Timer_start(dist_timer);
        Echo_Beg = Timer_getCount(dist_timer);
    } else
    {
        Echo_End = Timer_getCount(dist_timer);
        Timer_stop(dist_timer);
        DistTime = Echo_End - Echo_Beg;
        if(DistTime > 0)
        {
            ans = GetDistance(DistTime);
            //UART_PRINT(" beg: %d end: %d diff: %i\n\r", Echo_Beg, Echo_End, DistTime);
        }
    }
    if(ans >= 5)
    {
        // 5 seconds reached, turn LED blue and BT on
        if(strcmp(StatusSt, OPEN_SPACE) == 0)
        {
            // if status was open
            RGB_Color(BLUE);
            GPIO_write(Board_GPIO_BT, 1);
            strcpy(StatusSt, BUSY_SPACE);
            strcpy(ChangedBitSt, CHANGEDBIT_1);
            sendFlag = 1;
        }
        if(sendFlag == 0)
        {
            strcpy(ChangedBitSt, CHANGEDBIT_0);
        }
    }
    else
    {
        //UART_PRINT("BT should be done [%s] **************\n\r", StatusSt);
        if((strcmp(StatusSt, BUSY_SPACE) == 0) ||
                (strcmp(StatusSt, CLOSED_SPACE) == 0) ||
                (strcmp(StatusSt, ILLEGAL_SPACE) == 0))
        {
            // if space was busy or closed
            GPIO_write(Board_GPIO_B, RGB_OFF);
            GPIO_write(Board_GPIO_B1, RGB_OFF);
            GPIO_write(Board_GPIO_B2, RGB_OFF);
            GPIO_write(Board_GPIO_BT, 0);
            strcpy(StatusSt, OPEN_SPACE);
            strcpy(ChangedBitSt, CHANGEDBIT_1);
            sendFlag = 1;
        }
        if(sendFlag == 0)
        {
            strcpy(ChangedBitSt, CHANGEDBIT_0);
        }

    }
}

/*
 * Test interrupt function, toggles the test LED during every interrupt
 */

void Test_Callback(uint_least8_t index)
{
    GPIO_toggle(Board_GPIO_TEST);
}

/*
 *  ======== mainThread ========
 */

void *mainThread(void *arg0)
{
    int32_t             RetVal ;
    pthread_attr_t      pAttrs_spawn;
    struct sched_param  priParam;

    /* Call driver init functions */
    GPIO_init();
    SPI_init();
    UART_init();
    Timer_init();

    RetVal = initAppVariables();
    /* Initialize UART terminal for UART_PRINT() */
    InitTerm();

    strcpy(ChangedBitSt, CHANGEDBIT_0);
    strcpy(ParkingLotSt, ParkingLot);
    strcpy(ParkingSpaceSt, ParkingSpace);
    strcpy(StatusSt, OPEN_SPACE);
    /* Configure the LED pin */
    GPIO_setConfig(Board_GPIO_LED0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Configure Echo Pin for interrupts */

    GPIO_setConfig(Board_GPIO_Echo, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_BOTH_EDGES);

    /* Turn on user LED */
    GPIO_write(Board_GPIO_LED0, Board_GPIO_LED_ON);

    GPIO_write(Board_GPIO_TEST, 0);
    GPIO_write(Board_GPIO_TRIG, 0);
    RGB_Color(NONE);
    GPIO_write(Board_GPIO_BT, 0);

    /* Create the sl_Task internal spawn thread */
    pthread_attr_init(&pAttrs_spawn);
    priParam.sched_priority = SPAWN_TASK_PRIORITY;
    RetVal = pthread_attr_setschedparam(&pAttrs_spawn, &priParam);
    RetVal |= pthread_attr_setstacksize(&pAttrs_spawn, TASK_STACK_SIZE);

    /* The SimpleLink host driver architecture mandate spawn thread to be created prior to calling Sl_start (turning the NWP on). */
    /* The purpose of this thread is to handle asynchronous events sent from the NWP.
     * Every event is classified and later handled by the Host driver event handlers. */
    RetVal = pthread_create(&gSpawn_thread, &pAttrs_spawn, sl_Task, NULL);
    if(RetVal < 0)
    {
        /* Handle Error */
         UART_PRINT("Network Terminal - Unable to create spawn thread \n");
         return(NULL);
    }

    RetVal = ConfigureSimpleLinkToDefaultState();
    if(RetVal < 0)
    {
        // Handle Error
        UART_PRINT("Network Terminal - Couldn't configure Network Processor\n");
        return(NULL);
    }
    retError(RetVal, "Configure");

    //safety_mode = 1;

    if(safety_mode == 0)
    {
        UART_PRINT("Wifi Connection Started \n\r");
        Wifi_Connect();
    }

    /* Configure timer params */
    Timer_Params_init(&dist_params);
    dist_params.period = 1000000; /* 1s */
    dist_params.periodUnits = Timer_PERIOD_US;
    dist_params.timerMode = Timer_FREE_RUNNING;

    /* Open timer to be started with Timer_Start() */
    dist_timer = Timer_open(Board_TIMER0, &dist_params);

    if (dist_timer == NULL) {
        /* Failed to initialized timer */
        UART_PRINT("Distance Timer failed to initialize \n\r");
        while (1);
    }

    /* callback and interrupt command */
    GPIO_setCallback(Board_GPIO_Echo, Echo_NoTimer_Callback);

    GPIO_enableInt(Board_GPIO_Echo);

    while(1)
    {
        Work();
        //RGB_TestFunc();
    }
}

/*
 *          exPulse(useconds_t time)
 *
 * Test function for sending a pulse through the test LED
 * time: time in microseconds for the pulse
 */

void exPulse(useconds_t time)
{
    GPIO_write(Board_GPIO_TEST, 1);
    usleep(time);
    GPIO_write(Board_GPIO_TEST, 0);
    usleep(time);
    return;
}

/*
 *          setPulse(useconds_t time)
 *
 * Sends pulse to the trig pin on the sensor for the parameter time in microseconds
 * time: micro second time for the pulse
 */

void setPulse(useconds_t time)
{
    GPIO_write(Board_GPIO_TRIG, 1);
    usleep(time);
    GPIO_write(Board_GPIO_TRIG, 0);
    return;
}

/*
 *          DoNothing(void)
 *
 * Function for clarity of code to describe when nothing is happening
 */

void DoNothing(void)
{
    return;
}

/*
 *          GetDistance(uint32_t distime)
 *
 * Function that takes in the time length of the pulse from the echo pin
 * If the pulse is less than the set threshold, then the test LED will turn on the signify that the
 * sensor is blocked
 * distime: timer value from the pulse length from the echo pin of the sensor
 * return: count in seconds of how long an object has been in range of the sensor
 */

int GetDistance(uint32_t distime)
{
    if(distime < THRESHOLD)
    {
        if(strcmp(StatusSt, OPEN_SPACE) == 0)
        {
            // if spot was open
            GPIO_write(Board_GPIO_TEST, RGB_ON);
            powerFlag = 0;
        }
        BTcount++;
    } else
    {
        RGB_Color(NONE);
        // power management flag
        powerFlag = 1;

        GPIO_write(Board_GPIO_TEST, RGB_OFF);
        BTcount = 0;
    }
    return BTcount;
}

/*
 *          Wifi_Connect(void)
 *
 * Function to connect the board to the Internet with the password and ssid set with #define
 */

void Wifi_Connect(void)
{
    SlWlanSecParams_t secParams = {0};
    int32_t ret = 0;

    secParams.Key = WIFI_KEY;
    secParams.KeyLen = strlen(WIFI_KEY);
    secParams.Type = SL_WLAN_SEC_TYPE_WPA_WPA2;
    //secParams.Type = SL_WLAN_SEC_TYPE_OPEN;

    if(parking_CB.Role != ROLE_STA)
    {
        ret = sl_WlanSetMode(ROLE_STA);
        retError(ret, "WLAN set STA");

        ret = sl_Stop(SL_STOP_TIMEOUT);
        retError(ret, "SL stop");

        ret = sl_Start(0, 0, 0);
        retError(ret, "SL start");
    }
    /* Here wer'e in STA mode */
    parking_CB.Role = ROLE_STA ;

    ret = sl_WlanConnect((signed char*)WIFI_SSID, strlen(WIFI_SSID), 0, &secParams, 0);
    if(ret < 0)
    {
        strcpy(StatusSt, BROKEN_SPACE);
        RGB_Color(ALL);
    }
    retError(ret, "Wifi connection");

    UART_PRINT("Trying to connect to AP : %s\n\r", WIFI_SSID);

    return;
}

/*
 *          retError(int32_t val, char* message)
 *
 * Error checking function, will print to the uart terminal whether the function succeeded or not
 * val: status result from commands
 * message: type of command that is going through error checking, added to the message for uart_print
 */

void retError(int32_t val, char* message)
{
    if(val < 0)
    {
        UART_PRINT("%s Error: %d \n\r", message, val);
    } else
    {
        UART_PRINT("%s success \n\r", message);
    }
    return;
}

/*
 *          FreeConnectCmd(ConnectCmd_t *ConnectParams)
 *
 * Frees commands from memory
 */

void FreeConnectCmd(ConnectCmd_t *ConnectParams)
{
    if (ConnectParams->ssid != NULL)
    {
        free(ConnectParams->ssid);
        ConnectParams->ssid = NULL;
    }

    if (ConnectParams->ip != NULL)
    {
        free(ConnectParams->ip);
        ConnectParams->ip = NULL;
    }

    if (ConnectParams->gw != NULL)
    {
        free(ConnectParams->gw);
        ConnectParams->gw = NULL;
    }

    if (ConnectParams->dns != NULL)
    {
        free(ConnectParams->dns);
        ConnectParams->dns = NULL;
    }

    if (ConnectParams->secParams.Key != NULL)
    {
        free(ConnectParams->secParams.Key);
        ConnectParams->secParams.Key = NULL;
    }

    if (ConnectParams->secParamsEnt.User != NULL)
    {
        free(ConnectParams->secParamsEnt.User);
        ConnectParams->secParamsEnt.User = NULL;
    }

    return;
}

/*
 *          initAppVariables()
 *
 * Initializes and memsets for control black variable for the parking sensor
 */

int32_t    initAppVariables(void)
{
    int32_t ret = 0;

    parking_CB.Status = 0 ;
    parking_CB.Role = ROLE_RESERVED;
    parking_CB.Exit = FALSE;

    memset(&parking_CB.CmdBuffer, 0x0, CMD_BUFFER_LEN);
    memset(&parking_CB.gDataBuffer, 0x0, sizeof(parking_CB.gDataBuffer));
    memset(&parking_CB.CON_CB, 0x0, sizeof(parking_CB.CON_CB));

    ret = sem_init(&parking_CB.CON_CB.connectEventSyncObj,    0, 0);
    if(ret != 0)
    {
        //SHOW_WARNING(ret, OS_ERROR);
        retError(ret, "sem init 1");
        return -1;
    }

    ret = sem_init(&parking_CB.CON_CB.eventCompletedSyncObj,  0, 0);
    if(ret != 0)
    {
        retError(ret, "sem init 2");
        return -1;
    }

    ret = sem_init(&parking_CB.CON_CB.ip4acquireEventSyncObj, 0, 0);
    if(ret != 0)
    {
        retError(ret, "sem init 3");
        return -1;
    }

    ret = sem_init(&parking_CB.CON_CB.ip6acquireEventSyncObj, 0, 0);
    if(ret != 0)
    {
        retError(ret, "sem init 4");
        return -1;
    }

    memset(&parking_CB.P2P_CB, 0x0, sizeof(parking_CB.P2P_CB));

    ret = sem_init(&parking_CB.P2P_CB.DeviceFound, 0, 0);
    if(ret != 0)
    {
        retError(ret, "sem init 5");
        return -1;
    }

    ret = sem_init(&parking_CB.P2P_CB.RcvConReq, 0, 0);
    if(ret != 0)
    {
        retError(ret, "sem init 6");
        return -1;
    }

    ret = sem_init(&parking_CB.P2P_CB.RcvNegReq, 0, 0);
    if(ret != 0)
    {
        retError(ret, "sem init 7");
        return -1;
    }

    ret = sem_init(&parking_CB.WowlanSleepSem, 0, 0);
    if(ret != 0)
    {
        retError(ret, "sem init 8");
        return -1;
    }

    return ret;
}

/*
#define ParkingLot "East"
#define ParkingSpace "12"
#define OPEN_SPACE "open"
#define BUSY_SPACE "busy"
#define CLOSED_SPACE "closed"
#define BROKEN_SPACE "broken"
#define CHANGEDBIT_0 '0'
#define CHANGEDBIT_1 '1'

char ChangedBitSt[10];
char ParkingLotSt[10];
char ParkingSpaceSt[10];
char StatusSt[10];
*/

/*
 *          SendStatusMessage()
 *
 * Send current status message to uart terminal and signals to tx_send() to send the message to the
 * gateway
 */

void SendStatusMessage(void)
{
    strcat(Status_Message, ChangedBitSt);
    strcat(Status_Message, ParkingLotSt);
    strcat(Status_Message, ":");
    strcat(Status_Message, ParkingSpaceSt);
    strcat(Status_Message, " ");
    strcat(Status_Message, StatusSt);
    UART_PRINT("%s \n\r", Status_Message);
    if(safety_mode == 0)
    {
        TX_Send();
    }
    strcpy(Status_Message, "");
    return;
}

/*
 *          Socket_Connect()
 *
 * Connects CC3220s to the gateway and sets timeout for socket commands
 */

void Socket_Connect(void)
{
    int32_t             status;
    int32_t             nonBlocking;
    char                *ip = LAPTOP_IP;

    UART_PRINT("IP Address: [%s] [%s] \n\r", LAPTOP_IP, ip);

    /* filling the TCP server socket address */
    sAddr.in4.sin_family = SL_AF_INET;

    /* Since this is the client's side, we must know beforehand the IP address
     * and the port of the server wer'e trying to connect.
     */
    sAddr.in4.sin_port = sl_Htons((unsigned short)PORTNUM);
    sAddr.in4.sin_addr.s_addr = sl_Htonl(SL_IPV4_VAL(192,168,137,65));

    sa = (SlSockAddr_t*)&sAddr.in4;
    addrSize = sizeof(SlSockAddrIn6_t);
    UART_PRINT("addrSize: [%d]\n\r", addrSize);

    /* Get socket descriptor - this would be the
     * socket descriptor for the TCP session.
     */
    UART_PRINT("Literally before %i \n\r", sock);
    sock = sl_Socket(sa->sa_family, SL_SOCK_STREAM, TCP_PROTOCOL_FLAGS);
    UART_PRINT("Literally after %i \n\r", sock);
    retError(sock, "Socket set");

    // Timeout for socket commands

    struct SlTimeval_t tv;

    tv.tv_sec = 120;
    tv.tv_usec = 0;
    status = sl_SetSockOpt(sock, SL_SOL_SOCKET, SL_SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    /* Set socket as non-blocking socket (if needed):
     * Non-blocking sockets allows user to handle other tasks rather than block
     * on socket API calls. If an API call using the Non-blocking socket descriptor
     * returns 'SL_ERROR_BSD_EAGAIN' - this indicate that the user should try the API again later.
     */
    //nb = 1;
    if(TRUE == nb)
    {
        nonBlocking = TRUE;
        status = sl_SetSockOpt(sock, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &nonBlocking, sizeof(nonBlocking));

        if(status < 0)
        {
            UART_PRINT("[line:%d, error:%d] %s\n\r", __LINE__, status, SL_SOCKET_ERROR);
            sl_Close(sock);
            return;
        }
    }

    status = -1;

    while(status < 0)
    {
        /* Calling 'sl_Connect' followed by server's
         * 'sl_Accept' would start session with
         * the TCP server. */
        UART_PRINT("About to connect \n\r");
        status = sl_Connect(sock, sa, addrSize);
        UART_PRINT("After connect %d \n\r", status);
        if((status == SL_ERROR_BSD_EALREADY) && (TRUE == nb))
        {
            sleep(1);
            continue;
        }
        if(status < 0)
        {
            UART_PRINT("[line:%d, error:%d] %s\n\r", __LINE__, status, SL_SOCKET_ERROR);
            sl_Close(sock);
            return;
        }
        break;
    }
    return;
}

/*
 *          TX_Send()
 *
 * Sends current status message to the gateway via the socket
 */

void TX_Send(void)
{
    int32_t  buflen;
    uint32_t sent_bytes = 0;
    //uint32_t bytes_to_send = (numberOfPackets * BUF_LEN);
    uint32_t bytes_to_send = strlen(Status_Message);
    int32_t             status;

    while(sent_bytes < bytes_to_send)
    {
        if (bytes_to_send - sent_bytes >= BUFF_SIZE)
            buflen = BUFF_SIZE;
        else
            buflen  = bytes_to_send - sent_bytes;

        /* Send packets to the server */
        UART_PRINT("sock: %i About to send: %s \n\r", sock, Status_Message);
        int tx_count = 0;
        while(1)
        {
            status = sl_Send(sock, &Status_Message, buflen, 0);
            if((status == SL_ERROR_BSD_EAGAIN) && (TRUE == nb))
            {
                sleep(1);
                continue;
            }
            else if(status < 0)
            {
                ++tx_count;
                UART_PRINT("Tx Attempt: %i \n\r", tx_count);
                if(tx_count == 5)
                {
                    UART_PRINT("[line:%d, error:%d] %s\n\r", __LINE__, status, SL_SOCKET_ERROR);
                    sl_Close(sock);
                    return;
                }

            }
            else
            {
                // if tx successful break out of while loop
                break;
            }
        }
        i++;
        sent_bytes += status;
    }

    UART_PRINT("Sent %u packets (%u bytes) successfully\n\r",
               i,
               sent_bytes);
    RX_Recieve();
    return;
}

/*
 *                       RX_Recieve()
 *
 * Waits to receive acknowledgment from the gateway,
 * blocks but has 10 minute timeout to wait for the
 * user
 */

void RX_Recieve(void)
{
    uint32_t rcvd_bytes = 0;
    int32_t  status;

    memset(Recv_Message, 0, BUFF_SIZE);

    UART_PRINT("Before receive \n\r");
    status = sl_Recv(sock, Recv_Message, BUFF_SIZE, 0);
    UART_PRINT("After receive \n\r");
        if((status == SL_ERROR_BSD_EAGAIN) && (TRUE == nb))
        {
            sleep(1);
        }
        else if(status < 0)
        {
            UART_PRINT("[line:%d, error:%d] %s\n\r", __LINE__, status, BSD_SOCKET_ERROR);
            sl_Close(sock);
            return;
        }
        else if (status == 0)
        {
            UART_PRINT("TCP Server closed the connection\n\r");
            //break;
        }
        rcvd_bytes += status;

    UART_PRINT("Received %u packets (%u bytes) successfully\n\r",(rcvd_bytes/BUFF_SIZE), rcvd_bytes);
    UART_PRINT("We got %s \n\r", Recv_Message);
    status = sl_Close(sock);
    retError(status, "Socket Close");
    UART_PRINT("Sock after close %i %d\n\r", sock, status);
    Check_Response(Recv_Message);
    return;
}

/*
 *          Check_Response()
 *
 * Checks the response message from the gateway and
 * adjusts the LED color accordingly
 */

void Check_Response(char *message)
{
    GPIO_write(Board_GPIO_BT, 0);
    if(strcmp(message, CLOSED_SPACE) == 0)
    {
        RGB_Color(GREEN);
        strcpy(StatusSt, message);
    }
    else if(strcmp(message, ILLEGAL_SPACE) == 0)
    {
        RGB_Color(RED);
        strcpy(StatusSt, message);
    }
    else if(strcmp(message, OK_SPACE) == 0)
    {

    }
    else if(strcmp(message, ACK_SPACE) == 0)
    {

    }
    else
    {
        // Error or some other string sent, resends the message to see
        // if it will work better now
        // do not want to change the status message as a whole
        // #robustness B)
        RGB_Color(PURPLE);
        sleep(1);
        if(safety_mode == 0)
        {
            Socket_Connect();
        }
        SendStatusMessage();
    }
    return;
}

/*
 *          socket_Test()
 *
 * Function used to test the connection to the socket by sending a
 * fake message and waiting for a response from the gateway
 */

void socket_Test(void)
{
    Socket_Connect();
    // Example busy message
    strcpy(ChangedBitSt, CHANGEDBIT_1);
    strcpy(ParkingLotSt, ParkingLot);
    strcpy(ParkingSpaceSt, ParkingSpace);
    strcpy(StatusSt, BUSY_SPACE);
    SendStatusMessage();
    //
    return;
}

/*
 *          Work()
 *
 * Main function that sends pulse to the trig pin and uses the
 * sendflag to determine whether to send a status message or
 * not depending on the interrupt
 * Does not use the timer callback
 * Actually works
 */

void Work(void)
{
    int32_t status = 0;
    // Send a 10 us pulse to trig pin
    setPulse(10);

    UART_PRINT("[%s] [%s] \n\r", ChangedBitSt, StatusSt);
    // Send message indicating an event has occurred
    if(sendFlag == 1)
    {
        if(safety_mode == 0)
        {
            UART_PRINT("Reconnecting to Socket... \n\r");
            Socket_Connect();
            SendStatusMessage();
        }
        else
        {
            SendStatusMessage();
            UART_PRINT("Safety Response... \n\r");
            Safety_Response();
        }
        sendFlag = 0;
    }

    // Retain heartbeat message counter during hibernation
    if(getHibRetentionReg() >= 1)
    {
        ZeroHour = getHibRetentionReg();
    }
    else
    {
        ZeroHour = 1;
    }
    ZeroHour++;
    setHibRetentionReg(ZeroHour);
    //UART_PRINT("ZeroHour: %i Retention Register: %i\n\r", ZeroHour, getHibRetentionReg());

    // Heartbeat message counter
    if(ZeroHour%30 == 0)
    {
        // every 15 minutes
        sendFlag = 1;
        ZeroHour = 0;
        //RGB_Color(YELLOW);
        //sleep(1);
        //RGB_Color(NONE);
    }

    // Keeping the green light on for 20 seconds when
    // closed message has been sent for successful parking
    if(greenCount > 0)
    {
        greenCount++;
        if(greenCount >= 20)
        {
            RGB_Color(NONE);
            greenCount = 0;
        }
    }

    sleep(1);
    two_runs++;

    // power management shut off if all the conditions are met
    // function Work() has gone twice through, not trying to send a message
    // and if the powerflag has been sent high
    if(two_runs >= 2)
    {
        if(sendFlag == 0)
        {
            if(powerFlag == 1)
            {
                status = sl_Stop(SL_STOP_TIMEOUT);
                retError(status, "SL_stop");
                powerShutdown(NOT_ACTIVE_DURATION_MSEC);
                powerFlag = 0;
                two_runs = 0;
            }
        }
    }
    return;
}

/*
#define RED 1
#define GREEN 2
#define BLUE 3
#define YELLOW 4
#define PURPLE 5
#define MARINE 6
#define ALL 7
#define NONE 8
 */

/*
 *          RGB_TestFunc()
 *
 * Test function to toggle through all of the RGB colors every half second
 * uses sleep function instead of timers with callback
 */

void RGB_TestFunc(void)
{
    // 1 Red
    RGB_Color(RED);
    sleep(1);
    // 2 Green
    RGB_Color(GREEN);
    sleep(1);
    // 3 Blue
    RGB_Color(BLUE);
    sleep(1);
    // 4 Yellow
    RGB_Color(YELLOW);
    sleep(1);
    // 5 Purple
    RGB_Color(PURPLE);
    sleep(1);
    // 6 Marine
    RGB_Color(MARINE);
    sleep(1);
     // 7 All
    RGB_Color(ALL);
    sleep(1);
    // 8 None
    RGB_Color(NONE);
    sleep(1);
    return;
}

/*
#define RED 1
#define GREEN 2
#define BLUE 3
#define YELLOW 4
#define PURPLE 5
#define MARINE 6
#define ALL 7
#define NONE 8
 */

/*
 *              RGB_Color()
 *
 * Sets the RGB LEDs to the color parameter sent in
 * For green (correct verification for the spot) the green LED is only for
 * 60 seconds to save power
 */

void RGB_Color(int color)
{
    switch(color)
    {
    case NONE:
        GPIO_write(Board_GPIO_R, RGB_OFF);
        GPIO_write(Board_GPIO_G, RGB_OFF);
        GPIO_write(Board_GPIO_B, RGB_OFF);

        GPIO_write(Board_GPIO_R1, RGB_OFF);
        GPIO_write(Board_GPIO_G1, RGB_OFF);
        GPIO_write(Board_GPIO_B1, RGB_OFF);

        if(safety_mode == 0)
        {
            GPIO_write(Board_GPIO_R2, RGB_OFF);
            GPIO_write(Board_GPIO_G2, RGB_OFF);
            GPIO_write(Board_GPIO_B2, RGB_OFF);
        }

        greenCount = 0;
        break;
    case ALL:
        GPIO_write(Board_GPIO_R, RGB_ON);
        GPIO_write(Board_GPIO_G, RGB_ON);
        GPIO_write(Board_GPIO_B, RGB_ON);

        GPIO_write(Board_GPIO_R1, RGB_ON);
        GPIO_write(Board_GPIO_G1, RGB_ON);
        GPIO_write(Board_GPIO_B1, RGB_ON);

        if(safety_mode == 0)
        {
            GPIO_write(Board_GPIO_R2, RGB_ON);
            GPIO_write(Board_GPIO_G2, RGB_ON);
            GPIO_write(Board_GPIO_B2, RGB_ON);
        }

        greenCount = 0;
        break;
    case RED:
        GPIO_write(Board_GPIO_R, RGB_ON);
        GPIO_write(Board_GPIO_G, RGB_OFF);
        GPIO_write(Board_GPIO_B, RGB_OFF);

        GPIO_write(Board_GPIO_R1, RGB_ON);
        GPIO_write(Board_GPIO_G1, RGB_OFF);
        GPIO_write(Board_GPIO_B1, RGB_OFF);

        if(safety_mode == 0)
        {
            GPIO_write(Board_GPIO_R2, RGB_ON);
            GPIO_write(Board_GPIO_G2, RGB_OFF);
            GPIO_write(Board_GPIO_B2, RGB_OFF);
        }

        greenCount = 0;
        break;
    case GREEN:
        GPIO_write(Board_GPIO_R, RGB_OFF);
        GPIO_write(Board_GPIO_G, RGB_ON);
        GPIO_write(Board_GPIO_B, RGB_OFF);

        GPIO_write(Board_GPIO_R1, RGB_OFF);
        GPIO_write(Board_GPIO_G1, RGB_ON);
        GPIO_write(Board_GPIO_B1, RGB_OFF);

        if(safety_mode == 0)
        {
            GPIO_write(Board_GPIO_R2, RGB_OFF);
            GPIO_write(Board_GPIO_G2, RGB_ON);
            GPIO_write(Board_GPIO_B2, RGB_OFF);

        }

        greenCount = 1;
        break;
    case BLUE:
        GPIO_write(Board_GPIO_R, RGB_OFF);
        GPIO_write(Board_GPIO_G, RGB_OFF);
        GPIO_write(Board_GPIO_B, RGB_ON);

        GPIO_write(Board_GPIO_R1, RGB_OFF);
        GPIO_write(Board_GPIO_G1, RGB_OFF);
        GPIO_write(Board_GPIO_B1, RGB_ON);

        if(safety_mode == 0)
        {
            GPIO_write(Board_GPIO_R2, RGB_OFF);
            GPIO_write(Board_GPIO_G2, RGB_OFF);
            GPIO_write(Board_GPIO_B2, RGB_ON);
        }

        greenCount = 0;
        break;
    case YELLOW:
        GPIO_write(Board_GPIO_R, RGB_ON);
        GPIO_write(Board_GPIO_G, RGB_ON);
        GPIO_write(Board_GPIO_B, RGB_OFF);

        GPIO_write(Board_GPIO_R1, RGB_ON);
        GPIO_write(Board_GPIO_G1, RGB_ON);
        GPIO_write(Board_GPIO_B1, RGB_OFF);

        if(safety_mode == 0)
        {
            GPIO_write(Board_GPIO_R2, RGB_ON);
            GPIO_write(Board_GPIO_G2, RGB_ON);
            GPIO_write(Board_GPIO_B2, RGB_OFF);
        }

        greenCount = 0;
        break;
    case PURPLE:
        GPIO_write(Board_GPIO_R, RGB_ON);
        GPIO_write(Board_GPIO_G, RGB_OFF);
        GPIO_write(Board_GPIO_B, RGB_ON);

        GPIO_write(Board_GPIO_R1, RGB_ON);
        GPIO_write(Board_GPIO_G1, RGB_OFF);
        GPIO_write(Board_GPIO_B1, RGB_ON);

        if(safety_mode == 0)
        {
            GPIO_write(Board_GPIO_R2, RGB_ON);
            GPIO_write(Board_GPIO_G2, RGB_OFF);
            GPIO_write(Board_GPIO_B2, RGB_ON);
        }

        greenCount = 0;
    case MARINE:
        GPIO_write(Board_GPIO_R, RGB_OFF);
        GPIO_write(Board_GPIO_G, RGB_ON);
        GPIO_write(Board_GPIO_B, RGB_ON);

        GPIO_write(Board_GPIO_R1, RGB_OFF);
        GPIO_write(Board_GPIO_G1, RGB_ON);
        GPIO_write(Board_GPIO_B1, RGB_ON);

        if(safety_mode == 0)
        {
            GPIO_write(Board_GPIO_R2, RGB_OFF);
            GPIO_write(Board_GPIO_G2, RGB_ON);
            GPIO_write(Board_GPIO_B2, RGB_ON);

        }

        greenCount = 0;
        break;
    default:
        UART_PRINT("Not a valid color name \n\r");
        greenCount = 0;
        break;
    }
    return;
}

/*
 *          Safety_Response()
 *
 * Response to message when safety flag is up
 * provides response when not dealing with sockets or wifi
 *
 */

void Safety_Response()
{
    char safemessage[10] = "";
    strcpy(safemessage, "");
    if(strcmp(ChangedBitSt, CHANGEDBIT_0) == 0)
    {
        strcpy(safemessage, ACK_SPACE);
    }
    else if(strcmp(StatusSt, BUSY_SPACE) == 0)
    {
        sleep(5);
        strcpy(safemessage, CLOSED_SPACE);
    }
    else if(strcmp(StatusSt, OPEN_SPACE) == 0)
    {
        strcpy(safemessage, OK_SPACE);
    }
    UART_PRINT("We got %s \n\r", safemessage);
    Check_Response(safemessage);
    return;
}
