//***********************************************************************************************
// Filename: gpio.c
// Name: Scott McElroy
// Description: This is a driver file for the led strip toggling
// ********Function takes in an RGB value and clocks it out to the LEDs including which***********
// Source Code Used: Snippets of code have been used from VegetableAvenger Repo
// Source Code Functions GitRepo: https://github.com/VegetableAvenger/BBBIOlib
// Date: December 5, 2019
// Used for Final Project of UC Boulder ECEN5013
//***********************************************************************************************


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include "gpio.h"

//************************************************
//******From VegetableAvenger*********************
/* GPIO Port number set of Beaglebone Black P8 .
 * -1 as GND or VCC , Number 0/1/2 as GPIO 0/1/2
 */
const signed char p8_PortSet[] = {-1, -1, 1, 1, 1, 1, 2, 2,
				2, 2, 1, 1, 0, 0, 1, 1, 
				0, 2, 0, 1, 1, 1, 1, 1, 
				1, 1, 2, 2, 2, 2, 0, 0, 
				0, 2, 0, 2, 2, 2, 2, 2, 
				2, 2, 2, 2, 2, 2};

/* GPIO Port ID set of Beaglebone Black P8  ,
 * 0 as GND , offset X as GPIO ID m this value must combine with GPIO number
 */
const unsigned int p8_PortIDSet[] = {0,	0,	1<<6,	1<<7,	1<<2,	1<<3,	1<<2,	1<<3,	
				1<<5,	1<<4,	1<<13,	1<<12,	1<<23,	1<<26,	1<<15,	
				1<<14,	1<<27,	1<<1,	1<<22,	1<<31,	1<<30,	1<<5,	
				1<<4,	1<<1,	1<<0,	1<<29,	1<<22,	1<<24,	1<<23,	
				1<<25,	1<<10,	1<<11,	1<<9,	1<<17,	1<<8,	1<<16,	
				1<<14,	1<<15,	1<<12,	1<<13,	1<<10,	1<<11,	1<<8,	
				1<<9,	1<<6,	1<<7};

/* GPIO Port number set of Beaglebone Black P9  ,
 * -1 as GND or VCC , 0 / 1 / 2 for GPIO 0/1/2
 */
const signed char p9_PortSet[] = {-1, -1, -1, -1, -1, -1, -1,
				-1, -1, -1, 0, 1, 0, 1, 1, 
				1, 0, 0, 0, 0, 0, 0, 1, 0, 
				3, 0, 3, 3, 3, 3, 3, -1, -1, 
				-1, -1, -1, -1, -1, -1, -1, 
				0, 0, -1, -1, -1, -1};

/*  GPIO Port ID set of Beaglebone Black P9  ,
 * 0 as GND , offset X as GPIO ID m this value must combine with GPIO number
 */
const unsigned int p9_PortIDSet[]={0,	0,	0,	0,	0,	0,	0,	0,	
				0,	0,	1<<30,	1<<28,	1<<31,	1<<18,	1<<16,	1<<19,	
				1<<5,	1<<4,	1<<13,	1<<12,	1<<3,	1<<2,	1<<17,	
				1<<15,	1<<21,	1<<14,	1<<19,	1<<17,	1<<15,	1<<16,	
				1<<14,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1<<20,	
				1<<7,	0,	0,	0,	0};

/* Memory mapping offset if GPIO , means the memory address of GPIOs */
const unsigned int GPIO_AddressOffset[]={BBBIO_GPIO0_ADDR, BBBIO_GPIO1_ADDR, BBBIO_GPIO2_ADDR, BBBIO_GPIO3_ADDR};

/* pointer to const Port set and Port ID set array */
signed char* PortSet_ptr[2];
unsigned int* PortIDSet_ptr[2];
int mem_fd=0;
volatile unsigned int *gpio_addr[4] = {NULL, NULL, NULL, NULL};
volatile unsigned int *cm_per_addr = NULL;
volatile unsigned int *cm_wkup_addr = NULL;
volatile unsigned int *CM_ptr = NULL;

//**********************************************************
//*********Functions*******************************
//**********************************************************

//*********************************
//* Library Init
//* ********************************

