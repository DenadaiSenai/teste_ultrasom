#include <HCSR04.h>

#define COUNT 100

byte triggerPin = 40;
byte echoCount = 2;
byte* echoPins = new byte[echoCount]{ 30, 32 };

double* distances;
double soma[2], media[2] = {0.0, 0.0};

void setup() {
  Serial.begin(115200);
  HCSR04.begin(triggerPin, echoPins, echoCount);
}

void medirDistancia() {
  soma[0] = soma[1] = 0.0;
  for(char i=0; i<COUNT; i++) {
    distances = HCSR04.measureDistanceCm();
    soma[0] += distances[0];
    soma[1] += distances[1];
    delay(1);
  }
  media[0] = soma[0] / COUNT;
  media[1] = soma[1] / COUNT;
}

void loop() {
  medirDistancia();
  Serial.print("Y:");
  Serial.print(media[0]);
  Serial.print(",X:");
  Serial.print(media[1]);
  Serial.println();  
}