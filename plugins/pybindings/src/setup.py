#  imports
from setuptools import setup, Extension
from pathlib import Path
import sysconfig
from datetime import datetime

# figure source files
srcfiles = [str(cppfile) for cppfile in Path("src/").glob("**/*.cpp")]

# figure platform
platform = sysconfig.get_platform()
pyver = sysconfig.get_python_version()
buildstr = '"' + platform + '-for-python-' + pyver + '"'

# figure time stamp
nowstr = datetime.now().strftime('"%Y-%m-%dT%H-%M-%S"')

# figure relevant system  libraries
syslibs=[]
if "win" in platform:
  syslibs=["winmm", "wsock32"]

ext = Extension(
  name="_faudes",
  sources=srcfiles,
  include_dirs=["src"],
  libraries=syslibs,
  define_macros=[
    ("FAUDES_BUILD_DSO","1"),
    ("FAUDES_MUTE_RTIAUTOLOAD","1"),
    ("FAUDES_BUILDENV",buildstr),
    ("FAUDES_BUILTIME",nowstr),
    ("FAUDES_MUTE_LUA","1")
  ] 
)
                   
setup(
  name="faudes",
  ext_modules=[ext],
  py_modules=["faudes"]
)
