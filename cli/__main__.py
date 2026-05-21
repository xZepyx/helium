import runpy
import sys
from pathlib import Path

sys.argv[0] = "helium"
runpy.run_path(str(Path(__file__).parent / "helium"), run_name="__main__")
