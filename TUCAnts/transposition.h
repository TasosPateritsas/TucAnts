#ifndef _TRANSPOSITION_H
#define _TRANSPOSITION_H
#include "board.h"
#include "move.h"

#define MAX_TABLE_SIZE 86028121
#define OPEN_ADDRESSING 1000

typedef struct {
	unsigned long zobrist_key;
	int upperBound;
	int lowerBound;
	char upperDepth;
	char lowerDepth;
	char validity; 
}Transpotion;

Transpotion * hashTable;



unsigned int simpleHash(long);
void hash_table_init();
void freeTable();
void zobrist_init();
unsigned long zobrist_hash(Position *);
void saveTransposition(Position*, int, int, char);
void saveLower(Position*, int, char);
void saveUpper(Position*, int, char);
Transpotion* retrieveTransposition(Position*);

#endif