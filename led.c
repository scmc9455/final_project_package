//***********************************************************************************************
// Filename: led.c
// Name: Scott McElroy
// Description: This is a driver file for the led strip toggling
// ********Function takes in an RGB value and clocks it out to the LEDs including which***********
// Source Code Used: Snippets of code have been used from Derek Molloys Repo
// Source Code Functions GitRepo: https://github.com/derekmolloy/boneDeviceTree/tree/master/gpio
// Date: November23, 2019
// Used for Final Project of UC Boulder ECEN5013
//***********************************************************************************************

#define DEBUG 1

#include "led.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>

//setup a compiler switch statement for turning print statements on or off
#if DEBUG == 1
#define PDEBUG(fmt,args...) printf(fmt, ## args)
#else
#define PDEBUG(fmt,args...) /*don't print anything*/
#endif

#define LED_CLOCK 		53 //gpio led clock
#define LED_DATA		54 //gpio led clock data

#define CLOCKS_PER_LED 	24
#define BINARY_NUM		8 //bits per color

//keeping track of how many clocking cycles have happened - 24 per LED
int clock_count;

//******************************************
//interrupt handler for signal interrupts
//******************************************
void interruptHandler(int sig){

    uint32_t ret;
    gpio_get_value(LED_CLOCK, &ret);

    if(ret == LOW){
    	gpio_set_value(LED_CLOCK,HIGH);
    }else{
    	gpio_set_value(LED_CLOCK,LOW);
    }

}

//******************************************
// Integer to binary
// binValue address must be 8 positions
//******************************************
int decToBinary(int decValue, int *binValue){

	int i=0;

	if(binValue == NULL){
		return -1;
	}

	if(decValue == 0){
		*binValue = 0;
	}

	//This loop stores the binary data with MSB first
	while(decValue > 0){
		//convert to a modulo 2
		*(binValue+i) = decValue % 2;
		//divide value by 2
		decValue = decValue / 2;
		i++;
	}

	return 0;
}

