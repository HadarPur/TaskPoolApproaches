#include "mpi.h"
#include <stdio.h>
#include <math.h>

#define HEAVY 1000
#define SIZE 40
#define RADIUS 5 

// This function performs heavy computations, 
// its run time depends on x and y values
double heavy(int x, int y)
{
	int i, loop;
	double sum = 0;

	if (sqrt((x - SIZE / 2)*(x - SIZE / 2) + (y - SIZE / 2)*(y - SIZE / 2)) < RADIUS)
		loop = x*y + 1;
	else
		loop = y + x;

	for (i = 0; i < loop*HEAVY; i++)
		sum += sin(exp(sin((double)i / HEAVY)));

	return  sum;
}

//Master function
void staticMaster(int numprocs, MPI_Status status)
{
	double answer = 0, temp;
	int i;

	for (i = 0; i < numprocs - 1; i++)
	{
		MPI_Recv(&temp, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
		answer += temp;
	}

	printf("answer = %e\n", answer);
	printf("answer should be = 5.699353e+07\n");
}

//Slave function
void staticSlave(int myid, int numprocs)
{
	double answer = 0;
	int size = SIZE;
	int part = SIZE / (numprocs - 1);
	int x, y;


	for (x = (myid - 1) * part; x < myid * part; x++)
		for (y = 0; y < size; y++)
			answer += heavy(x, y);

	MPI_Send(&answer, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);

}

int main(int argc, char *argv[])
{
	double t1, t2;
	int myid, numprocs, namelen;
	char processor_name[MPI_MAX_PROCESSOR_NAME];

	int errorCode = 999;

	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	MPI_Get_processor_name(processor_name, &namelen);

	MPI_Status status;

	// Check that there is 3 proceeses min, 1 master and 2 slaves
	if (numprocs < 3)
	{
		printf("Static MPI project need minimum 3 proccesses\n");
		MPI_Abort(MPI_COMM_WORLD, errorCode);
	}

	if (myid == 0)
	{
		t1 = MPI_Wtime();
		staticMaster(numprocs, status);
		t2 = MPI_Wtime();
		printf("MPI_Wtime measured a heavy func to be: %1.5f\n", t2 - t1);
	}
	else
	{
		staticSlave(myid, numprocs);
		//printf("Computer name %s\n", processor_name);
	}

	MPI_Finalize();
}


