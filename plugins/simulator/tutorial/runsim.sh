#/bin/sh

# faudes simulator executable
SIMFAUDES=../../../bin/simfaudes


# extract sim file
SIMFILE=$1
shift 1



echo This script runs $SIMFAUDES on the tutorial $SIMFILE
echo
echo You may specify additional options, eg 
echo \  \"-i\" for interactive or 
echo \  \"-bs nnn\"  to limit simulations to nnn steps
echo \  \"-q\"  for quiet operation
echo
echo Press Enter to proceed or ctrl-C to bail out.
read

$SIMFAUDES $@ $SIMFILE
