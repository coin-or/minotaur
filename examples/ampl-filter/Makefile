# A (simple) Makefile for building minotaur executables.

CXX = g++
CXXFLAGS = -ggdb
LD_ADD = -llapack -ldl

MINOTAUR_INCDIR = ../../build-debug/include
MINOTAUR_LIBDIR = ../../build-debug/lib

BOOST_INCDIR = ../../build-debug/minotaur-externals

ASL_DIR = ../../build-debug/minotaur-externals/asl
ASL_LIB = ../../build-debug/minotaur-externals/asl/amplsolver.a
FILTER_LIB = ../../build-debug/minotaur-externals/filter-87/src/filterlib.a

RPATH_FLAGS = -Wl,-rpath=${MINOTAUR_LIBDIR}

.PHONY: default

default: bqpd-qp filter-qp

%: %.cpp
	$(CXX) $(CXXFLAGS) -I$(MINOTAUR_INCDIR) -I$(BOOST_INCDIR) \
	-I$(ASL_DIR) -L$(MINOTAUR_LIBDIR) $@.cpp -o $@ -lminotaur \
	-lmntrfiltersqp -lmntrbqpd -lmntrampl \
	${FILTER_LIB} ${ASL_LIB} $(RPATH_FLAGS) $(LD_ADD)

