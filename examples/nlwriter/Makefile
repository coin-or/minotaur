# A (simple) Makefile for building minotaur executables.

CXX = g++
CXXFLAGS = -ggdb
LD_ADD = -llapack -lblas -lgfortran -ldl

MNTR_INCDIR = ../../build-debug/include/minotaur
MNTR_LIBDIR = ../../build-debug/lib
TP_DIR = /home/amahajan/minotaur/third-party

LIBS = -L${MNTR_LIBDIR} -lmntripopt -lmntrfiltersqp -lmntrosilp -lmntrampl -lminotaur -lmntrengfac ${MNTR_LIBDIR}/libmntrbqpd.a -L${TP_DIR}/lib -lipopt -lcoinmumps -lOsiClp -lOsi -lClp ${TP_DIR}/lib/libfiltersqp.a ${TP_DIR}/lib/libbqpd.a ${TP_DIR}/lib/amplsolver.a 
INC = -I${TP_DIR}/include -I${TP_DIR}/include/asl

RPATH_FLAGS = -Wl,-rpath=${MNTR_LIBDIR} -Wl,-rpath=${TP_DIR}/lib

.PHONY: default

default: 
	cd /home/amahajan/minotaur/build-debug && make -j 8 install 
	$(CXX) $(CXXFLAGS) -I$(MNTR_INCDIR) $(INC) \
	-L$(MNTR_LIBDIR) test.cpp -o test \
	$(RPATH_FLAGS) ${LIBS} $(LD_ADD)

