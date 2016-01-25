#ifndef H_FRACTAL
#define H_FRACTAL

/*
**	VARIABLES
*/
double size_x;
double size_y;
int scale_factor;
double offset_x;
double offset_y;

/*
**	FONCTIONS
*/
void createFractal(int rank, int nbProc, char* file, int nbIteration);
#endif
