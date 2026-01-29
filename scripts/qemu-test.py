#!/usr/bin/env python3
# Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""
QEMU Test Runner for F9 Microkernel

Runs test suite or fault tests under QEMU and parses UART output.
Exits with 0 on success, non-zero on failure.

Usage:
    # Test suite
    python3 scripts/qemu-test.py build/netduinoplus2/f9.elf [-t TIMEOUT]

    # Fault tests (expect kernel panic)
    python3 scripts/qemu-test.py build/netduinoplus2/f9.elf --fault [-t TIMEOUT]
"""

import argparse
import fcntl
import os
import re
import select
import subprocess
import sys
import time
from dataclasses import dataclass, field
from typing import Optional

# Maximum unexpected output lines to buffer (prevent memory growth)
MAX_UNEXPECTED_LINES = 100

# Fault type to expected panic message mapping
FAULT_PANIC_PATTERNS = {
    "mpu_write_to_code": [
        r"Memory fault",
        r"MEMFAULT",
        r"DACCVIOL",
    ],
    "stack_canary_trip": [
        r"Stack overflow",
        r"canary",
    ],
}


@dataclass
class TestResults:
    """Accumulated test results."""

    passed: int = 0
    failed: int = 0
    skipped: int = 0
    tests_run: list = field(default_factory=list)
    exit_code: Optional[int] = None
    unexpected_output: list = field(default_factory=list)


@dataclass
class FaultTestResults:
    """Results for fault tests."""

    expected_fault: Optional[str] = None
    panic_found: bool = False
    panic_message: str = ""
    all_output: list = field(default_factory=list)
    exit_code: int = 1


def parse_test_line(line: str, results: TestResults) -> bool:
    """
    Parse a test output line and update results.

    Returns True if [TEST:EXIT] was found.
    """
    line = line.strip()

    # Match test markers (silent tracking)
    if match := re.match(r"\[TEST:START\]\s+(\S+)", line):
        return False

    if match := re.match(r"\[TEST:RUN\]\s+(\S+)", line):
        test = match.group(1)
        results.tests_run.append(test)
        return False

    if match := re.match(r"\[TEST:PASS\]\s+(\S+)", line):
        results.passed += 1
        return False

    if match := re.match(r"\[TEST:FAIL\]\s+(\S+)", line):
        results.failed += 1
        return False

    if match := re.match(r"\[TEST:SKIP\]\s+(\S+)", line):
        results.skipped += 1
        return False

    if match := re.match(
        r"\[TEST:SUMMARY\]\s+passed=(\d+)\s+failed=(\d+)\s+skipped=(\d+)", line
    ):
        # Verify counts match our tracking
        reported_passed = int(match.group(1))
        reported_failed = int(match.group(2))
        reported_skipped = int(match.group(3))
        if (
            reported_passed != results.passed
            or reported_failed != results.failed
            or reported_skipped != results.skipped
        ):
            print(
                f"[WARN] Count mismatch: tracked {results.passed}/{results.failed}/{results.skipped}, "
                f"reported {reported_passed}/{reported_failed}/{reported_skipped}"
            )
        return False

    if match := re.match(r"\[TEST:EXIT\]\s+(\d+)", line):
        results.exit_code = int(match.group(1))
        return True

    return False


def set_nonblocking(fd):
    """Set file descriptor to non-blocking mode."""
    flags = fcntl.fcntl(fd, fcntl.F_GETFL)
    fcntl.fcntl(fd, fcntl.F_SETFL, flags | os.O_NONBLOCK)


def run_qemu(elf_path: str, timeout: int) -> TestResults:
    """
    Run QEMU and parse test output.

    Args:
        elf_path: Path to the ELF file
        timeout: Timeout in seconds

    Returns:
        TestResults with accumulated test data
    """
    results = TestResults()

    # Find QEMU binary
    qemu = os.environ.get("QEMU", "qemu-system-arm")

    cmd = [
        qemu,
        "-M",
        "netduinoplus2",
        "-nographic",
        "-serial",
        "mon:stdio",
        "-kernel",
        elf_path,
    ]

    print(f"[QEMU] Starting: {' '.join(cmd)}")
    print(f"[QEMU] Timeout: {timeout}s")
    print("-" * 60)

    try:
        proc = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            stdin=subprocess.PIPE,
            text=True,
            encoding="utf-8",
            errors="replace",  # Replace invalid UTF-8 with replacement character
            bufsize=1,
        )
    except FileNotFoundError:
        print(f"[ERROR] QEMU not found: {qemu}")
        print("[ERROR] Install QEMU or set QEMU environment variable")
        results.exit_code = 127
        return results

    # Set stdout to non-blocking to prevent readline() from blocking
    set_nonblocking(proc.stdout.fileno())

    start_time = time.monotonic()  # Use monotonic clock for reliable timeout
    exit_found = False
    test_started = False  # Only show output after test suite starts
    line_buffer = ""  # Buffer for partial lines

    try:
        while True:
            # Read available data with timeout using select
            ready, _, _ = select.select([proc.stdout], [], [], 0.1)

            eof_reached = False
            if ready:
                try:
                    chunk = proc.stdout.read(4096)
                    if chunk:
                        line_buffer += chunk
                    elif chunk == "":  # EOF - stdout closed
                        eof_reached = True
                except (IOError, OSError):
                    pass  # No data available yet (non-blocking)
                except UnicodeDecodeError as e:
                    # Handle invalid UTF-8 (should be rare with errors='replace')
                    print(f"[WARNING] Unicode decode error: {e}", file=sys.stderr)
                    pass

            # Process complete lines (even after EOF to drain buffer)
            while "\n" in line_buffer:
                line, line_buffer = line_buffer.split("\n", 1)
                stripped = line.rstrip()

                # Parse test markers (before display filtering)
                exit_requested = parse_test_line(line, results)

                # Check for test suite start marker
                if "=== Running" in stripped:
                    test_started = True

                # Display only test results (=== Running or Test lines)
                if test_started and stripped:
                    if stripped.startswith("=== ") or stripped.startswith("Test "):
                        sys.stdout.write(f"  {stripped}\n")
                        sys.stdout.flush()
                    elif not stripped.startswith("[TEST:"):
                        # Filter known debug messages (not unexpected)
                        # Use exact prefixes to avoid hiding real errors
                        is_known_debug = stripped.startswith(
                            "IPC: "
                        ) or stripped.startswith("THREAD_CREATE:")
                        # Buffer unexpected output for debugging (bounded)
                        if (
                            not is_known_debug
                            and len(results.unexpected_output) < MAX_UNEXPECTED_LINES
                        ):
                            results.unexpected_output.append(stripped)

                if exit_requested:
                    exit_found = True
                    break

            if exit_found:
                # Give a moment for any final output
                time.sleep(0.1)
                break

            if eof_reached:
                # Process any remaining incomplete line
                if line_buffer.strip():
                    parse_test_line(line_buffer, results)
                break

            # Check timeout AFTER processing available data (use monotonic clock)
            elapsed = time.monotonic() - start_time
            if elapsed > timeout:
                print(f"\n[ERROR] Timeout after {timeout}s")
                results.exit_code = 1  # Timeout is failure
                break

            # Check if process exited (after processing data)
            if proc.poll() is not None:
                # Drain any remaining data
                try:
                    remaining = proc.stdout.read()
                    if remaining:
                        line_buffer += remaining
                        # Process remaining complete lines
                        for line in line_buffer.split("\n"):
                            if line.strip():
                                parse_test_line(line, results)
                except (IOError, OSError):
                    pass
                if not exit_found:
                    rc = proc.returncode
                    print(f"[QEMU] Process exited with code {rc}")
                break

    except KeyboardInterrupt:
        print("\n[QEMU] Interrupted by user")
        results.exit_code = 1

    finally:
        # Terminate QEMU and cleanup resources
        if proc.poll() is None:
            proc.terminate()
            try:
                proc.wait(timeout=2)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait()
        # Close file descriptors
        if proc.stdout:
            proc.stdout.close()
        if proc.stdin:
            proc.stdin.close()

    print("-" * 60)

    # If no exit marker found, treat as failure
    if results.exit_code is None:
        results.exit_code = 1
        if not exit_found:
            print("[ERROR] No [TEST:EXIT] marker found")

    return results


def run_qemu_fault(elf_path: str, timeout: int) -> FaultTestResults:
    """
    Run QEMU for fault tests (expecting kernel panic).

    Args:
        elf_path: Path to the ELF file
        timeout: Timeout in seconds

    Returns:
        FaultTestResults with fault test data
    """
    results = FaultTestResults()

    # Find QEMU binary
    qemu = os.environ.get("QEMU", "qemu-system-arm")

    cmd = [
        qemu,
        "-M",
        "netduinoplus2",
        "-nographic",
        "-serial",
        "mon:stdio",
        "-kernel",
        elf_path,
    ]

    print(f"[QEMU] Starting fault test: {' '.join(cmd)}")
    print(f"[QEMU] Timeout: {timeout}s")
    print("-" * 60)

    try:
        proc = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            stdin=subprocess.PIPE,
            text=True,
            encoding="utf-8",
            errors="replace",  # Replace invalid UTF-8 with replacement character
            bufsize=1,
        )
    except FileNotFoundError:
        print(f"[ERROR] QEMU not found: {qemu}")
        results.exit_code = 127
        return results

    # Set stdout to non-blocking
    set_nonblocking(proc.stdout.fileno())

    start_time = time.monotonic()
    line_buffer = ""

    try:
        while True:
            ready, _, _ = select.select([proc.stdout], [], [], 0.1)

            eof_reached = False
            if ready:
                try:
                    chunk = proc.stdout.read(4096)
                    if chunk:
                        line_buffer += chunk
                    elif chunk == "":
                        eof_reached = True
                except (IOError, OSError):
                    pass
                except UnicodeDecodeError as e:
                    # Handle invalid UTF-8 (should be rare with errors='replace')
                    print(f"[WARNING] Unicode decode error: {e}", file=sys.stderr)
                    pass

            # Process complete lines
            while "\n" in line_buffer:
                line, line_buffer = line_buffer.split("\n", 1)
                stripped = line.rstrip()

                if stripped:
                    # Store all output
                    if len(results.all_output) < MAX_UNEXPECTED_LINES:
                        results.all_output.append(stripped)

                    # Display output
                    sys.stdout.write(f"  {stripped}\n")
                    sys.stdout.flush()

                    # Check for expected fault marker
                    if match := re.match(r"\[FAULT:EXPECT\]\s+(\S+)", stripped):
                        results.expected_fault = match.group(1)
                        print(f"[FAULT] Expecting: {results.expected_fault}")

                    # Check for panic messages
                    if "panic" in stripped.lower() or "PANIC" in stripped:
                        results.panic_found = True
                        results.panic_message = stripped

                    # Check for specific fault patterns
                    if results.expected_fault:
                        patterns = FAULT_PANIC_PATTERNS.get(results.expected_fault, [])
                        for pattern in patterns:
                            if re.search(pattern, stripped, re.IGNORECASE):
                                results.panic_found = True
                                results.panic_message = stripped
                                break

            if eof_reached:
                break

            # Check timeout
            elapsed = time.monotonic() - start_time
            if elapsed > timeout:
                print(f"\n[FAULT] Timeout after {timeout}s")
                break

            # Check if process exited
            if proc.poll() is not None:
                # Drain remaining data
                try:
                    remaining = proc.stdout.read()
                    if remaining:
                        for line in remaining.split("\n"):
                            if line.strip():
                                results.all_output.append(line.strip())
                                sys.stdout.write(f"  {line.strip()}\n")
                                # Check for panic in remaining output
                                if "panic" in line.lower() or "PANIC" in line:
                                    results.panic_found = True
                                    results.panic_message = line.strip()
                except (IOError, OSError):
                    pass
                break

    except KeyboardInterrupt:
        print("\n[QEMU] Interrupted by user")

    finally:
        if proc.poll() is None:
            proc.terminate()
            try:
                proc.wait(timeout=2)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait()
        if proc.stdout:
            proc.stdout.close()
        if proc.stdin:
            proc.stdin.close()

    print("-" * 60)

    # Determine success/failure for fault test
    if results.expected_fault and results.panic_found:
        # Check if panic matches expected fault type
        patterns = FAULT_PANIC_PATTERNS.get(results.expected_fault, [])
        matched = False
        for pattern in patterns:
            if re.search(pattern, results.panic_message, re.IGNORECASE):
                matched = True
                break
        if matched or "panic" in results.panic_message.lower():
            results.exit_code = 0  # Expected panic found = success
        else:
            results.exit_code = 1  # Wrong type of panic
    else:
        results.exit_code = 1  # No expected fault or no panic

    return results


def main():
    parser = argparse.ArgumentParser(description="Run F9 tests under QEMU")
    parser.add_argument("elf_file", help="Path to the ELF file to run")
    parser.add_argument(
        "-t", "--timeout", type=int, default=30, help="Timeout in seconds (default: 30)"
    )
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose output")
    parser.add_argument(
        "--fault", action="store_true", help="Run as fault test (expect kernel panic)"
    )

    args = parser.parse_args()

    # Validate timeout
    if args.timeout <= 0:
        print(f"[ERROR] Timeout must be positive: {args.timeout}")
        sys.exit(1)

    if not os.path.exists(args.elf_file):
        print(f"[ERROR] ELF file not found: {args.elf_file}")
        sys.exit(1)

    if args.fault:
        # Fault test mode
        results = run_qemu_fault(args.elf_file, args.timeout)

        print()
        print("=" * 60)
        if results.expected_fault:
            print(f"FAULT TEST: {results.expected_fault}")
        else:
            print("FAULT TEST: (no expected fault marker found)")
        print("=" * 60)

        if results.exit_code == 0:
            print(f"[RESULT] PASSED - Expected panic detected")
            if results.panic_message:
                print(f"  Panic: {results.panic_message}")
        else:
            print("[RESULT] FAILED")
            if not results.expected_fault:
                print("  No [FAULT:EXPECT] marker found in output")
            elif not results.panic_found:
                print("  Expected panic did not occur")
            else:
                print(f"  Panic message: {results.panic_message}")

        sys.exit(results.exit_code)

    else:
        # Normal test suite mode
        results = run_qemu(args.elf_file, args.timeout)

        print()
        print("=" * 60)
        print(
            f"SUMMARY: {results.passed} passed, {results.failed} failed, {results.skipped} skipped"
        )
        print("=" * 60)

        if results.failed > 0:
            print("[RESULT] FAILED")
            if results.unexpected_output:
                print("\n[DEBUG] Unexpected output:")
                for line in results.unexpected_output[-20:]:
                    print(f"  {line}")
        elif results.passed > 0:
            print("[RESULT] PASSED")
        else:
            print("[RESULT] NO TESTS RUN")
            results.exit_code = 1
            if results.unexpected_output:
                print("\n[DEBUG] Unexpected output:")
                for line in results.unexpected_output[-20:]:
                    print(f"  {line}")

        sys.exit(results.exit_code)


if __name__ == "__main__":
    main()
