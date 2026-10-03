## Summary of Changes

Briefly explain the goal of this pull request, what problem it solves, and the technical approach taken.

Fixes # (issue)

---

## Type of Change

- [ ] 🐛 Bug fix (non-breaking change fixing an issue)
- [ ] ✨ New feature (non-breaking change adding functionality)
- [ ] ⚡ Performance optimization (latency reduction, memory savings, faster I/O)
- [ ] ♻️ Code refactoring (no functional change, internal cleanup)
- [ ] 📝 Documentation update
- [ ] 🔧 Build / CI / Tooling improvements

---

## Verification & Testing

Describe how you tested these changes:

- [ ] **Dual-mode verification:** Ran `./meteoscope <data> <data> --serial` and `--parallel` to ensure output
  consistency.
- [ ] **Build verification:** Verified clean build with no compiler warnings on C++20 standard:
  ```bash
  cmake -B build -DCMAKE_BUILD_TYPE=Release
  cmake --build build --config Release
  ```
- [ ] **Output inspection:** Checked CSV outputs and SVG maps for regression.

---

## Contributor Checklist

- [ ] My code adheres to the project's [Coding Standards](CONTRIBUTING.md#coding-standards--c-style).
- [ ] I have self-reviewed my changes.
- [ ] I have commented my code where necessary, particularly in complex algorithmic areas.
- [ ] I have updated relevant documentation (README.md, README.cs.md, or Doxygen comments).
- [ ] My commit messages follow [Conventional Commits](CONTRIBUTING.md#3-commit-guidelines-conventional-commits).
- [ ] No extraneous debug logs, temporary files, or sensitive information were committed.
