REM ================================== build libFAUDES with MSVC toolchain

REM ==== borrow GNU make from MSYS/MinGW
set GNUMAKE=C:\msys64\ucrt64\bin\mingw32-make.exe

REM ==== set paths for MSVC command line tools
"%ProgramFiles%\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

REM ==== build libFAUDES shared/release
%GNUMAKE% FAUDES_PLATFORM=cl_win clean
%GNUMAKE% FAUDES_PLATFORM=cl_win -j20

REM ==== build libFAUDES static/debugging
%GNUMAKE% FAUDES_PLATFORM=cl_win clean
%GNUMAKE% FAUDES_PLATFORM=cl_win FAUDES_LINKING="static debug" -j20
%GNUMAKE% FAUDES_PLATFORM=cl_win FAUDES_LINKING="static debug" -j20 tutorial



