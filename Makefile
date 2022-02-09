#Makefile for Test Task project

TARGET = client server

.PHONY: all clean

all: $(TARGET)

.c.:
	gcc -c $^

server: server.c
	gcc -o $@ $< -lpthread

clean:
	rm -f $(TARGET)