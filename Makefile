CFLAGS=-Wall
CC=mpicc
CAIRO_LIB=$(shell pkg-config cairo --cflags --libs)
GTK_LIB=$(shell pkg-config gtk+-3.0 --cflags --libs)
STATIC_LIB=source/fractal.c source/geometry.c source/utils.c source/interface.c

all : clean build
	mpirun -n 3 bin/fractal "param" 3
unique : clean build
	mpirun -n 1 bin/fractal "param" 5	
koch: clean build
	mpirun -n 3 bin/fractal "sample/kochCurve" 5
mcWorterPentigree: clean build
	mpirun -n 3 bin/fractal "sample/mcWorterPentigree" 8		
clean :
	rm -f bin/fractal *.o *.txt *.png
build:
	$(CC) $(CFLAGS) -g $(STATIC_LIB) $(CAIRO_LIB) $(GTK_LIB) -o bin/fractal
