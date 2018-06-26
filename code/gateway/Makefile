#Makefile

all : server client checkin

server: server.c jsonParse.c server.h
	gcc -pthread -I -Wall server.c -o server

client: client.c
	gcc -Wall client.c -o client

checkin: checkin.c
	gcc -Wall checkin.c -o checkin

run: client server
	./server 1111

clean:
	rm server client checkin
