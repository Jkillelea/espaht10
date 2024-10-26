#include <stdio.h>

#include "aht10.hpp"

int main(int argc, char *argv[])
{
    Aht10 aht("/dev/i2c-0");
    aht.begin();
    while (true) {
        if (aht.poll()) {
        }
    }
    return 0;
}
