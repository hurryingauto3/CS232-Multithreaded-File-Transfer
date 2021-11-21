all: server client
server: server.c helpers.h
	gcc -o server server.c helpers.h -lpthread
client: client.c helpers.h
	gcc -o client client.c helpers.h -lpthread
