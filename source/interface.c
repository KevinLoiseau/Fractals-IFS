#include <gtk/gtk.h>

void on_open_image (GtkButton* button, gpointer user_data){
	GtkWidget *image = GTK_WIDGET (user_data);
	GtkWidget *toplevel = gtk_widget_get_toplevel (image);
	GtkFileFilter *filter = gtk_file_filter_new ();
	GtkWidget *dialog = gtk_file_chooser_dialog_new (("Open image"), GTK_WINDOW (toplevel), GTK_FILE_CHOOSER_ACTION_OPEN, "_Ok", GTK_RESPONSE_ACCEPT, "_Cancel", GTK_RESPONSE_CANCEL, NULL);

	gtk_file_filter_add_pixbuf_formats (filter);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog),
	                             filter);

	switch (gtk_dialog_run (GTK_DIALOG (dialog)))
	{
		case GTK_RESPONSE_ACCEPT:
		{
			gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
			gtk_image_set_from_file (GTK_IMAGE (image), filename);
			break;
		}
		default:
			break;
	}
	gtk_widget_destroy (dialog);
}

GtkWidget* create_window (char* img){
	GtkWidget *window;
	//GtkWidget *button;
	GtkWidget *image;
	GtkWidget *box;

	/* Set up the UI */
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Fractal-IFS");

	box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
	//button = gtk_button_new_with_label ("Open image");
	image = gtk_image_new_from_file (img);

	gtk_box_pack_start (GTK_BOX (box), image, TRUE, TRUE, 0);
	//gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);

	gtk_container_add (GTK_CONTAINER (window), box);

	/* Signal de fermeture de la fenetre */
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

	return window;
}