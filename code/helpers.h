#include <arpa/inet.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>

// #define SIZE 1024

struct thread_data {
  FILE *fp;
  long s, f, fSize;
  int id, port;
  char *ip;
  char *fileName;
};

void error(char *msg) {
  printf("\033[1;31m");
  printf("[-]Error: %s\n", msg);
  printf("\033[0m");
}

void success(char *msg) {
  printf("\033[1;32m");
  printf("[+]Success: %s\n", msg);
  printf("\033[0m");
}

void wait(char *msg) {
  printf("\033[1;33m");
  printf("[*]Waiting for %s...\n", msg);
  printf("\033[0m");
}

void reply(char *msg) {
  printf("\033[1;34m");
  printf("[S]%s\n", msg);
  printf("\033[0m");
}