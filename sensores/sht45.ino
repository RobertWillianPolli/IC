#include <Wire.h>
#include <Adafruit_SHT4x.h>

Adafruit_SHT4x sht4 = Adafruit_SHT4x();

void setup() {
  Serial.begin(115200);

  while (!Serial) {
    delay(10);
  }

  Serial.println("Inicializando SHT40...");

  if (!sht4.begin()) {
    Serial.println("Sensor SHT40 nao encontrado!");
    while (1) {
      delay(10);
    }
  }

  Serial.println("SHT40 encontrado!");

  // Precisão alta
  sht4.setPrecision(SHT4X_HIGH_PRECISION);

  // Aquecimento desligado
  sht4.setHeater(SHT4X_NO_HEATER);
}

void loop() {

  sensors_event_t humidity, temp;

  if (!sht4.getEvent(&humidity, &temp)) {
    Serial.println("Erro de leitura");
    delay(1000);
    return;
  }

  Serial.print("Temperatura: ");
  Serial.print(temp.temperature);
  Serial.println(" °C");

  Serial.print("Umidade: ");
  Serial.print(humidity.relative_humidity);
  Serial.println(" %");

  Serial.println();

  delay(1000);
}
