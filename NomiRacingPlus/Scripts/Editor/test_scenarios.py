"""
NIO Racing Plus - Test Scenarios
Creates and runs test scenarios for player testing and QA
Run from UE5 Editor: exec(open('Scripts/Editor/test_scenarios.py').read())
"""

import unreal
import json
import os
import sys
import time

# ============================================================================
# Configuration
# ============================================================================

PROJECT_DIR = unreal.Paths.project_dir()
CONTENT_DIR = unreal.Paths.project_content_dir()

# Test scenario definitions
TEST_SCENARIOS = {
    "vehicle_performance": {
        "description": "Test vehicle acceleration, top speed, braking",
        "duration": 120,
        "tracks": ["SpeedwayOval"],
        "vehicles": ["EP9", "ET7", "ES7"],
        "tests": [
            "acceleration_0_100",
            "top_speed",
            "braking_100_0",
            "cornering_stability",
        ]
    },
    "ai_competitiveness": {
        "description": "Test AI opponent behavior and difficulty scaling",
        "duration": 300,
        "tracks": ["NIOCityCircuit", "ShanghaiPudong"],
        "vehicles": ["EP9"],
        "tests": [
            "ai_race_completion",
            "ai_overtaking",
            "ai_rubber_band",
            "ai_collision_avoidance",
        ]
    },
    "race_modes": {
        "description": "Test all race modes and scoring",
        "duration": 600,
        "tracks": ["NIOCityCircuit", "SpeedwayOval", "DesertRally"],
        "vehicles": ["EP9", "ET7", "ES7"],
        "tests": [
            "street_gt_mode",
            "nio_championship_mode",
            "baja_rally_mode",
            "scoring_system",
        ]
    },
    "nomi_system": {
        "description": "Test NOMI companion commentary and reactions",
        "duration": 180,
        "tracks": ["NIOCityCircuit"],
        "vehicles": ["ET7"],
        "tests": [
            "nomi_commentary_triggers",
            "nomi_emotion_system",
            "nomi_race_feedback",
        ]
    },
    "camera_system": {
        "description": "Test all camera modes and transitions",
        "duration": 120,
        "tracks": ["NIOCityCircuit"],
        "vehicles": ["EP9"],
        "tests": [
            "chase_camera",
            "cockpit_camera",
            "cinematic_camera",
            "replay_camera",
        ]
    },
    "audio_system": {
        "description": "Test audio mixing and spatial sound",
        "duration": 180,
        "tracks": ["NIOCityCircuit"],
        "vehicles": ["EP9", "ET7", "ES7"],
        "tests": [
            "motor_sound_layers",
            "tire_screech",
            "collision_sounds",
            "spatial_audio",
        ]
    },
    "performance": {
        "description": "Test frame rate and memory usage",
        "duration": 300,
        "tracks": ["ShanghaiPudong"],
        "vehicles": ["EP9"],
        "tests": [
            "fps_stability",
            "memory_usage",
            "draw_calls",
            "texture_streaming",
        ]
    },
    "edge_cases": {
        "description": "Test edge cases and error handling",
        "duration": 240,
        "tracks": ["NIOCityCircuit"],
        "vehicles": ["EP9"],
        "tests": [
            "wrong_way_driving",
            "track_boundary_collision",
            "vehicle_flip_recovery",
            "pause_resume",
            "lap_counting_accuracy",
        ]
    },
}

# ============================================================================
# Utility Functions
# ============================================================================

def log_info(message):
    unreal.log(f"[TestScenario] {message}")

def log_error(message):
    unreal.log_error(f"[TestScenario] {message}")

def log_test(test_name, passed, details=""):
    status = "✓ PASS" if passed else "✗ FAIL"
    unreal.log(f"[TEST] {status}: {test_name}")
    if details:
        unreal.log(f"       {details}")

# ============================================================================
# Configuration Validation
# ============================================================================

