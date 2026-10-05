import copy
import importlib.util
import json
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location(
    "budget", ROOT / "tools/check_power_budget.py"
)
budget = importlib.util.module_from_spec(spec)
spec.loader.exec_module(budget)
BASE = json.loads((ROOT / "hardware/power-budget.json").read_text())


def test_reference_arithmetic():
    result = budget.calculate(BASE)
    assert result["aux_with_margin_ma"] == 106.25
    assert result["input_allowance_ma"] == 456.33
    assert result["ldo_dissipation_w"] == pytest.approx(0.181025)
    assert result["reference_junction_c"] == pytest.approx(73.309)
    assert result["planning_checks_pass"] and not result["physical_validation"]


@pytest.mark.parametrize(
    "key,value",
    [
        ("host_allowance_ma", 450),
        ("aux_design_limit_ma", 90),
        ("ambient_scenario_c", 80),
    ],
)
def test_over_budget_is_not_approved(key, value):
    data = copy.deepcopy(BASE)
    data[key] = value
    assert not budget.calculate(data)["planning_checks_pass"]


@pytest.mark.parametrize("value", [-1, float("nan"), float("inf"), True, "30"])
def test_invalid_load(value):
    data = copy.deepcopy(BASE)
    data["loads"][0]["allowance_ma"] = value
    with pytest.raises(ValueError):
        budget.calculate(data)


def test_no_hidden_duplicate_loads():
    data = copy.deepcopy(BASE)
    data["loads"].append(data["loads"][0])
    with pytest.raises(ValueError, match="duplicate"):
        budget.calculate(data)


@pytest.mark.parametrize("voltage", [3.4, 6.1])
def test_outside_reference_operating_range(voltage):
    data = copy.deepcopy(BASE)
    data["input_v"] = voltage
    with pytest.raises(ValueError):
        budget.calculate(data)


def test_high_input_scenario_increases_heat():
    data = copy.deepcopy(BASE)
    data["input_v"] = 5.25
    result = budget.calculate(data)
    assert result["ldo_dissipation_w"] == pytest.approx(0.207608)
    assert result["reference_junction_c"] == pytest.approx(78.2)
