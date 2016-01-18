#include <cairo.h>
#include <gtk/gtk.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../header/geometry.h"
#include "../header/utils.h"
#include "../header/drawing.h"
#define Malloc(type) (type *) malloc(sizeof(type))
#define MallocTab(type,size) (type *) malloc(sizeof(type)*size)
#define TAILLE_MAX 100 // Tableau de taille 100

//Variable Globales
int nbObjectGrphTot;

// Fonctions
static void do_drawing(cairo_t *);
static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{  
	do_drawing(cr);  

	return FALSE;
}

static void do_drawing(cairo_t *cr)
{
	int i;

	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_set_line_width(cr, 0.5);

	for(i = 0 ; i < nbObjectGrphTot; i++) {
		cairo_move_to (cr, fractal[i].a.x*300+150, fractal[i].a.y*300+150);
		cairo_line_to (cr, fractal[i].b.x*300+150, fractal[i].b.y*300+150);
	}

	cairo_stroke(cr);     
}

/* 
*** Ouver un fichier, lit la configuration de la fractal et retourne w (schéma) par référence et le nb de fonction 
*/
void LoadFractal(char* file, double **w, int *nbFonctions){
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

int main (int argc, char *argv[])
{
	int nbIteration;
	int nbObjetACalculer = 0, complementACalculer = 0, resteACalculer = 0;
	int numprocessors, rank;
	int i,j;

	MPI_Datatype pointDt;
	MPI_Datatype segmentDt;

	int iStartOffset, iEndOffset, iStart, iEnd, indice;
	double startTime, endTime, speedup;

	double **w;
	int nbFonctions=0;
	
	GtkWidget *window;
	GtkWidget *darea;
	if(argc != 3) {
		printf("miss argument\n");
		return 0;		
	}
	
	nbIteration = atoi(argv[2]);

	w=malloc(sizeof(double*));
	LoadFractal(argv[1], w, &nbFonctions);
	nbObjectGrphTot = power(nbFonctions, nbIteration);

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocessors);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	startTime = MPI_Wtime();
	 

	MPI_Type_contiguous(2, MPI_DOUBLE, &pointDt);
	MPI_Type_commit(&pointDt);
	
	MPI_Type_contiguous(2, pointDt, &segmentDt);
	MPI_Type_commit(&segmentDt);

	nbObjetACalculer = nbObjectGrphTot/numprocessors;
	resteACalculer = nbObjectGrphTot - (nbObjetACalculer*numprocessors);
		
	fractal = MallocTab(segment, nbObjectGrphTot);	

	if(resteACalculer > rank) {
		complementACalculer = 1;
	}
	morceau = MallocTab(segment, nbObjetACalculer+complementACalculer);
	for(i = 0 ; i<nbObjetACalculer+complementACalculer ; i++) {
		morceau[i] = createSegment(0.0, 1.0, 0.0, 0.0);
	}

	iStartOffset = min(rank,resteACalculer);
	iEndOffset = min(rank+1,resteACalculer);
	iStart = nbObjetACalculer*rank + iStartOffset;
	iEnd = nbObjetACalculer*(rank+1) + iEndOffset;

	for(i = iStart ; i<iEnd; i++) {
		for(j = 0; j < nbIteration; j++) {
			indice = (i/power(nbFonctions,nbIteration-1-j)) % nbFonctions;
			applyAffineFonctions(&morceau[i-iStart], w[indice][0], w[indice][1], w[indice][2], w[indice][3], w[indice][4], w[indice][5]);
		}
	}


	MPI_Gather(morceau, (nbObjetACalculer+complementACalculer), segmentDt, fractal, (nbObjetACalculer+complementACalculer), segmentDt, 0, MPI_COMM_WORLD);

	if(rank == 0 ) {		
		endTime = MPI_Wtime();
		speedup = endTime - startTime;

		printf("speedup : %f - %f = %f\n", startTime, endTime, speedup);

		//Affichage de la fenêtre
		gtk_init(&argc, &argv);

		window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

		darea = gtk_drawing_area_new();
		gtk_container_add(GTK_CONTAINER(window), darea);

		g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(on_draw_event), NULL);
		g_signal_connect(G_OBJECT(window), "destroy",G_CALLBACK(gtk_main_quit), NULL);

		gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
		gtk_window_set_default_size(GTK_WINDOW(window), 600, 600); 
		gtk_window_set_title(GTK_WINDOW(window), "Fractal IFS");

		gtk_widget_show_all(window);

		gtk_main();
	}


   	MPI_Finalize();

   	return 0;
}