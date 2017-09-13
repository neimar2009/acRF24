// acRF24_basic_RX_attiny.ino
/*
 * Copyright (c) 2017 by Acácio Neimar de Oliveira <neimar2009@gmail.com>
 */

#include <acRF24.h>

uint8_t greenLED = 3; // LED indicardor de envio.
uint8_t redLED = 4;   // LED indicador de falha.
uint8_t w = 0;

const uint8_t pipeSufixo[] = { 'A', 'B', 'C', 'D'};//-> 4 bytes excedentes de RX_ADDR_P0 e TX_ADDR.
const uint8_t radioIDs[]   = { 1, 2, 3, 4, 5, 6};  //-> IDs dos rádio que estarão no rádio enlace.
const uint8_t RFchannel    = 4;  // Canal de comunicação.
const uint8_t radioID      = 4;  // ID do rádio.

acRF24Class radio(radioID);/*
    1 - radioID, csn, ce, irq;
    2 - radioID, csn, ce;
    3 - radioID, csn;
    4 - radioIDs
  radioID é a identificação deste rádio, é enviado junto 
  com a mensagem quando o modo fan-out entiver ativo.
  Isto dá a identificação da procedência da mensagem. */

char t[] = "Teste 1234567890";

//-----------------------------------------------------------------
//  Melhor é ser redundante que ser relapso.
//-----------------------------------------------------------------

void setup() {

  initRadio();
  
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  // Tem o propósito de indicar a inicialização.
  // Sem a indicação há o indício de meu funcionamento
  // no sistema suprimido de fios. Verifique se o 
  // resistor e o capacitor são os indicados.
  for (int i = 0; i < 3; ++i) {
    flashLED(redLED, 200);
    delay(200);
  }
}

void initRadio() {
  /** Usar a mesma configuração para todos os rádios. **/
  /** Use the same setting for all radios. **/

  // Inicia 'radio'.
  radio.begin();

  // Define novo sufixo dos pipes, 'RX_ADDR_P0' e 'TX_ADDR'.
  radio.setSufixo(pipeSufixo);
  
  /*  Identificação:
    Registra os IDs dos rádios substituindo os existentes.
    Valores válidos de identificação, de 1 a 254. */
  radio.setRadios(radioIDs, sizeof(radioIDs));

  //  Canal de operação.
  radio.setRFchannel(RFchannel);
  
  /*  Potência de saída:
    PA_18dbm,   // Output -18 dbm 
    PA_12dbm,   // Output -12 dbm 
    PA_6dbm,    // Output  -6 dbm 
    PA_0dbm,    // Output   0 dbm 
    PA_5dbm     // Output   5 dbm */
  radio.setPApower(PA_0dbm);

  /*  Velocidade de transmissão:
    DR_1Mbps         // 1Mbps
    DR_2Mbps         // 2Mbps
    para __SE8R01__
      DR_500Kbps     // 500Kbps
    para __nRF24L01P__
      DR_250Kbps     // 250Kbps
    DR_LOWKbps       // 250Kbps ou 500Kbps */
  radio.setDataRate(DR_1Mbps);

  //-- Configuração RX --

  // Habilita tamanho dinâmico para pipe 0 e pipe 1.
  radio.setDynamicPayload();

  // Abilita AutoAckknowledgement para os pipes 0 e 1.
  radio.setAutoAcknowledgement(true);

  // Modo Fan-Out tráz o identifidor do rádio que enviou a mensagem.
  radio.enableFanOut(true);

  //=== Configuração de TX =======
  // TX é automática

  /*  Auto retransmissão:
    Define a quantidade de auto retransmissão.
    O intervalo entre retransmissões é calculada automaticamente
    de acordo com o especificado no manual, não sendo necessário
    usar 'setAutoRetransmission()' */
  radio.setAutoRetransmissionCount(15);

  /*  setAutoRetransmission:
    Após 'setAutoRetransmission()' é possível definir
    o intervalo de retransmissão através de: */
  // setAutoRetransmissionDelay(intervalo);

  /* watchTX:
    Quando o rádio receptor cai, por longo período, ACK não
    retorna, fazendo o transmissor ficar inoperante.
    'watchTX()' define o tempo em milesegundos que o transmissor
    ficará reenviando o conteúdo dos 'fifo' esperando um ACK,
    após este tempo 'flushTX()' é chamado, e assim libera o
    transmissor para operar com outros rádios. */
  radio.watchTX(2000); // <- Tentativa de comunicação por 2 segundos.
}

