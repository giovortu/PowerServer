#ifndef POWER_WEB_SERVER_H
#define POWER_WEB_SERVER_H


#include <Arduino.h>
#include <EmonLib.h>
#if !defined (ARDUINO_ARCH_ESP8266)
#include <WiFi.h>
#include <WebServer.h>
#include <mdns.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#endif

#include <WiFiUdp.h>
#include <PubSubClient.h>

//Constants and defines
#define UFFICIO 1

#define AUTO_READ_DELAY 30000
#define HTTP_PORT 9090

#if !defined (ARDUINO_ARCH_ESP8266)
#define ANALOG_PIN 34
#else
#define ANALOG_PIN A0
#endif

#ifdef UFFICIO
const char * _SSID      = "abinsula-28";
const char * _PASSWORD  = "uff1c10v14l3umb3rt028";
const char *MQTT_BROKER = "10.0.128.128";
#else
const char * _SSID      = "ddwrt";
const char * _PASSWORD  = "CASA3m1l71a_Rrx1140";
const char *MQTT_BROKER = "192.168.0.227";
#endif

const int MQTT_PORT = 1883;
const char* topic_send = "/home/sensors/powermeter/read";
const char* topic_recv = "/home/sensors/powermeter/write";

const char* HOSTNAME = "power_meter";

byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x00, 0xCD };

long lastReadingTime = 0;
double irms, power;
int g_mqttConnectionTimeout = 0;
bool force_read = false;

//Objects
#if !defined (ARDUINO_ARCH_ESP8266)
WebServer server(HTTP_PORT);
#else
ESP8266WebServer server(HTTP_PORT);
#endif

WiFiClient espClient;
EnergyMonitor emon1;
PubSubClient mqtt;

//Functions
String getData();
bool mqttConnect();
void mqttCallback(char* topic, byte* payload, unsigned int length);

#endif