#!/usr/bin/env bash
# Copyright (c) 2025 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Check all tracked text files for trailing newlines
# Excludes: externals/ (third-party code), binary files, build artifacts
# Reference: https://medium.com/@alexey.inkin/how-to-force-newline-at-end-of-files-and-why-you-should-do-it-fdf76d1d090e

set -e -u -o pipefail

ret=0
show=0
checked=0
failed=0
echo "Checking for missing trailing newlines..."

while IFS= read -rd '' f; do
	# Skip excluded directories and files
	case "$f" in
	externals/*) continue ;;                   # Third-party code
	.config | .config.* | build/*) continue ;; # Build artifacts
	*.bin | *.elf | *.o | *.a) continue ;;     # Binaries
	esac

	# Skip empty files (e.g., __init__.py markers)
	[ -s "$f" ] || continue

	# Only check text files
	if file --mime-encoding "$f" | grep -qv binary; then
		checked=$((checked + 1))
		tail -c1 <"$f" | read -r _ || show=1
		if [ $show -eq 1 ]; then
			echo "  ✗ Missing newline: $f" >&2
			failed=$((failed + 1))
			ret=1
			show=0
		fi
	fi
done < <(git ls-files -z)

# Summary
echo ""
echo "Newline check summary:"
echo "  Files checked: $checked"
if [ $ret -eq 0 ]; then
	echo "  Result: ✓ PASS - All files end with newline"
else
	echo "  Files failed: $failed"
	echo "  Result: ✗ FAIL - Some files missing trailing newline" >&2
	echo ""
	echo "To fix: Add newline to end of each file listed above" >&2
fi

exit $ret
