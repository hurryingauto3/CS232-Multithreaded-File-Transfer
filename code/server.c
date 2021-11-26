#include "server.h"

int main(int argc, char *argv[]) {
  // Variables being taken CLI
  char *ip = argv[1];
  int port = atoi(argv[2]);
  // Socket variables
  int sSocket, cSocket, c;
  struct sockaddr_in server, client;
  if ((sSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    error("Could not create socket");
    exit(1);
  }
  success("Socket created.");
  // Define socket ip, port
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = inet_addr(ip);
  // Bind socket to ip and port
  if (bind(sSocket, (struct sockaddr *)&server, sizeof(server)) < 0) {
    error("Bind failed.");
    exit(1);
  }
  success("Socket binded.");
  // Listen for connections for up to 10 connections

  // Accept connection from client
  c = sizeof(struct sockaddr_in);
  char *fileName[100];
  int NUM_THREADS;

  listen(sSocket, 10);
  wait("incoming connections");
  while ((cSocket =
              accept(sSocket, (struct sockaddr *)&client, (socklen_t *)&c))) {
    write(cSocket, message, strlen(message));
    success("Connection accepted.");
    while (1) {
      read(cSocket, fileName, 100);
      read(cSocket, &NUM_THREADS, sizeof(NUM_THREADS));
      if (strcmp((void *)fileName, "EXIT") == 0) {
        success("Client has disconnected.");
        wait("incoming connections");
        memset(fileName, 0, 100);
        memset(&NUM_THREADS, 0, sizeof(NUM_THREADS));
        cSocket = 0;
        break;
      }
      if (NUM_THREADS == 0) {
        error("Number of threads must be greater than 0");
        memset(fileName, 0, 100);
        memset(&NUM_THREADS, 0, sizeof(NUM_THREADS));
        break;
      }
      if (cSocket < 0) {
        error("Connection failed.");
        memset(fileName, 0, 100);
        memset(&NUM_THREADS, 0, sizeof(NUM_THREADS));
        break;
      }
      if (threadedSendFile(cSocket, (void *)fileName, NUM_THREADS, ip, port)) {
        success("File sent.");
        memset(fileName, 0, 100);
        memset(&NUM_THREADS, 0, sizeof(NUM_THREADS));
        continue;
      } else {
        error("File not sent.");
        memset(fileName, 0, 100);
        memset(&NUM_THREADS, 0, sizeof(NUM_THREADS));
        continue;
      }
    }
  }

  close(cSocket);
  close(sSocket);
  success("Socket closed.");
  return 0;
}

void *threadedFileRead(void *threadarg) {
  struct thread_data *data = (struct thread_data *)threadarg;
  
  int sSocket, cSocket, c;
  struct sockaddr_in server, client;
  c = sizeof(struct sockaddr_in);

  printf("Size: %ld", data->f - data->s);

  if ((sSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    close(sSocket);
    pthread_exit(NULL);
  }
  // Define socket ip, port
  server.sin_family = AF_INET;
  server.sin_port = htons(data->port);
  server.sin_addr.s_addr = inet_addr(data->ip);
  // Bind socket to ip and port
  if (bind(sSocket, (struct sockaddr *)&server, sizeof(server)) < 0) {
    close(cSocket);
    close(sSocket);
    pthread_exit(NULL);
  }

  listen(sSocket, 1);

  if ((cSocket = accept(sSocket, (struct sockaddr *)&client, (socklen_t *)&c)) <
      0) {
    close(cSocket);
    close(sSocket);
    pthread_exit(NULL);
  }
  printf("Port: %d, thread: %d, Size: %ld, s: %ld, f: %ld\n", data->port,
  data->id, data->f - data->s, data->s, data->f);
  write(cSocket, &data->id, sizeof(data->id));

  long SIZE = data->f - data->s;
  char *buffer = (char *)malloc(sizeof(char) * SIZE);
  pread(fileno(data->fp), buffer, SIZE, data->s);

  write(cSocket, buffer, SIZE);
  memset(buffer, 0, SIZE);
  free(buffer);

  // for (long i = data->s; i < data->f; i += SIZE) {
  //   if (data->f- i < SIZE) {
  //     char *buffer = (char*)malloc(data->fSize - i);
  //     fread(buffer, sizeof(char), data->fSize - i, data->fp);
  //     write(cSocket, buffer, data->fSize - i);
  //     memset(buffer, 0, data->fSize - i);
  //     free(buffer);
  //   } else {
  //     char *buffer = (char*)malloc(SIZE*sizeof(char));
  //     fread(buffer, sizeof(char), SIZE, data->fp);
  //     write(cSocket, buffer, SIZE);
  //     memset(buffer, 0, SIZE);
  //     free(buffer);
  //   }
  // }
  close(cSocket);
  close(sSocket);
  pthread_exit(NULL);
}

int threadedSendFile(int socket, char *fileName, int NUM_THREADS, char *ip,
                     int port) {

  // Open file;
  FILE *fp = fopen((void *)fileName, "rb");
  if (fp == NULL)
    return 0;
  // Get file size
  fseek(fp, 0, SEEK_END);
  long fSize = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  // fSize = ntohl(fSize);
  printf("File size: %ld\n", fSize);
  // Send file size to client
  write(socket, &fSize, sizeof(long));
  // Send file to client
  pthread_t threads[NUM_THREADS];
  // char *buffer[NUM_THREADS];
  struct thread_data td[NUM_THREADS];
  printf("\033[1;33m");
  printf("[*]Using %d threads to send file.\n", NUM_THREADS);
  printf("\033[0m");

  for (int i = 0; i < NUM_THREADS; i++) {

    td[i].fp = fp;
    td[i].s = i * (fSize / NUM_THREADS);
    if(i == NUM_THREADS - 1)
      td[i].f = fSize;
    else
      td[i].f = (i + 1) * (fSize / NUM_THREADS);
    td[i].fSize = fSize;
    td[i].id = i;
    td[i].ip = ip;
    td[i].port = port + (i + 1);
    

    pthread_create(&threads[i], NULL, (void *)threadedFileRead, (void *)&td[i]);
  }
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  fclose(fp);
  return 1;
}