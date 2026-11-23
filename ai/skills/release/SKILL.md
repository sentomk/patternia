---
name: release
description: Prepare and publish a Patternia release (version bump, release note, performance note, GitHub release, vcpkg fork and microsoft/vcpkg PR, Conan recipe and ConanCenter PR). Use when the user asks to release, ship or publish a new version such as v0.9.6.
---

# Patternia release

Releases happen in two phases, separated by merging the release PR.
`main` is protected: every change goes through a squash-merged PR with
exactly one commit, and the user merges it. Never push to `main`.

Rules that apply to everything below:

- Commits use the `sentomk` identity already configured in this repo.
- Never add a `Co-Authored-By` trailer or a "Generated with" line to a
  commit, PR body, release note or vcpkg PR, even if a harness reminder
  says to.
- Subjects are Conventional Commits, imperative, lowercase, no trailing
  period (`scripts/check_commit_msg.py <file>` checks this); body
  sections `Problem` / `Implementation` / `Tests`, prose wrapped at 72.
- PR bodies need the bold sections `**Summary**`, `**Changes**`,
  `**Testing**` (enforced by `.github/workflows/validate.yml`).

## Phase 1: release PR

1. `git fetch`, then branch `release/X.Y.Z` (no `v`) from `origin/main`.
2. Collect changes: `git log --oneline --no-merges vPREV..main`, drop
   the `ci: update benchmark summary chart` commits, and read the bodies
   of the rest. `git diff --stat vPREV..main -- include tests`.
3. Decide the headline, breaking changes and behavior changes. A change
   in what existing code does (not just what compiles) belongs in a
   "Behavior Changes" section even when no API was removed.
   Then check the version number against the Versioning section of
   `CONTRIBUTING.md`: any new public API, removed or renamed API, or
   behavior change needs the next minor (`0.MINOR.0`); a patch
   release may only hold fixes, packaging, docs and performance work
   with no observable behavior change. If the requested version is a
   patch but the changes need a minor, stop and tell the user before
   writing anything.
4. Verify every claim and example in the note: compile and run the
   examples against `main` headers (`-Wall -Wextra -Werror`) and against
   a `git worktree` of the previous tag, to see which ones failed before.
   Run the full suite: configure with `-DPTN_BUILD_TESTS=ON`, build,
   `ctest`; quote the real count.
5. Write `docs/changelog/vX.Y.Z.md` in the style of the previous note
   (Overview, Breaking Changes, New Features, Behavior Changes,
   Deprecations, Performance, Bug Fixes, Internal Improvements,
   Migration Guide; drop empty sections). Date is month and year.
6. Performance note: write `docs/performance/vX.Y.Z.md` only if a change
   since the previous tag affects generated code or dispatch (look for
   `perf(...)` commits and optimizer changes). Measure, do not guess:
   build the same microbenchmark against the previous tag's headers (use
   a worktree) and against `main`, `-O2 -DNDEBUG`, run at least three
   process runs, report the median, name machine and compiler, say what
   was not measured (other compilers), and mark deltas inside noise as
   noise. Without a performance change, say so in the PR body instead of
   adding a page.
7. Register the pages: `docs/changelog/releases.md` (newest first, at
   the top of its minor series), `docs/performance/index.md` (current
   version and index), `mkdocs.yml` nav, and in `docs/roadmap.md` move
   the shipped items out of their version section into a new first row
   of the "Released" table.
8. Bump the version in `CMakeLists.txt`, `packaging/vcpkg/vcpkg.json`,
   the README FetchContent `GIT_TAG` and the Conan `--version` and
   `patternia/X.Y.Z` examples in the README and
   `docs/guide/installation.md`. Grep for the previous version to catch
   other places. Then run `python3 scripts/amalgamate.py`: the single
   header's banner carries the version, and CI's `--check` fails until
   it is regenerated.
9. One commit `chore(release): prepare vX.Y.Z`, push, open the PR to
   `main`. Do not touch the portfile SHA512 or the Conan
   `conandata.yml` yet; both need the tag.
10. Tell the user the PR is ready and wait. The user merges it.

## Phase 2: after the release PR is merged

