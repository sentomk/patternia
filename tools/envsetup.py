#!/usr/bin/env python3
"""Install and verify Patternia's pinned development tools.

The tools listed in tools/requirements.txt are installed into the ignored
.tools/venv directory, so every contributor and every CI job formats with
exactly the same clang-format.
"""

from __future__ import annotations

import hashlib
import os
import subprocess
import sys
import venv
from pathlib import Path
from typing import Dict, NoReturn

repository_root = Path(__file__).resolve().parents[1]
requirements_path = repository_root / "tools" / "requirements.txt"
tool_root = repository_root / ".tools"
environment_path = tool_root / "venv"
stamp_path = tool_root / "requirements.sha256"


def fail(message: str) -> NoReturn:
    print(message, file=sys.stderr)
    raise SystemExit(2)


def read_pins() -> Dict[str, str]:
    pins: Dict[str, str] = {}
    for raw_line in requirements_path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split("==")
        if len(parts) != 2 or not all(parts):
            fail(f"tool requirement must be an exact pin: {line}")
        pins[parts[0].lower().replace("_", "-")] = parts[1]
    if set(pins) != {"clang-format"}:
        fail("tools/requirements.txt must pin exactly: clang-format")
    return pins


def executable(name: str) -> Path:
    scripts = environment_path / ("Scripts" if os.name == "nt" else "bin")
    return scripts / (f"{name}.exe" if os.name == "nt" else name)


def clang_format_path() -> Path:
    return executable("clang-format")


def requirements_digest() -> str:
    return hashlib.sha256(requirements_path.read_bytes()).hexdigest()


def first_line(command: list) -> str:
    result = subprocess.run(
        command,
        cwd=repository_root,
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    return result.stdout.splitlines()[0]


def verify_tools(pins: Dict[str, str]) -> str:
    version = first_line([str(clang_format_path()), "--version"])
    if f" version {pins['clang-format']}" not in version:
        fail(f"expected clang-format {pins['clang-format']}, got: {version}")
    return version


def pip_available(python: Path) -> bool:
    if not python.is_file():
        return False
    try:
        result = subprocess.run(
            [str(python), "-m", "pip", "--version"],
            check=False,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
    except OSError:
        return False
    return result.returncode == 0


def create_environment() -> None:
    try:
        venv.EnvBuilder(with_pip=True, clear=True).create(environment_path)
    except (OSError, subprocess.CalledProcessError):
        fail(
            "could not create .tools/venv with pip; install Python's "
            "venv/ensurepip support (Debian/Ubuntu: python3-venv) and rerun"
        )
    if not pip_available(executable("python")):
        fail(
            "created .tools/venv without pip; install Python's "
            "venv/ensurepip support (Debian/Ubuntu: python3-venv) and rerun"
        )


def setup_environment(verbose: bool = False) -> Path:
    """Make sure the pinned tools exist and return the clang-format path."""
    pins = read_pins()
    digest = requirements_digest()
    installed = (
        stamp_path.is_file()
        and stamp_path.read_text(encoding="utf-8").strip() == digest
        and clang_format_path().is_file()
    )

    if not installed:
        print("Installing pinned tools into .tools/venv ...", file=sys.stderr, flush=True)
        tool_root.mkdir(parents=True, exist_ok=True)
        if not pip_available(executable("python")):
            create_environment()
        subprocess.run(
            [
                str(executable("python")),
                "-m",
                "pip",
                "install",
                "--disable-pip-version-check",
                "--no-input",
                "--no-deps",
                "--upgrade",
                "--requirement",
                str(requirements_path),
            ],
            cwd=repository_root,
            check=True,
            stdout=sys.stderr,
        )
        version = verify_tools(pins)
        stamp_path.write_text(f"{digest}\n", encoding="utf-8")
    else:
        version = verify_tools(pins)

    if verbose:
        print(version)
    return clang_format_path()


def main() -> int:
    arguments = sys.argv[1:]
    if arguments in (["-h"], ["--help"]):
        print("usage: tools/envsetup.py [--versions]")
        return 0
    if arguments not in ([], ["--versions"]):
        print("usage: tools/envsetup.py [--versions]", file=sys.stderr)
        return 2
    setup_environment(verbose=arguments == ["--versions"])
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
