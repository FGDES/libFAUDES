echo "================================== build libFAUDES with MSVC toolchain"

echo "=== (need to configure sources before e.g. in an MSYS environment)"

echo "=== sanity check for libFAUDES working directory"
if not exist "VERSION" (
  echo "ERROR: no VERSION file in current directory"
  exit /b
)

echo "==== borrow GNU make, e.g. from MSYS2 installation"
set GNUMAKE=C:\msys64\ucrt64\bin\mingw32-make.exe 

echo "==== set paths for MSVC command line tools"
call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"

echo "==== build libFAUDES static/debugging/testing (keeping faudesd.lib)"
%GNUMAKE% FAUDES_PLATFORM=cl_win clean
%GNUMAKE% FAUDES_PLATFORM=cl_win FAUDES_LINKING="static debug" -j
%GNUMAKE% FAUDES_PLATFORM=cl_win FAUDES_LINKING="static debug" -j tutorial
%GNUMAKE% FAUDES_PLATFORM=cl_win FAUDES_LINKING="static debug" test

echo "==== build libFAUDES shared/release (keeping faudes.dll and ./bin/*.exe)"
%GNUMAKE% FAUDES_PLATFORM=cl_win clean
%GNUMAKE% FAUDES_PLATFORM=cl_win FAUDES_LINKING="shared" -j 


