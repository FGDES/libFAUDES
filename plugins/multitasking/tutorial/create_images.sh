#!/bin/sh

# simple script to run tutorials, create images, and to 
# installs them in the libfaudes doxygen tree

# configure
LIBFAUDES=../../..

DOTWRITE=../../../bin/mtc2dot
DOTEXEC=dot
CONVERT=convert
DSTDIR=$LIBFAUDES/plugins/multitasking/src/doxygen/faudes_images
GEN2REF=$LIBFAUDES/tools/gen2ref/gen2ref.pl

# advertise
echo ======================================================
echo ===  running tutorials ===============================
echo ======================================================

rm -f tmp_*
./mtc_2_functions
./mtc_3_observer


# advertise
echo ======================================================
echo ===  converting gen to png/svg =======================
echo ======================================================


# loop all .gen files for graph
for FILE in tmp_mtc_*.gen ; do
  BASE=$(basename $FILE .gen)
  echo ============= processing $BASE
  $DOTWRITE $FILE
  $DOTEXEC -Tsvg -Gbgcolor=transparent -Gsize=10,10 $BASE.dot -o $BASE.svg
  $CONVERT -background none $BASE.svg $BASE.png
done;

# loop all .gen files for fref page
for FILE in tmp_mtc_*.gen ; do
  BASE=$(basename $FILE .gen)
  echo ============= processing $BASE
  $GEN2REF $BASE.gen > $BASE.fref
done;



# advertise
echo ======================================================
echo ===  copy do doc =====================================
echo ======================================================

cp -v tmp_mtc*.png $DSTDIR
cp -v tmp_mtc*.svg $DSTDIR
cp -v tmp_mtc*.fref $DSTDIR