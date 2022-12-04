#ifndef QUEUE
#define QUEUE


typedef struct TNode {
  int id;
  char buffer[200];
  int len;
  struct TNode* next;
} TNode;
int insert(int id, const char * chunck, int len, struct TNode **ptr_init);
int removeList(struct TNode **ptr_init);

#endif