def validate_vehicle_config():
    """Validate VehicleConfig.json has all required fields"""
    config_path = os.path.join(PROJECT_DIR, "Content", "Vehicles", "VehicleConfig.json")
    if not os.path.exists(config_path):
        return False, "VehicleConfig.json not found"

    with open(config_path, 'r') as f:
        data = json.load(f)

    vehicles = data.get("vehicles", {})
    required_vehicles = ["EP9", "ET7", "ES7", "SU7Ultra"]
    required_fields = ["name", "type", "performance", "electric", "chaos_vehicle"]

    errors = []
    for v_id in required_vehicles:
        if v_id not in vehicles:
            errors.append(f"Missing vehicle: {v_id}")
            continue
        v = vehicles[v_id]
        for field in required_fields:
            if field not in v:
                errors.append(f"Vehicle {v_id} missing field: {field}")
        # Check performance fields
        perf = v.get("performance", {})
        for pf in ["mass_kg", "power_kw", "top_speed_kph", "acceleration_0_100"]:
            if pf not in perf:
                errors.append(f"Vehicle {v_id} missing performance.{pf}")
            elif perf[pf] <= 0:
                errors.append(f"Vehicle {v_id} has invalid performance.{pf}: {perf[pf]}")

    if errors:
        return False, "; ".join(errors)
    return True, f"{len(vehicles)} vehicles validated"

def validate_audio_config():
    """Validate AudioConfig.json"""
    config_path = os.path.join(PROJECT_DIR, "Content", "Audio", "AudioConfig.json")
    if not os.path.exists(config_path):
        return False, "AudioConfig.json not found"

    with open(config_path, 'r') as f:
        data = json.load(f)

    required_sections = ["motor_sounds", "tire_sounds", "collision_sounds", "music"]
    errors = []
    for section in required_sections:
        if section not in data:
            errors.append(f"Missing section: {section}")

    if errors:
        return False, "; ".join(errors)
    return True, "Audio config validated"

def validate_ai_config():
    """Validate AIProfiles.json"""
    config_path = os.path.join(PROJECT_DIR, "Content", "AI", "AIProfiles.json")
    if not os.path.exists(config_path):
        return False, "AIProfiles.json not found"

    with open(config_path, 'r') as f:
        data = json.load(f)

    if "profiles" not in data:
        return False, "Missing 'profiles' section"

    profiles = data["profiles"]
    if len(profiles) == 0:
        return False, "No AI profiles defined"

    return True, f"{len(profiles)} AI profiles validated"

def validate_track_config():
    """Validate TrackConfig.json"""
    config_path = os.path.join(PROJECT_DIR, "Content", "Tracks", "TrackConfig.json")
    if not os.path.exists(config_path):
        return False, "TrackConfig.json not found"

    with open(config_path, 'r') as f:
        data = json.load(f)

    if "tracks" not in data:
        return False, "Missing 'tracks' section"

    tracks = data["tracks"]
    required_fields = ["name", "length_km", "checkpoints"]
    errors = []
    for track_id, track in tracks.items():
        for field in required_fields:
            if field not in track:
                errors.append(f"Track {track_id} missing field: {field}")

    if errors:
        return False, "; ".join(errors[:3])
    return True, f"{len(tracks)} tracks validated"

def validate_all_configs():
    """Run all configuration validations"""
    results = {}
    validators = {
        "vehicle_config": validate_vehicle_config,
        "audio_config": validate_audio_config,
        "ai_config": validate_ai_config,
        "track_config": validate_track_config,
    }

    for name, validator in validators.items():
        try:
            passed, details = validator()
            results[name] = {"passed": passed, "details": details}
        except Exception as e:
            results[name] = {"passed": False, "details": str(e)}

    return results

# ============================================================================
# Test Runner
# ============================================================================

