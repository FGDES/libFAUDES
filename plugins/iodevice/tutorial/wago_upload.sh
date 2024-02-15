# Upload to ipc
#

# Absolut path to libfaudes and device base-directory
LIBFAUDES=../../..
IODEVICE=$LIBFAUDES/plugins/iodevice
WAGODEVICE=$LIBFAUDES/plugins/iodevice


# Make objects
echo compile
make -C $LIBFAUDES bin/iomonitor
make -C $LIBFAUDES bin/simfaudes


# Safe ipc memory
echo strip binaries
strip $LIBFAUDES/bin/iomonitor
strip $LIBFAUDES/bin/simfaudes


# Export binary to WAGO-IPC
# Note: ftp uses ~/.netrc for wago userid/passwd 
# Example: machine 131.188.131.250 login root password wago+++++
echo ftp transfer
ftp 131.188.131.250 <<EOF
prompt
put $LIBFAUDES/bin/iomonitor   /home/faudes/bin/iomonitor
put $LIBFAUDES/bin/simfaudes   /home/faudes/bin/simfaudes
cd /home/faudes/data
lcd $WAGODEVICE/tutorial/data
mput *
bye

EOF