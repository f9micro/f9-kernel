#!/usr/bin/env bash
# Copyright (c) 2025 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Format validation for C, shell, and Python files
# The -e is not set because we want to collect all format mismatches at once

set -u -o pipefail

# In CI environment, require all formatting tools to be present
# GitHub Actions sets CI=true automatically (also accepts CI=1, CI=yes, etc.)
REQUIRE_TOOLS="false"
if [ -n "${CI:-}" ]; then
	REQUIRE_TOOLS="true"
fi

C_FORMAT_EXIT=0
SH_FORMAT_EXIT=0
PY_FORMAT_EXIT=0

# Use git ls-files to exclude submodules and untracked files
# F9 Microkernel structure: include/, kernel/, platform/, user/, board/, loader/
# Exclude: externals/ (third-party code with own style)
C_SOURCES=()
while IFS= read -r file; do
	[ -n "$file" ] && C_SOURCES+=("$file")
done < <(git ls-files -- 'include/*.h' 'include/**/*.h' 'kernel/*.c' 'platform/*.c' 'platform/**/*.c' 'platform/*.h' 'platform/**/*.h' 'user/*.c' 'user/**/*.c' 'user/*.h' 'user/**/*.h' 'board/*.c' 'board/**/*.c' 'board/*.h' 'board/**/*.h' 'loader/*.c' 'loader/**/*.c' 'loader/*.h' 'loader/**/*.h' | grep -v '^externals/' || true)

if [ ${#C_SOURCES[@]} -gt 0 ]; then
	# Try clang-format with version suffix first, then generic
	CLANG_FORMAT=""
	for cmd in clang-format-21 clang-format-20 clang-format-19 clang-format; do
		if command -v "$cmd" >/dev/null 2>&1; then
			CLANG_FORMAT="$cmd"
			break
		fi
	done

	if [ -n "$CLANG_FORMAT" ]; then
		echo "Checking ${#C_SOURCES[@]} C/C++ files with $CLANG_FORMAT..."
		$CLANG_FORMAT -n --Werror "${C_SOURCES[@]}"
		C_FORMAT_EXIT=$?
		if [ $C_FORMAT_EXIT -eq 0 ]; then
			echo "  ✓ All C/C++ files properly formatted"
		else
			echo "  ✗ C/C++ format violations found" >&2
		fi
	else
		if [ "$REQUIRE_TOOLS" = "true" ]; then
			echo "ERROR: clang-format not found (required in CI)" >&2
			C_FORMAT_EXIT=1
		else
			echo "Skipping C format check: clang-format not found" >&2
		fi
	fi
else
	echo "No C/C++ files to check"
fi

SH_SOURCES=()
while IFS= read -r file; do
	[ -n "$file" ] && SH_SOURCES+=("$file")
done < <(git ls-files -- '*.sh' '.ci/*.sh' 'scripts/*.sh' | grep -v '^externals/' || true)

if [ ${#SH_SOURCES[@]} -gt 0 ]; then
	if command -v shfmt >/dev/null 2>&1; then
		echo "Checking ${#SH_SOURCES[@]} shell scripts with shfmt..."
		MISMATCHED_SH=$(shfmt -l "${SH_SOURCES[@]}")
		if [ -n "$MISMATCHED_SH" ]; then
			echo "  ✗ The following shell scripts need formatting:" >&2
			printf '    %s\n' $MISMATCHED_SH >&2
			echo "  Run: shfmt -w \$(git ls-files '*.sh' '.ci/*.sh' 'scripts/*.sh')" >&2
			SH_FORMAT_EXIT=1
		else
			echo "  ✓ All shell scripts properly formatted"
		fi
	else
		if [ "$REQUIRE_TOOLS" = "true" ]; then
			echo "ERROR: shfmt not found (required in CI)" >&2
			SH_FORMAT_EXIT=1
		else
			echo "Skipping shell script format check: shfmt not found" >&2
		fi
	fi
else
	echo "No shell scripts to check"
fi

PY_SOURCES=()
while IFS= read -r file; do
	[ -n "$file" ] && PY_SOURCES+=("$file")
done < <(git ls-files -- 'scripts/*.py' | grep -v '^externals/' || true)

if [ ${#PY_SOURCES[@]} -gt 0 ]; then
	if command -v black >/dev/null 2>&1; then
		echo "Checking ${#PY_SOURCES[@]} Python files with black..."
		black --check --quiet "${PY_SOURCES[@]}" 2>&1
		PY_FORMAT_EXIT=$?
		if [ $PY_FORMAT_EXIT -eq 0 ]; then
			echo "  ✓ All Python files properly formatted"
		else
			echo "  ✗ Python format violations found" >&2
			echo "  Run: black scripts/*.py" >&2
		fi
	else
		if [ "$REQUIRE_TOOLS" = "true" ]; then
			echo "ERROR: black not found (required in CI)" >&2
			PY_FORMAT_EXIT=1
		else
			echo "Skipping Python format check: black not found" >&2
		fi
	fi
else
	echo "No Python files to check"
fi

# Summary
echo ""
echo "Format check summary:"
echo "  C/C++:  $([ $C_FORMAT_EXIT -eq 0 ] && echo "✓ PASS" || echo "✗ FAIL")"
echo "  Shell:  $([ $SH_FORMAT_EXIT -eq 0 ] && echo "✓ PASS" || echo "✗ FAIL")"
echo "  Python: $([ $PY_FORMAT_EXIT -eq 0 ] && echo "✓ PASS" || echo "✗ FAIL")"

# Use logical OR to avoid exit code overflow (codes are mod 256)
if [ $C_FORMAT_EXIT -ne 0 ] || [ $SH_FORMAT_EXIT -ne 0 ] || [ $PY_FORMAT_EXIT -ne 0 ]; then
	echo ""
	echo "Format check FAILED"
	exit 1
fi

echo ""
echo "Format check PASSED"
exit 0
