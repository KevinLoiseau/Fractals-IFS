CFLAGS=-Wall
CC=mpicc
CAIRO_LIB=$(shell pkg-config cairo --cflags --libs)
GTK_LIB=$(shell pkg-config gtk+-3.0 --cflags --libs)
STATIC_LIB=source/fractal.c source/geometry.c source/utils.c source/interface.c

all : clean build
	mpirun -n 5 bin/fractal "param" 7
unique : clean build
	mpirun -n 1 bin/fractal "param" 5	
kochCurve: clean build
	mpirun -n 3 bin/fractal "sample/kochCurve" 5
kochSnowflake: clean build
	mpirun -n 3 bin/fractal "sample/kochSnowflake" 5
pentigree: clean build
	mpirun -n 3 bin/fractal "sample/mcWorterPentigree" 8	
pentagon: clean build
	mpirun -n 3 bin/fractal "sample/pentagon" 8
levyDragon: clean build
	mpirun -n 3 bin/fractal "sample/levyDragon" 17	
christmasTree: clean build
	mpirun -n 3 bin/fractal "sample/twinChristmasTree" 7
twinDragon: clean build
	mpirun -n 3 bin/fractal "sample/twinDragon" 15
clean :
	rm -f bin/fractal *.o *.txt *.png
build:
	$(CC) $(CFLAGS) -g $(STATIC_LIB) $(CAIRO_LIB) $(GTK_LIB) -o bin/fractal
