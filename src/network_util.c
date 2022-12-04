#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <mqueue.h>
#include "network_util.h"
#include "queue.h"

int newBufferLen = 0;
TNode * ptr_init = NULL;
int idx = 0;

struct sockaddr_in format_addr(char * ip, int port)
{
  struct sockaddr_in server_addr;
  memset(&server_addr, 0x0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip);
  server_addr.sin_port = htons(port);
  return server_addr;
}

int start_socket() {
  int start_socket;
  start_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (start_socket < 0) {
    perror("[SOCKET] - FAIL - Failed to open socket.\n");
    exit(EXIT_FAILURE);
  }
  return start_socket;
}

mqd_t start_enlace_queue() {
    mqd_t mq;
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = newBufferLen;
    attr.mq_curmsgs = 0;

    mq = mq_open("/enlace", O_CREAT | O_RDONLY, 0644, &attr);
    return mq;
}

mqd_t start_server_queue() {
    mqd_t mq;
    mq = mq_open("/server", O_WRONLY);
    return mq;
}

void stop_queue(mqd_t queue, char * name, char * buffer) {
  free(buffer);
  mq_close(queue);
  mq_unlink(name);
  exit(0);
}

int receive_message(int sd, struct sockaddr_in endClient, int bufferLen) {
  char * buffer;
  int lenMessage;
  int len = sizeof(endClient);
  int stop = 0;
  buffer = (char *)malloc(bufferLen * sizeof(char));
  printf("[SOCKET][SERVER] - INFO - Waiting client message...\n");
  memset(buffer, 0x0, bufferLen);
  lenMessage = recvfrom(sd, buffer, bufferLen, 0, (struct sockaddr *)&endClient, (unsigned int *)&len);
  
  if(strstr(buffer, "len")) {
    printf("[SOCKET][SERVER] - INFO - Buffer size before: %d\n", newBufferLen);
    char *ptr = strtok(buffer, ":");
    ptr = strtok(NULL, ":");
    newBufferLen = atoi(ptr);
    printf("[SOCKET][SERVER] - INFO - Buffer size after: %d\n", newBufferLen);
  }

  if(!strcmp(buffer, "stop")) {
    stop = 1;
  }
  if(!strcmp(buffer, "print")) {
    FILE * f2 = fopen("./src/static/received.csv", "wb");
    TNode * temp;
    for(temp=ptr_init; temp != NULL; temp=temp->next) {
      memset(buffer, 0x0, newBufferLen);
      printf("%d -> %s\n", temp->id, temp->buffer);
      fwrite(temp->buffer, sizeof(char), strlen(ptr_init->buffer), f2);
    }
    fclose(f2);
  } else {
   insert(idx, buffer, lenMessage, &ptr_init);
   idx += 1;
  }
  free(buffer);
  return stop;
}

void send_message(int sd, struct sockaddr_in endClient, char * message) {
  sendto(sd, message, strlen(message), 0, (const struct sockaddr *)&endClient, sizeof(endClient));
}

void create_server(char * ip, int port, int bufferLen) {
  printf("[SOCKET][SERVER] - INFO - Starting socket...\n");
  int sd = start_socket();

  int stop = 0;
  struct sockaddr_in server = format_addr(ip, port);

  if (bind(sd, (const struct sockaddr *)&server, sizeof(server)) < 0) {
   perror("[SOCKET][SERVER] - FAIL - Failed to bind socket.\n");
   exit(EXIT_FAILURE); 
  }
  printf("[SOCKET][SERVER] - INFO - Server ready to receive messages! IP: %s\tPORT: %d\n", ip, port);

  pid_t pid = fork();
  newBufferLen = bufferLen;

  if (pid == 0) {
    mqd_t mq_enlace, mq_server;
    char * buffer = (char *) malloc(newBufferLen * sizeof(char));
    char sizes[10];
    int must_stop = 0;

    mq_enlace = start_enlace_queue();
    mq_server = start_server_queue();

    do {
        mq_receive(mq_enlace, buffer, newBufferLen, NULL);
        if (strncmp(buffer, "exit", strlen("exit"))) {
          sprintf(sizes, "%d", (int)strlen(buffer));
          mq_send(mq_server, sizes, 10, 0);
        } else {
          must_stop = 1;
        }
    } while (!must_stop);

    stop_queue(mq_enlace, "/enlace", buffer);
  } else {
    mqd_t mq_enlace, mq_server;
    char buffer_2[10];

    mq_enlace = start_enlace_queue();
    mq_server = start_server_queue();

    while(1){
      if(stop == 1) {
        int bytes = 0;
        while(1){
          printf("[SOCKET][SERVER] - INFO - Sending frame.\n");
          mq_send(mq_enlace, ptr_init->buffer, newBufferLen, 0);
          printf("[SOCKET][SERVER] - INFO - Receiving enlace.\n");
          bytes = mq_receive(mq_server, buffer_2, 10, NULL);
          printf("[SOCKET][SERVER] - INFO - Checking frame.\n");
          if(ptr_init->next != NULL && bytes > 0) {
            if(ptr_init->len == atoi(buffer_2) || (ptr_init->len == 225 && atoi(buffer_2) == 203)){
              printf("Success Frame!\n");
              ptr_init = ptr_init->next;
            }
          } else {
            break;
          }
        }
        mq_send(mq_enlace, "exit", newBufferLen, 0);
        mq_close(mq_server);
        mq_unlink("/server");
        break;
      }
      stop = receive_message(sd, server, newBufferLen); 
    }
    wait(NULL);
    printf("[SOCKET][SERVER] - INFO - File successfully sent.\n");
    exit(0);
  }
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
  send_message(sd, server, "stop");

  free(buffer);
} 
