//***********************************************************************************************
// Filename: gpio.h
// Name: Scott McElroy
// Description: This is a driver file for the led strip toggling
// ********Function takes in an RGB value and clocks it out to the LEDs including which***********
// Source Code Used: Snippets of code have been used from VegetableAvenger Repo
// Source Code Functions GitRepo: https://github.com/VegetableAvenger/BBBIOlib
// Date: December 5, 2019
// Used for Final Project of UC Boulder ECEN5013
//***********************************************************************************************

#ifndef _GPIO_H_
#define _GPIO_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>

#define BBBIO_CM_PER_LEN	0x4000
#define BBBIO_CM_PER_ADDR 	0x44e00000
#define BBBIO_CM_WKUP_OFFSET_FROM_CM_PER	0x400
#define BBBIO_GPIO0_ADDR	0x44e07000
#define BBBIO_GPIO1_ADDR	0x4804c000
#define BBBIO_GPIO2_ADDR	0x481AC000
#define BBBIO_GPIO3_ADDR	0x481AE000
#define BBBIO_GPIOX_LEN		0x1000
#define BBBIO_CONTROL_LEN	0x2000
#define BBBIO_CONTROL_MODULE	0x44e10000
#define BBBIO_GPIO_OE		0x134
#define BBBIO_GPIO_SETDATAOUT	0x194
#define BBBIO_GPIO_CLEARDATAOUT	0x190
#define BBBIO_GPIO_DATAIN	0x138

#define BBBIO_DIR_OUT	0
#define BBBIO_DIR_IN 	1

//*********************************
//* Library Init
//* ********************************
int iolib_init(void);

//* ********************************
//* Library free
//* ********************************
int iolib_free(void);

//* ********************************
//* Set I/O direction (Input/Output)
//* ********************************

int iolib_setdir(char port, char pin, char dir);

//* ********************************
//* Set and get high or low (Input/Output)
//* ********************************

void pin_high(char port, char pin);

//* ********************************
//* Set and get high or low (Input/Output)
//* ********************************

void pin_high(char port, char pin);
void pin_low(char port, char pin);
char is_high(char port, char pin);
char is_low(char port, char pin);

#endif //_GPIO_H_








