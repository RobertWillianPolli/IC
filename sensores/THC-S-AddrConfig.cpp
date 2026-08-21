/*
  ==============================================================
                 THC-S RS485 CONFIGURATOR
                 Arduino UNO R4 Minima
                 TTL485 V2.0 / MAX1348
  ==============================================================

  FUNÇÕES

    [1] Ler endereço do sensor
    [2] Alterar endereço do sensor
    [3] Testar comunicação
    [4] Informações
    [5] Sair/mostrar menu novamente

  COMUNICAÇÃO

    USB:
      Serial
      115200 baud

    RS485:
      Serial1
      4800 baud
      8N1

  PROTOCOLO

    Modbus RTU

  REGISTRO DO SLAVE ID

    0x07D0

  FUNÇÕES MODBUS

    0x03 = Read Holding Register
    0x06 = Write Single Register

  FAIXA DE ENDEREÇOS

    1 ... 254

  ==============================================================

  LIGAÇÃO

    Arduino UNO R4 Minima
          |
          | Serial1
          |
          +---- TTL485
                  |
                  +---- THC-S

  ATENÇÃO

    Não desconecte o sensor durante a alteração do endereço.

    Após alterar o endereço, o sensor poderá deixar de responder
    ao endereço antigo imediatamente.

  ==============================================================
*/


// =============================================================
// CONFIGURAÇÕES
// =============================================================

#define RS485 Serial1

#define MONITOR_BAUD 115200
#define SENSOR_BAUD  4800

#define SLAVE_ID_REGISTER 0x07D0

#define RESPONSE_TIMEOUT 1000


// =============================================================
// CRC16 MODBUS
// =============================================================

uint16_t modbusCRC(uint8_t *data, uint8_t length)
{
  uint16_t crc = 0xFFFF;

  for (uint8_t i = 0; i < length; i++)
  {
    crc ^= data[i];

    for (uint8_t j = 0; j < 8; j++)
    {
      if (crc & 0x0001)
      {
        crc >>= 1;
        crc ^= 0xA001;
      }
      else
      {
        crc >>= 1;
      }
    }
  }

  return crc;
}


// =============================================================
// IMPRIME BYTE EM HEX
// =============================================================

void printByteHex(uint8_t value)
{
  if (value < 0x10)
    Serial.print("0");

  Serial.print(value, HEX);
}


// =============================================================
// IMPRIME FRAME EM HEX
// =============================================================

void printHex(uint8_t *data, uint8_t length)
{
  for (uint8_t i = 0; i < length; i++)
  {
    printByteHex(data[i]);

    if (i < length - 1)
      Serial.print(" ");
  }

  Serial.println();
}


// =============================================================
// LIMPA BUFFER RS485
// =============================================================

void clearRS485()
{
  while (RS485.available())
    RS485.read();
}


// =============================================================
// PAUSA ESPERANDO ENTRADA DO USUÁRIO
// =============================================================

void waitForSerial()
{
  while (!Serial.available())
  {
    delay(10);
  }
}


// =============================================================
// LIMPA BUFFER DO MONITOR SERIAL
// =============================================================

void clearSerial()
{
  while (Serial.available())
    Serial.read();
}


// =============================================================
// RECEBE FRAME RS485
// =============================================================

uint8_t receiveRS485(
  uint8_t *buffer,
  uint8_t maxLength,
  unsigned long timeout
)
{
  uint8_t length = 0;

  unsigned long start = millis();

  bool receiving = false;

  unsigned long lastByte = start;


  while (millis() - start < timeout)
  {
    while (RS485.available())
    {
      receiving = true;

      uint8_t b = RS485.read();

      if (length < maxLength)
      {
        buffer[length++] = b;
      }

      lastByte = millis();
    }


    /*
      Se já recebemos dados e passaram 5 ms sem
      chegar outro byte, consideramos o frame completo.
    */

    if (
      receiving &&
      millis() - lastByte >= 5
    )
    {
      break;
    }
  }

  return length;
}


// =============================================================
// VERIFICA CRC
// =============================================================

