export OSTYPE
export FAUDES_PLUGINS="synthesis observer diagnosis iosystem coordinationcontrol luabindings"
export FAUDES_DEBUG="core_checked core_exceptions"
export FAUDES_OPTIONS=
export FAUDES_PLATFORM="gcc_generic"

make dist-clean
make configure FAUDES_DEBUG=\\"$FAUDES_DEBUG\\" SWIG:=/usr/bin/swig
make clean
cd ..
