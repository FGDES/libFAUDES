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

#DOCU_GENS="elevator_plant_cabin elevator_plant_door elevator_plant_lbarrier elevator_spec_core elevator_super_core elevator_spec_cabmot elevator_spec_opcmd elevator_spec_cabmots elevator_spec_opcmds elevator_spec_scheds elevator_spec_drsaf1 elevator_spec_drsaf2 elevator_spec_drmot1 elevator_super_full decdemo_planta decdemo_plantb decdemo_contra decdemo_contrb decdemo_cloop decdemo_hcloop"

## loop all .gen files
#for BASE in $DOCU_GENS ; do
#  echo ============= processing $BASE
#  $DOTWRITE tmp_$BASE.gen
#  $DOTEXEC -Tsvg -Gbgcolor=transparent -Gsize=10,10 tmp_$BASE.dot -o tmp_$BASE.svg
#  $CONVERT -background none tmp_$BASE.svg tmp_$BASE.png
#done;

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
