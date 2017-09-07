// acRF24.cpp
/*
 * Copyright (c) 2017 by Acácio Neimar de Oliveira <neimar2009@gmail.com>
 * acRF24 library.
*/

#include <Arduino.h>
#include "acRF24.h"

#ifndef PIN_SPI_SCK
  #if defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    //#define PIN_SPI_SS    (3)
    #define PIN_SPI_MOSI  (6)
    #define PIN_SPI_MISO  (5)
    #define PIN_SPI_SCK   (4)
  #elif defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    //#define PIN_SPI_SS    (3)
    #define PIN_SPI_MOSI  (0)
    #define PIN_SPI_MISO  (1)
    #define PIN_SPI_SCK   (2)
  #endif
  #ifdef PIN_SPI_SCK
    //static const uint8_t SS   = PIN_SPI_SS;
    static const uint8_t MOSI = PIN_SPI_MOSI;
    static const uint8_t MISO = PIN_SPI_MISO;
    static const uint8_t SCK  = PIN_SPI_SCK;
  #endif
#endif

#define MOSI_MSBFIRST(_bit)  {digitalWrite(MOSI, (_bit & 0x80) ? HIGH : LOW); }
#define MISO_MSBFIRST(_byte) {digitalRead(MISO) ? _byte |= 1 : _byte &= 0xFE; }

// TODO: Verificar a validade de 'SPI_CLOCK_LIMIT'
#define SPI_CLOCK_LIMIT ((uint32_t)(F_CPU / SPI_RF24_LIMIT))

uint8_t SPI_transfer(uint8_t data) {

  for(int i=0; i<8; i++){     // output 8-bit
    MOSI_MSBFIRST(data);      // Ajusta a porta MOSI com o bit mais significativo de data.
    delayMicroseconds(SPI_CLOCK_LIMIT); // Tempo de preparação do bit, pelo mastre e pelo escravo.
    digitalWrite(SCK, HIGH);  // Set SCK high. Send the bit
    data <<= 1;               // shift next bit into MSB..
    MISO_MSBFIRST(data);      // capture MISO bit
    delayMicroseconds(SPI_CLOCK_LIMIT);	// Tempo de cópia do bit, pelo mastre e pelo escravo.
    digitalWrite(SCK, LOW);   // SCK clock down. Reset.
  }
  return data;                // return read unsigned char
}

//== Inicialização ==========================================================

acRF24Class::acRF24Class(u8 selfID, u8 CSpin = xFF, u8 CEpin = xFF, u8 IRQpin = xFF) :
  pv_selfID(selfID), CS(CSpin), CE(CEpin), IRQ(IRQpin) {
  // --  
  flagState(ACTIVED_CS, CSpin != xFF);
  flagState(ACTIVED_CE, CEpin != xFF);
  flagState(ACTIVED_IRQ, IRQpin != xFF);
}

void acRF24Class::begin() {

  pinMode(SCK, OUTPUT);
  pinMode(MOSI, OUTPUT);
  pinMode(MISO, INPUT);
  //
  digitalWrite(SCK, LOW);
  digitalWrite(MOSI, LOW);
  //
  if (flagState(ACTIVED_CS)) {
    pinMode(CS, OUTPUT);
    digitalWrite(CS, HIGH);
    flagState(SELECTED, false);
  } else {
	  setCS(false);
	}
  //
  if(flagState(ACTIVED_CE)) {
    pinMode(CE, OUTPUT);
    digitalWrite(CE, LOW);
  } else {
    flagState(ENABLED, true);
  }
  //
  if (flagState(ACTIVED_IRQ)) pinMode(IRQ, INPUT);

  // Espera até que o chip esteja ativo.
  do {
     delayMicroseconds(T_POWERON);
  } while(!chipActived());

  // TODO: Revisar, e implementar
  resetConfig();

  //----------------------------------------------
  // Atualizações das variáveis indicadoras.
  //----------------------------------------------

  // Preenche 'sufixo[]'.
  rRegister(RX_ADDR_P0);
  setSufixo(recData+1);

  // Registra a identificação do rádio.
  recData[0] = getSelfID();
  wRegister(RX_ADDR_P1);

  // Captura os rádios registrados.
  for (uint8_t i = 0; i < 6; ++i) {
    rRegister(RX_ADDR_P0 + i);
    setRadioID(i, recData[0]);
  }

  // Atualisa o canal ativo. 
  pv_RFchannel = rRegister(RF_CH) & RF_CH__REG_RF_CH;
  
  //----------------------------------------------
  // config
  //----------------------------------------------

  #ifdef __SE8R01__

    configBank1();
  #elif defined __nRF24L01P__
	
    toggleFeature();
	#endif

	setPowerDown();
}

//-- Privados

void acRF24Class::resetConfig() {

  recData[0] = CONFIG__EN_CRC | CONFIG__CRCO;
  wRegister(CONFIG);      		// Address: 00h

  recData[0] = _SETUP_AW__AW5BYTES;
  wRegister(SETUP_AW);    		// Address: 03h

  recData[0] = STATUS__MAX_RT | STATUS__TX_DS | STATUS__RX_DR;
  wRegister(STATUS);					// Address: 07h

  recData[0] = 0;
  wRegister(DYNPD); 					// Address: 1Ch

  pv_flagState &= 0x00FF;     // <- Don't remove zeros
  recData[0] = 0;
  wRegister(FEATURE); 				// Address: 1Dh

  // Calibração
	rRegister(SETUP_VALUE);
	recData[3] = 0x80; // Main band gap wait counter. Default: 0x10 (16us)
	wRegister(SETUP_VALUE);			// Address: 1Eh

	recData[0] = 0x77; // Default: 0x32
	recData[1] = 0;
	wRegister(PRE_GURD);    		// Address: 1Fh

	getMode();
}

