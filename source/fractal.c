#include <cairo.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../header/geometry.h"
#include "../header/utils.h"
#define Malloc(type) (type *) malloc(sizeof(type))
#define MallocTab(type,size) (type *) malloc(sizeof(type)*size)

double (*w)[6] = (double [][6]) {{0.5, -0.5, 0.5, 0.5, 0.0, 0.0}, {0.5, 0.5, -0.5, 0.5, 0.5, 0.5}}; //Levy Dragon OK
/*double (*w)[6] = (double [][6]) {{0.382, 0, 0, 0.385, 0.3072, 0.619},
				 {0.382, 0, 0, 0.385, 0.6033, 0.4044},
				 {0.382, 0, 0, 0.385, 0.0139, 0.4044},
				 {0.382, 0, 0, 0.385, 0.1253, 0.0595},
				 {0.382, 0, 0, 0.385, 0.492, 0.0595}}; //Crystal OK*/
/*double (*w)[6] = (double [][6]) {{0, -0.5, 0.5, 0, 0.5, 0},
				 {0, 0.5, -0.5, 0, 0.5, 0.5},
				 {0.5, 0, 0, 0.5, 0.25, 0.5}}; //Twin Christmas tree OK*/
/*double (*w)[6] = (double [][6]) {{0.33333, 0, 0, 0.33333, 0, 0},
				 {0.16667, 0.288675135, -0.288675135, 0.16667, 0.33333, 0},
				 {0.16667, -0.288675135, 0.288675135, 0.16667, 0.5, -0.288675135},
				 {0.33333, 0, 0, 0.333, 0.66667, 0}}; //Koch Curve OK*/

/*double (*w)[6] = (double [][6]) {{0.33333, 0, 0, 0.33333, 0, 0}, //ok
				 {0.16667, 0.288675135, -0.288675135, 0.16667, 0.33333, 0}, //ok
				 {0.16667, -0.288675135, 0.288675135, 0.16667, 0.5, -0.288675135}, //ok
				 {0.33333, 0, 0, 0.333, 0.66667, 0}, //ok
				 {0.16667, -0.288675135, 0.288675135, 0.16667, 0, 0}, //ok
				 {0.16667, 0.288675135, -0.288675135, 0.16667, 0.33333, 0},
				 {0.16667, -0.288675135, 0.288675135, 0.16667, 0.5, -0.288675135},
				 {0.16667, -0.288675135, 0.288675135, 0.16667, 0.33333, 0}, //en cours
				 {0.16667, 0.288675135, -0.288675135, -0.16667, 0.288675135, 0.288675135},
				 {0.16667, 0.288675135, -0.288675135, 0.16667, 0.33333, 0},
				 {0.16667, -0.288675135, 0.288675135, 0.16667, 0.5, -0.288675135},
				 {0.16667, 0.288675135, -0.288675135, 0.16667, 0.33333, 0}}; //Koch snowfloke*/

