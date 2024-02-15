#!/bin/bash


# Script to spell check all registered files that changed since last check
# 
# Notes:
# 1) run the script from the libfaudes root directory


# configure
LIBFAUDES=.
SPELL=$LIBFAUDES/tools/aspell

# test/warn current directory
PWDBASE=$(basename $(pwd))
if test x$PWDBASE == x${PWDBASE#libfaudes} ; then { 
  echo error: you must make libfaudes the current directory to run this script;
  exit 1;
} fi


# check files for changes and run spellcheck
for MD5 in $SPELL/*.md5; do {
  FILE=$(cat $MD5 | sed -e s/.*\ //)
  if md5sum --check --status $MD5; then {
    echo $FILE unchanged w.r.t. $MD5
  } else {
    echo $FILE changed since last spellcheck -- process file ? [Y/n];
    OK=$(read)
    if test x$OK != xn; then $SPELL/aspell_file.sh $FILE; fi
  } fi
} done

