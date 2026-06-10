#!/usr/bin/env python3
"""
Generate synthetic force capture CSV data for Iter 0 diagnosis simulation.
Produces realistic data for all 4 schemes across 3 scenarios.

Usage: python3 generate_sample_data.py <output_dir>
"""

import sys
import os
import math
import random
import csv

random.seed(42)

# Scheme mapping
SCHEME_NAMES = {0: "FrictionHack", 1: "NoInternalPhysics", 2: "Both", 3: "Default"}

# Vehicle parameters (EP9)
MASS_KG = 1800.0
PEAK_TORQUE_NM = 1480.0
FRONTAL_AREA = 2.0
CD = 0.30
CL = 3.5

# Tire parameters (semi-slick)
BASEFRICTION = 1.1
TIRE_RADIUS_M = 0.33


def pacejka_force(B, C, D, E, slip):
    """Simplified Pacejka Magic Formula for synthetic data generation."""
    Bx = B * slip
    inner = Bx - E * (Bx - math.atan(Bx))
    return D * math.sin(C * math.atan(inner))


def simulate_straight_accel(scheme, noise=0.02):
    """
    Simulate 0-100 km/h straight-line acceleration.
    Returns list of (frame, scheme, speed_cms, accel_cms2, w0..w3 data)
    """
    rows = []
    speed_ms = 1.0  # start at 1 m/s
    dt = 1.0 / 60.0

    # Scheme affects effective grip and force doubling
    if scheme == 3:  # Default: both active
        force_mult = 1.95  # ~2x force from double system
        grip_quality = 0.85  # inconsistent grip
    elif scheme == 0:  # A: FrictionHack
        force_mult = 0.99  # ~99% Pacejka
        grip_quality = 0.93  # slight residual Chaos
    elif scheme == 1:  # B: NoInternalPhysics
        force_mult = 1.0  # 100% Pacejka
        grip_quality = 1.0  # clean
    else:  # C: Both
        force_mult = 1.0
        grip_quality = 1.0

    for frame in range(600):  # 10 seconds at 60fps
        # Longitudinal force from Pacejka (simplified: slip ratio increases with speed)
        slip_ratio = 0.05 + (speed_ms / 100.0) * 0.1  # increases slightly with speed
        slip_ratio = min(slip_ratio, 0.15)

        # Pacejka longitudinal force
        long_force_pacejka = pacejka_force(12.0, 1.65, 1.0, -0.1, slip_ratio * 100.0)
        long_force_pacejka *= BASEFRICTION * MASS_KG * 9.8

        # Apply scheme multiplier
        total_force = long_force_pacejka * force_mult * grip_quality

        # Add noise
        total_force *= (1.0 + random.gauss(0, noise))

        # Physics: F = ma + drag + rolling resistance
        drag = 0.5 * 1.225 * speed_ms * speed_ms * CD * FRONTAL_AREA
        rolling_resist = 0.015 * MASS_KG * 9.8
        net_force = total_force - drag - rolling_resist

        accel = net_force / MASS_KG
        speed_ms += accel * dt
        if speed_ms < 0:
            speed_ms = 0

        speed_cms = speed_ms * 100.0
        accel_cms2 = accel * 100.0

        # Per-wheel data
        wheel_data = []
        for w in range(4):
            is_grd = 1 if w < 2 or speed_ms > 2.0 else 0  # rear lifts slightly
            sr = slip_ratio * (1.0 + 0.1 * (w % 2))
            sa = random.gauss(0, 0.5)  # near zero on straight
            load = MASS_KG * 9.8 / 4.0
            # Weight transfer
            long_transfer = (MASS_KG * accel * 0.5) / 2.8 * 0.5
            if w < 2:
                load += long_transfer * (1 if w == 0 else -1) * 0.1
            else:
                load -= long_transfer * (1 if w == 2 else -1) * 0.1

            p_long = long_force_pacejka / MASS_KG * load / (MASS_KG * 9.8 / 4.0) * (0.5 if w >= 2 else 1.0)
            p_lat = random.gauss(0, 50.0)  # minimal lateral on straight

            wheel_data.extend([is_grd, sr, sa, load, p_long * 100.0, p_lat * 100.0])

        row = [frame, scheme, speed_cms, accel_cms2] + wheel_data

        # Fill pad for missing wheels
        while len(wheel_data) < 24:
            wheel_data.extend([0, 0, 0, 0, 0, 0])

        row = [frame, scheme, speed_cms, accel_cms2] + wheel_data[:24]
        row += [sum(wheel_data[i*6+4] for i in range(min(4, len(wheel_data)//6))),
                sum(wheel_data[i*6+5] for i in range(min(4, len(wheel_data)//6)))]

        rows.append(row)

        if speed_ms * 3.6 >= 100:
            break

    return rows


def simulate_medium_corner(scheme, noise=0.03):
    """
    Simulate medium-speed corner (60-80 km/h).
    """
    rows = []
    speed_ms = 19.44  # 70 km/h
    dt = 1.0 / 60.0
    steer_angle = 15.0  # degrees

    if scheme == 3:
        force_mult = 1.9
        grip_quality = 0.82
        lat_noise = 0.08
    elif scheme == 0:
        force_mult = 0.98
        grip_quality = 0.91
        lat_noise = 0.04
    elif scheme == 1:
        force_mult = 1.0
        grip_quality = 1.0
        lat_noise = 0.02
    else:
        force_mult = 1.0
        grip_quality = 1.0
        lat_noise = 0.02

    for frame in range(300):
        slip_angle = steer_angle * (speed_ms / 30.0) * 0.1
        slip_angle = min(slip_angle, 12.0)

        lat_force_pacejka = pacejka_force(10.0, 1.55, 1.0, -0.3, slip_angle)
        lat_force_pacejka *= BASEFRICTION * MASS_KG * 9.8 * force_mult * grip_quality

        lat_force_pacejka *= (1.0 + random.gauss(0, lat_noise))

        centripetal_accel = lat_force_pacejka / MASS_KG
        lateral_g = centripetal_accel / 9.8

        speed_cms = speed_ms * 100.0
        accel_cms2 = random.gauss(0, 50)  # minor accel/brake adjustments

        wheel_data = []
        for w in range(4):
            is_grd = 1
            sr = random.gauss(0, 0.02)
            sa = slip_angle * (1.0 + 0.2 * (-1 if w in [1, 3] else 1))
            load = MASS_KG * 9.8 / 4.0
            # Lateral weight transfer
            lat_transfer = (MASS_KG * centripetal_accel * 0.5) / 1.5 * 0.5
            if w in [0, 2]:  # left side
                load -= lat_transfer * 0.5
            else:
                load += lat_transfer * 0.5

            p_long = random.gauss(0, 100) * force_mult * grip_quality
            p_lat = lat_force_pacejka / 4.0 * (0.7 if w >= 2 else 1.3)

            if w in [1, 3]:
                p_lat *= -1

            wheel_data.extend([is_grd, sr, sa, max(load, 100), p_long * 100.0, p_lat * 100.0])

        row = [frame, scheme, speed_cms, accel_cms2] + wheel_data[:24]
        row += [sum(wheel_data[i*6+4] for i in range(4)),
                sum(abs(wheel_data[i*6+5]) for i in range(4))]

        rows.append(row)

        speed_ms += random.gauss(0, 0.1) * dt * 10

    return rows


def simulate_hairpin(scheme, noise=0.04):
    """
    Simulate low-speed hairpin corner (20-40 km/h).
    """
    rows = []
    speed_ms = 8.33  # 30 km/h
    dt = 1.0 / 60.0

    if scheme == 3:
        force_mult = 1.85
        grip_quality = 0.78
        spin_risk = 0.12
    elif scheme == 0:
        force_mult = 0.97
        grip_quality = 0.88
        spin_risk = 0.05
    elif scheme == 1:
        force_mult = 1.0
        grip_quality = 1.0
        spin_risk = 0.01
    else:
        force_mult = 1.0
        grip_quality = 1.0
        spin_risk = 0.01

    for frame in range(300):
        steer_angle = 25.0 + 5.0 * math.sin(frame * 0.05)
        slip_angle = steer_angle * (speed_ms / 15.0) * 0.15
        slip_angle = min(slip_angle, 20.0)

        lat_force = pacejka_force(10.0, 1.55, 1.0, -0.3, slip_angle)
        lat_force *= BASEFRICTION * MASS_KG * 9.8 * force_mult * grip_quality

        # Add occasional spin-out risk for Default scheme
        if random.random() < spin_risk * 0.1:
            lat_force *= 0.3  # sudden grip loss

        lat_force *= (1.0 + random.gauss(0, noise))

        speed_cms = speed_ms * 100.0
        accel_cms2 = random.gauss(0, 100)

        wheel_data = []
        for w in range(4):
            is_grd = 1
            sr = random.gauss(0, 0.05)
            sa = slip_angle * (1.0 + 0.3 * (-1 if w in [1, 3] else 1))
            load = MASS_KG * 9.8 / 4.0
            # Sharp lateral + longitudinal weight transfer
            centripetal = lat_force / MASS_KG
            lat_transfer = (MASS_KG * centripetal * 0.5) / 1.5 * 0.5
            long_transfer = random.gauss(0, 200)  # brake/throttle transitions
            if w in [0, 2]:
                load -= lat_transfer * 0.5
            else:
                load += lat_transfer * 0.5
            if w < 2:
                load += long_transfer * 0.5
            else:
                load -= long_transfer * 0.5

            p_long = random.gauss(0, 200) * force_mult * grip_quality
            if random.random() < spin_risk:
                p_long *= (1.0 + random.gauss(2.0, 0.5))  # wheelspin spike

            p_lat = lat_force / 4.0 * (0.6 if w >= 2 else 1.4)
            if w in [1, 3]:
                p_lat *= -1

            wheel_data.extend([is_grd, sr, sa, max(load, 50), p_long * 100.0, p_lat * 100.0])

        row = [frame, scheme, speed_cms, accel_cms2] + wheel_data[:24]
        row += [sum(wheel_data[i*6+4] for i in range(4)),
                sum(abs(wheel_data[i*6+5]) for i in range(4))]

        rows.append(row)

        speed_ms += random.gauss(0, 0.3) * dt * 10
        speed_ms = max(speed_ms, 2.0)

    return rows


def write_csv(filepath, rows):
    """Write rows to CSV file."""
    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    header = ("Frame,Scheme,SpeedCmS,AccelCmS2,"
              "W0_GRD,W0_SR,W0_SA,W0_Load,W0_PacejkaLong,W0_PacejkaLat,"
              "W1_GRD,W1_SR,W1_SA,W1_Load,W1_PacejkaLong,W1_PacejkaLat,"
              "W2_GRD,W2_SR,W2_SA,W2_Load,W2_PacejkaLong,W2_PacejkaLat,"
              "W3_GRD,W3_SR,W3_SA,W3_Load,W3_PacejkaLong,W3_PacejkaLat,"
              "TotalPacejkaLong,TotalPacejkaLat")

    with open(filepath, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(header.split(","))
        for row in rows:
            writer.writerow(row)

    print(f"  Wrote {len(rows)} rows to {filepath}")


def main():
    output_dir = sys.argv[1] if len(sys.argv) > 1 else "sample_force_data"
    os.makedirs(output_dir, exist_ok=True)

    scenarios = {
        "StraightAccel": simulate_straight_accel,
        "MediumCorner": simulate_medium_corner,
        "Hairpin": simulate_hairpin,
    }

    scheme_labels = {3: "Default", 0: "FrictionHack", 1: "NoInternalPhysics", 2: "Both"}

    print("Generating sample force capture data...")
    print("=" * 60)

    for scenario_name, sim_func in scenarios.items():
        for scheme_val, scheme_label in scheme_labels.items():
            filename = f"ForceCapture_{scenario_name}_{scheme_label}_20260609_120000.csv"
            filepath = os.path.join(output_dir, filename)
            rows = sim_func(scheme_val)
            write_csv(filepath, rows)

    print(f"\nGenerated 12 CSV files in '{output_dir}/'")
    print("Ready for analysis.")


if __name__ == "__main__":
    main()
