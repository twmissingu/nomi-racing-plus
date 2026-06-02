#!/usr/bin/env python3
"""
NIO Racing Plus - Configuration Validator
Validates all game configuration files for correctness.
Run standalone: python3 Scripts/validate_configs.py
"""

import json
import os
import sys

# Project root (parent of Scripts/)
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(SCRIPT_DIR)
CONTENT_DIR = os.path.join(PROJECT_DIR, "Content")
SOURCE_DIR = os.path.join(PROJECT_DIR, "Source", "NomiRacingPlus")

# Colors
RED = '\033[0;31m'
GREEN = '\033[0;32m'
YELLOW = '\033[1;33m'
NC = '\033[0m'

passed = 0
failed = 0
warned = 0

def log_pass(msg):
    global passed
    print(f"  {GREEN}✓{NC} {msg}")
    passed += 1

def log_fail(msg):
    global failed
    print(f"  {RED}✗{NC} {msg}")
    failed += 1

def log_warn(msg):
    global warned
    print(f"  {YELLOW}⚠{NC} {msg}")
    warned += 1

def validate_vehicle_config():
    """Validate VehicleConfig.json"""
    print(f"\n{YELLOW}━━━ Vehicle Config ━━━{NC}")
    path = os.path.join(CONTENT_DIR, "Vehicles", "VehicleConfig.json")
    if not os.path.exists(path):
        log_fail("VehicleConfig.json not found")
        return

    with open(path, 'r') as f:
        data = json.load(f)

    vehicles = data.get("vehicles", {})
    if not vehicles:
        log_fail("No vehicles defined")
        return

    required_vehicles = ["EP9", "ET7", "ES7", "SU7Ultra"]
    for v_id in required_vehicles:
        if v_id not in vehicles:
            log_fail(f"Missing vehicle: {v_id}")
            continue

        v = vehicles[v_id]
        name = v.get("name", v_id)
        perf = v.get("performance", {})
        electric = v.get("electric", {})
        chaos = v.get("chaos_vehicle", {})

        # Check required performance fields
        required_perf = ["mass_kg", "power_kw", "top_speed_kph", "acceleration_0_100"]
        for field in required_perf:
            if field not in perf:
                log_fail(f"{name}: missing performance.{field}")
            elif perf[field] <= 0:
                log_fail(f"{name}: invalid performance.{field}: {perf[field]}")

        # Check electric config
        if not electric:
            log_fail(f"{name}: missing electric config")
        elif electric.get("battery_capacity_kwh", 0) <= 0:
            log_fail(f"{name}: invalid battery capacity")

        # Check chaos vehicle config
        if not chaos:
            log_fail(f"{name}: missing chaos_vehicle config")
        else:
            if "engine" not in chaos:
                log_fail(f"{name}: missing engine config")
            if "tires" not in chaos:
                log_fail(f"{name}: missing tires config")

        # Check LOD config
        lod = v.get("lod", {})
        if not lod:
            log_warn(f"{name}: missing LOD config")
        else:
            lod0 = lod.get("lod0_faces", 0)
            lod1 = lod.get("lod1_faces", 0)
            lod2 = lod.get("lod2_faces", 0)
            if lod0 <= lod1 or lod1 <= lod2:
                log_fail(f"{name}: LOD chain invalid ({lod0}>{lod1}>{lod2})")

        log_pass(f"{name} ({v_id})")

    log_info = lambda m: print(f"  {GREEN}ℹ{NC} {m}")
    log_info(f"{len(vehicles)} vehicles defined")

def validate_audio_config():
    """Validate AudioConfig.json"""
    print(f"\n{YELLOW}━━━ Audio Config ━━━{NC}")
    path = os.path.join(CONTENT_DIR, "Audio", "AudioConfig.json")
    if not os.path.exists(path):
        log_fail("AudioConfig.json not found")
        return

    with open(path, 'r') as f:
        data = json.load(f)

    # Check top-level structure
    if "audio_config" not in data:
        log_fail("Missing audio_config section")
        return

    config = data["audio_config"]
    required_sections = ["motor_sound_profiles", "sfx", "music", "spatial_audio"]
    for section in required_sections:
        if section in config:
            log_pass(f"{section} section present")
        else:
            log_fail(f"Missing {section} section")

    # Check motor sound profiles
    profiles = config.get("motor_sound_profiles", {})
    if profiles:
        log_pass(f"{len(profiles)} motor sound profiles")
    else:
        log_warn("No motor sound profiles defined")

def validate_ai_config():
    """Validate AIProfiles.json"""
    print(f"\n{YELLOW}━━━ AI Config ━━━{NC}")
    path = os.path.join(CONTENT_DIR, "AI", "AIProfiles.json")
    if not os.path.exists(path):
        log_fail("AIProfiles.json not found")
        return

    with open(path, 'r') as f:
        data = json.load(f)

    profiles = data.get("ai_profiles", [])
    if not profiles:
        log_fail("No AI profiles defined")
    else:
        log_pass(f"{len(profiles)} AI profiles defined")

    # Check other important sections
    important_sections = ["difficulty_scaling", "rubber_band_profiles", "slipstream_settings"]
    for section in important_sections:
        if section in data:
            log_pass(f"{section} configured")
        else:
            log_warn(f"Missing {section}")

