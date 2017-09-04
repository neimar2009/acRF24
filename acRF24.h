// acRF24.h
/*
 * Copyright (c) 2017 by Acácio Neimar de Oliveira <neimar2009@gmail.com>
 * acRF24 library.
 */

#pragma once

#define __SE8R01__        // <- Comment if you do not use
// or
// #define __nRF24L01P__

// #define TEST_VARS  // <- Tirar o comentário para fazer os testes.

#define xFF                    0xFF
#ifdef __SE8R01__
  #define BANK0                 0x00
  #define BANK1                 0x80
#endif
//-----------------------------------------------------------------------------
// RF24 SPI Commands
  #define R_REGISTER          0X00  // Read command and status registers.
                                    // AAAAA=5 bit register map address
  #define W_REGISTER          0X20  // Write command and status registers.
                                    // AAAAA=5 bit register map address Executable
                                    // in power down or standby modes only. 
  #define ACTIVATE            0x50  // This write command followed by data 0x73
                                    // activates the following features:
                                    // • R_RX_PL_WID
                                    // • W_ACK_PAYLOAD
                                    // • W_TX_PAYLOAD_NOACK
                                    // A new ACTIVATE command with the same data
                                    // deactivates them again. This is executable
                                    // in power down or stand by modes only.
                                    // The R_RX_PL_WID, W_ACK_PAYLOAD, and
                                    // W_TX_PAYLOAD_NOACK features registers are
                                    // initially in a deactivated state; a write
                                    // has no effect, a read only results in zeros
                                    // on MISO. To activate these registers, use
                                    // the ACTIVATE command followed by data 0x73.
                                    // Then they can be accessed as any other
                                    // register. Use the same command and data to
                                    // deactivate the registers again.
                                    //
                                    // This write command followed by data 0x53
                                    // toggles the register bank, and the current
                                    // register bank number can be read out from
                                    // REG7 [7]
  #define R_RX_PL_WID         0x60  // Read RX payload width for the top R_RX_PAYLOAD
                                    // in the RX FIFO. Note: Flush RX FIFO if the
                                    // read value is larger than 32 bytes.
  #define R_RX_PAYLOAD        0x61  // Read RX-payload: 1-32 bytes. A read
                                    // operation always starts at byte 0.
                                    // Payload is deleted from FIFO after it is
                                    // read. Used in RX mode.       
  #define W_TX_PAYLOAD        0xA0  // Write TX-payload: 1 – 32 bytes. A write
                                    // operation always starts at byte 0 used
                                    // in TX payload.
  #define W_ACK_PAYLOAD       0xA8  // Used in RX mode. Write Payload to be
                                    // transmitted together with ACK packet on PIPE
                                    // PPP. (PPP valid in the range from 000 to 101).
                                    // Maximum three ACK packet payloads can be
                                    // pending. Payloads with same PPP are handled
                                    // using first in - first out principle. Write
                                    // payload: 1– 32 bytes. A write operation
                                    // always starts at byte 0.
  #define W_TX_PAYLOAD_NO_ACK 0xB0  // Used in TX mode. Disables AUTOACK on 
                                    // this specific packet.
  #define FLUSH_TX            0xE1  // Flush TX FIFO, used in TX mode
  #define FLUSH_RX            0xE2  // Flush RX FIFO, used in RX mode. Should
                                    // not be executed during transmission of
                                    // acknowledge, that is, acknowledge package
                                    // will not be completed
  #define REUSE_TX_PL         0xE3  // Used for a PTX operation. Reuse last
                                    // transmitted payload.TX payload reuse is
                                    // active until W_TX_PAYLOAD or FLUSH TX is
                                    // executed. TX payload reuse must not be
                                    // activated or deactivated duringpackage
                                    // transmission.
  #define NOP                 xFF  // No Operation. Might be used to read
                                    // the STATUS register
