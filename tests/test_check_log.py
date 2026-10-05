import csv
import importlib.util
import io
from pathlib import Path

import pytest

spec = importlib.util.spec_from_file_location(
    "check_log", Path(__file__).parents[1] / "tools/check_log.py"
)
log = importlib.util.module_from_spec(spec)
spec.loader.exec_module(log)


def capture(changes=None):
    rows = [
        dict(
            uptime_ms=10000,
            window_ms=10000,
            window_counts=100,
            cpm=600,
            total_counts=100,
            counts_valid=1,
        ),
        dict(
            uptime_ms=20000,
            window_ms=10000,
            window_counts=100,
            cpm=600,
            total_counts=200,
            counts_valid=1,
        ),
        dict(
            uptime_ms=30000,
            window_ms=10000,
            window_counts=100,
            cpm=600,
            total_counts=300,
            counts_valid=1,
        ),
    ]
    if changes:
        for index, values in changes:
            rows[index].update(values)
    stream = io.StringIO()
    writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
    writer.writeheader()
    writer.writerows(rows)
    return stream.getvalue()


def test_injected_frequency():
    assert log.check(capture(), 10, 12) == 2


def test_uptime_rollover():
    text = capture(
        [
            (0, {"uptime_ms": 0xFFFFFFFF - 4999}),
            (1, {"uptime_ms": 5000}),
            (2, {"uptime_ms": 15000}),
        ]
    )
    assert log.check(text, 10, 12) == 2


@pytest.mark.parametrize(
    "change",
    [
        {"cpm": "nan"},
        {"cpm": 599},
        {"counts_valid": 0},
        {"window_ms": 0},
        {"window_ms": -1},
        {"window_counts": 99},
        {"total_counts": 100},
        {"total_counts": 0x100000000},
        {"uptime_ms": 30000},
        {"window_ms": "bad"},
        {"cpm": ""},
        {"window_counts": "100.0"},
    ],
)
def test_inconsistent_capture_rejected(change):
    with pytest.raises(ValueError):
        log.check(capture([(1, change)]), 10, 12)


@pytest.mark.parametrize("hz,tolerance", [(0, 12), (float("nan"), 12), (10, -1)])
def test_invalid_reference_rejected(hz, tolerance):
    with pytest.raises(ValueError):
        log.check(capture(), hz, tolerance)


def test_correct_math_wrong_frequency_rejected():
    with pytest.raises(ValueError, match="count-rate error"):
        log.check(capture(), 20, 12)


@pytest.mark.parametrize("text", ["cpm\n600\n", "", capture().splitlines()[0]])
def test_bad_capture_rejected(text):
    with pytest.raises(ValueError):
        log.check(text, 10, 12)