bool checkCRC(
  uint8_t *buffer,
  uint8_t length
)
{
  if (length < 3)
    return false;


  uint16_t receivedCRC =
      buffer[length - 2]
      |
      (
        (uint16_t)
        buffer[length - 1]
        << 8
      );


  uint16_t calculatedCRC =
      modbusCRC(
        buffer,
        length - 2
      );


  return (
    receivedCRC ==
    calculatedCRC
  );
}


// =============================================================
// MOSTRA TX
// =============================================================

void showTX(
  uint8_t *data,
  uint8_t length
)
{
  Serial.print("TX: ");

  printHex(
    data,
    length
  );
}


// =============================================================
// MOSTRA RX
// =============================================================

void showRX(
  uint8_t *data,
  uint8_t length
)
{
  Serial.print("RX: ");

  if (length == 0)
  {
    Serial.println("(sem resposta)");
    return;
  }

  printHex(
    data,
    length
  );
}


// =============================================================
// LEITURA DE UM REGISTRO
// =============================================================

bool readRegister(
  uint8_t slave,
  uint16_t registerAddress,
  uint16_t &value
)
{
  uint8_t request[8];


  // -----------------------------------------------------------
  // MONTAGEM DO FRAME
  // -----------------------------------------------------------

  request[0] = slave;

  request[1] = 0x03;

  request[2] =
      registerAddress >> 8;

  request[3] =
      registerAddress & 0xFF;

  request[4] = 0x00;
  request[5] = 0x01;


  // -----------------------------------------------------------
  // CRC
  // -----------------------------------------------------------

  uint16_t crc =
      modbusCRC(
        request,
        6
      );


  request[6] =
      crc & 0xFF;

  request[7] =
      crc >> 8;


  // -----------------------------------------------------------
  // LIMPA RX
  // -----------------------------------------------------------

  clearRS485();


  // -----------------------------------------------------------
  // ENVIA
  // -----------------------------------------------------------

  showTX(
    request,
    8
  );


  RS485.write(
    request,
    8
  );

  RS485.flush();


  // -----------------------------------------------------------
  // RECEBE
  // -----------------------------------------------------------

  uint8_t response[32];


  uint8_t length =
      receiveRS485(
        response,
        sizeof(response),
        RESPONSE_TIMEOUT
      );


  showRX(
    response,
    length
  );


  // -----------------------------------------------------------
  // SEM RESPOSTA
  // -----------------------------------------------------------

  if (length == 0)
  {
    Serial.println(
      "ERRO: sensor não respondeu."
    );

    return false;
  }


  // -----------------------------------------------------------
  // CRC
  // -----------------------------------------------------------

  if (!checkCRC(
        response,
        length
      ))
  {
    Serial.println(
      "ERRO: CRC inválido."
    );

    return false;
  }


  Serial.println(
    "CRC: OK"
  );


  // -----------------------------------------------------------
  // EXCEÇÃO MODBUS
  // -----------------------------------------------------------

  if (
    length >= 5 &&
    (response[1] & 0x80)
  )
  {
    Serial.print(
      "ERRO MODBUS: código 0x"
    );

    printByteHex(
      response[2]
    );

    Serial.println();

    return false;
  }


  // -----------------------------------------------------------
  // ENDEREÇO
  // -----------------------------------------------------------

  if (
    response[0] != slave
  )
  {
    Serial.println(
      "ERRO: endereço inesperado."
    );

    return false;
  }


  // -----------------------------------------------------------
  // FUNÇÃO
  // -----------------------------------------------------------

  if (
    response[1] != 0x03
  )
  {
    Serial.println(
      "ERRO: função inesperada."
    );

    return false;
  }


  // -----------------------------------------------------------
  // TAMANHO
  // -----------------------------------------------------------

  if (
    length != 7
  )
  {
    Serial.println(
      "ERRO: tamanho da resposta inesperado."
    );

    return false;
  }


  // -----------------------------------------------------------
  // BYTE COUNT
  // -----------------------------------------------------------

  if (
    response[2] != 0x02
  )
  {
    Serial.println(
      "ERRO: quantidade de bytes inesperada."
    );

    return false;
  }


  // -----------------------------------------------------------
  // VALOR
  // -----------------------------------------------------------

  value =
      (
        (uint16_t)
        response[3]
        << 8
      )
      |
      response[4];


  return true;
}


