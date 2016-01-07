#include <stdio.h>
#include <stdlib.h>
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
