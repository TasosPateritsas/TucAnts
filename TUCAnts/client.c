#include "global.h"
#include "board.h"
#include "move.h"
#include "comm.h"
#include "list.h"
#include "transposition.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h> 
#include <sys/time.h>


#define INF 999999999
#define MAX_TIME 10
/**********************************************************/
Position gamePosition;		// Position we are going to use

Move myMove;				// move to save our choice and send it to the server

unsigned char myColor;				// to store our color
int mySocket;				// our socket

char * agentName = "TopBoy";		//default name.. change it! keep in mind MAX_NAME_LENGTH

char * ip = "127.0.0.1";	// default ip (local machine)
/**********************************************************/


int iterative_deepening(Position*,Move*);
int evaluate(Position*);
int canMoveFrom( char row, char col, char player, Position * pos ) ;
int main( int argc, char ** argv )
{
	int c;
	opterr = 0;

	while( ( c = getopt ( argc, argv, "i:p:h" ) ) != -1 )
		switch( c )
		{
			case 'h':
				printf( "[-i ip] [-p port]\n" );
				return 0;
			case 'i':
				ip = optarg;
				break;
			case 'p':
				port = optarg;
				break;
			case '?':
				if( optopt == 'i' || optopt == 'p' )
					printf( "Option -%c requires an argument.\n", ( char ) optopt );
				else if( isprint( optopt ) )
					printf( "Unknown option -%c\n", ( char ) optopt );
				else
					printf( "Unknown option character -%c\n", ( char ) optopt );
				return 1;
			default:
			return 1;
		}



	connectToTarget( port, ip, &mySocket );
	zobrist_init();
	hash_table_init();

	char msg;

/**********************************************************/
// used in random
	srand( time( NULL ) );
	
/**********************************************************/

	while( 1 )
	{

		msg = recvMsg( mySocket );

		switch ( msg )
		{
			case NM_REQUEST_NAME:		//server asks for our name
				sendName( agentName, mySocket );
				break;

			case NM_NEW_POSITION:		//server is trying to send us a new position
				getPosition( &gamePosition, mySocket );
				printPosition( &gamePosition );
				break;

			case NM_COLOR_W:			//server indorms us that we have WHITE color
				myColor = WHITE;
				printf("My color is %d\n",myColor);
				break;

			case NM_COLOR_B:			//server indorms us that we have BLACK color
				myColor = BLACK;
				printf("My color is %d\n",myColor);
				break;

			
			case NM_REQUEST_MOVE:		//server requests our move
				myMove.color = myColor;


				if( !canMove( &gamePosition, myColor ) )
				{
					myMove.tile[ 0 ][ 0 ] = -1;		//null move
				}
				else
				{
					Move* move = malloc(sizeof(Move));
					Position* pos = &gamePosition;
					Position* temp = malloc(sizeof(Position));
					memcpy(temp, pos, sizeof(Position));
					iterative_deepening(temp, move);
					free(temp);
					myMove = *move;
					free(move);
				}

				sendMove( &myMove, mySocket );			//send our move
				doMove( &gamePosition, &myMove );		
				printPosition( &gamePosition );

				break;

			case NM_QUIT:			//server wants us to quit...we shall obey
				close( mySocket );
				return 0;
		}

	} 
	freeTable();
	return 0;
}


void findJump(list* moves, Move* move, int d ,char i, char j, Position* pos){
	
	int jumps =0;
	int playerDirection =0;
	
	char color = move->color;
	move->tile[0][d] = i;
	move->tile[1][d] = j;

	if( color == WHITE )		
		playerDirection = 1;
	else
		playerDirection = -1;

	jumps = canJump(i,j,color,pos);

	if(!jumps){
		move->tile[0][d+1] = -1;

		if(isLegal(pos,move))
			push(moves,move);
		else
			free(move);
		return;
	}

	//left
	if(jumps == 1)
		findJump(moves,move,d+1,i+2*playerDirection,j-2,pos);
	else if (jumps == 2) // rigth
		findJump(moves,move,d+1,i+2*playerDirection,j+2,pos);
	else {// both
		Move* move2 = malloc(sizeof(Move));
		memcpy(move2,move,sizeof(Move));
		findJump(moves,move,d+1,i+2*playerDirection,j-2,pos);
		findJump(moves,move2,d+1,i+2*playerDirection,j+2,pos);
	}
}



