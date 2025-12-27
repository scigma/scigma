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
    return subprocess.check_output(cmd, text=True).strip()

def otool_deps(dylib):
    out = run(["otool", "-L", dylib])
    deps = []
    for line in out.splitlines()[1:]:
        dep = line.strip().split(" ")[0]
        deps.append(dep)
    return deps

def is_system_lib(path):
    return any(path.startswith(p) for p in SYSTEM_PREFIXES)

def rpaths(dylib):
    out = run(["otool", "-l", dylib])
    paths = []
    lines = out.splitlines()
    for i, line in enumerate(lines):
        if "cmd LC_RPATH" in line:
            # path is two lines below
            path_line = lines[i + 2].strip()
            if path_line.startswith("path "):
                paths.append(path_line.split("path ")[1].split(" (")[0])
    return paths

def resolve_real_path(dep, owner_dylib):
    # Absolute path
    p = Path(dep)
    if p.is_absolute() and p.exists():
        return p.resolve()

    # @rpath resolution
    if dep.startswith("@rpath/"):
        name = dep.split("/", 1)[1]
        for rp in rpaths(owner_dylib):
            candidate = Path(rp) / name
            if candidate.exists():
                return candidate.resolve()

    return None

def bundle(bundle_dir):
    bundle_dir = Path(bundle_dir).resolve()
    dylibs = set(bundle_dir.glob("*.dylib"))
    processed = set()

    while dylibs:
        lib = dylibs.pop()
        print("processing " + str(lib))
        if lib in processed:
            print("already processed")
            continue
        processed.add(lib)

        for dep in otool_deps(str(lib)):
            if is_system_lib(dep):
                print(str(dep)+ " is system lib")
                continue

            dep_path = resolve_real_path(dep, str(lib))
            if dep_path is None:
                print(str(dep)+ " cannot resolve")
                continue

            target = bundle_dir / dep_path.name
            if not target.exists():
                print(f"Copying {dep_path} → {target}")
                shutil.copy(dep_path, target)
                target.chmod(0o755)
                dylibs.add(target)

    for lib in bundle_dir.glob("*.dylib"):
        # Rewrite dependencies to local loader path
        for dep in otool_deps(str(lib)):
            if is_system_lib(dep):
                continue
            dep_path = resolve_real_path(dep)
            if dep_path is None:
                continue
            subprocess.run([
                "install_name_tool",
                "-change",
                dep,
                f"@loader_path/{dep_path.name}",
                str(lib)
            ], check=True)

        # Fix install IDs
        subprocess.run([
            "install_name_tool",
            "-id",
            f"@loader_path/{lib.name}",
            str(lib)
        ], check=True)

'''     # Ad-hoc codesign (required for dlopen)
        subprocess.run([
            "codesign",
            "--force",
            "--deep",
            "--sign",
            "-",
            str(lib)
        ], check=True)
'''

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: bundle_macos_dylibs.py <bundle_dir>")
        sys.exit(1)

    bundle(sys.argv[1])

