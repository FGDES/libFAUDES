#/bin/sh

# faudes simulator executable
SIMFAUDES=../../../bin/simfaudes


echo This script runs a supervisor to control two conveyor belts from the lrtlab setup

$SIMFAUDES -d data/conveyor_comedi.dev -l tmp_supervisor.log -la data/conveyor_supervisor.sim 


