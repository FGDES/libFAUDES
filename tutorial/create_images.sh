#!/bin/sh

# simple script to run tutorials, create images, and to 
# install them in the libfaudes doxygen tree

# note: this script co-exists in various variants around different plugins
# on some stage it should be unifed and go to tools -- tm 2025/07

# sanity check
if [ ! -d ../plugins/synthesis ]; then
    echo "error: the current path appears not to be a libFAUDES tutorial"
    return
fi


# configure
LIBFAUDES=..

DOTWRITE=$LIBFAUDES/bin/gen2dot
LUAFAUDES=$LIBFAUDES/bin/luafaudes
DOTEXEC=dot 
CONVERT=convert
DSTDIR=$LIBFAUDES/src/doxygen/faudes_images
GEN2REF=$LIBFAUDES/tools/gen2ref/gen2ref.pl

# advertise
echo ======================================================
echo ===  running tutorials ===============================
echo ======================================================

rm tmp_*

./3_functions
./1_generator

cp data/simplemachine.gen tmp_simplemachine.gen

# advertise
echo ======================================================
echo ===  converting gen to png/svg/fref ==================
echo ======================================================


# loop all .gen files for graphics
for FILE in tmp_*.gen ; do
  BASE=$(basename $FILE .gen)
  echo ============= processing $BASE
  $DOTWRITE $FILE
  $DOTEXEC -Efontname=Arial -Nfontname=Arial -Tsvg -Gbgcolor=transparent -Gsize=10,10 $BASE.dot -o $BASE.svg
  $DOTEXEC -Efontname=Arial -Nfontname=Arial -Tpng -Gbgcolor=transparent -Gsize=8,8 $BASE.dot -o $BASE.png
  #$CONVERT -background none $BASE.svg $BASE.png  # tends to mess with size/crop, tends to hickup on MacOS
done;

# loop all .gen files for fref page
for FILE in tmp_*.gen ; do
  BASE=$(basename $FILE .gen)
  echo ============= processing $BASE
  $GEN2REF $BASE.gen > $BASE.fref
done;

# advertise
echo ======================================================
echo ===  copy do doc =====================================
echo ======================================================

#rm $DSTDIR/tmp_*
#cp -v tmp_*.png $DSTDIR
#cp -v tmp_*.svg $DSTDIR
#cp -v tmp_*.fref $DSTDIR


