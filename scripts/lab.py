#!/usr/bin/env python3

import argparse
import csv
from datetime import datetime, timezone
import hashlib
import json
import os
import platform
import stat
import subprocess
import time
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SDK_HASH = "75d6d2171f845f356a2c9f445b963217fca0f8c4d741ffcea08f04ec9146464a"
LIBRARIES = (
    "Eco.System1", "Eco.InterfaceBus1", "Eco.MemoryManager1",
    "Eco.FileSystemManagement1",
)
FORBIDDEN = {
    "malloc", "calloc", "realloc", "free", "memcpy", "memmove", "memset",
    "memcmp", "qsort", "bsearch", "printf", "fprintf", "sprintf", "snprintf",
    "puts", "putchar", "fopen", "fclose", "fread", "fwrite", "strlen",
    "strcpy", "strncpy", "strcmp", "pthread_create", "pthread_join",
}


def tree_hash(directory):
    digest = hashlib.sha256()
    for path in sorted(directory.rglob("*")):
        if path.is_symlink():
            raise RuntimeError("The SDK tree must not contain symbolic links.")
        if path.is_file():
            digest.update(path.relative_to(directory).as_posix().encode())
            digest.update(hashlib.sha256(path.read_bytes()).digest())
    return digest.hexdigest()


def run(arguments, log, timeout=120):
    completed = subprocess.run(
        [str(value) for value in arguments], cwd=ROOT,
        capture_output=True, text=True, timeout=timeout,
    )
    log.append("$ " + " ".join(str(value) for value in arguments))
    log.append(completed.stdout)
    log.append(completed.stderr)
    if completed.returncode:
        raise RuntimeError(completed.stdout + completed.stderr)
    return completed.stdout


def sdk_location(explicit=None):
    if explicit:
        return explicit.expanduser().resolve()
    if os.environ.get("ECO_FRAMEWORK"):
        return Path(os.environ["ECO_FRAMEWORK"]).expanduser().resolve()
    config = ROOT / ".eco/config.json"
    if config.is_file():
        return (ROOT / json.loads(config.read_text())["framework"]).resolve()
    return ROOT / ".eco/Eco.Framework"


def check_sdk(framework):
    if not (framework / "Eco.Core1/SharedFiles/IEcoBase1.h").is_file():
        raise RuntimeError("Eco SDK is missing. Run: python3 scripts/lab.py setup")
    if tree_hash(framework) != SDK_HASH:
        raise RuntimeError("Expected Eco.Framework_20260918.zip; SDK checksum differs.")


def setup(args):
    destination = ROOT / ".eco"
    destination.mkdir(exist_ok=True)
    if args.framework:
        framework = args.framework.expanduser().resolve()
    elif args.archive is None and (destination / "config.json").is_file():
        framework = sdk_location()
    else:
        archive = args.archive or (
            Path.home() / "Downloads/Design operating systems/Eco.Framework_20260918.zip"
        )
        if not archive.is_file():
            raise RuntimeError("Supply the SDK archive with setup --archive PATH.")
        with zipfile.ZipFile(archive) as source:
            for member in source.infolist():
                path = Path(member.filename)
                mode = member.external_attr >> 16
                if path.is_absolute() or ".." in path.parts or stat.S_ISLNK(mode):
                    raise RuntimeError("Unsafe archive member: " + member.filename)
            extracted = destination / "extracted"
            extracted.mkdir(exist_ok=True)
            source.extractall(extracted)
        candidates = list(extracted.glob("**/Eco.Core1/SharedFiles/IEcoBase1.h"))
        if len(candidates) != 1:
            raise RuntimeError("Cannot identify one Eco.Framework directory in the archive.")
        framework = candidates[0].parents[2]
    check_sdk(framework)
    try:
        saved = str(framework.relative_to(ROOT))
    except ValueError:
        saved = str(framework)
    (destination / "config.json").write_text(
        json.dumps({"framework": saved, "sha256": SDK_HASH}, indent=2) + "\n"
    )
    print("Eco SDK configured:", framework)


