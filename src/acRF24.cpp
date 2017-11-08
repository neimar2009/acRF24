/*
 * Copyright (c) 2017 by Acácio Neimar de Oliveira <neimar2009@gmail.com>
 * acRF24.cpp library.
*/

#include <Arduino.h>
#include "acRF24.h"

#if !defined(PIN_SPI_SCK) && !defined(SCK)
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

#define SPI_CLOCK_LIMIT ((uint32_t)(((1/SPI_RF24_LIMIT)/2)*1000000))

uint8_t SPI_transfer(uint8_t data) {

  for(int i=0; i<8; i++){     // output 8-bit
    MOSI_MSBFIRST(data);      // Torna disponível na porta MOSI o bit mais significativo de data.
    delayMicroseconds(SPI_CLOCK_LIMIT); // Tempo de estabilização do bit, pelo mastre e pelo escravo.
    digitalWrite(SCK, HIGH);  // Set SCK high. Send the bit
    data <<= 1;               // shift next bit to left...
    MISO_MSBFIRST(data);      // capture MISO bit. Ready to reset.
    delayMicroseconds(SPI_CLOCK_LIMIT); // Tempo de captura pelo escravo.
    digitalWrite(SCK, LOW);   // SCK clock down. Reset.
  }
  return data;                // return read unsigned char
}

//== Inicialização ==========================================================

void acRF24Class::begin() {

  //
  if(isActiveCE()) {
    pinMode(CE, OUTPUT);
    digitalWrite(CE, LOW);
  } 
  flag(C_ENABLED, !isActiveCE());

  //
  if (isActiveCS()) {
    pinMode(CS, OUTPUT);
    digitalWrite(CS, HIGH);
  }
  flag(C_SELECTED, false);

  // Ajusta pino de interrupção se houver.
  if (isActiveIRQ()) pinMode(IRQ, INPUT);

  // Pino de entrada MISO
  pinMode(MISO, INPUT);
  
  // Ajusta pino de saída MOSI
  pinMode(MOSI, OUTPUT);
  digitalWrite(MOSI, LOW);

  // Pino de compasso SCK
  pinMode(SCK, OUTPUT);  
  digitalWrite(SCK, !isActiveCS());

  // Espera até que o chip esteja ativo.
  do {
     delayMicroseconds((uint16_t)T_POWERON);
  } while(!chipActived());

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

  // TODO: Revisar, e implementar
  resetConfig();

  #ifdef __SE8R01__

    configBank1();
  #elif defined __nRF24L01P__
  
    toggleFeature();
  #endif
}

//-- Privados

