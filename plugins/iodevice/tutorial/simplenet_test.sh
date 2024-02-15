#/bin/sh

# netcat or nc executable
NETCAT=netcat

# faudes simulator executable
SIMFAUDES=../../../bin/simfaudes


echo This tutorial demonstrates the protocol of the simplenet IO device.
echo It uses the executor to simulate a simplemachine plant model
echo and accepts plant input events \(physically\: actuators\) on a
echo simplenet device. In turn, plant output events are reported
echo via a simplenet device. 
echo 
echo In order to inspect the protocol, two netcat processes are started
echo in separate xterm windows to report outputs and accept inputs, resp.
echo You may want to compile the iodevice plugin with debugging output
echo enabled to inspect network connection interna.
echo
echo To stop the simulation, cancel this script via ctrl-C,
echo and close the xterm windows
echo
echo Press Return to continue
read


echo 1. Connect to simulator device to report output events 

( sleep 5 ; xterm -T "Demo: Output Event Monitor" -e $NETCAT localhost  40000 ; )  &

echo 2. Open an input event server on a xterm

xterm -T "Demo: Kbd Input Event Server" -e $NETCAT -l -p 40001 &

echo 3. Advertise my input event server 

( sleep 10 ; echo "<Advert> \"simpleloop\" \"simplesupervisor\" \"localhost:40001\"  </Advert>" | $NETCAT -u localhost 40000 ; ) &

echo 4. Start simulation
$SIMFAUDES -d data/machine_simplenet.dev data/machine.sim   


