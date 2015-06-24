#!/bin/bash

## LOGS
WEB_DIR=/mcs/web/research/projects/minotaur/nightly/build-log/petsc-mini

LINE="--------------------------------------------------"

echo "Minotaur daily mac test report"
date
echo "Detailed log available at " \
     "http://www.mcs.anl.gov/minotaur/nightly/build-log/petsc-mini"

echo ""
ssh petsc-mini-nightly-test

rm -rf ${WEB_DIR}
mkdir ${WEB_DIR}
rsync -a petsc-mini-rsync:src/minotaur-test/ ${WEB_DIR}/
cat ${WEB_DIR}/build-*err > ${WEB_DIR}/minotaur.err

echo ""
echo "Summary"
echo ${LINE}
cat ${WEB_DIR}/summary.log

echo ""
echo "Errors:"
echo ${LINE}

for f in ${WEB_DIR}/build-*err
do
	echo " "
	echo Errors in `basename ${f} .err`:
	echo ${LINE}
	cat ${f}
done


echo ""
echo "End of tests"
date
echo ${LINE}

