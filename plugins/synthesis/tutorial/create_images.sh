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
echo ===  running tutorials to generate .gen files ========
echo ======================================================

rm tmp_*

$LUAFAUDES ./syn_1_simple.lua
$LUAFAUDES ./syn_3_reduction.lua
$LUAFAUDES ./syn_4_validate.lua
$LUAFAUDES ./syn_5_stdcons.lua

# pass on to common image processing
. $LIBFAUDES/tools/imgproc/imgproc.sh

# advertise
echo ======================================================
echo ===  copy to doc =====================================
echo ======================================================

#rm $DSTDIR/tmp_*
cp -v tmp_*.png $DSTDIR
cp -v tmp_*.svg $DSTDIR
cp -v tmp_*.fref $DSTDIR




