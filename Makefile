SRC = src/*.c
TAR = DGY
CC = gcc
CFLAGS = -g -Wall

all:
	${CC} -o ${TAR} ${CFLAGS} ${SRC}