int nbFonctions = 2;
void applyAffineFonctions(segment* seg, double a, double b, double c, double d, double e, double f);
void applyAffineFonctionPoint(point* point, double a, double b, double c, double d, double e, double f);
int main (int argc, char *argv[])
{
	
	int nbIteration;
	int i,j;
	int nbObjetACalculer = 0, complementACalculer = 0, resteACalculer = 0;
	int numprocessors, rank;
	int nbObjectGrphTot;

	
   	int tag = 0;
   	MPI_Status status;
	MPI_Datatype pointDt;
	MPI_Datatype segmentDt;

	int iStartOffset;
	int iEndOffset;
	int iStart;
	int iEnd;
	int indice;
	cairo_surface_t *surface;
	cairo_t *cr;
	segment* result;
	
	if(argc != 2) {
		
		printf("miss argument\n");
		return 0;		
	}
	nbIteration = atoi(argv[1]);
	nbObjectGrphTot = power(nbFonctions, nbIteration);
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocessors);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	 

	for( i = 0; i < argc; i++) {
		printf("p%d arg[%d] : %s\n",rank, i, argv[i]);
	}

	MPI_Type_contiguous(2, MPI_DOUBLE, &pointDt);
	MPI_Type_commit(&pointDt);
	
	MPI_Type_contiguous(2, pointDt, &segmentDt);
	MPI_Type_commit(&segmentDt);

	nbObjetACalculer = nbObjectGrphTot/numprocessors;
	resteACalculer = nbObjectGrphTot - (nbObjetACalculer*numprocessors);
	
	if ( rank == 0 )
	{
		surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 600, 600);
        	cr = cairo_create (surface);
		printf("Nombre d'objets graphique à calculer : %d\n", nbObjectGrphTot);
		printf("Reste à repartir : %d\n", resteACalculer);
		for(i = 0 ; i < nbFonctions; i++) {
			printf("f%d(a:%f ,b:%f, c:%f, d:%f, e:%f, f:%f)\n",i, w[i][0], w[i][1], w[i][2], w[i][3], w[i][4], w[i][5]);
		}
    	}
	if(resteACalculer > rank) {
		complementACalculer = 1;
	}
	result = MallocTab(segment, nbObjetACalculer+complementACalculer);
	for(i = 0 ; i<nbObjetACalculer+complementACalculer ; i++) {
		result[i] = createSegment(0.0, 1.0, 0.0, 0.0);
	}
	printf("p%d => Nombre d'objet graphique à calculer : %d\n", rank, nbObjetACalculer+complementACalculer); 
	iStartOffset = min(rank,resteACalculer);
	iEndOffset = min(rank+1,resteACalculer);
	iStart = nbObjetACalculer*rank + iStartOffset;
	iEnd = nbObjetACalculer*(rank+1) + iEndOffset;
	for(i = iStart ; i<iEnd; i++) {
		printf("============\n");
		for(j = 0; j < nbIteration; j++) {
			indice = (i/power(nbFonctions,nbIteration-1-j)) % nbFonctions;
			printf("p%d objet %d iteration %d: fonctions applique %d\n",rank, i+1,j,indice+1);
			applyAffineFonctions(&result[i-iStart], w[indice][0], w[indice][1], w[indice][2], w[indice][3], w[indice][4], w[indice][5]);
		}
		printf("p%d objet %d application dans result[%d]\n", rank, i+1, i-iStart);
		printf("============\n");
	}
	
	printf("##############\n");
	
	for(i = 0 ; i < nbObjetACalculer+complementACalculer ; i++) {
		printf("p%d result[%d] a(%f,%f) b(%f,%f)\n",rank,i, result[i].a.x, result[i].a.y, result[i].b.x, result[i].b.y);
	}
	
	printf("##############\n");

	if(rank == 0 ) {
		for(i = 0 ; i < nbObjetACalculer+complementACalculer ; i++) {
			cairo_move_to (cr, result[i].a.x*300+150, result[i].a.y*300+150);
			cairo_line_to (cr, result[i].b.x*300+150, result[i].b.y*300+150);
		}
		for(i = 1; i<numprocessors; i++) {
			if(resteACalculer > i) {
				complementACalculer = 1;
			}
			MPI_Recv(result, nbObjetACalculer+complementACalculer, segmentDt, i, tag, MPI_COMM_WORLD, &status);
			for(j = 0 ; j < nbObjetACalculer+complementACalculer ; j++) {
				cairo_move_to (cr, result[j].a.x*300+150, result[j].a.y*300+150);
				cairo_line_to (cr, result[j].b.x*300+150, result[j].b.y*300+150);
			}
		}		
		cairo_set_line_width(cr, 1.0);
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
		cairo_stroke(cr);
		cairo_destroy (cr);
		cairo_surface_write_to_png (surface, "hello.png");
		cairo_surface_destroy (surface);
	} else {
		MPI_Send(result, nbObjetACalculer+complementACalculer, segmentDt, 0, tag, MPI_COMM_WORLD);
	}
   	MPI_Finalize();
   	return 0;
}


void applyAffineFonctions(segment* seg, double a, double b, double c, double d, double e, double f) {
	double originX = seg->a.x;
	double originY = seg->a.y;
	applyAffineFonctionPoint(&(seg->a), a, b, c, d, e, f);
	applyAffineFonctionPoint(&(seg->b), a, b, c, d, e, f);
}

void applyAffineFonctionPoint(point* point, double a, double b, double c, double d, double e, double f) {
	double x = point->x;
	double y = point->y;
	point->x = a * x + b * y + e;	
	point->y = c * x + d * y + f;

}