// =============================================================
// ESCREVE UM REGISTRO
// =============================================================

bool writeRegister(
  uint8_t slave,
  uint16_t registerAddress,
  uint16_t value
)
{
  uint8_t request[8];


  // -----------------------------------------------------------
  // MONTAGEM
  // -----------------------------------------------------------

  request[0] = slave;

  request[1] = 0x06;

  request[2] =
      registerAddress >> 8;

  request[3] =
      registerAddress & 0xFF;

  request[4] =
      value >> 8;

  request[5] =
      value & 0xFF;


  // -----------------------------------------------------------
  // CRC
  // -----------------------------------------------------------

  uint16_t crc =
      modbusCRC(
        request,
        6
      );


  request[6] =
      crc & 0xFF;

  request[7] =
      crc >> 8;


  // -----------------------------------------------------------
  // LIMPA RX
  // -----------------------------------------------------------

  clearRS485();


  // -----------------------------------------------------------
  // ENVIA
  // -----------------------------------------------------------

  showTX(
    request,
    8
  );


  RS485.write(
    request,
    8
  );

  RS485.flush();


  // -----------------------------------------------------------
  // RECEBE
  // -----------------------------------------------------------

  uint8_t response[32];


  uint8_t length =
      receiveRS485(
        response,
        sizeof(response),
        RESPONSE_TIMEOUT
      );


  showRX(
    response,
    length
  );


  // -----------------------------------------------------------
  // SEM RESPOSTA
  // -----------------------------------------------------------

  if (length == 0)
  {
    Serial.println(
      "ERRO: sensor não respondeu à escrita."
    );

    return false;
  }


  // -----------------------------------------------------------
  // CRC
  // -----------------------------------------------------------

  if (!checkCRC(
        response,
        length
      ))
  {
    Serial.println(
      "ERRO: CRC inválido."
    );

    return false;
  }


  Serial.println(
    "CRC: OK"
  );


  // -----------------------------------------------------------
  // EXCEÇÃO MODBUS
  // -----------------------------------------------------------

  if (
    length >= 5 &&
    (response[1] & 0x80)
  )
  {
    Serial.print(
      "ERRO MODBUS: código 0x"
    );

    printByteHex(
      response[2]
    );

    Serial.println();

    return false;
  }


  // -----------------------------------------------------------
  // RESPOSTA DE FUNÇÃO 06
  //
  // Para uma escrita válida, o sensor deve devolver
  // exatamente o frame enviado.
  // -----------------------------------------------------------

  if (
    length != 8
  )
  {
    Serial.println(
      "ERRO: tamanho da resposta inesperado."
    );

    return false;
  }


  for (uint8_t i = 0; i < 8; i++)
  {
    if (
      response[i] !=
      request[i]
    )
    {
      Serial.println(
        "ERRO: resposta diferente do comando enviado."
      );

      return false;
    }
  }


  Serial.println(
    "ESCRITA: OK"
  );


  return true;
}


// =============================================================
// LER ENDEREÇO
// =============================================================

bool getSensorAddress(
  uint8_t address,
  uint16_t &sensorAddress
)
{
  return readRegister(
    address,
    SLAVE_ID_REGISTER,
    sensorAddress
  );
}


// =============================================================
// MENU — LER ENDEREÇO
// =============================================================

