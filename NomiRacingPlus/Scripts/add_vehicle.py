#!/usr/bin/env python3
"""
NIO Racing Plus - Add Custom Vehicle from Sketchfab
One-command vehicle import pipeline

Usage:
    python3 add_vehicle.py <sketchfab_url> [--color R G B] [--name "Custom Name"]

Examples:
    python3 add_vehicle.py https://sketchfab.com/3d-models/porsche-911-gt3-rs-2023-abc123
    python3 add_vehicle.py https://sketchfab.com/3d-models/tesla-model-s-xyz789 --color 0.8 0.1 0.1 --name "Red Tesla"
    python3 add_vehicle.py https://sketchfab.com/3d-models/ferrari-488-def456 --api-token YOUR_TOKEN
"""

import argparse
import json
import os
import re
import struct
import sys
import urllib.request
import urllib.error
from pathlib import Path

# ============================================================================
# Configuration
# ============================================================================

PROJECT_DIR = Path(__file__).parent.parent
CONTENT_DIR = PROJECT_DIR / "Content"
VEHICLES_DIR = CONTENT_DIR / "Vehicles"
SCRIPTS_DIR = PROJECT_DIR / "Scripts"

# Default colors for quick selection
PRESET_COLORS = {
    "blue":     (0.0, 0.2, 0.8),
    "red":      (0.8, 0.05, 0.05),
    "green":    (0.05, 0.6, 0.15),
    "black":    (0.02, 0.02, 0.02),
    "white":    (0.9, 0.9, 0.9),
    "silver":   (0.7, 0.7, 0.75),
    "gold":     (0.85, 0.7, 0.2),
    "orange":   (0.9, 0.4, 0.05),
    "purple":   (0.5, 0.1, 0.7),
    "sky_blue": (0.4, 0.7, 1.0),
    "cyan":     (0.0, 0.8, 0.8),
    "pink":     (0.9, 0.3, 0.5),
    "matte_black": (0.05, 0.05, 0.05),  # Slightly lighter than gloss black
    "champagne": (0.85, 0.75, 0.6),
}

# ============================================================================
# Utility Functions
# ============================================================================

def log_info(msg):
    print(f"\033[32m[INFO]\033[0m {msg}")

def log_step(msg):
    print(f"\033[34m[STEP]\033[0m {msg}")

def log_warn(msg):
    print(f"\033[33m[WARN]\033[0m {msg}")

def log_error(msg):
    print(f"\033[31m[ERROR]\033[0m {msg}")

def log_success(msg):
    print(f"\033[32m[OK]\033[0m {msg}")

def extract_uid_from_url(url):
    """Extract model UID from Sketchfab URL"""
    # Handle various URL formats (stop at ? or # for query params)
    patterns = [
        r'sketchfab\.com/3d-models/.*-([a-f0-9]{32})(?:[?#]|$)',
        r'sketchfab\.com/3d-models/([a-f0-9]{32})(?:[?#]|$)',
        r'models/([a-f0-9]{32})(?:[?#]|$)',
    ]

    for pattern in patterns:
        match = re.search(pattern, url)
        if match:
            return match.group(1)

    # Check if it's already a UID
    if re.match(r'^[a-f0-9]{32}$', url):
        return url

    return None

def api_request(endpoint, token=None):
    """Make Sketchfab API request with size limits"""
    url = f"https://api.sketchfab.com/v3{endpoint}"
    headers = {"User-Agent": "NIO-Racing-Plus/1.0"}
    if token:
        headers["Authorization"] = f"Token {token}"

    req = urllib.request.Request(url, headers=headers)
    try:
        with urllib.request.urlopen(req, timeout=30) as resp:
            # Limit response size to 10MB for API calls
            content = resp.read(10 * 1024 * 1024)
            return json.loads(content)
    except urllib.error.URLError as e:
        log_error(f"API request failed: {e.reason}")
        return None
    except json.JSONDecodeError as e:
        log_error(f"Invalid JSON response: {e}")
        return None
    except Exception as e:
        log_error(f"API request failed: {e}")
        return None

