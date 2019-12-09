//=======================================================================
// Filename: server.c
// Description: Program designed for operation of sockets, open, reading,
// writing, closing and will be used to communicate to the led strip
// Arguments: ???
// Author: Scott McElroy
// Date: December 7, 2019
// Student ID: 105837849
// Course: ECEN5013-002B
// Assignment: Final Project
// INPUT format to server = P-## R-### G-### B-###
//========================================================================
//========================================================================

#define _GNU_SOURCE

#define DEBUG 0

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/select.h>
#include <pthread.h>
#include <sys/queue.h>
#include <time.h>

//setup a compiler switch statement for turning print statements on or off
#if DEBUG == 1
#define PDEBUG(fmt,args...) printf(fmt, ## args)
#else
#define PDEBUG(fmt,args...) /*don't print anything*/
#endif

//defines for the socket
#define DOMAIN 	   		PF_INET
#define SOCK_TYPE  		SOCK_STREAM
#define CONNECTION_NODE NULL
#define CONNECTION_PORT	"9000" //can also change file to pass in a port value
#define LISTEN_BACKLOG 	1
#define INPUT_SIZE		22
#define NEW_LINE_CHAR	10
#define BUF_SIZE		23
#define ZERO			0x30
#define ONE				0x31
#define TWO				0x32
#define THREE			0x33
#define FOUR			0x34
#define FIVE			0x35
#define SIX				0x36
#define SEVEN			0x37
#define EIGHT			0x38
#define NINE			0x39
#define SPACE			0x20
#define NULL_TERM		0x0
#define LED_CALL		"./led "
#define LED_CALL_COUNT  7

bool INT_EXIT = true;

//*************************************************
//======================Interrupt Handler==========
//*************************************************
void interruptHandler(int sig){
	//set the interrupt exit to trigger a graceful terminate
	INT_EXIT = false;
}	

