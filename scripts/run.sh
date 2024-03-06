#!/bin/bash

##############################################################################
# Run instances listed in a file and save the output to individual files

# Set the following parameters before running

OUTDIR=/home/23m1523/minotaur/scripts/test1						## directory where output files are stored
INSTDIR=/home/23m1523/minlplib              						## directory where input files are located
INSTLIST=/home/23m1523/minotaur/scripts/instances.csv 					## which instances to run
EXEC='/home/23m1523/minotaur/build/bin/mglob --bnb_time_limit 600 --log_level 3'	## what to run
FILESUFF=".nl"                                            				## .nl or .mps etc
KILLAFTER=5                                             				## should be more than timelimit
NCPUS=10
MEMLIM=20000000                                                                         ## Max memory to use per process in kB

# End of parameters

##############################################################################

echo setting hard time limit of $KILLAFTER
echo pwd is `pwd`


if [ -d $OUTDIR ];
then
	echo writing log to $OUTDIR
else
	echo directory $OUTDIR not found
	exit 9
fi

cat $INSTLIST | awk '{print $1}' | parallel --timeout $KILLAFTER --eta -j $NCPUS "(echo {} start ; ulimit -S -v $MEMLIM; $EXEC $INSTDIR/{}$FILESUFF > $OUTDIR/{}.out 2>> $OUTDIR/{}.err)" 

# { print $column number} - column number in file 


