#include "client.h"

int main(int argc, char *argv[]) {
  ////// Initialize variables //////
  char *ip = argv[1];
  int port = atoi(argv[2]);
  // Socket variables
  int cSocket;
  struct sockaddr_in server;
  char *server_reply[100];
  ////// Establish Connection //////
  if ((cSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    error("Could not create socket.");
    exit(1);
  }
  success("Socket created.");
  // Set server address
  server.sin_addr.s_addr = inet_addr(ip);
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  // Connect to server
  if (connect(cSocket, (struct sockaddr *)&server, sizeof(server)) < 0) {
    error("Connection failed.");
    exit(1);
  }
  success("Connection established.");
  char *fileName[100];
  char *newfileName[100];
  int NUM_THREADS = 0;
  /////// Begin file request ///////
  read(cSocket, server_reply, 100);
  reply((void *)server_reply);
  /////// File name sent to server ///////
  while (1) {
    printf("Requested file name [Enter \"EXIT\" to quit]: ");
    scanf("%s", fileName);
    write(cSocket, fileName, strlen((void *)fileName));
    if (strcmp((void *)fileName, "EXIT") == 0) {
      memset(&NUM_THREADS, 0, sizeof(NUM_THREADS));
      memset(newfileName, 0, sizeof(newfileName));
      memset(fileName, 0, sizeof(fileName));
      break;
    }
    while (NUM_THREADS < 1) {
      printf("Number of threads must be greater than 0.\n");
      printf("Number of threads: ");
      scanf("%d", &NUM_THREADS);
    }
    write(cSocket, &NUM_THREADS, sizeof(NUM_THREADS));
    /////// File name sent to server ///////
    printf("Save file as: ");
    scanf("%s", newfileName);
    /////// File recieved from server ///////
    if (threadedRecieveFile(cSocket, (void *)newfileName, NUM_THREADS, ip,
                            port)) {
      success("File received.");
      memset(&NUM_THREADS, 0, sizeof(NUM_THREADS));
      memset(newfileName, 0, sizeof(newfileName));
      memset(fileName, 0, sizeof(fileName));
      continue;
    } else {
      error("File could not be recieved.");
      memset(&NUM_THREADS, 0, sizeof(NUM_THREADS));
      memset(newfileName, 0, sizeof(newfileName));
      memset(fileName, 0, sizeof(fileName));
      continue;
    }
    if (cSocket == -1) {
      error("Connection lost.");
      memset(&NUM_THREADS, 0, sizeof(NUM_THREADS));
      memset(newfileName, 0, sizeof(newfileName));
      memset(fileName, 0, sizeof(fileName));
      exit(1);
    }
    /////// File sent to server ///////
  }
  free(ip);
  close(cSocket);
  success("Socket closed.");
  return 0;
  /////// End file request ///////
}

void *threadedFileWrite(void *threadarg) {

  struct thread_data *data = (struct thread_data *)threadarg;
  int cSocket;
  struct sockaddr_in server;

  char *file = (char *)malloc(sizeof(char) * 20);
  char part[3];

  if ((cSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    close(cSocket);
  // Set server address
  server.sin_addr.s_addr = inet_addr(data->ip);
  server.sin_family = AF_INET;
  server.sin_port = htons(data->port);
  // Connect to server
  if (connect(cSocket, (struct sockaddr *)&server, sizeof(server)) < 0)
    close(cSocket);

  printf("Port: %d, thread: %d, Size: %ld, s: %ld, f: %ld, filepart: %s\n",
         data->port, data->id, data->f - data->s, data->s, data->f,
         data->fileName);
  
  FILE *fp = fopen(data->fileName, "wb");

  long SIZE = (data->f - data->s)/512;
  char *buffer = (char *)malloc(sizeof(char) * SIZE);
  int n;
  for(long i = data->s ; i < data->f ; i+= SIZE) {
    if(i + SIZE > data->f) {
      n = read(cSocket, buffer, data->f - i);
      fwrite(buffer, sizeof(char), n, fp);
      // pwrite(fileno(fp), buffer, data->f - i, i);
      memset(buffer, 0, SIZE);
    } else {
      n = read(cSocket, buffer, SIZE);
      fwrite(buffer, sizeof(char), n, fp);
      // pwrite(fileno(fp), buffer, SIZE, i);
      memset(buffer, 0, SIZE);
    }

  }
  memset(buffer, 0, sizeof(buffer));
  free(buffer);

  fclose(fp);
  free(file);
  close(cSocket);
  pthread_exit(NULL);
}

int threadedRecieveFile(int socket, char *filename, int NUM_THREADS, char *ip,
                        int port) {
  // Recieve file size
  long fSize;
  read(socket, &fSize, sizeof(long));
  pthread_t threads[NUM_THREADS];
  struct thread_data td[NUM_THREADS];
  FILE *fp = fopen(filename, "wb");

  if (fp == NULL)
    return 0;

  printf("\033[1;33m");
  printf("[*]Using %d threads to recieve file.\n", NUM_THREADS);
  printf("\033[0m");

  for (int i = 0; i < NUM_THREADS; i++) {
    // Add data to struct for each thread
    char part[3];
    sprintf(part, "%d", i);
    td[i].fileName = (char *)malloc(sizeof(char) * 20);
    sprintf(td[i].fileName, "%s%s%s", temp, part, ext);
    memset(part, 0, sizeof(part));
    td[i].s = i * (fSize / NUM_THREADS);
    if (i == (NUM_THREADS - 1))
      td[i].f = fSize - (fSize / NUM_THREADS);
    else
      td[i].f = (i + 1) * (fSize / NUM_THREADS);
    td[i].fSize = fSize;
    td[i].id = i;
    td[i].ip = ip;
    td[i].port = port + (i + 1);
    printf("Port: %d, thread: %d, Size: %ld, s: %ld, f: %ld\n", td[i].port,
           td[i].id, td[i].f - td[i].s, td[i].s, td[i].f);

    // Create thread
    pthread_create(&threads[i], NULL, (void *)threadedFileWrite,
                   (void *)&td[i]);
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  for (int i = 0; i < NUM_THREADS; i++) {

    FILE *temp = fopen(td[i].fileName, "rb");
    fseek(temp, 0, SEEK_END);
    long fsize = ftell(temp);
    fseek(temp, 0, SEEK_SET);

    char *buffer = (char *)malloc(sizeof(char) * fsize);
    fread(buffer, 1, fsize, temp);
    fwrite(buffer, sizeof(char), fsize, fp);
    memset(buffer, 0, sizeof(buffer));

    free(buffer);
    fclose(temp);
    remove(td[i].fileName);
  }

  fclose(fp);
  return 1;
}