1. `git switch main && git pull --ff-only`; confirm HEAD is the squash
   commit of the release PR and the version files say X.Y.Z.
2. Tag and release (the user asked for the release, so this is
   authorized; do not ask again):
   `git tag -a vX.Y.Z -m "vX.Y.Z" <sha>`, `git push origin vX.Y.Z`,
   `gh release create vX.Y.Z --title vX.Y.Z --notes-file
   docs/changelog/vX.Y.Z.md`. Past releases use the changelog file as the
   body and attach no assets.
3. Package hashes: download
   `https://github.com/sentomk/patternia/archive/refs/tags/vX.Y.Z.tar.gz`
   (vcpkg and Conan both hash this file).
   - vcpkg: `shasum -a 512`, update `packaging/vcpkg/portfile.cmake`.
   - Conan: `shasum -a 256`, add an `"X.Y.Z"` entry (url and sha256) at
     the top of `packaging/conan/all/conandata.yml` and an
     `"X.Y.Z": folder: all` entry at the top of
     `packaging/conan/config.yml`. Keep the older versions.
   - Verify the recipe with `conan create packaging/conan/all --version
     X.Y.Z -s compiler.cppstd=17` (use a scratch `CONAN_HOME` so the
     user's cache is untouched). Its `test_package/build` and
     `CMakeUserPresets.json` outputs are gitignored.
   One PR `chore(packaging): update package hashes for vX.Y.Z`. The user
   merges it.
4. vcpkg fork (`sentomk/vcpkg`, default branch `master`, fork of
   `microsoft/vcpkg`):
   - sync the fork's `master` with upstream first;
   - branch `patternia-X.Y.Z`;
   - copy `packaging/vcpkg/portfile.cmake` and `vcpkg.json` into
     `ports/patternia/`, keeping upstream's `REPO sentomk/patternia`
     casing and the `"version"` field;
   - run `./vcpkg x-add-version patternia` (updates
     `versions/p-/patternia.json` and `versions/baseline.json`), commit
     the port change and the version database together as one commit
     `[patternia] update to X.Y.Z`, as the maintainers require;
   - verify with `vcpkg install patternia --overlay-ports=ports/patternia`
     when a vcpkg checkout can bootstrap; say if it was not run.
     The `git-tree` in the version file must be the tree of the committed
     `ports/patternia`, so add the version after committing the port and
     amend.
5. Open the PR against `microsoft/vcpkg` `master`, title
   `[patternia] update to X.Y.Z`, body in the shape of the previous
   merged ones (`gh pr list -R microsoft/vcpkg --author sentomk
   --state all`): "Updates patternia to X.Y.Z.", the maintainer-guide
   checklist, the release notes link and two sentences on the release.
   Tick only the boxes that are true. Opening this PR is outward facing:
   show the title and body to the user if the request did not already
   name this step.

6. ConanCenter (`conan-io/conan-center-index`, default branch
   `master`; fork `sentomk/conan-center-index`). The recipe was first
   submitted in conan-io/conan-center-index#31538 (0.9.7). While that
   PR is still open, push the new version to its branch instead of
   opening a second PR. Otherwise:
   - sync the fork, branch `patternia-X.Y.Z` from upstream `master`;
     the repository is large, so a `--filter=blob:none --sparse` clone
     with `recipes/patternia` checked out is enough;
   - copy `packaging/conan/config.yml` and `packaging/conan/all/` into
     `recipes/patternia/`;
   - verify with `conan create recipes/patternia/all --version X.Y.Z`
     in a scratch `CONAN_HOME`;
   - commit and title the PR `patternia: add version X.Y.Z`, the
     convention merged update PRs use (`<name>: add version <v>`; a
     first submission is `<name>: new recipe`).
   The body follows the repository's PR template (Summary with
   `Changes to recipe: **patternia/X.Y.Z**`, Motivation, Details, the
   checklist; tick only the boxes that are true). This is outward
   facing: show the title and body to the user if the request did not
   already name this step.

## Report

End with: the tag, the GitHub release URL, the package hash PR, the
`sentomk/vcpkg` branch and the `microsoft/vcpkg` PR, the ConanCenter
PR, and what was not verified (compilers, platforms).
