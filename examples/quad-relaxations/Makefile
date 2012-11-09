# A (simple) Makefile for building minotaur executables.

CXX = g++
CXXFLAGS = -ggdb
LD_ADD = 

#AMPL_DIR = /scratch/jeff/minotaur-externals/asl
AMPL_DIR = /sandbox/mahajan/minotaur/build-debug/minotaur-externals/asl
AMPL_LIB = ${AMPL_DIR}/amplsolver.a

COIN_DIR = /sandbox/mahajan/minotaur/build-debug/minotaur-externals/osi-0.102.1/build
COIN_INC_DIR = ${COIN_DIR}/include/coin
COIN_LIB_DIR = ${COIN_DIR}/lib
COIN_LIB = -L${COIN_DIR}/lib -lOsiClp -lClp -lCoinUtils

IPOPT_DIR = /sandbox/mahajan/minotaur/build-debug/minotaur-externals/ipopt-3.8.1/build
IPOPT_INC_DIR = ${IPOPT_DIR}/include/coin
IPOPT_LIB_DIR = ${IPOPT_DIR}/lib

FILTER_DIR = /sandbox/mahajan/minotaur/build-debug/minotaur-externals/filter-sqp/src
FILTER_LIB = ${FILTER_DIR}/filterlib.a

MINOTAUR_INC = -I../../build-debug/include/minotaur/ \
	-I../../src/interfaces/ampl/ \
	-I../../src/engines/OsiLP/ \
	-I${AMPL_DIR}\
	-I${COIN_INC_DIR} \
	-I${IPOPT_INC_DIR}

MINOTAUR_LIB_DIR = ../../build-debug/lib

MINOTAUR_LIB = -L${MINOTAUR_LIB_DIR} -lminotaur -lmntrampl -lmntrfiltersqp -lmntripopt -lmntrosilp -L${IPOPT_LIB_DIR} -lipopt ${FILTER_LIB}

BR_SRCS := relax.cpp RelaxQuad.cpp
BR_OBJS := ${BR_SRCS:.cpp=.o}

CXXFLAGS += ${MINOTAUR_INC}
LD_ADD += ${MINOTAUR_LIB} ${AMPL_LIB} -Wl,-rpath=${MINOTAUR_LIB_DIR} -Wl,-rpath=${IPOPT_LIB_DIR} 

bilinear-relax: ${BR_OBJS} ${MINOTAUR_LIB_DIR}/libminotaur.so
	${CXX} -o relax ${BR_OBJS} ${CXXFLAGS} ${LD_ADD} 

.cpp.o:
	${CXX} ${CXXFLAGS} -c $< -o $@

clean:
	rm -fv *.o relax
