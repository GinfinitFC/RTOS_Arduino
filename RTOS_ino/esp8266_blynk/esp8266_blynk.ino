#define BLYNK_PRINT Serial

#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>

BlynkTimer timer;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "_7sT_AZUNNs2VkbxMqCkfWOwM9wBjrSl";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "COM-818";
char pass[] = "0398B757";

#define pot A0

void readSensor(){
  int value;
  int mappedvalue;

  value = analogRead(pot); // analog read of potentiometer
  mappedvalue = map(value, 0, 1023, 0, 22);

  //Serial.println(mappedvalue);

  Blynk.virtualWrite(V7, mappedvalue); // send value to blynk app
  Blynk.virtualWrite(V8, mappedvalue); // send value to blynk app

}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // setup blynk app
  Blynk.begin(auth, ssid, pass);

  // Setup a function to be called every second
  timer.setInterval(500L, readSensor);
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  timer.run();
}
