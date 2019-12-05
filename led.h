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

#include <stdint.h>

//**********************************
//Source Code from Derek Molloy
// GITHUB REPO: https://github.com/derekmolloy/boneDeviceTree/tree/master/gpio
 /****************************************************************
 * Constants
 ****************************************************************/

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

typedef enum{
	INPUT_PIN=0,
	OUTPUT_PIN=1
}PIN_DIRECTION;

typedef enum{
	LOW=0,
	HIGH=1
}PIN_VALUE;

/****************************************************************
 * gpio_export
 ****************************************************************/
int gpio_export(uint32_t gpio);
int gpio_unexport(uint32_t gpio);
int gpio_set_dir(uint32_t gpio, PIN_DIRECTION out_flag);
int gpio_set_value(uint32_t gpio, PIN_VALUE value);
int gpio_get_value(uint32_t gpio, uint32_t *value);
int gpio_set_edge(uint32_t gpio, char *edge);
int gpio_fd_open(uint32_t gpio);
int gpio_fd_close(uint32_t fd);

//**************************************************************
//  Alternate function - author Scott McElroy
//  Functon leaves the file open for gpio
//  Call this function once! Be sure to close the gpio at the end
//**************************************************************
int gpio_set_value_indef(uint32_t gpio, PIN_VALUE value);
void gpio_set(int fd, PIN_VALUE value);
void gpio_close(int fd);

#endif /* LED_H_ */
