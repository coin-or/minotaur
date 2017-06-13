
CXX = g++
CXXFLAGS = -ggdb
FORTRAN = gfortran
OUT = knapsack

THIRDPARTY = ../../release/third-party-0.2.1-linux-x86_64
MINOTAUR_INCDIR = ../../build-debug/include/minotaur
MINOTAUR_LIBDIR = ../../build-debug/lib

BOOST_INCDIR = $(THIRDPARTY)

FILTER_LIB = $(THIRDPARTY)/lib/libfiltersqp.a $(THIRDPARTY)/lib/libbqpd.a

.PHONY: default

default: knapsack

knapsack: knapsack.o main.o
	$(FORTRAN) knapsack.o main.o -L$(MINOTAUR_LIBDIR) -lmntrfiltersqp \
	-lminotaur $(FILTER_LIB) -lstdc++ -llapack -lblas -o $(OUT)

knapsack.o: knapsack.cpp
	$(CXX) -c $(CXXFLAGS) -I$(MINOTAUR_INCDIR) -I$(BOOST_INCDIR) \
	knapsack.cpp


main.o: main.f
	$(FORTRAN) -c main.f
	

clean:
	@rm -fv *.o $(OUT)