#ifdef __SE8R01__   // configBank1()
  void acRF24Class::configBank1() {

  	// -----------------------------------------
  	// Calibração inicial
  	// -----------------------------------------

  	  uint8_t* p = recData;	   
  	  uint8_t dr = getDataRate();
  	  uint8_t m  = getMode();
  	  setPowerDown();
  	  selectBank(BANK1);

  	  p[0] = 0x40; p[1] = 0x00; p[2] = 0x10;
  	  if (dr == DR_2Mbps) 
  	       { p[3] = 0xE6; }
  	  else { p[3] = 0xE4; }
  	  spiTransfer(W_REGISTER | BANK1_PLL_CTL0, p, 4);

  	  // p[0] = 0x20; p[1] = 0x08; p[2] = 0x50; p[3] = 0x40; p[4] = 0x50;
  	  // spiTransfer(W_REGISTER | BANK1_CAL_CTL,  p, 5);

  	  p[0] = 0x00; p[1] = 0x00;
  	  if (dr == DR_2Mbps)
  	       { p[2]=0x1E;}
  	  else { p[2]=0x1F;}
  	  spiTransfer(W_REGISTER | BANK1_IF_FREQ, p, 3);

  	  if (dr == DR_2Mbps)
  	       { p[0] = 0x29;}
  	  else { p[0] = 0x14;}
  	  spiTransfer(W_REGISTER | BANK1_FDEV, p, 1);
    /*
  	  p[0] = 0x00;
  	  spiTransfer(W_REGISTER | BANK1_DAC_CAL_LOW, p, 1);

  	  p[0] = 0x7F;
  	  spiTransfer(W_REGISTER | BANK1_DAC_CAL_HI,  p, 1);
  	*/
  	  // // ?? Pode dar erro
  	  // p[0] = 0x02; p[1] = 0xC1; p[2] = 0xEB; p[3] = 0x1C;// p[4] = 0x01;
  	  // spiTransfer(W_REGISTER | BANK1_AGC_GAIN, p, 4);  // 4 ou 5

  	  // // ?? Pode dar erro
  	  // p[0] = 0x97; p[1] = 0x64; p[2] = 0x00; p[3] = 0x81;// p[4] = 0x1F;
  	  // spiTransfer(W_REGISTER | BANK1_RF_IVGEN, p, 4);  // 4 ou 5
  	
      // - 0F BANK1_CTUNING     [12h] 
      p[0] = 0x12;
      spiTransfer(W_REGISTER | BANK1_CTUNING, p, 1);
      // - 10 BANK1_FTUNING     [02h] [05h] 
      p[0] = 0x02; p[1] = 0x05;
      spiTransfer(W_REGISTER | BANK1_FTUNING, p, 1);
      // - 12 BANK1_FAGC_CTRL   [00h] [40h] [A3h]
      p[0] = 0x00; p[1] = 0x40; p[2] = 0xA3;
      spiTransfer(W_REGISTER | BANK1_FAGC_CTRL, p, 1);
  	
  	  selectBank(BANK0);
  	  // setModeTX() e setPowerDown() só funcionam com o banco 0 ativo.
      setModeTX();
      setPowerDown();
      delay(50);       // Tempo para autocalibração.
  	// -----------------------------------------
  	// Inicialização da parte analógica do circuito
  	// -----------------------------------------

  	  selectBank(BANK1);

  	  p[0] = 0x40; p[1] = 0x01; p[2] = 0x30;  
  	  if (dr == DR_2Mbps)
  	       { p[3] = 0xE2;}
  	  else { p[3] = 0xE0;}
  	  spiTransfer(W_REGISTER | BANK1_PLL_CTL0, p, 4);

  	  // p[0] = 0x29; p[1] = 0x89; p[2] = 0x55; p[3] = 0x40; p[4] = 0x50;  
  	  // spiTransfer(W_REGISTER | BANK1_CAL_CTL, p, 5);

  	  if (dr == DR_2Mbps)
  	       { p[0] = 0x29;}
  	  else { p[0] = 0x14;}       
  	  spiTransfer(W_REGISTER | BANK1_FDEV, p, 1);

  	  // !! Dá erro
  	  // p[0] = 0x55; p[1] = 0xC2; p[2] = 0x09; p[3] = 0xAC;
  	  // spiTransfer(W_REGISTER | BANK1_RX_CTRL,     p, 4 );

  	  // perde pacote
  	  // p[0] = 0x00; p[1] = 0x14; p[2] = 0x08; p[3] = 0x29;
  	  // spiTransfer(W_REGISTER | BANK1_FAGC_CTRL_1, p, 4 );

  	  // // ?? Pode dar erro
  	  // p[0] = 0x02; p[1] = 0xC1; p[2] = 0xCB; p[3] = 0x1C;
  	  // spiTransfer(W_REGISTER | BANK1_AGC_GAIN, p, 4 );

  	  // // ?? Pode dar erro
  	  // p[0] = 0x97; p[1] = 0x64; p[2] = 0x00; p[3] = 0x01;
  	  // spiTransfer(W_REGISTER | BANK1_RF_IVGEN, p, 4 );

  	  p[0] = 0x2A; p[1] = 0x04; p[2] = 0x00; p[3] = 0x7D;
  	  spiTransfer(W_REGISTER | BANK1_TEST_PKDET, p, 4 );

  	  selectBank(BANK0);
  	  // goMode() só funciona com o banco 0 ativo.
  	  goMode(m);
  }
#endif

//== Controle de estado do chip ===============================================

