## Overview

Serial WiFi is a capability designed to provide easy, self-contained terminal access behavior, over UART interface. 
It also provides a ‘driver-less’ solution which allows the use of AT Commands to operates the various range of networking capabilities which the CC3220/CC3120 device family provides. 
This application runs over the UART interface, behaves as a command line interpreter and allows secure, robust end-to-end communication.
For details about the AT Commands and thier parameters please refer to the AT Commands and attributes: [Commands and attributes](../../../../../source/ti/net/atcmd/AT_Commands_command_list.html)

## Usage

* Build the project and flash it by using the Uniflash tool for cc32xx,  
Or equivalently, run debug session on the IDE of your choice.
* Open a serial port session (e.g. 'HyperTerminal','puTTY', 'Tera Term' etc.) to the appropriate COM port -   
listed as 'User UART'.
The COM port can be determined via Device Manager in Windows or via `ls /dev/tty*` in Linux.

The connection should have the following connection settings:
```
    Baud-rate:    115200
    Data bits:         8
    Stop bits:         1
    Parity:         None
    Flow Control:   None
```

* Run the example by pressing the reset button or by running debug session through your IDE.  
 `Board_LED0` turns ON to indicate the Application initialization is complete.

Once the application has complete it's initialization and the network processor is up,  
the application banner would be displayed, showing version details:
```
         ==============================================
            Serial WIFI Example Ver: 1.1.1
         ==============================================

         CHIP: 0x20000010
         MAC:  2.0.0.0
         PHY:  2.2.0.4
         NWP:  3.2.0.0
         ROM:  0
         HOST: 2.0.1.17
         MAC address: 08:00:28:5b:55:e0

         ==============================================
```
First, the device tries to connect to the stored AP by the AUTO connect policy.
If the device doesn't connect to an AP in 6secs, the application prompts the user to connect to a known AP using the Local control mode. 
special escape sequence prefix `\\<` is used to define AT Command in local control mode (mode in which we control the local Simplelink device).
User can use AT WLAN connect command to connect to an AP:
```
\\<AT+WlanConnect = [SSID],[BSSID],[SecurityType],[SecurityKey],[SecurityExtUser],[SecurityExtAnonUser],[SecurityExtEapMethod]
```
Or use AT WLAN add profile to connect to an AP and create profile for auto connect next time:
```
\\<AT+WlanProfileAdd = [SSID],[BSSID],[SecurityType],[SecurityKey],[SecurityExtUser],[SecurityExtAnonUser],[SecurityExtEapMethod],[Priority]
```
For example:
```
\\<at+wlanconnect=TP-LINK_123456,,open,,,,
\\<at+wlanprofileadd=TP-LINK_123456,,open,,,,,6
```
After connection is established with the AP, the device search automatically for remote peer by mDNS.
Once the remote peer was founded, the device try to connect it based on secure sockets (all certificate files should be flash by using the Uniflash tool for cc32xx)    
Default Security method is SSLV3 and Cipher is RSA WITH RC4 128 SHA.

At this point, user can take control over the remote peer by AT commands or simply send data from Local UART to the remote UART.
special escape sequence prefix `\\>` is used to define AT Command in remote control mode (mode in which we control the remote Simplelink device).


## Application details

* The Serial WiFi application supports the following features:
1. Provides access to all the SimpleLink Host APIs through the AT commands instruction set
2. Auto discovery of available peer using mDNS
3. All communication is secure using the Secure Sockets
4. Terminal mode for serial cable replacement
5. Local/Remote Control modes, to issue commands to the locally/remotely connected Simplelink device.

* The Serial WiFi application basically operates in 3 modes parallel:
1. Terminal mode:
In this case the interpreter mode behaves as pure point-to-point cable replacement. 
This is the most common and obvious use in which a serial cable formerly used to carry information is replaced by SimpleLink devices on both end of the line.
As default all data that comes from the UART will be routed to the remote peer unless it is local or remote control data which has special escape sequence prefix 
2. Local control mode:
In this mode it is possible to issue AT commands to the locally connected device.
The AT commands library encompass a close set of options to control the SimpleLink device while guaranteeing sandbox restrictions. 
In order to issue local control command, add special escape sequence prefix (//<) to the AT command.
3. Remote control mode:
This allows issuing AT commands to the remotely connected device the same as the Local control commands. 
In order to issue remote control command, add special escape sequence prefix (//>) to the AT command.

* AT commands response and AT asynchronous events should be routed according to source (local or remote) of last received command.
As default it will be routed locally.
* AT WLAN commands were blocked in remote control mode.
* The local escape sequence prefix (//<) and the remote escape sequence prefix (//>) could be simply redefine but it required new project build.
* For getting usage help for specific AT command, put '?' instead of parameters ([AT Command name] = ?)

### Commands
Syntax:
```
    AT<command name>=<param1>, <param2>,  ,<paramX>
```

-	Command that contains parameters, should be includes equal sign (=) between the command name and the first parameter.
-	Command that contains parameters, should be includes comma mark (,) as delimiter between them - comma delimiters are mandatory!
-	In case the parameter defined as "ignore" or "optional", it could be left empty but the comma delimiter should be mentioned - it looks like two conjunction delimiters (,,).
-	Parameter that left empty be treated as 0 or NULL (according to parameter type) and in case it was not defined as "ignore" or "optional" - error should be raise.
-	String parameter contains spaces should be enclosed with quotes (" ")
-	String parameter contains comma delimiter (,) should be enclosed with quotes (" ")
-	Numeric value parameter could be on of the following:

    *    Decimal

    *	 Hexadecimal - should be with prefix of zero x notation (0x)

-	Numeric array parameter could be enclosed with square brackets ([ ]).
-	Numeric array parameter could be one of the following:

    *	IPv4  address - contains 4 numeric values (8 bits each) with point mark (.)  as delimiter between them enclosed with or without square bracket - X.X.X.X or [X.X.X.X]
    
    *	IPv6 address -  contains 4 numeric values (32 bit each) with colon mark (:)  as delimiter between them enclosed with or without square bracket - X:X:X:X or [X:X:X:X]
    
    *	MAC address - contains 6 numeric values (8 bit each) with colon mark (:)  as delimiter between them enclosed with or without square bracket - X:X:X:X:X:X or [X:X:X:X:X:X]
    
-	Bit mask parameter should contains values with vertical bar ( | ) as delimiter between them enclosed with or without square bracket - X|X|X or [X|X|X]
-	AT command handler allows for the AT commands to be entered in upper or lower case and with spaces between the arguments.
-	Data parameter should be one of the following formats:

    *	Binary format
    
    *	Base64 format - binary to text encoding

### Command return status
Command return status could be one of the following cases:

*	Command that return values:
```
    <command name >: <value1>, ,<valueX>
```
*	Command that return success:
```
    OK
```
*	Command that return failure:
```
    ERROR:<error description>, <error code>
```

-	Command return status should include colon mark (:) between the command name and the first value.
-	Command return status that contains list values should include semicolon mark (;) as delimiter between the list members.

### Asynchronous event
The events may arrive at any time.
They will always be built in the following format:
```
    <event name>: <event ID>,<value1>, ,<valueX>
```

*	Event should include colon mark (:) between the event name and the event ID.
 
Please install the latest CC32xx Service Pack in order to get the most optimization and lowest current consumption.

## References

**For full list of commands and attributes, please refer to [Commands and attributes](../../../../../source/ti/net/atcmd/AT_Commands_command_list.html)**
