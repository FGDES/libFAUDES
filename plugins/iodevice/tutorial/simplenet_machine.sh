#/bin/sh

# faudes simulator executable
SIMFAUDES=../../../bin/simfaudes


echo This script runs a simple machine with a simple net io device

$SIMFAUDES -d data/machine_simplenet.dev -dr -l tmp_simplemachine.log -la data/machine.sim 


