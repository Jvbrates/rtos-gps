CC=gcc
CC_FLAGS=-Wall
C_SOURCE=$(wildcard *.c)
LIBS=-lm \
		 -lpthread \
     -lrt

server:
	$(CC) -o server $(C_SOURCE) $(LIBS) $(CC_FLAGS)
