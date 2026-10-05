"""STEP-derived bench-cover review; intact factory detector guard is mandatory."""

import hashlib
import json
from pathlib import Path

import cadquery as cq
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
from matplotlib.patches import Circle, Rectangle  # noqa: E402
from mpl_toolkits.mplot3d.art3d import Poly3DCollection  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
MECHANICAL = ROOT / "mechanical"


def digest(path):
    data = path.read_bytes()
    if path.suffix in {".json", ".py"}:
        data = data.replace(b"\r\n", b"\n")
    return hashlib.sha256(data).hexdigest()


def signature(shape):
    b = shape.BoundingBox()
    return [shape.Volume(), b.xmin, b.xmax, b.ymin, b.ymax, b.zmin, b.zmax]


def render():
    manifest_path = ROOT / "hardware/rev-a.json"
    parameter_path = MECHANICAL / "enclosure-parameters.json"
    manifest = json.loads(manifest_path.read_text())
    parameters = json.loads(parameter_path.read_text())
    components = [c for c in manifest["components"] if "position" in c]
    colors = {"U1": "#477cbd", "U2": "#a589bd", "U3": "#dc987d",
              "U4": "#57a899", "U5": "#80bad0", "S1": "#d1b95e"}
    sources = [manifest_path, parameter_path, Path(__file__).resolve()]
    bodies = []
    for name, filename, color, alpha in [
        ("Tray", "bench-tray-draft.step", "#aebecb", .25),
        ("Lid", "bench-lid-draft.step", "#bad6db", .40),
        ("Fixture", "fixture-plate.step", "#aeb7c2", .80),
    ]:
        path = MECHANICAL / filename
        shape = cq.importers.importStep(str(path)).val()
        if len(shape.Solids()) != 1 or not shape.isValid() or shape.Volume() <= 0:
            raise ValueError("Invalid candidate solid: " + filename)
        bodies.append((name, shape, color, alpha))
        sources.append(path)
    for c in components:
        shape = cq.Workplane("XY").box(*c["size"]).translate(tuple(c["position"])).val()
        bodies.append((c["ref"], shape, colors[c["ref"]], 1.0))
    assembly_path = MECHANICAL / "enclosure-layout.step"
    sources.append(assembly_path)
    saved = cq.importers.importStep(str(assembly_path)).val()
    if len(saved.Solids()) != len(bodies):
        raise ValueError("Assembly body count differs from render inputs")
    unmatched = [signature(s) for s in saved.Solids()]
    records = []
    for name, shape, _, _ in bodies:
        sig = signature(shape)
        match = next((i for i, other in enumerate(unmatched)
                      if all(abs(a - b) < 1e-4 for a, b in zip(sig, other))), None)
        if match is None:
            raise ValueError("Assembly volume/bounds mismatch: " + name)
        unmatched.pop(match)
        records.append({"name": name, "bounds_signature": sig})

    lid_bounds = records[1]["bounds_signature"]
    tray_bounds = records[0]["bounds_signature"]
    if abs(lid_bounds[5] - parameters["lid_bottom_z"]) > 1e-4:
        raise ValueError("Lid datum differs from enclosure parameters")
    lid_width, lid_depth = lid_bounds[2] - lid_bounds[1], lid_bounds[4] - lid_bounds[3]
    cover_height = lid_bounds[6] - tray_bounds[5]

    fig = plt.figure(figsize=(16, 9), facecolor="#f7f8fb")
    fig.suptitle("IONTRAIL ONE  /  BENCH-COVER REVIEW", x=.045, ha="left", y=.96,
                 fontsize=21, fontweight="bold", color="#233247")
    fig.text(.045, .905, "DRAFT ONLY  |  Retain the intact factory detector guard  |  No physical validation",
             fontsize=12, color="#a64329")
    ax = fig.add_axes([.035, .30, .43, .53], projection="3d", facecolor="#f7f8fb")
    ax.set_proj_type("ortho")
    display_lid_offset = 35
    for (name, shape, color, alpha), record in zip(bodies, records):
        shown = shape.translate((0, 0, display_lid_offset)) if name == "Lid" else shape
        vertices, triangles = shown.tessellate(.15, .2)
        points = [v.toTuple() for v in vertices]
        faces = [[points[i] for i in tri] for tri in triangles]
        ax.add_collection3d(Poly3DCollection(faces, facecolors=color, alpha=alpha,
                                            edgecolors="none", zsort="average"))
        record["triangles"] = len(triangles)
    ax.set(xlim=(-108, 108), ylim=(-60, 60), zlim=(-12, 90),
           xlabel="X / mm", ylabel="Y / mm", zlabel="Z / mm")
    ax.set_box_aspect((216, 120, 102))
    ax.view_init(elev=29, azim=-60)
    ax.tick_params(labelsize=8)
    ax.set_title("A  /  Exploded review: lid shown +35 mm Z", loc="left", fontsize=12, pad=12)

    plan = fig.add_axes([.53, .37, .42, .39], facecolor="#f7f8fb")
    plan.set_aspect("equal")
    plan.add_patch(Rectangle((lid_bounds[1], lid_bounds[3]), lid_width, lid_depth,
                            fill=False, edgecolor="#547785", linestyle="--", linewidth=1.5))
    w, d, _ = manifest["plate"]
    plan.add_patch(Rectangle((-w / 2, -d / 2), w, d, facecolor="#e4e8ee",
                            edgecolor="#687d93", linewidth=1.4))
    for x, y in manifest["fixture_holes"]:
        plan.add_patch(Circle((x, y), manifest["fixture_hole_d"] / 2,
                              facecolor="white", edgecolor="#687d93"))
    labels = {"U1": "U1\nESP32-C3", "U2": "U2\nLDO", "U4": "U4\nSHT40",
              "U5": "U5\nOLED", "S1": "S1"}
    for c in components:
        x, y, _ = c["position"]
        sx, sy, _ = c["size"]
        plan.add_patch(Rectangle((x - sx / 2, y - sy / 2), sx, sy,
                                facecolor=colors[c["ref"]], edgecolor="#445164"))
        guard_size = " x ".join(f"{value:g}" for value in c["size"])
        label = labels.get(c["ref"], "U3 / factory-guard envelope\n" + guard_size +
                           " mm ASSUMED\nDo not open or clamp the tube")
        plan.text(x, y, label, ha="center", va="center", fontsize=8.5, color="#182638")
    plan.set(xlim=(-109, 109), ylim=(-59, 59), xlabel="X / mm", ylabel="Y / mm")
    plan.set_title("B  /  Envelope placement plan (lid / tray walls omitted)",
                   loc="left", fontsize=12, pad=14)
    plan.tick_params(labelsize=8)
    fig.text(.54, .275, "Dashed: lid footprint, not wall profile. Plate holes are fixture holes only.\n"
             "Boxes are module allowances, not PCB/connector or detector mounting patterns.",
             fontsize=9, color="#42536a", linespacing=1.6)
    fig.text(.05, .205, "WHAT THE DRAFT SHOWS", fontsize=11, weight="bold", color="#233247")
    fig.text(.05, .105, f"{lid_width:.0f} x {lid_depth:.0f} mm lid; "
             f"{cover_height:.1f} mm overall cover height before feet/fasteners.\n"
             "Lid displacement is display-only; assembly STEP remains closed.\n"
             "Three vents over climate bay; left-wall USB opening still provisional.",
             fontsize=10, linespacing=1.6)
    fig.text(.54, .205, "RELEASE GATES STILL OPEN", fontsize=11, weight="bold", color="#a64329")
    fig.text(.54, .105, "S1 cannot be pressed through the closed lid; actuator extension unresolved.\n"
             "Guard dimensions, module retention, strain relief and insulating feet unverified.\n"
             "Secondary cover is NOT a rated electrical guard or a safety-meter enclosure.",
             fontsize=10, linespacing=1.6)
    fig.text(.05, .035, "Visual review only. No fit, touch-access, material or high-voltage approval. "
             "Read mechanical/README.md before any physical work.", fontsize=9, color="#58667a")
    output = MECHANICAL / "enclosure-visual-review.png"
    fig.savefig(output, dpi=150, facecolor=fig.get_facecolor())
    plt.close(fig)
    report = {"physical_validation": False, "assembly_body_match": True,
              "match_basis": "one-to-one volume within 1e-4 mm3 and bounds within 1e-4 mm",
              "rendered_bodies": len(bodies), "parts": records,
              "lid_xy_mm": [lid_width, lid_depth], "closed_cover_height_mm": cover_height,
              "display_only_lid_offset_mm": [0, 0, display_lid_offset],
              "not_proven": ["topological identity", "fit", "retention", "electrical guarding"],
              "generator_versions": {"cadquery": cq.__version__, "matplotlib": matplotlib.__version__},
              "hash_policy": "STEP/PNG native bytes; JSON/Python CRLF normalized to LF",
              "render_sha256": digest(output),
              "sources_sha256": {p.relative_to(ROOT).as_posix(): digest(p) for p in sources}}
    (MECHANICAL / "enclosure-visual-review.json").write_text(json.dumps(report, indent=2) + "\n")
    print(f"Rendered {len(bodies)} matched bodies: {output}")


if __name__ == "__main__":
    render()