//-----------------------------------------------------------------------------
// RF24 registers addresses
  // -> Referente ao banco 0
  #define CONFIG          0x00    // 'Config' register address
  #define EN_AA           0x01    // 'Enable Auto Acknowledgment' register address
  #define EN_RXADDR       0x02    // 'Enabled RX addresses' register address
  #define SETUP_AW        0x03    // 'Setup address width' register address
  #define SETUP_RETR      0x04    // 'Setup Auto. Retrans' register address
  #define RF_CH           0x05    // 'RF channel' register address
  #define RF_SETUP        0x06    // 'RF setup' register address
  #define STATUS          0x07    // 'Status' register address
  #define OBSERVE_TX      0x08    // 'Observe TX' register address
  #define RPD             0x09    // 'Received Power Detector' register address
  #define RX_ADDR_P0      0x0A    // 'RX address pipe0' register address
  #define RX_ADDR_P1      0x0B    // 'RX address pipe1' register address
  #define RX_ADDR_P2      0x0C    // 'RX address pipe2' register address
  #define RX_ADDR_P3      0x0D    // 'RX address pipe3' register address
  #define RX_ADDR_P4      0x0E    // 'RX address pipe4' register address
  #define RX_ADDR_P5      0x0F    // 'RX address pipe5' register address
  #define TX_ADDR         0x10    // 'TX address' register address
  #define RX_PW_P0        0x11    // 'RX payload width, pipe0' register address
  #define RX_PW_P1        0x12    // 'RX payload width, pipe1' register address
  #define RX_PW_P2        0x13    // 'RX payload width, pipe2' register address
  #define RX_PW_P3        0x14    // 'RX payload width, pipe3' register address
  #define RX_PW_P4        0x15    // 'RX payload width, pipe4' register address
  #define RX_PW_P5        0x16    // 'RX payload width, pipe5' register address
  #define FIFO_STATUS     0x17    // 'FIFO Status Register' register address
  #define DYNPD           0x1C    // 'Enable dynamic payload length' register address
  #define FEATURE         0x1D    // 'Feature' register address
  #define SETUP_VALUE     0x1E
  #define PRE_GURD        0x1F
//-----------------------------------------------------------------------------
// RF24 Bank1 register
  #ifdef __SE8R01__
    #define BANK1_LINE          0x00
    #define BANK1_PLL_CTL0      0x01
    #define BANK1_PLL_CTL1      0x02
    #define BANK1_CAL_CTL       0x03
    #define BANK1_A_CNT_REG     0x04
    #define BANK1_B_CNT_REG     0x05
    #define BANK1_RESERVED0     0x06
    #define BANK1_STATUS        0x07
    #define BANK1_STATE         0x08
    #define BANK1_CHAN          0x09
    #define BANK1_IF_FREQ       0x0A
    #define BANK1_AFC_COR       0x0B
    #define BANK1_FDEV          0x0C
    #define BANK1_DAC_RANGE     0x0D
    #define BANK1_DAC_IN        0x0E
    #define BANK1_CTUNING       0x0F
    #define BANK1_FTUNING       0x10
    #define BANK1_RX_CTRL       0x11
    #define BANK1_FAGC_CTRL     0x12
    #define BANK1_FAGC_CTRL_1   0x13
    #define BANK1_DAC_CAL_LOW   0x17
    #define BANK1_DAC_CAL_HI    0x18
    #define BANK1_RESERVED1     0x19
    #define BANK1_DOC_DACI      0x1A
    #define BANK1_DOC_DACQ      0x1B
    #define BANK1_AGC_CTRL      0x1C
    #define BANK1_AGC_GAIN      0x1D
    #define BANK1_RF_IVGEN      0x1E
    #define BANK1_TEST_PKDET    0x1F
  #endif
//-----------------------------------------------------------------------------
// RF24 CONFIG fields 0x00
  #define CONFIG__PRIM_RX       0x01  // b00000001 RX/TX control
  #define CONFIG__PWR_UP        0X02  // b00000010 Power up control
  #define CONFIG__CRCO          0x04  // b00000100 CRC encoding scheme
  #define CONFIG__EN_CRC        0x08  // b00001000 Enable CRC. Forced high if one of
                                      //           the bits in the EN_AA is high
  #define CONFIG__MASK_MA_X_RT  0x10  // b00010000 Mask interrupt caused by MAX_RT
  #define CONFIG__MASK_TX_DS    0x20  // b00100000 Mask interrupt caused by TX_DS
  #define CONFIG__MASK_RX_DR    0x40  // b01000000 Mask interrupt caused by RX_DR
  #define CONFIG__RESET         0x80  // b10000000 Reserved. Only 0 allowed
//-----------------------------------------------------------------------------
// RF24 EN_AA fields 0x01
  #define EN_AA__ENAA_P0   0x01  // Enable auto acknowledgement data pipe 0
  #define EN_AA__ENAA_P1   0x02  // Enable auto acknowledgement data pipe 1
  #define EN_AA__ENAA_P2   0x04  // Enable auto acknowledgement data pipe 2
  #define EN_AA__ENAA_P3   0x08  // Enable auto acknowledgement data pipe 3
  #define EN_AA__ENAA_P4   0x10  // Enable auto acknowledgement data pipe 4
  #define EN_AA__ENAA_P5   0x20  // Enable auto acknowledgement data pipe 5
  #define EN_AA__RESERVED  0xC0  // Only 0 allowed