def download_file(url, output_path, desc="Downloading", max_size_mb=500):
    """Download file with progress and size limit"""
    req = urllib.request.Request(url, headers={"User-Agent": "NIO-Racing-Plus/1.0"})
    max_bytes = max_size_mb * 1024 * 1024

    try:
        with urllib.request.urlopen(req, timeout=600) as resp:
            total = int(resp.headers.get('Content-Length', 0))

            # Check file size before downloading
            if total > max_bytes:
                log_error(f"File too large ({total // (1024*1024)} MB > {max_size_mb} MB limit)")
                return None

            downloaded = 0
            block_size = 1024 * 1024  # 1MB

            with open(output_path, 'wb') as f:
                while True:
                    chunk = resp.read(block_size)
                    if not chunk:
                        break
                    f.write(chunk)
                    downloaded += len(chunk)

                    # Safety check during download
                    if downloaded > max_bytes:
                        log_error(f"Download exceeded size limit ({max_size_mb} MB)")
                        f.close()
                        os.remove(output_path)
                        return None

                    if total > 0:
                        pct = downloaded * 100 / total
                        mb_down = downloaded // (1024 * 1024)
                        mb_total = total // (1024 * 1024)
                        print(f"\r  {desc}: {pct:.1f}% ({mb_down}/{mb_total} MB)", end="", flush=True)

            print()
            return downloaded
    except urllib.error.URLError as e:
        log_error(f"Download failed: {e.reason}")
        return None
    except Exception as e:
        log_error(f"Download failed: {e}")
        return None

# ============================================================================
# Model Analysis
# ============================================================================

class SketchfabModel:
    """Sketchfab model data and operations"""

    def __init__(self, url, token=None):
        self.url = url
        self.token = token or os.environ.get("SKETCHFAB_TOKEN", "")
        self.uid = None
        self.info = None
        self.name = ""
        self.faces = 0
        self.textures = 0
        self.materials = 0
        self.downloadable = False
        self.license = ""

    def analyze(self):
        """Analyze model from URL"""
        log_step("Analyzing model...")

        # Extract UID
        self.uid = extract_uid_from_url(self.url)
        if not self.uid:
            log_error(f"Cannot extract model UID from URL: {self.url}")
            return False

        # Get model info
        self.info = api_request(f"/models/{self.uid}")
        if not self.info:
            log_error("Failed to get model info")
            return False

        self.name = self.info.get("name", "Unknown")
        self.faces = self.info.get("faceCount", 0)
        self.textures = self.info.get("textureCount", 0)
        self.materials = self.info.get("materialCount", 0)
        self.downloadable = self.info.get("isDownloadable", False)
        self.license = self.info.get("license", {}).get("label", "Unknown")

        # Print analysis
        print()
        print(f"  ┌─────────────────────────────────────────┐")
        print(f"  │ Model Analysis                          │")
        print(f"  ├─────────────────────────────────────────┤")
        print(f"  │ Name:       {self.name:<28}│")
        print(f"  │ Faces:      {self.faces:<28,}│")
        print(f"  │ Textures:   {self.textures:<28}│")
        print(f"  │ Materials:  {self.materials:<28}│")
        print(f"  │ License:    {self.license:<28}│")
        print(f"  │ Downloadable: {'Yes' if self.downloadable else 'No':<26}│")
        print(f"  └─────────────────────────────────────────┘")
        print()

        if not self.downloadable:
            log_error("Model is not downloadable")
            return False

        # Quality assessment
        if self.faces > 2000000:
            log_warn(f"High poly count ({self.faces:,}) - may need LOD reduction")
        elif self.faces < 10000:
            log_warn(f"Low poly count ({self.faces:,}) - may look basic")

        if self.textures == 0:
            log_warn("No textures - will need manual material painting")

        return True

    def download(self, output_dir):
        """Download GLB file"""
        if not self.token:
            log_error("API token required for download")
            log_info("Get token from: https://sketchfab.com/settings/password")
            log_info("Set via: --api-token TOKEN or SKETCHFAB_TOKEN env var")
            return None

        log_step("Getting download URL...")

        # Get download URL
        data = api_request(f"/models/{self.uid}/download", self.token)
        if not data:
            log_error("Failed to get download URL")
            return None

        glb_url = data.get("glb", {}).get("url")
        if not glb_url:
            log_error("No GLB download available")
            return None

        # Download
        output_path = os.path.join(output_dir, "source.glb")
        log_step(f"Downloading {self.name}...")
        size = download_file(glb_url, output_path, "Downloading")

        log_success(f"Downloaded: {size // (1024*1024)} MB")
        return output_path

