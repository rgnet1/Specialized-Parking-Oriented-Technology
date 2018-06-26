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

#ifndef __CMD_PARSER_H__
#define __CMD_PARSER_H__

/* Application includes */
#include "socket_cmd.h"
#include "network_terminal.h"

/* Application defines */
#define MAX_SERVICE_NAME_LENGTH      (63)
#define CHANNEL_MASK_ALL            (0x1FFF)
#define RSSI_TH_MAX                 (-95)


/* Command structures */

typedef struct ScanCmd
{
    uint8_t         numOfentries;       /* Number of Scan Entries to retrieve from the NWP */
    uint8_t         index;              /* The netEntries array position to start write results from */
}ScanCmd_t;


typedef struct SetPolicyCmd
{
    uint8_t                  turnOff;                 /* State of the scans - ON or OFF */
    uint32_t                 ScanIntervalinSec;       /* Default is set to 10 Seconds */
    uint8_t                  hiddenSsid;              /* Displays hidden SSID's - Yes or No */
    SlWlanScanParamCommand_t ScanParamConfig;         /* Scan parameters configurations - RSSI threshold and channel mask */
}SetPolicyCmd_t;


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


typedef struct StartApCmd
{
    uint8_t                 *ssid;           /* Ap's SSID */
    uint8_t                 hidden;          /* Determine if AP has hidden SSID */
    uint8_t                 channel;         /* 802.11 WLAN channel [1-11] */
    uint8_t                 tx_pow;          /* The AP's TX power */
    uint8_t                 sta_limit;       /* Limits the number of stations that the AP's has */
    SlWlanSecParams_t       secParams;       /* Security parameters - Security Type and Password */
}StartApCmd_t;


typedef struct CreateFilterCmd
{
    SlWlanRxFilterRuleType_t     ruleType;       /* Header or combination filter */
    SlWlanRxFilterID_t           filterID;       /* Returned value for 'sl_WlanRxFilterAdd()' */
    SlWlanRxFilterFlags_u        flags;          /* Dictates filter behavior */
    SlWlanRxFilterRule_u         rule;           /* Match criteria */
    SlWlanRxFilterTrigger_t      trigger;        /* What are the preconditions to trigger the filter */
    SlWlanRxFilterAction_t       action;         /* Operation that execute upon a filter match */
}CreateFilterCmd_t;


typedef struct WoWLANEnableCmd
{
    SlWlanRxFilterRuleType_t     ruleType;       /* Header or combination filter */
    SlWlanRxFilterID_t           filterID;       /* Returned value for 'sl_WlanRxFilterAdd()' */
    SlWlanRxFilterFlags_u        flags;          /* Dictates filter behavior */
    SlWlanRxFilterRule_u         rule;           /* Match criteria */
    SlWlanRxFilterTrigger_t      trigger;        /* What are the preconditions to trigger the filter */
    SlWlanRxFilterAction_t       action;         /* Operation that execute upon a filter match */
}WoWLANEnableCmd_t;

typedef struct PingCmd
{
    uint8_t               *host;         /* Host name or address */
    SlNetAppPingCommand_t pingCmd;       /* SimpleLink Ping command stracture */
}PingCmd_t;

typedef struct mDnsAdvertiseCmd
{
    uint16_t        service_port;                                /* Port on which to accept service requests */
    uint32_t        service_ttl;                                 /* Service time to leave; time to stop advertising. */
    uint8_t         service_name[MAX_SERVICE_NAME_LENGTH];       /* Service name */
    uint8_t        *adv_text;                                    /* Advertised text. */
    uint8_t        *dev_name;                                    /* device part of the advertise address */
}mDnsAdvertiseCmd_t;

typedef struct mDnsQueryCmd
{
    uint8_t        ServiceName[MAX_SERVICE_NAME_LENGTH];       /* The service name to query */
    uint8_t        OneShotFlag;                                /* Sets the query type: One shot or continuous */
}mDnsQueryCmd_t;

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

typedef struct RecvCmd
{
    int32_t                   numberOfPackets;           /* Number of packets to receive */
    uint8_t                   *ip;                       /* IP address */
    int16_t                   portNumber;                /* Server's port address*/
    uint8_t                   udpFlag;                   /* Decides the type of transport protocol */
    uint8_t                   server;                    /* Send as server or client flag */
    uint8_t                   nb;                        /* Blocking or non-blocking on socket */
    uint8_t                   ipv6;                      /* IPV4 or IPv6 flag. By default IPV4 enable */
    ip_t                      ipAddr;                    /* IP in binary format */
}RecvCmd_t;


/* Function prototypes */
int32_t ParseScanCmd(void *arg, ScanCmd_t *scanParams);
int32_t ParseSetPolicyCmd(void *arg, SetPolicyCmd_t *SetPolicyParams);
int32_t ParseConnectCmd(void *arg, ConnectCmd_t *ConnectParams);
int32_t ParseStartApCmd(void *arg, StartApCmd_t *StartApParams);
int32_t ParseCreateFilterCmd(void *arg,  CreateFilterCmd_t *CreateFilterParams);
int32_t ParseEnableWoWLANCmd(void *arg, WoWLANEnableCmd_t *WoWLANEnableParams);
int32_t ParsePingCmd(void *arg, PingCmd_t *pingParams);
int32_t ParsemDNSAdvertiseCmd(void *arg, mDnsAdvertiseCmd_t *mDNSAdvertiseParams);
int32_t ParsemDNSQueryCmd(void *arg,  mDnsQueryCmd_t *QueryCmdParams);
int32_t mDNScreateServiceName(char *p_sevName, char *p_servOver , char *p_servType, char *p_adv_service_name);
int32_t ParseSendCmd(void *arg,  SendCmd_t *SendCmdParams);
int32_t ParseRecvCmd(void *arg,  RecvCmd_t *RecvCmdParams);
int32_t ParseCmd(void *arg);
void FreeConnectCmd(ConnectCmd_t *ConnectParams);
void FreeStartApCmd(StartApCmd_t *StartApParams);
void FreePingCmd(PingCmd_t *pingParams);
void FreemDNSAdvertiseCmd(mDnsAdvertiseCmd_t *mDNSAdvertiseParams);
void FreeSendCmd(SendCmd_t *SendCmdParams);
void FreeRecvCmd(RecvCmd_t *RecvCmdParams);

#endif /* __CMD_PARSER_H__ */
