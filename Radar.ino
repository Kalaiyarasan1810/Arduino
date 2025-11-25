#include <Servo.h>
#include <Wire.h>
#include <QMC5883LCompass.h>
#include "DHT.h"

// --- DHT11 setup ---
#define DHTPIN 5           // moved here to avoid conflict with sound sensor
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// --- Compass setup ---
QMC5883LCompass compass;

// --- Ultrasonic sensors ---
const int trig1 = 9;   // sweeping
const int echo1 = 10;
const int trig2 = 6;   // fixed
const int echo2 = 7;

// --- Other sensors ---
const int soundAnalog = A0;
const int mq2Analog   = A1;
const int pirPin      = 4;

// --- Servo ---
const int servoPin = 11;
Servo pan;

const int MAX_RANGE_CM = 40;

// --- Measure distance function ---
int measureCm(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long dur = pulseIn(echoPin, HIGH, 30000UL);
  if (dur == 0) return 0;
  int cm = dur * 0.034 / 2.0;
  if (cm < 2 || cm > 400) return 0;
  return cm;
}

// --- Setup ---
void setup() {
  Serial.begin(9600);
  Wire.begin();
  compass.init();
  dht.begin();

  pan.attach(servoPin);

  pinMode(trig1, OUTPUT);
  pinMode(echo1, INPUT);
  pinMode(trig2, OUTPUT);
  pinMode(echo2, INPUT);
  pinMode(pirPin, INPUT);

  delay(2000); // PIR settle (module still needs 30–60 s warm-up)
}

// --- Main loop ---
void loop() {
  for (int a = 0; a <= 180; a++) scanAndSend(a);
  for (int a = 180; a >= 0; a--) scanAndSend(a);
}

// --- Sweep + send serial data ---
void scanAndSend(int angle) {
  pan.write(angle);
  delay(15);

  int d1 = measureCm(trig1, echo1);
  delay(20);
  int d2 = measureCm(trig2, echo2);

  int soundLevel = analogRead(soundAnalog);
  int smokeLevel = analogRead(mq2Analog);
  int pir = digitalRead(pirPin);

  compass.read();
  int heading = compass.getAzimuth();

  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) { temp = 0; hum = 0; }

  // Format: angle,dSweep,dFixed,sound,smoke,heading,pir,temp,hum.
  Serial.print(angle);      Serial.print(",");
  Serial.print(d1);         Serial.print(",");
  Serial.print(d2);         Serial.print(",");
  Serial.print(soundLevel); Serial.print(",");
  Serial.print(smokeLevel); Serial.print(",");
  Serial.print(heading);    Serial.print(",");
  Serial.print(pir);        Serial.print(",");
  Serial.print(temp, 1);    Serial.print(",");
  Serial.print(hum, 1);     Serial.print(".");
}