void acRF24Class::resetConfig() {

  recData[0] = CONFIG__EN_CRC | CONFIG__CRCO;
  wRegister(CONFIG);          // Address: 00h

  recData[0] = _SETUP_AW__AW5BYTES;
  wRegister(SETUP_AW);        // Address: 03h

  recData[0] = STATUS__MAX_RT | STATUS__TX_DS | STATUS__RX_DR;
  wRegister(STATUS);          // Address: 07h

  recData[0] = 0;
  wRegister(DYNPD);           // Address: 1Ch

  pv_flagState &= 0x00FF;     // <- Don't remove zeros
  recData[0] = 0;
  wRegister(FEATURE);         // Address: 1Dh

  // Calibração
  //[28h] [32h] [80h] [10h] [00h] default
  rRegister(SETUP_VALUE);
  recData[3] = 0x90;          // Main band gap wait counter. Default: 0x10 (16us)
  wRegister(SETUP_VALUE);     // Address: 1Eh
  delayMicroseconds(5);

  recData[0] = 0x77;          // Default: 0x32
  recData[1] = 0;
  wRegister(PRE_GURD);        // Address: 1Fh

  flushTX();
  flushRX();
  // getMode();                  // Atualiza a variável indicadora de modo.
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

      // - 0x01 PLL_CTL0
      p[0] = 0x40; p[1] = 0x00; p[2] = 0x10;
      if (dr == DR_2Mbps) 
           { p[3] = 0xE6; }
      else { p[3] = 0xE4; }
      spiTransfer(W_REGISTER | BANK1_PLL_CTL0, p, 4);

      // // - 0x03 CAL_CTL
      // p[0] = 0x20; p[1] = 0x08; p[2] = 0x50; p[3] = 0x40; p[4] = 0x50;
      // spiTransfer(W_REGISTER | BANK1_CAL_CTL,  p, 5);

      // - 0x0A IF_FREQ
      p[0] = 0x00; p[1] = 0x00;
      if (dr == DR_2Mbps)
           { p[2]=0x1E;}
      else { p[2]=0x1F;}
      spiTransfer(W_REGISTER | BANK1_IF_FREQ, p, 3);

      // // - 0x0C FDEV
      // if (dr == DR_2Mbps)
      //      { p[0] = 0x29;}
      // else { p[0] = 0x14;}
      // spiTransfer(W_REGISTER | BANK1_FDEV, p, 1);

      // // -- 0x17 DAC_CAL_LOW
      // p[0] = 0x00;
      // spiTransfer(W_REGISTER | BANK1_DAC_CAL_LOW, p, 1);

      // // -- 0x18 DAC_CAL_HI
      // p[0] = 0x7F;
      // spiTransfer(W_REGISTER | BANK1_DAC_CAL_HI,  p, 1);

      // // -- 0x1D AGC_GAIN      ?? Pode dar erro
      // p[0] = 0x02; p[1] = 0xC1; p[2] = 0xEB; p[3] = 0x1C; p[4] = 0x01;
      // spiTransfer(W_REGISTER | BANK1_AGC_GAIN, p, 5);  // 4 ou 5

      // // -- 0x1E RF_IVGEN     ?? Pode dar erro
      // p[0] = 0x97; p[1] = 0x64; p[2] = 0x00; p[3] = 0x81;// p[4] = 0x1F;
      // spiTransfer(W_REGISTER | BANK1_RF_IVGEN, p, 4);  // 4 ou 5
    
      // // - 0x0F CTUNING     [12h] 
      // p[0] = 0x12;
      // spiTransfer(W_REGISTER | BANK1_CTUNING, p, 1);

      // // - 0x10 FTUNING     [02h] [05h] 
      // p[0] = 0x02; p[1] = 0x05;
      // spiTransfer(W_REGISTER | BANK1_FTUNING, p, 1);
      
      // // - 0x12 FAGC_CTRL   [00h] [40h] [A3h]
      // p[0] = 0x00; p[1] = 0x40; p[2] = 0xA3;
      // spiTransfer(W_REGISTER | BANK1_FAGC_CTRL, p, 1);
    
      selectBank(BANK0);
      uint8_t rConf = rRegister(CONFIG);
      for (uint8_t i = 0; i < 5; ++i){
        recData[0] = 3;
        wRegister(CONFIG);
        delay(15);
        recData[0] = 1;
        wRegister(CONFIG);
        delay(25);
      }
      recData[0] = 3;
      wRegister(CONFIG);
      delay(25);
      recData[0] = rConf;
      wRegister(CONFIG);

    // --------------------------------------------
    // Inicialização da parte analógica do circuito
    // --------------------------------------------

      selectBank(BANK1);

      // - 0x01 PLL_CTL0
      p[0] = 0x40; p[1] = 0x01; p[2] = 0x30;  
      if (dr == DR_2Mbps)
           { p[3] = 0xE2;}
      else { p[3] = 0xE0;}
      spiTransfer(W_REGISTER | BANK1_PLL_CTL0, p, 4);

      // // - 0x03 CAL_CTL
      // p[0] = 0x29; p[1] = 0x89; p[2] = 0x55; p[3] = 0x40; p[4] = 0x50;  
      // spiTransfer(W_REGISTER | BANK1_CAL_CTL, p, 5);

      // - 0x0C BANK1_FDEV
      if (dr == DR_2Mbps)
           { p[0] = 0x29;}
      else { p[0] = 0x14;}       
      spiTransfer(W_REGISTER | BANK1_FDEV, p, 1);

      // // -- 0x11 RX_CTRL       !! Dá erro !!
      // p[0] = 0x55; p[1] = 0xC2; p[2] = 0x09; p[3] = 0xAC;
      // spiTransfer(W_REGISTER | BANK1_RX_CTRL,     p, 4 );

      // // -- 0x12 FAGC_CTRL     perde pacote
      // p[0] = 0x00; p[1] = 0x14; p[2] = 0x08; p[3] = 0x29;
      // spiTransfer(W_REGISTER | BANK1_FAGC_CTRL_1, p, 4 );

      // // -- 0x1D AGC_GAIN      ?? Pode dar erro
      // p[0] = 0x02; p[1] = 0xC1; p[2] = 0xCB; p[3] = 0x1C; p[4] = 0x01;
      // spiTransfer(W_REGISTER | BANK1_AGC_GAIN, p, 5 ); // 4 ou 5

      // // -- 0x1E RF_IVGEN     ?? Pode dar erro
      // p[0] = 0x97; p[1] = 0x64; p[2] = 0x00; p[3] = 0x01;
      // spiTransfer(W_REGISTER | BANK1_RF_IVGEN, p, 4 );

      // // -- 0x1F TEST_PKDET
      // p[0] = 0x2A; p[1] = 0x04; p[2] = 0x00; p[3] = 0x7D;
      // spiTransfer(W_REGISTER | BANK1_TEST_PKDET, p, 4 );

      selectBank(BANK0);

      // goMode() só funciona com o banco 0 ativo.
      goMode(m);
  }
