"""
NIO Racing Plus - Game Balance Configuration
Tools for tuning vehicle physics, AI difficulty, and race parameters
Run from UE5 Editor: exec(open('Scripts/Editor/game_balance.py').read())
"""

import unreal
import json
import os
import math

# ============================================================================
# Configuration
# ============================================================================

PROJECT_DIR = unreal.Paths.project_dir()
CONTENT_DIR = unreal.Paths.project_content_dir()

# Balance parameter ranges
PARAMETER_RANGES = {
    "vehicle_mass": {"min": 1000, "max": 3000, "unit": "kg"},
    "motor_power": {"min": 200, "max": 1500, "unit": "kW"},
    "motor_torque": {"min": 400, "max": 2000, "unit": "Nm"},
    "top_speed": {"min": 150, "max": 350, "unit": "kph"},
    "acceleration_0_100": {"min": 2.0, "max": 8.0, "unit": "seconds"},
    "braking_distance": {"min": 30, "max": 50, "unit": "meters"},
    "cornering_g": {"min": 0.8, "max": 2.0, "unit": "g"},
    "ai_reaction_time": {"min": 0.1, "max": 0.5, "unit": "seconds"},
    "ai_accuracy": {"min": 0.5, "max": 1.0, "unit": "ratio"},
    "rubber_band_strength": {"min": 0.0, "max": 1.0, "unit": "ratio"},
}

# ============================================================================
# Utility Functions
# ============================================================================

def log_info(message):
    unreal.log(f"[GameBalance] {message}")

def log_error(message):
    unreal.log_error(f"[GameBalance] {message}")

def load_json_config(path):
    """Load JSON configuration file"""
    try:
        with open(path, 'r') as f:
            return json.load(f)
    except Exception as e:
        log_error(f"Failed to load config: {e}")
        return None

def save_json_config(path, data):
    """Save JSON configuration file"""
    try:
        with open(path, 'w') as f:
            json.dump(data, f, indent=2)
        return True
    except Exception as e:
        log_error(f"Failed to save config: {e}")
        return False

# ============================================================================
# Vehicle Balance
# ============================================================================

class VehicleBalancer:
    def __init__(self, vehicle_type):
        self.vehicle_type = vehicle_type
        self.config_path = os.path.join(CONTENT_DIR, "Vehicles", f"{vehicle_type}_Physics.json")
        self.config = load_json_config(self.config_path)

    def get_current_stats(self):
        """Get current vehicle statistics"""
        if not self.config:
            return None

        return {
            "mass": self.config.get("mass", {}).get("total", 2000),
            "power": self.config.get("motor", {}).get("max_power", 500),
            "torque": self.config.get("motor", {}).get("max_torque", 800),
            "top_speed": self.config.get("motor", {}).get("max_speed", 250),
            "acceleration": self.calculate_acceleration(),
            "braking": self.calculate_braking(),
            "cornering": self.calculate_cornering(),
        }

    def calculate_acceleration(self):
        """Calculate 0-100 kph time estimate"""
        if not self.config:
            return 5.0

        mass = self.config.get("mass", {}).get("total", 2000)
        torque = self.config.get("motor", {}).get("max_torque", 800)
        gear_ratio = self.config.get("transmission", {}).get("gears", [3.5])[0]
        final_drive = self.config.get("transmission", {}).get("final_drive", 3.5)
        wheel_radius = 0.35  # meters

        # F = T * gear * final_drive / r
        force = torque * gear_ratio * final_drive / wheel_radius
        # a = F / m
        acceleration = force / mass
        # t = v / a (simplified)
        time_to_100 = (100 / 3.6) / acceleration

        return round(time_to_100, 1)

    def calculate_braking(self):
        """Calculate braking distance estimate"""
        if not self.config:
            return 40.0

        mass = self.config.get("mass", {}).get("total", 2000)
        brake_force = self.config.get("brakes", {}).get("max_torque", 3000) * 4
        friction = 1.0  # Assume good tire friction

        # F_brake = mu * m * g
        deceleration = (brake_force / mass) + (friction * 9.81)
        # d = v^2 / (2 * a)
        speed_ms = 100 / 3.6  # 100 kph in m/s
        distance = (speed_ms ** 2) / (2 * deceleration)

        return round(distance, 1)

    def calculate_cornering(self):
        """Calculate cornering G-force estimate"""
        if not self.config:
            return 1.0

        # Simplified cornering calculation
        tire_grip = self.config.get("tires", {}).get("front", {}).get("friction", 1.0)
        downforce = self.config.get("aerodynamics", {}).get("downforce_coefficient", 0.3)
        mass = self.config.get("mass", {}).get("total", 2000)

        # G = grip * (1 + downforce * v^2 / (m * g))
        # Simplified for typical cornering speed (100 kph)
        speed_factor = 1 + downforce * (100/3.6)**2 / (mass * 9.81) * 1000
        g_force = tire_grip * min(speed_factor, 3.0)

        return round(g_force, 2)

    def adjust_parameter(self, parameter, value):
        """Adjust a vehicle parameter"""
        if not self.config:
            return False

        # Validate value
        if parameter in PARAMETER_RANGES:
            range_info = PARAMETER_RANGES[parameter]
            value = max(range_info["min"], min(range_info["max"], value))

        # Apply parameter
        if parameter == "vehicle_mass":
            self.config.setdefault("mass", {})["total"] = value
        elif parameter == "motor_power":
            self.config.setdefault("motor", {})["max_power"] = value
        elif parameter == "motor_torque":
            self.config.setdefault("motor", {})["max_torque"] = value
        elif parameter == "top_speed":
            self.config.setdefault("motor", {})["max_speed"] = value

        return True

    def save(self):
        """Save configuration"""
        return save_json_config(self.config_path, self.config)

    def print_stats(self):
        """Print vehicle statistics"""
        stats = self.get_current_stats()
        if not stats:
            log_error(f"No stats available for {self.vehicle_type}")
            return

        log_info(f"\n{'='*50}")
        log_info(f"  {self.vehicle_type} Vehicle Statistics")
        log_info(f"{'='*50}")
        log_info(f"  Mass: {stats['mass']} kg")
        log_info(f"  Power: {stats['power']} kW ({stats['power']*1.34:.0f} HP)")
        log_info(f"  Torque: {stats['torque']} Nm")
        log_info(f"  Top Speed: {stats['top_speed']} kph")
        log_info(f"  0-100 kph: {stats['acceleration']} seconds")
        log_info(f"  Braking (100-0): {stats['braking']} meters")
        log_info(f"  Cornering: {stats['cornering']} g")
        log_info(f"{'='*50}\n")

