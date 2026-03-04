/*

This program uses the ncurses library
Install ncurses with:
sudo apt install libncurses-dev

Please use the Makefile for building, or append -l and -lncurses as gcc options

*/

#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ncurses.h>

#define MIN(a,b)  ((a)<(b)?(a):(b))
#define MAX(a,b)  ((a)>(b)?(a):(b))

// use half blocks so one row of text is two rows of cells
// this results in a display much closer to a square
#define FULLBLOCK "█"
#define TOPHALF "▀"
#define BOTTOMHALF "▄"

// used for finding left, right, top, and bottom of a subsection
// "size" here is the size of the whole board, not the sub-board
#define PBLOCK_MIN(pos, pdivs, size) (pos*(size)/pdivs)
#define PBLOCK_MAX(pos, pdivs, size) ((pos+1)*(size)/pdivs - 1)

// we read one extra row and column in each direction, to find neighbors
#define PBLOCK_READ_MIN(pos, pdivs, size) MAX(PBLOCK_MIN(pos, pdivs, size) - 1, 0)
#define PBLOCK_READ_MAX(pos, pdivs, size) MIN(PBLOCK_MAX(pos, pdivs, size) + 1, size - 1)

// find a process's row and column position
#define PBLOCK_COL(id, pcols, prows) (id % pcols)
#define PBLOCK_ROW(id, pcols, prows) (id / pcols)

/*
+----+
|▄█▀ |
| ▀  |
+----+

*/


/*

R-pentomino
[ ##]
[## ]
[ # ]

*/

/*
Rule:
B3/S23

* Dead cell becomes alive with 3 live neighbors
* Live cells with 2 or 3 live neighbors survive
* All other cells become dead

A cell has 8 neighbors

*/

void cursorToHome() {
    // Write the sequence for clearing the display:
    // \x1B[2J - Clears the visible window
    // \x1B[3J - Clears the scroll back
    // \x1B[1;1H - Move cursor back to its home coordinates (top left)
    fputs("\x1B[3J\x1B[1;1H", stdout);  
    fflush(stdout);
}

void printGame(char ** board, int w, int h) {
    // print upper frame
    printf("╔");
    for(int i = 0; i < w; i ++) {
        printf("═");
    }
    printf("╗\n");

    for(int row = 0; row < h; row += 2) {
        // print left frame
        printf("║");
        // print cells
        for(int col = 0; col < w; col ++) {
            char top = board[row][col];
            char bottom = 0; 
            if(row+1 < h)
                bottom = board[row+1][col];
            if(top && bottom)
                printf(FULLBLOCK);
            else if(top)
                printf(TOPHALF);
            else if(bottom)
                printf(BOTTOMHALF);
            else
                printf(" ");
        }
        // print right frame
        printf("║\n");
    }

    // print lower frame
    printf("╚");
    for(int i = 0; i < w; i ++) {
        printf("═");
    }
    printf("╝\n");

    fflush(stdout);
}

int cellNeighbors(char ** board, int cols, int rows, int x, int y) {
    int neighbors = 0;
    for(int iy = MAX(y-1, 0); iy <= MIN(y+1, rows-1); iy ++) {
        for(int ix = MAX(x-1, 0); ix <= MIN(x+1, cols-1); ix ++) {
            // skip the cell itself
            if(ix == x && iy == y)
                continue;
            neighbors += board[iy][ix];
        }
    }
    return neighbors;
}

char cellNextState(char ** board, int cols, int rows, int x, int y) {
    int n = cellNeighbors(board, cols, rows, x, y);
    // birth
    if(!board[y][x] && n == 3)
        return 1;
    // sustain
    else if(board[y][x] && (n == 2 || n == 3))
        return 1;
    // die
    else
        return 0;
}

char ** arrayToMatrix(char * array, int w, int h) {
    char ** matrix = (char **) malloc (h * sizeof(char *));
    for(int i = 0; i < h; i ++) {
        matrix[i] = &array[i*w];
    }

    return matrix;
}

