#include <stdint.h>
#include <stdio.h>

#ifdef ARDUINO
#include <Wire.h>
#include "arduino/i2cdev.hpp"
#define DEBUG(fmt, ...) Serial.printf("%s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__)

#elif  defined __linux__
#include "posix/i2cdev.hpp"
#define DEBUG(fmt, ...) printf("%s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#define delay(ms) usleep(1000*(ms))
#else
#error "No platform defined"
#endif


// AHT10 sensor driver
class Aht10 : I2CDev
{
    public:
#ifdef ARDUINO
        Aht10(TwoWire &wire)
        : I2CDev(wire, AHTX0_I2CADDR_DEFAULT)
        {
        }
#elif defined __linux__
        Aht10(const char *bus)
        : I2CDev(bus, AHTX0_I2CADDR_DEFAULT)
        {
        }
#else
#error "No platform defined"
#endif

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

