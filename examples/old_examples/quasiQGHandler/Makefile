XXFLAGS = -ggdb
LD_ADD = 
#-L/home/local/MORGRIDGE/mhamzeei/svn-mods/minotaur/trunk/src/minotaur/scripts/build/minotaur-externals/ipopt-3.8.3/build/ThirdParty/Blas/

HEADER = /home/mahdi/Programs/minotaur/trunk/src/minotaur/examples/quasiQGHandler/include

#Mahdi's MIN_BASE
MIN_BASE = /home/mahdi/Programs/minotaur/trunk/src/minotaur/scripts/build

#MIN_BASE = ../../build-debug

BOOST_INCDIR = ${MIN_BASE}/minotaur-externals

#paths to AMPL library and include directory
AMPL_DIR = ${MIN_BASE}/minotaur-externals/asl
AMPL_LIB = ${AMPL_DIR}/amplsolver.a
AMPL_INC = ${AMPL_DIR}

#paths to IPOPT library and include directory
IPOPT_INC = ${MIN_BASE}/minotaur-externals/ipopt-3.8.3/build/include/coin
IPOPT_LIB_DIR = ${MIN_BASE}/minotaur-externals/ipopt-3.8.3/build/lib
IPOPT_LIB = -L${IPOPT_LIB_DIR} -lipopt -Wl,-rpath=${IPOPT_LIB_DIR}

#paths to FilterSQP library and include directory
FILTER_LIB = ${MIN_BASE}/minotaur-externals/filter-87/src/filterlib.a

#paths to OSI library
OSI_LIB_DIR = ${MIN_BASE}/minotaur-externals/osi-0.102.1/build/lib
OSI_LIB = -L${OSI_LIB_DIR} -lOsiClp -lOsiCpx -lOsi -lClp -lCoinUtils -lpthread -Wl,-rpath=${OSI_LIB_DIR}
 
CPLEX_DIR =/home/mahdi/Programs/ilog/cplex/include/ilcplex
CPLEXLIB_DIR =-L/home/mahdi/Programs/ilog/cplex/lib/x86-64_sles10_4.1/static_pic -lcplex

#paths to MINOTAUR library and include directory
MINOTAUR_INC = ${MIN_BASE}/include/minotaur
MINOTAUR_LIB_DIR = ${MIN_BASE}/lib
###############################################################################

#<<<<<<< .mine
#MINOTAUR_LIB = -L${MINOTAUR_LIB_DIR} -lminotaur -lmntrampl -lmntrbqpd -lmntrfiltersqp -lmntrosilp -lmntripopt -lmntrengfac
#=======
MINOTAUR_LIB = -L${MINOTAUR_LIB_DIR}  -lmntrampl -lmntrengfac -lmntrbqpd -lmntrosilp -lmntrfiltersqp -lmntripopt -lminotaur -Wl,-rpath=${MINOTAUR_LIB_DIR}

#MINOTAUR_LIB = -L${MINOTAUR_LIB_DIR} -lmntrampl -lmntrengfac -lmntrbqpd -lmntrfiltersqp -lmntrosilp -lmntripopt -lminotaur -Wl,-rpath=${MINOTAUR_LIB_DIR}
#>>>>>>> .r1974


#qg_SRCS := qg.cpp quasiQGHandler.cpp
#qg_OBJS := ${qg_SRCS:.cpp=.o}

#CXXFLAGS += -I${MINOTAUR_INC} -I${BOOST_INCDIR} -I${AMPL_INC} -I${IPOPT_INC} -I${CPLEX_DIR} 
#LD_ADD += ${MINOTAUR_LIB} ${AMPL_LIB} ${FILTER_LIB} ${OSI_LIB} ${IPOPT_LIB} ${CPLEXLIB_DIR}

#qg: ${qg_OBJS} 	
#	${CXX} -o qg ${qg_OBJS} ${CXXFLAGS} ${LD_ADD}

TEST_SRCS := qg.cpp quasiQGHandler.cpp
TEST_OBJS := ${TEST_SRCS:.cpp=.o}

CXXFLAGS += -I${MINOTAUR_INC} -I${BOOST_INCDIR} -I${AMPL_INC} -I${IPOPT_INC} -I${CPLEX_DIR}
LD_ADD += ${MINOTAUR_LIB} ${AMPL_LIB} ${FILTER_LIB} ${OSI_LIB} ${IPOPT_LIB} ${CPLEXLIB_DIR}
#LD_ADD += -ldl -L/home/local/MORGRIDGE/mhamzeei/doc/lapack-3.4.0/lib -llapack 
#LD_ADD += -L/home/local/MORGRIDGE/mhamzeei/doc/lapack-3.4.0/lib -lblas

qg: ${TEST_OBJS} 
	${CXX} -o qg ${TEST_OBJS} ${XXFLAGS} ${CXXFLAGS}  ${LD_ADD}

.cpp.o:
	${CXX} ${XXFLAGS} ${CXXFLAGS} -c $< -o $@

clean:
	rm -f *.o qg
