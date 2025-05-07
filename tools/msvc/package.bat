echo "================================== build libFAUDES installer with Inno Setup"

echo "=== (need to build libFAUDES with MS VC before, e.g. via msvc/build.bat)"

echo "=== sanity check for libFAUDES working directory"
if not exist "VERSION" (
  echo "ERROR: no VERSION file in current directory"
  exit /b
)

echo "==== have Inno Setup"
set INNOSCC="%ProgramFiles(x86)%\Inno Setup 6\iscc"

echo "==== set FAUDES version numbers"
call tools\msvc\VERSION.bat

echo "==== set paths for MSVC redistributables"
set FAUDES_MSVCREDIST="%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC\14.38.33130\x64\Microsoft.VC143.CRT"

echo "==== run Inno to build setup.exe"
%INNOSCC% tools\msvc\package.iss /DVMAJOR=%FAUDES_VERSION_MAJOR% /DVMINOR=%FAUDES_VERSION_MINOR% /DMSVCREDIST=%FAUDES_MSVCREDIST%



