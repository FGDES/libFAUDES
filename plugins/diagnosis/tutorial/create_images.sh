#!/bin/sh


# simple script to run tutorials, create images, and to 
# installs them in the libfaudes doxygen tree

# configure
LIBFAUDES=../../..

DOTWRITE=$LIBFAUDES/bin/gen2dot
DIAGDOTWRITE=$LIBFAUDES/bin/diag2dot
LUAFAUDES=$LIBFAUDES/bin/luafaudes
DOTEXEC=dot
CONVERT=convert
DSTDIR=$LIBFAUDES/plugins/diagnosis/src/doxygen/faudes_images
GEN2REF=$LIBFAUDES/tools/gen2ref/gen2ref.pl

# advertise
echo ======================================================
echo ===  running tutorials ===============================
echo ======================================================

rm -f tmp_*

./diag_1_eventdiagnosis
./diag_2_languagediagnosis
./diag_3_modulardiagnosis
./diag_4_decentralizeddiagnosis

# advertise
echo ======================================================
echo ===  converting gen to png/svg =======================
echo ======================================================

# loop all diagnoser .gen files
for FILE in tmp_diag_diagnoser*.gen ; do
  BASE=$(basename $FILE .gen)
  echo ============= gen processing $BASE
  $DIAGDOTWRITE $FILE
  $DOTEXEC -Tsvg -Gbgcolor=transparent -Gsize=10,10 $BASE.dot -o $BASE.svg
  $CONVERT -background none $BASE.svg $BASE.png
done;

# loop all system .gen files
for FILE in tmp_diag_system*.gen ; do
  BASE=$(basename $FILE .gen)
  echo ============= diag processing $BASE
  $DOTWRITE $FILE
  $DOTEXEC -Tsvg -Gbgcolor=transparent -Gsize=10,10 $BASE.dot -o $BASE.svg
  $CONVERT -background none $BASE.svg $BASE.png
done;

# loop all spec .gen files
for FILE in tmp_diag_spec*.gen ; do
  BASE=$(basename $FILE .gen)
  echo ============= diag processing $BASE
  $DOTWRITE $FILE
  $DOTEXEC -Tsvg -Gbgcolor=transparent -Gsize=10,10 $BASE.dot -o $BASE.svg
  $CONVERT -background none $BASE.svg $BASE.png
done;

# loop all .gen files for ref page
for FILE in tmp_diag*.gen ; do
  BASE=$(basename $FILE .gen)
  echo ============= processing $BASE
  $GEN2REF $BASE.gen > $BASE.fref
done;


# advertise
echo ======================================================
echo ===  copy do doc =====================================
echo ======================================================

rm $DSTDIR/tmp_diag*
cp -v tmp_diag_*.png $DSTDIR
cp -v tmp_diag_*.svg $DSTDIR
cp -v tmp_diag_*.fref $DSTDIR