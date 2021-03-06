/****************************************************************
 * Laplace MPI Template C Version                                         
 *                                                               
 * T is initially 0.0                                            
 * Boundaries are as follows                                     
 *                                                               
 *                T                      4 sub-grids            
 *   0  +-------------------+  0    +-------------------+       
 *      |                   |       |                   |           
 *      |                   |       |-------------------|         
 *      |                   |       |                   |      
 *   T  |                   |  T    |-------------------|             
 *      |                   |       |                   |     
 *      |                   |       |-------------------|            
 *      |                   |       |                   |   
 *   0  +-------------------+ 100   +-------------------+         
 *      0         T       100                                    
 *                                                                 
 * Each PE only has a local subgrid.
 * Each PE works on a sub grid and then sends         
 * its boundaries to neighbors.
 *                                                                 
 *  John Urbanic, PSC 2014
 *
 *******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <mpi.h>

#define COLUMNS      10000
#define ROWS_GLOBAL  10000       // this is a "global" row count
#define NPES           25        // number of processors
//NPES not actually used
#define ROWS (ROWS_GLOBAL/NPES)  // number of real local rows
// communication tags
#define DOWN     100
#define UP       101   

#define MAX_TEMP_ERROR 0.01

double Temperature[ROWS+2][COLUMNS+2];
double Temperature_last[ROWS+2][COLUMNS+2];

void initialize(int npes, int my_PE_num);
void track_progress(int iter);


int main(int argc, char *argv[]) {

    int i, j;
    int max_iterations;
    int iteration=1;
    double dt;
    struct timeval start_time, stop_time, elapsed_time;

    int        npes;                // number of PEs
    int        my_PE_num;           // my PE number
    double     dt_global=100;       // delta t across all PEs
    MPI_Status status;              // status returned by MPI calls

    // the usual MPI startup routines
    MPI_Init(&argc, &argv);

    // verify only NPES PEs are being used
    MPI_Comm_rank(MPI_COMM_WORLD, &my_PE_num);
    MPI_Comm_size(MPI_COMM_WORLD, &npes);

    //if(npes != 4)
    //{
    //   printf("Incorrect number of cores allocated.\n");
    //   exit(1);
    //}

    // PE 0 asks for input
    if(my_PE_num==0)
    {
      printf("How many iterations? [1-4000]\n");
      scanf("%d", &max_iterations);
    
      gettimeofday(&start_time,NULL);
    }

    // bcast max iterations to other PEs
    MPI_Bcast(&max_iterations, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    initialize(npes, my_PE_num);
    

    while ( dt_global > MAX_TEMP_ERROR && iteration <= max_iterations ) {
        
        // main calculation: average my four neighbors
        for(i = 1; i <= ROWS; i++) {
            for(j = 1; j <= COLUMNS; j++) {
                Temperature[i][j] = 0.25 * (Temperature_last[i+1][j] + Temperature_last[i-1][j] +
                                            Temperature_last[i][j+1] + Temperature_last[i][j-1]);
            }
        }
        // COMMUNICATION PHASE: send and receive ghost rows for next iteration
        // 1 sends bottom to 2, 2 sends top to 1, etc

        //send down & receive
        if(my_PE_num == 0)
        {
          MPI_Send(&Temperature[ROWS][1], COLUMNS,MPI_DOUBLE,
            my_PE_num+1, DOWN, MPI_COMM_WORLD);
        }
        else
        {
          MPI_Recv(&Temperature_last[0][1],COLUMNS,MPI_DOUBLE,
            my_PE_num-1, DOWN, MPI_COMM_WORLD, &status);
          if(my_PE_num != npes-1)
            MPI_Send(&Temperature[ROWS][1], COLUMNS, MPI_DOUBLE,
              my_PE_num+1, DOWN, MPI_COMM_WORLD);
        }

        //send up & receive
        if(my_PE_num == npes-1)
        {
          MPI_Send(&Temperature[1][1], COLUMNS, MPI_DOUBLE,
           my_PE_num-1, UP, MPI_COMM_WORLD);
        }
        else
        {
          MPI_Recv(&Temperature_last[ROWS+1][1], COLUMNS, MPI_DOUBLE,
           my_PE_num+1, UP, MPI_COMM_WORLD, &status);
          if(my_PE_num != 0)
            MPI_Send(&Temperature[1][1], COLUMNS, MPI_DOUBLE,
             my_PE_num-1, UP, MPI_COMM_WORLD);
        }
        
        dt = 0.0;

        for(i = 1; i <= ROWS; i++){
            for(j = 1; j <= COLUMNS; j++){
	        dt = fmax( fabs(Temperature[i][j]-Temperature_last[i][j]), dt);
	        Temperature_last[i][j] = Temperature[i][j];
            } 
        }

        MPI_Reduce(&dt, &dt_global, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
   
        // periodically print test values - only for PE in lower corner
        if((iteration % 100) == 0) {
            if (my_PE_num == npes-1){
      //          track_progress(iteration);
	    }
        }
	iteration++;

        MPI_Bcast(&dt_global, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

    // Slightly more accurate timing and cleaner output 
    MPI_Barrier(MPI_COMM_WORLD);

    // PE 0 finish timing and output values
    if (my_PE_num==0){
        gettimeofday(&stop_time,NULL);
	timersub(&stop_time, &start_time, &elapsed_time);

	printf("\nMax error at iteration %d was %f\n", iteration-1, dt_global);
	printf("Total time was %f seconds.\n", elapsed_time.tv_sec+elapsed_time.tv_usec/1000000.0);
    }
    //if(my_PE_num==18)
    //  printf("%f\n",Temperature[30][900]);
    MPI_Finalize();
}



void initialize(int npes, int my_PE_num){

    int i,j;

    for(i = 0; i <= ROWS+1; i++){//rows + 2, including ghost regions/bars
        for (j = 0; j <= COLUMNS+1; j++){ //columns + 2, ghost/bar
            Temperature_last[i][j] = 0.0;
        }
    }

    // Local boundary condition endpoints
    // 0 -> first row all 0s (already done)
    // 3 -> last row scales right to 100
    // all -> right column scales down to 100
    if(my_PE_num == npes-1)
    {
       for(j = 0; j <= COLUMNS+1; j++)
       {
         //bottom linear increase
         Temperature_last[ROWS+1][j] = (100.0/COLUMNS)*j;
       }
    }

    for(i = 0; i <= ROWS+1;i++)
    {
      //right side linear increase
      Temperature_last[i][COLUMNS+1] = 
        ((ROWS*my_PE_num)+i)*(100.0/ROWS_GLOBAL);
      //printf("PE %d: [%d,%d]=%f\n",my_PE_num, i, COLUMNS+1,
      //  Temperature_last[i][COLUMNS+1]);
    }
}


// only called by last PE
void track_progress(int iteration) {

    int i;

    printf("---------- Iteration number: %d ------------\n", iteration);

    // output global coordinates so user doesn't have to understand decomposition
    for(i = 5; i >= 0; i--) {
      printf("[%d,%d]: %5.2f  ", ROWS_GLOBAL-i, COLUMNS-i, Temperature[ROWS-i][COLUMNS-i]);
    }
    printf("\n");
}