#endif

//== Controle de estado do chip ===============================================

void acRF24Class::setCE( bool enable) {

  if (isActiveCE()) {
    if (flag(C_ENABLED) == enable) return;
    digitalWrite(CE, enable);
    flag(C_ENABLED, enable);
    getMode();
  }
  if (enable) delayMicroseconds(T_HCE);
}

void acRF24Class::setCS( bool select) {

  if (flag(C_SELECTED) == select) return;
  flag(C_SELECTED, select);

  select = !select; // Select with negative.

  if (isActiveCS()) {
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

  // if (flag(_MODE__POWERDOWN)) return;
  if (getMode() == _MODE__POWERDOWN) return;  

  rRegister(CONFIG);
  recData[0] &= ~CONFIG__PWR_UP;
  wRegister(CONFIG);
  setCE(LOW);
  flag(_MODE__POWERDOWN, true);
}

// Força a entrar no estado de standby-I
void acRF24Class::setStandbyRX() {
  
  // if (flag(_MODE__STANDBYRX)) return;
  uint8_t mode = getMode();
  if (mode == _MODE__STANDBYRX) return;
  if (mode == _MODE__RX) {
    setCE(LOW);
  } else {
    goStandby(RX);    
  }
  flag(_MODE__STANDBYRX, true);
}

// Força a entrar no estado de standby-II
void acRF24Class::setStandbyTX() {

  // if (flag(_MODE__STANDBYTX)) return;
  uint8_t mode = getMode();
  if (mode == _MODE__STANDBYTX) return;
  if (mode == _MODE__TX) {
    flushTX();
    setCE(HIGH);
  } else {
    goStandby(TX);    
  }
  flag(_MODE__STANDBYTX, true);
}

void acRF24Class::setModeRX() {

  // if (flag(_MODE__RX)) return;
  uint8_t mode = getMode();
  if (mode == _MODE__RX) return;
  if (mode != _MODE__STANDBYRX) {
    goStandby(RX);
  }
  setCE(HIGH);
  flag(_MODE__RX, true);
}

void acRF24Class::setModeTX() {
  
  // if (flag(_MODE__TX)) return;
  uint8_t m = getMode();
  if (m == _MODE__TX) return;
  if (m == _MODE__STANDBYTX) {
    flag(_MODE__TX, true);
    if (flag(C_ENABLED)) return;
  } else {
    goStandby(TX);
  }
  setCE(HIGH);
  flag(_MODE__TX, true);
}

uint8_t acRF24Class::getMode() {

  // TODO: Ajustar o método para melhorar a análise do modo.

  uint8_t rec = rRegister(CONFIG);
  // Mode: power_down
  if(!(rec & CONFIG__PWR_UP)) {
    flag(_MODE__POWERDOWN, true);
    return _MODE__POWERDOWN;
  }

  if(!flag(C_ENABLED)) {
    if(rec & CONFIG__PRIM_RX) {
      // Mode: standby-I -> StandbyRX
      flag(_MODE__STANDBYRX, true);
      return _MODE__STANDBYRX;
    } else {
      // Mode: standby-I -> StandbyTX
      flag(_MODE__STANDBYTX, true);
      return _MODE__STANDBYTX;
    }
  }

  if(rec & CONFIG__PRIM_RX) {
    // Mode: modeRX
    flag(_MODE__RX, true);
    return _MODE__RX;
  } else {
    // Mode: standby-II -> StandbyTX
    if((rRegister( FIFO_STATUS) & FIFO_STATUS__TX_EMPTY) == FIFO_STATUS__TX_EMPTY) {
      flag(_MODE__STANDBYTX, true);
      return _MODE__STANDBYTX;
    }
    // Mode: modeTX
    flag(_MODE__TX, true);
    return _MODE__TX;    
  }
}

// -- private --

void acRF24Class::goStandby( bool RXtx) {

  rRegister(CONFIG);
  bool pd = (recData[0] & CONFIG__PWR_UP) == 0;
  if (RXtx) { //<- RX
    if((recData[0] & CONFIG__PRIM_RX == 0) || pd) {
      recData[0] =  recData[0] | CONFIG__PWR_UP  | CONFIG__PRIM_RX;
      wRegister(CONFIG);
    }
    setCE(LOW);
    flag(_MODE__STANDBYRX, true);
  } else {   //<- TX
    if((recData[0] & CONFIG__PRIM_RX == CONFIG__PRIM_RX) || pd) {
      recData[0] = (recData[0] | CONFIG__PWR_UP) & ~CONFIG__PRIM_RX;
      wRegister(CONFIG);
    }
    flushTX();
    setCE(HIGH);
    flag(_MODE__STANDBYTX, true);
  }
  if(pd) delayMicroseconds(T_PD2STBY);
}

void acRF24Class::goMode(uint8_t m) {

  switch (m) {
    case _MODE__POWERDOWN : setPowerDown(); break;
    case _MODE__STANDBYRX : setStandbyRX(); break;
    case _MODE__RX        : setModeRX();    break;
    case _MODE__TX        : setModeTX();    break;
    case _MODE__STANDBYTX : setStandbyTX(); break;
    default: 
      flag(MODE__CTRL, true);
    break;
  }
}

//== Comunicação ============================================================

// cmd    : ação;
// dataOUT: dados que serão transmitidos;
// dataIN : dados recebidos;
// amount : quantidade de dados.
void acRF24Class::spiTransfer(uint8_t cmd, uint8_t* dataOUT, uint8_t* dataIN, uint8_t amount) {

  setCS(true);  // or setCSn(LOW);
  pv_lastStatus = SPI_transfer(cmd);
  // Se Fun-Out estiver ativo, enviar ou receber primeiro o cabeçalho.
  if ((cmd == R_RX_PAYLOAD || cmd == W_TX_PAYLOAD) && isFanOut()) {
    if(cmd == R_RX_PAYLOAD) {
      pv_sourceID = SPI_transfer(0);
    } else {
      SPI_transfer(pv_selfID);
    }
  }
  // 
  for (int i = 0; i < amount; ++i) {
    dataIN[i] = SPI_transfer(dataOUT[i]);
  }
  setCS(false); // or setCSn(HIGH);

  dataIN[amount] = 0;
}

void acRF24Class::spiTransfer(uint8_t cmd, uint8_t* buf, uint8_t amount) {
  
  spiTransfer(cmd, buf, buf, amount);
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
    case R_RX_PAYLOAD:        // pronto (ready)
    case W_TX_PAYLOAD:        // pronto (ready)
    case W_ACK_PAYLOAD: 
    case W_TX_PAYLOAD_NO_ACK: // pronto (ready)
      p = payload;
      break;
    case FLUSH_TX:            // pronto (ready)
    case FLUSH_RX:            // pronto (ready)
    case REUSE_TX_PL:         // pronto (ready)
    case NOP:                 // pronto (ready)
      pv_recAmount = 0;
      break;
    default:
      pv_recAmount = 1;
      break;
  }

  spiTransfer(cmd, p, p, pv_recAmount);
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
  return internalRXpayloadWidth();
}