//-----------------------------------------------------------------------------
// RF24 EN_RXADDR fields 0x02
  #define EN_RXADDR__ERX_P0   0x01  // Enable data pipe 0
  #define EN_RXADDR__ERX_P1   0x02  // Enable data pipe 1
  #define EN_RXADDR__ERX_P2   0x04  // Enable data pipe 2
  #define EN_RXADDR__ERX_P3   0x08  // Enable data pipe 3
  #define EN_RXADDR__ERX_P4   0x10  // Enable data pipe 4
  #define EN_RXADDR__ERX_P5   0x20  // Enable data pipe 5
  #define EN_RXADDR__RESERVED 0xC0  // Only 0 allowed
//-----------------------------------------------------------------------------
// RF24 SETUP_AW fields 0x03
  #define SETUP_AW__AWBYTES   0x03  // Setup of Address Widths
  #define _SETUP_AW__AW5BYTES 0x03  // Width 5 bytes (common for all data pipes)
  #define _SETUP_AW__AW4BYTES 0x02  // Width 4 bytes (common for all data pipes)
  // #define SETUP_AW__MANDATORY  0x02  // Mandatory high
  #define SETUP_AW__RESERVED  0xFC  // Only 0 allowed
//-----------------------------------------------------------------------------
// RF24 SETUP_RETR fields 0x04
  #define SETUP_RETR__ARC   0x0F // Re-Transmit disabled. Of 1 at 15 Re-Transmit 
                                 // on fail of AA
  #define SETUP_RETR__ARD   0xF0 // Add up from 0 to 15. Multiple waits of 256μS
//-----------------------------------------------------------------------------
// RF24 RF_CH fields 0x05

  #define RF_CH__REG_RF_CH  0x7F // Sets the frequency channel RF24 operates on
//-----------------------------------------------------------------------------
// RF24 RF_SETUP fields 0x06
  #if defined __SE8R01__
    #define RF_SETUP__PA_PWR          0x47 // PA power select. Output  +5 dbm
    #define _RF_SETUP__PA_PWR_18dbm   0x01 // Output -18 dbm
    #define _RF_SETUP__PA_PWR_12dbm   0x02 // Output -12 dbm
    #define _RF_SETUP__PA_PWR_6dbm    0x04 // Output  -6 dbm
    #define _RF_SETUP__PA_PWR_0dbm    0x40 // Output   0 dbm
    #define _RF_SETUP__PA_PWR_5dbm    0x47 // Output  +5 dbm
  #elif defined __nRF24L01P__
    #define RF_SETUP__PA_PWR          0x06 // PA power select. Output  +5 dbm
    #define _RF_SETUP__PA_PWR_18dbm   0x00 // Output -18 dbm
    #define _RF_SETUP__PA_PWR_12dbm   0x02 // Output -12 dbm
    #define _RF_SETUP__PA_PWR_6dbm    0x04 // Output  -6 dbm
    #define _RF_SETUP__PA_PWR_0dbm    0x06 // Output   0 dbm
    #define _RF_SETUP__PA_PWR_5dbm    0x06 // Output   0 dbm
  #endif
  #define RF_SETUP__RF_DR_HIGH        0x08 // Select between the high speed data rates.
                                           // This bit is donot care if RF_DR_LOW is set.
                                           // Encoding: [RF_DR_LOW, RF_DR_HIGH]
  #define RF_SETUP__RESERVED          0x10 // Reserved
  #define RF_SETUP__RF_DR_LOW         0x20 // See RF_DR_LOW for encoding.
  #define RF_SETUP__RF_DR_HIGH        0x08 // See RF_DR_HIGH for encoding.
  #define RF_SETUP__RF_DR             0x28 // Select between the speed data rates.
  #define _RF_SETUP__RF_DR_1Mbps      0x00 // 1Mbps
  #define _RF_SETUP__RF_DR_2Mbps      0x08 // 2Mbps
  #if defined __SE8R01__
    #define _RF_SETUP__RF_DR_500Kbps  0x28 // 500Kbps
  #elif defined __nRF24L01P__
    #define _RF_SETUP__RF_DR_250Kbps  0x20 // 250Kbps
  #endif
  #define RF_SETUP__CONT_WAVE         0x80 // Enables continuous carrier transmit
                                           // when high
