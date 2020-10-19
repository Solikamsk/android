#include "I2C_Anything.h"
/* FreqMeasure - Example with serial output
 * http://www.pjrc.com/teensy/td_libs_FreqMeasure.html
 *
 * This example code is in the public domain.
 */
#include <FreqMeasure.h>

void setup() {

  analogWrite(3,127);
  Serial.begin(115200);
  Serial.println("Start");
  FreqMeasure.begin();
}

double sum=0;

void loop() {

  //на 10 пине у нано измеряется
  if (FreqMeasure.available()) {
    // average several reading together
    sum = FreqMeasure.read();
    float frequency = FreqMeasure.countToFrequency(sum);
    Serial.print("Frequency ");
    Serial.println(frequency);
  } else {
    Serial.println("Not available");
  }
    delay(500);
}