# ============================================================================
# GLB Processing
# ============================================================================

class GLBProcessor:
    """Process GLB files for game integration"""

    def __init__(self, filepath):
        self.filepath = filepath
        self.gltf = None
        self.bin_data = None

    def read(self):
        """Read and parse GLB file"""
        with open(self.filepath, 'rb') as f:
            magic = f.read(4)
            if magic != b'glTF':
                log_error("Not a valid GLB file")
                return False

            version = struct.unpack('<I', f.read(4))[0]
            length = struct.unpack('<I', f.read(4))[0]

            while f.tell() < length:
                chunk_length = struct.unpack('<I', f.read(4))[0]
                chunk_type = f.read(4)
                chunk_data = f.read(chunk_length)

                if chunk_type == b'JSON':
                    self.gltf = json.loads(chunk_data.decode('utf-8'))
                elif chunk_type == b'BIN\x00':
                    self.bin_data = chunk_data

        return self.gltf is not None

    def write(self, output_path):
        """Write modified GLB file"""
        json_data = json.dumps(self.gltf, separators=(',', ':')).encode('utf-8')

        # Pad JSON to 4-byte alignment
        while len(json_data) % 4 != 0:
            json_data += b' '

        bin_data = self.bin_data or b''
        while len(bin_data) % 4 != 0:
            bin_data += b'\x00'

        total_length = 12 + 8 + len(json_data)
        if bin_data:
            total_length += 8 + len(bin_data)

        with open(output_path, 'wb') as f:
            f.write(b'glTF')
            f.write(struct.pack('<I', 2))
            f.write(struct.pack('<I', total_length))
            f.write(struct.pack('<I', len(json_data)))
            f.write(b'JSON')
            f.write(json_data)

            if bin_data:
                f.write(struct.pack('<I', len(bin_data)))
                f.write(b'BIN\x00')
                f.write(bin_data)

    def paint(self, body_color=(0.4, 0.7, 1.0), metallic=0.7, roughness=0.15):
        """Apply paint color to vehicle body"""
        log_step("Applying paint color...")

        # Create new materials
        materials = [
            {
                "name": "M_Body_Paint",
                "pbrMetallicRoughness": {
                    "baseColorFactor": [*body_color, 1.0],
                    "metallicFactor": metallic,
                    "roughnessFactor": roughness
                },
                "alphaMode": "OPAQUE"
            },
            {
                "name": "M_Glass",
                "pbrMetallicRoughness": {
                    "baseColorFactor": [0.1, 0.1, 0.12, 0.4],
                    "metallicFactor": 0.0,
                    "roughnessFactor": 0.05
                },
                "alphaMode": "BLEND",
                "doubleSided": True
            },
            {
                "name": "M_Chrome",
                "pbrMetallicRoughness": {
                    "baseColorFactor": [0.8, 0.8, 0.85, 1.0],
                    "metallicFactor": 1.0,
                    "roughnessFactor": 0.1
                },
                "alphaMode": "OPAQUE"
            },
            {
                "name": "M_Tire",
                "pbrMetallicRoughness": {
                    "baseColorFactor": [0.05, 0.05, 0.05, 1.0],
                    "metallicFactor": 0.0,
                    "roughnessFactor": 0.9
                },
                "alphaMode": "OPAQUE"
            },
            {
                "name": "M_Light_Front",
                "pbrMetallicRoughness": {
                    "baseColorFactor": [1.0, 1.0, 1.0, 1.0],
                    "metallicFactor": 0.0,
                    "roughnessFactor": 0.1
                },
                "emissiveFactor": [1.0, 0.95, 0.8],
                "alphaMode": "OPAQUE"
            },
            {
                "name": "M_Light_Rear",
                "pbrMetallicRoughness": {
                    "baseColorFactor": [0.8, 0.0, 0.0, 1.0],
                    "metallicFactor": 0.0,
                    "roughnessFactor": 0.2
                },
                "emissiveFactor": [1.0, 0.0, 0.0],
                "alphaMode": "OPAQUE"
            }
        ]

        # Map original materials to new ones
        material_mapping = {}
        old_materials = self.gltf.get("materials", [])

        for i, mat in enumerate(old_materials):
            name = mat.get("name", "").lower()
            if any(w in name for w in ["glass", "window", "transparent", "clear"]):
                material_mapping[i] = 1
            elif any(w in name for w in ["chrome", "metal", "aluminum", "wheel", "rim"]):
                material_mapping[i] = 2
            elif any(w in name for w in ["tire", "rubber", "tyre"]):
                material_mapping[i] = 3
            elif any(w in name for w in ["headlight", "light_front", "lamp"]):
                material_mapping[i] = 4
            elif any(w in name for w in ["taillight", "light_rear", "brake", "red"]):
                material_mapping[i] = 5
            else:
                material_mapping[i] = 0  # Default to body paint

        # Apply mapping to meshes
        for mesh in self.gltf.get("meshes", []):
            for prim in mesh.get("primitives", []):
                old_mat = prim.get("material", 0)
                prim["material"] = material_mapping.get(old_mat, 0)

        self.gltf["materials"] = materials
        log_success(f"Painted with {len(materials)} materials")

    def get_stats(self):
        """Get model statistics"""
        if not self.gltf:
            return {"faces": 0, "materials": 0, "textures": 0}

        faces = 0
        for m in self.gltf.get("meshes", []):
            prims = m.get("primitives") or [{}]
            if prims and "indices" in prims[0]:
                faces += len(prims[0]["indices"]) // 3

        return {
            "faces": faces,
            "materials": len(self.gltf.get("materials", [])),
            "textures": len(self.gltf.get("textures", [])),
        }

