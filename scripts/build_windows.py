#!/usr/bin/env python3
"""
VneIo Windows Build Script
Copyright (c) 2025–2026 Ajeet Singh Yadav. All rights reserved.
Licensed under the Apache License, Version 2.0 (the "License")
"""
import os
import sys
import subprocess
import argparse
import shutil
from pathlib import Path


def find_visual_studio():
    for p in [
        r"C:\Program Files\Microsoft Visual Studio\2022\Community",
        r"C:\Program Files\Microsoft Visual Studio\2022\Professional",
        r"C:\Program Files\Microsoft Visual Studio\2022\Enterprise",
        r"C:\Program Files\Microsoft Visual Studio\2019\Community",
    ]:
        if os.path.exists(p):
            return p
    return None


def check_visual_studio_env():
    try:
        subprocess.run(["cl"], capture_output=True, check=False)
        return True
    except FileNotFoundError:
        return False


def main():
    parser = argparse.ArgumentParser(description="Build VneIo for Windows")
    parser.add_argument("-t", "--build-type", choices=["Debug", "Release", "RelWithDebInfo", "MinSizeRel"], default="Debug")
    parser.add_argument("-a", "--action", choices=["configure", "build", "configure_and_build", "test"], default="configure_and_build")
    parser.add_argument("-j", "--jobs", type=int, default=10)
    parser.add_argument("--clean", action="store_true")
    args = parser.parse_args()

    if not check_visual_studio_env():
        vs = find_visual_studio()
        if vs:
            print("Run from Developer Command Prompt or run:")
            print(f"  {os.path.join(vs, 'Common7', 'Tools', 'VsDevCmd.bat')}")
        sys.exit(1)

    try:
        subprocess.run(["cmake", "--version"], capture_output=True, check=True)
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("CMake not found"); sys.exit(1)

    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    build_dir = project_root / "build" / args.build_type / "build-windows-cl"
    if args.clean and build_dir.exists():
        shutil.rmtree(build_dir)
    build_dir.mkdir(parents=True, exist_ok=True)

    cmake_cmd = [
        "cmake", "-G", "Visual Studio 17 2022", "-A", "x64",
        f"-DCMAKE_BUILD_TYPE={args.build_type}",
        "-DBUILD_TESTS=ON",
        str(project_root),
    ]
    if args.action in ["configure", "build", "configure_and_build", "test"]:
        subprocess.run(cmake_cmd, cwd=build_dir, check=True)
    if args.action in ["build", "configure_and_build", "test"]:
        subprocess.run(
            ["cmake", "--build", ".", "--config", args.build_type, "--parallel", str(args.jobs)],
            cwd=build_dir, check=True
        )
    if args.action == "test":
        subprocess.run(["ctest", "-C", args.build_type, "--output-on-failure"], cwd=build_dir, check=True)

    print("=== Build completed ===")
    print(f"Build directory: {build_dir}")


if __name__ == "__main__":
    main()
