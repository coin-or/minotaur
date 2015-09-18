# A (simple) Makefile for building minotaur executables.

CXX = g++
CXXFLAGS = -O3
LD_ADD = -llapack -lblas -lgfortran -ldl

MINOTAUR_INCDIR = ../../run-156/include/minotaur
MINOTAUR_LIBDIR = ../../run-156/lib

BOOST_INCDIR = /home/amahajan/minotaur/third-party/include

ASL_DIR = /home/amahajan/minotaur/third-party/lib/
ASL_LIB = $(ASL_DIR)/amplsolver.a
IPOPT_LIB_DIR = /home/amahajan/minotaur/third-party/lib
IPOPT_LIB = $(IPOPT_LIB_DIR)/libipopt.a $(IPOPT_LIB_DIR)/libcoinmumps.a

RPATH_FLAGS = -Wl,-rpath=${MINOTAUR_LIBDIR}

.PHONY: default

default: simple-bnb-cg simple-bnb-nocg

%: %.cpp
	$(CXX) $(CXXFLAGS) -I$(MINOTAUR_INCDIR) -I$(BOOST_INCDIR) \
	-I$(ASL_DIR) -L$(MINOTAUR_LIBDIR) $@.cpp -o $@ -lminotaur \
	-lmntripopt \
	-lmntrampl \
	${IPOPT_LIB} ${ASL_LIB} $(RPATH_FLAGS) $(LD_ADD)