def validate_track_config():
    """Validate TrackConfig.json"""
    print(f"\n{YELLOW}━━━ Track Config ━━━{NC}")
    path = os.path.join(CONTENT_DIR, "Maps", "TrackConfig.json")
    if not os.path.exists(path):
        log_fail("TrackConfig.json not found")
        return

    with open(path, 'r') as f:
        data = json.load(f)

    tracks = data.get("tracks", {})
    if not tracks:
        log_fail("No tracks defined")
        return

    for track_id, track in tracks.items():
        name = track.get("name", track_id)
        required = ["name", "length_km", "checkpoints"]
        for field in required:
            if field not in track:
                log_fail(f"{name}: missing {field}")
            else:
                log_pass(f"{name}")

    print(f"  {GREEN}ℹ{NC} {len(tracks)} tracks defined")

def validate_nomi_comments():
    """Validate NOMI comments config"""
    print(f"\n{YELLOW}━━━ NOMI Comments ━━━{NC}")
    path = os.path.join(CONTENT_DIR, "NOMI", "Comments", "DefaultComments.json")
    if not os.path.exists(path):
        log_fail("DefaultComments.json not found")
        return

    with open(path, 'r') as f:
        data = json.load(f)

    # Top-level keys are event types (overtake, drift, etc.)
    event_types = list(data.keys())
    if not event_types:
        log_fail("No comment event types defined")
    else:
        log_pass(f"{len(event_types)} comment event types defined")

    # Check each event type has comments
    for event_type in event_types:
        comments = data[event_type]
        if isinstance(comments, list) and len(comments) > 0:
            log_pass(f"{event_type}: {len(comments)} comments")
        else:
            log_warn(f"{event_type}: no comments")

def validate_source_files():
    """Validate key source files exist"""
    print(f"\n{YELLOW}━━━ Source Files ━━━{NC}")

    key_files = [
        ("Vehicles/VehicleStateManager.cpp", "Vehicle state manager"),
        ("Vehicles/NIOVehicleMovementComponent.cpp", "Vehicle movement"),
        ("Vehicles/TirePhysicsModel.cpp", "Tire physics"),
        ("AI/AIBehaviorTree.cpp", "AI behavior tree"),
        ("AI/AICarController.cpp", "AI car controller"),
        ("Race/RaceManager.cpp", "Race manager"),
        ("Race/CheckpointSystem.cpp", "Checkpoint system"),
        ("Core/CameraSystem.cpp", "Camera system"),
        ("Core/AudioManager.cpp", "Audio manager"),
    ]

    for rel_path, desc in key_files:
        full_path = os.path.join(SOURCE_DIR, rel_path)
        if os.path.exists(full_path):
            log_pass(f"{desc} ({rel_path})")
        else:
            log_fail(f"Missing {desc} ({rel_path})")

def validate_test_files():
    """Validate test suite exists"""
    print(f"\n{YELLOW}━━━ Test Suite ━━━{NC}")

    # Try multiple possible test directory locations
    test_dir = os.path.join(SOURCE_DIR, "Tests")
    if not os.path.exists(test_dir):
        # Try parent directory
        test_dir = os.path.join(PROJECT_DIR, "Source", "NomiRacingPlus", "Tests")
    if not os.path.exists(test_dir):
        log_fail("Tests directory not found")
        return

    headers = [f for f in os.listdir(test_dir) if f.endswith('.h')]
    sources = [f for f in os.listdir(test_dir) if f.endswith('.cpp')]

    log_pass(f"{len(headers)} test headers, {len(sources)} test sources")

    # Check key test files
    key_tests = [
        "TestUtilities.cpp",
        "VehicleSystemTest.cpp",
        "AITest.cpp",
        "RaceSystemTest.cpp",
    ]

    for test_file in key_tests:
        if os.path.exists(os.path.join(test_dir, test_file)):
            log_pass(f"{test_file}")
        else:
            log_fail(f"Missing {test_file}")

def main():
    print(f"\n{YELLOW}╔══════════════════════════════════════════════════╗{NC}")
    print(f"{YELLOW}║   NIO Racing Plus - Configuration Validator      ║{NC}")
    print(f"{YELLOW}╚══════════════════════════════════════════════════╝{NC}")

    validate_vehicle_config()
    validate_audio_config()
    validate_ai_config()
    validate_track_config()
    validate_nomi_comments()
    validate_source_files()
    validate_test_files()

    # Summary
    print(f"\n{YELLOW}━━━ Summary ━━━{NC}")
    print(f"  {GREEN}✓ Passed:{NC}  {passed}")
    print(f"  {YELLOW}⚠ Warned:{NC}  {warned}")
    print(f"  {RED}✗ Failed:{NC}  {failed}")

    if failed > 0:
        print(f"\n{RED}Validation FAILED{NC}")
        sys.exit(1)
    elif warned > 0:
        print(f"\n{YELLOW}Validation PASSED with warnings{NC}")
        sys.exit(0)
    else:
        print(f"\n{GREEN}Validation PASSED{NC}")
        sys.exit(0)

if __name__ == "__main__":
    main()
