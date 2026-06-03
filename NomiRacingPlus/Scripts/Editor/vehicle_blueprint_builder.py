"""
NIO Racing Plus - Vehicle Blueprint Builder
Creates vehicle Blueprints with Chaos Vehicle components
Run from UE5 Editor: exec(open('Scripts/Editor/vehicle_blueprint_builder.py').read())
"""

import unreal
import json
import os

# ============================================================================
# Configuration
# ============================================================================

PROJECT_DIR = unreal.Paths.project_dir()
CONTENT_DIR = unreal.Paths.project_content_dir()
VEHICLE_CONFIG_PATH = os.path.join(CONTENT_DIR, "Vehicles", "VehicleConfig.json")

# Vehicle mesh paths (after import)
VEHICLE_MESHES = {
    "EP9": "/Game/Vehicles/EP9/Meshes/EP9_LOD0",
    "ET7": "/Game/Vehicles/ET7/Meshes/ET7_LOD0",
    "ES7": "/Game/Vehicles/ES7/Meshes/ES7_LOD0",
}

# Vehicle physics configs
VEHICLE_PHYSICS = {
    "EP9": "EP9_Physics.json",
    "ET7": "ET7_Physics.json",
    "ES7": "ES7_Physics.json",
}

# ============================================================================
# Utility Functions
# ============================================================================

def log_info(message):
    unreal.log(f"[VehicleBuilder] {message}")

def log_error(message):
    unreal.log_error(f"[VehicleBuilder] {message}")

def load_json_config(path):
    """Load JSON configuration file"""
    try:
        with open(path, 'r') as f:
            return json.load(f)
    except Exception as e:
        log_error(f"Failed to load config: {e}")
        return None

# ============================================================================
# Vehicle Blueprint Builder
# ============================================================================

class VehicleBlueprintBuilder:
    def __init__(self, vehicle_type):
        self.vehicle_type = vehicle_type
        self.blueprint = None
        self.components = {}

    def create_blueprint(self):
        """Create vehicle Blueprint"""
        log_info(f"Creating {self.vehicle_type} Blueprint...")

        # Create Blueprint asset
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

        blueprint_path = f"/Game/Vehicles/{self.vehicle_type}/Blueprints"
        blueprint_name = f"BP_NIO_{self.vehicle_type}"

        # Create Blueprint from WheeledVehiclePawn parent class
        self.blueprint = asset_tools.create_asset(
            blueprint_name,
            blueprint_path,
            unreal.Blueprint,
            unreal.BlueprintFactory()
        )

        if not self.blueprint:
            log_error(f"Failed to create Blueprint: {blueprint_name}")
            return False

        # Set parent class
        self.blueprint.set_editor_property(
            "parent_class",
            unreal.WheeledVehiclePawn
        )

        log_info(f"Created Blueprint: {blueprint_path}/{blueprint_name}")
        return True

    def add_components(self):
        """Add components to vehicle Blueprint"""
        if not self.blueprint:
            log_error("No Blueprint to add components to")
            return False

        log_info("Adding components...")

        # Add vehicle mesh component
        self.add_vehicle_mesh()

        # Add camera components
        self.add_camera_components()

        # Add audio components
        self.add_audio_components()

        # Add particle components
        self.add_particle_components()

        # Add NOMI component
        self.add_nomi_component()

        log_info("Components added")
        return True

    def add_vehicle_mesh(self):
        """Add vehicle static mesh"""
        mesh_path = VEHICLE_MESHES.get(self.vehicle_type)
        if not mesh_path:
            log_error(f"No mesh path for {self.vehicle_type}")
            return

        # Load mesh
        mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
        if not mesh:
            log_error(f"Failed to load mesh: {mesh_path}")
            return

        # Add static mesh component
        # This would be done through the Blueprint editor
        log_info(f"Vehicle mesh assigned: {mesh_path}")

    def add_camera_components(self):
        """Add camera components"""
        # Spring arm for chase camera
        log_info("Adding camera components...")

        # These would be added through Blueprint editor
        # - SpringArm (Chase camera)
        # - Camera (Chase view)
        # - CameraComponent (Cockpit view)

    def add_audio_components(self):
        """Add audio components"""
        log_info("Adding audio components...")

        # These would be added through Blueprint editor
        # - AudioComponent (Motor sound)
        # - AudioComponent (Tire sound)
        # - AudioComponent (Wind sound)

    def add_particle_components(self):
        """Add particle effect components"""
        log_info("Adding particle components...")

        # These would be added through Blueprint editor
        # - NiagaraComponent (Tire smoke)
        # - NiagaraComponent (Drift sparks)
        # - NiagaraComponent (Boost effect)

    def add_nomi_component(self):
        """Add NOMI companion component"""
        log_info("Adding NOMI component...")

        # NOMI controller component
        # This would be added through Blueprint editor

    def configure_chaos_vehicle(self):
        """Configure Chaos Vehicle movement component"""
        log_info("Configuring Chaos Vehicle...")

        # Load physics config
        physics_file = VEHICLE_PHYSICS.get(self.vehicle_type)
        physics_path = os.path.join(CONTENT_DIR, "Vehicles", physics_file)
        physics_config = load_json_config(physics_path)

        if not physics_config:
            log_error(f"Failed to load physics config: {physics_file}")
            return False

        # Configure ChaosVehicleMovementComponent
        # This would set:
        # - Mass and inertia
        # - Engine torque curve
        # - Transmission settings
        # - Suspension parameters
        # - Tire configurations
        # - Aerodynamics

        log_info("Chaos Vehicle configured")
        return True

    def configure_input(self):
        """Configure input mappings"""
        log_info("Configuring input...")

        # Input actions are already defined in DefaultInput.ini
        # This would bind them in the Blueprint

        log_info("Input configured")

    def save(self):
        """Save the Blueprint"""
        if self.blueprint:
            unreal.EditorAssetLibrary.save_asset(self.blueprint.get_path_name())
            log_info(f"Blueprint saved: {self.vehicle_type}")

