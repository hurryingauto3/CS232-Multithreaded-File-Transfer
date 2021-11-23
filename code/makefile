all: server client testfile
server: server.c helpers.h
	gcc -o server server.c helpers.h -lpthread
client: client.c helpers.h
	gcc -o client client.c helpers.h -lpthread
testfile: testfile.c
	gcc -o testfile testfile.c helpers.h -lpthread