#!/bin/sh


# simple script to run tutorials, create images, and to 
# installs them in the libfaudes doxygen tree

# configure
LIBFAUDES=../../..

DOTWRITE=$LIBFAUDES/bin/gen2dot
LUAFAUDES=$LIBFAUDES/bin/luafaudes
DOTEXEC=dot
CONVERT=convert
DSTDIR=$LIBFAUDES/plugins/iodevice/src/doxygen/faudes_images
GEN2REF=$LIBFAUDES/tools/gen2ref/gen2ref.pl

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

# advertise
echo ======================================================
echo ===  converting gen to png/svg/html ==================
echo ======================================================

DOCU_GENS="elevator_plant_cabin elevator_plant_door elevator_plant_lbarrier elevator_spec_core elevator_super_core elevator_spec_cabmot elevator_spec_opcmd elevator_spec_cabmots elevator_spec_opcmds elevator_spec_scheds elevator_spec_drsaf1 elevator_spec_drsaf2 elevator_spec_drmot1 elevator_super_full decdemo_planta decdemo_plantb decdemo_contra decdemo_contrb decdemo_cloop decdemo_hcloop"

# loop all .gen files
for BASE in $DOCU_GENS ; do
  echo ============= processing $BASE
  $DOTWRITE tmp_$BASE.gen
  $DOTEXEC -Tsvg -Gbgcolor=transparent -Gsize=10,10 tmp_$BASE.dot -o tmp_$BASE.svg
  $CONVERT -background none tmp_$BASE.svg tmp_$BASE.png
done;

# loop all .gen files for gtml page
for BASE in $DOCU_GENS ; do
  echo ============= processing $BASE
  $GEN2REF tmp_$BASE.gen > tmp_$BASE.fref
done;


# advertise
echo ======================================================
echo ===  copy do doc =====================================
echo ======================================================

rm $DSTDIR/tmp_*
cp -v tmp_ele*.png $DSTDIR
cp -v tmp_ele*.svg $DSTDIR
cp -v tmp_ele*.fref $DSTDIR
cp -v tmp_decdemo*.png $DSTDIR
cp -v tmp_decdemo*.svg $DSTDIR
cp -v tmp_decdemo*.fref $DSTDIR