# ============================================================================
# Game Configuration Generator
# ============================================================================

class VehicleConfigGenerator:
    """Generate all game configuration files for a new vehicle"""

    def __init__(self, vehicle_id, display_name, vehicle_type="sedan"):
        self.vehicle_id = vehicle_id
        self.display_name = display_name
        self.vehicle_type = vehicle_type
        self.config_dir = CONTENT_DIR / "Vehicles"
        self.audio_path = CONTENT_DIR / "Audio" / "AudioConfig.json"
        self.tags_path = PROJECT_DIR / "Config" / "DefaultGameplayTags.ini"

    def estimate_specs(self, faces):
        """Estimate vehicle specifications based on poly count"""
        # Rough estimation for gameplay balance
        if faces > 1000000:
            return {
                "mass_kg": 1600,
                "power_kw": 800,
                "power_hp": 1088,
                "torque_nm": 1200,
                "top_speed_kph": 320,
                "acceleration_0_100": 2.5,
                "type": "hypercar"
            }
        elif faces > 500000:
            return {
                "mass_kg": 1900,
                "power_kw": 500,
                "power_hp": 680,
                "torque_nm": 850,
                "top_speed_kph": 280,
                "acceleration_0_100": 3.5,
                "type": "sports_car"
            }
        else:
            return {
                "mass_kg": 2100,
                "power_kw": 350,
                "power_hp": 476,
                "torque_nm": 600,
                "top_speed_kph": 250,
                "acceleration_0_100": 4.5,
                "type": "sedan"
            }

    def generate_physics_json(self, specs):
        """Generate physics configuration JSON"""
        log_step("Generating physics configuration...")

        physics = {
            "vehicle_id": self.vehicle_id,
            "vehicle_name": self.display_name,
            "vehicle_type": specs["type"],
            "mass": {
                "total_kg": specs["mass_kg"],
                "distribution": {"front_percent": 48.0, "rear_percent": 52.0}
            },
            "engine": {
                "type": "electric_dual_motor",
                "max_rpm": 18000,
                "torque_curve": [
                    {"rpm": 0, "normalized_torque": 1.00},
                    {"rpm": 5000, "normalized_torque": 1.00},
                    {"rpm": 10000, "normalized_torque": 0.90},
                    {"rpm": 15000, "normalized_torque": 0.60},
                    {"rpm": 18000, "normalized_torque": 0.00}
                ],
                "motor_details": {
                    "total_power_kw": specs["power_kw"],
                    "total_power_hp": specs["power_hp"],
                    "total_torque_nm": specs["torque_nm"]
                }
            },
            "transmission": {
                "type": "single_speed_reduction",
                "final_drive_ratio": 9.0
            },
            "performance_targets": {
                "zero_to_100_kph_sec": specs["acceleration_0_100"],
                "top_speed_kph": specs["top_speed_kph"]
            }
        }

        output_path = self.config_dir / f"{self.vehicle_id}_Physics.json"
        with open(output_path, 'w') as f:
            json.dump(physics, f, indent=4)

        log_success(f"Physics config: {output_path.name}")
        return physics

    def generate_vehicle_class(self):
        """Generate C++ vehicle class files"""
        log_step("Generating vehicle class...")

        class_name = f"A{self.vehicle_id}"

        # Header file
        header = f"""// Copyright NomiRacingPlus Project. All Rights Reserved.
// Auto-generated by add_vehicle.py

#pragma once

#include "CoreMinimal.h"
#include "Vehicles/NIOVehicleBase.h"
#include "{class_name}.generated.h"

/**
 * {self.display_name}
 * Custom vehicle added via Sketchfab import
 */
UCLASS(Blueprintable)
class NOMIRACINGPLUS_API {class_name} : public ANIOVehicleBase
{{
\tGENERATED_BODY()

public:
\t{class_name}();
}};
"""

        # Implementation file
        impl = f"""// Copyright NomiRacingPlus Project. All Rights Reserved.
// Auto-generated by add_vehicle.py

#include "Vehicles/{class_name}.h"

{class_name}::{class_name}()
{{
\tVehicleType = ENIOVehicleType::Custom;
}}
"""

        header_path = PROJECT_DIR / "Source" / "NomiRacingPlus" / "Vehicles" / f"{class_name}.h"
        impl_path = PROJECT_DIR / "Source" / "NomiRacingPlus" / "Vehicles" / f"{class_name}.cpp"

        with open(header_path, 'w') as f:
            f.write(header)
        with open(impl_path, 'w') as f:
            f.write(impl)

        log_success(f"Vehicle class: {class_name}")
        return header_path, impl_path

    def update_vehicle_config(self, specs):
        """Update VehicleConfig.json"""
        log_step("Updating vehicle config...")

        config_path = self.config_dir / "VehicleConfig.json"
        with open(config_path) as f:
            config = json.load(f)

        config["vehicles"][self.vehicle_id] = {
            "name": self.display_name,
            "type": specs["type"],
            "performance": {
                "mass_kg": specs["mass_kg"],
                "power_kw": specs["power_kw"],
                "power_hp": specs["power_hp"],
                "torque_nm": specs["torque_nm"],
                "top_speed_kph": specs["top_speed_kph"],
                "acceleration_0_100": specs["acceleration_0_100"]
            },
            "electric": {
                "battery_capacity_kwh": 80,
                "motor_count": 2,
                "regen_strength": 0.4
            }
        }

        with open(config_path, 'w') as f:
            json.dump(config, f, indent=4)

        log_success("VehicleConfig.json updated")

    def update_audio_config(self):
        """Add motor sound profile to AudioConfig.json"""
        log_step("Adding audio profile...")

        with open(self.audio_path) as f:
            config = json.load(f)

        # Add motor sound profile
        config["audio_config"]["motor_sound_profiles"][self.vehicle_id] = {
            "vehicle_id": self.vehicle_id,
            "vehicle_name": self.display_name,
            "vehicle_type": "custom",
            "motor_count": 2,
            "max_rpm": 18000,
            "layers": {
                "motor_whine": {
                    "sound_asset": f"Motor_{self.vehicle_id}_Whine",
                    "pitch_curve": [
                        {"rpm": 0, "pitch": 0.55},
                        {"rpm": 9000, "pitch": 1.00},
                        {"rpm": 18000, "pitch": 1.80}
                    ],
                    "volume_curve": [
                        {"rpm": 0, "volume": 0.10},
                        {"rpm": 9000, "volume": 0.80},
                        {"rpm": 18000, "volume": 0.60}
                    ]
                }
            }
        }

        # Add horn
        config["audio_config"]["sfx"]["horn"]["sounds"][self.vehicle_id] = f"Horn_{self.vehicle_id}"

        with open(self.audio_path, 'w') as f:
            json.dump(config, f, indent=4)

        log_success("AudioConfig.json updated")

    def update_gameplay_tags(self):
        """Add GameplayTag"""
        log_step("Adding gameplay tag...")

        with open(self.tags_path) as f:
            content = f.read()

        tag_line = f'+GameplayTagList=(Tag="Vehicle.Custom.{self.vehicle_id}",DevComment="{self.display_name}")'

        if tag_line not in content:
            # Add before last line
            content = content.rstrip() + "\n" + tag_line + "\n"
            with open(self.tags_path, 'w') as f:
                f.write(content)

        log_success(f"GameplayTag: Vehicle.Custom.{self.vehicle_id}")

    def generate_motor_sound(self):
        """Generate placeholder motor sound"""
        log_step("Generating motor sound...")

        try:
            import numpy as np
            import wave

            output_path = CONTENT_DIR / "Audio" / "Motor" / f"{self.vehicle_id}_motor_loop.wav"
            output_path.parent.mkdir(parents=True, exist_ok=True)

            sample_rate = 44100
            duration = 4.0
            num_samples = int(sample_rate * duration)
            t = np.linspace(0, duration, num_samples, endpoint=False)

            freq = np.linspace(150, 1200, num_samples)
            phase = 2 * np.pi * np.cumsum(freq) / sample_rate
            signal = 0.4 * np.sin(phase) + 0.2 * np.sin(2*phase) + 0.1 * np.sin(3*phase)
            signal += np.random.normal(0, 0.02, num_samples)
            signal = signal / np.max(np.abs(signal)) * 0.8

            with wave.open(str(output_path), 'w') as wav:
                wav.setnchannels(1)
                wav.setsampwidth(2)
                wav.setframerate(sample_rate)
                wav.writeframes(np.int16(signal * 32767).tobytes())

            log_success(f"Motor sound: {output_path.name}")
        except ImportError:
            log_warn("numpy not available, skipping motor sound generation")

