#include <Wire.h>
#include <Adafruit_SHT4x.h>

Adafruit_SHT4x sht4 = Adafruit_SHT4x();

void shtsetup() {

  Serial.println("Inicializando SHT40...");

  if (!sht4.begin()) {
    Serial.println("Sensor SHT40 nao encontrado!");
    return;
  }

  Serial.println("SHT40 encontrado!");

  // Precisão alta
  sht4.setPrecision(SHT4X_HIGH_PRECISION);

  // Aquecimento desligado
  sht4.setHeater(SHT4X_NO_HEATER);
}

float sthread(bool temp, bool humd) {

  sensors_event_t humidity, temperature;

  if (!sht4.getEvent(&humidity, &temperature)) {
    Serial.println("Erro de leitura");
    return -1;
  }

  if(temp){
    return temperature.temperature;
  }
  else if(humd){
    return humidity.relative_humidity;
  }

  else{
    return -1;
  }
}