void acRF24Class::setCE( bool enable) {

  if (flagState(ACTIVED_CE)) {
    if (flagState(ENABLED) == enable) return;
    digitalWrite(CE, enable);
    flagState(ENABLED, enable);
  }
  if (enable) delayMicroseconds(T_STBY2A);
}

void acRF24Class::setCS( bool select) {

  if (flagState(SELECTED) == select) return;
  flagState(SELECTED, select);

  select = !select; // Select with negative.

  if (flagState(ACTIVED_CS)) {
    digitalWrite(CS, select);
    if (!select) delayMicroseconds(T_PECE2CSN);
  } else {
    digitalWrite(SCK, select);
    select ? delayMicroseconds(T_PECSN2OFF) : delayMicroseconds(T_PECSN2ON + T_PECE2CSN);
  }
}

void acRF24Class::setCSn( bool selectn) {

  setCS(!selectn);
}

void acRF24Class::setPowerDown() {

  if (flagStateCtrl(_MODE__POWERDOWN)) return;
  setFlagStateCtrl(_MODE__POWERDOWN);

  rRegister(CONFIG);
  recData[0] &= ~CONFIG__PWR_UP;
  wRegister(CONFIG);
  setCE(LOW);
}

// Força a entrar no estado de standby-I
void acRF24Class::setStandbyRX() {
  
  if (flagStateCtrl(_MODE__STANDBYRX)) return;
  setFlagStateCtrl(_MODE__STANDBYRX);

  goStandby(RX);
  setCE(LOW);
}

// Força a entrar no estado de standby-II
void acRF24Class::setStandbyTX() {

  if (flagStateCtrl(_MODE__STANDBYTX)) return;
  setFlagStateCtrl(_MODE__STANDBYTX);

  goStandby(TX);
  flushTX();
  setCE(HIGH);
}

void acRF24Class::setModeRX() {

  if (flagStateCtrl(_MODE__MODERX)) return;
  setFlagStateCtrl(_MODE__MODERX);

  goStandby(RX);
  setCE(HIGH);
}

void acRF24Class::setModeTX() {
  
  if (flagStateCtrl(_MODE__MODETX)) return;
  setFlagStateCtrl(_MODE__MODETX);

  goStandby(TX);
  setCE(HIGH);
}

uint8_t acRF24Class::getMode() {

	if (flagState(MODE_STATE_CTRL)) return MODE_STATE_CTRL; // <- Indefinido.
  uint8_t rec = rRegister(CONFIG);
  // Mode: power_down
  if(!(rec & CONFIG__PWR_UP)) {
    setFlagStateCtrl(_MODE__POWERDOWN);
    return _MODE__POWERDOWN;
  }
  // Mode: standby-I -> StandbyRX
  if (!flagState(ENABLED)) {
    setFlagStateCtrl(_MODE__STANDBYRX);
    return _MODE__STANDBYRX;
  }
  // Mode: modeRX
  if(rec & CONFIG__PRIM_RX) {
    setFlagStateCtrl(_MODE__MODERX);
    return _MODE__MODERX;
  }
  // Mode: standby-II -> StandbyTX
  if((rRegister( FIFO_STATUS) & FIFO_STATUS__TX_EMPTY) == FIFO_STATUS__TX_EMPTY) {
    setFlagStateCtrl(_MODE__STANDBYTX);
    return _MODE__STANDBYTX;    
  }
  // Mode: modeTX
  setFlagStateCtrl(_MODE__MODETX);
  return _MODE__MODETX;
}

// -- private --

void acRF24Class::goStandby( bool RXtx) {

	rRegister(CONFIG);
	bool pd = recData[0] & CONFIG__PWR_UP == 0;
  if (RXtx) {
    recData[0] =  recData[0] | CONFIG__PWR_UP  | CONFIG__PRIM_RX;
  } else {
  	recData[0] = (recData[0] | CONFIG__PWR_UP) & ~CONFIG__PRIM_RX;
  }
  wRegister(CONFIG);
  pd ? delayMicroseconds(T_PD2STBY) : delayMicroseconds(T_STBY2A);
  // clearIRQ();
}

void acRF24Class::goMode(uint8_t m) {

  switch (m) {
    case _MODE__POWERDOWN : setPowerDown(); break;
    case _MODE__STANDBYRX : setStandbyRX(); break;
    case _MODE__MODERX    : setModeRX();    break;
    case _MODE__MODETX    : setModeTX();    break;
    case _MODE__STANDBYTX : setStandbyTX(); break;
    default: 
      setFlagStateCtrl(MODE_STATE_CTRL);
    break;
  }
}

//== Comunicação ============================================================

void acRF24Class::spiTransfer(uint8_t cmd, uint8_t* buf, uint8_t amount) {

  setCS(true);  // or setCSn(LOW);
  pv_lastStatus = SPI_transfer(cmd);
  for (int i = 0; i < amount; ++i) {
    buf[i] = SPI_transfer(buf[i]);
  }
  setCS(false); // or setCSn(HIGH);

  buf[amount] = NULL;
}

