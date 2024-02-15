#/bin/sh

# faudes simulator executable
SIMFAUDES=../../../bin/simfaudes


echo This script runs two simple machines with a simple net io device

$SIMFAUDES -d data/machinesab_simplenet.dev -dr -l tmp_machinesab.log -la data/machinesab.sim 