void menuReadAddress()
{
  Serial.println();
  Serial.println(
    "============================================"
  );

  Serial.println(
    "             LER ENDEREÇO"
  );

  Serial.println(
    "============================================"
  );

  Serial.println();

  Serial.println(
    "Digite o endereço que deseja consultar."
  );

  Serial.println(
    "Normalmente: 1"
  );

  Serial.print(
    "> "
  );


  waitForSerial();

  int address =
      Serial.parseInt();

  clearSerial();


  if (
    address < 1 ||
    address > 254
  )
  {
    Serial.println();
    Serial.println(
      "ERRO: endereço deve estar entre 1 e 254."
    );

    return;
  }


  Serial.println();

  Serial.print(
    "Consultando endereço "
  );

  Serial.print(
    address
  );

  Serial.println(
    "..."
  );

  Serial.println();


  uint16_t value;


  if (
    getSensorAddress(
      address,
      value
    )
  )
  {
    Serial.println();

    Serial.println(
      "--------------------------------------------"
    );

    Serial.print(
      "ENDEREÇO LIDO = "
    );

    Serial.println(
      value
    );

    Serial.println(
      "--------------------------------------------"
    );
  }
}


// =============================================================
// MENU — ALTERAR ENDEREÇO
// =============================================================

void menuChangeAddress()
{
  Serial.println();
  Serial.println(
    "============================================"
  );

  Serial.println(
    "            ALTERAR ENDEREÇO"
  );

  Serial.println(
    "============================================"
  );

  Serial.println();


  // -----------------------------------------------------------
  // ENDEREÇO ATUAL
  // -----------------------------------------------------------

  Serial.println(
    "Digite o endereço ATUAL do sensor:"
  );

  Serial.print(
    "> "
  );


  waitForSerial();

  int currentAddress =
      Serial.parseInt();

  clearSerial();


  if (
    currentAddress < 1 ||
    currentAddress > 254
  )
  {
    Serial.println();
    Serial.println(
      "ERRO: endereço inválido."
    );

    return;
  }


  // -----------------------------------------------------------
  // NOVO ENDEREÇO
  // -----------------------------------------------------------

  Serial.println();

  Serial.println(
    "Digite o NOVO endereço:"
  );

  Serial.print(
    "> "
  );


  waitForSerial();

  int newAddress =
      Serial.parseInt();

  clearSerial();


  if (
    newAddress < 1 ||
    newAddress > 254
  )
  {
    Serial.println();
    Serial.println(
      "ERRO: endereço deve estar entre 1 e 254."
    );

    return;
  }


  if (
    currentAddress ==
    newAddress
  )
  {
    Serial.println();

    Serial.println(
      "ERRO: os endereços são iguais."
    );

    return;
  }


  // -----------------------------------------------------------
  // CONFIRMAÇÃO
  // -----------------------------------------------------------

  Serial.println();

  Serial.println(
    "============================================"
  );

  Serial.println(
    "              CONFIRMAÇÃO"
  );

  Serial.println(
    "============================================"
  );

  Serial.println();

  Serial.print(
    "Endereço atual : "
  );

  Serial.println(
    currentAddress
  );

  Serial.print(
    "Novo endereço  : "
  );

  Serial.println(
    newAddress
  );

  Serial.println();

  Serial.println(
    "ATENÇÃO:"
  );

  Serial.println(
    "Após a alteração, o sensor deixará de"
  );

  Serial.println(
    "responder ao endereço antigo."
  );

  Serial.println();

  Serial.print(
    "Confirmar? Digite S para SIM ou N para NÃO: "
  );


  waitForSerial();

  char confirmation =
      Serial.read();

  clearSerial();


  if (
    confirmation != 'S' &&
    confirmation != 's'
  )
  {
    Serial.println();

    Serial.println(
      "Operação cancelada."
    );

    return;
  }


  // -----------------------------------------------------------
  // VERIFICA ENDEREÇO ATUAL ANTES DE ESCREVER
  // -----------------------------------------------------------

  Serial.println();

  Serial.println(
    "Verificando comunicação com o sensor..."
  );

  Serial.println();


  uint16_t detectedAddress;


  if (
    !getSensorAddress(
      currentAddress,
      detectedAddress
    )
  )
  {
    Serial.println();

    Serial.println(
      "ERRO: o sensor não respondeu ao endereço atual."
    );

    Serial.println(
      "Nenhuma alteração foi realizada."
    );

    return;
  }


  Serial.print(
    "Sensor confirmou endereço = "
  );

  Serial.println(
    detectedAddress
  );


  if (
    detectedAddress !=
    currentAddress
  )
  {
    Serial.println();

    Serial.println(
      "ERRO: endereço retornado não corresponde"
    );

    Serial.println(
      "ao endereço informado."
    );

    return;
  }


  // -----------------------------------------------------------
  // ALTERAÇÃO
  // -----------------------------------------------------------

  Serial.println();

  Serial.println(
    "Gravando novo endereço..."
  );

  Serial.println();


  bool success =
      writeRegister(
        currentAddress,
        SLAVE_ID_REGISTER,
        newAddress
      );


  if (!success)
  {
    Serial.println();

    Serial.println(
      "============================================"
    );

    Serial.println(
      "          ALTERAÇÃO NÃO CONFIRMADA"
    );

    Serial.println(
      "============================================"
    );

    return;
  }


  // -----------------------------------------------------------
  // ESPERA SENSOR
  // -----------------------------------------------------------

  /*
    Pequena pausa para o sensor aplicar a alteração.
  */

  delay(300);


  // -----------------------------------------------------------
  // TESTA NOVO ENDEREÇO
  // -----------------------------------------------------------

  Serial.println();

  Serial.println(
    "Testando o novo endereço..."
  );

  Serial.println();


  uint16_t verification;


  bool newAddressOK =
      getSensorAddress(
        newAddress,
        verification
      );


  if (
    newAddressOK &&
    verification == newAddress
  )
  {
    Serial.println();

    Serial.println(
      "============================================"
    );

    Serial.println(
      "         ALTERAÇÃO CONCLUÍDA"
    );

    Serial.println(
      "============================================"
    );

    Serial.println();

    Serial.print(
      "Endereço anterior : "
    );

    Serial.println(
      currentAddress
    );

    Serial.print(
      "Novo endereço     : "
    );

    Serial.println(
      newAddress
    );

    Serial.println();

    Serial.println(
      "O sensor está respondendo no novo endereço."
    );
  }
  else
  {
    Serial.println();

    Serial.println(
      "============================================"
    );

    Serial.println(
      "      ATENÇÃO: NOVO ENDEREÇO NÃO TESTADO"
    );

    Serial.println(
      "============================================"
    );

    Serial.println();

    Serial.println(
      "A escrita foi confirmada pelo sensor,"
    );

    Serial.println(
      "mas não foi possível validar o novo endereço."
    );

    Serial.println();

    Serial.println(
      "Não altere a ligação do sensor."
    );
  }
}