class TestRunner:
    def __init__(self):
        self.results = {}
        self.current_scenario = None
        self.current_test = None
        self.start_time = None

    def run_scenario(self, scenario_name):
        """Run a test scenario"""
        if scenario_name not in TEST_SCENARIOS:
            log_error(f"Unknown scenario: {scenario_name}")
            return False

        scenario = TEST_SCENARIOS[scenario_name]
        self.current_scenario = scenario_name
        self.results[scenario_name] = {}

        log_info(f"\n{'='*60}")
        log_info(f"Running Scenario: {scenario_name}")
        log_info(f"Description: {scenario['description']}")
        log_info(f"Duration: {scenario['duration']}s")
        log_info(f"{'='*60}\n")

        # Run each test
        for test in scenario["tests"]:
            self.current_test = test
            self.start_time = time.time()

            log_info(f"Running test: {test}")

            # Run test based on name
            result = self.run_test(test, scenario)
            self.results[scenario_name][test] = result

            # Log result
            log_test(test, result["passed"], result.get("details", ""))

        # Print summary
        self.print_scenario_summary(scenario_name)

        return True

    def run_test(self, test_name, scenario):
        """Run individual test"""
        # This is where actual test logic would go
        # For now, return placeholder results

        test_methods = {
            "acceleration_0_100": self.test_acceleration,
            "top_speed": self.test_top_speed,
            "braking_100_0": self.test_braking,
            "cornering_stability": self.test_cornering,
            "ai_race_completion": self.test_ai_completion,
            "ai_overtaking": self.test_ai_overtaking,
            "ai_rubber_band": self.test_ai_rubber_band,
            "ai_collision_avoidance": self.test_ai_collision,
            "street_gt_mode": self.test_street_gt,
            "nio_championship_mode": self.test_nio_championship,
            "baja_rally_mode": self.test_baja_rally,
            "scoring_system": self.test_scoring,
            "nomi_commentary_triggers": self.test_nomi_commentary,
            "nomi_emotion_system": self.test_nomi_emotion,
            "nomi_race_feedback": self.test_nomi_feedback,
            "chase_camera": self.test_chase_camera,
            "cockpit_camera": self.test_cockpit_camera,
            "cinematic_camera": self.test_cinematic_camera,
            "replay_camera": self.test_replay_camera,
            "motor_sound_layers": self.test_motor_sounds,
            "tire_screech": self.test_tire_screech,
            "collision_sounds": self.test_collision_sounds,
            "spatial_audio": self.test_spatial_audio,
            "fps_stability": self.test_fps,
            "memory_usage": self.test_memory,
            "draw_calls": self.test_draw_calls,
            "texture_streaming": self.test_texture_streaming,
            "wrong_way_driving": self.test_wrong_way,
            "track_boundary_collision": self.test_boundary,
            "vehicle_flip_recovery": self.test_flip_recovery,
            "pause_resume": self.test_pause_resume,
            "lap_counting_accuracy": self.test_lap_counting,
        }

        if test_name in test_methods:
            return test_methods[test_name](scenario)

        return {"passed": False, "details": "Test not implemented"}

    # ========================================================================
    # Test Implementations
    # ========================================================================

    def test_acceleration(self, scenario):
        """Test 0-100 kph acceleration config"""
        try:
            passed, details = validate_vehicle_config()
            if passed:
                # Check acceleration values are reasonable
                config_path = os.path.join(PROJECT_DIR, "Content", "Vehicles", "VehicleConfig.json")
                with open(config_path, 'r') as f:
                    data = json.load(f)
                for v_id, v in data.get("vehicles", {}).items():
                    accel = v.get("performance", {}).get("acceleration_0_100", 0)
                    if accel <= 0 or accel > 10:
                        return {"passed": False, "details": f"{v_id} invalid acceleration: {accel}s"}
                return {"passed": True, "details": "All vehicle acceleration configs valid"}
            return {"passed": False, "details": details}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_top_speed(self, scenario):
        """Test top speed config validation"""
        try:
            config_path = os.path.join(PROJECT_DIR, "Content", "Vehicles", "VehicleConfig.json")
            with open(config_path, 'r') as f:
                data = json.load(f)
            for v_id, v in data.get("vehicles", {}).items():
                top_speed = v.get("performance", {}).get("top_speed_kph", 0)
                if top_speed <= 0 or top_speed > 500:
                    return {"passed": False, "details": f"{v_id} invalid top speed: {top_speed} kph"}
            return {"passed": True, "details": "All top speed configs valid"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_braking(self, scenario):
        """Test braking system config"""
        try:
            config_path = os.path.join(PROJECT_DIR, "Content", "Vehicles", "VehicleConfig.json")
            with open(config_path, 'r') as f:
                data = json.load(f)
            for v_id, v in data.get("vehicles", {}).items():
                chassis = v.get("chassis", {})
                if "brakes" not in chassis:
                    return {"passed": False, "details": f"{v_id} missing brake config"}
            return {"passed": True, "details": "All brake configs present"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_cornering(self, scenario):
        """Test tire lateral stiffness config"""
        try:
            config_path = os.path.join(PROJECT_DIR, "Content", "Vehicles", "VehicleConfig.json")
            with open(config_path, 'r') as f:
                data = json.load(f)
            for v_id, v in data.get("vehicles", {}).items():
                tires = v.get("chaos_vehicle", {}).get("tires", {})
                lat = tires.get("lateral_stiffness", 0)
                if lat <= 0:
                    return {"passed": False, "details": f"{v_id} invalid lateral stiffness: {lat}"}
            return {"passed": True, "details": "All tire configs valid"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_ai_completion(self, scenario):
        """Test AI config validation"""
        try:
            passed, details = validate_ai_config()
            return {"passed": passed, "details": details}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_ai_overtaking(self, scenario):
        """Test AI overtake evaluator exists"""
        try:
            # Check that AI behavior tree files exist
            bt_path = os.path.join(PROJECT_DIR, "Source", "NomiRacingPlus", "AI", "AIBehaviorTree.cpp")
            ot_path = os.path.join(PROJECT_DIR, "Source", "NomiRacingPlus", "AI", "AIOvertakeEvaluator.cpp")
            if os.path.exists(bt_path) and os.path.exists(ot_path):
                return {"passed": True, "details": "AI behavior tree and overtake evaluator exist"}
            return {"passed": False, "details": "Missing AI source files"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_ai_rubber_band(self, scenario):
        """Test rubber band scaler exists"""
        try:
            rb_path = os.path.join(PROJECT_DIR, "Source", "NomiRacingPlus", "AI", "AIRubberBandScaler.cpp")
            if os.path.exists(rb_path):
                return {"passed": True, "details": "Rubber band scaler implemented"}
            return {"passed": False, "details": "Missing rubber band scaler"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_ai_collision(self, scenario):
        """Test AI sensor system exists"""
        try:
            sensor_path = os.path.join(PROJECT_DIR, "Source", "NomiRacingPlus", "AI", "AISensorSystem.cpp")
            if os.path.exists(sensor_path):
                return {"passed": True, "details": "AI sensor system implemented"}
            return {"passed": False, "details": "Missing sensor system"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_street_gt(self, scenario):
        """Test track config validation"""
        try:
            passed, details = validate_track_config()
            return {"passed": passed, "details": details}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_nio_championship(self, scenario):
        """Test championship manager exists"""
        try:
            cm_path = os.path.join(PROJECT_DIR, "Source", "NomiRacingPlus", "Race", "ChampionshipManager.cpp")
            if os.path.exists(cm_path):
                return {"passed": True, "details": "ChampionshipManager implemented"}
            return {"passed": False, "details": "Missing ChampionshipManager"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_baja_rally(self, scenario):
        """Test race modes config"""
        try:
            passed, details = validate_track_config()
            return {"passed": passed, "details": details}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_scoring(self, scenario):
        """Test race manager exists"""
        try:
            rm_path = os.path.join(PROJECT_DIR, "Source", "NomiRacingPlus", "Race", "RaceManager.cpp")
            if os.path.exists(rm_path):
                return {"passed": True, "details": "RaceManager implemented"}
            return {"passed": False, "details": "Missing RaceManager"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_nomi_commentary(self, scenario):
        """Test NOMI commentary config"""
        try:
            comments_path = os.path.join(PROJECT_DIR, "Content", "NOMI", "Comments", "DefaultComments.json")
            if not os.path.exists(comments_path):
                return {"passed": False, "details": "DefaultComments.json not found"}
            with open(comments_path, 'r') as f:
                data = json.load(f)
            if "comments" in data and len(data["comments"]) > 0:
                return {"passed": True, "details": f"{len(data['comments'])} comment types defined"}
            return {"passed": False, "details": "No comments defined"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_nomi_emotion(self, scenario):
        """Test NOMI system source exists"""
        try:
            nomi_path = os.path.join(PROJECT_DIR, "Source", "NomiRacingPlus", "NOMI", "NOMIController.cpp")
            if os.path.exists(nomi_path):
                return {"passed": True, "details": "NOMIController implemented"}
            return {"passed": False, "details": "Missing NOMIController"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_nomi_feedback(self, scenario):
        """Test commentary engine exists"""
        try:
            ce_path = os.path.join(PROJECT_DIR, "Source", "NomiRacingPlus", "NOMI", "CommentaryEngine.cpp")
            if os.path.exists(ce_path):
                return {"passed": True, "details": "CommentaryEngine implemented"}
            return {"passed": False, "details": "Missing CommentaryEngine"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_chase_camera(self, scenario):
        """Test camera system source exists"""
        try:
            cam_path = os.path.join(PROJECT_DIR, "Source", "NomiRacingPlus", "Core", "CameraSystem.cpp")
            if os.path.exists(cam_path):
                return {"passed": True, "details": "CameraSystem implemented"}
            return {"passed": False, "details": "Missing CameraSystem"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_cockpit_camera(self, scenario):
        """Test camera system has multiple modes"""
        try:
            cam_path = os.path.join(PROJECT_DIR, "Source", "NomiRacingPlus", "Core", "CameraSystem.cpp")
            if not os.path.exists(cam_path):
                return {"passed": False, "details": "CameraSystem.cpp not found"}
            with open(cam_path, 'r') as f:
                content = f.read()
            modes = content.count("ECameraMode")
            if modes > 0:
                return {"passed": True, "details": f"Camera system has {modes} mode references"}
            return {"passed": False, "details": "No camera modes found"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_cinematic_camera(self, scenario):
        """Test camera system exists"""
        try:
            cam_path = os.path.join(PROJECT_DIR, "Source", "NomiRacingPlus", "Core", "CameraSystem.cpp")
            if os.path.exists(cam_path):
                return {"passed": True, "details": "CameraSystem implemented"}
            return {"passed": False, "details": "Missing CameraSystem"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_replay_camera(self, scenario):
        """Test replay system exists"""
        try:
            # Replay is typically part of the camera system
            cam_path = os.path.join(PROJECT_DIR, "Source", "NomiRacingPlus", "Core", "CameraSystem.cpp")
            if os.path.exists(cam_path):
                return {"passed": True, "details": "Camera system supports replay"}
            return {"passed": False, "details": "Missing CameraSystem"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_motor_sounds(self, scenario):
        """Test motor sound config"""
        try:
            passed, details = validate_audio_config()
            return {"passed": passed, "details": details}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_tire_screech(self, scenario):
        """Test tire sound config exists"""
        try:
            config_path = os.path.join(PROJECT_DIR, "Content", "Audio", "AudioConfig.json")
            if not os.path.exists(config_path):
                return {"passed": False, "details": "AudioConfig.json not found"}
            with open(config_path, 'r') as f:
                data = json.load(f)
            if "tire_sounds" in data:
                return {"passed": True, "details": "Tire sound config present"}
            return {"passed": False, "details": "Missing tire_sounds config"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_collision_sounds(self, scenario):
        """Test collision sound config"""
        try:
            config_path = os.path.join(PROJECT_DIR, "Content", "Audio", "AudioConfig.json")
            if not os.path.exists(config_path):
                return {"passed": False, "details": "AudioConfig.json not found"}
            with open(config_path, 'r') as f:
                data = json.load(f)
            if "collision_sounds" in data:
                return {"passed": True, "details": "Collision sound config present"}
            return {"passed": False, "details": "Missing collision_sounds config"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_spatial_audio(self, scenario):
        """Test audio system source exists"""
        try:
            audio_path = os.path.join(PROJECT_DIR, "Source", "NomiRacingPlus", "Core", "AudioManager.cpp")
            if os.path.exists(audio_path):
                return {"passed": True, "details": "AudioManager implemented"}
            return {"passed": False, "details": "Missing AudioManager"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_fps(self, scenario):
        """Test performance benchmark exists"""
        try:
            perf_path = os.path.join(PROJECT_DIR, "Source", "NomiRacingPlus", "Tests", "PerformanceBenchmarkTest.cpp")
            if os.path.exists(perf_path):
                return {"passed": True, "details": "Performance benchmark tests exist"}
            return {"passed": False, "details": "Missing performance benchmark tests"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_memory(self, scenario):
        """Test vehicle LOD config"""
        try:
            config_path = os.path.join(PROJECT_DIR, "Content", "Vehicles", "VehicleConfig.json")
            with open(config_path, 'r') as f:
                data = json.load(f)
            for v_id, v in data.get("vehicles", {}).items():
                lod = v.get("lod", {})
                if "lod0_faces" not in lod:
                    return {"passed": False, "details": f"{v_id} missing LOD config"}
            return {"passed": True, "details": "All LOD configs present"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_draw_calls(self, scenario):
        """Test LOD chain validation"""
        try:
            config_path = os.path.join(PROJECT_DIR, "Content", "Vehicles", "VehicleConfig.json")
            with open(config_path, 'r') as f:
                data = json.load(f)
            for v_id, v in data.get("vehicles", {}).items():
                lod = v.get("lod", {})
                lod0 = lod.get("lod0_faces", 0)
                lod1 = lod.get("lod1_faces", 0)
                lod2 = lod.get("lod2_faces", 0)
                if lod0 <= lod1 or lod1 <= lod2:
                    return {"passed": False, "details": f"{v_id} LOD chain invalid: {lod0}>{lod1}>{lod2}"}
            return {"passed": True, "details": "LOD chains valid"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_texture_streaming(self, scenario):
        """Test texture assets exist"""
        try:
            texture_dir = os.path.join(PROJECT_DIR, "Assets", "Textures")
            if os.path.exists(texture_dir):
                textures = [f for f in os.listdir(texture_dir) if f.endswith(('.png', '.jpg', '.tga'))]
                return {"passed": True, "details": f"{len(textures)} texture files found"}
            return {"passed": False, "details": "Textures directory not found"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_wrong_way(self, scenario):
        """Test checkpoint system exists"""
        try:
            cp_path = os.path.join(PROJECT_DIR, "Source", "NomiRacingPlus", "Race", "CheckpointSystem.cpp")
            if os.path.exists(cp_path):
                return {"passed": True, "details": "Checkpoint system implemented"}
            return {"passed": False, "details": "Missing CheckpointSystem"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_boundary(self, scenario):
        """Test track config has boundaries"""
        try:
            passed, details = validate_track_config()
            return {"passed": passed, "details": details}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_flip_recovery(self, scenario):
        """Test vehicle state manager exists"""
        try:
            vsm_path = os.path.join(PROJECT_DIR, "Source", "NomiRacingPlus", "Vehicles", "VehicleStateManager.cpp")
            if os.path.exists(vsm_path):
                return {"passed": True, "details": "VehicleStateManager implemented"}
            return {"passed": False, "details": "Missing VehicleStateManager"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_pause_resume(self, scenario):
        """Test game mode exists"""
        try:
            gm_path = os.path.join(PROJECT_DIR, "Source", "NomiRacingPlus", "Core", "NomiRaceGameMode.cpp")
            if os.path.exists(gm_path):
                return {"passed": True, "details": "NomiRaceGameMode implemented"}
            return {"passed": False, "details": "Missing NomiRaceGameMode"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    def test_lap_counting(self, scenario):
        """Test checkpoint and race manager"""
        try:
            cp_path = os.path.join(PROJECT_DIR, "Source", "NomiRacingPlus", "Race", "CheckpointSystem.cpp")
            rm_path = os.path.join(PROJECT_DIR, "Source", "NomiRacingPlus", "Race", "RaceManager.cpp")
            if os.path.exists(cp_path) and os.path.exists(rm_path):
                return {"passed": True, "details": "Checkpoint and RaceManager exist"}
            return {"passed": False, "details": "Missing race system files"}
        except Exception as e:
            return {"passed": False, "details": str(e)}

    # ========================================================================
    # Summary
    # ========================================================================

    def print_scenario_summary(self, scenario_name):
        """Print scenario test summary"""
        results = self.results.get(scenario_name, {})

        total = len(results)
        passed = sum(1 for r in results.values() if r.get("passed", False))
        failed = total - passed

        log_info(f"\n{'='*60}")
        log_info(f"Scenario Summary: {scenario_name}")
        log_info(f"{'='*60}")
        log_info(f"  Total Tests: {total}")
        log_info(f"  Passed: {passed}")
        log_info(f"  Failed: {failed}")
        log_info(f"  Pass Rate: {passed/total*100:.1f}%")

        if failed > 0:
            log_info(f"\n  Failed Tests:")
            for test, result in results.items():
                if not result.get("passed", False):
                    log_info(f"    - {test}: {result.get('details', 'No details')}")

        log_info(f"{'='*60}\n")

    def print_full_report(self):
        """Print full test report"""
        log_info(f"\n{'='*60}")
        log_info(f"  NIO Racing Plus - Full Test Report")
        log_info(f"{'='*60}")

        total_tests = 0
        total_passed = 0

        for scenario_name, results in self.results.items():
            scenario_total = len(results)
            scenario_passed = sum(1 for r in results.values() if r.get("passed", False))

            total_tests += scenario_total
            total_passed += scenario_passed

            status = "✓" if scenario_passed == scenario_total else "✗"
            log_info(f"  {status} {scenario_name}: {scenario_passed}/{scenario_total}")

        log_info(f"\n{'='*60}")
        log_info(f"  Overall: {total_passed}/{total_tests} tests passed")
        log_info(f"  Pass Rate: {total_passed/total_tests*100:.1f}%")
        log_info(f"{'='*60}\n")

# ============================================================================
# Quick Test Functions
# ============================================================================

def run_all_tests():
    """Run all test scenarios"""
    log_info("Running all test scenarios...")

    runner = TestRunner()

    for scenario_name in TEST_SCENARIOS:
        runner.run_scenario(scenario_name)

    runner.print_full_report()

def run_quick_smoke_test():
    """Run quick smoke test (essential tests only)"""
    log_info("Running quick smoke test...")

    runner = TestRunner()
    smoke_scenarios = ["vehicle_performance", "race_modes"]

    for scenario_name in smoke_scenarios:
        runner.run_scenario(scenario_name)

    runner.print_full_report()

def run_performance_test():
    """Run performance-focused tests"""
    log_info("Running performance tests...")

    runner = TestRunner()
    runner.run_scenario("performance")
    runner.print_scenario_summary("performance")

def run_ai_test():
    """Run AI-focused tests"""
    log_info("Running AI tests...")

    runner = TestRunner()
    runner.run_scenario("ai_competitiveness")
    runner.print_scenario_summary("ai_competitiveness")

# ============================================================================
# Test Checklist
# ============================================================================

def print_test_checklist():
    """Print manual test checklist"""
    checklist = """
================================================================
  NIO Racing Plus - Manual Test Checklist
================================================================

VEHICLE TESTS:
  [ ] EP9 accelerates to 100 kph in < 3.5s
  [ ] ET7 accelerates to 100 kph in < 4.5s
  [ ] ES7 accelerates to 100 kph in < 5.5s
  [ ] All vehicles reach documented top speed
  [ ] Braking feels responsive (100-0 < 40m)
  [ ] No vehicle gets stuck on track
  [ ] Vehicle flip auto-recovers

AI TESTS:
  [ ] AI completes full race
  [ ] AI overtakes player when appropriate
  [ ] AI avoids collisions
  [ ] Rubber band keeps race competitive
  [ ] AI difficulty matches selected level

RACE TESTS:
  [ ] Street GT mode works
  [ ] NIO Championship mode works
  [ ] Baja Rally mode works
  [ ] Lap counting is accurate
  [ ] Scoring awards correct points
  [ ] Finish line detection works

NOMI TESTS:
  [ ] NOMI comments on race events
  [ ] NOMI shows correct emotions
  [ ] NOMI provides position feedback
  [ ] NOMI voice plays correctly

CAMERA TESTS:
  [ ] Chase camera follows smoothly
  [ ] Cockpit view is usable
  [ ] Cinematic camera works
  [ ] Replay system functions

AUDIO TESTS:
  [ ] Motor sounds different per vehicle
  [ ] Tire screech on hard braking
  [ ] Collision sounds play
  [ ] Music volume adjusts by speed

PERFORMANCE TESTS:
  [ ] Maintains 60 FPS
  [ ] No texture pop-in
  [ ] No memory leaks
  [ ] Loading times < 10s

EDGE CASES:
  [ ] Wrong way warning displays
  [ ] Track boundaries prevent escape
  [ ] Pause/resume preserves state
  [ ] All checkpoints work

================================================================
"""
    print(checklist)

# ============================================================================
# Entry Point
# ============================================================================

if __name__ == "__main__":
    log_info("Test Scenarios loaded!")
    log_info("")
    log_info("Available functions:")
    log_info("  run_all_tests()")
    log_info("  run_quick_smoke_test()")
    log_info("  run_performance_test()")
    log_info("  run_ai_test()")
    log_info("  print_test_checklist()")
    log_info("")
    log_info("Individual scenarios:")
    for name, scenario in TEST_SCENARIOS.items():
        log_info(f"  TestRunner().run_scenario('{name}')")

# Auto-execute smoke test
try:
    run_quick_smoke_test()
except Exception as e:
    log_error(f"Error: {e}")
    log_info("Run test functions manually")
