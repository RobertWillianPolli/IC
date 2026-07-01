// HW-390

void hwsetup(void) {  
  // Entradas do sensor
}

float hwread(bool converted) {
  const int SENSOR_AO  = A0;  // saída analógica
  
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
