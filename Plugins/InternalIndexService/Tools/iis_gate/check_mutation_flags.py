"""Scan IIS source for hard-coded mutation flags set to true."""

from __future__ import annotations

import os
import re

FLAGS = [
    "bAllowsMigrationDecision",
    "bAllowsPatchGeneration",
    "bAllowsProjectMutation",
]
_PAT = re.compile(r"(" + "|".join(FLAGS) + r")\s*=\s*true\b")


def scan_source_for_true_flags(roots: list[str]) -> list[str]:
    violations: list[str] = []
    for root in roots:
        if not os.path.isdir(root):
            continue
        for dirpath, _dirs, files in os.walk(root):
            for fn in files:
                if not fn.endswith((".cpp", ".h")):
                    continue
                path = os.path.join(dirpath, fn)
                with open(path, encoding="utf-8", errors="ignore") as fh:
                    for i, line in enumerate(fh, 1):
                        if _PAT.search(line):
                            violations.append(f"{path}:{i}: {line.strip()}")
    return violations
