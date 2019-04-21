
// ask_receiver.pde
// -*- mode: C++ -*-
// Simple example of how to use RadioHead to receive messages
// with a simple ASK transmitter in a very simple way.
// Implements a simplex (one-way) receiver with an Rx-B1 module

#include <RH_ASK.h>
#include <SPI.h> // Not actualy used but needed to compile

unsigned int speed = 2000;
uint8_t rxPin = 10; 
uint8_t txPin = 6;   // Any ways to avoid setting txPin?
uint8_t pttPin = 7;  // Any ways to avoid setting pttPin?
bool pttInverse = false;
RH_ASK driver(speed, rxPin, txPin, pttPin,pttInverse);

void setup()
{
    Serial.begin(9600); // Debugging only
    if (!driver.init())
         Serial.println("init failed");;
    pinMode(8,OUTPUT);
    digitalWrite(8,LOW);
    pinMode(11,OUTPUT);
    digitalWrite(11,HIGH);
    pinMode(9,INPUT); // Ensure high impedance at D9 
    pinMode(10,INPUT); // Ensure high impedance at D10

}

void loop()
{
    uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
    uint8_t buflen = sizeof(buf);

    if (driver.recv(buf, &buflen)) // Non-blocking
    {
 int i;
 // Message with a good checksum received, dump it.
 driver.printBuffer("Got:", buf, buflen);
    }
}
