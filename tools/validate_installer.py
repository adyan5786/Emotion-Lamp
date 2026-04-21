#!/usr/bin/env python3
"""Validate Emotion Lamp web installer manifest and artifact references.

Usage:
  python tools/validate_installer.py
  python tools/validate_installer.py --require-binaries
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path


EXPECTED_PARTS = [
    ("firmware/bootloader.bin", 0x1000),
    ("firmware/partitions.bin", 0x8000),
    ("firmware/firmware.bin", 0x10000),
]


def fail(msg: str) -> int:
    print(f"[installer-check] ERROR: {msg}")
    return 1


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--require-binaries", action="store_true", help="fail if firmware binaries are missing")
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parents[1]
    docs_dir = repo_root / "docs"
    manifest_path = docs_dir / "manifest.json"
    index_path = docs_dir / "index.html"

    if not docs_dir.exists():
        return fail("docs directory does not exist")
    if not index_path.exists():
        return fail("docs/index.html missing")
    if not manifest_path.exists():
        return fail("docs/manifest.json missing")

    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        return fail(f"manifest.json invalid JSON: {exc}")

    if "name" not in manifest or not isinstance(manifest["name"], str):
        return fail("manifest must include string field 'name'")
    if "version" not in manifest or not isinstance(manifest["version"], str):
        return fail("manifest must include string field 'version'")

    builds = manifest.get("builds")
    if not isinstance(builds, list) or not builds:
        return fail("manifest must include non-empty 'builds' array")

    first = builds[0]
    if first.get("chipFamily") != "ESP32":
        return fail("first build chipFamily must be ESP32")

    parts = first.get("parts")
    if not isinstance(parts, list) or len(parts) != 3:
        return fail("first build must include exactly 3 parts")

    for idx, (expected_path, expected_offset) in enumerate(EXPECTED_PARTS):
        part = parts[idx]
        if part.get("path") != expected_path:
            return fail(
                f"part {idx} path mismatch: expected '{expected_path}', got '{part.get('path')}'"
            )
        if part.get("offset") != expected_offset:
            return fail(
                f"part {idx} offset mismatch: expected {expected_offset}, got {part.get('offset')}"
            )

    # Verify index.html points install button at manifest.json
    html = index_path.read_text(encoding="utf-8")
    if 'manifest="manifest.json"' not in html:
        return fail("docs/index.html install button must reference manifest.json")

    if args.require_binaries:
        for rel, _ in EXPECTED_PARTS:
            p = docs_dir / rel
            if not p.exists():
                return fail(f"required firmware artifact missing: docs/{rel}")
            if p.stat().st_size == 0:
                return fail(f"firmware artifact is empty: docs/{rel}")

    print("[installer-check] OK: installer manifest and references are valid")
    if args.require_binaries:
        print("[installer-check] OK: firmware binaries exist and are non-empty")
    else:
        print("[installer-check] INFO: binary existence not enforced in this run")
    return 0


if __name__ == "__main__":
    sys.exit(main())
