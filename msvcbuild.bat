REM ================================== build libFAUDES with MSVC toolchain

REM ==== borrow GNU make, e.g. from MSYS2 installation
set GNUMAKE=C:\msys64\ucrt64\bin\mingw32-make.exe

REM ==== set paths for MSVC command line tools
call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

REM ==== have Inno Setup
set INNOSCC "%ProgramFiles(x86)%\Inno Setup 6\iscc"

REM ==== set FAUDES version numbers
call VERSION.bat

REM ==== build libFAUDES shared/release
%GNUMAKE% FAUDES_PLATFORM=cl_win clean
%GNUMAKE% FAUDES_PLATFORM=cl_win -j

REM ==== build libFAUDES static/debugging
%GNUMAKE% FAUDES_PLATFORM=cl_win clean
%GNUMAKE% FAUDES_PLATFORM=cl_win FAUDES_LINKING="static debug" -j
%GNUMAKE% FAUDES_PLATFORM=cl_win FAUDES_LINKING="static debug" -j tutorial
%GNUMAKE% FAUDES_PLATFORM=cl_win FAUDES_LINKING="static debug" test



