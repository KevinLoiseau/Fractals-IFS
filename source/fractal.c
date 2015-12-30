#include <cairo.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "../header/geometry.h"
#include "../header/utils.h"
#define Malloc(type) (type *) malloc(sizeof(type))
#define MallocTab(type,size) (type *) malloc(sizeof(type)*size)

double (*w)[6] = (double [][6]) {{0.5, -0.5, 0.5, 0.5, 0.0, 0.0}, {0.5, 0.5, -0.5, 0.5, 0.0, 0.0}}; //Levy Dragon 
/*double (*w)[6] = (double [][6]) {{0.382, 0, 0, 0.385, 0.3072, 0.619},
				 {0.382, 0, 0, 0.385, 0.6033, 0.4044},
				 {0.382, 0, 0, 0.385, 0.0139, 0.4044},
				 {0.382, 0, 0, 0.385, 0.1253, 0.0595},
				 {0.382, 0, 0, 0.385, 0.492, 0.0595}}; //Crystal*/
/*double (*w)[6] = (double [][6]) {{0, -0.5, 0.5, 0, 0.5, 0},
				 {0, 0.5, -0.5, 0, 0.5, 0.5},
				 {0.5, 0, 0, 0.5, 0.25, 0.5}}; //Twin Christmas tree */

int nbFonctions = 2;
int nbIteration = 3;
int main (int argc, char *argv[])
{
	int i,j;
	int nbObjetACalculer = 0, complementACalculer = 0, resteACalculer = 0;
	int numprocessors, rank;
	int nbObjectGrphTot = power(nbFonctions, nbIteration);
	segment* s0;
	segment** result;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocessors);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	 
	nbObjetACalculer = nbObjectGrphTot/numprocessors;
	resteACalculer = nbObjectGrphTot - (nbObjetACalculer*numprocessors);
	

	
	s0 = createSegment(0.0, 1.0, 0.0, 0.0);
	if ( rank == 0 )
	{
		printf("Nombre d'objets graphique à calculer : %d\n", nbObjectGrphTot);
		printf("Reste à repartir : %d\n", resteACalculer);
		printf("a(%f,%f) b(%f,%f)\n", s0->a->x, s0->a->y, s0->b->x, s0->b->y);
		for(i = 0 ; i < nbFonctions; i++) {
			printf("f%d(a:%f ,b:%f, c:%f, d:%f, e:%f, f:%f\n",i, w[i][0], w[i][1], w[i][2], w[i][3], w[i][4], w[i][5]);
		}
    	}
	if(resteACalculer > rank) {
		complementACalculer = 1;
	}
	result = MallocTab(segment*, nbObjetACalculer+complementACalculer);
	for(i = 0 ; i<nbObjetACalculer+complementACalculer ; i++) {
		result[i] = createSegment(0.0, 1.0, 0.0, 0.0);
	}
	printf("p%d => Nombre d'objet graphique à calculer : %d\n", rank, nbObjetACalculer+complementACalculer); 
	int iStartOffset = min(rank,resteACalculer);
	int iEndOffset = min(rank+1,resteACalculer);
	int iStart = nbObjetACalculer*rank + iStartOffset;
	int iEnd = nbObjetACalculer*(rank+1) + iEndOffset;
	int indice;
	for(i = iStart ; i<iEnd; i++) {
		printf("============\n");
		for(j = 0; j < nbIteration; j++) {
			indice = (i/power(nbFonctions,nbIteration-1-j)) % nbFonctions;
			printf("p%d objet %d iteration %d: fonctions applique %d\n",rank, i+1,j,indice+1);
		}
		printf("============\n");
	}
	
	printf("##############\n");
	for(i = 0 ; i < nbObjetACalculer+complementACalculer ; i++) {
		printf("p%d result[%d] a(%f,%f) b(%f,%f)\n",rank,i, result[i]->a->x, result[i]->a->y, result[i]->b->x, result[i]->b->y);
	}
	printf("##############\n");
   	MPI_Finalize();
   	return 0;
}
