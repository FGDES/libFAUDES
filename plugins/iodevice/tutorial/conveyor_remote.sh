#/bin/sh

# faudes simulator executable
SIMFAUDES=../../../bin/simfaudes


echo This script runs a supervisor to control two conveyor belts via a simplenet device

$SIMFAUDES -d data/conveyor_simplenet_remote.dev -l tmp_lrtcontrol.log -la data/conveyor_supervisor.sim


