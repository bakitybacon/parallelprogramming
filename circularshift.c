#include <stdio.h>
#include "mpi.h"

int main(int argc, char** argv)
{
   int peNum, totalPEs, i, receive;
   MPI_Status status;

   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &totalPEs);
   MPI_Comm_rank(MPI_COMM_WORLD, &peNum);

   if(peNum != totalPEs - 1) 
     MPI_Send(&peNum, 1, MPI_INT, peNum+1, 10, MPI_COMM_WORLD);
   else MPI_Send(&peNum, 1, MPI_INT, 0, 10, MPI_COMM_WORLD);

   if(peNum != 0)
     MPI_Recv(&receive, 1, MPI_INT, i - 1, 10, MPI_COMM_WORLD, &status);
   else MPI_Recv(&receive, 1, MPI_INT, totalPEs-1, 10, MPI_COMM_WORLD, &status);

   printf("PE %d received value %d\n",peNum,receive);   

   MPI_Finalize();
}
