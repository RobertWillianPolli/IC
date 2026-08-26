
// ===== BUFFER =====
const int BUFFER_SIZE = 10;
String buffer[BUFFER_SIZE];
int bufferIndex = 0;

char linhaCSV[200];

float umidade[6];

void setup() {
  Serial.begin(115200);
  sdsetup();
  thcsetup();
  rtcinit();

  rtcset(2026, 8, 25, 14, 30, 00);

  // Cabeçalho CSV
  sd_write("log.csv", "data,hora,umidade_%_1,umidade_%_2,umidade_%_3,umidade_%_4,umidade_%_5,umidade_%_6");
}

void loop() {

  //Descobrir quais sensores estão conectados

  for(int i = 0; i < 6; i++){
    thcsetaddr(i + 1);
    umidade[i] = thc_humid();
  }

  sprintf(linhaCSV,
      "%s,%.02f,%.02f,%.02f,%.02f,%.02f,%.02f",
      rtcnow(),
      umidade[0],
      umidade[1],
      umidade[2],
      umidade[3],
      umidade[4],
      umidade[5]
      );

  // buffer
  buffer[bufferIndex++] = String(linhaCSV);

  // grava no SD
  if (bufferIndex >= BUFFER_SIZE) {
    sd_buffer("log.csv", bufferIndex, buffer);
    bufferIndex = 0;
  }

  delay(1000);
}
