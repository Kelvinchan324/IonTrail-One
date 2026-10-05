"""Reproducible planning arithmetic, never a substitute for measured limits."""

import argparse
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def calculate(data):
    fields = (
        "input_v",
        "aux_v",
        "input_budget_ma",
        "host_allowance_ma",
        "aux_design_limit_ma",
        "aux_margin_factor",
        "ldo_quiescent_max_ma",
        "theta_ja_reference_c_per_w",
        "ambient_scenario_c",
        "junction_review_limit_c",
    )
    for key in fields:
        value = data[key]
        if (
            isinstance(value, bool)
            or not isinstance(value, (int, float))
            or not math.isfinite(value)
        ):
            raise ValueError(f"invalid finite number: {key}")
        if key != "ambient_scenario_c" and value <= 0:
            raise ValueError(f"nonpositive parameter: {key}")
    if not 4.3 <= data["input_v"] <= 6 or data["aux_v"] != 3.3:
        raise ValueError("input outside regulator planning range")
    if data["aux_margin_factor"] < 1:
        raise ValueError("margin must not reduce the load")
    if not data["loads"]:
        raise ValueError("at least one auxiliary load is required")
    refs = set()
    for load in data["loads"]:
        value = load["allowance_ma"]
        if load["ref"] in refs or not load.get("basis"):
            raise ValueError("duplicate load or missing basis")
        refs.add(load["ref"])
        if (
            isinstance(value, bool)
            or not isinstance(value, (int, float))
            or not math.isfinite(value)
            or value < 0
        ):
            raise ValueError("invalid load allowance")
    aux = (
        sum(load["allowance_ma"] for load in data["loads"]) * data["aux_margin_factor"]
    )
    total = data["host_allowance_ma"] + aux + data["ldo_quiescent_max_ma"]
    heat = (data["input_v"] - data["aux_v"]) * aux / 1000 + data["input_v"] * data[
        "ldo_quiescent_max_ma"
    ] / 1000
    junction = data["ambient_scenario_c"] + heat * data["theta_ja_reference_c_per_w"]
    results = [aux, total, heat, junction]
    if not all(math.isfinite(v) for v in results):
        raise ValueError("calculation overflow")
    checks = {
        "aux_within_regulator_rating": aux <= 600,
        "aux_within_design_allowance": aux <= data["aux_design_limit_ma"],
        "input_within_budget": total <= data["input_budget_ma"],
        "reference_junction_below_review_limit": junction
        <= data["junction_review_limit_c"],
    }
    return {
        "physical_validation": False,
        "status": "Planning arithmetic only",
        "aux_with_margin_ma": round(aux, 4),
        "input_allowance_ma": round(total, 4),
        "input_margin_ma": round(data["input_budget_ma"] - total, 4),
        "ldo_dissipation_w": round(heat, 6),
        "reference_junction_c": round(junction, 3),
        "checks": checks,
        "planning_checks_pass": all(checks.values()),
    }


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "path", nargs="?", type=Path, default=ROOT / "hardware/power-budget.json"
    )
    args = parser.parse_args()
    try:
        report = calculate(json.loads(args.path.read_text(encoding="utf-8")))
    except (ValueError, KeyError, TypeError, OSError) as exc:
        parser.exit(2, f"Invalid budget: {exc}\n")
    print(json.dumps(report, indent=2))
    raise SystemExit(0 if report["planning_checks_pass"] else 1)
