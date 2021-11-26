all: server client testfile
server: server.c helpers.h
	gcc -o server server.c helpers.h server.h -lpthread
client: client.c helpers.h
	gcc -o client client.c helpers.h client.h -lpthread
testfile: testfile.c
	gcc -o testfile testfile.c helpers.h -lpthread