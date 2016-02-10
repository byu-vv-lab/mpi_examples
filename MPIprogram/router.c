#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "mpi.h"

#define SIZE 10
#define PORTNUM 1000
#define NDIST 1.0
#define COLINDEX(a,b) (a==b)


typedef struct{
	int src;
	double dest[SIZE];
}DistVect;


int getNextNode(int i){
	if(i + 1 >= SIZE){
		return 0;
	}
	return i + 1;
	
}

int getPrevNode(int i){
	if(i - 1 < 0){
		return SIZE - 1;
	}
	return i - 1;
	
}


int main(int argc, char *argv[]){
	int numprocs;
	int nodenum;
	MPI_Status status;
	MPI_Request request;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &nodenum);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

	int nextNode = (nodenum+1)%numprocs;
	int prevNode;
	if(nodenum == 0)
	prevNode = numprocs-1;
	else prevNode = nodenum-1;
	//
	double distTable[2][SIZE];
	DistVect out;
	double distVector[SIZE];
	int i;
	for(i = 0; i < SIZE; i++){
		distTable[0][i] = INFINITY;
		distTable[1][i] = INFINITY;
		out.dest[i] = INFINITY;	
	}
	//distTable[1][nodenum] = 0;
	out.src = nodenum;
	out.dest[nodenum] = 0;
	out.dest[nextNode] = 1;
	out.dest[prevNode] = 1;
	//int count  = 0;
	
	int updated = 1;
	size_t size;
	DistVect in;
	

	int count = 0;
	while(count < 4){
		printf("[%d]count:%d\n",nodenum,count);
		if(updated){
			MPI_Send(&out, 1, MPI_BYTE, prevNode, 1, MPI_COMM_WORLD);
			MPI_Send(&out, 1, MPI_BYTE, nextNode, 1, MPI_COMM_WORLD);
			updated = 0;

			printf("Node %d updated\n", nodenum);
		}
		
		MPI_Irecv(&in, 10000, MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
		int timeout = 0;
		MPI_Wait(&request, &status);
		
printf("[%d]recving complete\n", nodenum);
		memcpy(&distTable[COLINDEX(in.src, nextNode)], &in.dest, sizeof(in.dest));

		//col = COLINDEX(in.src, nextNode);
		for(i = 0; i < SIZE; i++){
			if(i != nodenum){
				int col = (distTable[0][i] < distTable[1][i]) ? 0 : 1;
				if(out.dest[i] != distTable[col][i] + NDIST){
					out.dest[i] = distTable[col][i] + NDIST;
					updated = 1;
				}
				
			}
		}
		
printf("[%d]countcomplete:%d\n",nodenum,count);
		count++;
	}
	MPI_Finalize();
	
}

