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
call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64"

echo "==== build libFAUDES static/debug"
%GNUMAKE% FAUDES_PLATFORM=cl_win clean
%GNUMAKE% FAUDES_PLATFORM=cl_win libfaudes FAUDES_LINKING="static debug" -j 

echo "==== build libFAUDES shared/release"
%GNUMAKE% FAUDES_PLATFORM=cl_win clean
%GNUMAKE% FAUDES_PLATFORM=cl_win FAUDES_LINKING="shared" -j 