// Retorna o tamanho do Payload e o conteúdo passado para o payload[].
uint8_t acRF24Class::rRXpayload() {

  pv_recAmount = internalRXpayloadWidth();
  if(pv_recAmount == 0) return 0;

  command( R_RX_PAYLOAD);  // Carrega payload.
  if (pv_recAmount == 0) pv_sourceID = 0;

  uint8_t a = pv_recAmount;  //<- É alterado por clearRX_DR()
  // TODO: RX_DR é persistente?
  if (pv_lastStatus & STATUS__RX_DR) {
    clearRX_DR();
  }
  clearRX_DR();

  return a;
}

// Método de nível mais baixo, redobrar a atenção ao usar.
uint8_t acRF24Class::rRXpayload(void* buf, uint8_t len) {

  pv_recAmount = internalRXpayloadWidth();
  if (len > pv_recAmount) len = pv_recAmount;
  spiTransfer(R_RX_PAYLOAD, payload, buf, len);

  // TODO: RX_DR é persistente?
  if (pv_lastStatus & STATUS__TX_DS) {
    clearTX_DS();
  }
  clearRX_DR();
  return len;
}

// Retorna status
uint8_t acRF24Class::wTXpayload() {

  return internal_wTXpayload( W_TX_PAYLOAD);
}

