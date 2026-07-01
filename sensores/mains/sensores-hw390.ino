
// ===== BUFFER =====
const int BUFFER_SIZE = 10;
String buffer[BUFFER_SIZE];
int bufferIndex = 0;

char linhaCSV[100];

float umidade = 0;

// Alimentação do módulo rtc
  const int RTC_GND = A2;
  const int RTC_VCC = A3;

void setup() {

  // Alimentação do módulo RTC
  pinMode(RTC_GND, OUTPUT);
  pinMode(RTC_VCC, OUTPUT);

  digitalWrite(RTC_GND, LOW);   // GND
  digitalWrite(RTC_VCC, HIGH);  // +5V

  Serial.begin(115200);
  sdsetup();
  hwsetup();
  rtcinit();

  // Cabeçalho CSV
  sd_write("log.csv", "data,hora,umidade_%");
}

void loop() {
 
  umidade = hwread(true);
  
  Serial.println(umidade);
  
  sprintf(linhaCSV,
          "%s,%.02f",
          rtcnow(),
          umidade);
  
  // buffer
  buffer[bufferIndex++] = String(linhaCSV);

  // grava no SD
  if (bufferIndex >= BUFFER_SIZE) {
    sd_buffer("log.csv", bufferIndex, buffer);
    bufferIndex = 0;
  }

  delay(1000);
}