uint8_t acRF24Class::command(uint8_t cmd) {

	uint8_t* p = recData;

  switch (cmd) {
    case RX_ADDR_P0 :  
    case TX_ADDR :     
    case SETUP_VALUE : 
    case (W_REGISTER | RX_ADDR_P0) :  
    case (W_REGISTER | TX_ADDR) :     
    case (W_REGISTER | SETUP_VALUE) : 
      pv_recAmount = 5;
      break;
    case PRE_GURD :
    case (W_REGISTER | PRE_GURD) :
      pv_recAmount = 2;
      break;
    case R_RX_PAYLOAD:  			// pronto (ready)
    case W_TX_PAYLOAD:  			// pronto (ready)
    case W_ACK_PAYLOAD: 
    case W_TX_PAYLOAD_NO_ACK: // pronto (ready)
    	p = payload;
      break;
    case FLUSH_TX:      			// pronto (ready)
    case FLUSH_RX:      			// pronto (ready)
    case REUSE_TX_PL: 				// pronto (ready)
    case NOP:           			// pronto (ready)
      pv_recAmount = 0;
      break;
    default:
      pv_recAmount = 1;
      break;
  }

  spiTransfer(cmd, p, pv_recAmount);
  if(pv_recAmount == 0) recData[0] = pv_lastStatus;
  return recData[0];
}

//== Comandos Nativo ========================================================

// Retorna STATUS ou valor lido.
uint8_t acRF24Class::rRegister(uint8_t rec){
  // Retorna o valor lido em 'recData' e também
  // diretamente quando o tamanha é de um byte,
  // ou STATUS para retornos deferentes de um byte.
  return command(~R_REGISTER & rec);
}

// Retorna o status
uint8_t acRF24Class::wRegister(uint8_t rec) {

  // Escreve os valores previamente preenchidos em 'recData'.
  return command( W_REGISTER | rec);
}

// Retorna status
uint8_t acRF24Class::Activate(uint8_t cmd) {   // ACTIVATE

	recData[0] = cmd;
	command(ACTIVATE);
	return pv_lastStatus;
}

// Retorna a largura de dados
uint8_t acRF24Class::rRXpayloadWidth() {

	// O retorn é correspondente ao 'pipe' onde entrou os dados.
  uint8_t w = internalRXpayloadWidth();
  if (isFanOut() && w > 0 ) w--;
  return w;
}

// Retorna o tamanho do Payload e o conteúdo passado para o payload[].
uint8_t acRF24Class::rRXpayload() {

  pv_recAmount = internalRXpayloadWidth();
  if(pv_recAmount == 0) return 0;
  command( R_RX_PAYLOAD);  // Carrega payload.
  
  if (isFanOut()) {
    // Ajusta 'payload' em conformidade com 'Fan-Out'.
    pv_sourceID = payload[0];
    // memcpy(payload, payload+1, pv_recAmount);
    for (int i = 0; i < pv_recAmount; ++i) {
      payload[i] = payload[i+1];
    }
  	pv_recAmount--;
  }
  if (pv_recAmount == 0) pv_sourceID = 0;

  #ifdef __nRF24L01P__
	  uint8_t a = pv_recAmount;
	  clearRX_DR();
	  return a;
  #endif

  return pv_recAmount;
}

// Retorna status
uint8_t acRF24Class::wTXpayload() {

  return internal_wTXpayload( W_TX_PAYLOAD);
}

// Ainda não testado
uint8_t acRF24Class::wACKpayload() {      // W_ACK_PAYLOAD

  // TODO: Verificar o possível uso de W_ACK_PAYLOAD para
  //       responder um ACK no modo Fan-Out.
  pv_recAmount = internalTXpayloadWidth(); // <- ????????
  return command(W_ACK_PAYLOAD);
}

// Retorna status
uint8_t acRF24Class::wTXpayloadNoACK() {

  if (isDYN_ACK()) {
    return internal_wTXpayload( W_TX_PAYLOAD_NO_ACK);
  }
  return internal_wTXpayload( W_TX_PAYLOAD);
}

// Retorna status
uint8_t acRF24Class::reuseTXpayload() {     // REUSE_TX_PL

  return command(REUSE_TX_PL);
}

// Retorna status
uint8_t acRF24Class::flushRX() {

  return command(FLUSH_RX);
}

// Retorna status
uint8_t acRF24Class::flushTX() {

  return command(FLUSH_TX);
}

// Retorna status. Usar esta opção para obter status
uint8_t acRF24Class::nop() {          // NOP

  return command(NOP);
}

// -- privado --

uint8_t acRF24Class::internal_wTXpayload( uint8_t wTX) {

  pv_recAmount = internalTXpayloadWidth();
  if (isFanOut()) {
    // Ajusta 'payload' em conformidade com 'Fan-Out'.
    uint8_t i = (pv_recAmount > 31) ? 31 : pv_recAmount ;
    while (i > 0) {
      payload[i] = payload[i-1];
      i--;
    }
    payload[0] = getSelfID();
  }

  command(wTX);

  #ifdef __nRF24L01P__
	  uint8_t a = pv_lastStatus;
	  clearTX_DS();
	  return a;
  #endif

  return pv_lastStatus;
}

//== Configurações ==========================================================

void acRF24Class::setSufixo(uint8_t* buf) {

  // for (int i = 0; i < 4; ++i) {
  //   pv_sufixo[i] = buf[i];
  //   recData[i+1] = pv_sufixo[i];
  // }
  memcpy(pv_sufixo, buf, 4);

  rRegister(RX_ADDR_P0); // Assegura que não se perca o rádio registrado em RX_ADDR_P0
  getSufixo(&recData[1]);
  wRegister(RX_ADDR_P0);

  rRegister(TX_ADDR);
  getSufixo(&recData[1]);
  wRegister(TX_ADDR);

  #ifdef __nRF24L01P__
    rRegister(RX_ADDR_P1);// Assegura que não se perca o rádio registrado em RX_ADDR_P1
    getSufixo(&recData[1]);
    wRegister(RX_ADDR_P1);
  #endif
}

void acRF24Class::getSufixo(uint8_t* buf) {

  // for (int i = 0; i < 4; ++i) {
  //   buf[i] = pv_sufixo[i];
  // }
  memcpy(buf, pv_sufixo, 4);
}

