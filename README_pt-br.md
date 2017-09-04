
**Biblioteca acRF24 para se8r01 e nRF24L01+ para trabalhar com arduino e ATtiny84/85**

Por não encontrar um biblioteca que suprisse minhas necessidade desenvolvi esta que apresento aqui.
* Contém definições para uso com **nRF24L01+** (na versão 0.0.1 não está totalmente testada).
* Baseado nos manuais:
[SE8R01 specification version 1.6 2014-03-05](http://community.atmel.com/sites/default/files/forum_attachments/SE8R01_DataSheet_v1%20-%20副本.pdf)
 e [nRF24L01P Product Specification 1.0](https://www.nordicsemi.com/eng/content/download/2726/34069/file/nRF24L01P_Product_Specification_1_0.pdf).
* Desenvolvimento voltado à interface de alto nível.
* É reservado métodos para acesso de baixo nível ao chip.
* Contém SPI próprio, adaptável com conexõe suprimida para uso em ATtiny85.
* Métodos desenvolvidos no propósito de usar o automatismo já contido no chip.
* Desenvolvimento com o propósito de compatibilidade entre chips.
* Posibilita até 254 rádios.

Diretivas
------------
  A complilação está ativa para o chip **SE8R01** com a diretiva `__SE8R01__`.
  
  Em caso de compilar para **nRF24L01+**, usar a diretiva `__nRF24L01P__`.

  Vá ao topo do arquivo `acRF24.h` e altere o comentário como desejado.

```
#pragma once

#define __SE8R01__        // <- Comente se não usar
// or
// #define __nRF24L01P__

...

```


`sourceID()`
------------
  Modo Fan-Out usa o primeiro byte de payload para identificar o rádio do qual
  está sendo envianda a mensagem. Portanto o tamanho máximo de dados passa a
  ser 31. O processo é interno e é possível se ter acesso a informação de qual
  rádio está enviando a messagem, ao chamar `sourceID()`.
  
  Este método facilita o uso de até *254* rádios:    
  – Rádio ID 0 indica a inexistência de rádio e será ignorado;    
  – Rádio ID 255 indica cabeçalho, será ignorado.    
  Quantidade: 256 - ( neutro + cabeçalho ) = *254*.
  
  Para muitos rádios há um expressivo uso de memória, por este motivo foi escolhido uma
  configuração base de 12 rádios. Em havendo a necessidade de um número maior,
  então alterar em `acRF24.h`:

```
...

// Quantidade de rádios (Mudar para quantidade desejada).
#define RADIO_AMOUNT          12

// flag state
...
```

  Substituir 12 pela quantidade desejada. Observe o limite de *254*.


`watchTX()`
------------
  Quando cai o rádio receptor por longo período, *ACK* não retorna fazendo o
  transmissor ficar inoperante.

  `watchTX()` define o tempo em milesegundos que o transmissor ficará esperando
  a resposta *ACK*, enquanto espera é chamado `reuseTXpayload()`, após este
  tempo `flushTX()` é chamado e assim liberando o transmissor para operar com
  outros rádios.


`enableFanOut()`
------------
  Chame `enableFanOut(true)` para ativar a possibilidade de receber a
  identificação do rádio que está enviando a mensagem. Esta ativação deve ser
  comum aos rádios que irão se cominicar.


Ajuda!
------------
  Por falta da elaboração de um arquivo de ajuda, favor analizar os arquivos de
  exemplo. Adapte os mesmos para a necessidade do projeto.


Teste

  Os testes de desenvolvimento foram feitos entre um *Arduino UNO* e um *ATTiny85*.
  
  Inicialmente os exemplos serão baseados nesta configuração.


Atraso CSn e esqumático
------------
```  
T_PECSN2ON  = 50 * 0.1;          // <- Capacitância em pF, tempo em milisegundos.
        `--> 50Ω x 0.0000001uF   = 0.000005s  ->  5us; tempo de acionamento.

T_PECSN2OFF = 2200 * 0.1;        // <- Capacitância em pF, tempo em milisegundos.
        `--> 2.2kΩ x 0.0000001uF = 0.001s   ->   220us; tempo para desligamento.
```  
  Obs.: 
  * Resistor com valor muito baixo interfere no carregamento do código fonte.
  * Valor de 1kΩ foi testado e funcionou bem. Contudo se faz necessário
    conectá-lo somente após a carga do código fonte, na sequência dar reset.
  * Usar diodo de germânio que dá queda de tensão de 0,2V. Diodo de silício
    o valor mínino de tensão é de 0,6V sendo necessário para o chip 0,3V.
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
  A biblioteca providencia um método automático para ajustar o clock limite do chip.


Ajude-me
------------
  Devido a pouco tempo disponível para o desenvolvimento, apresento este projeto
  na forma que se vê. Pesso desculpas mas até o momento foi o que consequi fazer.
  
  Meu inglês é fraco, na medida do possível, que depende de tempo disponível,
  procederei a tradução.
  
  Comentários e sujestões ajudarão no aprimoramento do projeto. Seja bem vindo.


Agradecimentos
------------
  **Agradeço a God.**
  
------------

