#if 1

#include "Arduino.h"
#include "HardwareSerial.h"
//#include "readpixel_kbv.inc"
//#include "graphictest_kbv.inc"

HardwareSerial serial_kbv(USBTX, USBRX); 

uint32_t millis(void)
{
    static Timer t;
    static int first = 1;
    if (first) first = 0, t.start();
    return t.read_ms();
}

uint32_t micros(void)
{
    static Timer t;
    static int first = 1;
    if (first) first = 0, t.start();
    return t.read_us();
}

int main(void)
{
    setup();
    while (1) {
        loop();
    }
    return 0;  //never happens
}

#endif
