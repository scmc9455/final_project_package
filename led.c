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

#define DEBUG 0

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
#include <stdbool.h>
#include <sys/time.h>
#include "gpio.h"

//setup a compiler switch statement for turning print statements on or off
#if DEBUG == 1
#define PDEBUG(fmt,args...) printf(fmt, ## args)
#else
#define PDEBUG(fmt,args...) /*don't print anything*/
#endif

//LED GPIO is port x 32 + pin number
#define LED_CLOCK 		48 //gpio led clock
#define LED_DATA		60 //gpio led clock data
//defines for iolib
#define LED_CLOCK_PORT	9
#define LED_DATA_PORT	9
#define LED_CLOCK_PIN	12
#define LED_DATA_PIN	15

#define CLOCKS_PER_LED 	24
#define BINARY_NUM		8 //bits per color

#define CLOCKTIME 		100000  //nanoseconds settings for the timer
//keeping track of how many clocking cycles have happened - 24 per LED
int clock_count, data_position;
int clock_polarity = 0;

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
    int i;
	int led_position;
	struct timespec endTime;
    int *red_binary_num;
    int *green_binary_num;
    int *blue_binary_num;
    
    //****Functions from gpio.h****
    //initialize the iolib
    iolib_init();
    iolib_setdir(LED_CLOCK_PORT, LED_CLOCK_PIN, BBBIO_DIR_OUT);
    iolib_setdir(LED_DATA_PORT, LED_DATA_PIN, BBBIO_DIR_OUT);
    //***********

	//open a log file
    openlog(NULL, 0, LOG_USER);

    PDEBUG("LED Program Running\n");

    //memory allocation space initalization for 24 bits
    red_binary_num = malloc(BINARY_NUM * sizeof(int));
    if(red_binary_num == NULL){
    	PDEBUG("FAIL:red_binary_num = NULL\n");
    	syslog(LOG_ERR,"FAIL: red_binary_num malloc error");
    	return -1;
    }

    green_binary_num = malloc(BINARY_NUM * sizeof(int));
    if(green_binary_num == NULL){
        PDEBUG("FAIL:green_binary_num = NULL\n");
        syslog(LOG_ERR,"FAIL: green_binary_num malloc error");
        return -1;
    }

    blue_binary_num = malloc(BINARY_NUM * sizeof(int));
    if(blue_binary_num == NULL){
        PDEBUG("FAIL:blue_binary_num = NULL\n");
        syslog(LOG_ERR,"FAIL: blue_binary_num malloc error");
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
    		syslog(LOG_ERR,"FAIL: led strip position number to large");
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
    	syslog(LOG_INFO, "red = %d\n", red);
    	if(red > 255){
    		syslog(LOG_ERR,"FAIL: red number to large");
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
    	syslog(LOG_INFO, "green = %d\n", green);
    	if(green > 255){
    		syslog(LOG_ERR,"FAIL: green number to large");
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
    	syslog(LOG_INFO, "blue = %d\n", blue);
    	if(blue > 255){
    		syslog(LOG_ERR,"FAIL: blue number to large");
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
    	syslog(LOG_ERR,"USAGE ERROR: not enough arguments passed");
    	return -1;

    }//end of argc statement

    //******************************************************
    //   Setup the timer
    //******************************************************
	//sets the count to be position times clocks per
    clock_count = led_position*CLOCKS_PER_LED;
    // Data_position will help load the value of the data down the line
    // if position is 1 data_position is 0
    // if position is 2 data_position is 24
    data_position = CLOCKS_PER_LED * (led_position-1);
    PDEBUG("clock_count = %d\n", clock_count);

    PDEBUG("Setting up initial pin values for clock and data\n");
  	//set the initial value of the data
    //****************
    pin_low(LED_CLOCK_PORT, LED_CLOCK_PIN);
    pin_low(LED_DATA_PORT, LED_DATA_PIN);
    //****************
    
    //endTime for first bit load for 500us
    endTime.tv_sec = 0;
    endTime.tv_nsec = 500000;    
    
    if( nanosleep(&endTime, NULL)<0 ){
       	PDEBUG("Nanosleep failed on start of clocking");
       	syslog(LOG_ERR, "Nanosleep Fail on start of clocking - ERRNO = %s", strerror(errno) );
    }
    
    int bluep = 7;
    int greenp = 7;
    int redp = 7;
    syslog(LOG_INFO, "Reached pointer after the bluep, greenp, and redp");

    //this covers the first led load, the clocking needs to be loaded all the way through the strip
    //using the timer this loop counts 24 clocks based on the timing of the timer interval
    for( i=0; i<(clock_count+1); i++ ){
    		PDEBUG("Inside for loop: i=%d\n",i);
    		//syslog(LOG_INFO,"inside FOR loop: i=%d",i);
    		    		
    		//setting the clock value
   	    	pin_low(LED_CLOCK_PORT, LED_CLOCK_PIN);

    		PDEBUG("LED_CLOCK LOW");
   	    	//syslog(LOG_INFO, "Setting LED_CLOCK LOW");
    		//**************************//
    			//*******IF CLOCK LOW*******//
    			//ONLY LOAD THE FIRST 24, then this 24 bits moves through the LED strand to the place it need to be//
    			//Blue LSB gets loaded first

    		PDEBUG("GPIO clock_polarity is 0\n");
    		//syslog(LOG_INFO, "GPIO clock_polartiy is 0");
    		//load the blue data from the blue binary location into the data gpio
    		if( ((i<(8+data_position)) && (i>data_position)) && (redp >= 0)){
    			PDEBUG("Inside Red If Statement\n");
    			//syslog(LOG_INFO, "inside Red statement: redp=%d", redp);
    			PDEBUG("redp = %d\n", redp);
    			//loads the LSB first up to the MSB
    			int binSet = *(red_binary_num+redp);
    			PDEBUG("binSet = %d\n", binSet);
    			//syslog(LOG_INFO, "binSet = %d\n", binSet);
    			redp--;
    			//if binValue is a 1 or 0 set function appropriately
    			if(binSet == 1){
    				PDEBUG("GPIO Data Set HIGH\n");
    				//syslog(LOG_INFO, "GPIO_DATA Set HIGH");
    				pin_high(LED_DATA_PORT, LED_DATA_PIN);
    			}
    			if(binSet == 0){
    				PDEBUG("GPIO Data Set LOW\n");
    				//syslog(LOG_INFO, "GPIO_DATA Set LOW");
    				pin_low(LED_DATA_PORT, LED_DATA_PIN);
    			}
    		}//end of blue data load


    		//load the blue data from the blue binary location into the data gpio
    		if( ((i>(7+data_position)) && (i<(16+data_position))) && (greenp >= 0)){
    			PDEBUG("Inside Green If Statement\n");
    			//syslog(LOG_INFO, "inside Green statement: greenp=%d", greenp);
    			PDEBUG("greenp = %d\n", greenp);
    			//loads the LSB first up to the MSB
    			int binSet = *(green_binary_num+greenp);
    			PDEBUG("binSet = %d\n", binSet);
    			//syslog(LOG_INFO, "binSet = %d\n", binSet);
    			greenp--;
    		    //if binValue is a 1 or 0 set function appropriately
    		    if(binSet == 1){
    				PDEBUG("GPIO Data Set HIGH\n");
    				//syslog(LOG_INFO, "GPIO_DATA Set HIGH");
    		    	pin_high(LED_DATA_PORT, LED_DATA_PIN);
    		    	//gpio_set(data_fd, HIGH);
    		    }
    		    if(binSet == 0){
    				PDEBUG("GPIO Data Set LOW\n");
    				//syslog(LOG_INFO, "GPIO_DATA Set LOW");
    		    	pin_low(LED_DATA_PORT, LED_DATA_PIN);
    		    	//gpio_set(data_fd, LOW);
    		    }
    		}//end of green data load


    		//load the blue data from the blue binary location into the data gpio
    		if( ((i>(15+data_position)) && (i<(25+data_position))) && (bluep >= 0)){
    			PDEBUG("Inside Red If Statement\n");
    			//syslog(LOG_INFO, "inside Blue statement: Redp=%d", bluep);
    			PDEBUG("bluep = %d\n", bluep);
    			//loads the LSB first up to the MSB
    			int binSet = *(blue_binary_num+bluep);
    			PDEBUG("binSet = %d\n", binSet);
    			//syslog(LOG_INFO, "binSet = %d\n", binSet);
    			bluep--;
    		    //if binValue is a 1 or 0 set function appropriately
    		    if(binSet == 1){
    				PDEBUG("GPIO Data Set HIGH\n");
        			//syslog(LOG_INFO, "GPIO Data set to HIGH");
    		    	pin_high(LED_DATA_PORT, LED_DATA_PIN);
    		    	//gpio_set(data_fd, HIGH);
    		    }
    		    if(binSet == 0){
    				PDEBUG("GPIO Data Set LOW\n");
        			//syslog(LOG_INFO, "GPIO Data set to LOW");
					pin_low(LED_DATA_PORT, LED_DATA_PIN);
    				//gpio_set(data_fd, LOW);
    		    }
    		}//end of red data load
    		
    		//setting the clock value after the data is loaded
    		if(i < (clock_count)){
				pin_high(LED_CLOCK_PORT, LED_CLOCK_PIN);
				PDEBUG("LED_CLOCK HIGH");
			}else{
				pin_low(LED_CLOCK_PORT, LED_CLOCK_PIN);
			}		
   	    	//syslog(LOG_INFO, "Setting LED_CLOCK HIGH");
    		//***********
    }//end of for loop

    //finally set clock low to latch the data into the intended LED in the strand
	PDEBUG("GPIO CLOCK Set LOW for latching data\n");
	//syslog(LOG_INFO, "GPIO clock and data set to low");
	pin_low(LED_CLOCK_PORT, LED_CLOCK_PIN);
    pin_low(LED_DATA_PORT, LED_DATA_PIN);
	
    //The below needs to be done to make the program reentrant
    //free the pointers
    free(red_binary_num);
    free(green_binary_num);
    free(blue_binary_num);
    //release the GPIO
    
    iolib_free();

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

//**************************************************************
//  Alternate functions - author Scott McElroy
//  Functon leaves the file open for gpio
//  Call this function once! Be sure to close the gpio at the end
//**************************************************************
int gpio_set_value_indef(uint32_t gpio, PIN_VALUE value){
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-value");
		return fd;
	}

	if (value==LOW){
		write(fd, "0", 2);
	}else{
		write(fd, "1", 2);
	}
	
	return fd;
}
//******************************
void gpio_set(int fd, PIN_VALUE value){
	
	if (value==LOW){
		write(fd, "0", 2);
	}else{
		write(fd, "1", 2);
	}	
}
//*****************************
void gpio_close(int fd){
	
	close(fd);
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
