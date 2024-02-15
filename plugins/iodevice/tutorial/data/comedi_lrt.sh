#!/bin/sh

# This script configures the comedi kernel module to run the
# lrt lab experiment. It loads the module and configures our
# two advantech digital io cards as input and output device.
# respectively.
#
# The script requires root rights and thus should be enabled 
# by sudo. It tries to chnage ownership of the comedi devices
# in order to allow device access for simulator that is run by
# an ordinary user.

# example sudoers lines (use visodo to edit)
# User_Alias      TOYERS = tmoor, toy
# Cmnd_Alias      RUNTOY_ = /usr/local/bin/comedi_config, /usr/local/bin/comedi_lrt.sh
# TOYERS          toy = (root) NOPASSWD: RUNTOY_ 


DEVICE_A=/dev/comedi0
DEVICE_B=/dev/comedi1

echo configure comedi for lrt lab


echo 1. =========  loading module
/sbin/modprobe adv_pci_dio


echo 2. ========= setting up device 
comedi_config $DEVICE_A -r
comedi_config $DEVICE_B -r
comedi_config $DEVICE_A pci1754 2,5
comedi_config $DEVICE_B pci1752 2,9


echo 3a. ========= running test A
comedi_test --device $DEVICE_A
echo 3b. ========= running test B
comedi_test --device $DEVICE_B


echo 4a. ========== report
comedi_info -f $DEVICE_A
echo 4b. ========== report
comedi_info -f $DEVICE_B

echo 5. =========== steel
chown $SUDO_USER $DEVICE_A
chown $SUDO_USER $DEVICE_B

