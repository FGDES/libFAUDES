#!/bin/sh

# simple script to run tutorials, create images, and to 
# install them in the libfaudes doxygen tree

# configure
LIBFAUDES=../../..

DOTWRITE=$LIBFAUDES/bin/gen2dot
LUAFAUDES=$LIBFAUDES/bin/luafaudes
DOTEXEC=dot
CONVERT=convert
DSTDIR=$LIBFAUDES/plugins/hiosys/src/doxygen/faudes_images
GEN2REF=$LIBFAUDES/tools/gen2ref/gen2ref.pl

# advertise
echo ======================================================
echo ===  running tutorials ===============================
echo ======================================================

$LUAFAUDES ./hio_1_introduction.lua
$LUAFAUDES ./hio_2_hiogenerators.lua
$LUAFAUDES ./hio_3_hiofunctions.lua
$LUAFAUDES ./hio_4_transport_unit.lua

## careful: takes about 3 hours:
## ./hio_5_conveyor_belts

# remove unused tmp results
rm -f tmp_actualspec.gen
rm -f tmp_controller.gen
rm -f tmp_extloop.gen

# advertise
echo ======================================================
echo ===  converting gen to png/svg/html ==================
echo ======================================================


# loop all .svg files
for FILE in tmp*.gen ; do
  BASE=$(basename $FILE .gen)
  echo ============= processing $BASE
  $DOTWRITE $FILE
  $DOTEXEC -Tsvg  -Gsize=10,10 $BASE.dot -o $BASE.svg
  $CONVERT -background none $BASE.svg $BASE.png
done;

# loop all .gen files for html page
for FILE in tmp*.gen ; do
  BASE=$(basename $FILE .gen)
  echo ============= processing $BASE
  $GEN2REF $BASE.gen > $BASE.fref
done;

# advertise
echo ======================================================
echo ===  copy to doc =====================================
echo ======================================================

cp -v tmp_hio*.svg $DSTDIR
cp -v tmp_hio*.fref $DSTDIR
cp -v tmp_hio*.png $DSTDIR
