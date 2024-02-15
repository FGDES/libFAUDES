# elementary scripts to search and replace symbolic names
#
# this variant was used to rename classes in libFAUDES 2.20b

# record location
export here=$(pwd)

# process one file
processfile() {
  f=$(pwd)/$1
  echo ============= $f
  sed -e s/TmtcGenerator/TmtcXXNEWSYMBOL20XXGenerator/g -i "" $f
  sed -e s/mtcGenerator/MtcSystem/g -i "" $f
  sed -e s/TdiagGenerator/TdiagXXNEWSYMBOL20XXGenerator/g -i "" $f
  sed -e s/diagGenerator/Diagnoser/g -i "" $f
  sed -e s/TtGenerator/TtXXNEWSYMBOL20XXGenerator/g -i "" $f
  sed -e s/tGenerator/TimedGenerator/g -i "" $f
  sed -e s/TcGenerator/TcXXNEWSYMBOL20XXGenerator/g -i "" $f
  sed -e s/cGenerator/System/g -i "" $f
  sed -e s/TIoSystem/TioXXNEWSYMBOL20XXGenerator/g -i "" $f
  sed -e s/cGeneratorVector/SystemVector/g -i "" $f
  sed -e s/cEventSetVector/AlphaberVector/g -i "" $f
  sed -e s/cEventSet/Alphabet/g -i "" $f
  sed -e s/XXNEWSYMBOL20XX//g -i "" $f

  if [ $1 != "cfl_generator.cpp" ] ; then
  if [ $1 != "cfl_generator.h" ] ; then
    sed -e s/vGenerator/Generator/g -i "" $f
  fi
  fi
}

# recursion over all files incl subdirectories
processall() {
  for d in *; do
    ## skip empty dirs
    if [ $d == "*" ]; then
      continue
    fi
    ## skip lib dirs
    if [ $d == "tools" ]; then
      continue
    fi
    ## skip lib dirs
    if [ $d == "lib" ]; then
      continue
    fi
    ## skip lib dirs
    if [ $d == "wago" ]; then
      continue
    fi
    ## skip lib dirs
    if [ $d == "fsmsynth" ]; then
      continue
    fi
    ## recursion on directories
    if [ -d $d ]; then
      (cd $d; processall)
      continue
    fi
    ## extract extension
    base=$(basename $d)
    ext=`echo $base | sed -e 's/\(.*\\.\)//g'`
    base=`echo $base | sed -e 's/\(.*\)\..*/\1/g'`
    ## insist in file types
    if [ $ext != "cpp" ]; then
    if [ $ext != "h" ]; then
    if [ $ext != "i" ]; then
    if [ $ext != "rti" ]; then
      continue
    fi
    fi
    fi
    fi
    
    ## do process
    processfile $d
  done
}


# call script
cd $here/libfaudes_s2
processall
cd $here


echo ===== count $fcount