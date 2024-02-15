#!/bin/bash

# upload script for decdemo-experiment
SRCFILES="../../../bin/iomonitor ../../../bin/simfaudes data/decdemo_* ../src/iop_serial.cpp ../src/iop_sdevice.cpp"
DSTDIR=/root/decdemo

scp $SRCFILES  root@satchmo:$DSTDIR
scp $SRCFILES  root@espr-i:$DSTDIR
scp $SRCFILES  root@chianti:$DSTDIR


