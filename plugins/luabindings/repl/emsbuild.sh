#!/bin/bash

# shell script to build luafaudes with target node-js

FAUDES_SRC=../../../
source ~/opt/emsdk/emsdk_env.sh

############################################################################
# some consistency tests
if [ ! -d ../repl ]; then
    echo "error: the current directory appears not to be the repl sources: abort"
    return
fi
if [ ! -d $FAUDES_SRC/plugins/luabindings ]; then
    echo "no luabindings found: abort"
    return
fi

############################################################################
# clean all  libFAUDES 
make -C $FAUDES_SRC dist-clean

############################################################################
# configure libFAUDES 

# plug-ins
FAUDES_PLUGINS=""
FAUDES_PLUGINS+=" synthesis"
FAUDES_PLUGINS+=" observer"
FAUDES_PLUGINS+=" diagnosis"
FAUDES_PLUGINS+=" iosystem"
FAUDES_PLUGINS+=" coordinationcontrol"
FAUDES_PLUGINS+=" priorities"
FAUDES_PLUGINS+=" omegaaut"
FAUDES_PLUGINS+=" luabindings"

# developer
FAUDES_DEBUG=""
FAUDES_DEBUG+=" core_checked"
FAUDES_DEBUG+=" core_exceptions"

# options
FAUDES_OPTIONS="none"

# platform
FAUDES_PLATFORM="gcc_generic"

# export to make
export FAUDES_PLUGINS
export FAUDES_DEBUG
export FAUDES_OPTIONS
export FAUDES_PLATFORM

make -C $FAUDES_SRC -j configure
make -C $FAUDES_SRC clean


############################################################################
# build libFAUDES essentials 

make -C ../../.. FAUDES_PLATFORM=emcc_js FAUDES_LINKING=static  -j ./libfaudes.a
make -C ../../.. FAUDES_PLATFORM=emcc_js FAUDES_LINKING=static  -j plugins/luabindings/repl/luafaudes.js
