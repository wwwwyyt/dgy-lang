SRC = src/*.c
TAR = datamate
CC = gcc
CFLAGS = -g -Wall

all:
	${CC} -o ${TAR} ${CFLAGS} ${SRC}