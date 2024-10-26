#include "ESP8266WiFi.h"

#include "ESP8266WiFiType.h"
#include "HardwareSerial.h"
#include "core_esp8266_features.h"
#include <Arduino.h>
#include <Wire.h>
#include <cstdint>
#include "i2cdev_arduino.hpp"

// #define DEBUG(fmt, ...) Serial.printf("%s: " fmt "\n", __PRETTY_FUNCTION__, ##__VA_ARGS__)
#define DEBUG(fmt, ...) Serial.printf("%s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__)

// AHT10 sensor driver
class Aht10 : I2CDev
{
    public:
        Aht10(TwoWire &wire)
        : I2CDev(wire, AHTX0_I2CADDR_DEFAULT)
        {
        }

        uint8_t getStatus()
        {
            uint8_t status = 0;
            if (read(&status, 1) != 1)
            {
                DEBUG("read error");
                return 0xFF;
            }

            return status;
        }

        // Wait for the device to indicate it is ready.
        void waitReady()
        {
            while ((getStatus() & AHTX0_STATUS_BUSY) == AHTX0_STATUS_BUSY)
            {
                delay(10);
            }
        }

        bool begin()
        {
            if (write(&AHTX0_CMD_SOFTRESET, 1) != 1)
            {
                return false;
            }

            delay(20);

            const uint8_t calibrate_cmd[] = {AHTX0_CMD_CALIBRATE, 0x08, 0x00};

            if (write(calibrate_cmd, sizeof(calibrate_cmd)) != sizeof(calibrate_cmd))
            {
                return false;
            }

            waitReady();

            if ((getStatus() & AHTX0_STATUS_CALIBRATED) == 0)
            {
                DEBUG("Not calibrated!");
                return false;
            }

            return true;
        }


        bool poll()
        {
            // read the data and store it!
            const uint8_t cmd[3] = {AHTX0_CMD_TRIGGER, 0x33, 0};
            if (write(cmd, 3) != 3)
            {
                DEBUG("Write error");
                return false;
            }

            waitReady();

            uint8_t data[6] = {};
            if (read(data, sizeof(data)) != sizeof(data))
            {
                DEBUG("Read error");
                return false;
            }

            uint32_t h = data[1];
            h <<= 8;
            h |= data[2];
            h <<= 4;
            h |= data[3] >> 4;
            _humidity = ((float)h * 100) / 0x100000;

            uint32_t tdata = data[3] & 0x0F;
            tdata <<= 8;
            tdata |= data[4];
            tdata <<= 8;
            tdata |= data[5];
            _temperature = ((float)tdata * 200 / 0x100000) - 50;

            return true;
        }

        float humidity()
        {
            return _humidity;
        }

        float temperature()
        {
            return _temperature;
        }

    private:
        float _humidity = 0;
        float _temperature = 0;

        static constexpr uint8_t AHTX0_I2CADDR_DEFAULT   = 0x38; // AHT default I2C address
        static constexpr uint8_t AHTX0_I2CADDR_ALTERNATE = 0x39; // AHT alternate I2C address
        static constexpr uint8_t AHTX0_CMD_CALIBRATE     = 0xE1; // Calibration command
        static constexpr uint8_t AHTX0_CMD_TRIGGER       = 0xAC; // Trigger reading command
        static constexpr uint8_t AHTX0_CMD_SOFTRESET     = 0xBA; // Soft reset command
        static constexpr uint8_t AHTX0_STATUS_BUSY       = 0x80; // Status bit for busy
        static constexpr uint8_t AHTX0_STATUS_CALIBRATED = 0x08; // Status bit for calibrated
};

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