//*******************************************
//Main function loop
// inputting arguments need to be position and color (RGB)
//*******************************************
int main(int argc, char *argv[]){

	//variables
    int i, ret = 0;
	int led_position;
    timer_t timer_id;
    struct itimerspec itime;
    int *red_binary_num;
    int *green_binary_num;
    int *blue_binary_num;

	//open a log file
    openlog(NULL, 0, LOG_USER);

    PDEBUG("LED Program Running\n");

    //memory allocation space initalization for 24 bits
    red_binary_num = malloc(BINARY_NUM * sizeof(int));
    if(red_binary_num == NULL){
    	PDEBUG("FAIL:red_binary_num = NULL\n");
    	return -1;
    }

    green_binary_num = malloc(BINARY_NUM * sizeof(int));
    if(green_binary_num == NULL){
        PDEBUG("FAIL:green_binary_num = NULL\n");
        return -1;
    }

    blue_binary_num = malloc(BINARY_NUM * sizeof(int));
    if(blue_binary_num == NULL){
        PDEBUG("FAIL:blue_binary_num = NULL\n");
        return -1;
    }

	//check the value of the characters being called
    PDEBUG("argc = %d\n",argc);
    if(argc == 5){
    	//store positional argument
    	//PDEBUG("LED_position input: argv[1] = %c \n", *argv[1]);
    	led_position = atoi(argv[1]);
    	PDEBUG("led_position = %d\n", led_position);
    	//if the position is greater then 17 then it is beyond the strand length
    	if(led_position > 17){
    		//number to large
    		return -1;
    	}

    	//store the actaul value of the rgb
    	//the function also converts string to int
    	//only 8 bit numbers are allowed (255 for each)
    	//********************Red************************
    	//PDEBUG("RED_input: argv[2] = %c \n", *argv[2]);
    	int red = atoi(argv[2]);
    	PDEBUG("red = %d\n", red);
    	if(red > 255){
    		//number to large
    		return -1;
    	}
    	//binary number for red
    	decToBinary(red,red_binary_num);
    	PDEBUG("Red Binary Number = ");
    	for(int t=7; t>=0; t--){
    		PDEBUG("%d",*(red_binary_num+t));
    	}
    	PDEBUG("\n");

    	//*****************Green**********************
    	//PDEBUG("GREEN_input: argv[3] = %c \n", *argv[3]);
    	int green = atoi(argv[3]);
    	PDEBUG("green = %d\n", green);
    	if(green > 255){
    		//number to large
    		return -1;
    	}
    	//binary number for green
    	decToBinary(green,green_binary_num);
    	PDEBUG("Green Binary Number = ");
    	for(int t=7; t>=0; t--){
    		PDEBUG("%d",*(green_binary_num+t));
    	}
    	PDEBUG("\n");

    	//******************Blue*************************
    	//PDEBUG("BLUE_input: argv[4] = %c \n", *argv[4]);
    	int blue = atoi(argv[4]);
    	PDEBUG("blue = %d\n", blue);
    	if(blue > 255){
    		//number to large
    		return -1;
    	}
    	//binary number for blue
    	decToBinary(blue,blue_binary_num);
    	PDEBUG("Blue Binary Number = ");
    	for(int t=7; t>=0; t--){
    		PDEBUG("%d",*(blue_binary_num+t));
    	}
    	PDEBUG("\n");

    	//*******************************
    	// Should now have a binary representation of each color
    	//*******************************

    }else{
    	//failed without input data
    	PDEBUG("Wrong number on input arguments = %d\n", (argc-1));
    	PDEBUG("USAGE 4 inputs only: LED_POSITION RED_NUMBER GREEN_NUMBER BLUE_NUMBER\n");
    	return -1;
    }

    //******************************************************
    //   Setup the timer
    //******************************************************
	//sets the count to be position times clocks per
    clock_count = led_position*CLOCKS_PER_LED;
    PDEBUG("clock_count = %d\n", clock_count);
    	
    PDEBUG("Setting up LED_CLOCK\n");
	//initialize the gpio clocking for the LED
    gpio_export(LED_CLOCK);
	//set the direction of the pin
    gpio_set_dir(LED_CLOCK,OUTPUT_PIN);
    //initialize the gpio

    PDEBUG("Setting up LED_DATA\n");
    //initialize the gpio for data
    gpio_export(LED_DATA);
  	//set the direction of the pin
    gpio_set_dir(LED_DATA,OUTPUT_PIN);

    PDEBUG("Setting up initial pin values\n");
  	//set the initial value of the data
    gpio_set_value(LED_CLOCK,HIGH);
  	//set the initial value of the data
    gpio_set_value(LED_DATA,HIGH);

	//register the interrupt and interrupt handler
    signal(SIGALRM, interruptHandler);

    //create the timer
    PDEBUG("Calling Timer Create\n");
    ret = timer_create(CLOCK_MONOTONIC, NULL, &timer_id);
    if(ret < 0){
    	PDEBUG("Timer Create Fail\n");
    	syslog(LOG_ERR, "Timer Create Error");
    }

	//set so not equal to 1
    itime.it_value.tv_sec = 1;
    itime.it_value.tv_nsec = 0;
	//set for 1 second intervals
    itime.it_interval.tv_sec = 1;
    itime.it_interval.tv_nsec = 0;
	//set the timer and arm it *Timer running*
    PDEBUG("Timer settime called\n");
    ret = timer_settime(timer_id, CLOCK_REALTIME, &itime, NULL);
    if(ret < 0){
    	PDEBUG("Timer SetTime Fail\n");
    	syslog(LOG_ERR, "Timer Settime Error");
    }
    
    uint32_t returnedValue = 0;
    int bluep = 7;
    int greenp = 7;
    int redp = 7;

    //this covers the first led load, the clocking needs to be loaded all the way through the strip
    //using the timer this loop counts 24 clocks based on the timing of the timer interval
    for( i=0; i<clock_count; i++ ){
    		PDEBUG("Inside for loop: i=%d\n",i);
    		//sleep for up to 10 seconds
    		sleep(10);
    		//get the value of the LED clock, if low load a new binary value
    		gpio_get_value(LED_CLOCK, &returnedValue);
    		PDEBUG("returnedValue = %d\n", returnedValue);
    		//**************************//
    			//*******IF CLOCK LOW*******//
    			//ONLY LOAD THE FIRST 24, then this 24 bits moves through the LED strand to the place it need to be//
    			//Blue LSB gets loaded first
    		//if returned value then load a new clock value, the data gets latched by the serial clock on the rising edge
    		if( returnedValue == 0 ){

    			//load the blue data from the blue binary location into the data gpio
    			if( (i < 8) && (bluep >= 0)){
    				PDEBUG("Inside Blue If Statement\n");
    				PDEBUG("bluep = %d\n", bluep);
    				//loads the LSB first up to the MSB
    				int binSet = *(blue_binary_num+bluep);
    				bluep--;
    				//if binValue is a 1 or 0 set function appropriately
    				if(binSet == 1){
    					PDEBUG("GPIO Data Set HIGH\n");
    					gpio_set_value(LED_DATA, HIGH);
    				}
    				if(binSet == 0){
    					PDEBUG("GPIO Data Set LOW\n");
    					gpio_set_value(LED_DATA, LOW);
    				}
    			}//end of blue data load


    			//load the blue data from the blue binary location into the data gpio
    			if( (i < 16) && (greenp >= 0)){
    				PDEBUG("Inside Green If Statement\n");
    				PDEBUG("greenp = %d\n", greenp);
    				//loads the LSB first up to the MSB
    				int binSet = *(green_binary_num+greenp);
			    	greenp--;
    			    //if binValue is a 1 or 0 set function appropriately
    			    if(binSet == 1){
    					PDEBUG("GPIO Data Set HIGH\n");
    			    	gpio_set_value(LED_DATA, HIGH);
    			    }
    			    if(binSet == 0){
    					PDEBUG("GPIO Data Set LOW\n");
    			    	gpio_set_value(LED_DATA, LOW);
    			    }
    			}//end of green data load


    			//load the blue data from the blue binary location into the data gpio
    			if( (i < 25) && (redp >= 0)){
    				PDEBUG("Inside Red If Statement\n");
    				PDEBUG("redp = %d\n", redp);
    				//loads the LSB first up to the MSB
    				int binSet = *(red_binary_num+greenp);
			    	redp--;
    			    //if binValue is a 1 or 0 set function appropriately
    			    if(binSet == 1){
    					PDEBUG("GPIO Data Set HIGH\n");
    			    	gpio_set_value(LED_DATA, HIGH);
    			    }
    			    if(binSet == 0){
    					PDEBUG("GPIO Data Set LOW\n");
    			    	gpio_set_value(LED_DATA, LOW);
    			    }
    			}//end of red data load


    		}//end of returned value check
    }//end of for loop

    //finally set clock low to latch the data into the intended LED in the strand
	PDEBUG("GPIO CLOCK Set LOW for latching data\n");
    gpio_set_value(LED_CLOCK,LOW);

    return 0;
}



