#ifndef H_GEOMETRY
#define H_GEOMETRY

/*
**	STRUCTURES
*/
typedef struct Point {
	double x;
	double y;
} point;

typedef struct Segment {
	point a;
	point b;
} segment;


/*
**	VARIABLES
*/
segment* morceau;
segment* fractal;

/*
**	FONCTIONS
*/
segment createSegment(double xa, double xb, double ya, double yb);
void applyAffineFonctions(segment* seg, double a, double b, double c, double d, double e, double f);
void applyAffineFonctionPoint(point* point, double a, double b, double c, double d, double e, double f);
#endif
