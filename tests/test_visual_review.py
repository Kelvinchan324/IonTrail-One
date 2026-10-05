"""Check committed visual review freshness without installing a CAD runtime."""

import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def test_enclosure_review_sources_are_current():
    report = json.loads((ROOT / "mechanical/enclosure-visual-review.json").read_text())
    assert report["physical_validation"] is False
    assert report["assembly_body_match"] is True
    assert report["rendered_bodies"] == len(report["parts"]) == 9
    assert len({p["name"] for p in report["parts"]}) == 9
    assert report["display_only_lid_offset_mm"] == [0, 0, 35]
    expected = {
        "hardware/rev-a.json", "mechanical/enclosure-parameters.json",
        "mechanical/bench-tray-draft.step", "mechanical/bench-lid-draft.step",
        "mechanical/fixture-plate.step", "mechanical/enclosure-layout.step",
        "tools/render_enclosure_review.py",
    }
    assert set(report["sources_sha256"]) == expected
    for relative, digest in report["sources_sha256"].items():
        data = (ROOT / relative).read_bytes()
        if Path(relative).suffix in {".json", ".py"}:
            data = data.replace(b"\r\n", b"\n")
        assert hashlib.sha256(data).hexdigest() == digest, (
            f"Stale visual review: {relative}; rerender and visually inspect"
        )


def test_enclosure_image_matches_ledger():
    report = json.loads((ROOT / "mechanical/enclosure-visual-review.json").read_text())
    data = (ROOT / "mechanical/enclosure-visual-review.png").read_bytes()
    assert hashlib.sha256(data).hexdigest() == report["render_sha256"]
    assert data[:8] == b"\x89PNG\r\n\x1a\n"
    assert int.from_bytes(data[16:20], "big") == 2400
    assert int.from_bytes(data[20:24], "big") == 1350
