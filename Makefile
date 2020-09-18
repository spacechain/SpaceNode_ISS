#!/bin/bash

DIR_INC = ./include
DIR_LIB_INC = /home/twhite/mylibs/adrvLibs/include
DIR_LIB_LINK = /home/twhite/mylibs/adrvLibs/lib
#DIR_OPENSSL_INC = /home/twhite/mylibs/install/openssl/include
#DIR_OPENSSL_LIB = /home/twhite/mylibs/install/openssl/lib
DIR_SRC = ./src
DIR_OBJ = ./obj
DIR_BIN = ./bin

SRC_MODS = $(wildcard ${DIR_SRC}/mods/*.c)
SRC_TASKS = $(wildcard ${DIR_SRC}/tasks/*.c)
OBJ_MODS = $(patsubst %.c,${DIR_OBJ}/mods/%.o,$(notdir ${SRC_MODS}))
OBJ_TASKS = $(patsubst %.c,${DIR_OBJ}/tasks/%.o,$(notdir ${SRC_TASKS}))

TARGET = spacenode

BIN_TARGET = ${DIR_BIN}/${TARGET}

CROSS_COMPILE=arm-none-linux-gnueabihf-
CC=${CROSS_COMPILE}gcc
CFLAGS=-O3 -g3 -Wall -std=gnu99 -I${DIR_INC} -I${DIR_LIB_INC} -L${DIR_LIB_LINK}
LFLAGS= -lm -lgpg-error -lgcrypt -lssl -lcrypto 

${BIN_TARGET}:${OBJ_MODS} ${OBJ_TASKS}
	$(CC) $(OBJ_MODS) $(OBJ_TASKS) $(CFLAGS) $(LFLAGS) -o $@

${DIR_OBJ}/mods/%.o:${DIR_SRC}/mods/%.c
	$(CC) $(CFLAGS) $(LFLAGS) -c $< -o $@  

${DIR_OBJ}/tasks/%.o:${DIR_SRC}/tasks/%.c
	$(CC) $(CFLAGS) $(LFLAGS) -c $< -o $@  

./PHONY:clean
clean:
	rm -f ${DIR_OBJ}/mods/*.o
	rm -f ${DIR_OBJ}/tasks/*.o
	rm -f ${BIN_TARGET}