# ============================================================================
# AI Balance
# ============================================================================

class AIBalancer:
    def __init__(self):
        self.config_path = os.path.join(CONTENT_DIR, "AI", "AIProfiles.json")
        self.config = load_json_config(self.config_path)

    def get_difficulty_profiles(self):
        """Get all difficulty profiles"""
        if not self.config:
            return {}

        return self.config.get("difficulty_profiles", {})

    def adjust_difficulty(self, profile_name, adjustments):
        """Adjust difficulty profile"""
        if not self.config:
            return False

        profiles = self.config.get("difficulty_profiles", {})
        if profile_name not in profiles:
            log_error(f"Profile not found: {profile_name}")
            return False

        profile = profiles[profile_name]

        # Apply adjustments
        for key, value in adjustments.items():
            if key in profile:
                profile[key] = value

        return True

    def print_profiles(self):
        """Print all difficulty profiles"""
        profiles = self.get_difficulty_profiles()

        log_info(f"\n{'='*50}")
        log_info(f"  AI Difficulty Profiles")
        log_info(f"{'='*50}")

        for name, profile in profiles.items():
            log_info(f"\n  {name}:")
            log_info(f"    Reaction Time: {profile.get('reaction_time', 0.2)}s")
            log_info(f"    Accuracy: {profile.get('accuracy', 0.8)}")
            log_info(f"    Aggression: {profile.get('aggression', 0.5)}")
            log_info(f"    Speed Factor: {profile.get('speed_factor', 1.0)}")
            log_info(f"    Rubber Band: {profile.get('rubber_band', 0.5)}")

        log_info(f"\n{'='*50}\n")

# ============================================================================
# Race Balance
# ============================================================================

class RaceBalancer:
    def __init__(self):
        self.config_path = os.path.join(CONTENT_DIR, "Race", "RaceConfig.json")
        self.config = load_json_config(self.config_path)

    def get_race_modes(self):
        """Get all race modes"""
        if not self.config:
            return {}

        return self.config.get("race_modes", {})

    def adjust_scoring(self, mode_name, scoring_adjustments):
        """Adjust scoring system for a race mode"""
        if not self.config:
            return False

        modes = self.config.get("race_modes", {})
        if mode_name not in modes:
            log_error(f"Mode not found: {mode_name}")
            return False

        mode = modes[mode_name]
        scoring = mode.get("scoring", {})

        for key, value in scoring_adjustments.items():
            scoring[key] = value

        mode["scoring"] = scoring
        return True

    def print_race_modes(self):
        """Print all race modes"""
        modes = self.get_race_modes()

        log_info(f"\n{'='*50}")
        log_info(f"  Race Modes")
        log_info(f"{'='*50}")

        for name, mode in modes.items():
            log_info(f"\n  {name}:")
            log_info(f"    Vehicles: {', '.join(mode.get('vehicles', []))}")
            log_info(f"    Tracks: {', '.join(mode.get('tracks', []))}")
            log_info(f"    Laps: {mode.get('laps', 3)}")

            scoring = mode.get("scoring", {})
            if scoring:
                log_info(f"    Scoring:")
                for pos, points in scoring.items():
                    log_info(f"      {pos}: {points} pts")

        log_info(f"\n{'='*50}\n")

# ============================================================================
# Balance Analysis
# ============================================================================

