#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include "utils.h"
#include "queue.h"

#define MAX_BUFFER 200

int newBufferLen = 0;
TNode * ptr_init = NULL;
int idx = 0;


void send_message(int sd, struct sockaddr_in endClient, char * message) {
  sendto(sd, message, strlen(message), 0, (const struct sockaddr *)&endClient, sizeof(endClient));
}

void create_client(char * ip, int port, char * ip_server, int port_server, int bufferLen) {
  printf("[SOCKET][CLIENT] - INFO - Starting socket...\n");
  int sd = start_socket();

  printf("[SOCKET][CLIENT] - INFO - Configuring client and server.\n");
  struct sockaddr_in client = format_addr(ip, port);
  struct sockaddr_in server = format_addr(ip_server, port_server);

  printf("[SOCKET][CLIENT] - INFO - Reading message file.\n");
  FILE * f = fopen("./src/static/test.csv", "rb");
  size_t bytes;
  char * buffer = (char *)malloc(newBufferLen * sizeof(char));
  newBufferLen = bufferLen;
  
  printf("[SOCKET][CLIENT] - INFO - BIND IP: %s\tPORT: %d\tSERVER IP: %s\tPORT: %d\n", ip, port, ip_server, port_server);
  
  if(bind(sd, (const struct sockaddr *)&client, sizeof(client)) < 0) {
   perror("[SOCKET][CLIENT] - FAIL - Failed to bind socket.\n");
   exit(EXIT_FAILURE); 
  }

  if(f == NULL) {
    perror("[SOCKET][CLIENT] - FAIL - Error at message file!\n");
    exit(1); 
  }

  printf("%d\n", newBufferLen);

  while((bytes = fread(buffer, sizeof(char), newBufferLen, f))) {
    send_message(sd, server, buffer);
  }

  send_message(sd, server, "print");

  free(buffer);
} 

int main (int argc, char *argv[]) {
  char * ip_server, * ip_client;
  int port_client, port_server2;

  ip_server = argv[1];
  port_server2 = atoi(argv[2]);
  ip_client = argv[3];
  port_client = atoi(argv[4]);  

  create_client(ip_server, port_server2, ip_client, port_client, MAX_BUFFER);

  return 0; 
}
