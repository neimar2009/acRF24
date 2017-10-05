
## Biblioteca acRF24 para se8r01 e nRF24L01+ para trabalhar com arduino e ATtiny84/85

### Índice:
- [Desenvolvimento](https://github.com/neimar2009/acRF24/blob/master/docs/README_pt-br.md#desenvolvimento)
- [Diretives](https://github.com/neimar2009/acRF24/blob/master/docs/README_pt-br.md#diretivas)    
- [sourceID()](https://github.com/neimar2009/acRF24/blob/master/docs/README_pt-br.md#sourceid)    
- [watchTX()](https://github.com/neimar2009/acRF24/blob/master/docs/README_pt-br.md#watchtx)    
- [enableFanOut()](https://github.com/neimar2009/acRF24/blob/master/docs/README_pt-br.md#enablefanout)    
- [ATTiny](https://github.com/neimar2009/acRF24/blob/master/docs/README_pt-br.md#attiny)    
- [Atraso CSn e esquemático](https://github.com/neimar2009/acRF24/blob/master/docs/README_pt-br.md#atraso-csn-e-esquemático)    
- [Clock](https://github.com/neimar2009/acRF24/blob/master/docs/README_pt-br.md#clock)    
- [Ajuda!](https://github.com/neimar2009/acRF24/blob/master/docs/README_pt-br.md#ajuda)
- [Teste](https://github.com/neimar2009/acRF24/blob/master/docs/README_pt-br.md#teste)
- [Ajude-me](https://github.com/neimar2009/acRF24/blob/master/docs/README_pt-br.md#ajude-me)
- [Agradecimentos](https://github.com/neimar2009/acRF24/blob/master/docs/README_pt-br.md#agradecimentos)

Desenvolvimento
------------
Por não encontrar um biblioteca que suprisse minhas necessidade desenvolvi esta que apresento aqui.
* Contém definições para uso com **nRF24L01+** (na versão 0.0.3 não está totalmente testada).
* Baseado nos manuais:
[SE8R01 specification version 1.6 2014-03-05](http://community.atmel.com/sites/default/files/forum_attachments/SE8R01_DataSheet_v1%20-%20副本.pdf)
 e [nRF24L01P Product Specification 1.0](https://www.nordicsemi.com/eng/content/download/2726/34069/file/nRF24L01P_Product_Specification_1_0.pdf).
* Desenvolvimento voltado à interface de alto nível.
* É reservado métodos para acesso de baixo nível ao chip.
* Contém SPI próprio, adaptável com conexões suprimida para uso em ATtiny85.
* Métodos desenvolvidos no propósito de usar o automatismo já contido no chip.
* Desenvolvimento com o propósito de compatibilidade entre chips.
* Possibilita até 254 rádios.


Diretivas
------------
  A complilação está ativa para o chip **SE8R01** com a diretiva `__SE8R01__`.

  Em caso de compilar para **nRF24L01+**, usar a diretiva `__nRF24L01P__`.

  Vá ao topo do arquivo `acRF24directives.h` e altere o comentário como desejado.

```
/************************************************************/
/*           Comment out the unused directive.              */
/*                                                          */
#define __SE8R01__       // <- Comment if you do not use
// #define __nRF24L01P__    // <- Comment if you do not use
/*                                                          */
/************************************************************/
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
  então alterar em `acRF24directives.h`:

```
/************************************************************/
/*      Number of radios (Change to desired quantity).      */
/*                                                          */
#define RADIO_AMOUNT        12
/*                                                          */
/************************************************************/
```

  Substituir 12 pela quantidade desejada. Observe o limite de **254**.


`watchTX()`
------------
  Quando cai o rádio receptor por longo período, **ACK** não retorna fazendo o
  transmissor ficar inoperante.

  `watchTX()` define o tempo em milesegundos que o transmissor ficará esperando
  a resposta **ACK**, enquanto espera é chamado `reuseTXpayload()`, após este
  tempo `flushTX()` é chamado e assim liberando o transmissor para operar com
  outros rádios.


`enableFanOut()`
------------
  Chame `enableFanOut(true)` para ativar a possibilidade de receber a
  identificação do rádio que está enviando a mensagem. Esta ativação deve ser
  comum aos rádios que irão se cominicar.


ATTiny
------------
  Núcleo para ATtiny usado nesse desenvolvimento:

  [David A. Mellis](https://github.com/damellis/attiny)    
  URL de instalação:    
  'https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json'

  e

  [Spence Konde (aka Dr. Azzy)](https://github.com/SpenceKonde/ATTinyCore)    
  URL de instalação:    
  'http://drazzy.com/package_drazzy.com_index.json'

  Escolha o arquivo **json** correspondente, copie a URL e inclua em:

  _Preferências... -> URLs Adicionais para Gerenciamento de Placas:_

  Nota:
  - _Spence Konde_ é um trabalho mais completo.
  - _David A. Mellis_ usa menos recurso.

Atraso CSn e esquemático
------------
```   
  #define T_PECSN2OFF     220 // Capacitance in pF, time in milliseconds.
                              // Resistência externa escolhida  : 2200Ω
                              // Capacitor escolhido por padrão : 100nF
                              // 2.2kΩ x 0.0000001uF = 0.00022s -> 220us standby time.
```
  Note:
  * Ao alterar o valor do resistor, ajuste o valor da diretiva T_PECSN2OFF
    em "acRF24direcrives.h". Sem este ajuste o sistema pode não funcionar,
    ou funcionar com debilidade.    
  * Não é previsto a alteração do valor do capacitor, o ajuste é dado apenas pela
    alteração do resistor. Em caso de alteração deste valor, considere também a
    necessidade de ajustar T\_PECSN2ON. Valores menor que 5 para T_PECSN2ON provoca
    inconsistência ou inoperância no sistem. Favor reportar o resultado.
  * Resistor com valor muito baixo interfere no carregamento do código fonte.
  * Valor de 1kΩ foi testado e funcionou bem. Contudo se faz necessário
    conectá-lo somente após a carga do código fonte, na sequência dar reset.
  * Usar diodo de germânio que dá queda de tensão de 0,2V. Diodo de silício
    o valor mínino de tensão é de 0,6V sendo necessário para o chip 0,3V.
  * O resistor de 220Ω é opcional e serve para protejer o programador.
```
                                                           //
                               +----|<|----x--[2k2]--x----|<|---- 5V
                               |    1n60   |         |    LED
                               |           |         |   (red)
                               |  +---||---x         |          +-----+
                +-\/-+         |  |  100nF |         |--- CE   3| R R |
    RESET PB5  1|o   |8  Vcc --|--|--------|---------x--- VCC  2| S F |
    NC    PB3  2|    |7  PB2 --x--|--------|------------- SCK  5| E 2 |
    NC    PB4  3|    |6  PB1 -----|--------|--[220]------ MISO 7| 8 4 |
       +- GND  4|    |5  PB0 -----|--------|------------- MOSI 6| R L |
       |        +----+            |        +------------- CSN  4| 0 0 |
       +--------------------------x---------------------- GND  1| 1 1 |
                                                                +-----+
```
  |Solução de problema|
  :---:
  | Foi encontrado uma resistência parasita entre CSN e VCC em alguns chip SE8R01, valor este que ainda não está definido. Em caso de falha deve ser aumentado o valor de 2k2 para que em paralelo ao valor parasita resulte num valor aproximando de 2k2 desejado (experimente 3k3 ou 3k9). |


Clock
------------
  A biblioteca providencia um método automático para ajustar o clock limite do chip.


Teste
------------
  Os testes de desenvolvimento foram feitos entre um **Arduino UNO** e um **ATTiny85**.

  Inicialmente os exemplos serão baseados nesta configuração.


Arquivo de ajuda
------------
  Por falta da elaboração de um arquivo de ajuda, favor analizar os arquivos de
  exemplo. Adapte os mesmos para a necessidade do projeto.


Ajude-me
------------
  Devido a pouco tempo disponível para o desenvolvimento, apresento este projeto
  na forma que se vê. Me desculpe, mas até o momento é o que consegui desenvolver.

  Meu inglês é fraco, na medida do possível, que depende de tempo disponível,
  procederei a tradução.

  Comentários e sugestões ajudarão no aprimoramento do projeto. Seja bem vindo.


Agradecimentos
------------
  **Agradeço a God.**

------------