// Método de alto nível, redobrar a atenção ao usar.
uint8_t acRF24Class::wTXpayload(void* buf, uint8_t len) {

  spiTransfer(W_TX_PAYLOAD, buf, payload, len);
  recData[0] = pv_lastStatus;
  
  #ifdef __nRF24L01P__
    len = pv_lastStatus;
    clearTX_DS();
    return len;
  #endif

  return pv_lastStatus;
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

  command(wTX); //W_TX_PAYLOAD or W_TX_PAYLOAD_NO_ACK

  #ifdef __nRF24L01P__
    uint8_t a = pv_lastStatus;
    clearTX_DS();
    return a;
  #endif

  return pv_lastStatus;
}

//== Configurações ==========================================================

void acRF24Class::setSufixo(const void* buf) {

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

void acRF24Class::getSufixo(void* buf) {

  // for (int i = 0; i < 4; ++i) {
  //   buf[i] = pv_sufixo[i];
  // }
  memcpy(buf, pv_sufixo, 4);
}

void acRF24Class::setPayload(void* buf, uint8_t len) {

  setTXpayloadWidth(len);

  memcpy( payload, buf, len);
  payload[len] = 0;
}

void acRF24Class::getPayload(void* buf, uint8_t len) {
  
  // TODO: Testar e verificar a necessidade deste 'if'.
  if (len > 32) len = 32;
  memcpy( buf, payload, len);
}

void acRF24Class::setRFchannel(uint8_t ch) {

  // de 1 à 126
  if (ch < 1 || ch > 126) return;
  ch--;
  if (ch == pv_RFchannel) return;
  pv_RFchannel = ch;
  recData[0] = pv_RFchannel;
  wRegister(RF_CH);
}

uint8_t acRF24Class::getRFchannel() {

  return rRegister(RF_CH) + 1;
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

  #ifdef __SE8R01__

    configBank1();
  #endif

  //* Aqui é feiro uma auto configuração do tempo de espera
  //* em conformidade com a largura de banda escolhida.
  uint8_t ard = (uint8_t)dr;
  ard < 2 ? ard = 4 : ard = 6; //<- 256 * 2 = 1024uS or 256 * 6 = 1536uS
  setAutoRetransmissionDelay(ard);
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
  return (eDataRate)dr;
}

int8_t acRF24Class::receivedPower(uint8_t channel) {

  if (pv_RFchannel != channel) {
    setRFchannel(channel);
    delayMicroseconds(T_STBY2A + T_DELAY_AGC);
  }

  int8_t r = rRegister(RPD);

  if (pv_RFchannel != channel) {
    setRFchannel(pv_RFchannel);
  }

  return r;
}

int8_t acRF24Class::receivedPower() {

  return receivedPower(pv_RFchannel);
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

  if (flag(MODE_DYN_ACK) == en) return;

  rRegister(FEATURE);
  en ? recData[0] |=  (FEATURE__EN_DYN_ACK)
     : recData[0] &= ~(FEATURE__EN_DYN_ACK);
  wRegister(FEATURE);

  flag(MODE_DYN_ACK, en);
}

bool acRF24Class::isDYN_ACK() {

  return flag(MODE_DYN_ACK);
}

void acRF24Class::enableDPL(bool en) {

  if (flag(MODE_DPL) == en) return;
  
  rRegister(FEATURE);
  en ? recData[0] |=  (FEATURE__EN_DPL) 
     : recData[0] &= ~(FEATURE__EN_DPL);
  wRegister(FEATURE);

  // Atualiza o 'state'
  flag(MODE_DPL, en);
}

bool acRF24Class::isDPL() {

  return flag(MODE_DPL);
}

void acRF24Class::enableACK_PAY(bool en) {

  if (flag(MODE_ACK_PAY) == en) return;
  
  rRegister(FEATURE);
  en ? recData[0] |=  (FEATURE__EN_ACK_PAY) 
     : recData[0] &= ~(FEATURE__EN_ACK_PAY);
  wRegister(FEATURE);

  flag(MODE_ACK_PAY, en);
}

bool acRF24Class::isACK_PAY() {

  return flag(MODE_ACK_PAY);
}

// Registra o tamanho do payload a ser enviado.
void acRF24Class::setTXpayloadWidth(uint8_t w) {

  // 'txPayloadWidth' é válido apenas para o modo dinâmica.
  // 'txPayloadWidth' dependo do valor registrado no rádio
  // o qual receberá a mensagem.
  if (isStaticPayload(0)) {
    pv_txPayloadWidth = rRegister(RX_PW_P0);
  } else {
    if (isFanOut() && (w > 31)) w = 31;
    if (w > 32) w = 32;
    pv_txPayloadWidth = w;
  }
}

// Retorna o tamanho do envio para o payload.
uint8_t acRF24Class::internalTXpayloadWidth() {

  if (isStaticPayload(0)) {
    pv_txPayloadWidth = rRegister(RX_PW_P0);
    if(isFanOut() && (pv_txPayloadWidth > 0) && (pv_txPayloadWidth < 32))
      return (pv_txPayloadWidth - 1); // <- para tamanho estático.
  }
  return pv_txPayloadWidth;// <- para tamanho dinâmico.
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
    rRegister(RX_PW_P0 + pipe);// & RX_PW_Px__LEN;
  } else {
    if (command( R_RX_PL_WID) > 32 ){
      pv_sourceID = 0;
      flushRX();
      return 0;
    }
  }
  if(isFanOut() && (recData[0] > 0)) recData[0]--;
  return recData[0];// Contém o resultado da leitura 'R_RX_PL_WID'.  
}

