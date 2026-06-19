// HD-38

const int SENSOR_AO  = A0;  // saída analógica
const int SENSOR_DO  = A1;  // saída digital

const int SENSOR_GND = A2;  // alimentação GND
const int SENSOR_VCC = A3;  // alimentação +5V

bool serPrint = false;

void setup() {
  // Alimentação do sensor
  pinMode(SENSOR_GND, OUTPUT);
  pinMode(SENSOR_VCC, OUTPUT);

  digitalWrite(SENSOR_GND, LOW);
  digitalWrite(SENSOR_VCC, HIGH);

  // Entradas do sensor
  pinMode(SENSOR_DO, INPUT);

  Serial.begin(115200);

  delay(500); // estabilização da alimentação
}

void loop() {
  int leitura = analogRead(SENSOR_AO);

  // Converte para porcentagem aproximada -- Pensar na calibração
  int umidade = map(leitura, 1024, 0, 0, 100); // (xx, medida seca, medida molhada, xx)


  if (serPrint){
    Serial.print("ADC: ");
    Serial.print(leitura);

    Serial.print(" | Umidade: ");
    Serial.print(umidade);
    Serial.println("%");
  }
  else{
    Serial.println(umidade);
  }
  delay(500);
}
