# Run '8 conveyor belts' example - HIL simulation. Because of the hardware dependence, this script should be run on the lrtlab PC 'toy' only.

# (Re-) initialize digital IO cards
sudo comedi_lrt.sh

echo '######################################'
echo '########### smallhio example #########'
echo '########### S:Perk, Nov 2009 #########'
echo '######################################'


SIMFAUDES=./simfaudes_toy_2009_2.13t # =libfaudes/bin/simfaudes from libfaudes "2009", Version 2.13t

echo 'Running simulation hio_cbl2r_3SENSORS.sim with device ../comedi_lrtlab.dev.'
echo 'Press Ctrl+C to stop.'

# standard for presentation - no console output
 $SIMFAUDES -qq -d comedi_lrtlab.dev hio_cbl2r_3SENSORS.sim &> /dev/null

# Console output, logfile, etc.:
# $SIMFAUDES -v -d comedi_lrtlab.dev hio_cbl2r_3SENSORS.sim
# $SIMFAUDES -q -le -l logfile.txt -d comedi_lrtlab.dev hio_cbl2r_3SENSORS.sim &> /dev/null