#ifndef H_GEOMETRY
#define H_GEOMETRY
typedef struct Point {
	double x;
	double y;
} point;

typedef struct Segment {
	point a;
	point b;
} segment;

segment createSegment(double xa, double xb, double ya, double yb);
#endif
