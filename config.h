/*
* create a deviceconfig.h header that holds defines for device specific information
*
* #define DEVICE_NAME "My device Name"
* #define DEVICE_TYPE "ledv1"
* #define STASSID "MyWifiSSID"
* #define STAPSPK "MyWifiPassword"
*/
#include "deviceconfig.h"

#ifndef DEVICE_NAME
#error DEVICE_NAME is not defined
#endif

#ifndef DEVICE_TYPE
#error DEVICE_TYPE is not defined
#endif

#ifndef STASSID
#error STASSID is not defined
#endif

#ifndef STAPSK
#error STAPSK is not defined
#endif

#ifndef TRANSMIT_PIN
#error TRANSMIT_PIN is not defined
#endif

#ifndef SWITCH_COUNT
#error SWITCH_COUNT is not defined
#endif

String deviceName = DEVICE_NAME;
String deviceType = DEVICE_TYPE;

// TCP ===================

#include <ESP8266WiFi.h>

const char* ssid = STASSID;
const char* password = STAPSK;

unsigned int tcpPort = 4300;

WiFiServer wifiServer(tcpPort);

// UDP ====================

#include <WiFiUdp.h>

unsigned int udpPort = 4301;

WiFiUDP udp;

IPAddress localIP;

bool listening = true;

#include <RCSwitch.h>

RCSwitch sw = RCSwitch();

unsigned int switchCount = SWITCH_COUNT;