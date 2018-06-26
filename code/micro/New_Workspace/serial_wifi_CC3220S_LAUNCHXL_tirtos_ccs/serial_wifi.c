/*
 * Copyright (c) 2016, Texas Instruments Incorporated
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

//*****************************************************************************
// includes
//*****************************************************************************
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

/* TI-DRIVERS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/Power.h>
#include <ti/net/atcmd/atcmd.h>
#include <ti/net/atcmd/atcmd_defs.h>
#include <ti/drivers/net/wifi/simplelink.h>

#include "uart_term.h"
#include "pthread.h"

//*****************************************************************************
// defines
//*****************************************************************************
#define SERIALWIFI_APPLICATION_NAME         "Serial WIFI"
#define SERIALWIFI_APPLICATION_VERSION      "1.0.0"
#define SERIALWIFI_SECURE_SOCKETS
#define SERIALWIFI_STACK_SIZE               (4096)
#define SERIALWIFI_CMD_BUFFER_SIZE          (1024)
#define SERIALWIFI_EVENT_BUFFER_SIZE        (1024)
#define SERIALWIFI_NET_BUFFER_SIZE          (1024)

#define SERIALWIFI_SECURITY_METHOD          SL_SO_SEC_METHOD_SSLV3
#define SERIALWIFI_SECURITY_CIPHER          SL_SEC_MASK_SSL_RSA_WITH_RC4_128_SHA
#define SERIALWIFI_CERT_FILE_NAME           "dummy-root-ca-cert"
#define SERIALWIFI_CERT_KEY_FILE_NAME       "dummy-trusted-cert-key"
#define SERIALWIFI_CHAIN_FILE_NAME          "trusted-chain.pem"

#define SERIALWIFI_PREFIX_LOCAL_CTRL        "\\\\<"
#define SERIALWIFI_PREFIX_REMOTE_CTRL       "\\\\>"
#define SERIALWIFI_PREFIX_SIZE              (3)

#define SERIALWIFI_IP_ACQUIRED_WAIT_SEC     (6)
#define SERIALWIFI_STOP_TIMEOUT             (200)
#define SERIALWIFI_PORT                     (224)
#define SERIALWIFI_TTL                      (2000)
#define SERIALWIFI_DNS_NAME                 "CC3220._uart._tcp.local"
#define SERIALWIFI_ROUTE_LOCAL              (1)
#define SERIALWIFI_ROUTE_REMOTE             (2)

#define SERIALWIFI_CLR_STATUS_BIT_ALL(status_variable)  (status_variable = 0)
#define SERIALWIFI_SET_STATUS_BIT(status_variable, bit) (status_variable |= (1<<(bit)))
#define SERIALWIFI_CLR_STATUS_BIT(status_variable, bit) (status_variable &= ~(1<<(bit)))
#define SERIALWIFI_GET_STATUS_BIT(status_variable, bit) (0 != (status_variable & (1<<(bit))))

#define SERIALWIFI_IS_CONNECTED(status_variable)        SERIALWIFI_GET_STATUS_BIT(status_variable,STATUS_BIT_CONNECTION)
#define SERIALWIFI_IS_IP_ACQUIRED(status_variable)      SERIALWIFI_GET_STATUS_BIT(status_variable,STATUS_BIT_IP_ACQUIRED)

#define SERIALWIFI_ASSERT_ON_ERROR(error_code)\
{\
     if(error_code < 0) \
       {\
            UART_PRINT("error %d\r\n",error_code);\
            return error_code;\
     }\
}

//*****************************************************************************
// typedefs
//*****************************************************************************
// Status bits - These are used to set/reset the corresponding bits in
// given variable

typedef enum _SerialWifi_Status_e_
{
    STATUS_BIT_NWP_INIT = 0,        // If this bit is set: Network Processor is
                                    // powered up
    STATUS_BIT_CONNECTION,          // If this bit is set: the device is connected to
                                    // the AP or client is connected to device (AP)
    STATUS_BIT_IP_LEASED,           // If this bit is set: the device has leased IP to
                                    // any connected client
    STATUS_BIT_IP_ACQUIRED,         // If this bit is set: the device has acquired an IP
    STATUS_BIT_SMARTCONFIG_START,   // If this bit is set: the SmartConfiguration
                                    // process is started from SmartConfig app
    STATUS_BIT_P2P_DEV_FOUND,       // If this bit is set: the device (P2P mode)
                                    // found any p2p-device in scan
    STATUS_BIT_P2P_REQ_RECEIVED,    // If this bit is set: the device (P2P mode)
                                    // found any p2p-negotiation request
    STATUS_BIT_CONNECTION_FAILED,   // If this bit is set: the device(P2P mode)
                                    // connection to client(or reverse way) is failed
    STATUS_BIT_PING_DONE,           // If this bit is set: the device has completed
                                    // the ping operation
    STATUS_BIT_IPV6L_ACQUIRED,      // If this bit is set: the device has acquired an IPv6 address
    STATUS_BIT_IPV6G_ACQUIRED,      // If this bit is set: the device has acquired an IPv6 address
    STATUS_BIT_AUTHENTICATION_FAILED,
    STATUS_BIT_RESET_REQUIRED,
}SerialWifi_Status_e;


/* Control block definition */
typedef struct _SerialWifi_CB_t_
{
    char        cmdBuffer[SERIALWIFI_CMD_BUFFER_SIZE];
    char        eventBuffer[SERIALWIFI_EVENT_BUFFER_SIZE];
    char        netBuffer[SERIALWIFI_NET_BUFFER_SIZE];
    int16_t     socket;
    int16_t     sockTcpServer;
    uint32_t    status;
    uint8_t     configurationDone;
    uint8_t     tcpConnected;
    uint8_t     route;
}SerialWifi_CB_t;

