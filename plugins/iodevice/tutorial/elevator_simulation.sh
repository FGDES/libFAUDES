# Simulate elevator supervisor
#
# This scipt is part of the HIL-simulation project "elevator". 
# 
# 

# Configure
SIMFAUDES=../../../bin/simfaudes
SIMFILE=data/elevator_supervisor.sim

echo Elevator Simulation
echo
echo This script runs an interactive simulation of the supervisor for the 
echo elevator setup. The simulation also includes some plant components
echo to monitor the plant state.
echo
echo Sensor events are set to negative priority, hence, you must
echo trigger them explicitely. 
echo
echo Files
echo - observe plant  "data/elevator_plant_cabin.gen"
echo - observe plant  "data/elevator_plant_door.gen"
echo - supervisor     "data/elevator_super_full.sim"
echo
echo You may specify additional simulator options, eg 
echo \  \"-q\"  for quiet operation
echo \  \"-vv\"  for verbose operation
echo
echo Press Enter to proceed or ctrl-C to bail out.
read

echo $SIMFAUDES -v $@ -i $SIMFILE
$SIMFAUDES -v $@ -i $SIMFILE



