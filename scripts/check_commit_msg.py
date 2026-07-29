#!/usr/bin/env python3
"""Validate commit message format and style.

Enforces Conventional Commits structure plus a subset of the Google
CL-description style guidance that can be checked mechanically:

- ``type(scope): description`` structure with an allowed type
- description written in the imperative mood ("add", not "added"/"adds")
- description starts lowercase and has no trailing period
- subject length within a sane limit
- description is not a vague placeholder ("fix bug", "wip", "update")

Subjective guidance (body explains what + why, mentions shortcomings, etc.)
lives in CONTRIBUTING.md and is not enforced here.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

ALLOWED_TYPES = (
    "feat",
    "fix",
    "docs",
    "style",
    "refactor",
    "perf",
    "test",
    "chore",
    "build",
    "ci",
    "revert",
)

# type(scope)!: description  — scope and '!' optional. Description captured.
CONVENTIONAL_RE = re.compile(
    r"^(?P<type>" + "|".join(ALLOWED_TYPES) + r")"
    r"(?P<scope>\([a-z0-9_,./-]+\))?"
    r"(?P<breaking>!)?"
    r": (?P<desc>.+)$"
)

# Soft/hard limits for the whole subject line.
SUBJECT_SOFT_LIMIT = 50
SUBJECT_HARD_LIMIT = 72

# Common non-imperative first words: past tense, gerunds, and 3rd-person
# singular. The imperative form is the bare verb ("add", "fix", "remove").
NON_IMPERATIVE = {
    # past tense / participles
    "added": "add",
    "created": "create",
    "removed": "remove",
    "deleted": "delete",
    "fixed": "fix",
    "changed": "change",
    "updated": "update",
    "renamed": "rename",
    "moved": "move",
    "refactored": "refactor",
    "implemented": "implement",
    "introduced": "introduce",
    "improved": "improve",
    "bumped": "bump",
    "wired": "wire",
    "dropped": "drop",
    "merged": "merge",
    "reverted": "revert",
    "supported": "support",
    "enabled": "enable",
    "disabled": "disable",
    # gerunds
    "adding": "add",
    "creating": "create",
    "removing": "remove",
    "deleting": "delete",
    "fixing": "fix",
    "changing": "change",
    "updating": "update",
    "renaming": "rename",
    "moving": "move",
    "refactoring": "refactor",
    "implementing": "implement",
    "introducing": "introduce",
    "improving": "improve",
    "bumping": "bump",
    "wiring": "wire",
    "dropping": "drop",
    "merging": "merge",
    "reverting": "revert",
    "supporting": "support",
    "enabling": "enable",
    "disabling": "disable",
    # 3rd-person singular
    "adds": "add",
    "creates": "create",
    "removes": "remove",
    "deletes": "delete",
    "fixes": "fix",
    "changes": "change",
    "updates": "update",
    "renames": "rename",
    "moves": "move",
    "implements": "implement",
    "improves": "improve",
}

# Vague, low-information descriptions the Google guide calls out ("Fix bug",
# "Add patch", "Phase 1", …). Matched case-insensitively against the whole
# description, after normalising whitespace.
VAGUE_DESCRIPTIONS = {
    "fix bug",
    "fix bugs",
    "fix build",
    "fix it",
    "fix test",
    "fix tests",
    "fix stuff",
    "add patch",
    "add code",
    "add stuff",
    "add convenience functions",
    "update",
    "updates",
    "update code",
    "changes",
    "change code",
    "stuff",
    "wip",
    "misc",
    "cleanup",
    "minor changes",
    "small fixes",
}


def read_message(path: Path) -> str:
    text = path.read_text(encoding="utf-8")
    return text.splitlines()[0].strip() if text else ""


def is_exempt(subject: str) -> bool:
    if not subject:
        return False
    if subject.startswith("Merge "):
        return True
    if subject.startswith("fixup!") or subject.startswith("squash!"):
        return True
    if subject.startswith("Revert "):
        return True
    return False


def check_description(desc: str, errors: list[str]) -> None:
    """Apply Google-style content checks to the description segment."""
    normalized = re.sub(r"\s+", " ", desc.strip()).lower()

    # Vague / placeholder descriptions.
    if normalized in VAGUE_DESCRIPTIONS:
        errors.append(
            f"description {desc.strip()!r} is too vague — say specifically what "
            "changed (e.g. 'add rel_ops pattern combinator', not 'update')"
        )

    # Trailing period on the subject.
    if desc.rstrip().endswith("."):
        errors.append("description must not end with a period")

    # Capitalization: description should start lowercase (the type is the
    # sentence opener in conventional commits). Allow acronyms / identifiers
    # that are all-caps or contain internal capitals (e.g. 'DSL', 'API').
    first = desc.strip().split(" ", 1)[0]
    if first[:1].isupper() and not (first.isupper() or any(c.isupper() for c in first[1:])):
        errors.append(
            f"description should start lowercase ('{first.lower()}', not '{first}')"
        )

    # Imperative mood: reject past tense / gerund / 3rd-person first words.
    first_word = re.sub(r"[^a-zA-Z]", "", first).lower()
    if first_word in NON_IMPERATIVE:
        errors.append(
            f"use the imperative mood: '{NON_IMPERATIVE[first_word]}', "
            f"not '{first_word}' (write the subject as an order)"
        )


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: check_commit_msg.py <commit-msg-file>", file=sys.stderr)
        return 2

    subject = read_message(Path(sys.argv[1]))
    if is_exempt(subject):
        return 0

    errors: list[str] = []
    match = CONVENTIONAL_RE.match(subject)
    if not match:
        errors.append(
            "subject must use conventional commits: "
            "type(scope): description "
            f"(types: {', '.join(ALLOWED_TYPES)})"
        )
    else:
        desc = match.group("desc")
        if len(desc) < 4:
            errors.append("description must be at least four characters")
        else:
            check_description(desc, errors)

    # Subject length is advisory, not a hard failure: squash-merge appends
    # " (#NN)" which inflates the final subject, and the repo has a long
    # history of informative >50-char subjects. Warn but do not block.
    if len(subject) > SUBJECT_HARD_LIMIT:
        print(
            f"Commit message warning: subject is {len(subject)} chars; "
            f"aim for ≤ {SUBJECT_SOFT_LIMIT}, keep under {SUBJECT_HARD_LIMIT} "
            "where practical.",
            file=sys.stderr,
        )

    if errors:
        print("Commit message check failed:", file=sys.stderr)
        print(f"  subject: {subject!r}", file=sys.stderr)
        for error in errors:
            print(f"  - {error}", file=sys.stderr)
        print(
            "\nExample: feat(patterns): add rel_ops pattern combinator",
            file=sys.stderr,
        )
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