void acRF24Class::setPayload(uint8_t* buf, uint32_t len) {

  // TODO: Verificar a necessidade deste 'if'.
  if (len > 32) len = 32;
  setTXpayloadWidth(len);

  memcpy( payload, buf, len);
  payload[len] = NULL;
}

void acRF24Class::getPayload(void* buf, uint32_t len) {
  
  // TODO: Testar e verificar a necessidade deste 'if'.
  if (len > 32) len = 32;
  memcpy( buf, payload, len);
}

void acRF24Class::setRFchannel(uint8_t ch) {

  if (ch > 126) ch = 126;
  if (ch == pv_RFchannel) return;
  pv_RFchannel = ch;
  recData[0] = ch;
  wRegister(RF_CH);
}

uint8_t acRF24Class::getRFchannel() {

	return rRegister(RF_CH);
}

void acRF24Class::setPApower(ePA pa) {

  //if (pa < PA_18dbm || pa > PA_5dbm) return;
  rRegister(RF_SETUP);
  recData[0] &= ~(RF_SETUP__PA_PWR); // Limpa os bits para receber o novo valor.
  switch (pa) {
    case PA_18dbm :
      recData[0] |= _RF_SETUP__PA_PWR_18dbm; // Output -18dbm
      break;
    case PA_12dbm :
      recData[0] |= _RF_SETUP__PA_PWR_12dbm; // Output -12dbm
      break;
    case PA_6dbm:
      recData[0] |= _RF_SETUP__PA_PWR_6dbm;  // Output -6dbm
      break;
    case PA_0dbm:
      recData[0] |= _RF_SETUP__PA_PWR_0dbm;  // Output 0dbm
      break;
    default:
      recData[0] |= _RF_SETUP__PA_PWR_5dbm;  // Output 5dbm or 0dbm
    break;
  }
  wRegister(RF_SETUP);
}

ePA acRF24Class::getPApower() {

  rRegister(RF_SETUP);
  uint8_t pa = recData[0] & RF_SETUP__PA_PWR;
  switch (pa) {
    case _RF_SETUP__PA_PWR_18dbm:
      return PA_18dbm;
    case _RF_SETUP__PA_PWR_12dbm:
      return PA_12dbm;
    case _RF_SETUP__PA_PWR_6dbm:
      return PA_6dbm;
    case _RF_SETUP__PA_PWR_0dbm:
      return PA_0dbm;
    default:
      return PA_5dbm;
  }
}

void acRF24Class::setDataRate(eDataRate dr) {

  // TODO: Verificar a necessidade deste 'if'.
  if (dr < DR_1Mbps && dr > DR_LOWKbps) return;

  rRegister(RF_SETUP);
  recData[0] &= ~(RF_SETUP__RF_DR); // Limpa os bits para receber o novo valor.
  switch (dr) {
    /*
    case DR_1Mbps:     //<-  0
      recData[0] |= _RF_SETUP__RF_DR_1Mbps;
      break;
    */
    case DR_2Mbps:     //<-  1
      recData[0] |= _RF_SETUP__RF_DR_2Mbps;
      break;
    /*
		#ifdef __SE8R01__
    case DR_500Kbps :  //<-  2
      recData[0] |= _RF_SETUP__RF_DR_500Kbps;
      break;
		#elif defined __nRF24L01P__
    case DR_250Kbps :  //<-  2
      recData[0] |= _RF_SETUP__RF_DR_250Kbps;
      break;
    #endif
    */  
    case DR_LOWKbps :  //<-  2
      #ifdef __SE8R01__
      recData[0] |= _RF_SETUP__RF_DR_500Kbps;
      #elif defined __nRF24L01P__
      recData[0] |= _RF_SETUP__RF_DR_250Kbps;
      #endif
      break;
    default:
      // Não faz nada. Mantém _RF_SETUP__RF_DR_1Mbps que é zero.
    break;
  }
  wRegister(RF_SETUP);

  //* Aqui é feiro uma auto configuração do tempo de espera
  //* em conformidade com a largura de banda escolhida.
  dr < 2 ? dr = 4 : dr = 6; //<- 256 * 2 = 1024uS or 256 * 6 = 1536uS  
  setAutoRetransmissionDelay(dr);

	#ifdef __SE8R01__
	  configBank1();
	#endif
}

eDataRate acRF24Class::getDataRate() {

  // Retorna:
  // DR_1Mps   = 0,  // 1Mps
  // DR_2Mps   = 1,  // 2Mps
  // DR_500Kps = 2,  // 500Kps ou 250Kps
  // DR_250Kps = 2,  // 500Kps ou 250Kps

  // 0010 1000  <- __SE8R01__
  uint8_t dr = (rRegister(RF_SETUP) & RF_SETUP__RF_DR) >> 3;
  uint8_t drl = dr >> 1;
  dr = (dr & 2) | drl;
  if(dr == 3) dr = 2;   //<- Por razão de compatibilidade.
  return dr;
}

//-- Configurações de modo de operação ----------------------------------------

//-- RX --

void acRF24Class::setModePayload(uint8_t pipe, bool dyn, uint8_t len = 0) {

  // The payload length on the transmitter side is set by the
  // number of bytes clocked into the TX_FIFO and must equal
  // the value in the RX_PW_Px register on the receiver side.

  if (isFanOut() && !dyn) len++;

  recData[0] = (len > 32) ? 32 : len;
  wRegister( RX_PW_P0 + pipe);

  // Habilita o pipe correspondente.
  enablePipe( pipe, true);

  // Abilita desabilita DYNPD para o pipe requerido.
  rRegister(DYNPD);
  dyn ? bitSet(recData[0], pipe)
      : bitClear(recData[0], pipe);
  wRegister(DYNPD);
  // TODO: Não encontrei clareza no manual quanto ao procedimento a seguir.
  // ????: Deve ser ativado também no modo RX? Ou sómente no modo TX?
  if (dyn || rRegister(DYNPD) == 0) {
    enableDPL(dyn);
  }
}

