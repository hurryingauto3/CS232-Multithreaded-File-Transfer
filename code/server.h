#include "helpers.h"
#include <arpa/inet.h>
#include <dirent.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

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
int threadedSendFile(int socket, char *fileName, int NUM_THREADS, char *ip,
                     int port);

const char *message = "Hello Client, I have received your connection";
const char *dir = "server_dir";
static pthread_mutex_t readLock = PTHREAD_MUTEX_INITIALIZER;