#ifndef NETWORK_UTIL
#define NETWORK_UTIL

#include <mqueue.h>


struct sockaddr_in format_addr(char * ip, int port);
int start_socket();
int receive_message(int sd, struct sockaddr_in endClient, int bufferLen);
void stop_queue(mqd_t queue, char * name, char * buffer);
void send_message(int sd, struct sockaddr_in endClient, char * message);
void create_server(char * ip, int port, int bufferLen);
void create_client(char * ip, int port, char * ip_server, int port_server, int bufferLen);
mqd_t start_enlace_queue();
mqd_t start_server_queue();

#endif
