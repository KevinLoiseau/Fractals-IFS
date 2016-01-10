CFLAGS=-Wall
CC=mpicc
CAIRO_LIB=$(shell pkg-config cairo --cflags --libs)
GTK_LIB=$(shell pkg-config gtk+-3.0 --cflags --libs)
STATIC_LIB=source/fractal.c source/geometry.c source/utils.c source/dragonCurve.c

all : clean build
	mpirun -n 4 bin/fractal "source/param.txt" 13
clean :
	rm -f bin/fractal *.o *.txt *.png
build:
	$(CC) $(CFLAGS) -g $(STATIC_LIB) $(CAIRO_LIB) $(GTK_LIB) -o bin/fractal
