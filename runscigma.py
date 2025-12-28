import os
import sys
import subprocess
import traceback

def is_macos():
    return sys.platform == "darwin"

def dequarantine_scigma():
    try:
        scigma_path = os.path.abspath(os.path.dirname(__file__))

        subprocess.run(
            ["/usr/bin/xattr", "-dr", "com.apple.quarantine", scigma_path],
            check=False,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )

    except Exception:
        # Never let this kill startup
        pass

# ---- macOS-specific fix ----
if is_macos():
    dequarantine_scigma()

# ---- Try importing scigma ----
try:
    import scigma
except Exception:
    traceback.print_exc()
    print()
    print("Could not import scigma.")

# ---- Python 2 / 3 compatibility ----
try:
    input = raw_input
except NameError:
    pass

input("Press enter to exit.")
print("Exiting.")
