/*
 * Copyright (c) 2017 by Acácio Neimar de Oliveira <neimar2009@gmail.com>
 * acRF24diretives.h library.
 */

#if !defined(__SE8R01__) && !defined(__nRF24L01P__)
/************************************************************/
/*           Comment out the unused directive.              */
/*                                                          */
#define __SE8R01__       // <- Comment if you do not use
// #define __nRF24L01P__    // <- Comment if you do not use
/*                                                          */
/************************************************************/
#endif

#if !defined(RADIO_AMOUNT)
/************************************************************/
/*      Number of radios (Change to desired quantity).      */
/*                                                          */
#define RADIO_AMOUNT        24
/*                                                          */
/************************************************************/
#endif

#if !defined(T_PECSN2OFF)
/************************************************************/
/*           Chip deselection time. (see README.md)         */
/*                                                          */
#define T_PECSN2OFF        220
/*   2.2kΩ x 0.0000001uF = 0.00022s -> 220us standby time.  */
/************************************************************/
#endif

/************************************************************/
/*        Diretiva usada em testes de desenvolvimento.      */
/*		                                                    */
// #define __TEST_VARS__
/*		                                                    */
/************************************************************/
