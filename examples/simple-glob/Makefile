# A (simple) Makefile for building minotaur executables.

CXX = g++
CXXFLAGS = -ggdb
LD_ADD = -llapack -lblas -lgfortran -ldl

MINOTAUR_INCDIR = ../../build-n3/include/minotaur
MINOTAUR_LIBDIR = ../../build-n3/lib

BOOST_INCDIR = /home/amahajan/projects/minotaur-release/t1/third-party/include

ASL_DIR = /home/amahajan/projects/minotaur-release/t1/third-party/lib/
ASL_LIB = $(ASL_DIR)/amplsolver.a
FILTER_LIB = /home/amahajan/projects/minotaur-release/t1/third-party/lib/libfiltersqp.a /home/amahajan/projects/minotaur-release/t1/third-party/lib/libbqpd.a
OSI_LIB_DIR = /home/amahajan/projects/minotaur-release/t1/third-party/lib

RPATH_FLAGS = -Wl,-rpath=${MINOTAUR_LIBDIR}

.PHONY: default

default: simple-glob

%: %.cpp
	$(CXX) $(CXXFLAGS) -I$(MINOTAUR_INCDIR) -I$(BOOST_INCDIR) \
	-I$(ASL_DIR) -L$(MINOTAUR_LIBDIR) $@.cpp -o $@ -lminotaur \
	-lmntrfiltersqp -lmntrbqpd -lmntrosilp \
	-lmntrampl \
	${FILTER_LIB} ${ASL_LIB} \
	-L $(OSI_LIB_DIR) -lOsiClp -lOsi -lClp -lCoinUtils \
	$(RPATH_FLAGS) $(LD_ADD)

