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
#include "utils.h"
#include "queue.h"


struct sockaddr_in format_addr(char * ip, int port) {
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