void acRF24Class::setStaticPayload(uint8_t pipe, uint8_t len) {

  // The payload length on the transmitter side is set by the
  // number of bytes clocked into the TX_FIFO and must equal
  // the value in the RX_PW_Px register on the receiver side.
	setModePayload(pipe, false, len);
}

bool acRF24Class::isStaticPayload(uint8_t pipe) {

  return (!isDynamicPayload(pipe));
}

// Específico para uso com o modo fan-out
void acRF24Class::setStaticPayload(uint8_t len) {

  // The payload length on the transmitter side is set by the
  // number of bytes clocked into the TX_FIFO and must equal
  // the value in the RX_PW_Px register on the receiver side.
  setModePayload(0, false, len);
  setModePayload(1, false, len);
}

// Específico para uso com o modo fan-out
bool acRF24Class::isStaticPayload() {

  return (!isDynamicPayload(0) && !isDynamicPayload(1));
}

void acRF24Class::setDynamicPayload(uint8_t pipe) {

  setModePayload(pipe, true);
}

bool acRF24Class::isDynamicPayload(uint8_t pipe) {

  return (bitRead(rRegister(DYNPD), pipe));
}

// Específico para uso com o modo fan-out
void acRF24Class::setDynamicPayload() {

  setModePayload(0, true);
  setModePayload(1, true);
}

// Específico para uso com o modo fan-out
bool acRF24Class::isDynamicPayload() {

  return  isDynamicPayload(0) && isDynamicPayload(1);
}

void acRF24Class::setAutoAcknowledgement(uint8_t pipe, bool en) {

	rRegister(EN_AA);
	en ? bitSet(recData[0], pipe)
	   : bitClear(recData[0], pipe);
	wRegister(EN_AA);
  // TODO: Não encontrei clareza no manual quanto ao procedimento a seguir.
  // ????: Deve ser ativado também no RX? Ou sómete no TX?
  if (en || rRegister(EN_AA) == 0) {
    enableACK_PAY(en);
  }
}

bool acRF24Class::isAutoAcknowledgement(uint8_t pipe) {

  return (bitRead(rRegister(EN_AA), pipe));// && (rRegister(FEATURE) & FEATURE__EN_DPL));
}

// Específico para uso com o modo fan-out
void acRF24Class::setAutoAcknowledgement( bool en) {

  // TODO: Para uso com versão futura.  
  setAutoAcknowledgement(0, en);
  setAutoAcknowledgement(1, en);
}

// Específico para uso com o modo fan-out
bool acRF24Class::isAutoAcknowledgement() {

  // TODO: Para uso com versão futura.
  return isAutoAcknowledgement(0) && isAutoAcknowledgement(1);
}

// -- TX --

void acRF24Class::setAutoRetransmissionCount(uint8_t arc) {

  // Auto Retransmit Count
  arc &= SETUP_RETR__ARC;
  rRegister(SETUP_RETR);
  recData[0] = (recData[0] & SETUP_RETR__ARD) | arc;
  wRegister(SETUP_RETR);
}

uint8_t acRF24Class::getAutoRetransmissionCount() {

  return rRegister(SETUP_RETR) & SETUP_RETR__ARC;
}

void acRF24Class::setAutoRetransmissionDelay(uint8_t ard) {

  // Auto Retransmit Delay
  ard <<= 4;
  rRegister(SETUP_RETR);
  recData[0] = (recData[0] & SETUP_RETR__ARC) | ard;
  wRegister(SETUP_RETR);
}

uint8_t acRF24Class::getAutoRetransmissionDelay() {

  return rRegister(SETUP_RETR) & SETUP_RETR__ARD;
}

// -- private --

void acRF24Class::enablePipe(uint8_t pipe, bool en) {

  // Habilita o pipe correspondente.
  rRegister(EN_RXADDR);
  en ? bitSet(recData[0], pipe)
     : bitClear(recData[0], pipe);
  wRegister(EN_RXADDR);	
}

void acRF24Class::enableDYN_ACK(bool en) {

  if (flagState(MODE_DYN_ACK) == en) return;

  rRegister(FEATURE);
  en ? recData[0] |=  (FEATURE__EN_DYN_ACK)
     : recData[0] &= ~(FEATURE__EN_DYN_ACK);
  wRegister(FEATURE);

  flagState(MODE_DYN_ACK, en);
}

bool acRF24Class::isDYN_ACK() {

  return flagState(MODE_DYN_ACK);
}

void acRF24Class::enableDPL(bool en) {

  if (flagState(MODE_DPL) == en) return;
  
  rRegister(FEATURE);
  en ? recData[0] |=  (FEATURE__EN_DPL) 
     : recData[0] &= ~(FEATURE__EN_DPL);
  wRegister(FEATURE);

  // Atualiza o 'state'
  flagState(MODE_DPL, en);
}

bool acRF24Class::isDPL() {

  return flagState(MODE_DPL);
}

void acRF24Class::enableACK_PAY(bool en) {

  if (flagState(MODE_ACK_PAY) == en) return;
  
  rRegister(FEATURE);
  en ? recData[0] |=  (FEATURE__EN_ACK_PAY) 
     : recData[0] &= ~(FEATURE__EN_ACK_PAY);
  wRegister(FEATURE);

  flagState(MODE_ACK_PAY, en);
}