//****************************************************************************
// GLOBAL VARIABLES
//****************************************************************************

SerialWifi_CB_t SerialWifi_CB;

//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************

//*****************************************************************************
//
//! \brief The Function Handles WLAN Events
//!
//! \param[in]  pWlanEvent - Pointer to WLAN Event Info
//!
//! \return None
//!
//*****************************************************************************
int32_t SerialWifi_wlanEventHandler(SlWlanEvent_t *pWlanEvent)
{
    if(pWlanEvent == NULL)
    {
        return EVENT_PROPAGATION_BLOCK;
    }

     switch(pWlanEvent->Id)
     {
        case SL_WLAN_EVENT_CONNECT:
            SERIALWIFI_SET_STATUS_BIT(SerialWifi_CB.status, STATUS_BIT_CONNECTION);

            UART_PRINT("STA Connected to AP: %s , BSSID: %x:%x:%x:%x:%x:%x\n\r", pWlanEvent->Data.Connect.SsidName, pWlanEvent->Data.Connect.Bssid[0], pWlanEvent->Data.Connect.Bssid[1],
                       pWlanEvent->Data.Connect.Bssid[2], pWlanEvent->Data.Connect.Bssid[3], pWlanEvent->Data.Connect.Bssid[4], pWlanEvent->Data.Connect.Bssid[5]);
            break;

        case SL_WLAN_EVENT_DISCONNECT:
            SERIALWIFI_CLR_STATUS_BIT(SerialWifi_CB.status, STATUS_BIT_CONNECTION);
            SERIALWIFI_CLR_STATUS_BIT(SerialWifi_CB.status, STATUS_BIT_IP_ACQUIRED);

            break;

        case SL_WLAN_EVENT_STA_ADDED:
            /* when device is in AP mode and any client connects to it.       */
            SERIALWIFI_SET_STATUS_BIT(SerialWifi_CB.status, STATUS_BIT_CONNECTION);
            break;

        case SL_WLAN_EVENT_STA_REMOVED:
            /* when device is in AP mode and any client disconnects from it.  */
            SERIALWIFI_CLR_STATUS_BIT(SerialWifi_CB.status, STATUS_BIT_CONNECTION);
            SERIALWIFI_CLR_STATUS_BIT(SerialWifi_CB.status, STATUS_BIT_IP_LEASED);
            break;

        default:
            break;
    }
    return EVENT_PROPAGATION_BLOCK;
}

//*****************************************************************************
//
//! \brief The Function Handles the Fatal errors
//!
//! \param[in]  slFatalErrorEvent - Pointer to Fatal Error Event info
//!
//! \return None
//!
//*****************************************************************************
int32_t SerialWifi_fatalErrorEventHandler(SlDeviceFatal_t *slFatalErrorEvent)
{
    return EVENT_PROPAGATION_BLOCK;
}

//*****************************************************************************
//
//! \brief This function handles network events such as IP acquisition, IP
//!           leased, IP released etc.
//!
//! \param[in]  pNetAppEvent - Pointer to NetApp Event Info
//!
//! \return None
//!
//*****************************************************************************
int32_t SerialWifi_netAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
    if(pNetAppEvent == NULL)
    {
        return EVENT_PROPAGATION_BLOCK;
    }

    switch(pNetAppEvent->Id)
    {
        case SL_NETAPP_EVENT_IPV4_ACQUIRED:
            SERIALWIFI_SET_STATUS_BIT(SerialWifi_CB.status, STATUS_BIT_IP_ACQUIRED);
            UART_PRINT("IPv4 acquired: IP = %d.%d.%d.%d\n\r",\
                (uint8_t)SL_IPV4_BYTE(pNetAppEvent->Data.IpAcquiredV4.Ip,3),\
                (uint8_t)SL_IPV4_BYTE(pNetAppEvent->Data.IpAcquiredV4.Ip,2),\
                (uint8_t)SL_IPV4_BYTE(pNetAppEvent->Data.IpAcquiredV4.Ip,1),\
                (uint8_t)SL_IPV4_BYTE(pNetAppEvent->Data.IpAcquiredV4.Ip,0));
            break;

        case SL_NETAPP_EVENT_DHCPV4_LEASED:
            SERIALWIFI_SET_STATUS_BIT(SerialWifi_CB.status, STATUS_BIT_IP_LEASED);
            break;

        case SL_NETAPP_EVENT_DHCPV4_RELEASED:
            SERIALWIFI_CLR_STATUS_BIT(SerialWifi_CB.status, STATUS_BIT_IP_LEASED);
            break;

        default:
            break;
   }
    return EVENT_PROPAGATION_BLOCK;
}


//*****************************************************************************
//
//! \brief This function handles General Events
//!
//! \param[in]     pDevEvent - Pointer to General Event Info
//!
//! \return None
//!
//*****************************************************************************
int32_t SerialWifi_generalEventHandler(SlDeviceEvent_t *pDevEvent)
{
    return EVENT_PROPAGATION_BLOCK;
}

//*****************************************************************************
//
//! This function handles socket events indication
//!
//! \param[in]      pSock - Pointer to Socket Event Info
//!
//! \return None
//!
//*****************************************************************************
int32_t SerialWifi_sockEventHandler(SlSockEvent_t *pSock)
{
    return EVENT_PROPAGATION_BLOCK;
}

