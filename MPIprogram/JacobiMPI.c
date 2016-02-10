#include <stdio.h>
#include <mpi.h>
#include <sys/time.h>

#define MAXERROR	0.01
#define BIGERROR	9999999
#define MAXSTEPS 	400
#define TOTROWS		5768

#define ERRORSTEP	5

#define POINTROW	200
#define POINTCOL	500
#define FIXEDROW	400
#define FIXEDBEG	0
#define FIXEDEND	330

#define MIN(x,y) (((x) > (y))? (y): (x))
#define MAX(x,y) (((x) > (y))? (x): (y))


int iproc, nproc;

double When();

void SetPresetLocations(float *plate, int nrows, int ncols)
{
	int i, j;
	int min, max;

	/* Left and right edges should be 0 degrees */
	for (i = 0; i < nrows; i++)
	{
		plate[i * ncols + 0] = 0;
		plate[i * ncols + (ncols - 1)] = 0;
	}

	/* Top of plate should be 0 degrees */
	if (iproc == 0)
	{
		for (i = 0; i < ncols; i++)
			plate[i] = 0;
	}

	/* Bottom of plate should be 100 degrees */
	if (iproc == nproc - 1)
	{
		for (j = 0; j < ncols; j++)
			plate[(nrows - 1) * ncols + j] = 0;
	}

	/* Now set the fixed point */
	min = nrows * iproc;
	max = min + nrows;
	if ( (POINTROW >= min) && (POINTROW <= max) )
	{
		plate[(POINTROW - min) * ncols + POINTCOL] = 100;
	}

	/* Now set the fixed row */
	if ( (FIXEDROW >= min) && (FIXEDROW <= max) )
	{
		for (j = FIXEDBEG; j < FIXEDEND; j++)
			plate[ (FIXEDROW - min) * ncols + j] = 100;
	}
}

