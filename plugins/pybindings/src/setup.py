from setuptools import setup, Extension
from pathlib import Path

srcfiles = [str(cppfile) for cppfile in Path("src/").glob("**/*.cpp")]

ext = Extension(
  name="_faudes",
  sources=srcfiles,
  include_dirs=["src"],
  define_macros=[
    ("FAUDES_BUILD_DSO","1"),
    ("FAUDES_MUTE_RTIAUTOLOAD","1")
  ] 
)
                   
setup(
  name="faudes",
  ext_modules=[ext],
  py_modules=["faudes"]
)
