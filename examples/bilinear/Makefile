# A (simple) Makefile for building minotaur executables.

CXX = g++
CXXFLAGS = -g
LD_ADD = 

#AMPL_DIR = /home/mahdi/solvers
AMPL_DIR = /home/vahid/minotaur/trunk/src/minotaur/scripts/build/minotaur-externals/asl
#AMPL_DIR = /sandbox/mahajan/minotaur/minotaur-externals/asl
AMPL_DIR = /scratch/jeff/minotaur-externals/asl
#AMPL_DIR = /home/jeff/ampl/solvers
AMPL_LIB = ${AMPL_DIR}/amplsolver.a

COIN_DIR = /home/vahid/minotaur/trunk/src/minotaur/scripts/build/minotaur-externals/osi-0.102.1/build
#COIN_DIR = /home/mahdi/coin/Cbc2-4/builds/debug
#COIN_DIR = /home/vahid/coin/Cbc2-4/build/debug

#COIN_DIR = /sandbox/mahajan/minotaur/minotaur-externals/osi-0.102.1/build
COIN_DIR = /u/l/i/linderot/svn-mods/coin-Cbc/Release/debug
COIN_INC_DIR = ${COIN_DIR}/include/coin
COIN_LIB_DIR = ${COIN_DIR}/lib
COIN_LIB = -L${COIN_DIR}/lib -lOsiClp -lClp -lOsi -lCoinUtils 


MINOTAUR_INC = -I../../src/base/ \
	-I../../src/interfaces/ampl/ \
	-I../../src/engines/OsiLP/ \
	-I${AMPL_DIR}\
	-I${COIN_INC_DIR}

MINOTAUR_LIB_DIR = ../../scripts/build/lib

MINOTAUR_LIB = -L${MINOTAUR_LIB_DIR} -lminotaur -lmntrampl -lmntrosilp

BR_SRCS := bilinear-relax.cpp
BR_OBJS := ${BR_SRCS:.cpp=.o}

CXXFLAGS += ${MINOTAUR_INC}
#LD_ADD += ${MINOTAUR_LIB} ${AMPL_LIB} ${COIN_LIB} -Wl,-rpath=${MINOTAUR_LIB_DIR} -ldl  
LD_ADD += ${MINOTAUR_LIB} ${AMPL_LIB} -Wl,-rpath=${MINOTAUR_LIB_DIR} ${COIN_LIB}   -Wl,-rpath=${COIN_LIB_DIR} -ldl
bilinear-relax: ${BR_OBJS} ${MINOTAUR_LIB_DIR}/libminotaur.so
	${CXX} -o bilinear-relax ${BR_OBJS} ${CXXFLAGS} ${LD_ADD} 

.cpp.o:
	${CXX} ${CXXFLAGS} -c $< -o $@
