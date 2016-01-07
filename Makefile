CFLAGS=-Wall
CC=mpicc
CAIRO_LIB=`pkg-config --cflags --libs cairo`

all : clean build
	mpirun -n 4 bin/fractal 13 > output.txt
clean :
	rm -f bin/fractal *.o
build:
	$(CC) $(CFLAGS) source/fractal.c source/geometry.c source/utils.c -o bin/fractal $(CAIRO_LIB)
buildTest :
	$(CC) $(CFLAGS) source/test.c -o bin/test $(CAIRO_LIB)

test: buildTest
	mpirun -n 4 bin//test

download:
	#cairo
	sudo apt-get install -y libcairo2-dev
	#open mpi
	sudo apt-get install -y autotools-dev g++ build-essential openmpi-bin openmpi-doc libopenmpi-dev

install: download test
