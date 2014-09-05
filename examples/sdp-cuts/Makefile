# A (simple) Makefile for building minotaur executables.

#####################################################
### Edit the paths in this section. Then do 
### make 
#####################################################
#MINOTAUR_INCDIR = ../../build-debug/include/minotaur
MINOTAUR_INCDIR = /Users/hdong/workspace/Research/newbnb/build/include/minotaur
#MINOTAUR_LIBDIR = ../../build-debug/lib
MINOTAUR_LIBDIR = /Users/hdong/workspace/Research/newbnb/build/lib

#MINOTAUR_EXTDIR = ../../minotaur-externals/minotaur-externals/
MINOTAUR_EXTDIR = /Users/hdong/workspace/Research/newbnb/minotaur-0.1.1-dev-mac-x86_64/third-party

CXX = g++
CXXFLAGS = -ggdb 
LD_ADD = -lgfortran -llapack -ldl

#####################################################
### End of the section
#####################################################


BOOST_INCDIR = $(MINOTAUR_EXTDIR)
ASL_DIR = $(MINOTAUR_EXTDIR)/asl-20120126
ASL_LIB = $(ASL_DIR)/amplsolver.a
#FILTER_LIB = $(MINOTAUR_EXTDIR)/filter-87/src/filterlib.a
FILTER_LIB = $(MINOTAUR_EXTDIR)/filter-87/filterlib.a
#RPATH_FLAGS = -Wl,-rpath -Wl,${MINOTAUR_LIBDIR}
RPATH_FLAGS = -Wl,-rpath -Wl,${MINOTAUR_LIBDIR}
#RPATH_FLAGS = -Wl,-rpath,${MINOTAUR_LIBDIR}
#RPATH_FLAGS = -rpath=${MINOTAUR_LIBDIR}

.PHONY: all

all: relax

#%: %.cpp
#	$(CXX) $(CXXFLAGS) -I$(MINOTAUR_INCDIR) -I$(BOOST_INCDIR) \
#	-I$(ASL_DIR) -L$(MINOTAUR_LIBDIR) $@.cpp -o $@ -lminotaur \
#	-lmntrfiltersqp -lmntrbqpd \
#	-lmntrampl \
#	${FILTER_LIB} ${ASL_LIB} $(RPATH_FLAGS) $(LD_ADD)

relax: 
	$(CXX) $(CXXFLAGS) -I$(MINOTAUR_INCDIR) \
	-I$(ASL_DIR) -L$(MINOTAUR_LIBDIR) relax.cpp MIQCP.cpp CDlogdet.cpp -o nlbnb -lminotaur \
	-lmntrfiltersqp -lmntrbqpd \
	-lmntrampl -framework Accelerate \
	${FILTER_LIB} ${ASL_LIB} $(RPATH_FLAGS) $(LD_ADD)