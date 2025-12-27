#!/usr/bin/env python3

import subprocess
import sys
import shutil
from pathlib import Path

SYSTEM_PREFIXES = (
    "/usr/lib/",
    "/System/",
)

def run(cmd):
    """Run a command and return stdout as string."""
    return subprocess.check_output(cmd, text=True).strip()

def otool_deps(dylib):
    """Return a list of dependencies for a dylib using otool -L."""
    out = run(["otool", "-L", dylib])
    deps = []
    for line in out.splitlines()[1:]:
        dep = line.strip().split(" ")[0]  # first token only
        deps.append(dep)
    return deps

def is_system_lib(path):
    return any(path.startswith(p) for p in SYSTEM_PREFIXES)

def brew_gcc_lib_dir():
    """Return Homebrew GCC library directory, e.g., /opt/homebrew/opt/gcc/lib/gcc/current"""
    prefix = run(["brew", "--prefix", "gcc"])
    return Path(prefix) / "lib" / "gcc" / "current"

def resolve_dep_in_brew(dep, gcc_lib_dir):
    """Resolve a dependency by looking only in the Homebrew GCC lib dir by filename."""
    dep_name = Path(dep).name
    candidate = gcc_lib_dir / dep_name
    if candidate.exists():
        return candidate.resolve()
    return None

def bundle(bundle_dir):
    bundle_dir = Path(bundle_dir).resolve()
    gcc_lib_dir = brew_gcc_lib_dir()

    # Initial dylibs in the bundle
    dylibs_to_process = set(bundle_dir.glob("*.dylib"))
    processed = set()

    while dylibs_to_process:
        lib = dylibs_to_process.pop()
        if lib in processed:
            continue
        processed.add(lib)
        print(f"Processing {lib.name}")

        for dep in otool_deps(str(lib)):
            if is_system_lib(dep):
                continue

            dep_path = resolve_dep_in_brew(dep, gcc_lib_dir)
            if dep_path is None:
                print(f"WARNING: cannot resolve {dep}")
                continue

            target = bundle_dir / dep_path.name
            if not target.exists():
                print(f"Copying {dep_path} → {target}")
                shutil.copy(dep_path, target)
                target.chmod(0o755)
                dylibs_to_process.add(target)

    # Rewrite @rpath/@loader_path and codesign
    for lib in bundle_dir.glob("*.dylib"):
        for dep in otool_deps(str(lib)):
            if is_system_lib(dep):
                continue

            dep_path = resolve_dep_in_brew(dep, gcc_lib_dir)
            if dep_path is None:
                continue

            subprocess.run([
                "install_name_tool",
                "-change",
                dep,
                f"@loader_path/{dep_path.name}",
                str(lib)
            ], check=True)

        # Set install ID
        subprocess.run([
            "install_name_tool",
            "-id",
            f"@loader_path/{lib.name}",
            str(lib)
        ], check=True)

        # Codesign so macOS allows dlopen
        subprocess.run([
            "codesign",
            "--force",
            "--deep",
            "--sign",
            "-",
            str(lib)
        ], check=True)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: bundle_macos_dylibs.py <bundle_dir>")
        sys.exit(1)

    bundle(sys.argv[1])
