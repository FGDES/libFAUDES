# imports
import pytest
import os
import sys
from pathlib import Path
import faudes

# chd to location of this scipt
os.chdir(sys.path[0])

# figure test scripts
scripts = [str(script) for script in Path("./").glob("*.py")]

# remove non-tests
scripts = [script for script in scripts if script != "faudes.py" ]
scripts = [script for script in scripts if script != "test_all.py" ]

# tell what we have found
print('test cases found:')
print(scripts)


@pytest.mark.parametrize("script", scripts)
def test_one(script):
  faudes.FAUDES_TEST_NAME=script
  exec(open(script).read())


