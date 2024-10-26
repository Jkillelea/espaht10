#include <stdio.h>

#include "aht10.hpp"
#include "sys/unistd.h"

int main(int argc, char *argv[])
{
    Aht10 aht("/dev/i2c-0");
    aht.begin();
    while (true) {
        if (aht.poll()) {
            float temp = aht.temperature();
            float humid = aht.humidity();
            printf("Temp: %3.2f, Humidity: %3.2f\n", temp, humid);
        }

        sleep(1);
    }
    return 0;
}
