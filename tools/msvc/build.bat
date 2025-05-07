REM ================================== build libFAUDES with MSVC toolchain

REM === (need to configure sources before e.g. in an MSYS environment)

REM === sanity check
if not exist VERSION.bat {
  echo need to cd to libFAUDES base path
  exit
}

REM ==== borrow GNU make, e.g. from MSYS2 installation
set GNUMAKE=C:\msys64\ucrt64\bin\mingw32-make.exe 

REM ==== set paths for MSVC command line tools
call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

REM ==== build libFAUDES static/debugging/testing (keep "faudesd.lib")
%GNUMAKE% FAUDES_PLATFORM=cl_win clean
%GNUMAKE% FAUDES_PLATFORM=cl_win FAUDES_LINKING="static debug" -j
%GNUMAKE% FAUDES_PLATFORM=cl_win FAUDES_LINKING="static debug" -j tutorial
%GNUMAKE% FAUDES_PLATFORM=cl_win FAUDES_LINKING="static debug" test

REM ==== build libFAUDES shared/release (keep faudes.dll, and ./bin/*.exe))
%GNUMAKE% FAUDES_PLATFORM=cl_win clean
%GNUMAKE% FAUDES_PLATFORM=cl_win FAUDES_LINKING="shared" -j 


