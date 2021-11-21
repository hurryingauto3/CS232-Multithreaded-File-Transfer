#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "helpers.h"
#include <pthread.h>
#include <dirent.h>

/*
* Sends file to client in a single thread
* @param socket - socket to send file to
* @return 1 if file send, 0 if file not sent
*/
int sendFile(int socket);
/*
* Reads and Sends a certain chunk of a file
* @param *threadarg - struct containing fp, socket, buffer, s, f, fSize
* @return void*
*/
void *threadedFileRead(void *threadarg);
/*
* Sends file to client via multiple threads
* @param socket - socket to send file to
* @return 1 if file send, 0 if file not sent
*/
int threadedSendFile(int socket);

const char *message = "Hello Client, I have received your connection";
const char *dir = "server_dir";
static pthread_mutex_t readLock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{
	// Variables being taken CLI
	char *ip = argv[1];
	int port = atoi(argv[2]);
	// Socket variables
	int sSocket, cSocket, c;
	struct sockaddr_in server, client;

	if ((sSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		error("Could not create socket");
		exit(1);
	}
	success("Socket created.");
	// Define socket ip, port
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = inet_addr(ip);
	// Bind socket to ip and port
	if (bind(sSocket, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		error("Bind failed.");
		exit(1);
	}
	success("Socket binded.");
	// Listen for connections for up to 10 connections
	listen(sSocket, 10);
	wait("incoming connections");
	// Accept connection from client
	c = sizeof(struct sockaddr_in);
	while ((cSocket = accept(sSocket, (struct sockaddr *)&client, (socklen_t *)&c)))
	{
		// Server Side prompt and greeting sent.
		success("Connection accepted.");
		write(cSocket, message, strlen(message));
		printf("NUM THREADS: %d\n", NUM_THREADS);

		if (threadedSendFile(cSocket))
		{
			success("File sent.");
		}
		else
		{
			error("File not sent.");
		}
		wait("incoming connections");
		if (cSocket < 0)
		{
			error("Connection failed.");
			return 1;
		}
	}

	close(sSocket);
	success("Socket closed.");
	return 0;
}

int sendFile(int socket)
{
	// reads filename
	char *fileName[SIZE];
	read(socket, fileName, SIZE);

	// Open file;
	FILE *fp = fopen((void *)fileName, "rb");
	if (fp == NULL)
		return 0;
	// Get file size
	fseek(fp, 0L, SEEK_END);
	long fSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	// Send file size to client
	write(socket, &fSize, sizeof(long));
	// Send file to client
	char *buffer = (char *)malloc(sizeof(char) * SIZE);

	for (int i = 0; i < fSize; i += SIZE)
	{

		if (fSize - i < SIZE)
		{
			fread(buffer, sizeof(char), fSize - i, fp);
			write(socket, buffer, fSize - i);
		}
		else
		{
			fread(buffer, sizeof(char), SIZE, fp);
			write(socket, buffer, SIZE);
		}
	}
	fclose(fp);
	free(buffer);
	return 1;
}

void *threadedFileRead(void *threadarg)
{
	struct thread_data *data = (struct thread_data *)threadarg;
	FILE *fp = data->fp;
	int socket = data->socket;
	char *buffer = data->buffer;
	long s = data->s;
	long f = data->f;
	long fSize = data->fSize;
	printf("PID: %d, TID: %d\n", getpid(), pthread_self());
	for (long i = s; i < f; i += SIZE)
	{
		pthread_mutex_lock(&readLock);
		if (fSize - i < SIZE)
		{
			fread(buffer, sizeof(char), fSize - i, fp);
			write(socket, buffer, fSize - i);
		}
		else
		{
			fread(buffer, sizeof(char), SIZE, fp);
			write(socket, buffer, SIZE);
		}
		pthread_mutex_unlock(&readLock);
	}
}

int threadedSendFile(int socket)
{
	// reads filename
	char *fileName[SIZE];
	read(socket, fileName, SIZE);

	// Open file;
	FILE *fp = fopen((void *)fileName, "rb");
	if (fp == NULL)
		return 0;
	// Get file size
	fseek(fp, 0L, SEEK_END);
	long fSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	// Send file size to client
	write(socket, &fSize, sizeof(long));
	// Send file to client
	pthread_t threads[NUM_THREADS];
	char *buffer[NUM_THREADS];
	struct thread_data td[NUM_THREADS];
	printf("\033[1;33m");
	printf("[*]Using %d threads to send file.\n", NUM_THREADS);
	printf("\033[0m");
	for (int i = 0; i < NUM_THREADS; i++)
	{

		buffer[i] = (char *)malloc(SIZE);
		td[i].fp = fp;
		td[i].socket = socket;
		td[i].buffer = buffer[i];
		td[i].s = i * (fSize / NUM_THREADS);
		td[i].f = (i + 1) * (fSize / NUM_THREADS);
		td[i].fSize = fSize;

		// threadedFileRead((void *)&td[i]);
		pthread_create(&threads[i], NULL, (void *)threadedFileRead, (void *)&td[i]);
	}
	for (int i = 0; i < NUM_THREADS; i++)
	{
		pthread_join(threads[i], NULL);
	}
	fclose(fp);
	return 1;
}