//*************************************************
//======================ledDriver function Handler==========
//*************************************************
int ledDriver(char *pos, char *red, char *green, char *blue){
	
	//int ret;
	int count;
	char *buf = malloc(BUF_SIZE * sizeof(char) );	
		
	syslog(LOG_INFO, "Inside led driver function");	
	//fd = open('/bin/led', O_WRONLY);
	
	//**************************************************
	//*******position handling**************************
	//**************************************************
	//checking input values to see if value is too large
	//if the value is greater than 17, than return an error
	if( *pos > ONE ){
		if( *(pos+1) > SEVEN){
			syslog(LOG_ERR, "Position value is too large");
			return -1;
		}	
	}	
	//**************
	if(*pos == ONE){
		syslog(LOG_INFO, "Position value is greater than 10");
		//if the value is 10 or high load these two values into the buf
		*buf = *pos;
		*(buf+1) = *(pos+1);
		//fill the buff with a space
		*(buf+2) = (char)SPACE;
		//increase count to three since three values were loaded into buf
		count = 3;
		
	}else if(*(pos+1) > ZERO){	
		syslog(LOG_INFO, "Position value is less than 10");
		if( *pos == ZERO ){
			//if the first value was zero, than add the second value to the first buf value
			*buf = *(pos+1);
			//fill with a space to separate arguments
			*(buf+1) = (char)SPACE;
			//increase the count number to 2 since this is how many are stored into the buf
			count= 2;	
		}
			
	}else{
		syslog(LOG_ERR, "Position is not greater than 0");
		return -1;
	}//position is greater than zero
		
	//**************************************************
	//************end of position handling**************
	//**************************************************
	
	//**************************************************
	//*******red handling**************************
	//**************************************************
	//checking if the value is larger than 255
	if(*red > TWO){
		syslog(LOG_ERR, "R value is too large");
		return -1;
	}else if(*red == TWO) {
		if(*(red+1) > FIVE){
			syslog(LOG_ERR, "R value is too large");
			return -1;
		}else if(*(red+1) == FIVE){
			if(*(red+2) > FIVE){
				syslog(LOG_ERR, "R value is too large");
				return -1;
			}	
		}	
	}// end of 255 if statement checking

	//check to see of the number is above zero
	if(*(red) > ZERO){
		syslog(LOG_INFO, "Red value is greater than 99");
		//all three numbers need to bew added
		*(buf+count) = *red;
		*(buf+count+1) = *(red+1);
		*(buf+count+2) = *(red+2);
		//add space and 4 to count
		*(buf+count+3) = (char)SPACE;
		count = count + 4;
		//output show now be PP_RRR, where P is position and R is number
	
	}else if(*(red+1) > ZERO){
		syslog(LOG_INFO, "Red value is less than 100");
		//only two numbers need to be added and the first value disregarded
		*(buf+count) = *(red+1);
		*(buf+count+1) = *(red+2);
		//add space and 3 to count
		*(buf+count+2) = (char)SPACE;
		count = count + 3;
		//output show now be PP_RR, where P is position and R is number
	
	}else if( *(red+2) > ZERO){
		syslog(LOG_INFO, "Red value is less than 10");
		//only one number needs to be added and the first two values disregarded
		*(buf+count) = *(red+2);
		//add space and 3 to count
		*(buf+count+1) = (char)SPACE;
		count = count + 2;
    	//output show now be PP_R, where P is position and R is number

	}					
	
	//**************************************************
	//************end of red handling**************
	//**************************************************
	
	//**************************************************
	//*******green handling**************************
	//**************************************************
	//checking if the value is larger than 255
	if(*green > TWO){
		syslog(LOG_ERR, "G value is too large");
		return -1;
	}else if(*green == TWO) {
		if(*(green+1) > FIVE){
			syslog(LOG_ERR, "G value is too large");
			return -1;
		}else if(*(green+1) == FIVE){
			if(*(green+2) > FIVE){
				syslog(LOG_ERR, "G value is too large");
				return -1;
			}	
		}	
	}// end of 255 if statement checking	

	//check to see of the number is above zero
	if(*(green) > ZERO){
		syslog(LOG_INFO, "Green value is greater than 99");
		//all three numbers need to bew added
		*(buf+count) = *green;
		*(buf+count+1) = *(green+1);
		*(buf+count+2) = *(green+2);
		//add space and 4 to count
		*(buf+count+3) = (char)SPACE;
		count = count + 4;
		//output show now be PP_RRR, where P is position and R is number
	
	}else if(*(green+1) > ZERO){
		syslog(LOG_INFO, "Green value is less than 100");
		//only two numbers need to be added and the first value disregarded
		*(buf+count) = *(green+1);
		*(buf+count+1) = *(green+2);
		//add space and 3 to count
		*(buf+count+2) = (char)SPACE;
		count = count + 3;
		//output show now be PP_RR, where P is position and R is number
	
	}else if( *(green+2) > ZERO){
		syslog(LOG_INFO, "Green value is less than 10");
		//only one number needs to be added and the first two values disregarded
		*(buf+count) = *(green+2);
		//add space and 3 to count
		*(buf+count+1) = (char)SPACE;
		count = count + 2;
    	//output show now be PP_R, where P is position and R is number

	}					
	
	//**************************************************
	//************end of green handling**************
	//**************************************************
	
	//**************************************************
	//*******blue handling**************************
	//**************************************************
	//checking if the value is larger than 255
	if(*blue > TWO){
		syslog(LOG_ERR, "B value is too large");
		return -1;
	}else if(*blue == TWO) {
		if(*(blue+1) > FIVE){
			syslog(LOG_ERR, "B value is too large");
			return -1;
		}else if(*(blue+1) == FIVE){
			if(*(blue+2) > FIVE){
				syslog(LOG_ERR, "B value is too large");
				return -1;
			}	
		}	
	}// end of 255 if statement checking	
	
	//check to see of the number is above zero
	if(*(blue) > ZERO){
		syslog(LOG_INFO, "Blue value is greater than 99");
		//all three numbers need to bew added
		*(buf+count) = *blue;
		*(buf+count+1) = *(blue+1);
		*(buf+count+2) = *(blue+2);
		//add 3 to count
		count = count + 3;
		//output show now be PP_RRR, where P is position and R is number
	
	}else if(*(blue+1) > ZERO){
		syslog(LOG_INFO, "blue value is less than 100");
		//only two numbers need to be added and the first value disregarded
		*(buf+count) = *(blue+1);
		*(buf+count+1) = *(blue+2);
		//add 2 to count
		count = count + 2;
		//output show now be PP_RR, where P is position and R is number
	
	}else if( *(blue+2) > ZERO){
		syslog(LOG_INFO, "Blue value is less than 10");
		//only one number needs to be added and the first two values disregarded
		*(buf+count) = *(blue+2);
		//add 1 to count
		count = count + 1;
    	//output show now be PP_R, where P is position and R is number

	}					
	
	//**************************************************
	//************end of blue handling**************
	//**************************************************
	{
		char printString[LED_CALL_COUNT+BUF_SIZE] = LED_CALL;

		printf("String Output before buf addition = %s\n", printString);

		syslog(LOG_INFO, "***Outputting Buffer***");
		for(int i=0; i<count; i=i+1){
			syslog(LOG_INFO, "buf+%d = 0x%x", i, *(buf+i) );
			printString[(LED_CALL_COUNT-1) + i] = *(buf+i);
		}	
	
		printString[(LED_CALL_COUNT-1) + count] = NULL_TERM;
	
		printf("String Output after buffer = %s\n", printString);

		//function to change buf to string
		//need to use string = ./led P R G B
	
		//ret = system("created string");
		//syslog(LOG_INFO, "./led call ret = %d", ret);
	}
	
	free(buf);
	
	return 0;
}//end of ledDriver function	

