# Agent guide

This file is for coding agents working in this repository. Humans should
start with [CONTRIBUTING.md](CONTRIBUTING.md), which holds the full rules;
this page only points at them and lists the agent skills.

## Build and test

```bash
cmake -S . -B build -DPTN_BUILD_TESTS=ON
cmake --build build --target ptn_tests
ctest --test-dir build --output-on-failure
```

Patternia is a header-only C++17 library under `include/ptn/`. Logic
changes need tests under `tests/`. Formatting is checked in CI with the
`clang-format` version pinned in `tools/requirements.txt`.

## Changes and pull requests

- `main` is protected. Never push to it; open a pull request from a
  branch (`feat/`, `fix/`, `docs/`, `test/`, `chore/`, or `release/X.Y.Z`
  for a release).
- One commit per pull request. Commit subjects and PR titles use
  Conventional Commits (`scripts/check_commit_msg.py` checks them); body
  prose wraps at 72 characters.
- PR bodies need the bold sections `**Summary**`, `**Changes**` and
  `**Testing**`.
- Do not add attribution trailers or "generated with" lines to commits,
  pull requests, release notes or downstream pull requests.

## Skills

Agent skills live in `ai/skills/<name>/SKILL.md`. Each file starts with
`name` and `description` front matter and then holds plain Markdown
instructions, so any agent can read it directly. Load a skill when its
description matches the task.

| Skill | Use when |
|---|---|
| [`release`](ai/skills/release/SKILL.md) | Preparing or publishing a version: release PR, tag, GitHub release, vcpkg |

Claude Code only discovers skills under `.claude/skills/`, so each
skill there is a small `SKILL.md` stub with the same front matter that
tells the agent to read the real file in `ai/skills/`. Stubs are plain
files rather than symlinks so they also work on Windows clones with
`core.symlinks=false`. Add a skill by creating
`ai/skills/<name>/SKILL.md`, adding a row above, and copying
[`.claude/skills/release/SKILL.md`](.claude/skills/release/SKILL.md) to
`.claude/skills/<name>/SKILL.md` with that skill's name, description
and path.
