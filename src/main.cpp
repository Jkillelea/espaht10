#include <cstdio>
#include <Arduino.h>
#include <Wire.h>

#include "ESP8266WiFi.h"
#include "ESP8266WiFiType.h"
#include "HardwareSerial.h"
#include "core_esp8266_features.h"

#include "aht10.hpp"

#define DEBUG(fmt, ...) Serial.printf("%s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__)

void onStationModeConnected(const WiFiEventStationModeConnected &event)
{
    Serial.printf("Connected to %s\n", WiFi.SSID().c_str());
}

void onStationModeDisconnected(const WiFiEventStationModeDisconnected &event)
{
    Serial.printf("WiFi connection lost\n");
}

void onStationModeGotIP(const WiFiEventStationModeGotIP &event)
{
    Serial.printf("WiFi got IP\n");
}

void onStationModeDHCPTimeout()
{
    Serial.printf("DHCP timeout!\n");
}

void onStationModeAuthModeChanged(const WiFiEventStationModeAuthModeChanged &event)
{
    Serial.printf("WiFi auth mode changed!\n");
}

Aht10 aht10(Wire);

void setup()
{
    Serial.begin(115200);
    DEBUG("booting");
    delay(3000);
    DEBUG("ok");

    WiFi.setHostname("esp8266");
    WiFi.setAutoConnect(true);
    WiFi.enableSTA(true);
    WiFi.enableAP(false);
    WiFi.begin("bebble", "bebblehouse");

    WiFi.onStationModeConnected(onStationModeConnected);
    WiFi.onStationModeDisconnected(onStationModeDisconnected);
    WiFi.onStationModeGotIP(onStationModeGotIP);
    WiFi.onStationModeDHCPTimeout(onStationModeDHCPTimeout);
    WiFi.onStationModeAuthModeChanged(onStationModeAuthModeChanged);

    Wire.begin(D3, D4);
    bool ok = aht10.begin();

    if (ok) {
        Serial.println("AHT10 initialized");
    } else {
        Serial.println("AHT10 init fail!");
    }
}

void loop()
{
    // Serial.printf("Connected: %s\n", WiFi.isConnected()? "true" : "false");

    if (aht10.poll())
    {
        float temp = aht10.temperature();
        float humid = aht10.humidity();

        Serial.printf("Temp: %3.2f, Humidity: %3.2f\n", temp, humid);
    }

    delay(1000);
}
