#!/usr/bin/env python3

import argparse
import csv
import hashlib
from itertools import product
import json
import os
import platform
import shutil
import stat
import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SDK_HASH = "75d6d2171f845f356a2c9f445b963217fca0f8c4d741ffcea08f04ec9146464a"
MODULE = {
    "Lab1": "7DCE6311C35A5497A9FCE0931F6192CF",
    "Lab2": "C1D0D1064B165177A568B00307CA864B",
    "A": "4828F6552E4540E78121EBD220DC360E",
    "B": "AE202E543CE54550899603BD70C62565",
    "C": "4828F6552E4540E78121EBD220DC360E",
    "D": "3A8E44677E82475CB4A3719ED8397E61",
    "E": "872FEF1DE3314B87AD44D1E7C232C2F0",
}
ECO_NOINTERFACE = -19
ECO_COMPONENT_NOTFOUND = -23
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


def run(command, directory=ROOT):
    subprocess.run(command, cwd=directory, check=True, timeout=240)


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


def lesson_directory(education, letter):
    lesson = {"A": 2, "B": 3, "C": 5, "D": 6, "E": 7}[letter]
    return education / "Eco.Pro.Training/001.InsideACOM/Lessons" / (
        "Lesson%02d" % lesson) / ("Eco.Calculator" + letter)


def build_calculators(education, flags, output):
    if not lesson_directory(education, "A").is_dir():
        raise RuntimeError("Supply --education PATH to the Eco.Education checkout (see README).")
    headers = ["-I" + str(lesson_directory(education, name) / "SharedFiles") for name in "AB"]
    result = {}
    for letter in "ABCDE":
        lesson = lesson_directory(education, letter)
        target = output / (letter + ".dylib")
        # The lessons use the previous SDK spelling of the success constant.
        run(["clang", *flags, "-DECO_DLL", "-DERR_ECO_SUCCESES=ERR_ECO_SUCCESS",
             "-I" + str(lesson / "SharedFiles"), "-I" + str(lesson / "HeaderFiles"),
             *headers, "-dynamiclib", *sorted((lesson / "SourceFiles").glob("*.c")),
             "-o", target])
        result[letter] = target
    return result


def build_server(lab, server, flags, output):
    common = ROOT / "common/src"
    sources = [common / "EcoFactory.c", common / "EcoObject.c",
               ROOT / f"lab{lab}/SourceFiles/CEcoLab{server}.c"]
    if server == 1:
        sources.append(common / "BitonicSort.c")
        if lab == 2:
            sources.append(ROOT / "lab2/SourceFiles/Composition.c")
    target = output / (MODULE[f"Lab{server}"] + ".dylib")
    run(["clang", *flags, "-DECO_DLL", "-DARPO_SERVER=" + str(server),
         "-ffreestanding", "-fno-builtin", "-dynamiclib", "-install_name",
         "@rpath/" + target.name, *sources, "-o", target])
    symbols = subprocess.check_output(["nm", "-u", target], text=True)
    imported = {line.split()[-1].lstrip("_") for line in symbols.splitlines() if line.split()}
    if imported & FORBIDDEN:
        raise RuntimeError("Forbidden component imports: " + ", ".join(sorted(imported & FORBIDDEN)))
    return target


def combinations():
    for ac, b, d, e in product(("", "A", "C"), ("", "B"), ("", "D"), ("", "E")):
        # E aggregates B. D needs B or a provider under the A/C identifier.
        complete = bool((not e or b) and (ac or b) and (d or e or ac == "C"))
        yield ac + b + d + e, complete


