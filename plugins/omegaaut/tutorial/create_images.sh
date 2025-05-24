#!/bin/sh

# simple script to run tutorials, create images, and to 
# install them in the libfaudes doxygen tree

# sanity check
if [ ! -d ../../omegaaut/tutorial ]; then
    echo "error: the current directory appears not to be omegaaut/tutorial"
    return
fi


# configure
LIBFAUDES=../../..

DOTWRITE=$LIBFAUDES/bin/gen2dot
LUAFAUDES=$LIBFAUDES/bin/luafaudes
DOTEXEC=dot 
CONVERT=convert
DSTDIR=$LIBFAUDES/plugins/omegaaut/src/doxygen/faudes_images
GEN2REF=$LIBFAUDES/tools/gen2ref/gen2ref.pl

# advertise
echo ======================================================
echo ===  running tutorials ===============================
echo ======================================================

rm tmp_*

./omg_1_buechi
./omg_2_buechictrl


# advertise
echo ======================================================
echo ===  converting gen to png/svg/html ==================
echo ======================================================


# loop all .svg files
for FILE in tmp_*.gen ; do
  BASE=$(basename $FILE .gen)
  echo ============= processing $BASE
  $DOTWRITE $FILE
  $DOTEXEC -Efontname=Arial -Nfontname=Arial -Tsvg -Gbgcolor=transparent -Gsize=10,10 $BASE.dot -o $BASE.svg
  $CONVERT -background none $BASE.svg $BASE.png
done;

# loop all .gen files for ref page
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
cp -v tmp_omg*.png $DSTDIR
cp -v tmp_omg*.svg $DSTDIR
cp -v tmp_omg*.fref $DSTDIR
