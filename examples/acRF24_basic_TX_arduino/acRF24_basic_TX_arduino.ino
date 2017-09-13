// acRF24_basic_TX_arduino.ino
/*
 * Copyright (c) 2017 by Acácio Neimar de Oliveira <neimar2009@gmail.com>
 */

#include <acRF24.h>

#define print(x) (Serial.print((x)));

uint8_t c = 0;

const uint8_t pipeSufixo[] = { 'A', 'B', 'C', 'D'};//-> 4 bytes excedentes de RX_ADDR_P0 e TX_ADDR.
const uint8_t radioIDs[]   = { 1, 2, 3, 4, 5, 6};  //-> IDs dos rádio que estarão no rádio enlace.
const uint8_t RFchannel    = 4;  // Canal de comunicação.
const uint8_t radioID      = 3;  // ID do rádio.

acRF24Class radio(radioID, 7, 6);/*
  1 - radioID, csn, ce, irq;
  2 - radioID, csn, ce;
  3 - radioID, csn;
  4 - radioID.
  'radioID' é a identificação deste rádio, é enviado junto 
  com a mensagem quando o modo fan-out entiver ativo.
  Isto dá a identificação da procedência da mensagem. */

char t[] = "Teste 1234567890";

//-----------------------------------------------------------------
//  Melhor é ser redundante que ser relapso.
//-----------------------------------------------------------------

void setup() {
  
  Serial.begin(230400);//115200);//57600);
  initRadio();
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
  radio.setRadios(radioIDs,sizeof(radioIDs));

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

  /*  Define qual rádio será alvo da transmissão.
    Lembrar que se o alvo da transmissão for o próprio rádio transmissor,
    o sinal será enviado para si mesmo e será perdido. */
  radio.setTXradio(4);

  /* Entra em modo de transmissão (TX):
    Se o mode de transmissão já estiver ativo, ao reativá-lo o
    comando não disperdiçará processo ou recurso de memória. */
  radio.setModeTX();

  // Verifica se 'radio' está livre para nova transmissão.
  if (radio.isAvailableTX()) {

    // Prenche o payload com a mensagem desejada.
    // O tamanho será armazenado pelo rádio para upload.
    radio.setPayload( t, sizeof(t));

    // TESTE: Imprime payload para conferência.
    print("Mensagem [")
    print((char*)radio.payload);

    // Envia a mensagem e captura status em 'c'.
    c = radio.wTXpayload();

    // TESTE: Imprime o aviso de envio.
    print("] enviada.");
    print(" Status: ");
    print(c);
    print("\n");
  } 

  // 'delay()' simula tempo decorrido de qualquer outro processo necessário.
  delay(800);
}


