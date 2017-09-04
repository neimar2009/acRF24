
**Bibliothèque acRF24 pour se8r01 et nRF24L01+ pour travailler avec arduino et ATtiny84/85**

Parce que je n'ai pas trouvé une bibliothèque qui répondrait à mes besoins, j'ai développé celui que je présente ici.
* Contient des paramètres à utiliser avec **nRF24L01+** (dans la version 0.0.1 pas entièrement testé).
* Basé sur les manuels:
[SE8R01 specification version 1.6 2014-03-05](http://community.atmel.com/sites/default/files/forum_attachments/SE8R01_DataSheet_v1%20-%20副本.pdf)
 e [nRF24L01P Product Specification 1.0](https://www.nordicsemi.com/eng/content/download/2726/34069/file/nRF24L01P_Product_Specification_1_0.pdf).
* Développement axé sur l'interface de haut niveau.
* Les méthodes d'accès de faible niveau à la puce sont réservées.
* Contient SPI propriétaire et adaptatif avec des connexions supprimées à utiliser dans ATtiny85.
* Méthodes développées pour utiliser l'automatisme déjà contenu dans la puce.
* Développement pour la compatibilité des puce.
* Il permet jusqu'à 254 radios.

Directives
------------
  La compilation est active pour la puce **SE8R01** avec la directive `__SE8R01__`.
  
  En cas de compilation sur **nRF24L01+**, utilisez la directive `__nRF24L01P__`.

  Accédez au haut du fichier `acRF24.h` et modifiez le commentaire comme vous le souhaitez.

```
#pragma once

#define __SE8R01__        // <- Comment if you do not use
// or
// #define __nRF24L01P__

...

```


`sourceID()`
------------
  Le mode Fan-Out utilise le premier octet de charge utile pour identifier la radio à partir de laquelle le message est envoyé. Par conséquent, la taille maximale des données devient 31. Le processus est interne et il est possible d'avoir accès aux informations dont la radio envoie le message en appelant `sourceID()`.
  
  Cette méthode facilite l'utilisation de radios jusqu'à *254*:    
  – L'ID de radio 0 indique que la radio ne sera pas ignorée;    
  – Radio ID 255 indique l'en-tête, sera ignoré.    
  Quantité: 256 - (neutre + en-tête) = *254*.
  
  Pour de nombreuses radios, il existe une utilisation expressive de la mémoire, pour cette raison, une configuration de base de 12 radios a été choisie. Si un nombre plus grand est nécessaire, passez à `acRF24.h`:

```
...

// Number of radios (Change to desired quantity).
#define RADIO_AMOUNT          12

// flag state
...
```

  Remplacez 12 par le montant désiré. Observez la limite de *254*.


`watchTX()`
------------
  Lorsque le récepteur radio tombe pendant une longue période, *ACK* ne retourne pas, provoquant l'inopération de l'émetteur.

  `watchTX()` définit le temps en millisecondes que l'émetteur attendra la réponse *ACK*, alors que l'attente est appelée `reuseTXpayload()`, après cette période `flushTX()` est appelé et libère l'émetteur pour fonctionner avec d'autres radios


`enableFanOut()`
------------
  Appelez `enableFanOut(true)` pour permettre la possibilité de recevoir l'identification de la radio qui envoie le message. Cette activation devrait être commune aux radios qui commenceront.


Fichier d'aide!
------------
  Par manque d'élaboration d'un fichier d'aide, veuillez analyser les exemples de fichiers. Adaptez-vous aux besoins du projet.


Test
------------
  Les tests de développement ont été effectués entre un *Arduino UNO* et un *ATTiny85*.
  
  Initialement, les exemples seront basés sur cette configuration.


CSn delay, et schématique
------------
```  
T_PECSN2ON  = 50 * 0.1;          // <- Capacitance en pF, temps en millisecondes.
        `--> 50Ω x 0.0000001uF   = 0.000005s  ->  5us; temps de conduite.

T_PECSN2OFF = 2200 * 0.1;        // <- Capacitance en pF, temps en millisecondes.
        `--> 2.2kΩ x 0.0000001uF = 0.001s   ->   220us; temps de conduite.
```  
  Remarque:
  * Lorsque vous changez la valeur de la résistance, modifiez également la valeur de la directive `T_PECSN2OFF`.
  Sans cet ajustement, le système peut ne pas fonctionner ou fonctionner avec une faiblesse.
  * La résistance à très faible valeur interfère avec le chargement du code source.
  * La valeur de 1kΩ a été testée et fonctionne bien. Cependant, il est nécessaire de le connecter uniquement après le chargement du code source, dans la réinitialisation de la séquence.
  * Utilisez une diode de germanium qui donne une chute de tension de 0,2 V. Diode de silicium La valeur de tension minimale est de 0.6V pour la puce de 0.3V.
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
  La bibliothèque fournit une méthode automatique pour ajuster l'horloge limite de puce.


Aidez-moi
------------
  En raison du temps limité disponible pour le développement, je présente ce projet de la manière que vous le voyez. Personnellement, je suis désolé mais jusqu'à présent, j'ai pu le faire.
  
  Mon français est faible, dans la mesure du possible, selon le temps disponible, je traduirai.
  
  Les commentaires et les suggestions aideront à améliorer le projet. Bienvenue.


Remerciements
------------
  **Dieu merci.**
  
------------

