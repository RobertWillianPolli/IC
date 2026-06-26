// HD-38

const int SENSOR_AO  = A0;  // saída analógica
const int SENSOR_DO  = A1;  // saída digital
bool serPrint = false;

void hdsetup(void) {

  // Entradas do sensor
  pinMode(SENSOR_DO, INPUT);
}

float hdread(bool converted) {
  int leitura = analogRead(SENSOR_AO);

  // Converte para porcentagem aproximada -- Pensar na calibração
  int umidade = map(leitura, 1024, 0, 0, 100); // (xx, medida seca, medida molhada, xx)

  if (converted){
    return umidade;
  }
  else{
    return leitura;
  }

}
