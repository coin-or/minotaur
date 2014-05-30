# A (simple) Makefile for building minotaur executables.

CXX = g++
CXXFLAGS = -ggdb
LD_ADD = -llapack -ldl

MINOTAUR_INCDIR = ../../build-debug/include/minotaur
MINOTAUR_LIBDIR = ../../build-debug/lib

BOOST_INCDIR = ../../minotaur-externals/minotaur-externals/

ASL_DIR = ../../minotaur-externals/minotaur-externals/asl-20120126
ASL_LIB = $(ASL_DIR)/amplsolver.a
FILTER_LIB = ../../minotaur-externals/minotaur-externals/filter-87/src/filterlib.a

RPATH_FLAGS = -Wl,-rpath=${MINOTAUR_LIBDIR}

.PHONY: default

default: simple-bnb

%: %.cpp
	$(CXX) $(CXXFLAGS) -I$(MINOTAUR_INCDIR) -I$(BOOST_INCDIR) \
	-I$(ASL_DIR) -L$(MINOTAUR_LIBDIR) $@.cpp -o $@ -lminotaur \
	-lmntrfiltersqp -lmntrbqpd \
	-lmntrampl \
	${FILTER_LIB} ${ASL_LIB} $(RPATH_FLAGS) $(LD_ADD)

