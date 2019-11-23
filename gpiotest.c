#include "gpio.h"

void main(void){

	unsigned int pin = 53;
	gpio_export(pin);
	gpio_set_dir(pin,OUT);
	gpio_set_value(pin,1);

	int temp = 1;

	while(1){
		if(temp == 0){
			temp = 1;
		}else if(temp == 1){
			temp = 0;
		}	
	
		for(int i=0; i<10000; i++){
			gpio_set_value(pin,temp);
		}	

	}	

}
