#include <cairo.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../header/geometry.h"
#include "../header/utils.h"
#include "../header/fractal.h"

#define Malloc(type) (type *) malloc(sizeof(type))
#define MallocTab(type,size) (type *) malloc(sizeof(type)*size)
#define TAILLE_MAX 100 // Tableau de taille 100
#define DEFAULT_SCALE_FACTOR 400
#define DEFAULT_SIZE_X 600
#define DEFAULT_SIZE_Y 600
#define DEFAULT_OFFSET_X 150
#define DEFAULT_OFFSET_Y 450
#define NB_COEFF 6

static void draw_fractal_part(cairo_t *cr, int fractal_part_size, segment *fractal_part);
void createFractal(int rank, int nbProc, char* file, int nbIteration){
	double **w;
	int nbFonctions = 0;

	int nbObjetACalculer = 0, complementACalculer = 0, resteACalculer = 0, nbObjectGrphTot;
	int i,j,k;

	int iStartOffset, iEndOffset, iStart, iEnd, indice;
	double startTime, endTime, execTime;

	//On gère la taille de l'image
	double yMin = 0.0;
	double yMax = 0.0;
	double xMin = 0.0;
	double xMax = 1.0;
	double reduce_yMin;
	double reduce_yMax;
	double reduce_xMin;
	double reduce_xMax;
	size_x = DEFAULT_SIZE_X;
	size_y = DEFAULT_SIZE_Y;
	scale_factor =  DEFAULT_SCALE_FACTOR;
	offset_x = DEFAULT_OFFSET_X;
	offset_y = DEFAULT_OFFSET_Y;


   	MPI_Status status;
	MPI_Datatype pointDt;
	MPI_Datatype segmentDt;

	cairo_surface_t *surface;
	cairo_t *cr;

	// Pour charger les paramètres à partir d'un fichier en argument
	FILE* param = NULL;
	char ligne[TAILLE_MAX] = ""; // Chaîne vide de taille TAILLE_MAX
	int i_ligne=0; 
 	char flottant[NB_COEFF]={0};

	/*
	* Chargement des paramètres à partir du fichier
	*/
	if(rank == 0) {
		param = fopen(file, "r+");
		if (param != NULL){
			//On récupère le nombre de fonctions en comptant le nombre de lignes
			while (fgets(ligne, TAILLE_MAX, param) != NULL){
				nbFonctions++;
			}

			w=MallocTab(double*,nbFonctions);
			fseek(param, 0, SEEK_SET);
			//On forme w en parsant le fichier
			while (fgets(ligne, TAILLE_MAX, param) != NULL){
				i=0;
				w[i_ligne]=MallocTab(double,NB_COEFF);
				do{
					for(k=0 ; k<NB_COEFF ; k++){
						j=0;
						do{
							flottant[j]=ligne[i];
							j++;
							i++;
						}while(ligne[i] != ' ' && ligne[i] != '\n');
						w[i_ligne][k]=atof(flottant);			
						memset(flottant, 0, NB_COEFF);
					}
				}while(ligne[i]!='\n');
					i_ligne++;
			}
			fclose(param); //On ferme le fichier
		}else{
			printf("Impossible d'ouvrir le fichier %s", file);
			MPI_Abort(MPI_COMM_WORLD, 1);
			exit(1);
		}
	} 
	//Envoie des fonctions de root (rank 0) vers les autres processus
	MPI_Bcast(&nbFonctions, 1, MPI_INT,0, MPI_COMM_WORLD);
	if(rank != 0)
			w=MallocTab(double*,nbFonctions);
	for(i=0;i<nbFonctions; i++){
		if(rank!=0)
			w[i]=MallocTab(double,NB_COEFF);
		MPI_Bcast(w[i], NB_COEFF, MPI_DOUBLE,0, MPI_COMM_WORLD);		
	}

	//Calcul du nombre d'objet graphique à calcul
	//IFS déterministe
	nbObjectGrphTot = power(nbFonctions, nbIteration);

	// Init calcul temps exécution 
	startTime = MPI_Wtime();
	 
	//Définition de la structure point pour MPI
	MPI_Type_contiguous(2, MPI_DOUBLE, &pointDt);
	MPI_Type_commit(&pointDt);
	
	//Définition de la structure segment pour MPI
	MPI_Type_contiguous(2, pointDt, &segmentDt);
	MPI_Type_commit(&segmentDt);
	/* Répartition entre les processus de la fractal */
	//Division euclidienne
	//Dividande : nbObjectGrphTot
	//Diviseur : nbProc
	//Quotient : nbObjetACalculer
	//Reste : resteACalculer
	nbObjetACalculer = nbObjectGrphTot/nbProc;
	resteACalculer = nbObjectGrphTot - (nbObjetACalculer*nbProc);
	
	//Prise d'un calcul supplémentaire si rank < resteACalculer
	if(resteACalculer > rank) {
		complementACalculer = 1;
	}
	//FIX : possible problème de dépassement mémoire entre processus si taille(fractal) = nbObjetACalculer + complementACalculer avec complementACalculer = 0
	fractal = MallocTab(segment, nbObjetACalculer+1);

	for(i = 0 ; i<nbObjetACalculer+complementACalculer ; i++) {
		fractal[i] = createSegment(0.0, 1.0, 0.0, 0.0);
	}
	
	//Calcul des indices par rapport à l'indice de l'objet à calculer par rapport à la fractal
	//Indice utilisé pour déterminer les fonctions à utiliser pour les n itérations
	iStartOffset = min(rank,resteACalculer);
	iEndOffset = min(rank+1,resteACalculer);
	iStart = nbObjetACalculer*rank + iStartOffset;
	iEnd = nbObjetACalculer*(rank+1) + iEndOffset;
	

	for(i = iStart ; i<iEnd; i++) {
		//Application des n itérations sur l'objet initial
		//ex : Ai = f1*f2*f6*f5(A) où A est l'objet initiale et Ai un objet produit par les n itérations. 
		for(j = nbIteration-1; j > 0; j--) {
			indice = (i/power(nbFonctions,nbIteration-j)) % nbFonctions;
			applyAffineFonctions(&fractal[i-iStart], w[indice][0], w[indice][1], w[indice][2], w[indice][3], w[indice][4], w[indice][5]);
			
		}
		//Calcul local du minimum et du maximum sur la partie de fractale en cours de calcul
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
		
	}
		
	//Calcul global du minimum et du maximum sur l'ensemble de la fractale
	MPI_Reduce(&yMin,&reduce_yMin, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
	MPI_Reduce(&xMin,&reduce_xMin, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
	MPI_Reduce(&yMax,&reduce_yMax, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&xMax,&reduce_xMax, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	
	//Fin des calculs
	//Début de la phase graphique (dessin)
	if(rank == 0) {
		//Calcul de la taille de l'image
		size_x = ((int)(reduce_xMax * scale_factor - reduce_xMin * scale_factor)/100+1)*100;
		size_y = ((int)(reduce_yMax * scale_factor - reduce_yMin * scale_factor)/100+1)*100;
	
		//Calcul du décalage à appliquer pour center la fractale
		offset_x = 0;
		if (reduce_xMin< 0) {
			offset_x = - (reduce_xMin * scale_factor);
		}
		offset_x += (size_x - (reduce_xMax * scale_factor - reduce_xMin * scale_factor))/2;
		offset_y = size_y;
		if (reduce_yMin * scale_factor < 0) {
			offset_y += (reduce_yMin * scale_factor);
		}
		offset_y -= (size_y - (reduce_yMax * scale_factor - reduce_yMin * scale_factor))/2;


		// dessin
		surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, size_x, size_y);
		cr = cairo_create (surface);
		//Fond blanc
		cairo_rectangle(cr, 0.0, 0.0, size_x, size_y);
		cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
		cairo_fill(cr);
		
		//segments noirs
		cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
		draw_fractal_part(cr, nbObjetACalculer+complementACalculer, fractal);
		for(i = 1; i<nbProc; i++) {
			complementACalculer = 0;
			if(resteACalculer > i) {
				complementACalculer = 1;
			}
			MPI_Recv(fractal, nbObjetACalculer+1, segmentDt, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			draw_fractal_part(cr, nbObjetACalculer+complementACalculer, fractal);
		}

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
	}else{
		MPI_Send(fractal, nbObjetACalculer+1, segmentDt, 0, rank, MPI_COMM_WORLD);
		
	}
}

static void draw_fractal_part(cairo_t *cr, int fractal_part_size, segment *fractal_part) {
	int i;
	//Application de -scale_factor sur y car l'origine (0,0) commence en haut à gauche et avec y croissant vers le bas
	//La fractale a pour origine (0,0) en bas à gauche et y croisant vers le haut
	for(i = 0 ; i < fractal_part_size ; i++) {
		//Déplacement au point a du segment
		cairo_move_to (cr, fractal_part[i].a.x * scale_factor + offset_x, fractal_part[i].a.y * -scale_factor + offset_y);
		//Trace jusqu'au point b
		cairo_line_to (cr, fractal_part[i].b.x * scale_factor + offset_x, fractal_part[i].b.y * -scale_factor + offset_y);
	}
}