//-----------------------------------------------------------------------------
// RF24 STATUS fields 0x07
  #define STATUS__TX_FULL   0x01 // Read only. TX FIFO full flag
  #define STATUS__RX_P_NO   0x0E // Read only. Data pipe number for the payload
                                 // available for reading from RX_FIFO.
  #define STATUS__MAX_RT    0x10 // Maximum number of TX retransmits interrupt,
                                 // Write 1 to clear bit. If MAX_RT is asserted it
                                 // must be cleared to enable further communication.
  #define STATUS__TX_DS     0x20 // Data Sent TX FIFO interrupt. Asserted when packet
                                 // transmitted on TX. If AUTO_ACK isactivated, this
                                 // bit is set high only when ACK is received.
                                 // Write 1 to clear bit.
  #define STATUS__RX_DR     0x40 // Data Ready RX FIFO interrupt. Asserted when new
                                 // data arrives RX FIFO Write 1 to clear bit.
  #define STATUS__BANK      0x80 // Register BANK status.
                                 // Bit low register R/W is to register BANK0
//-----------------------------------------------------------------------------
// RF24 OBSERVE_TX fields 0x08
  #define OBSERVE_TX__ARC_CNT   0x0F // Read only. Count retransmitted packets. The counter is
                                     // reset when transmission of a new packet starts.
  #define OBSERVE_TX__LENOS_CNT 0xF0 // Read only. Count lost packets. The counter is overflow
                                     // protected to 15, and discontinues at max until reset. The
                                     // counter is reset by writing to RF_CH.
//-----------------------------------------------------------------------------
// RF24 RPD fields 0x09
  #define RPD__SIG_DBM_EST  xFF // Read only. estimated in-band signal level in dBm, should
                                 // support -100 ~ +10 dBm.
//-----------------------------------------------------------------------------
// RF24 RX_ADDR_P0 fields 0x0A
  #define RX_ADDR_P0__0to4  xFF // 0x7041882046. Receive address data pipe 0. 5 Bytes
                                 // maximumlength. (LSByte is written first. Write the number of
                                 // bytes defined by SETUP_AW).
//-----------------------------------------------------------------------------
// RF24 RX_ADDR_P1 fields 0x0B
  #define RX_ADDR_P1__P xFF // 0xC2. Receive address data pipe 1. Only LSB. 
                             // MSBytesare equal to RX_ADDR_P0[39:8]
//-----------------------------------------------------------------------------
// RF24 RX_ADDR_P2 fields 0x0C
  #define RX_ADDR_P2__P xFF // 0xC3. Receive address data pipe 2. Only LSB.
                             // MSBytesare equal to RX_ADDR_P0[39:8]
//-----------------------------------------------------------------------------
// RF24 RX_ADDR_P3 fields 0x0D
  #define RX_ADDR_P3__P xFF // 0xC4. Receive address data pipe 3. Only LSB.
                             // MSBytesare equal to RX_ADDR_P0[39:8]
//-----------------------------------------------------------------------------
// RF24 RX_ADDR_P4 fields 0x0E
  #define RX_ADDR_P4__P xFF // 0xC5. Receive address data pipe 4. Only LSB. 
                             // MSBytesare equal to RX_ADDR_P0[39:8]
//-----------------------------------------------------------------------------
// RF24 RX_ADDR_P5 fields 0x0F
  #define RX_ADDR_P5__P xFF // 0xC6. Receive address data pipe 5. Only LSB. 
                             // MSBytesare equal to RX_ADDR_P0[39:8]
//-----------------------------------------------------------------------------
// RF24 TX_ADDR fields 0x10

  #define TX_ADDR__0to4 xFF // 0x7041882046. Transmit address. Used for a PTX device only. (LSByte
                             // is written first)Set RX_ADDR_P0 equal to this address to handle
                             // automatic acknowledge if this is a PTX device with Protocol
                             // engine enabled.
//-----------------------------------------------------------------------------
// RF24 RX_PW_P0 fields 0x11
// RF24 RX_PW_P1 fields 0x12
// RF24 RX_PW_P2 fields 0x13
// RF24 RX_PW_P3 fields 0x14
// RF24 RX_PW_P4 fields 0x15
// RF24 RX_PW_P5 fields 0x16
  #define RX_PW_Px__LEN       0x3F // Number of bytes in RX payload in data pipe 0 (1 to 32 bytes)
  #define RX_PW_Px__RESERVED  0xC0 // Only 0 allowed
//-----------------------------------------------------------------------------
// RF24 RX_PW_P0 fields 0x11
  #define RX_PW_P0__LEN       0x3F // Number of bytes in RX payload in data pipe 0 (1 to 32 bytes)
  #define RX_PW_P0__RESERVED  0xC0 // Only 0 allowed
//-----------------------------------------------------------------------------
// RF24 RX_PW_P1 fields 0x12
  #define RX_PW_P1__LEN       0x3F // Number of bytes in RX payload in data pipe 1 (1 to 32 bytes)
  #define RX_PW_P1__RESERVED  0xC0 // Only 0 allowed
