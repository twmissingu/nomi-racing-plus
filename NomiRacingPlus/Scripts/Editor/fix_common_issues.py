"""
NIO Racing Plus - Fix Common Issues Script
Automatically fixes common project issues
Run from UE5 Editor: exec(open('Scripts/Editor/fix_common_issues.py').read())
"""

import unreal
import json
import os
from pathlib import Path

# ============================================================================
# Configuration
# ============================================================================

PROJECT_DIR = unreal.Paths.project_dir()
CONTENT_DIR = unreal.Paths.project_content_dir()

# ============================================================================
# Utility Functions
# ============================================================================

def log_info(msg):
    unreal.log(f"[Fix] {msg}")

def log_fix(msg):
    unreal.log(f"[FIXED] {msg}")

# ============================================================================
# Fix Functions
# ============================================================================

def fix_vehicle_config():
    """Ensure vehicle config has all required entries"""
    log_info("Fixing vehicle configuration...")

    config_path = os.path.join(CONTENT_DIR, "Vehicles", "VehicleConfig.json")

    try:
        with open(config_path) as f:
            config = json.load(f)
    except:
        config = {"vehicles": {}}

    # Ensure vehicles key exists
    if "vehicles" not in config:
        config["vehicles"] = {}

    # Add missing vehicle entries
    default_vehicles = {
        "EP9": {
            "name": "NIO EP9",
            "type": "hypercar",
            "performance": {
                "mass_kg": 1735,
                "power_kw": 1000,
                "power_hp": 1360,
                "torque_nm": 1480,
                "top_speed_kph": 313,
                "acceleration_0_100": 2.7
            }
        },
        "ET7": {
            "name": "NIO ET7",
            "type": "sedan",
            "performance": {
                "mass_kg": 2379,
                "power_kw": 480,
                "power_hp": 653,
                "torque_nm": 850,
                "top_speed_kph": 250,
                "acceleration_0_100": 3.8
            }
        },
        "ES7": {
            "name": "NIO ES7",
            "type": "suv",
            "performance": {
                "mass_kg": 2400,
                "power_kw": 480,
                "power_hp": 653,
                "torque_nm": 850,
                "top_speed_kph": 200,
                "acceleration_0_100": 4.5
            }
        },
        "SU7Ultra": {
            "name": "Xiaomi SU7 Ultra",
            "type": "super_sedan",
            "performance": {
                "mass_kg": 1900,
                "power_kw": 1138,
                "power_hp": 1548,
                "torque_nm": 1635,
                "top_speed_kph": 350,
                "acceleration_0_100": 1.98
            }
        }
    }

    for vehicle_id, vehicle_data in default_vehicles.items():
        if vehicle_id not in config["vehicles"]:
            config["vehicles"][vehicle_id] = vehicle_data
            log_fix(f"Added vehicle config: {vehicle_id}")

    # Save config
    with open(config_path, 'w') as f:
        json.dump(config, f, indent=4)

    log_info("Vehicle config fixed")

def fix_audio_config():
    """Ensure audio config has all required entries"""
    log_info("Fixing audio configuration...")

    config_path = os.path.join(CONTENT_DIR, "Audio", "AudioConfig.json")

    try:
        with open(config_path) as f:
            config = json.load(f)
    except:
        config = {"audio_config": {"motor_sound_profiles": {}}}

    # Ensure structure exists
    if "audio_config" not in config:
        config["audio_config"] = {}
    if "motor_sound_profiles" not in config["audio_config"]:
        config["audio_config"]["motor_sound_profiles"] = {}

    # Add missing motor profiles
    default_profiles = {
        "EP9": {
            "vehicle_id": "EP9",
            "vehicle_name": "NIO EP9",
            "max_rpm": 15000,
            "layers": {
                "motor_whine": {
                    "sound_asset": "Motor_EP9_Whine",
                    "pitch_curve": [
                        {"rpm": 0, "pitch": 0.60},
                        {"rpm": 7500, "pitch": 1.00},
                        {"rpm": 15000, "pitch": 1.80}
                    ]
                }
            }
        },
        "ET7": {
            "vehicle_id": "ET7",
            "vehicle_name": "NIO ET7",
            "max_rpm": 12000,
            "layers": {
                "motor_whine": {
                    "sound_asset": "Motor_ET7_Whine",
                    "pitch_curve": [
                        {"rpm": 0, "pitch": 0.50},
                        {"rpm": 6000, "pitch": 1.00},
                        {"rpm": 12000, "pitch": 1.60}
                    ]
                }
            }
        },
        "ES7": {
            "vehicle_id": "ES7",
            "vehicle_name": "NIO ES7",
            "max_rpm": 12000,
            "layers": {
                "motor_whine": {
                    "sound_asset": "Motor_ES7_Whine",
                    "pitch_curve": [
                        {"rpm": 0, "pitch": 0.45},
                        {"rpm": 6000, "pitch": 1.00},
                        {"rpm": 12000, "pitch": 1.50}
                    ]
                }
            }
        },
        "SU7Ultra": {
            "vehicle_id": "SU7Ultra",
            "vehicle_name": "Xiaomi SU7 Ultra",
            "max_rpm": 21000,
            "layers": {
                "motor_whine": {
                    "sound_asset": "Motor_SU7Ultra_Whine",
                    "pitch_curve": [
                        {"rpm": 0, "pitch": 0.55},
                        {"rpm": 10500, "pitch": 1.00},
                        {"rpm": 21000, "pitch": 2.00}
                    ]
                }
            }
        }
    }

    for vehicle_id, profile in default_profiles.items():
        if vehicle_id not in config["audio_config"]["motor_sound_profiles"]:
            config["audio_config"]["motor_sound_profiles"][vehicle_id] = profile
            log_fix(f"Added motor profile: {vehicle_id}")

    # Save config
    with open(config_path, 'w') as f:
        json.dump(config, f, indent=4)

    log_info("Audio config fixed")

