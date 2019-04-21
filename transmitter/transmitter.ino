
// ask_transmitter.pde
// -*- mode: C++ -*-
// Simple example of how to use RadioHead to transmit messages
// with a simple ASK transmitter in a very simple way.
// Implements a simplex (one-way) transmitter with an TX-C1 module

#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile
//#include <inttypes.h>

uint16_t speed = 2000;
uint8_t rxPin = 11; // Any ways to avoid setting rxPin?
uint8_t txPin = 12;
uint8_t pttPin = 10; // Any ways to avoid setting pttPin?
boolean pttInverse = false;
RH_ASK driver(speed, rxPin, txPin, pttPin,pttInverse);


void setup()
{
    Serial.begin(9600);  // Debugging only
    if (!driver.init())
         Serial.println("init failed");
}

void loop()
{
    const char *msg = "hello";

    driver.send((uint8_t *)msg, strlen(msg));
    driver.waitPacketSent();
    delay(200);
}
