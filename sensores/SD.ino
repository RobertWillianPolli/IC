#include "SD.h"
#include "SPI.h"

const int pinoSS = 10;
int dados = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(pinoSS, OUTPUT); // Declara pinoSS como saída
  
  digitalWrite(pinoSS, HIGH);     // Coloca o pino seletor em nível alto
  
  delay(100); // aguarda estabilização

  Serial.begin(9600);
  delay(2000); 

  if (!SD.begin(pinoSS)) {
    Serial.println("Falha no cartão SD");
    return;
  }
  else{
    Serial.println("SD pronto para o uso");
  }
}

void loop() {
  File arquivo = SD.open("medidas.txt", FILE_WRITE);

  dados += 1;

  if (arquivo){
    arquivo.println(dados);
    arquivo.close();
    Serial.println(dados);
  }
  delay(5000);
}
