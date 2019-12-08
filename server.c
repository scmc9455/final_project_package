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
// INPUT format to server = R-### G-### B-###
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
#define INPUT_SIZE		17
#define NEW_LINE_CHAR	10

bool INT_EXIT = true;

//*************************************************
//======================Interrupt Handler==========
//*************************************************
void interruptHandler(int sig){
	//set the interrupt exit to trigger a graceful terminate
	INT_EXIT = false;
}	

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
					syslog(LOG_INFO, "data received = %d", *buf);
					
					//add one to the pointer for the buffer
					socket_data_count = socket_data_count + 1;
					*(inputData + socket_data_count ) = socket_data; 
						
					if(*buf == NEW_LINE_CHAR){
						syslog(LOG_INFO, "NEW_LINE received - Sending back to caller");
						send(socketfd, inputData, socket_data_count, 0);
						transmit = false;
					}
					
				}else  if((readbytes < 0) && (errno != EAGAIN) ){
					//********handle later*************
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
				
				int *red = malloc(3 * sizeof(char) );
				int *green = malloc(3 * sizeof(char));
				int *blue = malloc(3 * sizeof(char) );
				
				close(socketfd);
				free(inputData);
				
				//input data format should be R-### G-### B-###
				if(socket_data_count == 17){
					//gathering red data
					if( (*inputData == 'R') ){
						*red = *(inputData+2);
						*(red+1) = *(inputData+3);
						*(red+2) = *(inputData+4);
					}
					syslog(LOG_INFO, "red data = 0x%x 0x%x 0x%x", *red, *(red+1), *(red+2));
					//gathering green data 
					if( (*(inputData+6) == 'G') ){
						*green = *(inputData+8);
						*(green+1) = *(inputData+9);
						*(green+2) = *(inputData+10);
					}
					syslog(LOG_INFO, "green data = 0x%x 0x%x 0x%x", *green, *(green+1), *(green+2));
					//gathering green data 
					if( (*(inputData+12) == 'B') ){
						*blue = *(inputData+14);
						*(blue+1) = *(inputData+15);
						*(blue+2) = *(inputData+16);
					}
					syslog(LOG_INFO, "blue data = 0x%x 0x%x 0x%x", *blue, *(blue+1), *(blue+2));
									
				}//end of socket data extraction
				
				//calling the led driver
				//ledDriver();
				
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
