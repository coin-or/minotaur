CC          := mpicxx
CFLAG       := -Wall 
HEADER      := ./include
BOOST       :=  /usr/include/boost/
COIN_HEADER :=  /sandbox/mahajan/minotaur/minotaur-externals/osi-0.102.1/build/include/coin/
LOCAL_LIB   := ./lib -L/sandbox/mahajan/minotaur/minotaur-externals/osi-0.102.1/build/lib

INCLUDE    = -I${HEADER} -I${COIN_HEADER} -I${BOOST}
LIB        = -L${LOCAL_LIB} -lpower -lOsiClp -lboost_mpi

all:sbdlp mylp solvemps pbdlp
# ========================================
# include the dependency file
# ========================================
-include .depend
depend: 
	g++ -MM *.cpp ${INCLUDE} > .depend

# ========================================
# define the suffix rules
# ========================================
pbdlp:pbdlp.o
	${CC} ${CFLAG} -o $@ $< ${INCLUDE} ${LIB}
solvemps:solvemps.o
	${CC} ${CFLAG} -o $@ $< ${INCLUDE} ${LIB}
mylp:mylp.o
	${CC} ${CFLAG} -o $@ $< ${INCLUDE} ${LIB}
sbdlp:sbdlp.o 
	${CC} ${CFLAG} -o $@ $< ${INCLUDE} ${LIB}
#	rm $^

%.o:%.cpp ./lib/libpower.a
	${CC} ${CFLAG} -c $< ${INCLUDE};


# ========================================
.PHONEY:clean depend push cleanall 
clean:
	rm -rf *~  sbdlp mylp solvemps pbdlp *.o  

