CC=gcc
RM=rm -f

default: all
all: server 
server: server.c
	$(CC) server.c -o server 
clean:
	$(RM) server temp
