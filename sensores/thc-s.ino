#include <ModbusMaster.h> //Doc Walker

ModbusMaster node;

void setup() {

  Serial.begin(115200);      // Monitor Serial USB

  Serial1.begin(4800);       // THC-S RS485

  node.begin(1, Serial1);    // Endereço Modbus = 1

  Serial.println("THC-S iniciado");
}

void loop() {

  uint8_t result;

  result = node.readHoldingRegisters(0x0000, 3);

  if (result == node.ku8MBSuccess) {

    float umidade =
      node.getResponseBuffer(0) / 10.0;

    float temperatura =
      ((int16_t)node.getResponseBuffer(1)) / 10.0;

    uint16_t ec =
      node.getResponseBuffer(2);

    Serial.print("Umidade: ");
    Serial.print(umidade);
    Serial.println(" %");

    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" C");

    Serial.print("EC: ");
    Serial.print(ec);
    Serial.println(" uS/cm");

    Serial.println("-------------------");
  }
  else {

    Serial.print("Erro Modbus: ");
    Serial.println(result);
  }

  delay(2000);
}