void main(int argc, char *argv[])
{
	float *iplate;
	float *oplate;
	float *tmp;
	int unbor, dnbor;
	int i, j, errori, errorj;
	int nrows, ncols;
	int steps;
	MPI_Status status;
	MPI_Request request1, request2, request3, request4;
	double t0, tottime;
	float maxerror, error, overallerror;
	int begin, end, last;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &iproc);
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);

	ncols = TOTROWS;
	nrows = ncols/nproc;

	iplate = (float *)malloc((nrows+2) * ncols * sizeof(float));
	oplate = (float *)malloc((nrows+2) * ncols * sizeof(float));

	/* Determine my up and down neighbors */
	unbor = (iproc + nproc - 1) % nproc;
	dnbor = (iproc + 1) % nproc;
	if (iproc == 0)
		unbor = -1;
	if (iproc == nproc - 1)
		dnbor = -1;

	/* Initialize the array */
	for (i = 0; i < nrows + 2; i++)
	{
		for (j = 0; j < ncols; j++)
		{
			/*
			iplate[i*ncols+j] = rand();
			*/
			iplate[i*ncols+j] = 50;
			oplate[i*ncols+j] = 50;
		}
        }

	/* Now set the preset fixed locations */
	SetPresetLocations(iplate, nrows, ncols);
	SetPresetLocations(oplate, nrows, ncols);

	/* Now proceed with the Jacobi algorithm */
	t0 = When();
	t0 = When();
	t0 = When();
	overallerror = BIGERROR;
	begin = (iproc == 0)? 2 : 1;
	end =   (iproc == (nproc - 1))? nrows : nrows-1;
	last = end - 1;
	for (steps = 0; steps < MAXSTEPS && overallerror > MAXERROR; steps++)
	{
	
		/* Exchange boundary conditions with my neighbors */
		if (unbor != -1)
		{
			MPI_Isend(&iplate[1*ncols], ncols, MPI_FLOAT, unbor, 1, 
								MPI_COMM_WORLD, &request1);
#ifdef IRECV
			MPI_Irecv(&iplate[0], ncols, MPI_FLOAT, unbor, 1, 
								MPI_COMM_WORLD, &request3);
#else
			MPI_Recv(&iplate[0], ncols, MPI_FLOAT, unbor, 1, 
								MPI_COMM_WORLD, &status);
#endif
		}

		if (dnbor != -1)
		{
			MPI_Isend(&iplate[(nrows-2) * ncols], ncols, MPI_FLOAT, dnbor, 1, 
			                          MPI_COMM_WORLD, &request2);
#ifdef IRECV
			MPI_Irecv(&iplate[(nrows-1) * ncols], ncols, MPI_FLOAT, dnbor, 1, 
			                          MPI_COMM_WORLD, &request4);
#else
			MPI_Recv(&iplate[(nrows-1) * ncols], ncols, MPI_FLOAT, dnbor, 1, 
			                          MPI_COMM_WORLD, &status);
#endif
		}

		/* Compute the 5 point stencil for the inner region of my region */
		for (i = begin + 1; i < end - 1; i++)
		{
			for (j = 1; j < ncols-1; j++)
			{
				oplate[i*ncols + j] = (4 * iplate[i*ncols + j] +
				                    		iplate[(i-1)*ncols + j] +
							        iplate[(i+1)*ncols + j] +
							        iplate[i*ncols + j-1] + 
							        iplate[i*ncols + j+1])/8.0;
			}
		}

		/* Wait for the messages to arrive */

		/* Now compute the 5 point stencil for the boundaries of my region */
#ifdef IRECV
		if (unbor != -1)
		{
			MPI_Wait(&request3, &status);
		}
#endif
		for (j = 1; j < ncols-1; j++)
		{
			oplate[begin*ncols + j] = (4 * iplate[begin*ncols + j] +
							iplate[(begin-1)*ncols + j] +
							iplate[(begin+1)*ncols + j] +
							iplate[begin*ncols + j-1] + 
							iplate[begin*ncols + j+1])/8.0;
		}
#ifdef IRECV
		if (dnbor != -1)
		{
			MPI_Wait(&request4, &status);
		}
#endif
		for (j = 1; j < ncols-1; j++)
		{
			oplate[last*ncols + j] = (4 * iplate[last*ncols + j] +
							iplate[(last-1)*ncols + j] +
							iplate[(last+1)*ncols + j] +
							iplate[last*ncols + j-1] + 
							iplate[last*ncols + j+1])/8.0;
		}

		/* Now swap the new value pointer with the old value pointer */
		tmp = oplate;
		oplate = iplate;
		iplate = tmp;
		SetPresetLocations(iplate, nrows, ncols);

		/* Check error. It is the max error */

		if (steps % ERRORSTEP == 0)
		{
			maxerror = 0;
			for (i = begin; i < end; i++)
			{
				/* Don't need to check edges */
				for (j = 1; j < ncols-1; j++)
				{
					/* Don't check fixed point or fixed row */
					if ((iproc * nrows + i == POINTROW) && (j == POINTCOL))
						continue;
					if ((iproc * nrows + i == FIXEDROW) && (j >= FIXEDBEG) && (j <= FIXEDEND))
						continue;
					error = iplate[i * ncols + j] - (iplate[(i-1)*ncols + j] +
									 iplate[(i+1)*ncols + j] +
									 iplate[i*ncols + j-1] + 
									 iplate[i*ncols + j+1])/4.0;
					if (error > maxerror)
					{
						errori = i;
						errorj = j;
					}
					maxerror = MAX(maxerror, error);
				}
			}
			MPI_Allreduce(&maxerror, &overallerror, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);
			/*
			if (iproc == 0)
			{
				printf("%d: %f error", steps, overallerror);
				printf(" maxerror %f -- [%d, %d]\n", maxerror, errori, errorj);
			}
			*/
		}

	}
	tottime = When() - t0;
	if (iproc == 0)
		printf("%d processors - %d Iterations and Total Time is: %lf sec.\n", nproc, steps, tottime);
	MPI_Finalize();
}
				

/* Return the current time in seconds, using a double precision number.       */
double When()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return ((double) tp.tv_sec + (double) tp.tv_usec * 1e-6);
}

