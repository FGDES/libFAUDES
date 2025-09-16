# elementary scripts to search and replace symbolic names
#
# this variant was used to rename synthesis functions in libFAUDES 2.33c
#
# note: this is recursive batch editing (!!!)
# - have a dry run before
# - check which files would be affected "grep -r -l -I SupTconNB plugins/*/src/* ./src/*"
# - make sure, that you are in a libFAUDES source tree 
# - no warranties, double check befor usage

# record location
export here=$(pwd)

# process one file
processfile() {
  f=$(pwd)/$1
  echo ============= $f
  #sed -i "" -e s/SupConNB/SupCon/g  $f
  #sed -i "" -e s/SupConCmplNB/SupConCmpl/g  $f
  #sed -i "" -e s/SupConNormNB/SupConNorm/g  $f
  #sed -i "" -e s/SupConNormCmplNB/SupConNormCmpl/g  $f
  #sed -i "" -e s/SupTconNB/SupTcon/g  $f
  sed -i "" -e s/\"S6\"/\"S3\"/g  $f
  sed -i "" -e s/\"S7\"/\"S4\"/g  $f
  sed -i "" -e s/\"S8\"/\"S5\"/g  $f
  sed -i "" -e s/\"S9\"/\"S6\"/g  $f
  sed -i "" -e s/\"S10\"/\"S7\"/g  $f
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
    #if [ $d == "fsmsynth" ]; then
    #  continue
    #fi
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
    if [ $ext != "fref" ]; then
      continue
    fi
    
    ## do process
    processfile $d
  done
}


# call script
processall
cd $here


echo ===== count $fcount
