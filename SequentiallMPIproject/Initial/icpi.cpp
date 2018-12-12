#include <stdio.h>
#include <math.h>
#include "mpi.h"

#define HEAVY 1000
#define SIZE 40
#define RADIUS 5 

// This function performs heavy computations, 
// its run time depends on x and y values
double heavy(int x, int y) {
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

int main(int argc, char *argv[]) {
	double t1, t2;
	int x, y;
	int size = SIZE;
	double answer = 0;

	MPI_Init(&argc, &argv);
	t1 = MPI_Wtime();

	for (x = 0; x < size; x++)
		for (y = 0; y < size; y++)
			answer += heavy(x, y);

	printf("answer = %e\n", answer);

	t2 = MPI_Wtime();
	printf("MPI_Wtime measured a heavy func to be: %1.5f\n", t2 - t1);
	fflush(stdout);

	MPI_Finalize();
}
