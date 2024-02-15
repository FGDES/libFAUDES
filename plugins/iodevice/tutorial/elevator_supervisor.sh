# Execute elevator supervisor on harware plant
#
# This scipt is part of the HIL-simulation project "elevator". 
# 
# 

# Configure
SIMFAUDES=../../../bin/simfaudes
SIMFILE=date/elevator_supervisor.sim
DEVFILE=date/elevator_comedi.dev

echo Elevator Simulation
echo
echo This script runs a supervisor on the actual elevator setup, i.e. 
echo performs a hardware-in-the-loop simulation.
echo 
echo You must set up the comedi device according to your setup, e.g.
echo by the script /usr/local/bin/comedi_lrt.sh for the LRT lab.
echo 
echo
echo Files
echo - observe plant  "data/elevator_plant_cabin.gen"
echo - observe plant  "data/elevator_plant_door.gen"
echo - supervisor     "data/elevator_super_full.sim"
echo - device conf     "data/elevator_comedio.dev"
echo
echo You may specify additional simulator options, eg 
echo \  \"-q\"  for quiet operation
echo \  \"-v\"  for verbose operation
echo
echo Press Enter to proceed or ctrl-C to bail out.
read


# perform simulation
$SIMFAUDES -d $DEVFILE $@ $SIMFILE