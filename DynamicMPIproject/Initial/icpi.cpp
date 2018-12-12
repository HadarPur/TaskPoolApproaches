#include <stdio.h>
#include <math.h>
#include "mpi.h"

#define HEAVY 1000
#define SIZE 40
#define RADIUS 5 
#define TRANSFER_TAG 0
#define TERMINATION_TAG 1


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
void dynamicMaster(int numprocs, MPI_Status status)
{
	double answer = 0, temp;
	int i, x = 0, y = 0;
	int size = SIZE;
	int coordinate[2] = {x, y};

	for (i = 1; i < numprocs; i++)
	{
		// set new coordinates for slaves 
		coordinate[0] = x;
		coordinate[1] = y;

		MPI_Send(coordinate, 2, MPI_INT, i, TRANSFER_TAG, MPI_COMM_WORLD);
		y++;
	}

	for (x = 0 ; x < SIZE; x++)
	{
		for (; y < SIZE; y++)
		{
			// recieve the temp result from slaves 
			MPI_Recv(&temp, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			answer += temp;

			// set new coordinates for slaves 
			coordinate[0] = x;
			coordinate[1] = y;

			MPI_Send(coordinate, 2, MPI_INT, status.MPI_SOURCE, TRANSFER_TAG, MPI_COMM_WORLD);
		}
		y = 0;
	}

	for (i = 1; i < numprocs; i++)
	{
		// recieve the temp result from slaves 
		MPI_Recv(&temp, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		answer += temp;

		// send termination tag to the slaves
		MPI_Send(coordinate, 2, MPI_INT, status.MPI_SOURCE, TERMINATION_TAG, MPI_COMM_WORLD);
	}

	printf("answer = %e\n", answer);
	printf("answer should be = 5.699353e+07\n");
	fflush(stdout);
}

//Slave function
void dynamicSlave(int myid, int numprocs, MPI_Status status)
{
	int coordinate[2];
	double temp;

	status.MPI_TAG = TRANSFER_TAG;

	MPI_Recv(coordinate, 2, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

	while (status.MPI_TAG != TERMINATION_TAG)
	{
		temp = heavy(coordinate[0], coordinate[1]);
		MPI_Send(&temp, 1, MPI_DOUBLE, 0, status.MPI_TAG, MPI_COMM_WORLD);
		MPI_Recv(&coordinate, 2, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	}
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
		printf("Dynamic MPI project need minimum 3 proccesses\n");
		MPI_Abort(MPI_COMM_WORLD, errorCode);
	}

	if (myid == 0)
	{
		t1 = MPI_Wtime();
		dynamicMaster(numprocs, status);
		t2 = MPI_Wtime();
		printf("MPI_Wtime measured a heavy func to be: %1.5f\n", t2 - t1);
	}
	else
	{
		dynamicSlave(myid, numprocs, status);
	//	printf("Computer name %s\n", processor_name);
	}

	MPI_Finalize();
}