class BalanceAnalyzer:
    def __init__(self):
        self.vehicles = {}
        self.ai = AIBalancer()
        self.race = RaceBalancer()

    def load_all_vehicles(self):
        """Load all vehicle configurations"""
        for vehicle_type in ["EP9", "ET7", "ES7"]:
            self.vehicles[vehicle_type] = VehicleBalancer(vehicle_type)

    def analyze_vehicle_balance(self):
        """Analyze vehicle balance"""
        self.load_all_vehicles()

        log_info(f"\n{'='*60}")
        log_info(f"  Vehicle Balance Analysis")
        log_info(f"{'='*60}")

        # Collect stats
        stats = {}
        for name, vehicle in self.vehicles.items():
            vehicle.print_stats()
            stats[name] = vehicle.get_current_stats()

        # Analyze balance
        if len(stats) >= 2:
            self.compare_vehicles(stats)

        return stats

    def compare_vehicles(self, stats):
        """Compare vehicle statistics"""
        log_info(f"\n  Comparison:")

        # Power-to-weight ratio
        for name, s in stats.items():
            if s:
                ptw = s["power"] / (s["mass"] / 1000)  # kW per ton
                log_info(f"    {name}: {ptw:.0f} kW/ton")

        # Performance ranking
        log_info(f"\n  Performance Ranking (0-100 kph):")
        sorted_by_accel = sorted(stats.items(), key=lambda x: x[1]["acceleration"] if x[1] else 999)
        for i, (name, s) in enumerate(sorted_by_accel, 1):
            if s:
                log_info(f"    {i}. {name}: {s['acceleration']}s")

    def suggest_adjustments(self):
        """Suggest balance adjustments"""
        log_info(f"\n{'='*60}")
        log_info(f"  Balance Suggestions")
        log_info(f"{'='*60}")

        suggestions = [
            "1. EP9 should be fastest (0-100 < 3.0s) but hardest to control",
            "2. ET7 should be balanced (0-100 ~4.0s) with good stability",
            "3. ES7 should be slowest (0-100 ~5.0s) but most forgiving",
            "4. AI difficulty should scale 0.7x to 1.1x player speed",
            "5. Rubber band should be subtle (0.3-0.5 strength)",
            "6. Scoring: 1st=25pts, 2nd=18pts, 3rd=15pts, etc.",
        ]

        for suggestion in suggestions:
            log_info(f"  {suggestion}")

        log_info(f"\n{'='*60}\n")

# ============================================================================
# Quick Balance Presets
# ============================================================================

def apply_arcade_preset():
    """Apply arcade-style balance preset"""
    log_info("Applying Arcade preset...")

    # Make vehicles more forgiving
    for vehicle_type in ["EP9", "ET7", "ES7"]:
        balancer = VehicleBalancer(vehicle_type)
        if balancer.config:
            # Increase tire grip
            for wheel in ["front_left", "front_right", "rear_left", "rear_right"]:
                if wheel in balancer.config.get("tires", {}):
                    balancer.config["tires"][wheel]["friction"] *= 1.2

            # Reduce mass slightly
            balancer.config["mass"]["total"] *= 0.9

            # Increase brake power
            balancer.config["brakes"]["max_torque"] *= 1.3

            balancer.save()

    log_info("Arcade preset applied")

def apply_simulation_preset():
    """Apply simulation-style balance preset"""
    log_info("Applying Simulation preset...")

    # Make vehicles more realistic
    for vehicle_type in ["EP9", "ET7", "ES7"]:
        balancer = VehicleBalancer(vehicle_type)
        if balancer.config:
            # Keep tire grip realistic
            # Keep mass realistic
            # Reduce brake power slightly
            balancer.config["brakes"]["max_torque"] *= 0.9

            # Increase suspension sensitivity
            for wheel in ["front_left", "front_right", "rear_left", "rear_right"]:
                if wheel in balancer.config.get("suspension", {}):
                    balancer.config["suspension"][wheel]["damping"] *= 1.1

            balancer.save()

    log_info("Simulation preset applied")

# ============================================================================
# Entry Point
# ============================================================================

if __name__ == "__main__":
    log_info("Game Balance Configuration loaded!")
    log_info("")
    log_info("Available tools:")
    log_info("  VehicleBalancer('EP9').print_stats()")
    log_info("  VehicleBalancer('ET7').print_stats()")
    log_info("  VehicleBalancer('ES7').print_stats()")
    log_info("  AIBalancer().print_profiles()")
    log_info("  RaceBalancer().print_race_modes()")
    log_info("  BalanceAnalyzer().analyze_vehicle_balance()")
    log_info("")
    log_info("Presets:")
    log_info("  apply_arcade_preset()")
    log_info("  apply_simulation_preset()")

# Auto-execute analysis
if __name__ == "__main__":
    try:
        analyzer = BalanceAnalyzer()
        analyzer.analyze_vehicle_balance()
        analyzer.suggest_adjustments()
    except Exception as e:
        log_error(f"Error: {e}")
        log_info("Run balance functions manually")
