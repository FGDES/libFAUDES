#!/bin/bash

# convenience script to create images from tutorial output
#
# - uses the tools dot (from GraphViz) and convert (from ImageMagick) 
# - by convention, scripts produce tmp_*.gen output, so this is used as source
# - after processing, the tmp_ prefix is dropped 
# - overall effect
#      tmp_*.gen ===> *.svg and *.png
#

# configure
LIBFAUDES=../..

DOTWRITE=$LIBFAUDES/bin/gen2dot
LUAFAUDES=$LIBFAUDES/bin/luafaudes
DOTEXEC=dot
CONVERT=convert

# advertise
echo ======================================================
echo ===  running all provided tutorials ==================
echo ======================================================

TUTORIALS=$(ls *.lua)
for TUTORIAL in $TUTORIALS ; do
  echo ============= running $TUTORIAL
  $LUAFAUDES -d ./extintro_example.lua
done;

# advertise
echo ======================================================
echo ===  converting tmp_*.gen to png/svg =================
echo ======================================================


# loop all tmp_*.gen files to produce *.png and *.svg
FILES=$(ls tmp_*.gen)
for FILE in $FILES ; do
  TMPBASE=$(basename $FILE .gen)
  DSTBASE=$(echo $TMPBASE | sed 's/tmp_//')
  echo ============= processing $TMPBASE to $DSTBASE
  cp $TMPBASE.gen $DSTBASE.gen
  $DOTWRITE $DSTBASE.gen
  $DOTEXEC -Tsvg -Gbgcolor=transparent -Gsize=10,10 $DSTBASE.dot -o $DSTBASE.svg
  $CONVERT -background none $DSTBASE.svg $DSTBASE.png
  rm $DSTBASE.dot
  rm $TMPBASE.*
done;


# advertise
echo ============= done
echo ======================================================
