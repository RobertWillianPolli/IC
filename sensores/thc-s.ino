#include <ModbusMaster.h> //Doc Walker

ModbusMaster node;

uint8_t result;
float temperature;
float humidity;
uint16_t ec;

void thcsetup() {

  Serial1.begin(4800);       // THC-S RS485
  node.begin(1, Serial1);    // Endereço Modbus = 1

  Serial.println("THC-S iniciado");
}

float thc_temp(){
  result = node.readHoldingRegisters(0x0000, 3);

  if (result == node.ku8MBSuccess) {
    temperature = ((int16_t)node.getResponseBuffer(1)) / 10.0;
    return temperature;
  }
  else{
    return -1;
  }
}

float thc_humid(){
  result = node.readHoldingRegisters(0x0000, 3);

  if (result == node.ku8MBSuccess) {
    humidity = node.getResponseBuffer(0) / 10.0;  
    return humidity;
  }
  else{
    return -1;
  }
}

uint16_t thc_ec(){
  result = node.readHoldingRegisters(0x0000, 3);

  if (result == node.ku8MBSuccess) {
    ec = node.getResponseBuffer(2);
      return ec;
  }
  else{
    return -1;
  }
}
