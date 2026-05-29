# Testing Documentation

This document describes how to run the various types of tests in the FileMove project.

## Test Types Overview

- **Unit Tests**: Validates individual components (such as `CommandLineParser`) in isolation using Google Test.
- **End-to-End (E2E) Tests**: Verifies complete application workflows, including file movement and configuration loading, within a controlled sandbox environment via a bash script.


## Unit Tests

Unit tests are implemented using **Google Test (GTest)**. They validate individual components like the `CommandLineParser` in isolation.

### Running Unit Tests

To run unit tests, you must first build the project using CMake.

1. **Configure and Build:**
   ```bash
   cd gemma4-win32
   mkdir build && cd build
   cmake ..
   make
   ```

2. **Execute Tests via CTest:**
   The easiest way to run all tests is using `ctest`:
   ```bash
   ctest
   ```

3. **Run Specific Test Binary:**
   Alternatively, you can run the test executable directly:
   ```bash
   ./FileMoveTests
   ```

## End-to-End (E2E) Tests

End-to-end tests verify the entire application workflow from a CLI perspective using a controlled sandbox environment.

### Running E2E Tests

The E2E tests are provided as a bash script that sets up a temporary directory, creates dummy files/configs, and executes the `FileMove` binary.

1. **Ensure the project is built:**
   Make sure you have already built the `FileMove` executable as described in the Unit Tests section.

2. **Run the E2E script:**
   ```bash
   bash gemma4-win32/tests/e2e_test.sh
   ```

## Test Coverage and Reporting

(Placeholder for future coverage reports)
