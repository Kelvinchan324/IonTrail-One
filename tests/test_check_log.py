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


def climate_capture(temperature="23.5", humidity="45.0", ok="1"):
    rows = list(csv.DictReader(io.StringIO(capture())))
    for row in rows:
        row.update(temperature_c=temperature, humidity_percent=humidity, sensor_ok=ok)
    stream = io.StringIO()
    writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
    writer.writeheader()
    writer.writerows(rows)
    return stream.getvalue()


@pytest.mark.parametrize(
    "temperature,humidity,ok",
    [
        ("23.5", "45", "1"),
        ("0", "0", "1"),
        ("20", "100", "1"),
        ("nan", "nan", "0"),
    ],
)
def test_climate_consistency_accepts_available_or_unavailable(
    temperature, humidity, ok
):
    assert log.check(climate_capture(temperature, humidity, ok), 10, 12) == 2


@pytest.mark.parametrize(
    "temperature,humidity,ok",
    [
        ("nan", "45", "1"),
        ("inf", "45", "1"),
        ("20", "nan", "1"),
        ("20", "-1", "1"),
        ("20", "101", "1"),
        ("", "45", "1"),
        ("20", "45", "0"),
        ("nan", "45", "0"),
        ("inf", "nan", "0"),
        ("20", "45", "true"),
        ("20", "45", "2"),
    ],
)
def test_invalid_climate_claims_rejected(temperature, humidity, ok):
    with pytest.raises(ValueError, match="CSV data row"):
        log.check(climate_capture(temperature, humidity, ok), 10, 12)


def test_partial_climate_schema_rejected():
    text = capture().replace("counts_valid", "counts_valid,temperature_c")
    with pytest.raises(ValueError, match="incomplete climate"):
        log.check(text, 10, 12)


def test_duplicate_columns_rejected():
    text = capture().replace("counts_valid", "counts_valid,cpm")
    with pytest.raises(ValueError, match="duplicate CSV"):
        log.check(text, 10, 12)
