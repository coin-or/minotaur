
CXX = g++
CXXFLAGS = -ggdb
FORTRAN = gfortran
OUT = knapsack

MINOTAUR_INCDIR = ../../build-k/include/minotaur
MINOTAUR_LIBDIR = ../../build-k/lib

BOOST_INCDIR = ../../build-debug/minotaur-externals

FILTER_LIB = ../../build-debug/minotaur-externals/filter-87/src/filterlib.a

.PHONY: default

default: knapsack

knapsack: knapsack.o main.o
	$(FORTRAN) knapsack.o main.o -L$(MINOTAUR_LIBDIR) -lmntrfiltersqp \
	-lminotaur $(FILTER_LIB) -lstdc++ -o $(OUT)

knapsack.o: knapsack.cpp
	$(CXX) -c $(CXXFLAGS) -I$(MINOTAUR_INCDIR) -I$(BOOST_INCDIR) \
	knapsack.cpp


main.o: main.f
	$(FORTRAN) -c main.f
	

clean:
	@rm -fv *.o $(OUT)
