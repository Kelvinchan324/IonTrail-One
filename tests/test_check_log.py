import importlib.util
from pathlib import Path
import pytest

spec = importlib.util.spec_from_file_location("check_log", Path(__file__).parents[1] / "tools/check_log.py")
log = importlib.util.module_from_spec(spec)
spec.loader.exec_module(log)


def test_injected_frequency():
    assert log.check("cpm\n0\n600\n600\n", 10, 12) == 2


@pytest.mark.parametrize("text", ["cpm\n600\n", "cpm\n600\n0\n600\n", "cpm\n600\nnan\n600\n"])
def test_bad_capture_rejected(text):
    with pytest.raises(ValueError):
        log.check(text, 10, 12)
