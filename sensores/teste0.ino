#include "SD.h"
#include "SPI.h"
#include <Wire.h>
#include <RTClib.h>
#include <ModbusMaster.h>

const int pinoSS = 10;

RTC_DS3231 rtc;
ModbusMaster node;

#define AJUSTAR_RELOGIO false

// ===== BUFFER =====
const int BUFFER_SIZE = 10;
String buffer[BUFFER_SIZE];
int bufferIndex = 0;

char linhaCSV[100];

uint8_t result;
float umidade = 0;

void setup() {
  pinMode(pinoSS, OUTPUT);
  digitalWrite(pinoSS, HIGH);

  Serial.begin(115200);
  delay(2000);

  if (!SD.begin(pinoSS)) {
    Serial.println("Falha no cartão SD");
    while (1);
  }

  if (!rtc.begin()) {
    Serial.println("Erro: DS3231 nao encontrado!");
    while (1);
  }

  if (AJUSTAR_RELOGIO) {
    rtc.adjust(DateTime(2026, 6, 23, 12, 8, 0));
  }

  // Modbus (AJUSTE PORTA/ID conforme seu THC-S)
  Serial1.begin(4800);
  node.begin(1, Serial1); // ID = 1 (ajuste se necessário)

  // Cabeçalho CSV
  if (!SD.exists("log.csv")) {
    File file = SD.open("log.csv", FILE_WRITE);
    if (file) {
      file.println("data,hora,umidade_%");
      file.close();
    }
  }
}

void salvarBuffer() {
  File file = SD.open("log.csv", FILE_WRITE);

  if (file) {
    for (int i = 0; i < bufferIndex; i++) {
      file.println(buffer[i]);
    }
    file.close();
    bufferIndex = 0;
  } else {
    Serial.println("Erro ao abrir SD");
  }
}

void loop() {
  DateTime now = rtc.now();

  // ===== MODBUS THC-S =====
  result = node.readHoldingRegisters(0x0000, 1);

  if (result == node.ku8MBSuccess) {
    umidade = node.getResponseBuffer(0) / 10.0;
  } else {
    umidade = -1; // erro de leitura
  }

  // ===== CSV =====
  sprintf(linhaCSV,
          "%02d/%02d/%04d,%02d:%02d:%02d,%.2f",
          now.day(),
          now.month(),
          now.year(),
          now.hour(),
          now.minute(),
          now.second(),
          umidade);

  // buffer
  buffer[bufferIndex] = String(linhaCSV);
  bufferIndex++;

  Serial.println(linhaCSV);

  // grava no SD
  if (bufferIndex >= BUFFER_SIZE) {
    salvarBuffer();
  }

  delay(5000);
}
