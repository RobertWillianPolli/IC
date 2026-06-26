// Controle do SD card

#include "SD.h"
#include "SPI.h"

const int pinoSS = 10;
int dados = 0;

void sdsetup(void) {
  
  pinMode(pinoSS, OUTPUT);    // Declara pinoSS como saída  
  digitalWrite(pinoSS, HIGH); // Coloca o pino seletor em nível alto

  delay(1000); 

  if (!SD.begin(pinoSS)) {
    Serial.println("Falha no cartão SD");
    return;
  }
  else{
    Serial.println("SD pronto para o uso");
  }
  return;
}

void sd_write(String file, String data) { // Salva uma string
  File arquivo = SD.open(file, FILE_WRITE);

  if (arquivo){
    arquivo.println(data);
    arquivo.close();
  }
  return;
}

void sd_buffer(String filename, int bufferIndex, String buffer[]){  // Salva um buffer de dados
  File file = SD.open(filename, FILE_WRITE);

  if (file) {
    for (int i = 0; i < bufferIndex; i++) {
      file.println(buffer[i]);
    }
    file.close();
  } else {
    Serial.println("Erro ao abrir SD");
  }
}
