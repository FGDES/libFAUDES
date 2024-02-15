#!/bin/bash


# Script to spell check one gtml/html file with aspell
# 
# Notes:
# 1) run the script from the libfaudes root directory
# 2) specify the file to check as argument relative to libfaudes root
# 3) an md5 sum will be recorded to register the file for future spelling


# configure
LIBFAUDES=.
SPELL=$LIBFAUDES/tools/aspell
ASPELL="aspell --mode=html --personal=$SPELL/faudeswords.txt"
FILE=$1

# test/warn current directory
PWDBASE=$(basename $(pwd))
if test x$PWDBASE == x${PWDBASE#libfaudes} ; then { 
  echo error: you must make libfaudes the current directory to run this script;
  exit 1;
} fi

# test/warn file existence
if test -r $FILE; then {
  echo spellcheck $FILE; 
} else { 
  echo cannot read file; 
  exit 1;
} fi


# derive file key by
# 1) replace / by _
# 2) replace . by _
# 3) replace multiple _ by one _
# 4) remove leading _
FILEKEY=$(echo $FILE | sed -e s/\\//_/g -e s/\\./_/g -e s/__*_/_/g -e s/^_//g).md5
echo using filekey $FILEKEY

# run aspell
echo running aspell on $FILE
$ASPELL  check $1

# record md5 (extract first word of output, ie ignore filename)
#MD5SUM=$(md5sum $FILE | sed -e s/\ .*// )

# record md5 incl filename
MD5SUM=$(md5sum $FILE)
md5sum $FILE > $SPELL/$FILEKEY
echo record $MD5SUM as $FILEKEY
