"""Check serial CSV against injected frequency; never certify dose."""
import argparse
import csv
import math
from pathlib import Path


def check(text, hz, tolerance):
    if not math.isfinite(hz) or hz <= 0 or not math.isfinite(tolerance) or tolerance < 0:
        raise ValueError("frequency must be positive and tolerance nonnegative")
    rows = list(csv.DictReader(line for line in text.splitlines() if line and not line.startswith("#")))
    if len(rows) < 3:
        raise ValueError("capture at least three complete windows")
    for row in rows[1:]:
        value = float(row["cpm"])
        if not math.isfinite(value) or abs(value - hz * 60) > tolerance:
            raise ValueError(f"count-rate error: {value}; expected {hz * 60} +/- {tolerance}")
    return len(rows) - 1


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path)
    parser.add_argument("--hz", type=float, default=10.0)
    parser.add_argument("--tolerance-cpm", type=float, default=12.0)
    args = parser.parse_args()
    print(f"PASS: {check(args.log.read_text(), args.hz, args.tolerance_cpm)} windows; injected pulses only")