//== Manipulação dos rádios e canais ==========================================

void acRF24Class::setRadios(const uint8_t* buf, uint8_t c) {

  // Registra uma nova lista de ID de rádios.
  uint8_t i = 0, p = 0;
  // Exclui os rádios existentes.
  for (i = 0; i < RADIO_AMOUNT; ++i) {
    if (i != 1) radio[i].ID = 0;
  }
  pv_radioCount = 0;

  i = 0;
  while((i < c) && (i < RADIO_AMOUNT)) {
    // Exclui selfID, radio 0 e 254 acima
    if ((buf[i] != getSelfID()) && (hasRadio(buf[i]) == RADIO_AMOUNT) && (buf[i] != 0) && (buf[i] < 0xFF)){
      setRadioID(p, buf[i]);
      p++;// <- Salta para o próximo pipe.
    }
    i++;
    if (p == 1) p++;// <- Salta o pipe do selfID.
  }
}

void acRF24Class::getRadios(uint8_t* buf, uint8_t c) {
  
  uint8_t i = 0;
  uint8_t r = 0;
  while( i < c && i < RADIO_AMOUNT) {
    if ((i != 1) && (radio[r].ID !=0)) {
      buf[i] = getRadio(r);
      i++;
    }
    r++;
  }
}

uint8_t acRF24Class::getRadio(uint8_t i) {

  if (i < radioCount()) {
    if (i > 0) i++; // <- Ignora selfID.
    // Ignora as identificações 0 (zero).
    while (i < RADIO_AMOUNT && radio[i].ID == 0) i++;
    return radio[i].ID;
  }
  return 0;
}

