//***********************************************************************************************
// Filename: led.h
// Name: Scott McElroy
// Description: This is a driver file for the led strip toggling
// Source Code Used: Snippets of code have been used from Derek Molloys Repo
// Source Code Functions GitRepo: https://github.com/derekmolloy/boneDeviceTree/tree/master/gpio
// Date: November23, 2019
// Used for Final Project of UC Boulder ECEN5013
//***********************************************************************************************

#ifndef LED_H_
#define LED_H_

//Timer function
int interruptHandler(int sig);

//**********************************
//Source Code from Derek Molloy
// GITHUB REPO: https://github.com/derekmolloy/boneDeviceTree/tree/master/gpio
 /****************************************************************
 * Constants
 ****************************************************************/

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

enum PIN_DIRECTION{
	INPUT_PIN=0,
	OUTPUT_PIN=1
};

enum PIN_VALUE{
	LOW=0,
	HIGH=1
};

/****************************************************************
 * gpio_export
 ****************************************************************/
int gpio_export(unsigned int gpio);
int gpio_unexport(unsigned int gpio);
int gpio_set_dir(unsigned int gpio, PIN_DIRECTION out_flag);
int gpio_set_value(unsigned int gpio, PIN_VALUE value);
int gpio_get_value(unsigned int gpio, unsigned int *value);
int gpio_set_edge(unsigned int gpio, char *edge);
int gpio_fd_open(unsigned int gpio);
int gpio_fd_close(int fd);

#endif /* LED_H_ */
