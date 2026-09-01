
// ===== BUFFER =====
const int BUFFER_SIZE = 10;
String buffer[BUFFER_SIZE];
int bufferIndex = 0;

char linhaCSV[100];
char umidades[50];

float umidade[4];

//Pinos de controle dos relés
// RELAY_1 = 4
// RELAY_2 = 5
// RELAY_3 = 6
// RELAY_4 = 7

void setup() {

  Serial.begin(115200);
  sdsetup();
  thcsetup();
  rtcinit();

  rtcset(2026,8,27,11,0,0);

  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);

  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  
  // Cabeçalho CSV
  sd_write("log.csv", "data,hora,umidade_%_1,umidade_%_2,umidade_%_3,umidade_%_4");
}

void loop() {

  for (int i=4;i<8;i++){
    digitalWrite(i, HIGH);
    umidade[i-4] = thc_humid(i-3);
    delay(500);
    digitalWrite(i, LOW);
  }

  snprintf(umidades, sizeof(umidades),
         "Umidades: %.2f %, %.2f %, %.2f %, %.2f %",
         umidade[0],
         umidade[1],
         umidade[2],
         umidade[3]);
  
  Serial.println(umidades);
  
  sprintf(linhaCSV,
          "%s,%.02f,%.02f,%.02f,%.02f",
          rtcnow(),
          umidade[0],
          umidade[1],
          umidade[2],
          umidade[3]);
  
  // buffer
  buffer[bufferIndex++] = String(linhaCSV);

  // grava no SD
  if (bufferIndex >= BUFFER_SIZE) {
    sd_buffer("log.csv", bufferIndex, buffer);
    bufferIndex = 0;
  }

  delay(1000);
}