//*************************************************
//====================Main function================
//*************************************************
int main(int argc, char *argv[]){
	
	//open a syslog connection
	openlog(NULL, 0, LOG_USER);
	
	//register the interruptHandler for signal interruptions
	signal(SIGINT, interruptHandler);
	signal(SIGTERM, interruptHandler);
	
	//=====================================================
	//***********Setup structure for the socket************
	struct addrinfo hints;
	struct addrinfo *result;
	//socket file discriptors
	int sfd, socketfd, ret;
	
	syslog(LOG_INFO, "Starting Socket Program: server");
	
	//zero out memory structure of addrinfo
	memset(&hints, 0, sizeof(struct addrinfo));
	//config sockets feature set
	hints.ai_family = DOMAIN; //network type from above 
	hints.ai_socktype = SOCK_TYPE; // set as socket type from defines
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	
	//***********get the socket ADDRESS************
	//=============================================
	//Getting the socket address information
	syslog(LOG_INFO, "Getting Socket Address");
	ret = getaddrinfo(CONNECTION_NODE,(const char *)CONNECTION_PORT,\
			&hints, &result);
	if(ret != 0 ){
		syslog(LOG_ERR, "getaddrinfo fail Error = %s", strerror(errno));
		return -1;
	}	
	//*******registering SOCKET*********
	//==================================
	//Internet for IPV6 and streamming socket
	//returns the socket file descriptor
	syslog(LOG_INFO, "Registering Socket Address");
	sfd = socket(DOMAIN, SOCK_TYPE, 0);
	if(sfd < 0){
		syslog(LOG_ERR, "socket register fail Error = %s", strerror(errno));
		return -1;
	}
	
	//********BIND the socket************
	//===================================
	syslog(LOG_INFO, "Binding Socket and Port");
	ret = bind(sfd, result->ai_addr, result->ai_addrlen );
	if(ret != 0 ){
		syslog(LOG_ERR, "bind fail Error = %s", strerror(errno));
		return -1;
	}
	
	//========================================
	//********running server as daemon********
	//========================================
/*	
 * {
	//creating and killing child and parent processes
	syslog(LOG_INFO, "Creating server as a daemon");
	***FORK*****
	syslog(LOG_INFO, "forking the daemon process");
	pid_t pid = fork();
	if(pid < 0){
		//fork failed to produce a child process
		syslog(LOG_ERR, "Fork Failed: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(pid > 0){
		//fork created a child successfully
		syslog(LOG_INFO, "Fork created successfully");
		exit(EXIT_SUCCESS);
	}
	
	//tie daemon to a new session and process group
	******SETSID******
	syslog(LOG_INFO, "setting a new session id");
	pid_t sid = setsid();
	if(sid < 0){
		//session id creation failed
		syslog(LOG_ERR, "Session ID creation failed: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}	

	//change the working directory to root
	*******CHDIR to ROOT********
	syslog(LOG_INFO, "changing the directory to root");
	int ret = chdir("/");
	if(ret < 0){
		//directory failed to change to root
		syslog(LOG_ERR, "Directory Filed to change to root: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	//open file descritpor and redirect them to /dev/null
	open("/dev/null", O_RDWR);
	dup(0);
	dup(0);
	* }
*/	
	//=======================================
	//******End of deamon creation***********
	//=======================================
	
	//=======================================
	//*******Listening on port***************
	//listen on the socket and port with 1 allowed connection
	syslog(LOG_INFO, "listening on port");
	ret = listen(sfd, LISTEN_BACKLOG);
	if(ret < 0 ){
		syslog(LOG_ERR, "Listen failer = %s", strerror(errno));
		return -1;
	}

	
	//*********************************************
	//***Need a loop for handling connections******
	//fd_set is a structure for signaling of the pselect function
	fd_set read_socket;
	//FD_ZERO zeros out all the signals
	FD_ZERO( &read_socket );
	//setup sigset for receiving signals for select
	sigset_t emptyset;
	sigemptyset(&emptyset);
	
	//*************************
	//open places to store data???
	//**************************
	
	//listen and connection loop
	while(INT_EXIT){
		
		//=====================================================
		//*********waiting for a connection********************
		//=====================================================
		//FD_SET adds the read_socket file descriptor to the signals to be monitored
		//the sfd is the port trying to make a connection
		FD_SET(sfd, &read_socket);
		syslog(LOG_INFO, "Waiting for connection to accept");
		
		//set up select for socket connection
		//select waits until a connection is trying to be made
		int selret = pselect(FD_SETSIZE, &read_socket, NULL, NULL, NULL, &emptyset);
		FD_ZERO( &read_socket );
		if( selret < 0){
			syslog(LOG_ERR,"Pselect error on connection wait error = %s", strerror(errno) );
			if(errno == EINTR){
				INT_EXIT = 0;
				goto TERMINATE;
			}
		}
	
		//***********socket accepting connection*******************
		//*********SOCKETFD is the accepted connection*************
		//*********************************************************
		socketfd = accept4(sfd, result->ai_addr, &(result->ai_addrlen), 0);
		//if socketfd is a positive value, a new socket can be connected
		if(ret < 0){
			//directory failed to change to root
			syslog(LOG_ERR, "Failed socket fd accept: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}
		
		//***********If socketfd is positive a connection has been made*******
		//checking to see what mode is returned from the connection
		if(socketfd >= 0){
		
			//varaibles for socket returns and reads 
			char socket_data = 0;
			int socket_data_count = 0;
			char *inputData = malloc( INPUT_SIZE*sizeof(char) );
			int readbytes = 0;
			char *buf = &socket_data;
			bool transmit = true;
			//bool wakeup = false;
			//bool connection_lost = false;
			
			////*************************************************//////////
			//**********Printing ipaddress**********************//////////
			{
			syslog(LOG_INFO, "Accepting connection and returning file descriptor");
			
			char ipstr[INET6_ADDRSTRLEN];
			void *addr;
			struct sockaddr_in *ipv4;
	
			//extract and convert IP address for printing
			ipv4 = (struct sockaddr_in *)result->ai_addr; 
			addr = &(ipv4->sin_addr);
			//convert IP addre to string and print to syslog
			inet_ntop(AF_INET, addr, ipstr, INET6_ADDRSTRLEN);
			//print out strings from ip address conversion
			syslog(LOG_INFO, "Accepted connection from %s", ipstr);	
			//***************************//
			}
		
			//=============================================================
			//***********Connected and handling incoming traffic***********
			//=============================================================
			while(transmit == true){

				//variables for waiting for pselect
				//*******variables for pselect******
				fd_set read_fd_set;
				FD_ZERO(&read_fd_set);
				
				//handle the blocking with pselect
				//**********block until data is ready********
				FD_SET(socketfd, &read_fd_set);
				int conret = pselect(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL, &emptyset);	
				if(conret < 0){
					syslog(LOG_ERR, "pSelect Error on trasmit: %s", strerror(errno));
					if(errno == EINTR){
						INT_EXIT = 0;
					}
				}

				//**************read the incoming data from the socket******
				//receiving data from SOCKETFD (accepted connection)********
				// takes in 1 byte and can be read from socket_data variable
				readbytes = recv(socketfd, buf, 1, MSG_DONTWAIT);
				syslog(LOG_INFO, "recv - readbytes = %d", readbytes);
				
				if(readbytes > 0){
					//*********handle later*******
					syslog(LOG_INFO, "data received = 0x%x", *buf);
					syslog(LOG_INFO, "data received (socket_data) = 0x%x", socket_data);
					
					//add one to the pointer for the buffer
					*(inputData + socket_data_count ) = socket_data; 
					syslog(LOG_INFO, "*(inputData+socket_data_count) = 0x%x", *(inputData+socket_data_count) );
					socket_data_count = socket_data_count + 1;
						
					if(*buf == NEW_LINE_CHAR){
						syslog(LOG_INFO, "NEW_LINE received - Sending back to caller");
						syslog(LOG_INFO, "socket_data_count = %d", socket_data_count );
						send(socketfd, inputData, socket_data_count, 0);
						transmit = false;
					}
					
				}else  if((readbytes < 0) && (errno != EAGAIN) ){
					//********handle later*************
					transmit = false;
					break;
				}else if(readbytes == 0){
					syslog(LOG_INFO, "No data is received");
					//resend the data back to the server
					transmit = false;
				}else{
					break;		
				}
				
			}//end of connection_lost while loop
			
			//if no more data and connection closed, close socketfd
			if(transmit == false){
				
				char *pos = malloc(2 * sizeof(char) );
				char *red = malloc(3 * sizeof(char) );
				char *green = malloc(3 * sizeof(char));
				char *blue = malloc(3 * sizeof(char) );
				
				close(socketfd);
				
				//input data format should be P-## R-### G-### B-###
				if(socket_data_count == 23){
					
					for(int i=0; i<22; i=i+1){
						syslog(LOG_INFO, "i=%d *(inputData+i) = 0x%x", i, *(inputData+i) );
					}
					
					//gathering pos data
					syslog(LOG_INFO, "*inputData = 0x%x", *inputData);
					if( (*inputData == 'P') ){
						*pos = *(inputData+2);
						*(pos+1) = *(inputData+3);
						syslog(LOG_INFO, "position data = 0x%x 0x%x", *pos, *(pos+1) );
					}
					
					//gathering red data
					syslog(LOG_INFO, "*(inputData+5) = 0x%x", *(inputData+5) );
					if( (*(inputData+5) == 'R') ){
						*red = *(inputData+7);
						*(red+1) = *(inputData+8);
						*(red+2) = *(inputData+9);
						syslog(LOG_INFO, "red data = 0x%x 0x%x 0x%x", *red, *(red+1), *(red+2));
					}
										
					//gathering green data 
					syslog(LOG_INFO, "*(inputData+11) = 0x%x", *(inputData+11) );
					if( (*(inputData+11) == 'G') ){
						*green = *(inputData+13);
						*(green+1) = *(inputData+14);
						*(green+2) = *(inputData+15);
						syslog(LOG_INFO, "green data = 0x%x 0x%x 0x%x", *green, *(green+1), *(green+2));
					}
					
					
					//gathering green data 
					syslog(LOG_INFO, "*(inputData+17) = 0x%x", *(inputData+17) );
					if( (*(inputData+17) == 'B') ){
						*blue = *(inputData+19);
						*(blue+1) = *(inputData+20);
						*(blue+2) = *(inputData+21);
						syslog(LOG_INFO, "blue data = 0x%x 0x%x 0x%x", *blue, *(blue+1), *(blue+2));
					}
							
					free(inputData);
									
					//calling the led driver
					syslog(LOG_INFO, "Calling ledDriver");
					ledDriver(pos, red, green, blue);
					socket_data_count = 0;

									
				}//end of socket data extraction
				
				
				free(red);
				free(green);
				free(blue);
			}
			
		}else if((socketfd < 0) && (errno != EAGAIN)){
			syslog(LOG_ERR, "Accept Connection Error: %s", strerror(errno));
			return -1;
		}
	
TERMINATE:
		if(!INT_EXIT){
			syslog(LOG_INFO, "INT_EXIT triggered");
			return -1;
		}	
			
	} //end of INT_EXIT while loop
		
}//end of main	