void loop() {

  /* Entra em modo de recepção (RX):
    Se o mode de recepção já estiver ativo, ao reativá-lo o
    comando não disperdiçará processo ou recurso de memória. */
  radio.setModeRX();

  // Verifica se 'radio' tem mensagem recebida.
  if(radio.isAvailableRX()){

    // Lê a mensagem recebida.
    radio.rRXpayload();

    // Compara se é a mesma mensagem contida em 't' e define
    // 'w' para identificação visual através do LED verde.
    // Acendimento curto indica mensagem igual a 't[]'.
    // Acendimento longo indica mensagem diferente de 't[]'.
    (strcmp(radio.payload, t) == 0) ? w = 100 : w = 1000;

    flashLED(greenLED, w);//<- Sinaliza o resultado.
  } else {
    flashLED(redLED, 100);// Indica que não há mensagem.
  }
  delay(800); // Simula tempo de outros processos.
}

void flashLED( uint8_t pinLED, uint32_t ms) {

  digitalWrite(pinLED, HIGH);
  delay(ms);
  digitalWrite(pinLED, LOW);  
}

// Atraso CSn e esqumático
  /****************************************************************************
  '''    
    #define T_PECSN2OFF     220 // Capacitance in pF, time in milliseconds.
                                // Resistência externa escolhida  : 2200Ω
                                // Capacitor escolhido por padrão : 100nF
                                // 2.2kΩ x 0.0000001uF = 0.00022s -> 220us standby time.
  '''
    Nota: 
    * Ao alterar o valor do resistor, ajuste o valor da diretiva T_PECSN2OFF
      em "acRF24direcrives.h". Sem este ajuste o sistema pode não funcionar,
      ou funcionar com debilidade.    
    * Não é previsto a alteração do valor do capacitor, o ajuste é dado apenas pela
      alteração do resistor. Em caso de alteração deste valor, considere também a 
      necessidade de ajustar T_PECSN2ON. Valores menor que 5 para T_PECSN2ON provoca
      inconsistência ou inoperância no sistem. Favor reportar o resultado.
    * Resistor com valor muito baixo interfere no carregamento do código fonte.
    * Valor de 1kΩ foi testado e funcionou bem. Contudo se faz necessário
      conectá-lo somente após a carga do código fonte, na sequência dar reset.
    * Usar diodo de germânio que dá queda de tensão de 0,2V. Diodo de silício
      o valor mínino de tensão é de 0,6V sendo necessário para o chip 0,3V.
  '''
                                                             //
                                 +----|<|----x--[2k2]--x----|<|---- 5V 
                                 |    1n60   |         |    LED
                                 |           |         |   (red)
                                 |  +---||---x         |          +-----+
                  +-\/-+         |  |  100nF |         |--- CE   3| R R |
      RESET PB5  1|o   |8  Vcc --|--|--------|---------x--- VCC  2| S F |
      NC    PB3  2|    |7  PB2 --x--|--------|------------- SCK  5| E 2 |
      NC    PB4  3|    |6  PB1 -----|--------|------------- MISO 6| 8 4 |
         +- GND  4|    |5  PB0 -----|--------|------------- MOSI 7| R L |
         |        +----+            |        +------------- CSN  4| 0 0 |
         +--------------------------x---------------------- GND  1| 1 1 |
                                                                  +-----+
  '''
  ****************************************************************************/
