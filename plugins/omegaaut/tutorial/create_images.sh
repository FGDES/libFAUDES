#!/bin/sh

# run relevant turtorials and process images for reference and doxugen

# configure
LIBFAUDES=../../..
LUAFAUDES=$LIBFAUDES/bin/luafaudes
GEN2DOT=$LIBFAUDES/bin/waut2dot
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

./omg_1_buechi
./omg_2_buechictrl
./omg_3_rabin
./omg_4_rabinctrl
. ./safra.sh
rm tmp_omg*.png

echo "=== tutorial: done (no error check)"
echo

# pass on to common image procssing
. $LIBFAUDES/tools/imgproc/imgproc.sh


# advertise
echo ======================================================
echo ===  copy do doc =====================================
echo ======================================================

rm $DSTDIR/tmp_*
cp -v tmp_omg*.png $DSTDIR
cp -v tmp_omg*.svg $DSTDIR
cp -v tmp_omg*.fref $DSTDIR
