#ifndef _LIST_H
#define _LIST_H
#include "move.h"

typedef struct node{
	Move* data;
	struct node *next;
} node;


typedef struct {
	node *start;
	node *tail;
} list;


void initList(list *sList);
void push(list *sList, Move* data);
Move* pop(list *sList);
void freeList(list*);
Move* top(list *sList);
void emptyList(list*);
#endif