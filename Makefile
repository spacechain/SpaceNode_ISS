#!/bin/bash

DIR_INC = ./include
DIR_GCRYPT_INC = /home/twhite/mylibs/install/libgcrypt-1.7.8/include
DIR_GCRYPT_LIB = /home/twhite/mylibs/install/libgcrypt-1.7.8/lib
DIR_GPGERROR_INC = /home/twhite/mylibs/install/libgpg-error-1.37/include
DIR_GPGERROR_LIB = /home/twhite/mylibs/install/libgpg-error-1.37/lib
DIR_OPENSSL_INC = /home/twhite/mylibs/install/openssl/include
DIR_OPENSSL_LIB = /home/twhite/mylibs/install/openssl/lib
DIR_SRC = ./src
DIR_OBJ = ./obj
DIR_BIN = ./bin

SRC_MODS = $(wildcard ${DIR_SRC}/mods/*.c)
SRC_TASKS = $(wildcard ${DIR_SRC}/tasks/*.c)
OBJ_MODS = $(patsubst %.c,${DIR_OBJ}/mods/%.o,$(notdir ${SRC_MODS}))
OBJ_TASKS = $(patsubst %.c,${DIR_OBJ}/tasks/%.o,$(notdir ${SRC_TASKS}))

TARGET = spacenode

BIN_TARGET = ${DIR_BIN}/${TARGET}

CC=${CROSS_COMPILE}gcc
CFLAGS=-O3 -g3 -Wall -std=gnu99 -I${DIR_INC} -I${DIR_GPGERROR_INC} -L${DIR_GPGERROR_LIB} -I${DIR_GCRYPT_INC} -L${DIR_GCRYPT_LIB} -I${DIR_OPENSSL_INC} -L${DIR_OPENSSL_LIB} 
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


