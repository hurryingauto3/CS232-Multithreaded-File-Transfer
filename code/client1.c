#include <arpa/inet.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "helpers.h"
#include <pthread.h>

// Forward declaration of functions
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
int threadedRecieveFile(int socket, char *filename);
static pthread_mutex_t writeLock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{
    ////// Initialize variables //////
    char *ip = argv[1];
    int port = atoi(argv[2]);
    char *fileName = argv[3];
    char *newfileName = argv[4];
    printf("%s\n", newfileName);
    // int NUM_THREADS = atoi(argv[5]);
    // Socket variables
    int cSocket;
    struct sockaddr_in server;
    char *server_reply[SIZE];

    ////// Establish Connection //////
    if ((cSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        error("Could not create socket.");
        exit(1);
    }
    success("Socket created.");
    // Set server address
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    // Connect to server
    if (connect(cSocket, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        error("Connection failed.");
        exit(1);
    }
    success("Connection established.");

    /////// Begin file request ///////
    read(cSocket, server_reply, SIZE);
    reply((void *)server_reply);
    /////// File name sent to server ///////
    // write(cSocket, &NUM_THREADS, sizeof(NUM_THREADS));
    write(cSocket, fileName, strlen(fileName));

    /////// File recieved from server ///////
    if (threadedRecieveFile(cSocket, newfileName))
        success("File received.");
    else
        error("File could not be recieved.");
    /////// File sent to server ///////
    close(cSocket);
    success("Socket closed.");
    return 0;
    /////// End file request ///////
}

int recieveFile(int socket, char *filename)
{
    // Server sends file size
    long fSize;
    read(socket, &fSize, sizeof(long));
    // printf("File size: %ld bytes\n", fSize);

    int n;
    char buffer[SIZE];
    FILE *fp = fopen(filename, "wb");

    if (fp == NULL)
        return 0;

    wait("for file to be recieved.");
    for (int i = 0; i < fSize; i += SIZE)
    {
        if ((n = read(socket, buffer, SIZE)) < 0)
        {
            error("Error reading file.");
            return 0;
        }
        fwrite(buffer, sizeof(char), n, fp);
    }

    fclose(fp);
    return 1;
}

void *threadedFileWrite(void *threadarg)
{
    int n;
    int id;
    // Get struct from threadarg
    struct thread_data *data = (struct thread_data *)threadarg;
    if(read(data->socket, &id, sizeof(int)) > 0)
    {
        printf("Thread %d\n", id);
    }
    else{
        sleep(1);
    }
    printf("PID: %d, TID: %d, rID: %d, ID: %d\n", getpid(), pthread_self(), id, data->id);
    if (id == data->id)
    {
        pthread_mutex_trylock(&writeLock);
        for (long i = data->s; i < data->f; i += SIZE)
        {
            if (data->fSize - i < SIZE)
            {

                if ((n = read(data->socket, data->buffer, data->fSize - i)) < 0)
                    error("Error writing file.");
                fwrite(data->buffer, sizeof(char), data->fSize - i, data->fp);
            }
            else
            {
                if ((n = read(data->socket, data->buffer, SIZE)) < 0)
                    error("Error writing file.");
                fwrite(data->buffer, sizeof(char), SIZE, data->fp);
            }
            bzero(data->buffer, SIZE);
        }
        pthread_mutex_unlock(&writeLock);
    }
    else
    {
        printf("Slep\n");   
        sleep(1);
    }
}

int threadedRecieveFile(int socket, char *filename)
{
    // Recieve file size
    long fSize;
    read(socket, &fSize, sizeof(long));
    // Initialize variables depending on number of threads
    char *buffer[NUM_THREADS];
    pthread_t threads[NUM_THREADS];
    struct thread_data td[NUM_THREADS];
    // Open file in write binary mode
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL)
        return 0;

    printf("\033[1;33m");
    printf("[*]Using %d threads to recieve file.\n", NUM_THREADS);
    printf("\033[0m");
    for (int i = 0; i < NUM_THREADS; i++)
    {
        // Add data to struct for each thread
        buffer[i] = (char *)malloc(SIZE);
        td[i].fp = fp;
        td[i].socket = socket;
        td[i].buffer = buffer[i];
        td[i].s = i * fSize / NUM_THREADS;
        td[i].f = (i + 1) * fSize / NUM_THREADS;
        td[i].fSize = fSize;
        td[i].id = i;
        // threadedFileWrite((void*)&td[i]);
        // Create thread to write to file
        pthread_create(&threads[i], NULL, (void *)threadedFileWrite, (void *)&td[i]);
    }
    // Wait for threads to finish
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    fclose(fp);
    return 1;
}