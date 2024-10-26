#pragma once

#include "sys/unistd.h"
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>

#ifndef DEBUG
#define DEBUG(fmt, ...) printf("%s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#endif

// Base class which implements read() and write() methods on an I2C bus
class I2CDev
{
    public:
        I2CDev(const char *bus, uint8_t addr)
        : addr(addr)
        {
            bool ok = true;

            fd = open(bus, O_RDWR);
            if (fd < 0)
            {
                ok = false;
                perror("open");
            }

            if (ok)
            {
                if (ioctl(fd, I2C_SLAVE, addr) < 0)
                {
                    ok = false;
                    perror("ioctl");
                }
            }
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

            size_t written = ::write(fd, (const uint8_t *) buf, len);
            if(written != len)
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

            size_t bytesRead = ::read(fd, (uint8_t *)buf, len);

            if (bytesRead != len)
            {
                DEBUG("Didn't read all bytes requested");
            }

            return bytesRead;
        }

    private:
        int fd = -1;
        uint8_t addr = 0;
};

