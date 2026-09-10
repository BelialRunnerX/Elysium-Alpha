#!/usr/bin/env python3
# Intended function: imported tools implementation for acceptance_report; preserves the agent-authored subsystem contract for later integration/debugging.
"""Run registered CTest proofs and render the Agent-42 milestone gate matrix.

The report intentionally distinguishes native/runtime gaps from headless proof.
A gate can never be promoted beyond the status declared in acceptance_matrix.json
merely because a broad headless executable returned zero.
"""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET
from pathlib import Path

VALID_STATUSES = {"PASS", "HEADLESS ONLY", "NATIVE PENDING"}


def run_ctest(build_dir: Path) -> tuple[dict[str, bool], str]:
    with tempfile.TemporaryDirectory(prefix="elysium_acceptance_") as td:
        junit = Path(td) / "ctest.xml"
        proc = subprocess.run(
            ["ctest", "--test-dir", str(build_dir), "--output-on-failure", "--output-junit", str(junit)],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            check=False,
        )
        results: dict[str, bool] = {}
        if junit.exists():
            root = ET.parse(junit).getroot()
            for case in root.iter("testcase"):
                name = case.attrib.get("name", "")
                failed = case.find("failure") is not None or case.find("error") is not None
                results[name] = not failed
        return results, proc.stdout


def compute_gate_status(gate: dict, tests: dict[str, bool]) -> tuple[str, list[str]]:
    declared = gate["status_when_tests_pass"]
    if declared not in VALID_STATUSES:
        raise ValueError(f"{gate['id']}: invalid status {declared!r}")

    missing = [name for name in gate.get("proof_tests", []) if name not in tests]
    failed = [name for name in gate.get("proof_tests", []) if name in tests and not tests[name]]
    problems = []
    if missing:
        problems.append("missing proof tests: " + ", ".join(missing))
    if failed:
        problems.append("failed proof tests: " + ", ".join(failed))

    # Keep the requested three-state report vocabulary. A broken/missing proof
    # cannot be reported PASS/HEADLESS ONLY, so demote it to NATIVE PENDING and
    # print the concrete problem rather than inventing a fourth ambiguous state.
    if problems:
        return "NATIVE PENDING", problems
    return declared, problems


def render(matrix: dict, tests: dict[str, bool], ctest_output: str, build_dir: Path) -> str:
    lines: list[str] = []
    lines.append("# Elysium Agent 42 - Acceptance Gate Report")
    lines.append("")
    lines.append(f"Build directory: `{build_dir}`")
    lines.append("")
    passed = sum(1 for ok in tests.values() if ok)
    lines.append(f"Registered CTest proof: **{passed}/{len(tests)} passed**.")
    lines.append("")
    lines.append("| Gate | Status | Automated proof | Scope note |")
    lines.append("|---|---|---|---|")
    for gate in matrix["gates"]:
        status, problems = compute_gate_status(gate, tests)
        proofs = gate.get("proof_tests", [])
        proof_text = ", ".join(f"`{p}`" for p in proofs) if proofs else "none in this checkpoint"
        note = gate["notes"]
        if problems:
            note += " **Proof issue:** " + "; ".join(problems)
        note = note.replace("|", "\\|")
        lines.append(f"| {gate['id']} - {gate['name']} | **{status}** | {proof_text} | {note} |")

    lines.extend([
        "",
        "## Authoritative fixture status",
        "",
        "The exact standalone **0/1/2/4/8 worker** mesh/reconstruction determinism regression is now registered as its own CTest entry and is locally executed. The coworker integration contract still owns the stronger address-built navigation wall fixture with **25 open / 43 detour / 0 through-wall**; this standalone tree has only a seam/local-obstacle route test, so the report does not relabel the 25/43/0 fixture as locally proven.",
        "",
        "## Raw CTest output",
        "",
        "```text",
        ctest_output.rstrip(),
        "```",
        "",
    ])
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--build", required=True, type=Path, help="Configured CMake build directory")
    parser.add_argument("--matrix", type=Path, default=Path(__file__).resolve().parents[1] / "tests" / "acceptance_matrix.json")
    parser.add_argument("--output", type=Path, help="Write Markdown report to this path; stdout if omitted")
    args = parser.parse_args()

    matrix = json.loads(args.matrix.read_text())
    if matrix.get("schema") != 1:
        raise SystemExit("Unsupported acceptance matrix schema")
    if set(matrix.get("status_vocabulary", [])) != VALID_STATUSES:
        raise SystemExit("Acceptance status vocabulary does not match Agent-42 contract")

    tests, ctest_output = run_ctest(args.build)
    report = render(matrix, tests, ctest_output, args.build)
    if args.output:
        args.output.write_text(report)
    else:
        print(report)

    # The tool itself fails only when an actually registered proof test fails.
    # Gates intentionally marked NATIVE PENDING remain an honest report state,
    # not a CI failure for features that do not exist yet.
    return 0 if tests and all(tests.values()) else 1


if __name__ == "__main__":
    sys.exit(main())
