"""Editable secondary bench cover; original SEN0463 guard must remain intact."""

import json
import math
from pathlib import Path

import cadquery as cq

ROOT = Path(__file__).resolve().parents[1]


def box(size, centre):
    return cq.Workplane("XY").box(*size).translate(tuple(centre))


def build():
    manifest = json.loads((ROOT / "hardware/rev-a.json").read_text())
    p = json.loads((ROOT / "mechanical/enclosure-parameters.json").read_text())
    for key, value in p.items():
        if key in {"status", "unresolved"}:
            continue
        values = value if isinstance(value, list) else [value]
        if not values or not all(
            type(v) in (int, float) and math.isfinite(v) and v > 0 for v in values
        ):
            raise ValueError("invalid positive finite parameter: " + key)
    w, d, plate_t = manifest["plate"]
    clear, wall, gap = p["plate_edge_clearance"], p["wall"], p["under_plate_gap"]
    top, floor, lid_t = p["lid_bottom_z"], p["floor"], p["lid_thickness"]
    if not (
        0.5 <= clear <= 3
        and 2 <= wall <= 5
        and 3 <= gap <= 15
        and 2 <= floor <= 6
        and 2 <= lid_t <= 6
        and 35 <= top <= 60
    ):
        raise ValueError("parameters outside this bench-cover design range")
    iw, idepth = w + 2 * clear, d + 2 * clear
    if not (
        manifest["fixture_hole_d"] + 4 <= p["fixture_boss_diameter"] <= 12
        and 2 <= p["lid_fastener_diameter"] <= 4
        and p["lid_ear_radius"] >= p["lid_fastener_diameter"] / 2 + 3
        and 4 <= p["lid_ear_depth"] <= 12
    ):
        raise ValueError("invalid support or lid-fastener dimensions")
    ow, od = iw + 2 * wall, idepth + 2 * wall
    bottom = -gap - floor
    tray = box((ow, od, top - bottom), (0, 0, (top + bottom) / 2))
    tray = tray.cut(box((iw, idepth, top + gap + 2), (0, 0, (top - gap) / 2 + 1)))
    # Four plate support bosses, with holes matching the existing fixture only.
    for x, y in manifest["fixture_holes"]:
        boss = (
            cq.Workplane("XY")
            .center(x, y)
            .circle(p["fixture_boss_diameter"] / 2)
            .extrude(gap)
            .translate((0, 0, -gap))
        )
        tray = tray.union(boss)
        hole = (
            cq.Workplane("XY")
            .center(x, y)
            .circle(manifest["fixture_hole_d"] / 2)
            .extrude(gap + floor + 2)
            .translate((0, 0, bottom - 1))
        )
        tray = tray.cut(hole)
    # External lid ears keep lid hardware outside the detector volume.
    fx, fy, radius = p["lid_fastener_x"], p["lid_fastener_y"], p["lid_ear_radius"]
    if fx - radius <= ow / 2 or fy + radius >= od / 2:
        raise ValueError("lid ears must stay outside walls with longitudinal clearance")
    for x in (-fx, fx):
        for y in (-fy, fy):
            bridge_width = fx - iw / 2
            bridge = box(
                (bridge_width + 1, radius * 2, p["lid_ear_depth"]),
                (
                    (1 if x > 0 else -1) * (fx + iw / 2) / 2,
                    y,
                    top - p["lid_ear_depth"] / 2,
                ),
            )
            ear = (
                cq.Workplane("XY")
                .center(x, y)
                .circle(radius)
                .extrude(p["lid_ear_depth"])
                .translate((0, 0, top - p["lid_ear_depth"]))
            )
            tray = tray.union(bridge).union(ear)
            hole = (
                cq.Workplane("XY")
                .center(x, y)
                .circle(p["lid_fastener_diameter"] / 2)
                .extrude(p["lid_ear_depth"] + 2)
                .translate((0, 0, top - p["lid_ear_depth"] - 1))
            )
            tray = tray.cut(hole)
    uy, uz = p["usb_opening_yz"]
    sy, sz = p["usb_opening_size_yz"]
    if (
        uy - sy / 2 <= 5
        or uy + sy / 2 >= idepth / 2 - 3
        or uz - sz / 2 <= plate_t
        or uz + sz / 2 >= top - 3
    ):
        raise ValueError("USB opening must stay in upper low-voltage bay")
    tray = tray.cut(box((wall + 2, sy, sz), (-iw / 2 - wall / 2, uy, uz)))
    lid = box((2 * (fx + radius), od, lid_t), (0, 0, top + lid_t / 2))
    for x in (-fx, fx):
        for y in (-fy, fy):
            lid = lid.cut(
                cq.Workplane("XY")
                .center(x, y)
                .circle(p["lid_fastener_diameter"] / 2)
                .extrude(lid_t + 2)
                .translate((0, 0, top - 1))
            )
    for x in p["sensor_vent_x"]:
        if (
            x - p["sensor_vent_size_xy"][0] / 2 < 50
            or x + p["sensor_vent_size_xy"][0] / 2 > iw / 2 - 3
            or p["sensor_vent_y"] - p["sensor_vent_size_xy"][1] / 2 < 10
            or p["sensor_vent_y"] + p["sensor_vent_size_xy"][1] / 2 > idepth / 2 - 3
        ):
            raise ValueError("vents must remain in sensor bay")
        lid = lid.cut(
            box(
                (*p["sensor_vent_size_xy"], lid_t + 2),
                (x, p["sensor_vent_y"], top + lid_t / 2),
            )
        )
    for name, part in (("bench-tray-draft", tray), ("bench-lid-draft", lid)):
        if (
            len(part.solids().vals()) != 1
            or not part.val().isValid()
            or part.val().Volume() <= 0
        ):
            raise ValueError("invalid connected solid: " + name)
        for c in manifest["components"]:
            if "position" not in c:
                continue
            overlap = part.intersect(box(c["size"], c["position"])).val()
            if overlap is not None and overlap.Volume() > 1e-6:
                raise ValueError(name + " intrudes into " + c["ref"])
        overlap = part.intersect(box((w, d, plate_t), (0, 0, plate_t / 2))).val()
        if overlap is not None and overlap.Volume() > 1e-6:
            raise ValueError("enclosure intrudes into fixture plate")
        for ext in ("step", "stl"):
            cq.exporters.export(part, str(ROOT / "mechanical" / (name + "." + ext)))
        recovered = cq.importers.importStep(str(ROOT / "mechanical" / (name + ".step")))
        if len(recovered.solids().vals()) != 1 or not recovered.val().isValid():
            raise ValueError("STEP roundtrip failed")
    assembly = cq.Assembly(name="IonTrail_secondary_bench_cover")
    assembly.add(tray, name="tray", color=cq.Color(0.3, 0.45, 0.55))
    assembly.add(lid, name="lid_candidate", color=cq.Color(0.75, 0.85, 0.9, 0.3))
    assembly.add(
        cq.importers.importStep(str(ROOT / "mechanical/fixture-plate.step")),
        name="fixture",
    )
    for c in manifest["components"]:
        if "position" in c:
            assembly.add(box(c["size"], c["position"]), name=c["ref"] + "_ENVELOPE")
    assembly.save(str(ROOT / "mechanical/enclosure-layout.step"))
    recovered = cq.importers.importStep(str(ROOT / "mechanical/enclosure-layout.step"))
    if len(recovered.solids().vals()) != 3 + sum(
        "position" in c for c in manifest["components"]
    ):
        raise ValueError("assembly STEP body count changed")
    report = {
        "physical_validation": False,
        "status": p["status"],
        "outer_lid_xy_mm": [2 * (fx + radius), od],
        "bottom_z_mm": bottom,
        "lid_top_z_mm": top + lid_t,
        "fixture_plate_z_mm": [0, plate_t],
        "tray_volume_mm3": round(tray.val().Volume(), 3),
        "lid_volume_mm3": round(lid.val().Volume(), 3),
        "unresolved": p["unresolved"],
    }
    (ROOT / "mechanical/enclosure-checks.json").write_text(
        json.dumps(report, indent=2) + "\n"
    )
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    build()