//-----------------------------------------------------------------------------
// RF24 RX_PW_P2 fields 0x13
  #define RX_PW_P2__LEN       0x3F // Number of bytes in RX payload in data pipe 2 (1 to 32 bytes)
  #define RX_PW_P2__RESERVED  0xC0 // Only 0 allowed
//-----------------------------------------------------------------------------
// RF24 RX_PW_P3 fields 0x14
  #define RX_PW_P3__LEN       0x3F // Number of bytes in RX payload in data pipe 3 (1 to 32 bytes)
  #define RX_PW_P3__RESERVED  0xC0 // Only 0 allowed
//-----------------------------------------------------------------------------
// RF24 RX_PW_P4 fields 0x15
  #define RX_PW_P4__LEN       0x3F // Number of bytes in RX payload in data pipe 4 (1 to 32 bytes)
  #define RX_PW_P4__RESERVED  0xC0 // Only 0 allowed
//-----------------------------------------------------------------------------
// RF24 RX_PW_P5 fields 0x16
  #define RX_PW_P5__LEN       0x3F // Number of bytes in RX payload in data pipe 5 (1 to 32 bytes)
  #define RX_PW_P5__RESERVED  0xC0 // Only 0 allowed
//-----------------------------------------------------------------------------
// RF24 FIFO_STATUS fields 0x17 -> Only 0 allowed
  #define FIFO_STATUS__RX_EMPTY     0x01 // RX FIFO empty flag.
  #define FIFO_STATUS__RX_FULL      0x02 // RX FIFO full flag.
  #define FIFO_STATUS__RX_RESERVED1 0x0C // Only '00' allowed
  #define FIFO_STATUS__TX_EMPTY     0x10 // TX FIFO empty flag.
  #define FIFO_STATUS__TX_FULL      0x20 // TX FIFO full flag.
  #define FIFO_STATUS__TX_REUSE_PL  0x40 // TX REUSE flag.
  #define FIFO_STATUS__RX_RESERVED2 0x80 // Only '0' allowed
//-----------------------------------------------------------------------------
// RF24 DYNPD fields 0x1C
  #define DYNPD__DPL_P0       0x01  // Enable dynamic payload length data pipe 0. 
  #define DYNPD__DPL_P1       0x02  // Enable dynamic payload length data pipe 1. 
  #define DYNPD__DPL_P2       0x04  // Enable dynamic payload length data pipe 2. 
  #define DYNPD__DPL_P3       0x08  // Enable dynamic payload length data pipe 3. 
  #define DYNPD__DPL_P4       0x10  // Enable dynamic payload length data pipe 4. 
  #define DYNPD__DPL_P5       0x20  // Enable dynamic payload length data pipe 5.
                                    // ( All abouve requires EN_DPL)
  #define DYNPD__DPL_RESERVED 0xC0  // Only '0' allowed
//-----------------------------------------------------------------------------
// RF24 FEATURE fields 0x1D
  #define FEATURE__EN_DYN_ACK 0x01  // Enables the W_TX_PAYLOAD_NOACK command
  #define FEATURE__EN_ACK_PAY 0x02  // Enables Payload with ACK
  #define FEATURE__EN_DPL     0x04  // Enables Dynamic Payload Length
  #define FEATURE__RESERVED   0xF8  // Only 0 allowed. 1 reset to default values
//-----------------------------------------------------------------------------
// RF24 SETUP_VALUE fields 0x1E
  #define SETUP_VALUE__RX_SETUP_VALUE xFF // RX_SETUP time, the time between Standby to RX mode
  #define SETUP_VALUE__TX_SETUP_VALUE xFF // TX_SETUP time, the time between Standby to TX mode
  #define SETUP_VALUE__RX_TM_CNT      xFF // 3rd byte. Rx timeout counter.
  #define SETUP_VALUE__REG_MBG_WAIT   xFF // 4th byte. Main bandgap wait counter
  #define SETUP_VALUE__REG_LNA_WAIT   xFF // 5th byte. Lna wait counter
//-----------------------------------------------------------------------------
// RF24 PRE_GURD fields 0x1F
  #define PRE_GURD__GRD_CNT   0x0F // Number of Pre-Guard bit before preamble
  #define PRE_GURD__GRD_EN    0x10 // Pre-Guard enable
  #define PRE_GURD__TAIL_CTL  0x80 // Number of repeat bit after the CRC
  #define PRE_GURD__SPARE_REG xFF // 2nd byte. Output to analogue
