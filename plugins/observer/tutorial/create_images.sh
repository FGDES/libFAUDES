#!/bin/sh


# simple script to run tutorials, create images, and to 
# installs them in the libfaudes doxygen tree

# configure
OBSERVER=../

DOTWRITE=../../../bin/gen2dot
DOTEXEC=dot
CONVERT=convert
DSTDIR=$OBSERVER/src/doxygen/faudes_images

# advertise
echo ======================================================
echo ===  running tutorials ===============================
echo ======================================================

./op_ex_synthesis


# advertise
echo ======================================================
echo ===  converting gen to png/svg =======================
echo ======================================================


# loop all .gen files
for FILE in data/ex_synthesis/*.gen ; do
  BASE=$(basename $FILE .gen)
  echo ============= processing $BASE
  $DOTWRITE $FILE
  $DOTEXEC -Tsvg -Gbgcolor=transparent -Gsize=7 data/ex_synthesis/$BASE.dot -o data/ex_synthesis/$BASE.svg
  $CONVERT -background none data/ex_synthesis/$BASE.svg data/ex_synthesis/$BASE.png
done;

# advertise
echo ======================================================
echo ===  copy do doc =====================================
echo ======================================================

cp -v data/ex_synthesis/*.png $DSTDIR