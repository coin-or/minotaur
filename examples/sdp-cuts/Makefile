# A (simple) Makefile for building minotaur executables.

#####################################################
### Edit the paths in this section. Then do 
### make 
#####################################################
MINOTAUR_INCDIR = ../../build-debug/include/minotaur
MINOTAUR_LIBDIR = ../../build-debug/lib

MINOTAUR_EXTDIR = ../../minotaur-externals/minotaur-externals/

CXX = g++
CXXFLAGS = -ggdb
LD_ADD = -llapack -ldl

#####################################################
### End of the section
#####################################################


BOOST_INCDIR = $(MINOTAUR_EXTDIR)
ASL_DIR = $(MINOTAUR_EXTDIR)/asl-20120126
ASL_LIB = $(ASL_DIR)/amplsolver.a
FILTER_LIB = $(MINOTAUR_EXTDIR)/filter-87/src/filterlib.a
RPATH_FLAGS = -Wl,-rpath=${MINOTAUR_LIBDIR}

.PHONY: default

default: relax

%: %.cpp
	$(CXX) $(CXXFLAGS) -I$(MINOTAUR_INCDIR) -I$(BOOST_INCDIR) \
	-I$(ASL_DIR) -L$(MINOTAUR_LIBDIR) $@.cpp -o $@ -lminotaur \
	-lmntrfiltersqp -lmntrbqpd \
	-lmntrampl \
	${FILTER_LIB} ${ASL_LIB} $(RPATH_FLAGS) $(LD_ADD)