// takes a rectangular section of the board
char * subBoard(char ** board, int size, int x, int y, int w, int h) {
    char * sub = (char *)malloc(w * h * sizeof(char));
    for(int iy = 0; iy < h; iy ++) {
        // copy from board row, offset by x, w bytes
        // copy to sub board at position defined by iy
        memcpy(sub + iy*w, board[y + iy] + x, w);
    }
    return sub;
}

// copies a rectangular subsection back into the full board
void joinBoard(char ** board, int size, char * sub, int x, int y, int w, int h) {
    for(int iy = 0; iy < h; iy ++) {
        memcpy(board[y + iy] + x, sub + iy*w, w);
    }
}

int main (int argc, char *argv[])
{
    int    c;              /* character input */
    char    quit;           /* Do I quit or not? */
    double  elapsed_time;   /* Parallel execution time */
    int     id;             /* Process ID number */
    int     p;              /* Number of processes */
    int     j;              /* Number of iterations */
    int     i;
    int     step;           /* Current iteration */
    int     size;           /* Size of the board */
    bool    draw;           /* Drawing enabled */
    bool    pause;          /* Pause between steps */
    bool    redraw;         /* Draw over last frame */
    int     pcols;          /* Process columns */
    int     prows;          /* Process rows */
    /* Full board. */
    char    *boardStorage;  /* All cells */
    char    **board;        /* All cells, in a matrix*/

    char    **sentBoards;    /* all sent sub boards, stored by p0 */
    
    /* Last frame's board. Read from. */
    char    *inBoardStorage; /* The section of the board this process uses */
    char    **inBoard;     /* The section of the board this process uses, in a matrix */

    /* Next frame's board. Written to. */
    char    *outBoardStorage; /* The section of the board this process uses */
    char    **outBoard;     /* The section of the board this process uses, in a matrix */

    /* Received boards, only used by p0 */
    char    **recvBoards;   /* All received sub boards */
    MPI_Request *recv_requests; /* Array of requests */
    MPI_Status  *recv_statuses;   /* Array of statuses */


    // send and recieve requests and status, for Isend and Irecv
    MPI_Request send_request, recv_request;
    MPI_Status send_status, recv_status;

    MPI_Init (&argc, &argv);

    MPI_Comm_rank (MPI_COMM_WORLD, &id);
    MPI_Comm_size (MPI_COMM_WORLD, &p);
    MPI_Barrier(MPI_COMM_WORLD);

    if (argc < 3) {
        if (!id)
            printf ("Command line: %s <size> <iterations> [-D|-r|-p]\n", argv[0]);
        MPI_Finalize();
        exit (1);
    }
    
    draw = true;
    redraw = false;
    pause = false;
    quit = 0;

    for(int i = 3; i < argc; i ++) {
        if(strcmp(argv[i], "-D") == 0)
            draw = false;
        if(strcmp(argv[i], "-r") == 0)
            redraw = true;
        if(strcmp(argv[i], "-p") == 0)
            pause = true;
    }

    size = atoi(argv[1]);
    j = atoi(argv[2]);

    if(j == 0)
        pause = true;

    // Initialize ncurses
    if(redraw || pause) {
        initscr();      // Start curses mode
        clear();
        noecho();       // Disable echoing            
        cbreak();       // Disable line buffering (non-canonical mode)
        keypad(stdscr, true);  // Enable special keys (arrow keys, F1, etc.)
    }
    
    if (size < 3) {
      if (!id) printf ("Size must be at least 3\n");
      MPI_Finalize();
      exit (1);
    }

    if (p > size*size) {
      if (!id) printf ("Too many processors\n");
      MPI_Finalize();
      exit (1);
    }

    // create the main board
    if(!id) {
        // use calloc so it starts empty
        // REMEMBER TO FREE BOARD
        boardStorage = (char *) calloc(size * size, sizeof(char));
        // REMEMBER TO FREE MATRIX
        board = arrayToMatrix(boardStorage, size, size);

        // place r-pentomino
        // c is left top corner position
        int c = size/2 - 1;
        board[c][c + 1] = 1;
        board[c][c + 2] = 1;
        board[c + 1][c] = 1;
        board[c + 1][c + 1] = 1;
        board[c + 2][c + 1] = 1;

        if(draw) {
            if(redraw)
                cursorToHome();
            printGame(board, size, size);
            if(redraw)
                refresh();

            if(pause) {
                c = getch();
                if(c == 'q') {
                    quit = 1;
                }
            }
        }
    }

    // calculate rows and columns for checkerboard decomposition
    // if p is a square number, we get a nice checkerboard with equal rows and columns
    // otherwise, with 6 processes for example, we may get 3 rows and 2 columns
    // in the worst case, with 1 or a prime number of processes, we get one column, and p rows,
    // and thus just have row-wise block striped decomposition
    pcols = (int) sqrt((double) p);
    while(p % pcols != 0) pcols --;
    prows = p/pcols;

    // my sub-board position
    int prow = PBLOCK_ROW(id, pcols, prows);
    int pcol = PBLOCK_COL(id, pcols, prows);
    int x = PBLOCK_MIN(pcol, pcols, size);
    int w = (PBLOCK_MAX(pcol, pcols, size) - x) + 1;
    int y = PBLOCK_MIN(prow, prows, size);
    int h = (PBLOCK_MAX(prow, prows, size) - y) + 1;

    int read_x = PBLOCK_READ_MIN(pcol, pcols, size);
    int read_w = (PBLOCK_READ_MAX(pcol, pcols, size) - read_x) + 1;
    int read_y = PBLOCK_READ_MIN(prow, prows, size);
    int read_h = (PBLOCK_READ_MAX(prow, prows, size) - read_y) + 1;

    int read_x_offset = x - read_x; // 1 or 0
    int read_y_offset = y - read_y; // 1 or 0


    /* Start the timer */
    elapsed_time = -MPI_Wtime();

    step = 0;

    // if j is 0, we loop infinitely
    while(!quit && (j <= 0 || step++ < j)) {
        // get read to recieve my sub-board from split
        if(id > 0) {
            // REMEMBER TO FREE IN BOARD
            inBoardStorage = (char *)malloc(read_w*read_h*sizeof(char));
            MPI_Irecv(inBoardStorage, read_w*read_h, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &recv_request);
        }
        // send and prepare to recieve sub boards to/from everybody
        if(!id) {
            // REMEMBER TO FREE SENT BOARDS
            sentBoards = (char **)malloc(p * sizeof(char *));
            // my sub board
            // REMEMBER TO FREE SUB BOARD
            inBoardStorage = subBoard(board, size, read_x, read_y, read_w, read_h);
            sentBoards[0] = inBoardStorage;

            // ready to recieve
            // REMEMBER TO FREE
            recv_requests = (MPI_Request *)malloc(p * sizeof(MPI_Request));
            recv_statuses = (MPI_Status *)malloc(p * sizeof(MPI_Status));
            recvBoards = (char **)malloc(p * sizeof(char *));
            recvBoards[0] = NULL; // not receiving one from myself
            
            for(i = 1; i < p; i ++) {
                int prow = PBLOCK_ROW(i, pcols, prows);
                int pcol = PBLOCK_COL(i, pcols, prows);
                
                // send

                int read_x = PBLOCK_READ_MIN(pcol, pcols, size);
                int read_w = (PBLOCK_READ_MAX(pcol, pcols, size) - read_x) + 1;
                int read_y = PBLOCK_READ_MIN(prow, prows, size);
                int read_h = (PBLOCK_READ_MAX(prow, prows, size) - read_y) + 1;
                // REMEMBER TO FREE SUB BOARD
                sentBoards[i] = subBoard(board, size, read_x, read_y, read_w, read_h);
                MPI_Isend(sentBoards[i], read_w*read_h, MPI_CHAR, i, 0, MPI_COMM_WORLD, &send_request);

                // recieve
                int x = PBLOCK_MIN(pcol, pcols, size);
                int w = (PBLOCK_MAX(pcol, pcols, size) - x) + 1;
                int y = PBLOCK_MIN(prow, prows, size);
                int h = (PBLOCK_MAX(prow, prows, size) - y) + 1;

                // REMEMBER TO FREE
                recvBoards[i] = malloc(w*h*sizeof(char));

                MPI_Irecv(recvBoards[i], w*h, MPI_CHAR, i, 0, MPI_COMM_WORLD, &recv_requests[i]);
            }
        }
        // wait till I've gotten my board
        if(id > 0) {
            MPI_Wait(&recv_request, &recv_status);
        }
        // REMEMBER TO FREE MATRIX
        inBoard = arrayToMatrix(inBoardStorage, read_w, read_h);

        // REMEMBER TO FREE OUT BOARD
        outBoardStorage = (char *)malloc(w*h*sizeof(char));
        // REMEMBER TO FREE MATRIX
        outBoard = arrayToMatrix(outBoardStorage, w, h);

        // process all cells
        for(int row = 0; row < h; row ++) {
            for(int col = 0; col < w; col ++) {
                outBoard[row][col] = cellNextState(inBoard, read_w, read_h, col + read_x_offset, row + read_y_offset);                
            }
        }

        // in board is not in use anymore
        free(inBoard);
        free(inBoardStorage);

        // send back to process 0
        if(id > 0) {
            MPI_Isend(outBoardStorage, w*h, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &send_request);
            // wait till fully sent
            MPI_Wait(&send_request, &send_status);
            // then FREE
            free(outBoard);
            free(outBoardStorage);
        }

        // process 0 recieving modified boards
        if(!id) {

            joinBoard(board, size, outBoardStorage, x, y, w, h);
            free(outBoard);
            free(outBoardStorage);

            for(i = 1; i < p; i ++) {
                int prow = PBLOCK_ROW(i, pcols, prows);
                int pcol = PBLOCK_COL(i, pcols, prows);
                int x = PBLOCK_MIN(pcol, pcols, size);
                int w = (PBLOCK_MAX(pcol, pcols, size) - x) + 1;
                int y = PBLOCK_MIN(prow, prows, size);
                int h = (PBLOCK_MAX(prow, prows, size) - y) + 1;

                //printf("P: %d\tX: %d\tY: %d\tW: %d\t H: %d\n", i, x, y, w, h);
                //printf("Col: %d/%d\tRow: %d/%d\n", pcol, pcols, prow, prows);
                
                MPI_Wait(&recv_requests[i], &recv_statuses[i]);
                
                joinBoard(board, size, recvBoards[i], x, y, w, h);
                free(recvBoards[i]);

                free(sentBoards[i]);
            }
            free(recvBoards);
            free(sentBoards);

            free(recv_requests);
            free(recv_statuses);

            if(draw) {
                if(redraw)
                    cursorToHome();
                printGame(board, size, size);

                if(pause) {
                    c = getch();
                    if(c == 'q') {
                        quit = 1;
                    }
                }

                if(redraw)
                    refresh();
            }
        }

        // if paused, check for quit
        if(pause)
            MPI_Bcast(&quit, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
        if(quit)
            break;
        
    }


    /* Stop the timer */
    elapsed_time += MPI_Wtime();
    
    refresh();
    endwin();
    fflush(stdout);

    /* Print the results */
    if (!id) {
        printf ("T: %10.6f\n", elapsed_time);
    }
    MPI_Finalize ();

    /* Cleanup */
    if(!id) {
        // FREE BOARD
        free(boardStorage);
        // FREE MATRIX
        free(board);
    }

    return 0;
}