// =============================================================
// TESTE DE COMUNICAÇÃO
// =============================================================

void menuTest()
{
  Serial.println();
  Serial.println(
    "============================================"
  );

  Serial.println(
    "          TESTE DE COMUNICAÇÃO"
  );

  Serial.println(
    "============================================"
  );

  Serial.println();

  Serial.println(
    "Digite o endereço do sensor:"
  );

  Serial.print(
    "> "
  );


  waitForSerial();

  int address =
      Serial.parseInt();

  clearSerial();


  if (
    address < 1 ||
    address > 254
  )
  {
    Serial.println();

    Serial.println(
      "ERRO: endereço inválido."
    );

    return;
  }


  Serial.println();

  Serial.print(
    "Testando endereço "
  );

  Serial.print(
    address
  );

  Serial.println(
    "..."
  );

  Serial.println();


  uint16_t value;


  if (
    getSensorAddress(
      address,
      value
    )
  )
  {
    Serial.println();

    Serial.println(
      "COMUNICAÇÃO: OK"
    );

    Serial.print(
      "Registro 0x07D0 = "
    );

    Serial.println(
      value
    );
  }
  else
  {
    Serial.println();

    Serial.println(
      "COMUNICAÇÃO: FALHOU"
    );
  }
}


// =============================================================
// INFORMAÇÕES
// =============================================================

