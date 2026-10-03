# Contributing to MeteoScope Engine

Thank you for your interest in contributing to **MeteoScope Engine**! We welcome contributions from developers of all
skill levels. Whether you are fixing bugs, optimizing algorithms, improving documentation, or proposing new features,
your support is greatly appreciated.

---

## Code of Conduct

By participating in this project, you agree to abide by our [Code of Conduct](CODE_OF_CONDUCT.md). Please ensure all
interactions remain respectful and inclusive.

---

## How to Contribute

### 1. Issue First

Before embarking on significant feature development or refactoring, please open an issue to discuss your ideas. This
ensures your contribution aligns with the project goals and avoids duplicate work.

### 2. Fork & Branch Workflow

We follow the standard GitHub flow:

1. **Fork** the repository to your own GitHub account.
2. **Clone** your fork locally:
   ```bash
   git clone https://github.com/<your-username>/meteoscope-engine.git
   cd meteoscope-engine
   ```
3. Create a descriptive feature branch:
   ```bash
   git checkout -b feat/simd-acceleration
   # or
   git checkout -b fix/csv-empty-line-handling
   ```

### 3. Commit Guidelines (Conventional Commits)

We enforce the [Conventional Commits](https://www.conventionalcommits.org/) standard. Commit messages must be structured
as follows:

```text
<type>(<scope>): <short description in imperative present tense>

[optional body explaining why the change was made]

[optional footer(s), e.g. Closes #123]
```

**Allowed types:**

- `feat`: A new feature or capability.
- `fix`: A bug fix.
- `perf`: A code change that improves performance without altering functionality.
- `refactor`: Code changes that neither fix a bug nor add a feature.
- `docs`: Documentation only changes.
- `style`: Changes that do not affect the meaning of the code (formatting, white-space).
- `test`: Adding missing tests or correcting existing tests.
- `ci`: Changes to CI configuration files or scripts.
- `chore`: Maintenance tasks, dependency bumps, or tool configuration.

**Examples:**

- `feat(io): add memory-mapped file reader for large datasets`
- `fix(parser): handle trailing empty line without crashing`
- `perf(reduction): vectorize monthly aggregation using std::execution::par_unseq`

---

## Coding Standards & C++ Style

To ensure code quality, readability, and maintainability, please adhere to:

1. **Language Standard:** Strict **C++20** standard compliance. Avoid compiler-specific extensions unless guarded by
   preprocessor macros (`#ifdef _WIN32`, etc.).
2. **Naming Conventions:**
    - **Types, Structs & Classes:** `PascalCase` (e.g., `Station`, `CsvParser`, `AnomalyDetector`).
    - **Functions & Methods:** `camelCase` (e.g., `computeMonthlyAverages`, `loadStationsSerial`).
    - **Variables & Parameters:** `camelCase` (e.g., `stationPath`, `passedFilters`).
    - **Constants & Enums:** `UPPER_SNAKE_CASE` (e.g., `LAT_MAX`, `SVG_WIDTH`).
3. **Memory Safety & Performance:**
    - Prefer pass-by-const-reference (`const Type &`) for non-trivial types.
    - Use `std::string_view` and `std::span` for non-owning views where appropriate.
    - Avoid manual dynamic memory allocations (`new` / `delete`). Use RAII containers (`std::vector`,
      `std::unique_ptr`).
    - Eliminate unnecessary allocations in hot parsing loops (prefer `std::from_chars` over `std::stod`/`stringstream`).
4. **Documentation:**
    - Document all public headers and functions using **Doxygen** format (`/** @brief ... @param ... @return ... */`).
    - Keep existing inline comments clean and meaningful.

---

## Verification & Testing

Before submitting a Pull Request, verify your changes:

1. **Clean Compilation:** The project must build cleanly with no warnings on your compiler:
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   ```
2. **Dual-Mode Verification:** Validate that both modes produce identical outputs:
   ```bash
   # Serial run
   ./build/meteoscope data/stations.csv data/measurements.csv --serial
   
   # Parallel run
   ./build/meteoscope data/stations.csv data/measurements.csv --parallel
   ```
3. **Output Integrity:** Check that output CSV files (`maps/serial_anomalies.csv` and `maps/parallel_anomalies.csv`)
   match.

---

## Submitting a Pull Request (PR)

1. Push your branch to your GitHub fork:
   ```bash
   git push origin feat/my-improvement
   ```
2. Open a Pull Request against the `main` branch.
3. Complete the [Pull Request Template](.github/pull_request_template.md).
4. Maintainers will review your PR and may suggest improvements. Once approved, your PR will be squash-merged into
   `main`.

Thank you for contributing!