int iolib_init(void)
{
	int i;

	PortSet_ptr[0]=(signed char*)p8_PortSet;
	PortSet_ptr[1]=(signed char*)p9_PortSet;
	PortIDSet_ptr[0]=(unsigned int*)p8_PortIDSet;
	PortIDSet_ptr[1]=(unsigned int*)p9_PortIDSet;

	if(mem_fd){
	    syslog(LOG_ERR, "Memory already mapped");
	    return -1;
	}

	/* using memory mapped I/O */
	mem_fd=open("/dev/mem", O_RDWR);

	/* mapping Clock Module Peripheral Registers */
	cm_per_addr = mmap(0, BBBIO_CM_PER_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, BBBIO_CM_PER_ADDR);
	if(cm_per_addr == MAP_FAILED)
    	{
	    syslog(LOG_INFO, "iolib_init: CM_PER mmap failure! ,error :%s" ,strerror(errno));
	    return -1;
	}

	/* mapping Clock Module Wakeup Registers
	 *
	 *  Note : the reason why not using mmap to mapping cm_wkup direct is page boundar .
	 * 	   if using mmap directlly (like CM_PER ), it will cause an error about EINVAL (invalid argument) .
	 *	   because the address of CM_WKUP is 0x44E00400 , it doesn't align the page (4K).
	 */
	cm_wkup_addr =(void *)cm_per_addr + BBBIO_CM_WKUP_OFFSET_FROM_CM_PER;


	/* mapping Address of GPIO 0~4 */
	for (i=0; i<4; i++) {
		gpio_addr[i] = mmap(0 ,BBBIO_GPIOX_LEN ,PROT_READ | PROT_WRITE ,MAP_SHARED ,mem_fd ,GPIO_AddressOffset[i]);
		if(gpio_addr[i] == MAP_FAILED) {
			syslog(LOG_INFO, "iolib_init: gpio mmap failure!");
			return -1;
		}
	}

	/* mapping Control Module Registers
	 * for pin mux control , or display expansion header informaiton
	 *
	 * Useless now , this register must be privigle mode .
	 */
	CM_ptr = mmap(0, BBBIO_CONTROL_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, BBBIO_CONTROL_MODULE);
	if(CM_ptr == MAP_FAILED) {
		syslog(LOG_INFO, "iolib_init: control module mmap failure!");
		return -1;
	}

    return 0;
}

//* ********************************
//* Library free
//* ********************************
int iolib_free(void)
{
	if (mem_fd!=0) {
		close(mem_fd);
	}
	return 0;
}

//* ********************************
//* Set I/O direction (Input/Output)
//* ********************************

int iolib_setdir(char port, char pin, char dir)
{
	int param_error=0;			// parameter error
	volatile unsigned int* reg;		// GPIO register

	// sanity checks
	if (mem_fd == 0)
		param_error=1;
	if ((port < 8) || (port > 9))		// if input is not port8 and port 9 , because BBB support P8/P9 Connector
		param_error=1;
	if ((pin < 1 ) || (pin > 46))		// if pin over/underflow , range : 1~46
		param_error=1;
	if (PortSet_ptr[port - 8][pin - 1] < 0)	// pass GND OR VCC (PortSet as -1)
		param_error=1;

	if (param_error)
	{
		syslog(LOG_ERR, "iolib_setdir: parameter error!");
		return -1;
	}

	syslog(LOG_INFO, "iolib_setdir: PortSet_ptr P%d.%d , %X",port ,pin , PortSet_ptr[port-8][pin-1]);

	reg=(void*)gpio_addr[PortSet_ptr[port-8][pin-1]] +BBBIO_GPIO_OE;

	if (dir == BBBIO_DIR_OUT) {
		*reg &= ~(PortIDSet_ptr[port-8][pin-1]);
	}
	else if (dir == BBBIO_DIR_IN) {
		*reg |= PortIDSet_ptr[port-8][pin-1];
	}

	return 0;
}

//* ********************************
//* Set and get high or low (Input/Output)
//* ********************************

void pin_high(char port, char pin)
{
	*((unsigned int *)((void *)gpio_addr[PortSet_ptr[port-8][pin-1]]+BBBIO_GPIO_SETDATAOUT)) = PortIDSet_ptr[port-8][pin-1];
}

void pin_low(char port, char pin)
{
	*((unsigned int *)((void *)gpio_addr[PortSet_ptr[port-8][pin-1]]+BBBIO_GPIO_CLEARDATAOUT)) = PortIDSet_ptr[port-8][pin-1];
}

char is_high(char port, char pin)
{
	return ((*((unsigned int *)((void *)gpio_addr[PortSet_ptr[port-8][pin-1]]+BBBIO_GPIO_DATAIN)) & PortIDSet_ptr[port-8][pin-1])!=0);
}

char is_low(char port, char pin)
{
	return ((*((unsigned int *)((void *)gpio_addr[PortSet_ptr[port-8][pin-1]]+BBBIO_GPIO_DATAIN)) & PortIDSet_ptr[port-8][pin-1])==0);
}


//************************************************************************************
//*****************END OF FILE********************************************************
//************************************************************************************
