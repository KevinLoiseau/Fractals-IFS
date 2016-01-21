#include <cairo.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../header/geometry.h"
#include "../header/utils.h"
#define Malloc(type) (type *) malloc(sizeof(type))
#define MallocTab(type,size) (type *) malloc(sizeof(type)*size)
#define TAILLE_MAX 100 // Tableau de taille 100

//Variable Globales
int nbObjectGrphTot;

// Fonctions
void LoadFractal(char* file, double **w, int *nbFonctions);


int main (int argc, char *argv[])
{
	int nbIteration;
	int nbObjetACalculer = 0, complementACalculer = 0, resteACalculer = 0;
	int nbProc, rank, _nbProc, _rank;
	int i,j;
   	int tag = 0;

	MPI_Datatype pointDt;
	MPI_Datatype segmentDt;
   	//MPI_Request req;
   	MPI_Status status;

	int iStartOffset, iEndOffset, iStart, iEnd, indice;
	double startTime, endTime, speedup;

	double **w;
	int nbFonctions=0;
	
	cairo_surface_t *surface;
	cairo_t *cr;

	if(argc != 3) {
		printf("miss argument\n");
		return 0;		
	}
	
	/* Préparation des variables de la fractales */
	nbIteration = atoi(argv[2]);

	w=malloc(sizeof(double*));
	LoadFractal(argv[1], w, &nbFonctions);
	nbObjectGrphTot = power(nbFonctions, nbIteration);

	/* Initialisation de MPI */
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nbProc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if(nbProc > 1){
		_nbProc = nbProc - 1;
		_rank = rank-1;
	}else{
		_nbProc = nbProc;
		_rank = rank;
	}

	// Init speedup 
	startTime = MPI_Wtime();
	 

	MPI_Type_contiguous(2, MPI_DOUBLE, &pointDt);
	MPI_Type_commit(&pointDt);
	
	MPI_Type_contiguous(2, pointDt, &segmentDt);
	MPI_Type_commit(&segmentDt);

	/* Répartition entre les processus de la fractal */
	nbObjetACalculer = nbObjectGrphTot/_nbProc;
	resteACalculer = nbObjectGrphTot - (nbObjetACalculer*_nbProc);

	if(resteACalculer > _rank) {
		complementACalculer = 1;
	}

	fractal = MallocTab(segment, nbObjetACalculer+complementACalculer);

	for(i = 0 ; i<nbObjetACalculer+complementACalculer ; i++) {
		fractal[i] = createSegment(0.0, 1.0, 0.0, 0.0);
	}

	if(rank > 0 || nbProc == 1){
		iStartOffset = min(_rank,resteACalculer);
		iEndOffset = min(_rank+1,resteACalculer);
		iStart = nbObjetACalculer*_rank + iStartOffset;
		iEnd = nbObjetACalculer*(_rank+1) + iEndOffset;

		for(i = iStart ; i<iEnd; i++) {
			for(j = 0; j < nbIteration; j++) {
				indice = (i/power(nbFonctions,nbIteration-1-j)) % nbFonctions;
				applyAffineFonctions(&fractal[i-iStart], w[indice][0], w[indice][1], w[indice][2], w[indice][3], w[indice][4], w[indice][5]);
			}
		}
	}

	if(rank == 0 ) {
		// Speedup 
		endTime = MPI_Wtime();
		speedup = endTime - startTime;
		// Affichage du speedup
		printf("speedup : %f - %f = %f\n", startTime, endTime, speedup);

		// dessin
		surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 600, 600);
		cr = cairo_create (surface);

		if(nbProc == 1){
			for(i = 0 ; i < nbObjetACalculer+complementACalculer ; i++) {
				cairo_move_to (cr, fractal[i].a.x*300+150, fractal[i].a.y*300+150);
				cairo_line_to (cr, fractal[i].b.x*300+150, fractal[i].b.y*300+150);
			}
		}else{
			for(i = 1; i<nbProc; i++) {
				if(resteACalculer > i) {
					complementACalculer = 1;
				}

				//MPI_Irecv(fractal, nbObjetACalculer+complementACalculer, segmentDt, i, tag, MPI_COMM_WORLD, &req);
				MPI_Recv(fractal, nbObjetACalculer+complementACalculer, segmentDt, i, tag, MPI_COMM_WORLD, &status);
				for(j = 0 ; j < nbObjetACalculer+complementACalculer ; j++) {
					cairo_move_to (cr, fractal[j].a.x*300+150, fractal[j].a.y*300+150);
					cairo_line_to (cr, fractal[j].b.x*300+150, fractal[j].b.y*300+150);
				}
			}
		}

		cairo_set_line_width(cr, 1.0);
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
		cairo_stroke(cr);
		cairo_destroy (cr);
		cairo_surface_write_to_png (surface, "fractal.png");
		cairo_surface_destroy (surface);
	}else{
		//MPI_Isend(fractal, nbObjetACalculer+complementACalculer, segmentDt, 0, tag, MPI_COMM_WORLD, &req);
		MPI_Send(fractal, nbObjetACalculer+complementACalculer, segmentDt, 0, tag, MPI_COMM_WORLD);
	}

   	MPI_Finalize();

   	return 0;
}

/*
*** FONCTIONS
*/

/* 
*** Ouver un fichier, lit la configuration de la fractal et retourne w (schéma) par référence et le nb de fonction 
*/void LoadFractal(char* file, double **w, int *nbFonctions){
	// Pour charger les paramètres à partir d'un fichier en argument
	FILE* param = NULL;
	char ligne[TAILLE_MAX] = ""; // Chaîne vide de taille TAILLE_MAX
	int i_ligne=0; 
 	char flottant[6]={0};
	int nbColonnes=6;
	int i,j,k;
	int nbFct=0;

	/*
	* Chargement des paramètres à partir du fichier
	*/
	param = fopen(file, "r+");
	if (param != NULL){
		//On récupère le nombre de fonctions en comptant le nombre de lignes
		while (fgets(ligne, TAILLE_MAX, param) != NULL){
			nbFct++;
		}
		fseek(param, 0, SEEK_SET);

		//On forme w en parsant le fichier
        	while (fgets(ligne, TAILLE_MAX, param) != NULL){
			i=0;
			w[i_ligne]=malloc(nbColonnes * sizeof(double));
			do{
				for(k=0 ; k<nbColonnes ; k++){
					j=0;
					do{
						flottant[j]=ligne[i];
						j++;
						i++;
					}while(ligne[i] != ' ' && ligne[i] != '\n');
					w[i_ligne][k]=atof(flottant);
					memset(flottant, 0, nbColonnes);
				}
			}while(ligne[i]!='\n');
				i_ligne++;
		}
		fclose(param); //On ferme le fichier
		*nbFonctions = nbFct;
	}else{
		printf("Impossible d'ouvrir le fichier %s", file);
	}
}