# ============================================================================
# Main Pipeline
# ============================================================================

def add_vehicle(url, color=None, name=None, api_token=None):
    """Main pipeline to add a vehicle from Sketchfab"""

    print()
    print("=" * 50)
    print("  NIO Racing Plus - Add Custom Vehicle")
    print("=" * 50)
    print()

    # Step 1: Analyze model
    model = SketchfabModel(url, api_token)
    if not model.analyze():
        return False

    # Generate vehicle ID from name
    if not name:
        name = model.name

    # Clean name for ID (remove special chars, keep alphanumeric)
    vehicle_id = re.sub(r'[^a-zA-Z0-9]', '', name.title().replace(" ", ""))
    vehicle_id = vehicle_id[:20]  # Limit length

    # Check for ID collision and add suffix if needed
    existing_dir = VEHICLES_DIR / vehicle_id
    if existing_dir.exists():
        suffix = 1
        while existing_dir.exists():
            suffix += 1
            existing_dir = VEHICLES_DIR / f"{vehicle_id}{suffix}"
        vehicle_id = f"{vehicle_id}{suffix}"
        log_warn(f"Vehicle ID collision, using: {vehicle_id}")

    log_info(f"Vehicle ID: {vehicle_id}")
    log_info(f"Display Name: {name}")

    # Create output directory
    output_dir = VEHICLES_DIR / vehicle_id
    output_dir.mkdir(parents=True, exist_ok=True)

    # Step 2: Download model
    glb_path = model.download(str(output_dir))
    if not glb_path:
        return False

    # Step 3: Process GLB
    processor = GLBProcessor(glb_path)
    if not processor.read():
        return False

    # Apply color if specified or if no textures
    if model.textures == 0 or color:
        if color:
            if isinstance(color, str) and color in PRESET_COLORS:
                rgb = PRESET_COLORS[color]
            else:
                rgb = color
        else:
            rgb = (0.4, 0.7, 1.0)  # Default sky blue

        processor.paint(body_color=rgb)

        # Save painted version
        painted_path = output_dir / f"{vehicle_id}.glb"
        processor.write(str(painted_path))

        # Replace original
        os.replace(str(painted_path), str(output_dir / "source.glb"))
        log_success("Model painted and saved")

    # Step 4: Generate configurations
    generator = VehicleConfigGenerator(vehicle_id, name)

    specs = generator.estimate_specs(model.faces)
    generator.generate_physics_json(specs)
    generator.generate_vehicle_class()
    generator.update_vehicle_config(specs)
    generator.update_audio_config()
    generator.update_gameplay_tags()
    generator.generate_motor_sound()

    # Step 5: Summary
    class_name = f"A{vehicle_id}"
    print()
    print("=" * 50)
    print("  Vehicle Added Successfully!")
    print("=" * 50)
    print()
    print(f"  Name:      {name}")
    print(f"  ID:        {vehicle_id}")
    print(f"  Faces:     {model.faces:,}")
    print(f"  Power:     {specs['power_hp']} HP")
    print(f"  Top Speed: {specs['top_speed_kph']} kph")
    print(f"  0-100:     {specs['acceleration_0_100']}s")
    print()
    print("  Files created:")
    print(f"    - Content/Vehicles/{vehicle_id}/source.glb")
    print(f"    - Content/Vehicles/{vehicle_id}_Physics.json")
    print(f"    - Content/Audio/Motor/{vehicle_id}_motor_loop.wav")
    print(f"    - Source/.../Vehicles/{class_name}.h")
    print(f"    - Source/.../Vehicles/{class_name}.cpp")
    print()
    print("  Config updated:")
    print(f"    - VehicleConfig.json (+{vehicle_id})")
    print(f"    - AudioConfig.json (+{vehicle_id})")
    print(f"    - DefaultGameplayTags.ini (+Vehicle.Custom.{vehicle_id})")
    print()
    print("  Next steps:")
    print(f"    1. Build project in UE5")
    print(f"    2. Run: exec(open('Scripts/Editor/run_full_setup.py').read())")
    print(f"    3. Select {name} in vehicle selection")
    print()
    print("=" * 50)

    return True

