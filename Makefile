SRC = src/*.c
TAR = dgy
CC = zig cc
CFLAGS = -g -Wall

all:
	${CC} -o ${TAR} ${CFLAGS} ${SRC}
