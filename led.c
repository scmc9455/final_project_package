//***********************************************************************************************
// Filename: led.c
// Name: Scott McElroy
// Description: This is a driver file for the led strip toggling
// Source Code Used: Snippets of code have been used from Derek Molloys Repo
// Source Code Functions GitRepo: https://github.com/derekmolloy/boneDeviceTree/tree/master/gpio
// Date: November23, 2019
// Used for Final Project of UC Boulder ECEN5013
//***********************************************************************************************


#include "led.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>

#define GPIO_NUM 53

void interruptHandler(int sig){

    uint32_t ret;
    gpio_get_value(GPIO_NUM, &ret);

    if(ret == LOW){
	gpio_set_value(GPIO_NUM,HIGH);
    }else{
	gpio_set_value(GPIO_NUM,LOW);
    }
}

int main(void){

    timer_t timer_id;
    struct itimerspec itime;

	//initialize the gpio
    gpio_export(GPIO_NUM);
	//set the direction of the pin
    gpio_set_dir(GPIO_NUM,OUTPUT_PIN);
    	//set the initial value
    gpio_set_value(GPIO_NUM,HIGH);

/*    while(1){

	uint32_t ret;
	gpio_get_value(GPIO_NUM, &ret);
	
        if(ret == LOW){
	    gpio_set_value(GPIO_NUM,HIGH);
    	}else{
	    gpio_set_value(GPIO_NUM,LOW);
    	}

	for(int i=0; i<100000000; i++){
	}//end of for loop
    
    }//end of while loop
*/
	//register the interrupt
    signal(SIGALRM, interruptHandler);
	//create the timer
    timer_create(CLOCK_MONOTONIC, NULL, &timer_id);

	//set so not equal to 1
    itime.it_value.tv_sec = 1;
    itime.it_value.tv_nsec = 0;
	//set for 1 second intervals
    itime.it_interval.tv_sec = 1;
    itime.it_interval.tv_nsec = 0;
	//set the timer and arm it *Timer running*
    timer_settime(timer_id, CLOCK_REALTIME, &itime, NULL);
    
    while(1);

//check return values

    return 0;
}



//**********************************
//Source Code from Derek Molloy
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
