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

def resolve_real_path(dep):
    # @rpath or relative: resolve via otool -L search path is opaque,
    # but Homebrew deps are always absolute by the time otool prints them.
    if dep.startswith("@"):
        return None
    p = Path(dep)
    if p.exists():
        return p.resolve()
    return None

def bundle(bundle_dir):
    bundle_dir = Path(bundle_dir).resolve()
    dylibs = set(bundle_dir.glob("*.dylib"))
    processed = set()

    while dylibs:
        lib = dylibs.pop()
        if lib in processed:
            print("already processed")
            continue
        processed.add(lib)

        for dep in otool_deps(str(lib)):
            if is_system_lib(dep):
                continue

            dep_path = resolve_real_path(dep)
            if dep_path is None:
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

        # Ad-hoc codesign (required for dlopen)
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

