"""
NIO Racing Plus - Project Validation Script
Validates project integrity and readiness for gameplay
Run from UE5 Editor: exec(open('Scripts/Editor/validate_project.py').read())
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
    unreal.log(f"[Validate] {msg}")

def log_pass(msg):
    unreal.log(f"[PASS] {msg}")

def log_fail(msg):
    unreal.log_error(f"[FAIL] {msg}")

def log_warn(msg):
    unreal.log_warning(f"[WARN] {msg}")

# ============================================================================
# Validation Checks
# ============================================================================

class ProjectValidator:
    def __init__(self):
        self.passed = 0
        self.failed = 0
        self.warnings = 0

    def run_all_checks(self):
        """Run all validation checks"""
        log_info("=" * 60)
        log_info("NIO Racing Plus - Project Validation")
        log_info("=" * 60)
        print()

        self.check_vehicle_configs()
        self.check_audio_configs()
        self.check_track_configs()
        self.check_ai_configs()
        self.check_material_assets()
        self.check_vehicle_blueprints()
        self.check_audio_assets()
        self.check_input_mappings()

        self.print_summary()
        return self.failed == 0

    def check_vehicle_configs(self):
        """Check vehicle configuration files"""
        log_info("Checking vehicle configurations...")

        vehicles = ["EP9", "ET7", "ES7", "SU7Ultra"]
        config_path = os.path.join(CONTENT_DIR, "Vehicles", "VehicleConfig.json")

        if os.path.exists(config_path):
            try:
                with open(config_path) as f:
                    config = json.load(f)

                for vehicle in vehicles:
                    if vehicle in config.get("vehicles", {}):
                        log_pass(f"Vehicle config: {vehicle}")
                        self.passed += 1
                    else:
                        log_fail(f"Vehicle config missing: {vehicle}")
                        self.failed += 1
            except Exception as e:
                log_fail(f"VehicleConfig.json error: {e}")
                self.failed += 1
        else:
            log_fail("VehicleConfig.json not found")
            self.failed += 1

        # Check physics files
        for vehicle in vehicles:
            physics_path = os.path.join(CONTENT_DIR, "Vehicles", f"{vehicle}_Physics.json")
            if os.path.exists(physics_path):
                log_pass(f"Physics config: {vehicle}")
                self.passed += 1
            else:
                log_warn(f"Physics config missing: {vehicle}")
                self.warnings += 1

    def check_audio_configs(self):
        """Check audio configuration"""
        log_info("Checking audio configurations...")

        audio_path = os.path.join(CONTENT_DIR, "Audio", "AudioConfig.json")
        if os.path.exists(audio_path):
            try:
                with open(audio_path) as f:
                    config = json.load(f)

                # Check motor profiles
                profiles = config.get("audio_config", {}).get("motor_sound_profiles", {})
                for vehicle in ["EP9", "ET7", "ES7", "SU7Ultra"]:
                    if vehicle in profiles:
                        log_pass(f"Motor sound profile: {vehicle}")
                        self.passed += 1
                    else:
                        log_fail(f"Motor sound profile missing: {vehicle}")
                        self.failed += 1
            except Exception as e:
                log_fail(f"AudioConfig.json error: {e}")
                self.failed += 1
        else:
            log_fail("AudioConfig.json not found")
            self.failed += 1

    def check_track_configs(self):
        """Check track configurations"""
        log_info("Checking track configurations...")

        track_path = os.path.join(CONTENT_DIR, "Maps", "TrackConfig.json")
        if os.path.exists(track_path):
            try:
                with open(track_path) as f:
                    config = json.load(f)

                tracks = config.get("tracks", [])
                log_pass(f"Track config: {len(tracks)} tracks defined")
                self.passed += 1
            except Exception as e:
                log_fail(f"TrackConfig.json error: {e}")
                self.failed += 1
        else:
            log_fail("TrackConfig.json not found")
            self.failed += 1

    def check_ai_configs(self):
        """Check AI configurations"""
        log_info("Checking AI configurations...")

        ai_path = os.path.join(CONTENT_DIR, "AI", "AIProfiles.json")
        if os.path.exists(ai_path):
            try:
                with open(ai_path) as f:
                    config = json.load(f)

                profiles = config.get("difficulty_profiles", {})
                log_pass(f"AI config: {len(profiles)} difficulty profiles")
                self.passed += 1
            except Exception as e:
                log_fail(f"AIProfiles.json error: {e}")
                self.failed += 1
        else:
            log_fail("AIProfiles.json not found")
            self.failed += 1

    def check_material_assets(self):
        """Check material assets exist"""
        log_info("Checking material assets...")

        # Check if material directories exist
        materials_dir = os.path.join(CONTENT_DIR, "Materials")
        if os.path.exists(materials_dir):
            log_pass("Materials directory exists")
            self.passed += 1
        else:
            log_warn("Materials directory not found (will be created by setup)")
            self.warnings += 1

    def check_vehicle_blueprints(self):
        """Check vehicle blueprints"""
        log_info("Checking vehicle blueprints...")

        # Check for vehicle blueprint directories
        for vehicle in ["EP9", "ET7", "ES7", "SU7Ultra"]:
            bp_dir = os.path.join(CONTENT_DIR, "Vehicles", vehicle)
            if os.path.exists(bp_dir):
                log_pass(f"Vehicle directory: {vehicle}")
                self.passed += 1
            else:
                log_warn(f"Vehicle directory not found: {vehicle} (will be created)")
                self.warnings += 1

    def check_audio_assets(self):
        """Check audio assets"""
        log_info("Checking audio assets...")

        audio_dirs = ["Motor", "SFX", "UI"]
        for dir_name in audio_dirs:
            dir_path = os.path.join(CONTENT_DIR, "Audio", dir_name)
            if os.path.exists(dir_path):
                wav_count = len([f for f in os.listdir(dir_path) if f.endswith('.wav')])
                if wav_count > 0:
                    log_pass(f"Audio {dir_name}: {wav_count} files")
                    self.passed += 1
                else:
                    log_warn(f"Audio {dir_name}: no WAV files")
                    self.warnings += 1
            else:
                log_warn(f"Audio {dir_name}: directory not found")
                self.warnings += 1

    def check_input_mappings(self):
        """Check input mappings"""
        log_info("Checking input mappings...")

        # Check DefaultInput.ini
        input_path = os.path.join(PROJECT_DIR, "Config", "DefaultInput.ini")
        if os.path.exists(input_path):
            log_pass("DefaultInput.ini exists")
            self.passed += 1
        else:
            log_fail("DefaultInput.ini not found")
            self.failed += 1

        # Check DefaultGameplayTags.ini
        tags_path = os.path.join(PROJECT_DIR, "Config", "DefaultGameplayTags.ini")
        if os.path.exists(tags_path):
            log_pass("DefaultGameplayTags.ini exists")
            self.passed += 1
        else:
            log_fail("DefaultGameplayTags.ini not found")
            self.failed += 1

    def print_summary(self):
        """Print validation summary"""
        print()
        log_info("=" * 60)
        log_info("Validation Summary")
        log_info("=" * 60)
        log_info(f"  Passed:   {self.passed}")
        log_info(f"  Failed:   {self.failed}")
        log_info(f"  Warnings: {self.warnings}")
        print()

        if self.failed == 0:
            log_info("RESULT: ALL CHECKS PASSED!")
            log_info("Project is ready for gameplay.")
        else:
            log_fail(f"RESULT: {self.failed} CHECKS FAILED")
            log_info("Please fix the issues above before playing.")

        log_info("=" * 60)

# ============================================================================
# Entry Point
# ============================================================================

def run_validation():
    """Run project validation"""
    validator = ProjectValidator()
    return validator.run_all_checks()

# Auto-execute
try:
    run_validation()
except Exception as e:
    log_fail(f"Validation error: {e}")
    import traceback
    traceback.print_exc()
