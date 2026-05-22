// Programa para leitura dos sensores de umidade
// 22/05/2026

#include "SPI.h"
#include "Wire.h"
#include "TimerOne.h"
#include "SD.h"
#include "DFRobot_SHT20.h"
#include "Adafruit_SHT4x.h"

// Este programa controlará: 
// 4 sensores analógicos (2 capacitivos e 2 resistivos)
// 2 sensores digitais (capacitivos)
// 1 leitor de cartão SD

#define CS 10     // Pino CS da SPI
#define VCONT 6   // Pino de controle da alimentação

const int SHT20_ADDR = 0x00; // Endereço I2C

volatile unsigned int contadorSeg = 0;
const unsigned int TIMER = 600;       // Tempo de interrução de 10 minutos, em segundos
volatile bool tempoEsgotado = false;

const int analogPins[] = {A0, A1, A2, A3};

void contaTempo();
void escreveSD(const String& dados);

DFRobot_SHT20 sht20(&Wire, SHT20_ADDR);
Adafruit_SHT4x sht4;

void setup() {
  // put your setup code here, to run once:
  pinMode(CS, OUTPUT);      // Configura o chip select como saída
  pinMode(VCONT, OUTPUT);   // Configura o pino de controle da alimentação como saída
  digitalWrite(CS, HIGH);   // Coloca o pino seletor em nível alto
  digitalWrite(VCONT, LOW); // Inicia com os sensores desligados
  
  if (!SD.begin(CS)) {
    Serial.println("Falha no cartão SD");
  }
  
  Wire.begin();
  Serial.begin(9600);

  sht20.initSHT20();   // Inicializa o SHT20

  if (!sht4.begin()) {
    Serial.println("SHT4x não encontrado");
  }
  else{
    Serial.print("Serial Number: 0x");
    Serial.print(sht4.readSerial(), HEX);
    //uint32_t serial;
    //sht4.getSerialNumber(&serial);
  }

  File arquivo = SD.open("medidas.txt", FILE_WRITE);

  if (arquivo){
    arquivo.println("Medida dos sensores de umidade");
    arquivo.println("Resistivo 1;Resistivo 2;CapAnalog 1;CapAnalog 2;CapDigital 1;CapDigital 2");
    arquivo.close();
  }
  else{
    // print erro
  }
  
  Timer1.initialize(1000000);         // Interrupção a cada um segundo
  Timer1.attachInterrupt(contaTempo); // Função de tratamento da interrupção
}

void loop() {
  // put your main code here, to run repeatedly:
  if (tempoEsgotado){
    
    Timer1.stop();        // Pausa o contador
    
    noInterrupts();
    contadorSeg = 0;
    tempoEsgotado = false;
    interrupts();

    sensor_event_t humidity, temp;
    
    String data = "";
    float anRead;

    digitalWrite(VCONT, HIGH); // Liga os sensores

    //@TODO: Módulo RTC 
 
    for (int ain = 0; ain < 4; ain++){
      
      // Precisa converter a tensão?
      anRead = analogRead(analogPins[ain]) * (5.0 / 1023.0);

      data += String(anRead, 2);          // Duas casas decimais
      data += ";";
    }

    // Leitura dos sensores I2C
    // sht20
    float humd = sht20.readHumidity();
    
    data += String(humd, 2);
    data += ";";

    // sht4x
    sht4.getEvent(&humidity, &temp);

    data += String(humidity.relative_humidity, 2);
    
    escreveSD(data);

    digitalWrite(VCONT, LOW); // Desliga os sensores
    
    Timer1.restart();         // Reinicia o contador
  }
}

void contaTempo(){
  contadorSeg++;

  tempoEsgotado = (contadorSeg >= TIMER);
}

void escreveSD(const String& dados){
    File arquivo = SD.open("medidas.txt", FILE_WRITE);

    if (arquivo){
      arquivo.println(dados);
      arquivo.close();
    }   
}
