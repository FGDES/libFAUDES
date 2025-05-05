
 libFAUDES - Friedrich Alexander University Discrete Event Systems Library

 Copyright (C) 2006 Bernd Opitz
 Copyright (C) 2006-2009 Thomas Moor, Klaus Schmidt, Sebastian Perk
 Copyright (C) 2010-2025 Thomas Moor, Klaus Schmidt


===============================================================================
Directions for the User
===============================================================================

This is an MS-Windows 64bit binary package of libFAUDES. It is provided for 
convenience. For further information on libFAUDES and friends, please consult

https://fgdes.tf.fau.de/faudes

This binary distribution was built with the compiler provided by MS Visual Studio
Version 2022 in a 64bit MS Windows 11 environment. It includes

- the tools lufaudes, simfaudes and friends in .\libFAUDES\
- MS VC DLLs aka MS VC redistributables; see below (!)
- the HTML documentation in .\libFAUDES\Doc\index.html
- some Lua examples in .\libFAUDES\Examles
- for application development the C++ headers in .libFAUDES\include and a static
  debug version "faudesd.lib" of libFAUDES, compiled with /MDd. 

Note on MS VC redistributables. Building libFAUDES with the MS compilers has the
pro that we are completely within the MS ecosystem. Unfortunally, MS Windows does
not come with all DLLs required by executables compiled with MS compilers. The
libFAUDES binary package does include those DLLs that we believe might be missing
if you do not have the MS compilers on your system. This is for convenience. The
proper way is to get those DLLs directly from MS. They are free of charge and you
may google them by the keyword "vcredist.x64.exe".


===============================================================================
Directions for the Developer
===============================================================================

The sources of the package are hosted on GitHub, see

https://github.com/FGDES/libFAUDES

The general build process consists of multiple stages and is documented at

https://fgdes.tf.fau.de/faudes/faudes_build.html

Specifically, we used an MSYS2 environment for the configuration stage and an
MS Windows command promt for compilation and packaging the installer. Corresponding
batch files are included with the libFAUDES sources.

- msvcbuild.bat to build shared and static versions of libFAUDES
- msvcdist.bat to build the installer

However, the above files heavily depend of the development environment. If you want
to build your own, you will need to inspect/understand/adjust. In the case you do
not need to distribute binary packages, you are better of by sticking to MSYS
for all development stages.


 
