# A (simple) Makefile for building minotaur executables on Linux.
# To build the binaries, the user should first install minotaur, then fill the following details.

CXX = g++
CXXFLAGS = -ggdb
LD_ADD = -lgfortran -llapack -ldl

# directory containing third-party libraries on which Minotaur depends
THIRDPARTY = ../../../third-party

# directories where Minotaur is installed
MINCDIR = ../../../build/include/minotaur
MLIBDIR = ../../../build/lib

#####################################################################
## Users should normally not edit below this line
#####################################################################




ASL_LIB = $(THIRDPARTY)/lib/amplsolver.a
FILTER_LIB = $(THIRDPARTY)/lib/libfiltersqp.a $(THIRDPARTY)/lib/libbqpd.a

RPATH_FLAGS = -Wl,-rpath=$(MLIBDIR):$(THIRDPARTY)/lib

.PHONY: default

default: simple-qg-cg simple-qg-nocg

%: %.cpp
	$(CXX) $(CXXFLAGS) -I$(MINCDIR) -I$(THIRDPARTY)/include/asl/ \
	-L$(MLIBDIR) \
	-L$(THIRDPARTY)/lib \
	 $@.cpp  -o $@ -lminotaur \
	 -lipopt -lOsiClp -lOsi -lClp -lCoinUtils -lcoinmumps \
	${FILTER_LIB} ${ASL_LIB} $(RPATH_FLAGS) $(LD_ADD)

clean:
	-rm -f simple-qg-cg simple-qg-nocg
