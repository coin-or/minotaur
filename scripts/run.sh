#!/bin/bash

##############################################################################
# Run instances listed in a file and save the output to individual files

# Set the following parameters before running

OUTDIR=small-500
INSTDIR=/home/amahajan/instances/minlplib/nl              ## directory where input files are located
INSTLIST=/home/amahajan/instances/minlplib/minlplib.solu  ## which instances to run
EXEC='./bin/mglob --bnb_time_limit 120 --log_level 3 '    ## what to run
FILESUFF=".nl"                                            ## .nl or .mps etc
KILLAFTER=240                                             ## should be more than timelimit

# End of parameters

##############################################################################

echo setting hard time limit of $KILLAFTER
echo pwd is `pwd`
ulimit -S -t $KILLAFTER

if [ -d $OUTDIR ];
then
	echo writing log to $OUTDIR
else
	echo directory $OUTDIR not found
	exit 9
fi

for i in `cat $INSTLIST | awk '{print $2}'`
do
	echo $EXEC $INSTDIR/$i$FILESUFF
	$EXEC $INSTDIR/$i$FILESUFF > $OUTDIR/$i.out 2>> $OUTDIR/all.err
done


