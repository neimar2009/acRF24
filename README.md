
**Library acRF24 for se8r01 and nRF24L01+ for working with arduino and ATtiny84/85**

[Poruguês](docs/README_pt-br.md), [Français](docs/README_fr.md), [русский](docs/README_ru.md)

Because I did not find a library that would meet my needs I developed the one I present here.
* Contains settings for use with **nRF24L01+** (in version 0.0.1 not fully tested).
* Based on manuals:
[SE8R01 specification version 1.6 2014-03-05](http://community.atmel.com/sites/default/files/forum_attachments/SE8R01_DataSheet_v1%20-%20副本.pdf)
 and [nRF24L01P Product Specification 1.0](https://www.nordicsemi.com/eng/content/download/2726/34069/file/nRF24L01P_Product_Specification_1_0.pdf).
* Development focused on the high level interface.
* Methods for low-level access to the chip are reserved.
* Contains own SPI, adaptable with suppressed connections for use in ATtiny85.
* Methods developed in order to use the automatism already contained in the chip.
* Development for the purpose of chip compatibility.
* It enables up to 254 radios.

Directives
------------
  The compilation is active for the **SE8R01** chip with the `__SE8R01__` directive.    
  In case of compiling to **nRF24L01+**, use the `__nRF24L01P__` directive.    
  Go to the top of the file `acRF24.h` and change the comment as desired.

```
#pragma once

#define __SE8R01__        // <- Comment if you do not use
// or
// #define __nRF24L01P__

...

```

`sourceID()`
------------
  Fan-Out Mode uses the first byte of payload to identify the radio from which
  the message is being sent. Therefore the maximum data size becomes 31. The
  process is internal and it is possible to have access to the information of
  which radio is sending the message, when calling `sourceID ()`.
  
  This method facilitates the use of up to *254* radios:    
  – Radio ID 0 indicates no radio and will be ignored;    
  – Radio ID 255 indicates header, will be ignored.    
  Quantity: 256 - (neutral + header) = *254*.
  
  For many radios there is an expressive use of memory, for this reason a base
  configuration of 12 radios has been chosen. If there is a need for a larger
  number, then change to `acRF24.h`:

```
...

// Number of radios (Change to desired quantity).
#define RADIO_AMOUNT          12

// flag state
...
```
  Replace 12 with the desired amount. Observe the limit of *254*.


`watchTX()`
------------
  When the radio receiver falls for a long period of time, * ACK * does not
  return causing the transmitter to become inoperative.
  
  `watchTX()` sets the time in milliseconds that the transmitter will wait
  for the * ACK * response, while waiting is called `reuseTXpayload()`, after
  this time `flushTX()` is called and thus releasing the transmitter to
  operate with others radios


`enableFanOut()`
------------
  Call `enableFanOut(true)` to enable the possibility of receiving the
  identification of the radio that is sending the message. This activation
  should be common to radios that will commence.


ATTiny
------------
  Core for ATTiny developed by [David A. Mellis](https://github.com/damellis/attiny)


CSn delay, and schematic
------------
```
  T_PECSN2ON  = 50 * 0.1;          // <- Capacitance in pF, time in milliseconds.
          `--> 50Ω x 0.0000001uF   = 0.000005s  ->  5us; drive time.

  T_PECSN2OFF = 2200 * 0.1;        // <- Capacitance in pF, time in milliseconds.
          `--> 2.2kΩ x 0.0000001uF = 0.001s   ->   220us; drive time.
```
  Note: 
  * When changing the value of the resistor, also change the value of the `T_PECSN2OFF` directive.    
    Without this adjustment the system may not work, or operate with weakness.
  * Resistor with very low value interferes with loading the source code.
  * Value of 1kΩ was tested and worked well. However it is necessary to connect
    it only after loading the source code, in the sequence reset.
  * Use Germanium diode that gives voltage drop of 0.2V. Silicon Diode the
    minimum voltage value is 0.6V being required for the 0.3V chip.
```
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
```

Clock
------------
 The library provides an automatic method to adjust the chip limit clock.


Help!
------------
  For lack of elaboration of a help file, please analyze the sample files.
  Adapt the same to the project need.


Test
------------
  Development tests were done between an *Arduino UNO* and an *ATTiny85*.
  
  Initially the examples will be based on this configuration.


Help me
------------
  Due to the limited time available for development, I present this project in the
  way that you see it. Personally, I'm sorry but so far I have been able to do so.
  
  My English is weak, to the extent possible, depending on available time, I will
  translate.
  
  Comments and suggestions will help in improving the project. Welcome.

Thanks
------------
  **I thank God.**
  
------------

