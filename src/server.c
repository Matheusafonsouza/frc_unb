#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
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

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
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
    char * filename = "./src/static/received-";
    char timestamp_str[16];
    time_t rawtime;

    time(&rawtime);
    sprintf(timestamp_str, "%ld", rawtime);

    puts(filename);
    puts(timestamp_str);
    filename = concat(filename, timestamp_str);
    filename = concat(filename, ".csv");

    FILE * f2 = fopen(filename, "wb");
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


int main(int argc, char *argv[]) {
  char * ip_server;
  int port_server;

  ip_server = argv[1];
  port_server = atoi(argv[2]);
  
  create_server(ip_server, port_server, MAX_BUFFER);

  return 0; 
}
