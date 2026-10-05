"""Check serial CSV against injected frequency; never certify dose."""

import argparse
import csv
import math
from pathlib import Path


def check(text, hz, tolerance):
    if (
        not math.isfinite(hz)
        or hz <= 0
        or not math.isfinite(tolerance)
        or tolerance < 0
    ):
        raise ValueError("frequency must be positive and tolerance nonnegative")
    required = {
        "uptime_ms",
        "window_ms",
        "window_counts",
        "cpm",
        "total_counts",
        "counts_valid",
    }
    reader = csv.DictReader(
        line for line in text.splitlines() if line and not line.startswith("#")
    )
    if not required.issubset(reader.fieldnames or []):
        raise ValueError("missing counter columns; use the current firmware CSV header")
    rows = list(reader)
    if len(rows) < 3:
        raise ValueError("capture at least three complete windows")
    previous = None
    for index, row in enumerate(rows, start=1):
        try:
            if None in row or any(row.get(key) in (None, "") for key in required):
                raise ValueError("malformed CSV row")
            integers = {key: int(row[key]) for key in required - {"cpm"}}
            if any(not 0 <= value <= 0xFFFFFFFF for value in integers.values()):
                raise ValueError("counter field outside uint32 range")
            if integers["counts_valid"] != 1:
                raise ValueError("firmware flagged invalid/overflowed counts")
            window = integers["window_ms"]
            if not 0 < window < 0x80000000:
                raise ValueError("invalid elapsed window")
            value = float(row["cpm"])
            calculated = integers["window_counts"] * 60000 / window
            if not math.isfinite(value) or abs(value - calculated) > 0.015:
                raise ValueError("CPM does not match counts and elapsed time")
            if integers["total_counts"] < integers["window_counts"]:
                raise ValueError("total smaller than window count")
            if previous is not None:
                elapsed = (integers["uptime_ms"] - previous["uptime_ms"]) & 0xFFFFFFFF
                if abs(elapsed - window) > 100:
                    raise ValueError("missing/duplicated window or inconsistent uptime")
                if (
                    integers["total_counts"] - previous["total_counts"]
                    != integers["window_counts"]
                ):
                    raise ValueError("total-count discontinuity; recapture after reset")
                if abs(value - hz * 60) > tolerance:
                    raise ValueError(
                        f"count-rate error: {value}; expected {hz * 60} +/- {tolerance}"
                    )
            previous = integers
        except (ValueError, TypeError) as exc:
            raise ValueError(f"CSV data row {index}: {exc}") from exc
    return len(rows) - 1


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path)
    parser.add_argument("--hz", type=float, default=10.0)
    parser.add_argument("--tolerance-cpm", type=float, default=12.0)
    args = parser.parse_args()
    print(
        f"PASS: {check(args.log.read_text(), args.hz, args.tolerance_cpm)} windows; injected pulses only"
    )
