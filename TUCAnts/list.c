#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void initList(list *lis){
	lis->start= NULL;
	lis->tail= NULL;
}
void freeList(list * lis){
	while(lis->start != NULL)
		free(pop(lis));
	free(lis);
}
void emptyList(list * lis){
	while(lis->start != NULL)
		free(pop(lis));
}
void push(list *lis, Move* data)
{
	node *p;
	p = malloc(sizeof(node));
	p->data = data;
	p->next = lis->start;
	if(lis->start == NULL)lis->tail = p;
	lis->start = p;
}
Move* top(list *lis){
	if(lis->start == NULL) return NULL;
	if(lis == NULL )return NULL;
	return lis->start->data;
}
Move* pop(list *lis)
{
	if(lis->start == NULL) return NULL;
	if(lis == NULL )return NULL;
	Move *move;
	node *p = lis->start;
	lis->start = lis->start->next;
	if(lis->start == NULL)
		lis->tail = NULL;
	move = p->data;
	free(p);
	
	return move;
}




