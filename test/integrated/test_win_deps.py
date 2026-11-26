#!/usr/bin/env python3
# -*- coding: utf8 -*-"

"""
Ensure no new DLL dependency is added involuntarily, and verify that all
needed non-system DLLs are included in the release archive.
"""

import difflib
import os
import sys
import pefile

def get_direct_imports(pe_path: str) -> list[str]:
    pe = pefile.PE(pe_path, fast_load=True)
    pe.parse_data_directories(
        directories=[pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_IMPORT"]]
    )
    if not hasattr(pe, "DIRECTORY_ENTRY_IMPORT"):
        return []
    return [entry.dll.decode("utf-8") for entry in pe.DIRECTORY_ENTRY_IMPORT]


# System DLL expected to be available by default on Windows.
system_dlls = {
  "advapi32.dll",
  "gdi32.dll",
  "imm32.dll",
  "kernel32.dll",
  "msvcrt.dll",
  "ole32.dll",
  "oleaut32.dll",
  "setupapi.dll",
  "shell32.dll",
  "user32.dll",
  "version.dll",
  "winmm.dll",
  "ws2_32.dll",
  # Not sure if they count as system. They are used by the msys2 build (ucrt64)
  # and apparently don't need to be distributed.
  "dwrite.dll",
  "rpcrt4.dll",
  "usp10.dll",
  "api-ms-win-crt-convert-l1-1-0.dll",
  "api-ms-win-crt-environment-l1-1-0.dll",
  "api-ms-win-crt-filesystem-l1-1-0.dll",
  "api-ms-win-crt-heap-l1-1-0.dll",
  "api-ms-win-crt-locale-l1-1-0.dll",
  "api-ms-win-crt-math-l1-1-0.dll",
  "api-ms-win-crt-private-l1-1-0.dll",
  "api-ms-win-crt-process-l1-1-0.dll",
  "api-ms-win-crt-runtime-l1-1-0.dll",
  "api-ms-win-crt-stdio-l1-1-0.dll",
  "api-ms-win-crt-string-l1-1-0.dll",
  "api-ms-win-crt-time-l1-1-0.dll",
  "api-ms-win-crt-utility-l1-1-0.dll",
}

def resolve_transitive_deps(archive_dir: str, exe_name: str) -> tuple[set[str], list[str]]:
    seen: set[str] = set()
    bundled_deps: set[str] = set()
    missing_deps: list[str] = []
    queue: list[str] = [os.path.join(archive_dir, exe_name)]

    while queue:
        current_path = queue.pop(0)
        for dll in get_direct_imports(current_path):
            dll_lower = dll.lower()
            if dll_lower in seen:
                continue
            seen.add(dll_lower)

            if dll_lower in system_dlls:
                continue

            matched_file = next(
                (f for f in os.listdir(archive_dir) if f.lower() == dll_lower),
                None,
            )
            if matched_file is None:
                missing_deps.append(dll)
            else:
                bundled_deps.add(matched_file)
                queue.append(os.path.join(archive_dir, matched_file))

    return bundled_deps, missing_deps


def get_expected_deps(build_type: str) -> list[str]:
    if build_type == "ucrt64":
        return sorted(
            [
                "SDL2.dll",
                "libbrotlicommon.dll",
                "libbrotlidec.dll",
                "libbz2-1.dll",
                "libfreetype-6.dll",
                "libgcc_s_seh-1.dll",
                "libglib-2.0-0.dll",
                "libgraphite2.dll",
                "libharfbuzz-0.dll",
                "libiconv-2.dll",
                "libintl-8.dll",
                "libpcre2-8-0.dll",
                "libpng16-16.dll",
                "libstdc++-6.dll",
                "libwinpthread-1.dll",
                "libz-1.dll",
                "zlib1.dll",
            ]
        )

    if build_type == "win32" or build_type == "win64":
        gcc_dll = "libgcc_s_dw2-1.dll" if build_type == "win32" else "libgcc_s_seh-1.dll"
        return sorted(
            [
                "SDL2.dll",
                gcc_dll,
                "libstdc++-6.dll",
                "libwinpthread-1.dll",
            ]
        )

    raise ValueError(f"Unsupported build_type '{build_type}'")


def main() -> int:
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <win32|win64|ucrt64> <exe path>", file=sys.stderr)
        return 1

    build_type = sys.argv[1]
    exe_path = sys.argv[2]

    if build_type == "win32":
        arch = "win32"
    elif build_type == "win64":
        arch = "win64"
    elif build_type == "ucrt64":
        arch = "win64"
    else:
        print(f"ERROR: Unsupported build type: '{build_type}'")
        return 1

    if not os.path.isfile(exe_path):
        print(f"!! Error: {exe_path} not found", file=sys.stderr)
        return 1

    archive_dir = os.path.dirname(exe_path)
    exe_name = os.path.basename(exe_path)
    bundled_deps, missing_deps = resolve_transitive_deps(archive_dir, exe_name)

    if missing_deps:
        print(f"!! Missing required DLLs in {archive_dir}:")
        for dll in sorted(missing_deps):
            print(f"  - {dll}")
        return 1

    actual = sorted(bundled_deps)
    expected = get_expected_deps(build_type)

    print("actual dependencies:")
    for dll in actual:
        print(dll)

    if actual != expected:
        print(f"\n!! dependencies test for {arch} failed:")
        diff = difflib.unified_diff(
            expected, actual, fromfile="expected", tofile="actual", lineterm=""
        )
        for line in diff:
            print(line)
        return 1

    print("\nThe test was successful!")
    return 0


if __name__ == "__main__":
    sys.exit(main())
