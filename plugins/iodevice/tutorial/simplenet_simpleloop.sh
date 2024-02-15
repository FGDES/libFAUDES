#/bin/sh


# faudes simulator executable
SIMFAUDES=../../../bin/simfaudes


echo This tutorial runs a closed loop configuration with a simplemachine
echo as plant model and a simplesupervisor as controller. Both are equipped
echo with a simplenet device to serve relevant sensor events to each other.
echo
echo Execution is logged in tmp_ files.
echo
echo To stop the simulation, close the extra xtrem windows.
echo Press Return to continue
read

echo 1. Start the plant model on a separate xterm

xterm -T "Demo: Simplemachine Plant Model" -e $SIMFAUDES -d data/machine_simplenet.dev -l tmp_simplemachine.log -la data/machine.sim &

echo 2. Wait to make sure the plant is ready

sleep 5

echo 3. Start the supervisor on a separate xterm

xterm -T "Demo: Simplesupervisor" -e $SIMFAUDES -d data/supervisor_simplenet.dev -l tmp_simplesupervisor.log -la data/supervisor.sim &


