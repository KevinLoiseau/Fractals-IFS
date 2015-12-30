#include <cairo.h>
#include <mpi.h>
#include <stdio.h>
int
main (int argc, char *argv[])
{

	int numprocessors, rank, namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
 
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocessors);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(processor_name, &namelen);
 
    if ( rank == 0 )
    {
	printf("Processor name: %s\n",processor_name);
	printf("master (%d/%d)\n", rank, numprocessors);
    } else {
        printf("slave (%d/%d)\n", rank, numprocessors);
   }
   MPI_Finalize();
        cairo_surface_t *surface =
            cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 240, 80);
        cairo_t *cr =
            cairo_create (surface);

        cairo_select_font_face (cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size (cr, 32.0);
        cairo_set_source_rgb (cr, 0.0, 0.0, 1.0);
        cairo_move_to (cr, 10.0, 50.0);
        cairo_show_text (cr, "Hello, world");



        cairo_destroy (cr);
        cairo_surface_write_to_png (surface, "hello.png");
        cairo_surface_destroy (surface);
        return 0;
}
