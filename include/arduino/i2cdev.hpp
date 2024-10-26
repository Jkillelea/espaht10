#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <stdint.h>

#ifndef DEBUG
#define DEBUG(fmt, ...) Serial.printf("%s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#endif

// Base class which implements read() and write() methods on an I2C bus
class I2CDev
{
    public:
        I2CDev(TwoWire &wire, uint8_t addr)
        : i2c(wire)
        , addr(addr)
        {
        }

    protected:
        template<typename T>
        size_t write(const T *buf, size_t len)
        {
            if (!buf)
            {
                DEBUG("null buffer");
                return 0;
            }

            i2c.beginTransmission(addr);

            size_t written = i2c.write((const uint8_t *) buf, len);
            if(i2c.endTransmission() != 0)
            {
                DEBUG("I2C Write error");
                return 0;
            }

            return written;
        }

        template<typename T>
        size_t read(T *buf, size_t len)
        {
            if (!buf)
            {
                DEBUG("null buffer");
                return 0;
            }

            i2c.beginTransmission(addr);
            i2c.requestFrom(addr, len);

            size_t bytesRead = i2c.readBytes((uint8_t *)buf, len);
            if (bytesRead != len)
            {
                DEBUG("Didn't read all bytes requested");
            }

            int end = i2c.endTransmission();
            if(end != 0)
            {
                DEBUG("I2C Read error");
                return 0;
            }

            return bytesRead;
        }

    private:
        TwoWire &i2c;
        uint8_t addr{0};
};

