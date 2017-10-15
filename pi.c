#include <mpi.h>
#include <math.h>
int main( int argc, char **argv )
{
int n, my_pe_num, numprocs, index;
float mypi, pi, h, x, start, end;
MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD, &numprocs );
MPI_Comm_rank(MPI_COMM_WORLD, &my_pe_num );
if( my_pe_num == 0 ){
printf("How many intervals? ");
scanf("%d", &n);
}
MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
mypi = 0;
h= (float) 2/n; /*Size of each slice*/
start = (my_pe_num*2/numprocs)-1; /*Slices for this PE*/
end = ((my_pe_num+1)*2/numprocs)-1;
for (x = start; x < end; x = x+h)
mypi = mypi + h * 2* sqrt(1-x*x);
MPI_Reduce(&mypi, &pi, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
if( my_pe_num == 0 ){
printf("Pi is approximately %f\n", pi);
printf("Error is %f\n", pi-3.14159265358979323846);
}
MPI_Finalize();
}
