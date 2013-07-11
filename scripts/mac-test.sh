#!/bin/bash

## The directory ${TEST_DIR} is deleted everyday and recreated.
## All builds will take place within ${TEST_DIR}
TEST_DIR=/Users/mahajan/src/minotaur-test

## svn info
USERNAME=ashu
SVN_REPOS="https://repo.anl-external.org/repos/minotaur"

## LOGS
WEB_HOST=login.mcs.anl.gov
WEB_DIR=/home/mahajan/public_html/minotaur/build-log/petsc-mini

## parallel flag
CPUS="1"

## summary file
SUMMARY="summary.log"

## where is doxygen, leave empty if it is already in PATH
DOXYGEN=

## where is cmake, leave empty if it is already in PATH
CMAKE=

## url where logs are accessible.
URL="http://www.mcs.anl.gov/~mahajan/minotaur/build-log/petsc-mini"

##########################################################################
# end of configurable options
##########################################################################

function doTest {
# empty $NAME is not allowed
if [ "x${NAME}" == "x" ]
then
  echo>&2 "NAME not defined for this test. Exiting."
  exit 1
fi

# empty OPTIONS is allowed
./scripts/build_with_externals -M . -d ./${NAME} -e ./minotaur-externals \
                               -j ${CPUS} -l ${NAME}.log -r ${NAME}.err   \
			       ${OPTIONS}
cd ${NAME}
make test >> ../${NAME}.log 2>> ../${NAME}.err
make install >> ../${NAME}.log 2>> ../${NAME}.err
cd - >> /dev/null

}

function checkTest {
okMatch=
okMatch=`grep -E -o "OK (.*)" ${NAME}.err`
if [[ x${okMatch} == "x" ]]
then
  echo make test output for ${NAME}: ERROR >> ${SUMMARY}
else
  echo make test output for ${NAME}: ${okMatch} >> ${SUMMARY}
fi
}

function listBins {
ls -lt ${NAME}/bin >> ${NAME}.log
ls -lt ${NAME}/lib >> ${NAME}.log
ls ${NAME}/include/minotaur/* >> ${NAME}.log
}

function testFiles {
files_exist=1
for f in ${FILES}
do
  if [ ! -e ${NAME}/${f} ]
  then
    files_exist=0
    break
  fi
done

if [ ${files_exist} -eq 1 ]
then
  echo "compiling ${NAME}: OK." >> ${SUMMARY}
else
  echo "compiling ${NAME}: ERROR. ${f} not built." >> ${SUMMARY}
fi
}

function addDoxygen {
if [[ x${DOXYGEN} == "x" ]]
then
  echo "Not using any user specified doxygen installation"
else
  export PATH=${DOXYGEN}:${PATH}
fi
}

function addCmake {
if [[ x${CMAKE} == "x" ]]
then
  echo "Not using any user specified cmake installation"
else
  export PATH=${CMAKE}:${PATH}
fi
}

##########################################################################
# Main
##########################################################################

# remove previous files
rm -rf ${TEST_DIR}

# add doxygen to path
addDoxygen

# add cmake to path
addCmake

# get latest version
svn co ${SVN_REPOS} ${TEST_DIR} &> svn.log

if [ -d ${TEST_DIR}/src ]
then
  echo "Minotaur src directory checked out."
else
  echo>&2 "Error checking out svn!"
  exit 1
fi

mv svn.log ${TEST_DIR}/
cd ${TEST_DIR}
NAME=
OPTIONS=
##########################################################################
## TEST 1
## Build all externals
##########################################################################
NAME=all_externals
./scripts/build_externals -d . -j ${CPUS} -l ${NAME}.log -r ${NAME}.err -O -C -s

##########################################################################
## TEST 2
## Build Minotaur base alone
##########################################################################

## debug
NAME=build-base-debug-static
FILES="lib/libminotaur.a"
OPTIONS=-fioabs
doTest; testFiles; checkTest

## static
NAME=build-base-static
OPTIONS=-fioas
FILES="lib/libminotaur.a"
doTest; testFiles; checkTest

##########################################################################
## TEST 3
## Build Minotaur with filter alone
##########################################################################
NAME=build-filter-static
OPTIONS=-ioas
FILES="lib/libminotaur.a lib/libmntrbqpd.a lib/libmntrfiltersqp.a"
doTest; testFiles; checkTest

## debug
NAME=build-filter-debug-static
OPTIONS=-ioabs
doTest; testFiles; checkTest

##########################################################################
## TEST 4
## Build Minotaur with ipopt alone
##########################################################################
NAME=build-ipopt-static
OPTIONS=-foas
FILES="lib/libminotaur.a lib/libmntripopt.a"
doTest; testFiles; checkTest

## static
NAME=build-ipopt-debug-static
OPTIONS=-foabs
doTest; testFiles; checkTest

##########################################################################
## TEST 5
## Build Minotaur with osi alone
##########################################################################
NAME=build-osi-static
OPTIONS=-ifas
FILES="lib/libminotaur.a lib/libmntrosilp.a"
doTest; testFiles; checkTest

## debug
NAME=build-osi-debug-static
OPTIONS=-ifabs
doTest; testFiles; checkTest


##########################################################################
## TEST 6
## Build Minotaur with ampl alone
##########################################################################
NAME=build-ampl-static
OPTIONS=-ifos
FILES="lib/libminotaur.a lib/libmntrampl.a"
doTest; listBins; testFiles; checkTest

## debug
NAME=build-ampl-debug-static
OPTIONS=-ifobs
doTest; listBins; testFiles; checkTest

##########################################################################
## TEST 7
## Build Minotaur with ampl + osi + ipopt + filter 
##########################################################################
NAME=build-all-static
OPTIONS=-s
FILES="lib/libminotaur.a lib/libmntrbqpd.a lib/libmntrampl.a lib/libmntrfiltersqp.a lib/libmntripopt.a lib/libmntrosilp.a"
doTest; listBins; testFiles; checkTest

## debug
NAME=build-all-debug-static
OPTIONS=-bs
doTest; listBins; testFiles; checkTest

## valgrind
#cd ${NAME}/src/testing
#valgrind --run-libc-freeres=no --leak-check=full --show-reachable=yes \
#.libs/lt-unittest all >> ../../../${NAME}.out 2>> ../../../${NAME}.err
#cd - >> /dev/null


##########################################################################
## Test manual build without automake
##########################################################################
## we build with -j 1, otherwise it doesn't build sometimes.
NAME=build-all-manual
OPTIONS=-x
## on petsc-mac mini:
export LDFLAGS=-L/usr/local/hpc-gf46n/lib

./scripts/build_with_externals -M . -d ./${NAME} -e ./minotaur-externals \
                               -j 1 -l ${NAME}.log -r ${NAME}.err  \
			       ${OPTIONS}
ls -lt ${NAME} >> ${NAME}.log
FILES="lib/libminotaur.a lib/libmntrbqpd.a lib/libmntrampl.a lib/libmntrfiltersqp.a lib/libmntripopt.a lib/libmntrosilp.a"
testFiles

