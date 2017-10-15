#include <stdio.h>
#include "mpi.h"

int main(int argc, char** argv)
{
  int peNum, i, receive, send;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &peNum);
  MPI_Barrier(MPI_COMM_WORLD);
  for(i = 200; i > 0; i--)
  {
    printf("hi im %d\n",peNum);
    if(i == peNum)
    {
      MPI_Send(&peNum, 1, MPI_INT, 0, 42, MPI_COMM_WORLD);
    }
  }
 
  if(peNum == 0)
  {
    int max = 0;
    for(i = 200; i > 0; i--)
    {
      MPI_Recv(&receive, 1, MPI_INT, MPI_ANY_SOURCE, 42, MPI_COMM_WORLD, &status);
      if(receive > max)
        max = receive;
    }
    printf("I am running on %d threads.\n",max);
  }

  MPI_Finalize();
}
