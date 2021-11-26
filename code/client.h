#include "helpers.h"
#include <arpa/inet.h>
#include <dirent.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * Recieves file from server in a single thread
 * @param socket - socket to recieve file from
 * @param filename - name of file to recieve
 * @return 1 if file recieved, 0 if file not recieved
 */
int recieveFile(int socket, char *filename);
/*
 * Writes a certain chunk to a file
 * @param *threadarg - struct containing fp, socket, buffer, s, f, fSize
 * @return void*
 */
void *threadedFileWrite(void *threadarg);
/*
 * Recieves file from server via multiple threads
 * @param socket - socket to recieve file from
 * @param filename - name of file to recieve
 * @return 1 if file recieved, 0 if file not recieved
 */
int threadedRecieveFile(int socket, char *filename, int NUM_THREADS, char *ip,
                        int port);

static pthread_mutex_t writeLock = PTHREAD_MUTEX_INITIALIZER;

char *dir = "clientDir";
char *temp = "temp";
char *ext = ".part";