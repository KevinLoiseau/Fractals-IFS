#include <cairo.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include "../header/geometry.h"
#include "../header/utils.h"
#include "../header/interface.h"
#define Malloc(type) (type *) malloc(sizeof(type))
#define MallocTab(type,size) (type *) malloc(sizeof(type)*size)
#define TAILLE_MAX 100 // Tableau de taille 100

//Variable Globales
int nbObjectGrphTot;

int main (int argc, char *argv[])
{
	int nbIteration;
	int nbObjetACalculer = 0, complementACalculer = 0, resteACalculer = 0;
	int nbProc, rank, _nbProc, _rank;
	int i,j,k;
   	int tag = 0;

	MPI_Datatype pointDt;
	MPI_Datatype segmentDt;
   	MPI_Status status;

	int iStartOffset, iEndOffset, iStart, iEnd, indice;
	double startTime, endTime, execTime;

	double **w;
	int nbFonctions=0;

	double yMin = 0.0;
	double yMax = 0.0;
	double xMin = 0.0;
	double xMax = 1.0;
	double yRatio = 0;
	double xRatio = 0;	
	double yCenter = (yMax-yMin)/2;
	double xCenter = (xMax-xMin)/2;

	cairo_surface_t *surface;
	cairo_t *cr;

	GtkWidget *window;

	// Pour charger les paramètres à partir d'un fichier en argument
	FILE* param = NULL;
	char ligne[TAILLE_MAX] = ""; // Chaîne vide de taille TAILLE_MAX
	int i_ligne=0; 
 	char flottant[6]={0};
	int nbColonnes=6;
	int nbFct=0;
	char* file = argv[1];

	if(argc != 3) {
		printf("miss argument\n");
		MPI_Abort(MPI_COMM_WORLD, 1);
		exit(1);		
	}
	
	/* Préparation des variables de la fractales */
	nbIteration = atoi(argv[2]);
	/* Initialisation de MPI */
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nbProc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	/*
	* Chargement des paramètres à partir du fichier
	*/
	//if(rank == 0) {
		param = fopen(file, "r+");
		if (param != NULL){
			//On récupère le nombre de fonctions en comptant le nombre de lignes
			while (fgets(ligne, TAILLE_MAX, param) != NULL){
				nbFct++;
			}
			nbFonctions = nbFct;
			w=MallocTab(double*,nbFonctions);
			fseek(param, 0, SEEK_SET);
			//On forme w en parsant le fichier
			while (fgets(ligne, TAILLE_MAX, param) != NULL){
				i=0;
				w[i_ligne]=MallocTab(double,nbColonnes);
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
			//for(i = 1; i<nbProc ; i++) {
			//	MPI_Send(&nbFonctions, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
			//}
		}else{
			printf("Impossible d'ouvrir le fichier %s", file);
			MPI_Abort(MPI_COMM_WORLD, 1);
			exit(1);
		}
	//} else {
	//	MPI_Recv(&nbFonctions, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	//	printf("(rank %d) nbFonctions : %d",rank, nbFonctions);
	//} 

	nbObjectGrphTot = power(nbFonctions, nbIteration);

	//if(nbProc > 1){
	//	_nbProc = nbProc - 1;
	//	_rank = rank-1;
	//}else{
		_nbProc = nbProc;
		_rank = rank;
	//}

	// Init speedup 
	startTime = MPI_Wtime();
	 

	MPI_Type_contiguous(2, MPI_DOUBLE, &pointDt);
	MPI_Type_commit(&pointDt);
	
	MPI_Type_contiguous(2, pointDt, &segmentDt);
	MPI_Type_commit(&segmentDt);
	/* Répartition entre les processus de la fractal */
	nbObjetACalculer = nbObjectGrphTot/_nbProc;
	resteACalculer = nbObjectGrphTot - (nbObjetACalculer*_nbProc);
	//if(rank > 0 || nbProc == 1){
		
		if(resteACalculer > _rank) {
			complementACalculer = 1;
		}
		fractal = MallocTab(segment, nbObjetACalculer+complementACalculer);
		printf("(rank %d) nb object Total : %d\n", rank, nbObjectGrphTot);
		printf("(rank %d) nb object à calculer : %d\n", rank, nbObjetACalculer+complementACalculer);
		for(i = 0 ; i<nbObjetACalculer+complementACalculer ; i++) {
			fractal[i] = createSegment(0.0, 1.0, 0.0, 0.0);
		}
		
		iStartOffset = min(_rank,resteACalculer);
		iEndOffset = min(_rank+1,resteACalculer);
		iStart = nbObjetACalculer*_rank + iStartOffset;
		iEnd = nbObjetACalculer*(_rank+1) + iEndOffset;
		
		printf("(rank %d) iStart: %d, iEnd: %d\n", rank, iStart, iEnd-1);
		for(i = iStart ; i<iEnd; i++) {
			for(j = 0; j < nbIteration; j++) {
				indice = (i/power(nbFonctions,nbIteration-1-j)) % nbFonctions;
				applyAffineFonctions(&fractal[i-iStart], w[indice][0], w[indice][1], w[indice][2], w[indice][3], w[indice][4], w[indice][5]);
				
			}
			if(xMin > fractal[i-iStart].a.x) {
				xMin = fractal[i-iStart].a.x;
			} else if (xMin > fractal[i-iStart].b.x) {
				xMin = fractal[i-iStart].b.x;
			}
			if(xMax < fractal[i-iStart].a.x) {
				xMax = fractal[i-iStart].a.x;
			} else if (xMax < fractal[i-iStart].b.x) {
				xMax = fractal[i-iStart].b.x;
			}
			if(yMin > fractal[i-iStart].a.y) {
				yMin = fractal[i-iStart].a.y;
			} else if (yMin > fractal[i-iStart].b.y) {
				yMin = fractal[i-iStart].b.y;
			}
			if(yMax < fractal[i-iStart].a.y) {
				yMax = fractal[i-iStart].a.y;
			} else if (yMax < fractal[i-iStart].b.y) {
				yMax = fractal[i-iStart].b.y;
			}
			xRatio = xMax / xMin;
			yRatio = yMax / yMin;
			yCenter = (yMax-yMin)/2;
			xCenter = (xMax-xMin)/2;
			
		}
		printf("(rank %d) xMin: %f, xMax: %f, yMin: %f, yMax: %f\n", rank, xMin, xMax, yMin, yMax);
		printf("(rank %d) xCenter: %f, yCenter: %f\n", rank,  xCenter, yCenter);
		printf("(rank %d) xRatio: %f, yRatio: %f\n", rank,  xRatio, yRatio);
	//}

	if(rank == 0) {
		// Speedup 

		// dessin
		surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1000, 1000);
		cr = cairo_create (surface);
		cairo_rectangle(cr, 0.0, 0.0, 1000, 1000);
		cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
		cairo_fill(cr);
		
		cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
		//if(nbProc == 1){
			for(i = 0 ; i < nbObjetACalculer+complementACalculer ; i++) {
				cairo_move_to (cr, fractal[i].a.x*500+250, fractal[i].a.y*-500+750);
				cairo_line_to (cr, fractal[i].b.x*500+250, fractal[i].b.y*-500+750);
			}
		//}else{
			for(i = 1; i<nbProc; i++) {
				complementACalculer = 0;
				if(resteACalculer > i) {
					complementACalculer = 1;
				}
				
				//fractal = MallocTab(segment, nbObjetACalculer+complementACalculer);
				MPI_Recv(fractal, nbObjetACalculer+complementACalculer, segmentDt, i, tag, MPI_COMM_WORLD, &status);
				for(j = 0 ; j < nbObjetACalculer+complementACalculer ; j++) {
					cairo_move_to (cr, fractal[j].a.x*500+250, fractal[j].a.y*-500+750);
					cairo_line_to (cr, fractal[j].b.x*500+250, fractal[j].b.y*-500+750);
				}
			}
		//}


		// Temps d'exécution 
		endTime = MPI_Wtime();
		execTime = endTime - startTime;
		// Affichage du temps d'exécution 
		printf("Temps d'exécution : %f - %f = %f\n", startTime, endTime, execTime);

		//Dessin et mise en image png de la fractal
		cairo_set_line_width(cr, 1.0);
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
		cairo_stroke(cr);
		cairo_destroy (cr);
		cairo_surface_write_to_png (surface, "fractal.png");
		cairo_surface_destroy (surface);

		// Création de la fenêtre graphique
		gtk_init(&argc, &argv);

		window = create_window("fractal.png");

		gtk_widget_show_all(window);
  		gtk_main();

	}else{
		MPI_Send(fractal, nbObjetACalculer+complementACalculer, segmentDt, 0, tag, MPI_COMM_WORLD);
	}

   	MPI_Finalize();
	return 0;
}
