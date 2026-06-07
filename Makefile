SRC = src/*.c
TAR = dgy
CC = gcc
CFLAGS = -g -Wall

all:
	${CC} -o ${TAR} ${CFLAGS} ${SRC}