uint8_t acRF24Class::radioCount() {

  return pv_radioCount;
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

void acRF24Class::setRadioID(uint8_t p, uint8_t id) {

  // Não salvaguarda o rádio da posição 'p'.
  radio[p].ID = id;
  pv_radioCount++; // Atualiza a indicação de quantidade de rádios.
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
uint8_t acRF24Class::getRadioID(uint8_t p) {

  // p == pipe or position
  return radio[p].ID;
}

void acRF24Class::radioExchange(uint8_t r, uint8_t p) {

  // r: rádio; p: pipe.
  // O pipe 1 é reservado para o rádio base.
  if ((r == getRadioID(p)) || (p == 1)) return;
  // Procede a troca.
  uint8_t p_out = hasRadio(r);   // Posição para onde vai o rádio contido em 'p'.
  uint8_t r_out = getRadioID(p); // Rádio que está em 'p'.
  
  setRadioID(p, r);         // <--
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
    if ((r != pv_selfID) && (r != 0)) {
      if (getRadioID(i) == r) break;
    }
  }
  return i;
}

uint8_t acRF24Class::deleteRadio(uint8_t r) {

  // Retorna a posição do rádio excluido.
  // Retorna radio 'RADIO_AMOUNT' se rádio não encontrado.
  uint8_t p = hasRadio(r);
  if (p < RADIO_AMOUNT && p != 1) {
    setRadioID(p, 0);
    pv_radioCount--; // Atualiza a indicação de quantidade de rádios.
    return p;
  }
  return RADIO_AMOUNT;
}

// --

void acRF24Class::enableFanOut(bool en) {

  // TODO: Ativar fan-out apenas para o rádio configurado para tal.
  //       Enable fan-out only for the radio configured for this
  if (flag(MODE_FAN_OUT) == en) return;

  for (int i = 0; i < 6; ++i) {
    rRegister(RX_PW_P0 + i);
    if (recData[0] != 0){
      en ? (recData[0] += 1) 
         : (recData[0] -= 1);
      wRegister(RX_PW_P0 + i);
    }
  }     

  flag(MODE_FAN_OUT, en);
}

uint8_t acRF24Class::sourceID() {

  return pv_sourceID;
}

bool acRF24Class::isFanOut() {

  return ((pv_flagState & MODE_FAN_OUT) == MODE_FAN_OUT);
}

//== Comandos para fins de suporte ==========================================

// Verifica se o rádio está ativo.
bool acRF24Class::chipActived() {

  return ((rRegister(SETUP_AW) & SETUP_AW__AWBYTES) > 0);
}

bool acRF24Class::isAvailableRX() {
  
  // TODO: Só pode ser avaliável se o tamanho, dos dados, for maior que zero;
  //       Se Fan-Out estiver ativo e for canal 1;
  //       Se o rádio que está transmitindo estiver na lista de rádios.
  return !(rRegister( FIFO_STATUS) & FIFO_STATUS__RX_EMPTY) && chipActived();
}

bool acRF24Class::isAvailableTX() {

  if ( !chipActived()) return false;

  bool a = !(pv_lastStatus & STATUS__TX_FULL);

  if (!a && (pv_watchTX != 0)) {
    if (pv_watchTXinterval == 0) pv_watchTXinterval = millis();
    if (millis() - pv_watchTXinterval >= pv_watchTX) {
      pv_watchTXinterval = 0;
      flushTX();
      clearIRQ();
    } else {
      reuseTXpayload();
      delayMicroseconds(300);
    }
    a = !(pv_lastStatus & STATUS__TX_FULL);
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

uint8_t acRF24Class::lastStatus() {

  return pv_lastStatus;
}

uint8_t acRF24Class::staticTXpayloadWidth() {

  return rRegister(RX_PW_P0); // <- Tamanho estático.
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

bool acRF24Class::isActiveCS() {

  return CS != 0xFF;
}

bool acRF24Class::isActiveCE() {

  return CE != 0xFF;
}

bool acRF24Class::isActiveIRQ() {

  return IRQ != 0xFF;
}

bool acRF24Class::flag(uint16_t f) {

  if(f>MODE__CTRL) {
    return ((pv_flagState & f) == f);
  } else {
    return ((pv_flagState & MODE__CTRL) == f);
  }
}

void acRF24Class::flag(uint16_t f, bool e) {

  if(f>MODE__CTRL) {
    (e) ? (pv_flagState |= f)
        : (pv_flagState &= ~f);
  } else {
    pv_flagState &= ~MODE__CTRL;
    pv_flagState |= f;    
  }
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

void acRF24Class::clearMAX_RT() {

  // TODO: Conferir o efeito deste método.
  rRegister(STATUS);
  recData[0] |= STATUS__MAX_RT;
  wRegister(STATUS);
}

void acRF24Class::clearIRQ() {

  rRegister(STATUS);
  recData[0] |= (STATUS__MAX_RT | STATUS__TX_DS | STATUS__RX_DR);
  wRegister(STATUS);
}

#ifdef __TEST_VARS__

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
    #ifdef __SE8R01__
      sts[16] = true;
    #else
      sts[16] = false;
    #endif
    #ifdef __nRF24L01P__
      sts[17] = true;
    #else
      sts[17] = false;
    #endif
    // uint32_t pv_watchTX = 0;
    // uint32_t pv_watchTXinterval = 0;
  }
#endif


//---