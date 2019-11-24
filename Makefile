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
SRC = ./led.c
OBJS = ./led

ifeq ($(CC),)
	CC = $(CROSS_COMPILE)gcc
endif

ifeq ($(CCFLAGS),)
	CCFLAGS = -g -Wall -Werror
endif

ifeq ($(LDFLAGS),)
	LDFLAGS = -pthread -lrt
endif

#============Build executable file========================
#============Command "make build"=========================
.PHONY: build
build: all

.PHONY: all
all: clean $(OBJS)


TARGET:
	$(CC) $(CCFLAGS) $(INCLUDES) $@.o -o $@ $(LDFLAGS)

#==========REMOVES executable build and outfiles==========
#==========Command "make clean"===========================
.PHONY: clean
clean:
	rm -f *.o $(OJBS)


#===================builds==============================
led:led.c
	$(CC) $(CCFLAGS) $(INCLUDES) led.c -o led -lrt


#=====================================================================
#======================End of File====================================
#=====================================================================