//*****************************************************************************
//
//! This function register Host events
//!
//! \param[in]      doRegister - 1 register, 0 unregister
//!
//! \return None
//!
//*****************************************************************************
void SerialWifi_registerHostEvents(uint8_t doRegister)
{
    if (doRegister)
    {
        sl_RegisterEventHandler(SL_EVENT_HDL_WLAN , (void *)SerialWifi_wlanEventHandler);
        sl_RegisterEventHandler(SL_EVENT_HDL_FATAL_ERROR , (void *)SerialWifi_fatalErrorEventHandler);
        sl_RegisterEventHandler(SL_EVENT_HDL_NETAPP , (void *)SerialWifi_netAppEventHandler);
        sl_RegisterEventHandler(SL_EVENT_HDL_DEVICE_GENERAL , (void *)SerialWifi_generalEventHandler);
        sl_RegisterEventHandler(SL_EVENT_HDL_SOCKET , (void *)SerialWifi_sockEventHandler);
    }
    else
    {
        sl_RegisterEventHandler(SL_EVENT_HDL_WLAN , NULL);
        sl_RegisterEventHandler(SL_EVENT_HDL_FATAL_ERROR , NULL);
        sl_RegisterEventHandler(SL_EVENT_HDL_NETAPP , NULL);
        sl_RegisterEventHandler(SL_EVENT_HDL_DEVICE_GENERAL , NULL);
        sl_RegisterEventHandler(SL_EVENT_HDL_SOCKET , NULL);
    }
}
//*****************************************************************************
//
//! SerialWifi_initDevice
//!
//! @param  none
//!
//! @return 0 on success else error code
//!
//! @brief  Initialise the device to default state without deleting the stored
//!            profiles.
//
//*****************************************************************************

int32_t SerialWifi_initDevice(void)
{
    uint8_t val = 1;
    uint8_t configOpt = 0;
    uint8_t power = 0;

    int32_t retVal = -1;
    int32_t mode = -1;

    mode = sl_Start(0, 0, 0);
    SERIALWIFI_ASSERT_ON_ERROR(mode);

    // Check if the device is not in station-mode
    if (ROLE_STA != mode)
    {
        if (ROLE_AP == mode)
        {
            // If the device is in AP mode, we need to wait for this
            // event before doing anything
            while(!SERIALWIFI_IS_IP_ACQUIRED(SerialWifi_CB.status))
            {
            }
        }
    }
    else
    {
        // Set connection policy to Auto + Auto Provisisoning 
        // (Device's default connection policy)
        retVal = sl_WlanPolicySet(SL_WLAN_POLICY_CONNECTION,SL_WLAN_CONNECTION_POLICY(1, 0, 0, 1), NULL, 0);
        SERIALWIFI_ASSERT_ON_ERROR(retVal);

        // Device in station-mode. Disconnect previous connection if any
        // The function returns 0 if 'Disconnected done', negative number if already disconnected
        // Wait for 'disconnection' event if 0 is returned, Ignore other return-codes
        //
        retVal = sl_WlanDisconnect();
        if(0 == retVal)
        {
            // Wait
            while(SERIALWIFI_IS_CONNECTED(SerialWifi_CB.status))
            {
            }
        }

        // Enable DHCP client
        retVal = sl_NetCfgSet(SL_NETCFG_IPV4_STA_ADDR_MODE,1,1,&val);
        SERIALWIFI_ASSERT_ON_ERROR(retVal);

        // Disable scan
        configOpt = SL_WLAN_SCAN_POLICY(0, 0);
        retVal = sl_WlanPolicySet(SL_WLAN_POLICY_SCAN , configOpt, NULL, 0);
        SERIALWIFI_ASSERT_ON_ERROR(retVal);

        // Set Tx power level for station mode
        // Number between 0-15, as dB offset from max power - 0 will set maximum power
        power = 0;
        retVal = sl_WlanSet(SL_WLAN_CFG_GENERAL_PARAM_ID, SL_WLAN_GENERAL_PARAM_OPT_STA_TX_POWER, 1,(uint8_t *)&power);
        SERIALWIFI_ASSERT_ON_ERROR(retVal);

        // Set PM policy to normal
        retVal = sl_WlanPolicySet(SL_WLAN_POLICY_PM , SL_WLAN_NORMAL_POLICY, NULL, 0);
        SERIALWIFI_ASSERT_ON_ERROR(retVal);

        retVal = sl_Stop(SERIALWIFI_STOP_TIMEOUT);
        SERIALWIFI_ASSERT_ON_ERROR(retVal);

        SERIALWIFI_CLR_STATUS_BIT_ALL(SerialWifi_CB.status);

        //
        // Assumption is that the device is configured in station mode already
        // and it is in its default state
        //
        mode = sl_Start(0,0,0);

        if (mode < 0 || mode != ROLE_STA)
        {
            UART_PRINT("Failed to start the device \n\r");
            SERIALWIFI_ASSERT_ON_ERROR(mode);
        }
    }
    return mode;
}


