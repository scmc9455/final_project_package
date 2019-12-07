#======================================================================
# Filename: Makefile
# Description: Makefile for compiler to organize compiler files
# Author: Scott McElroy
# Date: November 23, 2019
# Student ID: 105837894
# Course: ECEN5013-002B
# License: Make Script released under GNU GPL (General Purpose License)
#======================================================================

CFLAGS = -Wall -Werror
#all buildable files
SRC = ./led.c ./gpio.c
OBJS = $(SRC:.c=.o)
TARGET:= led

ifeq ($(CC),)
	CC = $(CROSS_COMPILE)gcc
endif

ifeq ($(CCFLAGS),)
	CCFLAGS = -g -Wall #-Werror
endif

ifeq ($(LDFLAGS),)
	LDFLAGS = -lrt
endif

#============Build executable file========================
#============Command "make build"=========================
.PHONY: build
build: all

.PHONY: all 
all: clean server $(TARGET)
$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(OBJS) -o $(TARGET)	

#==========REMOVES executable build and outfiles==========
#==========Command "make clean"===========================
.PHONY: clean
clean:
	rm -f *.o $(OJBS) $(TARGET) server

gpio.o:gpio.c
	$(CC) $(CCFLAGS) $(INCLUDES) gpio.c -c -o gpio.o

led.o:led.c
	$(CC) $(CCFLAGS) $(INCLUDES) led.c -c -o led.o
	
server:server.c
	$(CC) $(CCFLAGS) -Werror $(INCLUDES) server.c -o server

#=====================================================================
#======================End of File====================================
#=====================================================================