list* generate_moves(Position *pos) {
	int i, j, jumpPossible = FALSE;


	list* moves = malloc(sizeof(list));
	initList(moves);
	char color = pos->turn;
	int playerDirection;
	if( color == WHITE )		
		playerDirection = 1;
	else
		playerDirection = -1;
	Move *move;
	for(i = 0; i< BOARD_ROWS; i++ ){
		for(j = 0;j < BOARD_COLUMNS; j++){
				if( pos->board[ i ][ j ] == color ){
				
					if(canJump(i,j,color,pos) ){
						if(!jumpPossible){
							emptyList(moves); 
						}
						jumpPossible = TRUE;
						move = malloc(sizeof(Move));
						move->color = color;
						findJump(moves, move, 0, i, j, pos); 
						
					}
					int move_poss = canMoveFrom ( i, j, color, pos);
					if((jumpPossible == FALSE) && move_poss)
					{
						if(move_poss % 2 == 1) //left move possible
						{	
							move = malloc(sizeof(Move));		
							move->tile[0][0] = i;
							move->tile[1][0] = j;
							move->tile[0][1] = i + playerDirection;
							move->tile[1][1] = j-1;
							move->tile[0][2] = -1;
							move->color = pos->turn;
							if(isLegal(pos, move)){
								push(moves, move);}
							else
								free(move);
						}
						if(move_poss > 1){
							move = malloc(sizeof(Move));	
							move->tile[0][0] = i;
							move->tile[1][0] = j;
							move->tile[0][1] = i + playerDirection;
							move->tile[1][1] = j+1;
							move->tile[0][2] = -1;
							move->color = pos->turn;
							if(isLegal(pos, move)){
								push(moves, move);}
							else
								free(move);
						}

					}

				}
			}
	}
	// not moves
	if(top(moves)==NULL){ 
		move = malloc(sizeof(Move));
		move->tile[0][0] = -1;
		move->color = color;
		
		push(moves, move);
		return moves;
	}
	return moves;
						
}

int minmax(int a , int b, bool min){

	if(min){
		if(a<b)
			return a;
		return b;
	}else{
		if(a>b)
			return a ;
		return b;
	}
}
int quiescence_search(Position* pos){
		int i, j;
		
		for( i = 0; i < BOARD_ROWS; i++)
			for( j = 0; j < BOARD_COLUMNS;j++)
				if( pos->board[i][j] == pos->turn)
					if( canJump(i,j,pos->turn,pos))
						return TRUE;
						
		return FALSE;

}
int evaluate (Position *pos) {
	int eval = 0;

	for (int i = 0; i < BOARD_ROWS; ++i)
	{
		for (int j = 0; j < BOARD_COLUMNS; ++j)
		{	
			//weigths
			int w =1 ;
			if(i < 3 ) w = 1;
			else if(i>=3 && i < 7 ) w=2;
			else w = 3;

			if(pos->board[i][j] == myColor){
				eval+=100;

				eval = (myColor == WHITE) ? eval+i*w : eval + (BOARD_ROWS-i-1)*w ;

			}else if ( pos->board[i][j] == getOtherSide(myColor)){
				eval-=100;

				eval = (myColor == BLACK) ? eval-i*w : eval - (BOARD_ROWS-i-1)*w ;
			}
		}
	}

	return eval+(pos->score[myColor] - pos->score[getOtherSide(myColor)])*85 ;
}