//*****************************************************
// Source Code below used from Derek Molloy github repo
// GITHUB REPO: https://github.com/derekmolloy/boneDeviceTree/tree/master/gpio
/****************************************************************
 * gpio_export
 ****************************************************************/
int gpio_export(uint32_t gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);

	return 0;
}

/****************************************************************
 * gpio_unexport
 ****************************************************************/
int gpio_unexport(uint32_t gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
	return 0;
}

/****************************************************************
 * gpio_set_dir
 ****************************************************************/
int gpio_set_dir(uint32_t gpio, PIN_DIRECTION out_flag)
{
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}

	if (out_flag == OUTPUT_PIN)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);

	close(fd);
	return 0;
}

/****************************************************************
 * gpio_set_value
 ****************************************************************/
int gpio_set_value(uint32_t gpio, PIN_VALUE value)
{
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-value");
		return fd;
	}

	if (value==LOW)
		write(fd, "0", 2);
	else
		write(fd, "1", 2);

	close(fd);
	return 0;
}

/****************************************************************
 * gpio_get_value
 ****************************************************************/
int gpio_get_value(uint32_t gpio, uint32_t *value)
{
	int fd;
	char buf[MAX_BUF];
	char ch;

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		perror("gpio/get-value");
		return fd;
	}

	read(fd, &ch, 1);

	if (ch != '0') {
		*value = 1;
	} else {
		*value = 0;
	}

	close(fd);
	return 0;
}

//*******************************************************************
//***************************End of File*****************************
//*******************************************************************