//*****************************************************************************
//
//! SerialWifi_startTcpServer
//!
//! @param  none
//!
//! @return 0 on success else error code
//!
//! @brief  Start Tcp Server socket
//
//*****************************************************************************
int32_t SerialWifi_startTcpServer(void)
{
    SlSockAddr_t localAddr;
    uint32_t nonBlocking;
    int32_t retVal;
    uint8_t secMethod;
    uint32_t secCipher;


    localAddr.sa_family = SL_AF_INET;
    localAddr.sa_data[0] = ((SERIALWIFI_PORT >> 8) & 0xFF);
    localAddr.sa_data[1] = (SERIALWIFI_PORT & 0xFF);

    //all 0 => Own IP address
    memset(&localAddr.sa_data[2], 0, 4);

#ifndef SERIALWIFI_SECURE_SOCKETS
    SerialWifi_CB.sockTcpServer = sl_Socket(SL_AF_INET, SL_SOCK_STREAM, SL_IPPROTO_TCP);

#else
    SerialWifi_CB.sockTcpServer= sl_Socket(SL_AF_INET, SL_SOCK_STREAM,SL_SEC_SOCKET);
    if( SerialWifi_CB.sockTcpServer < 0 )
    {
       UART_PRINT("Failed to create socket\r\n");
       return -1;
    }

    //
    // configure the socket as SSLV3.0
    //
    secMethod = SERIALWIFI_SECURITY_METHOD;
    retVal = sl_SetSockOpt(SerialWifi_CB.sockTcpServer, SL_SOL_SOCKET, SL_SO_SECMETHOD, &secMethod, sizeof(secMethod));

    if( retVal < 0 )
    {
        UART_PRINT("Failed to set socket options 16\r\n");
        sl_Close(SerialWifi_CB.sockTcpServer);
        return retVal;
    }

    //
    //configure the socket as RSA with RC4 128 SHA
    //
    secCipher = SERIALWIFI_SECURITY_CIPHER;
    retVal = sl_SetSockOpt(SerialWifi_CB.sockTcpServer, SL_SOL_SOCKET, SL_SO_SECURE_MASK, &secCipher, sizeof(secCipher));

    if( retVal < 0 )
    {
        UART_PRINT("Failed to set socket options 17\r\n");
        sl_Close(SerialWifi_CB.sockTcpServer);
        return retVal;
    }

    //
    //configure the socket with CA certificate - for server verification
    //
    retVal = sl_SetSockOpt( SerialWifi_CB.sockTcpServer, SL_SOL_SOCKET,SL_SO_SECURE_FILES_PRIVATE_KEY_FILE_NAME,
                            SERIALWIFI_CERT_KEY_FILE_NAME, strlen(SERIALWIFI_CERT_KEY_FILE_NAME));
    if( retVal < 0 )
    {
        UART_PRINT("Failed to set socket options 18\r\n");
        sl_Close(SerialWifi_CB.sockTcpServer);
        return retVal;
    }

    retVal = sl_SetSockOpt( SerialWifi_CB.sockTcpServer, SL_SOL_SOCKET, SL_SO_SECURE_FILES_CERTIFICATE_FILE_NAME,
                            SERIALWIFI_CHAIN_FILE_NAME, strlen(SERIALWIFI_CHAIN_FILE_NAME));
    if( retVal < 0 )
    {
        UART_PRINT("Failed to set socket options 19\r\n");
        sl_Close(SerialWifi_CB.sockTcpServer);
        return retVal;
    }

    retVal = sl_SetSockOpt( SerialWifi_CB.sockTcpServer, SL_SOL_SOCKET, SL_SO_SECURE_FILES_CA_FILE_NAME,
                            SERIALWIFI_CERT_FILE_NAME, strlen(SERIALWIFI_CERT_FILE_NAME));
    if( retVal < 0 )
    {
        UART_PRINT("Failed to set socket options 20\r\n");
        sl_Close(SerialWifi_CB.sockTcpServer);
        return retVal;
    }
#endif
    nonBlocking = 1;
    retVal = sl_SetSockOpt(SerialWifi_CB.sockTcpServer, SL_SOL_SOCKET,SL_SO_NONBLOCKING, &nonBlocking,sizeof(nonBlocking));
    if( retVal < 0 )
    {
        UART_PRINT("Failed to set socket options 21\r\n");
        sl_Close(SerialWifi_CB.sockTcpServer);
        return -1;
    }

    if (sl_Bind(SerialWifi_CB.sockTcpServer, &localAddr, sizeof(localAddr)) < 0)
    {
        UART_PRINT(" Bind Error\n\r");
        sl_Close(SerialWifi_CB.sockTcpServer);
        return -1;
    }

    if (sl_Listen(SerialWifi_CB.sockTcpServer, 0) < 0)
    {
        UART_PRINT(" Listen Error\n\r");
        sl_Close(SerialWifi_CB.sockTcpServer);
        return -1;
    }

    return 0;
}

