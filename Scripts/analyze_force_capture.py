#!/usr/bin/env python3
"""
Analyze force capture CSV files from NIO Racing Plus Iter 0 diagnostics.
Compares Default vs A/B/C schemes across StraightAccel/MediumCorner/Hairpin scenarios.

Usage:
    python analyze_force_capture.py <path_to_csv_directory>
"""

import sys
import os
import csv
import glob
import math
from collections import defaultdict

# Scheme enum mapping (must match ENIOTireForceScheme)
SCHEME_NAMES = {
    0: "A: FrictionHack",
    1: "B: NoInternalPhysics",
    2: "C: Both",
    3: "Default",
}


def parse_csv(filepath):
    """Parse a force capture CSV file into a list of dicts."""
    rows = []
    with open(filepath, "r") as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append(row)
    return rows


def compute_metrics(rows):
    """Compute key metrics from a capture session."""
    if not rows:
        return {}

    # Extract numeric fields
    speeds = [float(r["SpeedCmS"]) * 0.036 for r in rows]  # to km/h
    accels = [float(r["AccelCmS2"]) for r in rows]
    total_long = [float(r["TotalPacejkaLong"]) for r in rows]

    # 0-100 km/h acceleration time
    accel_time = None
    for i, s in enumerate(speeds):
        if s >= 100 and accel_time is None:
            accel_time = i / 60.0  # assuming 60fps capture
            break

    # Peak lateral force (as proxy for cornering grip)
    avg_lat = sum(abs(float(r["W1_PacejkaLat"])) + abs(float(r["W3_PacejkaLat"])) for r in rows) / max(len(rows), 1)

    # Force smoothness: coefficient of variation of total force
    if total_long:
        mean_f = sum(abs(f) for f in total_long) / len(total_long)
        var_f = sum((abs(f) - mean_f) ** 2 for f in total_long) / len(total_long)
        smoothness = math.sqrt(var_f) / max(mean_f, 0.01)
    else:
        smoothness = 0

    # Peak speed
    max_speed = max(speeds) if speeds else 0

    return {
        "accel_0_100_s": accel_time,
        "avg_lateral_force": avg_lat,
        "force_smoothness_cv": smoothness,
        "max_speed_kmh": max_speed,
        "num_frames": len(rows),
    }


def analyze_directory(csv_dir):
    """Analyze all CSV files in the given directory."""
    files = glob.glob(os.path.join(csv_dir, "ForceCapture_*.csv"))
    if not files:
        print(f"ERROR: No ForceCapture_*.csv files found in '{csv_dir}'")
        sys.exit(1)

    results = defaultdict(dict)

    for filepath in sorted(files):
        filename = os.path.basename(filepath)
        parts = filename.replace("ForceCapture_", "").split("_")

        # Parse scenario and scheme from filename
        # Format: ForceCapture_{Scenario}_{Scheme}_{Timestamp}.csv
        scenario = parts[0] if len(parts) >= 1 else "Unknown"
        scheme_str = parts[1] if len(parts) >= 2 else "Unknown"

        rows = parse_csv(filepath)
        metrics = compute_metrics(rows)

        # Detect scheme from data if possible
        if rows:
            scheme_val = int(rows[0].get("Scheme", 3))
        else:
            scheme_val = 3

        scheme_name = SCHEME_NAMES.get(scheme_val, f"Scheme_{scheme_val}")

        metrics["file"] = filename
        results[scenario][scheme_name] = metrics

    return results


def print_results(results):
    """Print formatted comparison table."""
    print("=" * 90)
    print("NIO Racing Plus — Iter 0 Force Diagnostic Results")
    print("=" * 90)

    for scenario in sorted(results.keys()):
        print(f"\n{'─' * 90}")
        print(f"Scenario: {scenario}")
        print(f"{'─' * 90}")
        print(f"{'Scheme':<25} {'0-100(s)':<12} {'Avg Lat Force':<16} {'Smoothness(CV)':<16} {'Max Speed':<12}")
        print(f"{'─' * 90}")

        for scheme in ["Default", "A: FrictionHack", "B: NoInternalPhysics", "C: Both"]:
            if scheme in results[scenario]:
                m = results[scenario][scheme]
                accel = f"{m['accel_0_100_s']:.2f}" if m['accel_0_100_s'] else "N/A"
                print(f"{scheme:<25} {accel:<12} {m['avg_lateral_force']:<16.1f} {m['force_smoothness_cv']:<16.4f} {m['max_speed_kmh']:<12.1f}")
            else:
                print(f"{scheme:<25} {'--':<12} {'--':<16} {'--':<16} {'--':<12}")

    print(f"\n{'─' * 90}")
    print("Recommendation Summary")
    print(f"{'─' * 90}")

    # Find best scheme across scenarios
    scheme_scores = defaultdict(list)
    for scenario, schemes in results.items():
        for scheme, metrics in schemes.items():
            if metrics["force_smoothness_cv"] > 0:
                scheme_scores[scheme].append(metrics["force_smoothness_cv"])

    for scheme, scores in sorted(scheme_scores.items()):
        avg = sum(scores) / len(scores)
        print(f"  {scheme:<25} avg smoothness: {avg:.4f}")

    print("\nNOTE: Lower smoothness(CV) = cleaner force application")
    print("     Optimal scheme has: lowest smoothness + highest lat force + stable behavior")
    print("=" * 90)


def main():
    if len(sys.argv) < 2:
        print("Usage: python analyze_force_capture.py <path_to_csv_directory>")
        sys.exit(1)

    csv_dir = sys.argv[1]
    if not os.path.isdir(csv_dir):
        print(f"ERROR: '{csv_dir}' is not a valid directory")
        sys.exit(1)

    results = analyze_directory(csv_dir)
    print_results(results)


if __name__ == "__main__":
    main()
