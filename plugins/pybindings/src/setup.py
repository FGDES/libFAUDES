#  imports
from setuptools import setup, Extension
from pathlib import Path
import sysconfig
from datetime import datetime

# figure source files
srcfiles = [str(cppfile) for cppfile in Path("src/").glob("**/*.cpp")]

# figure build
platform = sysconfig.get_platform()
pyver = sysconfig.get_python_version()
buildstr = platform + "-for-python-" + pyver
nowstr = datetime.now().strftime('%Y-%m-%dT%H-%M-%S')

ext = Extension(
  name="_faudes",
  sources=srcfiles,
  include_dirs=["src"],
  define_macros=[
    ("FAUDES_BUILD_DSO","1"),
    ("FAUDES_MUTE_RTIAUTOLOAD","1"),
    ("FAUDES_BUILDENV",buildstr),
    ("FAUDES_BUILTIME",nowstr)
  ] 
)
                   
setup(
  name="faudes",
  ext_modules=[ext],
  py_modules=["faudes"]
)