# ============================================================================
# CLI Entry Point
# ============================================================================

def main():
    parser = argparse.ArgumentParser(
        description="Add a custom vehicle from Sketchfab to NIO Racing Plus",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s https://sketchfab.com/3d-models/porsche-911-abc123
  %(prog)s https://sketchfab.com/3d-models/ferrari-488-def456 --color red
  %(prog)s https://sketchfab.com/3d-models/tesla-model-s-xyz789 --color 0.8 0.1 0.1
  %(prog)s https://sketchfab.com/3d-models/lambo-huracan-ghi012 --name "Lamborghini Huracan"

Color presets:
  blue, red, green, black, white, silver, gold, orange, purple,
  sky_blue, cyan, pink, matte_black, champagne

Or specify RGB values (0.0-1.0):
  --color 0.4 0.7 1.0  (sky blue)
  --color 0.8 0.05 0.05  (rosso corsa)
        """
    )

    parser.add_argument("url", help="Sketchfab model URL or UID")
    parser.add_argument("--color", nargs="+", help="Color preset name or RGB values (0.0-1.0)")
    parser.add_argument("--name", help="Custom display name for the vehicle")
    parser.add_argument("--api-token", help="Sketchfab API token (or set SKETCHFAB_TOKEN env var)")

    args = parser.parse_args()

    # Parse color
    color = None
    if args.color:
        if len(args.color) == 1 and args.color[0] in PRESET_COLORS:
            color = args.color[0]
        elif len(args.color) == 3:
            try:
                color = tuple(float(c) for c in args.color)
            except ValueError:
                log_error("Invalid RGB values")
                sys.exit(1)
        else:
            log_error(f"Unknown color: {args.color[0]}")
            sys.exit(1)

    # Run pipeline
    success = add_vehicle(args.url, color, args.name, args.api_token)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
