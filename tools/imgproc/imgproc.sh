#!/bin/sh

# script to convert tmp_*.gen to tmp_*.png/svg/fref.
# - this is all path/name based, take care
# - current dir must be a libFAUDS tutorial dir

# test whether we are a pluging (in contrast to core faudes)
LIBFAUDES=../../..
if [ ! -d $LIBFAUDES/plugins ]; then
  LIBFAUDES=..
fi

# system tools
DOTEXEC=dot 
CONVERT=convert

# configure paths
if [ -z $GEN2DOT ]; then
  GEN2DOT=$LIBFAUDES/bin/gen2dot
fi   
LUAFAUDES=$LIBFAUDES/bin/luafaudes
GEN2REF=$LIBFAUDES/tools/gen2ref/gen2ref.pl

# confiugre dot
# - dot's idea of an image dimension is in the unit "inch x inch"
# - setting a high dpi valiue gives many "pixl x pixl"
# - since PNG have no meta info on DPI, the more "pxl x pxl" the larger
# - summary so far: adjust size of the image in a browser by setting "-Gdpi"
# - finetune with "-Gsize" to for automatic scaling once the size is exceeded
DOTOPTS_BASE="-Efontname=Arial -Nfontname=Arial -Gbgcolor=transparent -Gdpi=72"
DOTOPTS_NRML="$DOTOPTS_BASE -Gsize=12,12"
DOTOPTS_WIDE="$DOTOPTS_BASE -Gsize=10,10"


# sanity check
if [ ! -f $LIBFAUDES/src/registry/cfl_definitions.rti ]; then
    echo "imgproc: error: the current path appears not to be a libFAUDES tutorial"
    return
fi
if [ ! -f $GEN2DOT ]; then
    echo "imgproc error: gen2dot (or friends)  not found" $GEN2DOT
    return
fi



# advertise
echo ======================================================
echo ===  converting gen to png/svg/fref ==================
echo ======================================================


# loop all .gen files for graphics
for FILE in tmp_*.gen ; do
  BASE=$(basename $FILE .gen)
  echo ============= processing $BASE
  $GEN2DOT $FILE
  $DOTEXEC $DOTOPTS_WIDE -Tsvg $BASE.dot -o $BASE.svg
  $DOTEXEC $DOTOPTS_NRML -Tpng $BASE.dot -o $BASE.png_pre
  $CONVERT -trim $BASE.png_pre $BASE.png
  rm $BASE.png_pre
done;

# loop all .gen files for fref page
for FILE in tmp_*.gen ; do
  BASE=$(basename $FILE .gen)
  echo ============= processing $BASE
  $GEN2REF $BASE.gen > $BASE.fref
done;