//-----------------------------------------------------------------------------
// RF24 ACTIVATE addresses
// RF24 TOGGLE_BANK fields 0x53
  #if defined __SE8R01__
  #define ACTIVATE__SWAP_BANK   0x53 // Toggle banks. Switch register register bank is done by SPI
                                     // command “ACTIVATE” followed by 0x53
                                     // 0: Register bank 0
                                     // 1: Register bank 1
  #elif defined __nRF24L01P__
  #define ACTIVATE__TOGGLE_CMD  0x73 // This key activates the following features:
                                     // • R_RX_PL_WID
                                     // • W_ACK_PAYLOAD
                                     // • W_TX_PAYLOAD_NOACK
                                     // A new ACTIVATE command with the same data deactivates them
                                     // again. This is executable in power down or stand by modes
                                     // only. The R_RX_PL_WID, W_ACK_PAYLOAD, and
                                     // W_TX_PAYLOAD_NOACK features registers are initially in a
                                     // deactivated state; a write has no effect, a read only
                                     // results in zeros on MISO. To activate these registers, use
                                     // the ACTIVATE command followed by data 0x73. Then they can
                                     // be accessed as any other register. Use the same command and
                                     // data to deactivate the registers again.
  #endif
//-----------------------------------------------------------------------------

//-- RF24 delay --

  #define SPI_RF24_LIMIT    10000000UL
  #define T_POWERON             100000    // Tpoweron     100ms
  #if defined __SE8R01__
    #define T_PD2STBY             2000    // Tpd2stby       2ms
    #define T_STBY2A               210    // Tstby2a      250us
  #elif defined __nRF24L01P__
    #define T_PD2STBY             1500    // Tpd2stby     1.5ms
    #define T_STBY2A               130    // Tstby2a      130us
  #endif
  #define T_DELAY_AGC               20    // Tdelay_AGC    20us
  #define T_HCE                     10    // Thce          10us
  #define T_PECE2CSN                 4    // Tpece2csn      4us
  #define T_PECSN2ON                 5    // Tpecsn2on      5us
  #define T_PECSN2OFF              220    // Tpecsn2off   220us

// Atraso CSn e esqumático
/****************************************************************************

  T_PECSN2ON  = 50 * 0,1;           // Capacitância em pF, tempo em milisegundos.
           |
           `--> 50Ω x 0,0000001uF   = 0,000005s  ->  5us; tempo de acionamento.

  T_PECSN2OFF =  2200 * 0,1;        // Capacitância em pF, tempo em milisegundos.
           |
           `--> 2.2kΩ x 0,0000001uF = 0,001s  ->  220us; tempo para desligamento.
  */  
  // Obs.:
  // * Ao alterar o valor do resistor, altere também o valor da diretiva `T_PECSN2OFF`.
  //   Sem este ajuste o sistema pode não funcionar, ou funcionar com debilidade.
  // * Resistor com valor muito baixo interfere no carregamento do código fonte.
  // * Valor de 1kΩ foi testado e funcionou bem. Contudo se faz necessário
  //   conectá-lo somente após a carga do código fonte, na sequência dar reset.
  // * Usar diodo de germânio que dá queda de tensão de 0,2V. Diodo de silício
  //   o valor mínino de tensão é de 0,6V sendo necessário para o chip 0,3V.
  /*  
                                                           //
                               +---|<|----x--[2k2]--x---|<|--- 5V 
                               |   1n60   |         |   LED
                               |          |         |  (red)
                               |  +--||---x         |          +-----+
                +-\/-+         |  | 100nF |         |--- CE   3| R R |
    RESET PB5  1|o   |8  Vcc --|--|-------|---------x--- VCC  2| S F |
    NC    PB3  2|    |7  PB2 --x--|-------|------------- SCK  5| E 2 |
    NC    PB4  3|    |6  PB1 -----|-------|------------- MISO 6| 8 4 |
       +- GND  4|    |5  PB0 -----|-------|------------- MOSI 7| R L |
       |        +----+            |       +------------- CSN  4| 0 0 |
       +--------------------------x--------------------- GND  1| 1 1 |
                                                               +-----+

****************************************************************************/
           

// -----------------------------------------------
// Class RF24
// -----------------------------------------------

// Number of radios (Change to desired quantity).
#define RADIO_AMOUNT          12

