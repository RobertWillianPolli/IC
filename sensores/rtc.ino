#include <Wire.h>
#include <RTClib.h> //by Adafruit

RTC_DS3231 rtc;

// Alimentação do módulo
const int RTC_GND = A2;
const int RTC_VCC = A3;

#define AJUSTAR_RELOGIO true

void setup() {
  // Alimentação do módulo RTC
  pinMode(RTC_GND, OUTPUT);
  pinMode(RTC_VCC, OUTPUT);

  digitalWrite(RTC_GND, LOW);   // GND
  digitalWrite(RTC_VCC, HIGH);  // +5V

  delay(100); // aguarda estabilização

  Serial.begin(115200);
  Wire.begin();

  if (!rtc.begin()) {
    Serial.println("Erro: DS3231 nao encontrado!");
    while (1);
  }

  if (AJUSTAR_RELOGIO) {
    rtc.adjust(DateTime(2026, 6, 18, 11, 11, 0));
    Serial.println("Relogio ajustado.");
  }
}

void loop() {
  DateTime now = rtc.now();

  Serial.print(now.day());
  Serial.print('/');
  Serial.print(now.month());
  Serial.print('/');
  Serial.print(now.year());

  Serial.print(" ");

  if (now.hour() < 10) Serial.print('0');
  Serial.print(now.hour());
  Serial.print(':');

  if (now.minute() < 10) Serial.print('0');
  Serial.print(now.minute());
  Serial.print(':');

  if (now.second() < 10) Serial.print('0');
  Serial.println(now.second());

  delay(1000);
}
