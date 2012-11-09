# A (simple) Makefile for building minotaur executables.

CXX = g++
CXXFLAGS = -g
LD_ADD = 

AMPL_DIR = /scratch/jeff/minotaur-externals/asl
AMPL_LIB = ${AMPL_DIR}/amplsolver.a

MINOTAUR_INC = -I../base -I../interfaces/ampl -I${AMPL_DIR}
MINOTAUR_LIB_DIR = ../../Release/debug/lib

MINOTAUR_LIB = -L${MINOTAUR_LIB_DIR} -lminotaur -lmntrampl

NSEPQ_SRCS := nsepq.cpp
NSEPQ_OBJS := ${NSEPQ_SRCS:.cpp=.o}

CXXFLAGS += ${MINOTAUR_INC}
LD_ADD += ${MINOTAUR_LIB} ${AMPL_LIB} -Wl,-rpath=${MINOTAUR_LIB_DIR}  

nsepq: ${NSEPQ_OBJS}
	${CXX} -o nsepq ${NSEPQ_OBJS} ${CXXFLAGS} ${LD_ADD} 

.cpp.o:
	${CXX} ${CXXFLAGS} -c $< -o $@