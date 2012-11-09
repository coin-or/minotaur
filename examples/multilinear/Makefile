# A (simple) Makefile for building minotaur executables.

CXX = g++
CXXFLAGS = -g
LD_ADD = 

#AMPL_DIR = /home/mahdi/solvers
#AMPL_DIR = /home/mahdi/Programs/2minotaur/trunk/src/minotaur/scripts/build/minotaur-externals/asl
#AMPL_DIR = /sandbox/mahajan/minotaur-test/minotaur-externals/asl
#AMPL_DIR = /scratch/jeff/minotaur-externals/asl
AMPL_DIR = /Users/linderoth/src/ASL/solvers
#AMPL_DIR=/home/jeff/ampl/solvers
AMPL_LIB = ${AMPL_DIR}/amplsolver.a

#COIN_DIR = /home/mahdi/Programs/2minotaur/trunk/src/minotaur/scripts/build/minotaur-externals/osi-0.102.1/build
#COIN_DIR = /home/mahdi/coin/Cbc2-4/builds/debug
#COIN_DIR = /home/vahid/coin/Cbc2-4/build/debug
#COIN_DIR = /sandbox/mahajan/minotaur-test/minotaur-externals/osi-0.102.1/build
#COIN_DIR = /u/l/i/linderot/svn-mods/coin-Cbc/Release/debug
COIN_DIR = /Users/linderoth/svn-mods/Bonmin-1.4/build

COIN_INC_DIR = ${COIN_DIR}/include/coin
COIN_LIB_DIR = ${COIN_DIR}/lib
#COIN_LIB = -L${COIN_DIR}/lib -lOsiClp -lClp -lOsi -lCoinUtils 
COIN_LIB = -L${COIN_DIR}/lib -lOsiClp -lClp -lOsi -lCoinUtils -lblas -llapack

BOOST_INC_DIR=/Users/linderoth/src/boost_1_46_1

MINOTAUR_INC = -I../../src/base/ \
	-I../../src/interfaces/ampl/ \
	-I../../src/engines/OsiLP/ \
	-I${AMPL_DIR}\
	-I${COIN_INC_DIR}\
	-I${BOOST_INC_DIR}

#MINOTAUR_LIB_DIR = ../../scripts/build/lib
MINOTAUR_LIB_DIR = ../../build/lib

MINOTAUR_LIB = -L${MINOTAUR_LIB_DIR} -lminotaur -lmntrampl -lmntrosilp

BR_SRCS := multilinear-relax.cpp
BR_OBJS := ${BR_SRCS:.cpp=.o}

CXXFLAGS += ${MINOTAUR_INC} 
#CXXFLAGS += -I../../scripts/build/minotaur-externals
#LD_ADD += ${MINOTAUR_LIB} ${AMPL_LIB} ${COIN_LIB} -Wl,-rpath=${MINOTAUR_LIB_DIR} -ldl  
#LD_ADD += ${MINOTAUR_LIB} ${AMPL_LIB} -Wl,-rpath=${MINOTAUR_LIB_DIR} ${COIN_LIB}   -Wl,-rpath=${COIN_LIB_DIR} -ldl

LD_ADD += ${MINOTAUR_LIB} ${AMPL_LIB} ${COIN_LIB}

#multilinear-relax: ${BR_OBJS} ${MINOTAUR_LIB_DIR}/libminotaur.so

multilinear-relax: ${BR_OBJS} ${MINOTAUR_LIB_DIR}/libminotaur.a
	${CXX} ${LD_ADD} -o multilinear-relax ${BR_OBJS} ${CXXFLAGS}

.cpp.o:
	${CXX} ${CXXFLAGS} -c $< -o $@

clean:
	rm -f ${BR_OBJS}
