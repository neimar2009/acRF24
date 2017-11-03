
#Library acRF24 for se8r01 and nRF24L01+ for working with arduino and ATtiny84/85#
----

:----:
|So far, the sample files are not upgraded to a new version.|

---
     
[Português](docs/README_pt-br.md), [Français](docs/README_fr.md), [русский](docs/README_ru.md)

Because I did not find a library that would meet my needs I developed the one I present here.
* Contains settings for use with **nRF24L01+** (in version 0.0.3 not fully tested).
* Based on manuals:
[SE8R01 specification version 1.6 2014-03-05](http://community.atmel.com/sites/default/files/forum_attachments/SE8R01_DataSheet_v1%20-%20副本.pdf)
 and [nRF24L01P Product Specification 1.0](https://www.nordicsemi.com/eng/content/download/2726/34069/file/nRF24L01P_Product_Specification_1_0.pdf).
* Development focused on the high level interface.
* Methods for low-level access to the chip are reserved.
* Contains own SPI, adaptable with suppressed connections for use in ATtiny85.
* Methods developed in order to use the automatism already contained in the chip.
* Development for the purpose of chip compatibility.
* It enables up to 254 radios.


------------
Directives
------------
  The compilation is active for the **SE8R01** chip with the `__SE8R01__` directive.    
  In case of compiling to **nRF24L01+**, use the `__nRF24L01P__` directive.    
  For convenience, the possibility of including the directives in the main file before the declaration `#include <acRF24.h>` was implemented,    

  or    

  go to the top of the file `acRF24directives.h` and change the comment as desired.

```
#if !defined(__SE8R01__) && !defined(__nRF24L01P__)
/************************************************************/
/*           Comment out the unused directive.              */
/*                                                          */
#define __SE8R01__       // <- Comment if you do not use
// #define __nRF24L01P__    // <- Comment if you do not use
/*                                                          */
/************************************************************/
#endif
```

------------
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
  configuration of 24 radios has been chosen. If a larger number of radios are required, include the directives    
  `#define RADIO_AMOUNT 24`    
  with the desired value in the main file before the    
  `#include <acRF24.h>`    
  statement.

  or    

  then change to `acRF24directives.h`:

```
#if !defined(RADIO_AMOUNT)
/************************************************************/
/*      Number of radios (Change to desired quantity).      */
/*                                                          */
#define RADIO_AMOUNT        24
/*                                                          */
/************************************************************/
#endif
```
  Replace 12 with the desired amount. Observe the limit of *254*.


------------
`watchTX()`
------------
  When the radio receiver falls for a long period of time, *ACK* does not
  return causing the transmitter to become inoperative.
  
  `watchTX()` sets the time in milliseconds that the transmitter will wait
  for the *ACK* response, while waiting is called `reuseTXpayload()`, after
  this time `flushTX()` is called and thus releasing the transmitter to
  operate with others radios


------------
`enableFanOut()`
------------
  Call `enableFanOut(true)` to enable the possibility of receiving the
  identification of the radio that is sending the message. This activation
  should be common to radios that will commence.


------------
ATTiny
------------
  Core for ATtiny used in this development:

  [David A. Mellis](https://github.com/damellis/attiny)    
  Installation URL:    
  'https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json'

  and

  [Spence Konde (aka Dr. Azzy)](https://github.com/SpenceKonde/ATTinyCore)    
  Installation URL:    
  'http://drazzy.com/package_drazzy.com_index.json'

  Choose the corresponding **json** file, copy the URL and include in:

  _Preferences ... -> Additional URLs for Card Management:_

  Notice:
  - _Spence Konde_ is a more complete work.
  - _David A. Mellis_ uses less resource.


------------
CSn delay, and schematic
------------
```
  #define T_PECSN2OFF     220 // Capacitance in pF, time in milliseconds.
                              // External resistance chosen   : 2200Ω
                              // Capacitor chosen by default  : 100nF
                              // 2.2kΩ x 0.0000001uF = 0.00022s -> 220us standby time.
```
  Note: 
  * When changing the resistor value, set the value of the T_PECSN2OFF directive
    to "acRF24direcrives.h". Without this adjustment the system may not work, or
    operate with weakness.
  * It is not foreseen to change the value of the capacitor, the adjustment is
    only given by the change of the resistor. If this value is changed, also
    consider the need to adjust T\_PECSN2ON. Values less than 5 for T_PECSN2ON
    causes inconsistency or inoperability in the system. Please report the result.
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


  |Troubleshoot|
  :---:
  | A parasitic resistance between CSN and VCC was found on some SE8R01 chip, which is still not defined. In case of failure the value of 2k2 must be increased so that in parallel to the parasitic value it results in an approximate value of 2k2 (try 2k7 or 3k3). The parasite resistor has an estimated value of 20k.|


------------
Clock
------------
 The library provides an automatic method to adjust the chip limit clock.


------------
Test
------------
  Development tests were done between an *Arduino UNO* and an *ATTiny85*.
  
  Initially the examples will be based on this configuration.


------------
Help file
------------
  For lack of elaboration of a help file, please analyze the sample files.
  Adapt the same to the project need.


------------
Help me
------------
  Due to the limited time available for development, I present this project in the
  way that you see it. I'm sorry, but so far I've been able to develop.
  
  My English is weak, to the extent possible, depending on available time, I will
  translate.
  
  Comments and suggestions will help in improving the project. Welcome.


------------
Thanks
------------
  **I thank God.**
  
------------

