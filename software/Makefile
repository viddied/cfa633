#Makefile for Crystalfontz UNIX examples

CC = gcc
LIBS = 
CFLAGS = -g -O -Iinclude -Wall -Wextra -Werror
LDFLAGS = -g

LIBSRC = include/serial.c include/cf_packet.c include/show_packet.c
LIBOBJ = $(LIBSRC:%.c=%.o)

all: menu readmemory readflash writeflash

menu: source/cfa633_menu.o source/menustructure.o $(LIBOBJ)
	$(CC) $(LDFLAGS) $(LIBOBJ) source/cfa633_menu.o source/menustructure.o -o cfa633_menu

readmemory: 633/cfa633_readmemory.o $(LIBOBJ)
	$(CC) $(LDFLAGS) $(LIBOBJ) source/cfa633_readmemory.o -o cfa633_readmemory

readflash: 633/cfa633_readflash.o $(LIBOBJ)
	$(CC) $(LDFLAGS) $(LIBOBJ) source/cfa633_readflash.o -o cfa633_readflash

writeflash: 633/cfa633_writeflash.o $(LIBOBJ)
	$(CC) $(LDFLAGS) $(LIBOBJ) source/cfa633_writeflash.o -o cfa633_writeflash

clean:
	rm -f $(LIBOBJ) source/cfa633_menu.o cfa633_menu source/cfa633_readflash.o cfa633_readflash cfa633_readmemory source/cfa633_readmemory.o source/cfa633_writeflash.o cfa633_writeflash source/menustructure.o
