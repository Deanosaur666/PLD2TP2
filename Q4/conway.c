#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define MIN(a,b)  ((a)<(b)?(a):(b))
#define MAX(a,b)  ((a)>(b)?(a):(b))

// use half blocks so one row of text is two rows of cells
// this results in a display much closer to a square
#define FULLBLOCK "█"
#define TOPHALF "▀"
#define BOTTOMHALF "▄"

// used for finding left, right, top, and bottom of a subsection
#define PBLOCK_MIN(id, pdivs, size) (id*(size)/pdivs)
#define PBLOCK_MAX(id, pdivs, size) ((id+1)*(size)/pdivs - 1)

// we read one extra row and column in each direction, to find neighbors
#define PBLOCK_READ_MIN(id, pdivs, size) MAX(PBLOCK_MIN(id, pdivs, size) - 1, 0)
#define PBLOCK_READ_MAX(id, pdivs, size) MIN(PBLOCK_MAX(id, pdivs, size) + 1, size - 1)

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

void printGame(char ** board, int size) {
    // print upper frame
    printf("╔");
    for(int i = 0; i < size; i ++) {
        printf("═");
    }
    printf("╗\n");

    for(int row = 0; row < size; row += 2) {
        // print left frame
        printf("║");
        // print cells
        for(int col = 0; col < size; col ++) {
            char top = board[row][col];
            char bottom = 0; 
            if(row+1 < size)
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
    for(int i = 0; i < size; i ++) {
        printf("═");
    }
    printf("╝\n");
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
    double  elapsed_time;   /* Parallel execution time */
    int     id;             /* Process ID number */
    int     p;              /* Number of processes */
    int     j;              /* Number of iterations */
    int     i;
    int     size;           /* Size of the board */
    bool    draw;           /* Drawing enabled */
    int     pcols;          /* Process columns */
    int     prows;          /* Process rows */
    /* Version of the board from the previous frame. Read from. */
    char    *boardStorage;  /* All cells */
    char    **board;        /* All cells, in a matrix*/
    /* Next frame's board. Written to. */
    char    *outBoardStorage; /* The section of the board this process uses */
    char    **outBoard;     /* The section of the board this process uses, in a matrix */

    MPI_Init (&argc, &argv);

    MPI_Comm_rank (MPI_COMM_WORLD, &id);
    MPI_Comm_size (MPI_COMM_WORLD, &p);
    MPI_Barrier(MPI_COMM_WORLD);

    if (argc < 3) {
        if (!id)
            printf ("Command line: %s <size> <iterations> [-D]\n", argv[0]);
        MPI_Finalize();
        exit (1);
    }
    if(argc == 4 && strcmp(argv[3], "-D"))
        draw = false;
    else
        draw = true;

    size = atoi(argv[1]);
    j = atoi(argv[2]);
    
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
        boardStorage = (char *) calloc(size * size, sizeof(char));
        board = arrayToMatrix(boardStorage, size, size);

        // place r-pentomino
        // c is left top corner position
        int c = size/2 - 1;
        board[c][c + 1] = 1;
        board[c][c + 2] = 1;
        board[c + 1][c] = 1;
        board[c + 1][c + 1] = 1;
        board[c + 2][c + 1] = 1;

        printGame(board, size);
    }

    // calculate rows and columns for checkerboard decomposition
    // if p is a square number, we get a nice checkerboard with equal rows and columns
    // otherwise, with 6 processes for example, we may get 3 rows and 2 columns
    // in the worst case, with 1 or a prime number of processes, we get one column, and p rows,
    // and thus just have row-wise block striped decomposition
    pcols = (int) sqrt((double) p);
    while(p % pcols != 0) pcols --;
    prows = p/pcols;


    /* Start the timer */
    elapsed_time = -MPI_Wtime();


    /* Stop the timer */

    elapsed_time += MPI_Wtime();

    /* Print the results */
    if (!id) {
        printf ("T: %10.6f\n", elapsed_time);
    }
    MPI_Finalize ();
    return 0;
}