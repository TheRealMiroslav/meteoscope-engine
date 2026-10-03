---
name: Bug Report
about: Create a report to help us fix a bug or unexpected behavior
title: "[BUG] "
labels: [ "bug" ]
assignees: ""
---

### Problem Description

A clear and concise description of what the bug is.

### Reproduction Steps

Steps to reproduce the behavior:

1. Prepare input datasets `stations.csv` and `measurements.csv` (or describe their structure).
2. Execute command:
   ```bash
   ./meteoscope <path_to_stations> <path_to_measurements> <--serial|--parallel>
   ```
3. Observe output / error.

### Expected Behavior

A clear and concise description of what you expected to happen.

### Actual Output / Traceback

```text
(Paste terminal output, crash stack trace, or unexpected CSV line here)
```

### Environment Information

- **OS:** (e.g. Windows 11, Ubuntu 22.04, macOS Sonoma)
- **Compiler:** (e.g. MSVC 19.38, GCC 12.2, Clang 16.0)
- **CMake Version:** (e.g. 3.28)
- **Execution Mode:** (`--serial` or `--parallel`)
- **Dataset Size:** (e.g. Small / Medium / Large / Custom)

### Additional Context

Add any other context, sample rows from problematic CSVs, or logs about the problem here.
