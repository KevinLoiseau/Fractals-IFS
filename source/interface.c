#include <gtk/gtk.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "../header/geometry.h"
#include "../header/fractal.h"

//static void on_open_image (GtkButton* button, gpointer user_data);
static GtkWidget* create_window (char* img);

int main (int argc, char *argv[])
{
	int nbProc, rank;

	GtkWidget *window;

	if(argc != 3) {
		printf("miss argument\n");
		MPI_Abort(MPI_COMM_WORLD, 1);
		exit(1);		
	}

	/* Initialisation de MPI */
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nbProc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	createFractal(rank, nbProc, argv[1], atoi(argv[2]));



	// Création de la fenêtre graphique pour le root (rank 0)
	if(rank ==0){
		
		gtk_init(&argc, &argv);
		window = create_window("fractal.png");

		gtk_widget_show_all(window);
		gtk_main();
	}

   	MPI_Finalize();
	return 0;
}

static GtkWidget* create_window (char* img){
	GtkWidget *window;
	GtkWidget *image;
	GtkWidget *box;

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Fractal-IFS");

	box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
	image = gtk_image_new_from_file (img);

	gtk_box_pack_start (GTK_BOX (box), image, TRUE, TRUE, 0);

	gtk_container_add (GTK_CONTAINER (window), box);

	/* Signal de fermeture de la fenetre */
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

	return window;
}