bool acRF24Class::isACK_PAY() {

  return flagState(MODE_ACK_PAY);
}

// Registra o tamanho do payload a ser enviado.
void acRF24Class::setTXpayloadWidth(uint8_t w) {

	// 'txPayloadWidth' é válido apenas para o modo dinâmica.
  // 'txPayloadWidth' dependo do valor registrado no rádio
  // o qual receberá a mensagem.
  if (isStaticPayload(0)) {
    pv_txPayloadWidth = rRegister(RX_PW_P0);
  } else {
    if (isFanOut()) w++;
    if (w > 32) w = 32;
    pv_txPayloadWidth = w;
  }
}

// Retorna o tamanho do envio para o payload.
uint8_t acRF24Class::internalTXpayloadWidth() {

  // TODO: Exluir na nova versão. 'pv_txPayloadWidth' já estará atualizado.
  if (isStaticPayload(0)) {
    pv_txPayloadWidth = rRegister(RX_PW_P0);
  }
  return pv_txPayloadWidth;// <- para tamanho dinâmico
}

uint8_t acRF24Class::internalRXpayloadWidth() {

	// Este método retorna o tamanho de 'payload'.
  // Se for estático o tamanho tem que estar predefinido
  // no registro 'RX_PW_Px'. Deve ser feito o registro
  // no momento de registrar os rádios.
  uint8_t pipe = rxPipeNo();
  // Verifica se RX_FIFO está vazio
  if (pipe == 7 ) {
    pv_sourceID = 0;
    return 0;
  }

  if (isStaticPayload(pipe)) {
    rRegister(RX_PW_P0 + pipe) & RX_PW_Px__LEN;
	} else {
	  if (command( R_RX_PL_WID) > 32 ){
      pv_sourceID = 0;
	    flushRX();
	    return 0;
	  }
	}
  return recData[0];// Contém o resultado da leitura 'R_RX_PL_WID'.  
}

//== Manipulação dos rádios e canais ==========================================

void acRF24Class::setRadios(uint8_t* buf) {

	// Transfere a lista de ID de rádios para o chip.
  uint32_t i, p = 0; // <-- short
  while( p < 6 ) {
  	if (buf[i] != getSelfID() && buf[i] != 0 && buf[i] <= 254){
    	pipeReplace(buf[i], p);
    	p++;
    }
  	i++;
  	if (p == 1) p++;
  }
  // Registra o restante de rádios para uso futuro.
  while(i < RADIO_AMOUNT) {
  	setRadioID(i, buf[i]);
  	i++;
  }
}

void acRF24Class::setTXradio(uint8_t r) {

  if (getTXradio() == r) return;
  radioExchange( r, 0);
  // Preenche os dados de FEATURE conforme as informação de RX.
  enableDPL(isDynamicPayload(0));
 	enableDYN_ACK(!isAutoAcknowledgement(0));
  if (isStaticPayload(0)) {
    // TODO: Para nova versão
    // pv_txPayloadWidth = getRadioPayloadWidth(0);
    pv_txPayloadWidth = rRegister(RX_PW_P0);
  }
}

uint8_t acRF24Class::getTXradio() {

  return getRadioID(0);
}

void acRF24Class::setTXaddr(uint8_t pipe) {

  // TX_ADDR assume o mesmo radio contido no pipe 0.
  setTXradio(getRadioID(pipe));
}

uint8_t acRF24Class::getTXaddr() {

  return getRadioID(0);
}

//-- privados

void acRF24Class::setRadioID(u8 p, u8 id) {

  radio[p].ID = id;
  // Sincroniza os rádio rádio com o devido pipe.
  if (p < 6) {
    if(p == 0 ) {
      // O alvo sempre será o rádio que estiver no pipe 0;
      pv_targetID = id;
      getSufixo(&recData[1]);
      recData[0] = id;
      wRegister(TX_ADDR);
      // Prepara sufixo para RX_ADDR_P0.
      getSufixo(&recData[1]);
    }

    #ifdef __nRF24L01P__
      // Prepara sufixo para RX_ADDR_P1.
      if (p == 1) getSufixo(&recData[1]);
    #endif

    recData[0] = id;
    wRegister(RX_ADDR_P0 + p);
  }
}

// Retorna o id do rádio do especificado pipe'.
uint8_t acRF24Class::getRadioID(u8 p) {

	// p == pipe or position
	return radio[p].ID;
}

void acRF24Class::radioExchange(uint8_t r, uint8_t p) {

  // r: rádio; p: pipe.
  // O pipe 1 é reservado para o rádio base.
  if ((p == 1) || (r == getRadioID(p))) return;
  // Procede a troca.
  uint8_t p_out = hasRadio(r); // Pipe que vai receber 'r_out'.
  uint8_t r_out = getRadioID(p); // Rádio que vai sair.
  
  setRadioID(p, r);       	// <--
  setRadioID(p_out, r_out); // -->
}

void acRF24Class::pipeReplace(uint8_t r, uint8_t p) {

  deleteRadio(r);   //<- Evita duplicidade.
  setRadioID(p, r);
}

// Retorna o pipe' onde está o rádio.
uint8_t acRF24Class::hasRadio(uint8_t r) {

  // Retorna posição do rádio ou 'radioAmount' se não houver.
  uint8_t i = 0;
  for (i = 0; i < RADIO_AMOUNT; ++i) {
    if (getRadioID(i) == r) break;
  }
  return i;
}

uint8_t acRF24Class::deleteRadio(uint8_t r) {

  // Retorna a posição do rádio excluido.
  // Retorna radio amount se rádio não encontrado.
  uint8_t p = hasRadio(r);
  if (p < RADIO_AMOUNT && p != 1) {
    setRadioID(p, 0);
    return p;
  }
  return RADIO_AMOUNT;
}

