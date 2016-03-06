# A (simple) Makefile for building minotaur executables.

CXX = g++
CXXFLAGS = -static
LD_ADD = 

MINOTAUR_BUILD_DIR = ../../build-debug-static
THIRD_PARTY_DIR = /home/amahajan/minotaur/third-party/

MINOTAUR_INCDIR = ${MINOTAUR_BUILD_DIR}/include/minotaur
MINOTAUR_LIBDIR = ${MINOTAUR_BUILD_DIR}/lib

RPATH_FLAGS = -Wl,-rpath=${MINOTAUR_LIBDIR} -Wl,-rpath=${THIRD_PARTY_DIR}/lib

.PHONY: default

default: qp2octave

qp2octave: qp2octave.cpp
	$(CXX) $(CXXFLAGS) $(RPATH_FLAGS) -I$(MINOTAUR_INCDIR) \
	-I$(THIRD_PARTY_DIR)/include \
	-I$(THIRD_PARTY_DIR)/include/coin \
	-L$(MINOTAUR_LIBDIR) $@.cpp -o $@ -lmntrampl -lminotaur \
	$(THIRD_PARTY_DIR)/lib/amplsolver.a -llapack -lblas -ldl 
	