def fix_gameplay_tags():
    """Ensure gameplay tags are present"""
    log_info("Fixing gameplay tags...")

    tags_path = os.path.join(PROJECT_DIR, "Config", "DefaultGameplayTags.ini")

    try:
        with open(tags_path) as f:
            content = f.read()
    except:
        content = "[Script/GameplayTags.GameplayTagsSettings]\n"

    # Required tags
    required_tags = [
        ('Vehicle.NIO.EP9', 'NIO EP9 Hypercar'),
        ('Vehicle.NIO.ET7', 'NIO ET7 Sedan'),
        ('Vehicle.NIO.ES7', 'NIO ES7 SUV'),
        ('Vehicle.NIO.ET5', 'NIO ET5 Sedan'),
        ('Vehicle.Xiaomi.SU7Ultra', 'Xiaomi SU7 Ultra'),
        ('Vehicle.Custom', 'Custom Vehicle'),
        ('Race.State.Countdown', 'Race countdown phase'),
        ('Race.State.Racing', 'Race in progress'),
        ('Race.State.Finished', 'Race finished'),
    ]

    added = 0
    for tag, comment in required_tags:
        tag_line = f'+GameplayTagList=(Tag="{tag}",DevComment="{comment}")'
        if tag not in content:
            content = content.rstrip() + "\n" + tag_line
            added += 1

    if added > 0:
        with open(tags_path, 'w') as f:
            f.write(content + "\n")
        log_fix(f"Added {added} gameplay tags")
    else:
        log_info("All gameplay tags present")

def fix_directory_structure():
    """Ensure required directories exist"""
    log_info("Fixing directory structure...")

    required_dirs = [
        "Vehicles/EP9",
        "Vehicles/ET7",
        "Vehicles/ES7",
        "Vehicles/SU7Ultra",
        "Vehicles/GT",
        "Audio/Motor",
        "Audio/SFX",
        "Audio/UI",
        "Audio/Music",
        "Materials",
        "Textures",
        "Maps",
        "UI/HUD",
        "UI/Menus",
    ]

    for dir_path in required_dirs:
        full_path = os.path.join(CONTENT_DIR, dir_path)
        if not os.path.exists(full_path):
            os.makedirs(full_path, exist_ok=True)
            log_fix(f"Created directory: {dir_path}")

    log_info("Directory structure fixed")

# ============================================================================
# Main Fix Function
# ============================================================================

def fix_all_issues():
    """Fix all common issues"""
    log_info("=" * 60)
    log_info("NIO Racing Plus - Fix Common Issues")
    log_info("=" * 60)
    print()

    fix_directory_structure()
    fix_vehicle_config()
    fix_audio_config()
    fix_gameplay_tags()

    print()
    log_info("=" * 60)
    log_info("All issues fixed!")
    log_info("=" * 60)

# ============================================================================
# Entry Point
# ============================================================================

if __name__ == "__main__":
    try:
        fix_all_issues()
    except Exception as e:
        log_info(f"Error: {e}")
        import traceback
        traceback.print_exc()
