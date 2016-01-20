#include <stdlib.h>
#include <math.h>
#include "../header/geometry.h"
#define Malloc(type) (type *) malloc(sizeof(type))
#define MallocTab(type,size) (type *) malloc(sizeof(type)*size)

segment createSegment(double xa, double xb, double ya, double yb) {
	segment* seg;
	seg = Malloc(segment);
	seg->a.x = xa;
	seg->a.y = ya;
	seg->b.x = xb;
	seg->b.y = yb;
	return *seg;
}

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
