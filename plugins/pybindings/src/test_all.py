# imports
import pytest
from pathlib import Path
import faudes

# figure test scripts
scripts = [str(script) for script in Path("./").glob("*.py")]

# remove non-tests
scripts = [script for script in scripts if script != "faudes.py" ]
scripts = [script for script in scripts if script != "test_all.py" ]


@pytest.mark.parametrize("script", scripts)
def test_one(script):
  faudes.FAUDES_TEST_NAME=script
  exec(open(script).read())