// --

void acRF24Class::enableFanOut(bool en) {

  // TODO: Ativar fan-out apenas para o rádio configurado para tal.
  //       Enable fan-out only for the radio configured for this
  if (flagState(MODE_FAN_OUT) == en) return;

	for (int i = 0; i < 6; ++i) {
		rRegister(RX_PW_P0 + i);
		if (recData[0] != 0){
			en ? (recData[0] += 1) 
         : (recData[0] -= 1);
   	  wRegister(RX_PW_P0 + i);
    }
	} 		

  flagState(MODE_FAN_OUT, en);
}

uint8_t acRF24Class::sourceID() {

	return pv_sourceID;
}

bool acRF24Class::isFanOut() {

  return flagState(MODE_FAN_OUT);
}

//== Comandos para fins de suporte ==========================================

// Verifica se o rádio está ativo.
bool acRF24Class::chipActived() {

  return (rRegister(SETUP_AW) & SETUP_AW__AWBYTES > 0);
}

bool acRF24Class::isAvailableRX() {
  
  return !(rRegister( FIFO_STATUS) & FIFO_STATUS__RX_EMPTY);
}

bool acRF24Class::isAvailableTX() {

  bool a = !(nop() & STATUS__TX_FULL);

  if (!a && pv_watchTX != 0){
    if (pv_watchTXinterval == 0) pv_watchTXinterval = millis();
    if (millis() - pv_watchTXinterval >= pv_watchTX) {
      pv_watchTXinterval = 0;
      flushTX();
    } else {
      reuseTXpayload();
      delay(4);
    }
    a = !(nop() & STATUS__TX_FULL);
  }
  return a;
}

void acRF24Class::watchTX(uint32_t ms) {

  // Quando o rádio receptor cai, por longo período, ACK não
  // retorna, fazendo o transmissor ficar inoperante.
  // 'watchTX()' define o tempo em milesegundos que o transmissor
  // ficará reenviando o conteúdo dos 'fifo' esperando um ACK,
  // após este tempo 'flushTX()' é chamado, e assim libera o
  // transmissor para operar com outros rádios.
  pv_watchTX = ms;
}

// Número do pipe que recebeu os dados.
uint8_t acRF24Class::rxPipeNo() {

  // Obtém o número do canal que recebeu dados.
  return (rRegister(STATUS) & STATUS__RX_P_NO) >> 1;
}

uint8_t acRF24Class::getSelfID() {

  return pv_selfID;
}

uint8_t acRF24Class::staticTXpayloadWidth() {

  return rRegister(RX_PW_P0); // <- Tamanho dinâmico.
}

//-- Pré configuração

#ifdef __SE8R01__
  // Retorna status
  uint8_t acRF24Class::selectBank(uint8_t bank) {

    uint8_t n = nop();
    if ((n & STATUS__BANK) != bank) {
      return Activate(ACTIVATE__SWAP_BANK);
    }
    return n;
  }
#elif defined __nRF24L01P__
  // ??? Deve voltar o estado da troca.
  uint8_t acRF24Class::toggleFeature() {

    return Activate( ACTIVATE__TOGGLE_CMD);
  }
#endif

//== Privados

bool acRF24Class::flagState(uint16_t f) {

  return ((pv_flagState & f) == f);
}

void acRF24Class::flagState(uint16_t f, bool e) {

  (e) ? (pv_flagState |= f)
      : (pv_flagState &= ~f);
}

void acRF24Class::setFlagStateCtrl(uint16_t f) {

  pv_flagState &= ~MODE_STATE_CTRL;
  pv_flagState |= f;
}

bool acRF24Class::flagStateCtrl(uint16_t f) {

  return ((pv_flagState & MODE_STATE_CTRL) == f);
}

void acRF24Class::clearTX_DS() {

  // TODO: Conferir o efeito deste método.
  rRegister(STATUS);
  recData[0] |= STATUS__TX_DS;
  wRegister(STATUS);
}

void acRF24Class::clearRX_DR() {

  // TODO: Conferir o efeito deste método.
  rRegister(STATUS);
  recData[0] |= STATUS__RX_DR;
  wRegister(STATUS);
}

void acRF24Class::clearIRQ() {

  rRegister(STATUS);
  recData[0] |= (STATUS__MAX_RT | STATUS__TX_DS | STATUS__RX_DR);
  wRegister(STATUS);
}

#ifdef TEST_VARS

	// Este método é para verificação das variáveis.
	void acRF24Class::getVars(uint8_t* sts) {

	  sts[0] = CS;
	  sts[1] = CE;
	  sts[2] = IRQ;
	  sts[3] = highByte(pv_flagState);
	  sts[4] = lowByte(pv_flagState);
	  sts[5] = pv_lastStatus;
	  sts[6] = pv_sufixo[0];
	  sts[7] = pv_sufixo[1];
	  sts[8] = pv_sufixo[2];
	  sts[9] = pv_sufixo[3];
	  sts[10] = pv_txPayloadWidth;
	  sts[11] = pv_RFchannel;
	  sts[12] = pv_selfID;
	  sts[13] = pv_targetID;
	  sts[14] = pv_sourceID;
	  sts[15] = pv_recAmount;
    sts[16] = false;
    #ifdef __SE8R01__
      sts[16] = true;
    #endif
    sts[17] = false;
    #ifdef __nRF24L01P__
      sts[17] = true;
    #endif
	  // uint32_t pv_watchTX = 0;
	  // uint32_t pv_watchTXinterval = 0;
	}
#endif


//---