void menuInformation()
{
  Serial.println();

  Serial.println(
    "============================================"
  );

  Serial.println(
    "              INFORMAÇÕES"
  );

  Serial.println(
    "============================================"
  );

  Serial.println();

  Serial.println(
    "Controlador : Arduino UNO R4 Minima"
  );

  Serial.println(
    "Interface   : TTL485 V2.0 / MAX1348"
  );

  Serial.println(
    "Protocolo   : Modbus RTU"
  );

  Serial.println(
    "USB         : 115200 baud"
  );

  Serial.println(
    "RS485       : 4800 baud"
  );

  Serial.println(
    "Formato     : 8N1"
  );

  Serial.println(
    "Registro ID : 0x07D0"
  );

  Serial.println(
    "Leitura     : função 03"
  );

  Serial.println(
    "Escrita     : função 06"
  );

  Serial.println(
    "Endereços   : 1 a 254"
  );

  Serial.println();
}


// =============================================================
// MENU PRINCIPAL
// =============================================================

void showMenu()
{
  Serial.println();

  Serial.println(
    "╔══════════════════════════════════════════╗"
  );

  Serial.println(
    "║          THC-S RS485 CONFIGURATOR        ║"
  );

  Serial.println(
    "╠══════════════════════════════════════════╣"
  );

  Serial.println(
    "║                                          ║"
  );

  Serial.println(
    "║   [1] Ler endereço                      ║"
  );

  Serial.println(
    "║   [2] Alterar endereço                   ║"
  );

  Serial.println(
    "║   [3] Testar comunicação                 ║"
  );

  Serial.println(
    "║   [4] Informações                        ║"
  );

  Serial.println(
    "║                                          ║"
  );

  Serial.println(
    "╚══════════════════════════════════════════╝"
  );

  Serial.println();

  Serial.print(
    "Escolha uma opção: "
  );
}


// =============================================================
// SETUP
// =============================================================

void setup()
{
  // -----------------------------------------------------------
  // USB
  // -----------------------------------------------------------

  Serial.begin(
    MONITOR_BAUD
  );


  // -----------------------------------------------------------
  // RS485
  // -----------------------------------------------------------

  RS485.begin(
    SENSOR_BAUD
  );


  // -----------------------------------------------------------
  // ESPERA USB
  // -----------------------------------------------------------

  delay(2000);


  // -----------------------------------------------------------
  // CABEÇALHO
  // -----------------------------------------------------------

  Serial.println();

  Serial.println(
    "============================================"
  );

  Serial.println(
    "          THC-S RS485 CONFIGURATOR"
  );

  Serial.println(
    "============================================"
  );

  Serial.println();

  Serial.println(
    "Arduino UNO R4 Minima"
  );

  Serial.println(
    "TTL485 V2.0 / MAX1348"
  );

  Serial.println(
    "Modbus RTU"
  );

  Serial.println();

  Serial.println(
    "USB  : 115200 baud"
  );

  Serial.println(
    "RS485: 4800 baud / 8N1"
  );

  Serial.println();

  Serial.println(
    "Registro do endereço: 0x07D0"
  );

  Serial.println();

  showMenu();
}


// =============================================================
// LOOP
// =============================================================

void loop()
{
  if (!Serial.available())
    return;


  char option =
      Serial.read();


  clearSerial();


  switch (option)
  {
    // ---------------------------------------------------------
    // 1 — LER
    // ---------------------------------------------------------

    case '1':

      menuReadAddress();

      break;


    // ---------------------------------------------------------
    // 2 — ALTERAR
    // ---------------------------------------------------------

    case '2':

      menuChangeAddress();

      break;


    // ---------------------------------------------------------
    // 3 — TESTAR
    // ---------------------------------------------------------

    case '3':

      menuTest();

      break;


    // ---------------------------------------------------------
    // 4 — INFORMAÇÕES
    // ---------------------------------------------------------

    case '4':

      menuInformation();

      break;


    // ---------------------------------------------------------
    // OUTRA TECLA
    // ---------------------------------------------------------

    default:

      Serial.println();

      Serial.println(
        "Opção inválida."
      );

      break;
  }


  showMenu();
}
