#  imports
from setuptools import setup, Extension
from pathlib import Path
import sysconfig
from datetime import datetime

# figure source files
srcfiles = [str(cppfile) for cppfile in Path("src/").glob("**/*.cpp")]

# rtiloader/rtiwrapper are formally not sources
srcfiles = [file for file in srcfiles if "rtiwrapper" not in file]
srcfiles = [file for file in srcfiles if "rtiloader" not in file]


# figure platform
platform = sysconfig.get_platform()
pyver = sysconfig.get_python_version()
buildstr = '"' + platform + '-for-python-' + pyver + '"'

# figure time stamp
nowstr = datetime.now().strftime('"%Y-%m-%dT%H-%M-%S"')

# figure relevant system  libraries
syslibs=None
sysdirs=None
if "win" in platform:
  syslibs=["winmm", "wsock32"]
if "mingw" in platform:
  syslibs=["winmm", "wsock32"]
  sysdirs=["/mingw64/lib"]

ext = Extension(
  name="_faudes",
  sources=srcfiles,
  include_dirs=["src"],
  libraries=syslibs,
  libraries_dirs=sysdirs,
  define_macros=[
    ("FAUDES_BUILD_DSO","1"),
    ("FAUDES_BUILDENV",buildstr),
    ("FAUDES_BUILTIME",nowstr),
    ("FAUDES_MUTE_LUA","1")
  ] 
)
                   
setup(
  name="faudes",
  ext_modules=[ext],
  py_modules=["faudes"],
)
