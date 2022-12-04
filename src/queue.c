#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include "queue.h"


int insert(int id, const char * chunck,int len, struct TNode **ptr_init) {
  TNode * p;
  p = (TNode *) malloc(sizeof(TNode));
  if(p == NULL) {
    return 0;
  }
  p->id = id;
  p->len = len;
  strcpy(p->buffer, chunck);
  p->next = NULL;

  if (*ptr_init == NULL) {
    printf("First item\n");
    *ptr_init = p;
  } else {
    TNode * temp = *ptr_init;
    while(temp->next != NULL)
    {
      temp = temp->next;
    }
    temp->next = p;
  }
  return 1;
}

int removeList(struct TNode **ptr_init) {
  TNode * ptr = *ptr_init;
  TNode * temp = *ptr_init;

  if (ptr_init == NULL) {
    return 0;
  } else {
    if (ptr->next == NULL) {
      ptr_init = NULL;
    } else {
      while (ptr->next != NULL) {
        temp = ptr;
        ptr = ptr->next;
      }
      temp->next = NULL;
    }
    free(ptr);
  }
  return 1;
}
