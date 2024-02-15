#!/bin/sh

# simple script to run tutorials, create images, and to 
# install them in the libfaudes doxygen tree

# configure
LIBFAUDES=../../..

DOTWRITE=$LIBFAUDES/bin/gen2dot
LUAFAUDES=$LIBFAUDES/bin/luafaudes
DOTEXEC=dot 
CONVERT=convert
DSTDIR=$LIBFAUDES/plugins/synthesis/src/doxygen/faudes_images
GEN2REF=$LIBFAUDES/tools/gen2ref/gen2ref.pl

# advertise
echo ======================================================
echo ===  running tutorials ===============================
echo ======================================================

rm tmp_*

#$LUAFAUDES ./syn_1_simple.lua
#$LUAFAUDES ./syn_3_reduction.lua
#$LUAFAUDES ./syn_4_validate.lua
#$LUAFAUDES ./syn_5_stdcons.lua
#./syn_2_omega

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
#cp -v tmp_syn*.png $DSTDIR
#cp -v tmp_syn*.svg $DSTDIR
#cp -v tmp_syn*.fref $DSTDIR