// flag state
  #define MODE_STATE_CTRL   0x0007
  #define _MODE__POWERDOWN  0x0000
  #define _MODE__STANDBYRX  0x0001
  #define _MODE__MODERX     0x0002
  #define _MODE__MODETX     0x0003
  #define _MODE__STANDBYTX  0x0004
  #define _MODE__INVALID_5  0x0005
  #define _MODE__INVALID_7  0x0006
  #define ACTIVED_CE        0x0008
  #define ACTIVED_CS        0X0010
  #define ACTIVED_IRQ       0x0020
  #define ENABLED           0x0040
  #define SELECTED          0X0080
  #define MODE_DYN_ACK      0x0100
  #define MODE_DPL          0X0200
  #define MODE_ACK_PAY      0X0400
  #define MODE_FAN_OUT      0X0800
  // #define MODE_UNDEFINED_5  0x1000
  // #define MODE_UNDEFINED_6  0x2000
  // #define MODE_UNDEFINED_7  0x4000
  // #define MODE_UNDEFINED_8  0x8000

#define RX HIGH
#define TX LOW

typedef enum {
  PA_18dbm = 0,   // Output -18 dbm
  PA_12dbm,       // Output -12 dbm
  PA_6dbm,        // Output  -6 dbm
  PA_0dbm,        // Output   0 dbm
  #ifdef __SE8R01__
    PA_5dbm       // Output   5 dbm
  #elif defined __nRF24L01P__
    PA_5dbm = 3
  #endif
} ePA;

typedef enum {
  DR_1Mbps      = 0,  // 1Mbps
  DR_2Mbps      = 1,  // 2Mbps
  DR_500Kbps    = 2,  // 500Kbps ou 250Kbps
  DR_250Kbps    = 2,  // 500Kbps ou 250Kbps
  DR_LOWKbps    = 2
} eDataRate;

typedef struct{
  uint8_t ID = 0x00;
  // uint8_t RX_PW = 0; //<- payloadWidth
  // uint8_t flag = ENAA | DPL | ENRX | NOACK;
  // e outros
} sRadio;