def build(lab, args, framework):
    output = ROOT / "build" / f"lab{lab}" / ("dynamic-" + args.command)
    output.mkdir(parents=True, exist_ok=True)
    (output / "result.json").unlink(missing_ok=True)
    (output / "benchmark.csv").unlink(missing_ok=True)
    sdk_flags = ["-DECO_APPLE", "-DECO_MAC", "-DECO_AARCH64", "-DUGUID_UTILITY"]
    for directory in sorted(framework.glob("*/SharedFiles")):
        sdk_flags += ["-isystem", str(directory)]
    flags = [*sdk_flags, "-std=c89", "-pedantic-errors", "-Wall", "-Wextra", "-Werror",
             "-Wno-unused-const-variable", "-O2", "-Iinclude", "-Icommon/include", "-Iclient"]
    if args.command == "test" and not args.no_sanitize:
        flags += ["-fsanitize=address,undefined", "-fno-sanitize-recover=all"]
    modules = {"Lab1": build_server(lab, 1, flags, output)}
    if lab == 2:
        modules["Lab2"] = build_server(lab, 2, flags, output)
        modules.update(build_calculators(args.education.resolve(), sdk_flags, output))
    sources = [ROOT / "client/ClientRuntime.c"]
    if args.command == "benchmark" or (args.command == "demo" and lab == 1):
        source = "client/Benchmark.c" if args.command == "benchmark" else "lab1/Demo.c"
        sources += [ROOT / source, ROOT / "client/ParallelExecutor.c"]
    else:
        sources += [ROOT / "client/UnitTestMain.c", ROOT / "lab1/UnitTest.c"]
        if lab == 2:
            sources.append(ROOT / "lab2/UnitTest.c")
    client = output / "client"
    runtime = next(framework.glob("Eco.System1/BuildFiles/Mac/arm64/DynamicRelease/*.a"))
    run(["clang", *flags, "-DARPO_LAB=" + str(lab), f"-DTEST_CLASS=CID_EcoLab{lab}", *sources, runtime, "-o", client])
    if args.command == "build":
        print("Built", output)
        return
    cases = list(combinations()) if lab == 2 and args.command == "test" else [
        ("BDE" if lab == 2 else "", True)]
    results = []
    for bundle, complete in cases:
        with tempfile.TemporaryDirectory(prefix="case-", dir=output) as case:
            directory = Path(case)
            shutil.copy2(client, directory / "client")
            for lib in framework.glob("*/BuildFiles/Mac/arm64/DynamicRelease/*.dylib"):
                shutil.copy2(lib, directory)
            for name in (["Lab1", "Lab2", *bundle] if lab == 2 else ["Lab1"]):
                shutil.copy2(modules[name], directory / (MODULE[name] + ".dylib"))
            run_result = subprocess.run(["./client"], cwd=directory, capture_output=True,
                                        text=True, timeout=240)
        if run_result.stderr:
            print(run_result.stderr, file=sys.stderr, end="")
        label = bundle or ("no calculators" if lab == 2 else "sort")
        if run_result.stdout.strip():
            print(run_result.stdout.strip())
        if complete:
            passed = run_result.returncode == 0
        else:
            # The SDK returns the signed Eco error as a process exit code.
            passed = any(run_result.returncode == code % 256 and
                         f"QueryComponent failed: {code}" in run_result.stdout.splitlines()
                         for code in (ECO_COMPONENT_NOTFOUND, ECO_NOINTERFACE))
        outcome = "OK" if complete else "expected rejection"
        print(f"Lab{lab} [{label}]: {outcome if passed else 'FAIL'}", flush=True)
        results.append({"calculators": bundle, "expected_creation": complete,
                        "exit": run_result.returncode, "passed": passed})
        if args.command == "benchmark" and passed:
            rows = [line.split(",")[1:] for line in run_result.stdout.splitlines()
                    if line.startswith("BENCH,")]
            with (output / "benchmark.csv").open("w", newline="") as target:
                writer = csv.writer(target)
                writer.writerow(["lab", "n", "pattern", "method", "repeat", "seconds", "exact"])
                writer.writerows(rows)
    summary = {"client_sha256": hashlib.sha256(client.read_bytes()).hexdigest(), "cases": results}
    (output / "result.json").write_text(json.dumps(summary, indent=2) + "\n")
    if not all(case["passed"] for case in results):
        raise RuntimeError(f"Unexpected result; see {output / 'result.json'}")
    print(f"Lab{lab}: {len(results)} expected results")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("command", choices=["setup", "build", "demo", "test", "benchmark"])
    parser.add_argument("--lab", choices=["1", "2", "all"], default="all")
    parser.add_argument("--framework", type=Path)
    parser.add_argument("--archive", type=Path)
    parser.add_argument("--no-sanitize", action="store_true")
    parser.add_argument("--education", type=Path, default=ROOT / ".eco/Eco.Education")
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
            build(lab, args, framework)
    except (RuntimeError, OSError, subprocess.SubprocessError) as error:
        parser.exit(1, str(error) + "\n")


if __name__ == "__main__":
    main()
