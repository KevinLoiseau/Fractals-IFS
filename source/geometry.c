#include <stdio.h>
#include <stdlib.h>
#include "../header/geometry.h"
#define Malloc(type) (type *) malloc(sizeof(type))
#define MallocTab(type,size) (type *) malloc(sizeof(type)*size)

segment* createSegment(double xa, double xb, double ya, double yb) {
	segment* seg;
	seg = Malloc(segment);
	seg->a = createPoint(xa, ya);
	seg->b = createPoint(xb, yb);
	return seg;
}

point* createPoint(double x, double y) {
	point* p;
	p = Malloc(point);
	p->x = x;
	p->y = y;
	return p;
}
