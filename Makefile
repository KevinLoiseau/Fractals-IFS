CFLAGS=-Wall
CC=mpicc
CAIRO_LIB=$(shell pkg-config cairo --cflags --libs)
GTK_LIB=$(shell pkg-config gtk+-3.0 --cflags --libs)
STATIC_LIB=source/interface.c source/fractal.c source/geometry.c source/utils.c

all : clean build
	 mpirun --mca plm_rsh_no_tree_spawn 1 --hostfile hosts bin/fractal "param" 7 > output.txt
unique : clean build unique
	mpirun --mca plm_rsh_no_tree_spawn 1 --hostfile hosts  bin/fractal "param" 5	
kochCurve: clean build
	mpirun --mca plm_rsh_no_tree_spawn 1 --hostfile hosts bin/fractal "sample/kochCurve" 5
kochSnowflake: clean build
	mpirun --mca plm_rsh_no_tree_spawn 1 --hostfile hosts bin/fractal "sample/kochSnowflake" 5
pentigree: clean build
	mpirun --mca plm_rsh_no_tree_spawn 1 --hostfile hosts bin/fractal "sample/mcWorterPentigree" 8	
pentagon: clean build
	mpirun --mca plm_rsh_no_tree_spawn 1 --hostfile hosts bin/fractal "sample/pentagon" 8
levyDragon: clean build
	mpirun --mca plm_rsh_no_tree_spawn 1 --hostfile hosts bin/fractal "sample/levyDragon" 17	
christmasTree: clean build
	mpirun --mca plm_rsh_no_tree_spawn 1 --hostfile hosts bin/fractal "sample/twinChristmasTree" 7
twinDragon: clean build
	mpirun --mca plm_rsh_no_tree_spawn 1 --hostfile hosts bin/fractal "sample/twinDragon" 15
clean :
	rm -f bin/fractal *.o *.txt *.png
build:
	$(CC) $(CFLAGS) -g $(STATIC_LIB) $(CAIRO_LIB) $(GTK_LIB) -o bin/fractal