//*****************************************************************************
//
//! \brief Display Application Banner
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
int32_t SerialWifi_displayBanner(void)
{
    int32_t     status = -1;
    uint8_t     macAddress[SL_MAC_ADDR_LEN];
    uint16_t    macAddressLen = SL_MAC_ADDR_LEN;
    uint16_t    configSize = 0;
    uint8_t     configOpt = SL_DEVICE_GENERAL_VERSION;
    SlDeviceVersion_t ver = {0};

    configSize = sizeof(SlDeviceVersion_t);
    status = sl_Start(0, 0, 0);
    if (status < 0)
    {
        return -1;
    }

    /* Print device version info. */
    status = sl_DeviceGet(SL_DEVICE_GENERAL, &configOpt, &configSize, (uint8_t*)(&ver));
    if (status < 0)
    {
        return -1;
    }

    /* Print device Mac address */
    status = sl_NetCfgGet(SL_NETCFG_MAC_ADDRESS_GET, 0, &macAddressLen, &macAddress[0]);
    if (status < 0)
    {
        return -1;
    }

    UART_PRINT("\n\n\n\r");
    UART_PRINT("\t ==============================================\n\r");
    UART_PRINT("\t    %s Example Ver: %s\n\r",SERIALWIFI_APPLICATION_NAME, SERIALWIFI_APPLICATION_VERSION);
    UART_PRINT("\t ==============================================\n\r");
    UART_PRINT("\n\r");
    UART_PRINT("\t CHIP: 0x%x",ver.ChipId);
    UART_PRINT("\n\r");
    UART_PRINT("\t MAC:  %d.%d.%d.%d",ver.FwVersion[0],ver.FwVersion[1],ver.FwVersion[2],ver.FwVersion[3]);
    UART_PRINT("\n\r");
    UART_PRINT("\t PHY:  %d.%d.%d.%d",ver.PhyVersion[0],ver.PhyVersion[1],ver.PhyVersion[2],ver.PhyVersion[3]);
    UART_PRINT("\n\r");
    UART_PRINT("\t NWP:  %d.%d.%d.%d",ver.NwpVersion[0],ver.NwpVersion[1],ver.NwpVersion[2],ver.NwpVersion[3]);
    UART_PRINT("\n\r");
    UART_PRINT("\t ROM:  %d",ver.RomVersion);
    UART_PRINT("\n\r");
    UART_PRINT("\t HOST: %s", SL_DRIVER_VERSION);
    UART_PRINT("\n\r");
    UART_PRINT("\t MAC address: %02x:%02x:%02x:%02x:%02x:%02x", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
    UART_PRINT("\n\r");
    UART_PRINT("\n\r");
    UART_PRINT("\t ==============================================\n\r");
    UART_PRINT("\n\r");
    UART_PRINT("\n\r");
    status = sl_Stop(SERIALWIFI_STOP_TIMEOUT);
    if (status < 0)
    {
        return -1;
    }

    return status;
}

//*****************************************************************************
//
//! SerialWifi_connectProcess
//!
//! @param  none
//!
//! @return 0 on success else error code
//!
//! @brief  Register mDNS service and try to establish secure TCP connection 
//!         as server or client.
//
//*****************************************************************************
void *SerialWifi_connectProcess(void *pvParameters)
{
    int32_t retVal;
    int8_t text[SL_NETAPP_MAX_DEVICE_URN_LEN];
    uint8_t textLen = SL_NETAPP_MAX_DEVICE_URN_LEN;
    uint32_t port;
    uint32_t addr;
    uint32_t nonBlocking;
    int16_t socketChild;
    SlSocklen_t addrLen;
    uint32_t rand_num;
    uint8_t seed;
    uint16_t seedLen = 1;
    uint8_t secMethod;
    uint32_t secCipher;
    SlSockAddrIn_t remoteAddr;
    uint32_t    i = 0;
    int32_t mode;
    uint8_t ssid[33];
    uint16_t len = 33;
    uint16_t config_opt = SL_WLAN_AP_OPT_SSID;

    mode = SerialWifi_initDevice();
    if (mode == ROLE_STA)
    {     
        UART_PRINT("Please wait...trying to connect to the AP\n\r");
        UART_PRINT("\n\r");
        retVal = sl_WlanPolicySet(SL_WLAN_POLICY_CONNECTION,SL_WLAN_CONNECTION_POLICY(1, 0, 0, 1), NULL, 0);
        while ((!SERIALWIFI_IS_IP_ACQUIRED(SerialWifi_CB.status)) && (i<SERIALWIFI_IP_ACQUIRED_WAIT_SEC))
        {
            sleep(1);
            i++;
        }

        if (!SERIALWIFI_IS_IP_ACQUIRED(SerialWifi_CB.status))
        {
            UART_PRINT("Please connect to your AP by AT WLAN connect command\n\r");
            UART_PRINT("For example: %sat%s=TP-LINK_123456,,open,,,,\n\r",SERIALWIFI_PREFIX_LOCAL_CTRL,ATCmd_wlanConnectStr);

            UART_PRINT("Or for auto connect use the AT WLAN add profile command\n\r");
            UART_PRINT("For example: %sat%s=TP-LINK_123456,,open,,,,,6\n\r",SERIALWIFI_PREFIX_LOCAL_CTRL,ATCmd_wlanProfileAddStr);
        }
    }
    else if (mode == ROLE_AP)
    {
		sl_Memset(ssid,0,33);
        sl_WlanGet(SL_WLAN_CFG_AP_ID, &config_opt , &len, ssid);
        UART_PRINT("AP SSID %s\n\r",ssid);        
    }
    else if (mode < 0)
    {
        return NULL;
    }

    while (1)
    {
        if ((!SERIALWIFI_IS_IP_ACQUIRED(SerialWifi_CB.status)) || (SerialWifi_CB.tcpConnected == 1))
        {
            sleep(1);
            continue;
        }

        if (SerialWifi_CB.configurationDone == 0)
        {
            //start Interpreter Mode
            retVal = SerialWifi_startTcpServer();
            if( retVal < 0 )
            {
                UART_PRINT("Failed to start tcp server\n\r");
                continue;
            }
            sl_NetAppGet(SL_NETAPP_DEVICE_ID, SL_NETAPP_DEVICE_URN,&textLen, (uint8_t *)text);

            retVal = sl_NetAppMDNSRegisterService((const int8_t *)SERIALWIFI_DNS_NAME,(uint8_t)strlen(SERIALWIFI_DNS_NAME),
                                                  text,textLen,SERIALWIFI_PORT,SERIALWIFI_TTL,SL_NETAPP_MDNS_OPTIONS_IS_UNIQUE_BIT|SL_NETAPP_MDNS_OPTIONS_IS_NOT_PERSISTENT);
            if( retVal < 0 )
            {
                UART_PRINT("Failed to register service\n\r");
                continue;
            }

            sl_NetUtilGet(SL_NETUTIL_TRUE_RANDOM, 0, &seed, &seedLen);
            srand(seed);
            SerialWifi_CB.configurationDone = 1;
        }

        if (SerialWifi_CB.tcpConnected == 0)
        {
            rand_num = (rand() % 8);
            sleep(rand_num);

            // if no  TCP connection, server should still be active -> check for
            // incoming requests
            addrLen = sizeof(SlSockAddrIn_t);
            socketChild = sl_Accept(SerialWifi_CB.sockTcpServer, (SlSockAddr_t *)&remoteAddr,&addrLen);

            if (socketChild >= 0)
            {
                SerialWifi_CB.tcpConnected = 1;
                //always work with one active socket
                SerialWifi_CB.socket = socketChild;

                UART_PRINT("TCP server connection established with %d.%d.%d.%d\n\r",
                    (remoteAddr.sin_addr.s_addr & (0x000000FF)) >> 0, (remoteAddr.sin_addr.s_addr & (0x0000FF00)) >> 8,
                    (remoteAddr.sin_addr.s_addr & (0x00FF0000)) >> 16, (remoteAddr.sin_addr.s_addr & (0xFF000000)) >> 24);
            }
        }

        if (SerialWifi_CB.tcpConnected == 0)
        {
            retVal = sl_NetAppDnsGetHostByService(
                           (int8_t *)SERIALWIFI_DNS_NAME,
                           (uint8_t)strlen(SERIALWIFI_DNS_NAME),
                           SL_AF_INET,(_u32 *)&addr,
                           (_u32 *)&port,
                           (_u16 *)&textLen,text);

            if(retVal == 0)
            {

                remoteAddr.sin_port = sl_Htons((uint16_t)port);
                remoteAddr.sin_addr.s_addr = sl_Htonl(addr);
                remoteAddr.sin_family = SL_AF_INET;


        #ifndef SERIALWIFI_SECURE_SOCKETS
                SerialWifi_CB.socket = sl_Socket(SL_AF_INET, SL_SOCK_STREAM, SL_IPPROTO_TCP);
        #else
                SerialWifi_CB.socket = sl_Socket(SL_AF_INET, SL_SOCK_STREAM, SL_SEC_SOCKET);

                if( SerialWifi_CB.socket < 0 )
                {
                    UART_PRINT("Failed to create socket\n\r");
                    continue;
                }
                //
                // configure the socket as SSLV3.0
                //
                secMethod = SERIALWIFI_SECURITY_METHOD;
                retVal = sl_SetSockOpt(SerialWifi_CB.socket, SL_SOL_SOCKET, \
                                           SL_SO_SECMETHOD, &secMethod, \
                                           sizeof(secMethod));
                if( retVal < 0 )
                {
                    UART_PRINT("Failed to set socket options 1\n\r");
                    sl_Close(SerialWifi_CB.socket);
                    continue;
                }

                //
                //configure the socket as RSA with RC4 128 SHA
                //
                secCipher = SERIALWIFI_SECURITY_CIPHER;
                retVal = sl_SetSockOpt(SerialWifi_CB.socket, SL_SOL_SOCKET, \
                                        SL_SO_SECURE_MASK, &secCipher, \
                                        sizeof(secCipher));
                if( retVal < 0 )
                {
                    UART_PRINT("Failed to set socket options 2\n\r");
                    sl_Close(SerialWifi_CB.socket);
                    continue;
                }
                retVal = sl_SetSockOpt(SerialWifi_CB.socket, SL_SOL_SOCKET, \
                                      SL_SO_SECURE_FILES_PRIVATE_KEY_FILE_NAME,\
                                      SERIALWIFI_CERT_KEY_FILE_NAME, \
                                      strlen(SERIALWIFI_CERT_KEY_FILE_NAME));
                if( retVal < 0 )
                {
                    UART_PRINT("Failed to set socket options 3\r\n");
                    sl_Close(SerialWifi_CB.socket);
                    continue;
                }

                retVal = sl_SetSockOpt(SerialWifi_CB.socket, SL_SOL_SOCKET, \
                                     SL_SO_SECURE_FILES_CERTIFICATE_FILE_NAME,\
                                     SERIALWIFI_CHAIN_FILE_NAME, \
                                     strlen(SERIALWIFI_CHAIN_FILE_NAME));
                if( retVal < 0 )
                {
                    UART_PRINT("Failed to set socket options 4\r\n");
                    sl_Close(SerialWifi_CB.socket);
                    continue;
                }

                retVal = sl_SetSockOpt(SerialWifi_CB.socket, SL_SOL_SOCKET, \
                                       SL_SO_SECURE_FILES_CA_FILE_NAME, \
                                       SERIALWIFI_CERT_FILE_NAME, \
                                       strlen(SERIALWIFI_CERT_FILE_NAME));
                if( retVal < 0 )
                {
                    UART_PRINT("Failed to set socket options 5\r\n");
                    sl_Close(SerialWifi_CB.socket);
                    continue;
                }

        #endif

                retVal = sl_Connect(SerialWifi_CB.socket, (SlSockAddr_t *)&remoteAddr,
                                        sizeof(SlSockAddr_t));

                if((retVal >= 0) || (retVal == SL_ERROR_BSD_ESECDATEERROR))
                {
                    nonBlocking = 1;
                    retVal = sl_SetSockOpt(SerialWifi_CB.socket, SL_SOL_SOCKET,
                                         SL_SO_NONBLOCKING,
                                         &nonBlocking, sizeof(nonBlocking));
                    if( retVal < 0 )
                    {
                        UART_PRINT("Failed to set socket options 7\r\n");
                        sl_Close(SerialWifi_CB.socket);
                        continue;
                    }

                    SerialWifi_CB.tcpConnected = 1;

                    UART_PRINT("TCP connection established with %d.%d.%d.%d\n\r",
                        (remoteAddr.sin_addr.s_addr & (0x000000FF)) >> 0, (remoteAddr.sin_addr.s_addr & (0x0000FF00)) >> 8,
                        (remoteAddr.sin_addr.s_addr & (0x00FF0000)) >> 16, (remoteAddr.sin_addr.s_addr & (0xFF000000)) >> 24);
                }
                else
                {
                    sl_Close(SerialWifi_CB.socket);
                }
            }
            else
            {
                //UART_PRINT("Failed to find peer\r\n");
                sl_NetAppGet(SL_NETAPP_DEVICE_ID, SL_NETAPP_DEVICE_URN,&textLen, (uint8_t *)text);
                retVal = sl_NetAppMDNSUnRegisterService((const int8_t *)SERIALWIFI_DNS_NAME,(uint8_t)strlen(SERIALWIFI_DNS_NAME), 0);

                retVal = sl_NetAppMDNSRegisterService((const int8_t *)SERIALWIFI_DNS_NAME,(uint8_t)strlen(SERIALWIFI_DNS_NAME),
                                                  text,textLen,SERIALWIFI_PORT,SERIALWIFI_TTL,SL_NETAPP_MDNS_OPTIONS_IS_UNIQUE_BIT|SL_NETAPP_MDNS_OPTIONS_IS_NOT_PERSISTENT);
                if( retVal < 0 )
                {
                    UART_PRINT("Failed to register service\n\r");
                    continue;
                }
            }
        }
    }
}

//*****************************************************************************
//
//! \brief SerialWifi recv message over TCP
//!
//! \param  buffer - Points to data
//!
//! \return number of received data
//!
//*****************************************************************************
int32_t SerialWifi_recv(char *buffer)
{
    int32_t bytesRx = 0;

    if (SerialWifi_CB.socket != -1)
    {
        //TCP
        if (SerialWifi_CB.tcpConnected == 1)
        {
            bytesRx = sl_Recv(SerialWifi_CB.socket, buffer,SERIALWIFI_NET_BUFFER_SIZE, 0);
        }
    }
    
    return bytesRx;
}
//*****************************************************************************
//
//! \brief SerialWifi send message over TCP
//!
//! \param  buffer - pointer to data
//!
//! \return number of sent bytes
//!
//*****************************************************************************
int32_t SerialWifi_send(char *buffer)
{
    int32_t bytesTx = 0;

    if (SerialWifi_CB.socket != -1)
    {
        if (SerialWifi_CB.tcpConnected == 1)
        {
            bytesTx = sl_Send(SerialWifi_CB.socket, buffer, strlen(buffer), 0);
            if (bytesTx == -1)
            {
                sl_Close(SerialWifi_CB.socket);
                SerialWifi_CB.socket = -1;
                SerialWifi_CB.tcpConnected = 0;
            }
        }
    }
    return bytesTx;
}
//*****************************************************************************
//
//! \brief  control AT event and add prefix to the event
//!
//! \param  buffer - pointer to data
//!
//! \return none
//!
//*****************************************************************************
int32_t SerialWifi_controlEvents(char *buffer)
{
    uint32_t eventLen;
    int32_t i;

    // add prefix
    eventLen = strlen(buffer);
    for (i=eventLen;i >= 0;i--)
    {
        buffer[i + SERIALWIFI_PREFIX_SIZE] =  buffer[i];
    }

    if (SerialWifi_CB.route == SERIALWIFI_ROUTE_LOCAL)
    {
        strncpy(buffer,SERIALWIFI_PREFIX_LOCAL_CTRL,SERIALWIFI_PREFIX_SIZE);
    }
    else if (SerialWifi_CB.route == SERIALWIFI_ROUTE_REMOTE)
    {
        strncpy(buffer,SERIALWIFI_PREFIX_REMOTE_CTRL,SERIALWIFI_PREFIX_SIZE);
    }

    return 0;
}

//*****************************************************************************
//
//! \brief  control AT commands
//!
//! \param  buffer - pointer to data
//!
//! \return none
//!
//*****************************************************************************
int32_t SerialWifi_controlCommands(char *buffer)
{
    if (strstr(buffer, "+wlan"))
    {
        return -1;

    }
 
    return 0;
}

//*****************************************************************************
//
//! \brief  Send data over network or over serial
//!
//! \param  buffer - pointer to data
//!         route -  source data (local or remote)
//!
//! \return none
//!
//*****************************************************************************
int32_t SerialWifi_forwardData(char *buffer,uint8_t route)
{
    //local control mode
    if (strncmp(buffer, SERIALWIFI_PREFIX_LOCAL_CTRL,SERIALWIFI_PREFIX_SIZE) == 0)
    {
        if (((buffer[SERIALWIFI_PREFIX_SIZE] == 'a') || (buffer[SERIALWIFI_PREFIX_SIZE] == 'A')) &&
           ((buffer[SERIALWIFI_PREFIX_SIZE + 1] == 't') || (buffer[SERIALWIFI_PREFIX_SIZE + 1] == 'T')))
        {
            SerialWifi_controlCommands(buffer + SERIALWIFI_PREFIX_SIZE);
            SerialWifi_CB.route = SERIALWIFI_ROUTE_LOCAL;
            ATCmd_send(buffer + SERIALWIFI_PREFIX_SIZE);
        }
        else
        {
            UART_PRINT("%s\n\r",buffer);
        }
    }
    //remote control mode
    else if (strncmp(buffer, SERIALWIFI_PREFIX_REMOTE_CTRL,SERIALWIFI_PREFIX_SIZE) == 0)
    {
        // send remote control command
        if (route == SERIALWIFI_ROUTE_LOCAL)
        {
            SerialWifi_send(buffer);
        }
        else if (route == SERIALWIFI_ROUTE_REMOTE)
        {
           if (((buffer[SERIALWIFI_PREFIX_SIZE] == 'a') || (buffer[SERIALWIFI_PREFIX_SIZE] == 'A')) &&
               ((buffer[SERIALWIFI_PREFIX_SIZE + 1] == 't') || (buffer[SERIALWIFI_PREFIX_SIZE + 1] == 'T')))
            {
                if (SerialWifi_controlCommands(buffer + SERIALWIFI_PREFIX_SIZE) == 0)
                {
                    SerialWifi_CB.route = SERIALWIFI_ROUTE_REMOTE;
                    ATCmd_send(buffer + SERIALWIFI_PREFIX_SIZE);
                }
            }
           else
           {
               UART_PRINT("%s\n\r",buffer);
           }
        }
    }
    // terminal mode
    else
    {
        // send terminal string
        if (route == SERIALWIFI_ROUTE_LOCAL)
        {
            SerialWifi_send(buffer);
        }
        // receive terminal string
        else if (route == SERIALWIFI_ROUTE_REMOTE)
        {
            UART_PRINT("%s\n\r",buffer);
        }
    }          
    //UART_PRINT("\n\r");

    return (0);
}


//*****************************************************************************
//
//!  SerialWifi_cmdProcess
//!
//!  \param  void
//!
//!  \return none
//!
//!  \brief read from UART and forward the data
//
//*****************************************************************************
int32_t SerialWifi_cmdProcess(void)
{
    int32_t     retVal;
    uint32_t    i = 1;

    while(i)
    {
        usleep(1000);
        /* Poll UART terminal to receive user command terminated by '/r' */
        retVal = GetCmd(SerialWifi_CB.cmdBuffer, SERIALWIFI_CMD_BUFFER_SIZE);
        if (retVal <= 0)
        {
            UART_PRINT("\n\r");
            continue;
        }
        UART_PRINT("\n\r");
        SerialWifi_forwardData(SerialWifi_CB.cmdBuffer,SERIALWIFI_ROUTE_LOCAL);
    }
    return 0 ;
}

//
//*****************************************************************************
//
//!  SerialWifi_eventProcess
//!
//!  \param  pvParameters
//!
//!  \return none
//!
//!  \brief   Get AT event and get network data
//
//*****************************************************************************

void *SerialWifi_eventProcess(void *pvParameters)
{
    int32_t status;

    while(1)
    {
        usleep(1000);
        /* check for receive local AT command event */
        status = ATCmd_recv(SerialWifi_CB.eventBuffer,1);
        if (status >= 0)
        {
            SerialWifi_controlEvents(SerialWifi_CB.eventBuffer);
            SerialWifi_forwardData(SerialWifi_CB.eventBuffer,SERIALWIFI_ROUTE_LOCAL);
        }

        /* check for receive message over the net */
        status = SerialWifi_recv(SerialWifi_CB.netBuffer);
        if (status > 0)
        {
            /* Set NULL in the end of string */
            SerialWifi_CB.netBuffer[status] = 0;
            SerialWifi_forwardData(SerialWifi_CB.netBuffer,SERIALWIFI_ROUTE_REMOTE);
        }
    }
}

//*****************************************************************************
//
//! mainThread
//!
//!  \param  pvParameters
//!
//!  \return none
//!
//!  \brief Task handler
//
//*****************************************************************************

void mainThread(void *pvParameters)
{
    int32_t             status = 0;
    pthread_attr_t      pAttrs;
    struct sched_param  priParam;
    pthread_t  eventThread = (pthread_t)NULL;
    pthread_t  connectThread = (pthread_t)NULL;

    Board_initGPIO();
    Board_initSPI();

    SerialWifi_CB.socket = -1;
    SerialWifi_CB.sockTcpServer = -1;
    SerialWifi_CB.status = 0;
    SerialWifi_CB.configurationDone = 0;
    SerialWifi_CB.tcpConnected = 0;
    SerialWifi_CB.route = SERIALWIFI_ROUTE_LOCAL;

    /* Configure the UART */
    InitTerm();

    ATCmd_create();
    SerialWifi_registerHostEvents(1);
    SerialWifi_displayBanner();

    pthread_attr_init(&pAttrs);
    priParam.sched_priority = 5;
    pthread_attr_setschedparam(&pAttrs, &priParam);
    pthread_attr_setstacksize(&pAttrs, SERIALWIFI_STACK_SIZE);

    status = pthread_create(&eventThread, &pAttrs, SerialWifi_eventProcess, NULL);
    if(status != 0)
    {
        UART_PRINT("could not create task\n\r");
        /* error handling */
        while(1);
    }

    pthread_attr_init(&pAttrs);
    priParam.sched_priority = 6;
    pthread_attr_setschedparam(&pAttrs, &priParam);
    pthread_attr_setstacksize(&pAttrs, SERIALWIFI_STACK_SIZE);
    pthread_attr_setdetachstate(&pAttrs,PTHREAD_CREATE_DETACHED);

    status = pthread_create(&connectThread, &pAttrs, SerialWifi_connectProcess, NULL);
    if(status != 0)
    {
        UART_PRINT("could not create task\n\r");
        /* error handling */
        while(1);
    }


    SerialWifi_cmdProcess();
}



//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
