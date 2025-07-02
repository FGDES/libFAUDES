#!/bin/sh

# run relevant turtorials and process images for reference and doxugen

# configure
LIBFAUDES=../../..
LUAFAUDES=$LIBFAUDES/bin/luafaudes
GEN2DOT=$LIBFAUDES/bin/gen2dot
DSTDIR=../src/doxygen/faudes_images

# sanity check
if [ ! -d $DSTDIR ]; then
    echo "error: the current path appears not to be a libFAUDES tutorial"
    return
fi
if [ ! -f $LIBFAUDES/src/registry/cfl_definitions.rti ]; then
    echo "error: the current path appears not to be a libFAUDES tutorial"
    return
fi


# advertise
echo ======================================================
echo ===  running tutorials ===============================
echo ======================================================

rm tmp_*

$LUAFAUDES ./elevator_synthesis.lua
$LUAFAUDES ./decdemo_verify.lua

for FILE in data/elevator_*.gen ; do
  BASE=$(basename $FILE)
  cp $FILE tmp_$BASE
done;

rm tmp_elevator_plant_full.*
rm tmp_elevator_spec_full.*

# pass on to common image processing
. $LIBFAUDES/tools/imgproc/imgproc.sh


# advertise
echo ======================================================
echo ===  copy do doc =====================================
echo ======================================================

#rm $DSTDIR/tmp_*
cp -v tmp_*.png $DSTDIR
cp -v tmp_*.svg $DSTDIR
cp -v tmp_*.fref $DSTDIR