int alpha_beta(Position *pos, char depth, int alpha, int beta, char player, Move* move){  
	
	Transpotion* curr= retrieveTransposition(pos);

	if((curr!= NULL)&&(move == NULL)){

		if((curr->validity & 0x2)&&(curr->lowerBound >= beta))
			if(curr->lowerDepth >= depth)
				return curr->lowerBound;

		if((curr->validity & 0x2))
			if(curr->lowerDepth >= depth)
				alpha = minmax(alpha, curr->lowerBound,FALSE); //max
	
		if((curr->validity & 0x4)&&(curr->upperBound <= alpha))
			if(curr->upperDepth >= depth)
				return curr->upperBound;
		
		if((curr->validity & 0x4))
			if(curr->upperDepth >= depth)
				beta = minmax(beta, curr->upperBound,TRUE); //min
	}
	

	if (depth <= 0) 
		if(!quiescence_search(pos)) 
			return evaluate(pos); 
		
		
	list *moves = generate_moves(pos);   
	Move * temp_data = NULL;


	if (top(moves) == NULL){     
		freeList(moves);
		return evaluate(pos);
	}

	
	int temp, g;
	Position* temp_pos = malloc(sizeof(Position));

	if (player){

		g = -INF;
		int a = alpha;
		while((g<beta)&&((temp_data = pop(moves)) != NULL)){ 

			memcpy(temp_pos, pos, sizeof(Position));
			doMove(temp_pos, temp_data);

			temp = alpha_beta(temp_pos, depth-1, a, beta, 0, NULL);

			if(g < temp){
				g = temp;
				if(move != NULL)
					memcpy(move, temp_data, sizeof(Move));	
			}
			a = minmax(a, g,FALSE);
	
			free(temp_data);
		}
	
	}else{
		g = INF;
		int b = beta;
		while((g>alpha)&&(temp_data = pop(moves)) != NULL){ 

			memcpy(temp_pos, pos, sizeof(Position));
			doMove(temp_pos, temp_data);

			temp = alpha_beta(temp_pos, depth-1, alpha, b, 1, NULL);
			
			g = minmax(g, temp,TRUE);
			b = minmax(b, g,TRUE);

			free(temp_data);
		}
		
	
	}

	freeList(moves);
	free(temp_pos);
	
	if(g <= alpha)
		saveUpper(pos, g, depth);
	if(g > alpha && g < beta)
		saveTransposition(pos, g, g, depth);
	if (g>= beta)
		saveLower(pos, g, depth);
	

	return g;
}



int MTDF(Position* pos, int f, char d, Move* move)
{
	Move* m = malloc(sizeof(Move));
	
	int g = f;
	int upper = INF;
	int lower = - INF;

	while (lower < upper){
		
		int b  = (g == lower)? g+1:g;
		g = alpha_beta(pos, d, b-1, b, 1, m);
		if (g < b)
			upper = g;
		else{
			memcpy(move, m, sizeof(Move));
			lower = g;
		}
	}
	free(m);
	return g;


}

int iterative_deepening(Position* pos, Move* move)
{
	int f = evaluate(pos); 
	char d=5; //~middle
	clock_t start = clock();
	while(1)
	{
		f = MTDF(pos, f, d, move);
		clock_t tmp  = (clock() - start)/CLOCKS_PER_SEC;
		printf("Time used: %ld for d:%d\n", tmp,d); 
		if((tmp >= MAX_TIME)||(d >=11)){ // max time = 7 and max depth 20 . Auto allazei
			printf("Max Score: %d\n", f);
			printf("Time used: %ld\n", tmp);
			printf("Depth of iteration: %d\n", d);
			break;
		}
		d +=1;
	}
	return f;
}



int canMoveFrom( char row, char col, char player, Position * pos ) 
{
	int rval = 0;
	assert( ( player ==  WHITE ) || ( player == BLACK ) );
	
	if(player == WHITE)
	{	
		if( row + 1 < BOARD_ROWS)
		{
			if(col-1>=0)
				if( (pos->board[ row + 1][ col - 1] == EMPTY)  || ( pos->board[row+1][col-1] == RTILE))
					rval +=  1;
			if(col+1<BOARD_COLUMNS)
				if( (pos->board[ row + 1][col+1] == EMPTY)  || (pos->board[row+1][col+1] == RTILE))
					rval += 2;	

		}
	}
	else
	{
		if(row-1>=0)
		{
			if(col-1>=0)
				if( (pos->board[row - 1][col-1] == EMPTY) ||(pos->board[ row - 1][ col - 1] == RTILE))
					rval += 1;	
			if(col+1<BOARD_COLUMNS)
				if( (pos->board[ row - 1][ col + 1] == EMPTY) || (pos->board[ row - 1][ col + 1] == RTILE)) 
					rval += 2;	
		}
	}

	return rval;

}

