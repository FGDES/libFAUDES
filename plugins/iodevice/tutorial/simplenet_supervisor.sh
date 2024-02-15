#/bin/sh

# faudes simulator executable
SIMFAUDES=../../../bin/simfaudes


echo This script runs the simple machine supervisor with a simple net io device

$SIMFAUDES -d data/supervisor_simplenet.dev -l tmp_supervisor.log -la data/supervisor.sim 


