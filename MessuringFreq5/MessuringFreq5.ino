#include <FreqMeasure.h>

void setup() {
  analogWrite(3,127);
  Serial.begin(115200);
  FreqMeasure.begin();
  Serial.println("St art");
}

double sum=0;
int count=0;

void loop() {
  if (FreqMeasure.available()) {
    // average several reading together
    sum = sum + FreqMeasure.read();
    count = count + 1;
    if (count > 0) {
      float frequency = FreqMeasure.countToFrequency(sum / count);
      Serial.println(frequency);
      sum = 0;
      count = 0;
    }
  }
  delay(100);
}
