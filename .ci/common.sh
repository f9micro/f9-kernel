#!/usr/bin/env bash

# Copyright (c) 2026 The F9 Microkernel Project. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Common utilities for CI scripts
# Can be sourced by other scripts or executed directly

# Bash strict mode (enabled only when executed directly, not sourced)
if ! (return 0 2>/dev/null); then
	set -euo pipefail
fi

# Platform detection
# Supported: Linux/x86_64, Linux/aarch64, macOS/arm64, macOS/x86_64

MACHINE_TYPE=$(uname -m)
OS_TYPE=$(uname -s)

check_platform() {
	case "${MACHINE_TYPE}/${OS_TYPE}" in
	x86_64/Linux | aarch64/Linux | arm64/Darwin | x86_64/Darwin) ;;

	*)
		print_error "Unsupported platform: ${MACHINE_TYPE}/${OS_TYPE}"
		print_error "Supported: Linux/x86_64, Linux/aarch64, macOS/arm64, macOS/x86_64"
		exit 1
		;;
	esac
}

if [ "${OS_TYPE}" = "Linux" ]; then
	PARALLEL=-j$(nproc)
else
	PARALLEL=-j$(sysctl -n hw.logicalcpu)
fi

# Color output helpers
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_success() {
	echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
	echo -e "${RED}[ERROR]${NC} $1" >&2
}

print_warning() {
	echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Assertion function for tests
# Usage: ASSERT <condition> <error_message>
ASSERT() {
	local condition=$1
	shift
	local message="$*"

	if ! eval "${condition}"; then
		print_error "Assertion failed: ${message}"
		print_error "Condition: ${condition}"
		return 1
	fi
}

# Cleanup function registry
CLEANUP_FUNCS=()

register_cleanup() {
	CLEANUP_FUNCS+=("$1")
}

cleanup() {
	local func
	for func in "${CLEANUP_FUNCS[@]-}"; do
		[ -n "${func}" ] || continue
		eval "${func}" || true
	done
}

trap cleanup EXIT

# Universal download utility with curl/wget compatibility
# Provides consistent interface regardless of which tool is available
# Security: Enforces HTTPS and supports optional checksum verification

# Detect available download tool (lazy initialization)
detect_download_tool() {
	if [ -n "${DOWNLOAD_TOOL:-}" ]; then
		return 0
	fi

	if command -v curl >/dev/null 2>&1; then
		DOWNLOAD_TOOL="curl"
	elif command -v wget >/dev/null 2>&1; then
		DOWNLOAD_TOOL="wget"
	else
		print_error "Neither curl nor wget is available"
		return 1
	fi
}

# Validate URL uses HTTPS (security requirement)
# Usage: validate_url <url>
# Returns: 0 if valid HTTPS URL, 1 otherwise
validate_url() {
	local url="$1"
	case "$url" in
	https://*)
		return 0
		;;
	http://*)
		print_error "HTTP URLs are not allowed for security reasons: $url"
		print_error "Use HTTPS instead"
		return 1
		;;
	*)
		print_error "Invalid URL scheme (must be HTTPS): $url"
		return 1
		;;
	esac
}

# Verify file checksum
# Usage: verify_checksum <file> <expected_sha256>
# Returns: 0 if checksum matches, 1 otherwise
verify_checksum() {
	local file="$1"
	local expected="$2"

	if [ -z "$expected" ]; then
		return 0 # No checksum provided, skip verification
	fi

	local actual
	if command -v sha256sum >/dev/null 2>&1; then
		actual=$(sha256sum "$file" | cut -d' ' -f1)
	elif command -v shasum >/dev/null 2>&1; then
		actual=$(shasum -a 256 "$file" | cut -d' ' -f1)
	else
		print_warning "No SHA256 tool available, skipping checksum verification"
		return 0
	fi

	if [ "$actual" != "$expected" ]; then
		print_error "Checksum mismatch for $file"
		print_error "Expected: $expected"
		print_error "Actual:   $actual"
		return 1
	fi

	return 0
}

# Download to stdout
# Usage: download_to_stdout <url>
download_to_stdout() {
	detect_download_tool || return 1
	local url="$1"
	validate_url "$url" || return 1

	case "$DOWNLOAD_TOOL" in
	curl)
		curl -fS --retry 5 --retry-delay 2 --retry-max-time 60 -sL "$url"
		;;
	wget)
		wget -qO- "$url"
		;;
	esac
}

# Download to file with optional checksum verification
# Usage: download_to_file <url> <output_file> [expected_sha256]
download_to_file() {
	detect_download_tool || return 1
	local url="$1"
	local output="$2"
	local checksum="${3:-}"

	validate_url "$url" || return 1

	local download_status
	case "$DOWNLOAD_TOOL" in
	curl)
		curl -fS --retry 5 --retry-delay 2 --retry-max-time 60 -sL -o "$output" "$url"
		download_status=$?
		;;
	wget)
		wget -q -O "$output" "$url"
		download_status=$?
		;;
	esac

	if [ $download_status -ne 0 ]; then
		print_error "Download failed: $url"
		return $download_status
	fi

	# Verify checksum if provided
	if [ -n "$checksum" ]; then
		verify_checksum "$output" "$checksum" || {
			rm -f "$output"
			return 1
		}
	fi

	return 0
}

# Download silently (no progress, suitable for CI)
# Usage: download_silent <url>
download_silent() {
	download_to_stdout "$@"
}

# Check if URL is accessible (HTTPS only)
# Usage: check_url <url>
# Returns: 0 if accessible, 1 otherwise
check_url() {
	detect_download_tool || return 1
	local url="$1"
	validate_url "$url" || return 1

	case "$DOWNLOAD_TOOL" in
	curl)
		curl -fS --retry 5 --retry-delay 2 --retry-max-time 60 -sL --head "$url" >/dev/null 2>&1
		;;
	wget)
		wget --spider -q "$url" 2>/dev/null
		;;
	esac
}

# F9 Microkernel specific utilities

# Get project root directory (location of .config file)
get_project_root() {
	local dir="$PWD"
	while [ "$dir" != "/" ]; do
		if [ -f "$dir/.config" ] || [ -f "$dir/Makefile" ]; then
			echo "$dir"
			return 0
		fi
		dir=$(dirname "$dir")
	done
	print_error "Not in F9 Microkernel project directory"
	return 1
}

# Check if running in git repository
is_git_repo() {
	git rev-parse --git-dir >/dev/null 2>&1
}

# Get current git branch name
get_git_branch() {
	git rev-parse --abbrev-ref HEAD 2>/dev/null
}

# Count files matching pattern
count_files() {
	local pattern="$1"
	find . -name "$pattern" -type f | wc -l | tr -d ' '
}
