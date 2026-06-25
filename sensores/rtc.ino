// Real-Time Clock

#include <Wire.h>
#include <RTClib.h> //by Adafruit

#define AJUSTAR_RELOGIO true

RTC_DS3231 rtc;

char date[20];

void rtcinit(void) {

  Wire.begin();

  if (!rtc.begin()) {
    Serial.println("Erro: DS3231 nao encontrado!");
    while (1);
  }
}

void rtcset(int ano, int mes, int dia, int hora, int minuto, int seg){
  rtc.adjust(DateTime(ano, mes, dia, hora, minuto, seg));
  Serial.println("Relogio ajustado.");
}

char* rtcnow(void) {
  DateTime now = rtc.now();

  sprintf(date,
          "%02d/%02d/%04d,%02d:%02d:%02d",
          now.day(),
          now.month(),
          now.year(),
          now.hour(),
          now.minute(),
          now.second());
          
  return date;
}
