#include "transposition.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
	long zobrist_table[BOARD_COLUMNS*BOARD_ROWS][2];
	long scores[2][16];
} Zobrist_Init;

Zobrist_Init zobristTable;


void hash_table_init(){

	hashTable = malloc(sizeof(Transpotion)*(MAX_TABLE_SIZE+OPEN_ADDRESSING));
	
	int i;
	for (i=0; i < MAX_TABLE_SIZE+OPEN_ADDRESSING; i++)
		hashTable[i].validity =0;

}

void zobrist_init(){

	int i;

	for (i=0; i < BOARD_COLUMNS*BOARD_ROWS; i++)
	{
		zobristTable.zobrist_table[i][0] = ((((long) rand() <<  0)) | (((long) rand() << 32)));
		zobristTable.zobrist_table[i][1] = ((((long) rand() <<  0)) | (((long) rand() << 32)));
	}
	for(i = 0; i< 16; i++)
	{
		zobristTable.scores[0][i] = ((((long) rand() <<  0)) | (((long) rand() << 32)));
		zobristTable.scores[1][i] = ((((long) rand() <<  0)) | (((long) rand() << 32)));
	}
}

unsigned int simpleHash(long zobrist){

	unsigned int hash = zobrist;

	hash = hash % MAX_TABLE_SIZE;
	return hash;
}
unsigned long zobrist_hash(Position* pos){
	int i,j;
	unsigned long h = 0;
	for (i= 0; i < BOARD_ROWS; i++)
		for (j=0; j < BOARD_COLUMNS; j++)
			if ( pos->board[i][j] == BLACK || pos->board[i][j] == WHITE )
				h ^= zobristTable.zobrist_table[i*BOARD_COLUMNS + j][(unsigned)pos->board[i][j]];
		
	h ^= zobristTable.scores[0][(unsigned)pos->score[0]];
	h ^= zobristTable.scores[1][(unsigned)pos->score[1]];

	return h;
}
unsigned int hash_func(Position* pos){
	unsigned long key = zobrist_hash(pos);
	unsigned int hash = simpleHash(key);
	
	int i=0;
	while ((hashTable[hash].validity & 0x1)&&(hashTable[hash].zobrist_key != key)&&(i<OPEN_ADDRESSING))
	{
			i++;
			hash++;
	}
	hashTable[hash].zobrist_key = key;
	return hash;
}

void saveTransposition(Position* aPos, int upperBound, int lowerBound, char depth){

	unsigned int hash = hash_func(aPos);
	
	if(((hashTable[hash].validity = 0x7) && (hashTable[hash].upperDepth + hashTable[hash].lowerDepth>=2*depth ))&&(rand() < RAND_MAX/2))
		return;

	hashTable[hash].lowerBound = lowerBound;
	hashTable[hash].upperBound = upperBound;
	hashTable[hash].validity = 0x7;
	hashTable[hash].upperDepth = depth;
	hashTable[hash].lowerDepth = depth;
	
	return;

}


void saveUpper(Position* aPos, int upperBound,  char depth){
	unsigned long key = zobrist_hash(aPos);
	unsigned int hash = simpleHash(key);
	int i=0;
	
	while ((hashTable[hash].validity & 0x1)&&( hashTable[ hash].zobrist_key != key)&&(i<OPEN_ADDRESSING))
	{
		i++;
		hash++;
	}
	
	if((hashTable[hash].validity & 0x1)&&( hashTable[ hash].zobrist_key == key))
	{

		if(((hashTable[hash].validity & 0x4) && (hashTable[hash].upperDepth >= depth))&&(rand() > RAND_MAX/2))
			return;
		hashTable[hash].upperBound = upperBound;
		hashTable[hash].upperDepth = depth;
		hashTable[hash].validity = hashTable[hash].validity | 0x4;
		return;

	}
	
	
	hashTable[hash].upperBound = upperBound;
	hashTable[hash].zobrist_key = key;
	hashTable[hash].validity = 0x5;
	hashTable[hash].upperDepth = depth;
	

}
void saveLower(Position* aPos, int lowerBound, char depth){
	unsigned long key = zobrist_hash(aPos);
	unsigned int hash = simpleHash(key);
	int i = 0;

	while ((hashTable[hash].validity & 0x1)&&( hashTable[hash].zobrist_key != key)&&(i<OPEN_ADDRESSING))
	{
		i++;
		hash++;
	}
	

	if(( hashTable[hash].validity & 0x1)&&( hashTable[hash].zobrist_key == key))
	{
		if(( hashTable[hash].validity & 0x2) && ( hashTable[hash].lowerDepth >= depth)&&(rand() > RAND_MAX/2))
			return;
		 hashTable[hash].lowerBound = lowerBound;
		 hashTable[hash].lowerDepth = depth;
		 hashTable[hash].validity =  hashTable[hash].validity | 0x2;
		return;

	}

	 hashTable[hash].zobrist_key = key;
	 hashTable[hash].lowerBound = lowerBound;
	 hashTable[hash].lowerDepth = depth;
	 hashTable[hash].validity = 0x3;
	return;

}
Transpotion* retrieveTransposition(Position* pos){

	unsigned long zobrist = zobrist_hash(pos);
	unsigned int hash = simpleHash(zobrist);

	int i =0;

	while(( hashTable[hash].validity & 0x1)&&(i < OPEN_ADDRESSING))
	{	

		if((hashTable[hash].zobrist_key == zobrist))
			return &hashTable[hash];
		i++;
		hash++;
	}
	return NULL;


}


void freeTable(){
	free(hashTable);
}
