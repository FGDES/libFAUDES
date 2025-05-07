
## libFAUDES - Friedrich-Alexander-University Discrete Event Systems Library

Copyright (C) 2006 Bernd Opitz

Copyright (C) 2006-2009 Thomas Moor, Klaus Schmidt, Sebastian Perk

Copyright (C) 2010-2025 Thomas Moor, Klaus Schmidt


## Directions for the User

The MS Windows 64bit binary package of libFAUDES is distributed for
convenience. Documentation and further resources are provided at

https://fgdes.tf.fau.de/faudes

This binary distribution was built with the toolchain from by MS Visual Studio
Version 2022 in a 64bit MS Windows 11 environment. It includes

- the tools *luafaudes.exe*, *simfaudes.exe* and friends in *.\libFAUDES\*
- MS VC DLLs aka MS VC redistributables; see below (!!!)
- the HTML documentation in *.\libFAUDES\Doc\index.html*
- some Lua examples in *.\libFAUDES\Examles\*
- the C++ headers in *.libFAUDES\include* and a static debug version
  *faudesd.lib* of libFAUDES, compiled with /MDd. 


## Note on MS VC redistributables.

Building libFAUDES with the MS compilers has the pro that the generated binaries
are completely within the MS ecosystem. Unfortunally, a plain MS Windows installation
does not come with all DLLs such binaries link to. The libFAUDES binary package therefore
optionally copies those DLLs locally into the libFAUDES folder. This is meant for
convenience. Alternatively, you can get those DLLs directly from MS. They are free
of charge and you may google them by the keyword "vcredist.x64.exe".


## Directions for the Developer

The sources of the package are hosted on GitHub, see

https://github.com/FGDES/libFAUDES

The general build process consists of multiple stages and is documented at

https://fgdes.tf.fau.de/faudes/faudes_build.html

Specifically, we used an MSYS2 environment for the configuration stage and an
MS Windows command prompt for compilation and packaging the installer. Corresponding
batch files are included with the libFAUDES sources.

- *tools/msvc/build.bat* to build shared and static versions of libFAUDES
- *tools/msvc/package.bat* to build the installer

However, the above files heavily depend of the development environment. If you want
to build your own libFAUDES distribution with MS VC compilers, you will need to
inspect/understand/adjust the batchfiles to your needs. In the case you do
not need to distribute binary packages, you are far better of by sticking to MSYS2
for all development stages and dont bother MS VC at all.


 
