#include <math.h>
#include "../header/geometry.h"
#include "../header/dragonCurve.h"

double (*w)[6] = (double [][6]) {{0.5, -0.5, 0.5, 0.5, 0.0, 0.0}, {0.5, 0.5, -0.5, 0.5, 0.5, 0.5}}; //Levy Dragon OK
int nbFonctions = 2;

void applyAffineFonctions(segment* seg, double a, double b, double c, double d, double e, double f) {
	applyAffineFonctionPoint(&(seg->a), a, b, c, d, e, f);
	applyAffineFonctionPoint(&(seg->b), a, b, c, d, e, f);
}

void applyAffineFonctionPoint(point* point, double a, double b, double c, double d, double e, double f) {
	double x = point->x;
	double y = point->y;
	point->x = a * x + b * y + e;	
	point->y = c * x + d * y + f;

}