def build(lab, command, framework, sanitize):
    output = ROOT / "build" / ("lab" + str(lab)) / command
    output.mkdir(parents=True, exist_ok=True)
    (output / "result.json").unlink(missing_ok=True)
    (output / "benchmark.csv").unlink(missing_ok=True)
    log = []
    started = time.perf_counter()
    try:
        clang = run(["xcrun", "--find", "clang"], log).strip()
        archiver = run(["xcrun", "--find", "ar"], log).strip()
        nm = run(["xcrun", "--find", "nm"], log).strip()
        compiler_version = run([clang, "--version"], log).splitlines()[0]
        sdkroot = run(["xcrun", "--show-sdk-path"], log).strip()
        flags = [
            "-std=c89", "-pedantic-errors", "-Wall", "-Wextra", "-Werror",
            "-Wno-unused-const-variable", "-O2", "-g", "-DECO_LIB",
            "-DUGUID_UTILITY", "-DECO_APPLE", "-DECO_MAC", "-DECO_AARCH64",
            "-DARPO_LAB=" + str(lab), "-isysroot", sdkroot,
        ]
        for directory in ("include", "common/include", "client", "tests"):
            flags.extend(["-I", str(ROOT / directory)])
        for directory in sorted(framework.glob("*/SharedFiles")):
            flags.extend(["-isystem", str(directory)])
        if sanitize:
            flags.extend(["-fsanitize=address,undefined", "-fno-sanitize-recover=all"])
        sources = sorted((ROOT / "common/src").glob("*.c"))
        sources.extend(sorted((ROOT / ("lab" + str(lab)) / "SourceFiles").glob("*.c")))
        if lab == 2:
            sources.extend(sorted((ROOT / "lab2/Calculators").glob("*.c")))
        objects = []
        undefined = set()
        for index, source in enumerate(sources):
            target = output / (str(index) + "-" + source.stem + ".o")
            run([clang, *flags, "-ffreestanding", "-fno-builtin",
                 "-c", source, "-o", target], log)
            names = run([nm, "-u", target], log)
            undefined.update(line.split()[-1].lstrip("_")
                             for line in names.splitlines() if line.split())
            objects.append(target)
        blocked = undefined & FORBIDDEN
        if blocked:
            raise RuntimeError("Forbidden component imports: " + ", ".join(sorted(blocked)))
        archive = output / ("libEcoLab" + str(lab) + ".a")
        archive.unlink(missing_ok=True)
        run([archiver, "rcs", archive, *objects], log)
        if command == "test":
            clients = sorted((ROOT / "tests").glob("*.c"))
        elif command == "benchmark":
            clients = [ROOT / "client/Benchmark.c"]
        else:
            clients = [ROOT / ("lab" + str(lab)) / "Demo.c"]
        clients += [ROOT / "client/ParallelExecutor.c", ROOT / "client/ClientRuntime.c"]
        libraries = []
        for name in LIBRARIES:
            matches = list((framework / name / "BuildFiles/Mac/arm64/StaticRelease").glob("lib*.a"))
            if len(matches) != 1:
                raise RuntimeError("Missing arm64 static library: " + name)
            libraries += matches
        executable = output / ("lab" + str(lab) + "-" + command)
        run([clang, *flags, *clients, archive, *libraries, "-o", executable], log)
        result = {
            "recorded_at_utc": datetime.now(timezone.utc).isoformat(),
            "compiler": compiler_version,
            "platform": platform.platform(),
            "source_sha256": {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
                              for directory in ("include", "common", "lab1", "lab2", "client", "tests", "scripts")
                              for p in sorted((ROOT / directory).rglob("*"))
                              if p.suffix in (".c", ".h", ".py")},
            "lab": lab, "command": command, "standard": "c89", "optimization": "O2",
            "sanitizers": ["address", "undefined"] if sanitize else [],
            "sdk_sha256": SDK_HASH,
            "component_sha256": {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
                                 for p in sources},
            "undefined_component_symbols": sorted(undefined),
            "build_seconds": time.perf_counter() - started,
            "executable": str(executable.relative_to(ROOT)),
        }
        if command != "build":
            result["output"] = run([executable], log, timeout=240)
            print(result["output"], end="")
            if command == "benchmark":
                rows = [line.split(",")[1:] for line in result["output"].splitlines()
                        if line.startswith("BENCH,")]
                with (output / "benchmark.csv").open("w", newline="") as target:
                    writer = csv.writer(target)
                    writer.writerow(["lab", "n", "pattern", "method", "repeat", "seconds", "exact"])
                    writer.writerows(rows)
        else:
            print("Built", executable.relative_to(ROOT))
        (output / "result.json").write_text(json.dumps(result, indent=2) + "\n")
        return result
    finally:
        (output / "build.log").write_text("\n".join(log))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=["setup", "build", "demo", "test", "benchmark"])
    parser.add_argument("--lab", choices=["1", "2", "all"], default="all")
    parser.add_argument("--framework", type=Path)
    parser.add_argument("--archive", type=Path)
    parser.add_argument("--no-sanitize", action="store_true")
    args = parser.parse_args()
    if platform.system() != "Darwin" or platform.machine() != "arm64":
        parser.error("This SDK profile requires macOS on Apple Silicon.")
    try:
        if args.command == "setup":
            setup(args)
            return
        framework = sdk_location(args.framework)
        check_sdk(framework)
        for lab in [1, 2] if args.lab == "all" else [int(args.lab)]:
            build(lab, args.command, framework,
                  args.command == "test" and not args.no_sanitize)
    except (RuntimeError, subprocess.TimeoutExpired, OSError) as error:
        parser.exit(1, str(error) + "\nSee build/lab*/COMMAND/build.log for the command log.\n")


if __name__ == "__main__":
    main()