class acRF24Class {
public:
  uint8_t payload[32];
  uint8_t recData[5];
//== Inicialização ============================================================
  acRF24Class(u8 selfID, u8 CSpin = xFF, u8 CEpin = xFF, u8 IRQpin = xFF);
  ~acRF24Class(){};
  void begin();
//== Controle do chip =========================================================
  void setPowerDown();      // CONFIG  0x00 (PWR_UP)
  void setStandbyRX();      // CONFIG  0x00 (PWR_UP, PRIM_RX)
  void setStandbyTX();      // CONFIG  0x00 (PWR_UP, PRIM_RX)
  void setModeRX();         // CONFIG  0x00 (PWR_UP, PRIM_RX)
  void setModeTX();         // CONFIG  0x00 (PWR_UP, PRIM_RX)
  uint8_t getMode();
//== Comunicação ==============================================================
  void spiTransfer(uint8_t cmd, uint8_t* buf, uint8_t amount);
  uint8_t command(uint8_t rec);
//== Comandos Nativo ==========================================================
  uint8_t rRegister(uint8_t rec);  // R_REGISTER
  uint8_t wRegister(uint8_t rec);  // W_REGISTER
  uint8_t Activate(uint8_t cmd);   // ACTIVATE
  uint8_t rRXpayloadWidth();       // R_RX_PL_WID
  uint8_t rRXpayload();            // R_RX_PAYLOAD
  uint8_t wTXpayload();            // W_TX_PLOAD
  uint8_t wACKpayload();           // W_ACK_PAYLOAD
  uint8_t wTXpayloadNoACK();       // W_TX_PAYLOAD_NO_ACK
  uint8_t reuseTXpayload();        // REUSE_TX_PL
  uint8_t flushRX();               // FLUSH_RX
  uint8_t flushTX();               // FLUSH_TX
  uint8_t nop();                   // NOP
//== Configurações ============================================================
  void setSufixo(uint8_t* buf);
  void getSufixo(uint8_t* buf);
  void setPayload(uint8_t* buf, uint32_t len);  // Copia para o payload[].
  void getPayload(void* buf, uint32_t len);     // Copia para o *buf.
  void setRFchannel(uint8_t ch);                // RF_CH      0x05
  uint8_t getRFchannel();
  void setPApower(ePA pa);                      // RF_SETUP   0x06 (PA_PWR)
  ePA getPApower();
  void setDataRate(eDataRate dr);               // RF_SETUP   0x06 (RF_DR)
  eDataRate getDataRate();
  //-- Configurações de modo de operação --------------------------------------
  // -- RX
  void setStaticPayload(uint8_t pipe, uint8_t len);
  bool isStaticPayload(uint8_t pipe);
  void setStaticPayload(uint8_t len);   // Específico para uso com o modo fan-out
  bool isStaticPayload();               // Específico para uso com o modo fan-out
  void setDynamicPayload(uint8_t pipe);
  bool isDynamicPayload(uint8_t pipe);
  void setDynamicPayload();             // Específico para uso com o modo fan-out
  bool isDynamicPayload();              // Específico para uso com o modo fan-out
  void setAutoAcknowledgement(uint8_t pipe, bool en);
  bool isAutoAcknowledgement(uint8_t pipe);
  void setAutoAcknowledgement(bool en); // Específico para uso com o modo fan-out
  bool isAutoAcknowledgement();         // Específico para uso com o modo fan-out
  // -- TX
  void setAutoRetransmissionCount(uint8_t arc);  // SETUP_RETR   0x04
  uint8_t getAutoRetransmissionCount();
  void setAutoRetransmissionDelay(uint8_t ard);
  uint8_t getAutoRetransmissionDelay();
//== Manipulação dos rádios e canais ==========================================
  // -- Rádios --------------------------------------------------------------
  void setRadios(uint8_t* buf);
  void setTXradio(uint8_t r);
  uint8_t getTXradio();
  void setTXaddr(uint8_t pipe);             // TX_ADDR    0x10
  uint8_t getTXaddr();
  // -- Fan-out ---------------------------------------------------------------
  void enableFanOut(bool en);
  uint8_t sourceID();
//== Comandos para fins de suporte ============================================
  bool chipActived();
  bool isAvailableRX();             // FIFO_STATUS  0x17 (RX_EMPTY)
  bool isAvailableTX();             // FIFO_STATUS  0x17 (TX_FULL)
  void watchTX(uint32_t ms);
  uint8_t rxPipeNo();               // STATUS       0x07
  uint8_t getSelfID();
  uint8_t staticTXpayloadWidth();
  #ifdef __SE8R01__
    uint8_t selectBank(uint8_t bank); // ACTIVATE__CHANGE_BANK
  #elif defined __nRF24L01P__
    uint8_t toggleFeature();          // ACTIVATE__TOGGLE_CMD
  #endif
  #ifdef TEST_VARS
   void getVars(uint8_t* sts); // <- Para testes.
  #endif
protected:
private:
  uint32_t pv_watchTXinterval = 0;
  uint32_t pv_watchTX = 0;
  uint8_t CS, CE, IRQ;
  uint16_t pv_flagState = MODE_STATE_CTRL;
  uint8_t pv_lastStatus;
  uint8_t pv_recAmount;
  uint8_t pv_sufixo[4];
  uint8_t pv_txPayloadWidth = 0;
  uint8_t pv_RFchannel;
  // -- Rádio
  uint8_t pv_selfID   = 0;
  uint8_t pv_targetID = 0;
  uint8_t pv_sourceID = 0;
  sRadio radio[RADIO_AMOUNT];
//== Inicialização ============================================================
  void resetConfig();
  #ifdef __SE8R01__
    void configBank1();
  #endif
//== Controle do chip =========================================================
  void setCE( bool enable);
  void setCS( bool select);
  void setCSn( bool selectn);
  void goStandby( bool RXtx);
  void goMode(uint8_t m);
//== Comunicação ==============================================================
  // -- RX
  void setModePayload(uint8_t pipe, bool dyn, uint8_t len = 0);
//== Comandos Nativo ==========================================================

  uint8_t internal_wTXpayload( uint8_t wTX);
//== Configurações ============================================================
  //-- Configurações de modo de operação --------------------------------------
  void enablePipe(uint8_t pipe, bool en);
  // -- TX
  void enableDYN_ACK(bool en);
  bool isDYN_ACK();
  void enableDPL(bool en);
  bool isDPL();
  void enableACK_PAY(bool en);
  bool isACK_PAY();
  void setTXpayloadWidth(uint8_t w);
  uint8_t internalTXpayloadWidth();
  // -- RX
  uint8_t internalRXpayloadWidth();
//== Manipulação dos rádios e canais ==========================================
  void setRadioID(u8 p, u8 id);
  uint8_t getRadioID(u8 p);
  void radioExchange(uint8_t r_in, uint8_t p_in);
  void pipeReplace(uint8_t r, uint8_t p);
  uint8_t hasRadio(uint8_t r);
  uint8_t deleteRadio(uint8_t r);
  // -- Fan-out ---------------------------------------------------------------
  bool isFanOut();
//== Comandos para fins de suporte ============================================
  bool flagState(uint16_t f);
  void flagState(uint16_t f, bool e);
  void setFlagStateCtrl(uint16_t f);
  bool flagStateCtrl(uint16_t f);
  void clearTX_DS();
  void clearRX_DR();
  void clearIRQ();
};

// #endif

