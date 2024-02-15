#/bin/sh

# faudes simulator executable
IOBRIDGE=../../../bin/iobridge


echo This script runs an io bridge to map hardware io events from the lrt lab experiment to simplenet events

$IOBRIDGE data/conveyor_comedi.dev data/conveyor_simplenet_field.dev