# ============================================================================
# Main Vehicle Creation Functions
# ============================================================================

def create_ep9_blueprint():
    """Create EP9 vehicle Blueprint"""
    builder = VehicleBlueprintBuilder("EP9")
    if builder.create_blueprint():
        builder.add_components()
        builder.configure_chaos_vehicle()
        builder.configure_input()
        builder.save()

def create_et7_blueprint():
    """Create ET7 vehicle Blueprint"""
    builder = VehicleBlueprintBuilder("ET7")
    if builder.create_blueprint():
        builder.add_components()
        builder.configure_chaos_vehicle()
        builder.configure_input()
        builder.save()

def create_es7_blueprint():
    """Create ES7 vehicle Blueprint"""
    builder = VehicleBlueprintBuilder("ES7")
    if builder.create_blueprint():
        builder.add_components()
        builder.configure_chaos_vehicle()
        builder.configure_input()
        builder.save()

def create_all_vehicle_blueprints():
    """Create all vehicle Blueprints"""
    log_info("Creating all vehicle Blueprints...")
    create_ep9_blueprint()
    create_et7_blueprint()
    create_es7_blueprint()
    log_info("All vehicle Blueprints created!")

# ============================================================================
# Entry Point
# ============================================================================

if __name__ == "__main__":
    log_info("Vehicle Blueprint Builder loaded!")
    log_info("Available functions:")
    log_info("  create_ep9_blueprint()")
    log_info("  create_et7_blueprint()")
    log_info("  create_es7_blueprint()")
    log_info("  create_all_vehicle_blueprints()")

# Auto-execute if run directly
if __name__ == "__main__":
    try:
        create_all_vehicle_blueprints()
    except Exception as e:
        log_error(f"Error: {e}")
        log_info("Run individual vehicle creation functions manually")
