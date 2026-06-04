"""
NIO Racing Plus - Track Builder
Creates racing tracks with landscapes, road splines, checkpoints, and environment props
Run from UE5 Editor: exec(open('Scripts/Editor/track_builder.py').read())
"""

import unreal
import math
import json
import os

# ============================================================================
# Configuration
# ============================================================================

PROJECT_DIR = unreal.Paths.project_dir()
CONTENT_DIR = unreal.Paths.project_content_dir()
TRACK_CONFIG_PATH = os.path.join(CONTENT_DIR, "Maps", "TrackConfig.json")

# Track dimensions
TRACK_WIDTH = 1200.0  # 12 meters in cm
BARRIER_HEIGHT = 100.0
CHECKPOINT_WIDTH = 1500.0
CHECKPOINT_HEIGHT = 500.0

# ============================================================================
# Utility Functions
# ============================================================================

def log_info(message):
    unreal.log(f"[TrackBuilder] {message}")

def log_error(message):
    unreal.log_error(f"[TrackBuilder] {message}")

def load_track_config():
    """Load track configuration from JSON, normalising dict → list if needed."""
    try:
        with open(TRACK_CONFIG_PATH, 'r') as f:
            config = json.load(f)

        # JSON stores tracks as dict keyed by ID, but the builder expects a
        # list so we can iterate/next().  Normalise inline.
        if isinstance(config.get("tracks"), dict):
            config["tracks"] = list(config["tracks"].values())

        return config
    except Exception as e:
        log_error(f"Failed to load track config: {e}")
        return None

def create_actor_from_class(actor_class, location, rotation, name):
    """Create an actor in the world"""
    world = unreal.EditorLevelLibrary.get_editor_world()
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        actor_class,
        location,
        rotation
    )
    if actor:
        actor.set_actor_label(name)
    return actor

def create_static_mesh_actor(mesh_path, location, rotation, name, scale=(1, 1, 1)):
    """Create a static mesh actor"""
    mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
    if not mesh:
        log_error(f"Failed to load mesh: {mesh_path}")
        return None

    actor = create_actor_from_class(
        unreal.StaticMeshActor,
        location,
        rotation,
        name
    )

    if actor:
        component = actor.static_mesh_component
        component.set_static_mesh(mesh)
        actor.set_actor_scale3d(unreal.Vector(*scale))

    return actor

def create_box_trigger(location, extent, name):
    """Create a box trigger volume.

    NOTE: UE5.7 Python API doesn't expose GetComponents() on TriggerBox,
    and Box is a Component not an Actor class.  We spawn a plain Actor as a
    placeholder marker.  Collision triggers for checkpoints must be added
    via Blueprint or C++.
    """
    actor = create_actor_from_class(
        unreal.Actor,
        location,
        unreal.Rotator(0, 0, 0),
        name
    )
    return actor

# ============================================================================
# Track Generation
# ============================================================================

class TrackBuilder:
    def __init__(self, track_name, track_config):
        self.track_name = track_name
        self.config = track_config
        self.track_actors = []
        self.waypoints = []
        self.checkpoints = []

    def build_track(self):
        """Build complete track"""
        log_info(f"Building track: {self.track_name}")

        # Clear existing level
        self.clear_level()

        # Create track components
        self.create_landscape()
        self.create_road_spline()
        self.create_barriers()
        self.create_checkpoints()
        self.create_start_finish()
        self.create_ai_waypoints()
        self.create_environment_props()
        self.create_lighting()
        self.create_nio_branding()

        log_info(f"Track {self.track_name} built successfully!")
        return True

    def clear_level(self):
        """Clear all actors from level"""
        actors = unreal.EditorLevelLibrary.get_all_level_actors()
        for actor in actors:
            # Keep essential actors
            if not isinstance(actor, (unreal.WorldSettings, unreal.GameModeBase)):
                unreal.EditorLevelLibrary.destroy_actor(actor)
        log_info("Level cleared")

    def create_landscape(self):
        """Create landscape terrain"""
        log_info("Creating landscape...")

        landscape_size = self.config.get("landscape_size", 8192)
        landscape_scale = self.config.get("landscape_scale", 100)

        # Create landscape using heightmap
        # For now, create a flat landscape
        landscape_location = unreal.Vector(0, 0, 0)
        landscape_rotation = unreal.Rotator(0, 0, 0)

        # Landscape creation requires editor scripting
        # This will be done through UE5's landscape tool
        log_info("Landscape creation requires UE5 Editor manual steps")
        log_info("  1. Open Landscape mode (Shift+2)")
        log_info("  2. Set Section Size: 63x63")
        log_info("  3. Set Overall Components: 8x8")
        log_info("  4. Click 'Create'")

        # Create landscape material
        self.create_landscape_material()

    def create_landscape_material(self):
        """Create landscape material with multiple layers"""
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

        # Create master landscape material
        material_path = "/Game/Materials/M_Landscape"
        material = asset_tools.create_asset(
            "M_Landscape",
            "/Game/Materials",
            unreal.Material,
            unreal.MaterialFactoryNew()
        )

        if material:
            # Add texture samples for different surface types
            # This creates a blendable landscape material
            log_info("Created landscape material: M_Landscape")

    def create_road_spline(self):
        """Create road spline path and road meshes.

        Note: UE5.7 removed SplineActor and the Python API does not expose
        add_component() or add_instance_component() on Actor.  We skip runtime
        SplineComponent creation and instead store point data on a simple Actor
        as tags for identification.  The actual road meshes are built by
        create_road_mesh_from_spline() from the raw point data.
        """
        log_info("Creating road spline...")

        road_points = self.config.get("road_points", [])
        if not road_points:
            road_points = self.generate_default_track_points()

        # Create a plain Actor to mark the spline path location in the world.
        # The road meshes are built separately from the point data below.
        spline_actor = create_actor_from_class(
            unreal.Actor,
            unreal.Vector(0, 0, 50),
            unreal.Rotator(0, 0, 0),
            f"{self.track_name}_RoadSpline"
        )

        if spline_actor:
            self.track_actors.append(spline_actor)

        # Build road mesh segments from the raw point data
        self.create_road_mesh_from_spline(road_points)

        log_info(f"Created road spline with {len(road_points)} points")

    def generate_default_track_points(self):
        """Generate default oval track points"""
        points = []
        radius = 20000  # 200m radius

        for i in range(36):
            angle = (i / 36) * 2 * math.pi
            x = radius * math.cos(angle)
            y = radius * math.sin(angle)
            z = 50  # Slight elevation
            points.append({"x": x, "y": y, "z": z})

        return points

    def create_road_mesh_from_spline(self, road_points):
        """Create road mesh along spline points"""
        # Create road segments between points
        for i in range(len(road_points) - 1):
            p1 = road_points[i]
            p2 = road_points[i + 1]

            # Calculate road segment position and rotation
            mid_x = (p1["x"] + p2["x"]) / 2
            mid_y = (p1["y"] + p2["y"]) / 2
            mid_z = (p1["z"] + p2["z"]) / 2

            dx = p2["x"] - p1["x"]
            dy = p2["y"] - p1["y"]
            angle = math.degrees(math.atan2(dy, dx))

            location = unreal.Vector(mid_x, mid_y, mid_z)
            rotation = unreal.Rotator(0, angle, 0)

            # Calculate segment length
            length = math.sqrt(dx*dx + dy*dy)

            # Create road mesh actor
            road_actor = create_static_mesh_actor(
                "/Engine/BasicShapes/Plane",
                location,
                rotation,
                f"Road_Segment_{i}",
                scale=(length / 100, TRACK_WIDTH / 100, 1)
            )

            if road_actor:
                self.track_actors.append(road_actor)

    def create_barriers(self):
        """Create track barriers"""
        log_info("Creating track barriers...")

        road_points = self.config.get("road_points", self.generate_default_track_points())

        # Create barriers on both sides of the track
        for i in range(len(road_points) - 1):
            p1 = road_points[i]
            p2 = road_points[i + 1]

            # Calculate perpendicular direction
            dx = p2["x"] - p1["x"]
            dy = p2["y"] - p1["y"]
            length = math.sqrt(dx*dx + dy*dy)

            if length == 0:
                continue

            # Normalized perpendicular
            nx = -dy / length
            ny = dx / length

            # Barrier offset from track center
            offset = TRACK_WIDTH / 2 + 50  # 50cm from track edge

            for side in [-1, 1]:
                # Left barrier
                barrier_x = (p1["x"] + p2["x"]) / 2 + nx * offset * side
                barrier_y = (p1["y"] + p2["y"]) / 2 + ny * offset * side
                barrier_z = p1["z"] + BARRIER_HEIGHT / 2

                location = unreal.Vector(barrier_x, barrier_y, barrier_z)
                angle = math.degrees(math.atan2(dy, dx))
                rotation = unreal.Rotator(0, angle, 0)

                barrier_actor = create_static_mesh_actor(
                    "/Engine/BasicShapes/Cube",
                    location,
                    rotation,
                    f"Barrier_{'L' if side == -1 else 'R'}_{i}",
                    scale=(length / 100, 0.5, BARRIER_HEIGHT / 100)
                )

                if barrier_actor:
                    self.track_actors.append(barrier_actor)

        log_info("Barriers created")

    def create_checkpoints(self):
        """Create checkpoint triggers"""
        log_info("Creating checkpoints...")

        road_points = self.config.get("road_points", self.generate_default_track_points())
        num_checkpoints = self.config.get("num_checkpoints", 5)

        # Distribute checkpoints evenly around track
        step = max(1, len(road_points) // num_checkpoints)

        for i in range(0, len(road_points) - 1, step):
            point = road_points[i]

            checkpoint_actor = create_box_trigger(
                unreal.Vector(point["x"], point["y"], point["z"] + CHECKPOINT_HEIGHT / 2),
                unreal.Vector(CHECKPOINT_WIDTH / 2, TRACK_WIDTH / 2, CHECKPOINT_HEIGHT / 2),
                f"Checkpoint_{len(self.checkpoints)}"
            )

            if checkpoint_actor:
                self.checkpoints.append(checkpoint_actor)
                self.track_actors.append(checkpoint_actor)

        log_info(f"Created {len(self.checkpoints)} checkpoints")

    def create_start_finish(self):
        """Create start/finish line"""
        log_info("Creating start/finish line...")

        road_points = self.config.get("road_points", self.generate_default_track_points())
        if not road_points:
            return

        start_point = road_points[0]

        # Create start/finish line visual
        sf_location = unreal.Vector(start_point["x"], start_point["y"], start_point["z"] + 5)
        sf_actor = create_static_mesh_actor(
            "/Engine/BasicShapes/Plane",
            sf_location,
            unreal.Rotator(0, 0, 0),
            "StartFinish_Line",
            scale=(TRACK_WIDTH / 100, 2, 1)
        )

        if sf_actor:
            self.track_actors.append(sf_actor)

        # Create start grid positions
        for i in range(8):  # 8 grid positions
            grid_x = start_point["x"] - (i // 2) * 500  # 5m spacing
            grid_y = start_point["y"] + (i % 2) * 200 - 100  # Alternating sides

            grid_location = unreal.Vector(grid_x, grid_y, start_point["z"])
            grid_actor = create_static_mesh_actor(
                "/Engine/BasicShapes/Plane",
                grid_location,
                unreal.Rotator(0, 0, 0),
                f"StartGrid_{i}",
                scale=(3, 6, 1)
            )

            if grid_actor:
                self.track_actors.append(grid_actor)

        log_info("Start/finish line created")

    def create_ai_waypoints(self):
        """Create AI waypoint path"""
        log_info("Creating AI waypoints...")

        road_points = self.config.get("road_points", self.generate_default_track_points())

        for i, point in enumerate(road_points):
            waypoint_location = unreal.Vector(point["x"], point["y"], point["z"] + 100)

            # Create waypoint marker (small sphere)
            waypoint_actor = create_static_mesh_actor(
                "/Engine/BasicShapes/Sphere",
                waypoint_location,
                unreal.Rotator(0, 0, 0),
                f"AI_Waypoint_{i}",
                scale=(0.5, 0.5, 0.5)
            )

            if waypoint_actor:
                # Hide in game, show in editor
                waypoint_actor.set_actor_hidden_in_game(True)
                self.waypoints.append(waypoint_actor)
                self.track_actors.append(waypoint_actor)

        log_info(f"Created {len(self.waypoints)} AI waypoints")

    def create_environment_props(self):
        """Create environment props (trees, buildings, etc.)"""
        log_info("Creating environment props...")

        track_type = self.config.get("type", "street")

        if track_type == "city":
            self.create_city_props()
        elif track_type == "mountain":
            self.create_mountain_props()
        elif track_type == "desert":
            self.create_desert_props()
        elif track_type == "oval":
            self.create_stadium_props()
        else:
            self.create_default_props()

    def create_city_props(self):
        """Create city environment props"""
        # Create buildings along the track
        road_points = self.config.get("road_points", self.generate_default_track_points())

        for i in range(0, len(road_points), 3):
            point = road_points[i]

            for side in [-1, 1]:
                # Calculate perpendicular position
                next_idx = (i + 1) % len(road_points)
                dx = road_points[next_idx]["x"] - point["x"]
                dy = road_points[next_idx]["y"] - point["y"]
                length = math.sqrt(dx*dx + dy*dy)

                if length == 0:
                    continue

                nx = -dy / length
                ny = dx / length

                building_offset = TRACK_WIDTH + 500 + (i % 3) * 200
                bx = point["x"] + nx * building_offset * side
                by = point["y"] + ny * building_offset * side

                # Random building height
                height = 1000 + (i * 137) % 2000

                building_actor = create_static_mesh_actor(
                    "/Engine/BasicShapes/Cube",
                    unreal.Vector(bx, by, height / 2),
                    unreal.Rotator(0, (i * 17) % 360, 0),
                    f"Building_{i}_{side}",
                    scale=(200, 200, height)
                )

                if building_actor:
                    self.track_actors.append(building_actor)

        log_info("City props created")

    def create_mountain_props(self):
        """Create mountain environment props"""
        # Create rocks and trees
        for i in range(50):
            x = (i * 347) % 40000 - 20000
            y = (i * 571) % 40000 - 20000
            z = 0

            rock_actor = create_static_mesh_actor(
                "/Engine/BasicShapes/Cube",
                unreal.Vector(x, y, z),
                unreal.Rotator(0, (i * 37) % 360, 0),
                f"Rock_{i}",
                scale=(100 + (i % 5) * 50, 100 + (i % 7) * 50, 50 + (i % 3) * 30)
            )

            if rock_actor:
                self.track_actors.append(rock_actor)

        log_info("Mountain props created")

    def create_desert_props(self):
        """Create desert environment props"""
        # Create sand dunes and cacti
        for i in range(30):
            x = (i * 457) % 60000 - 30000
            y = (i * 673) % 60000 - 30000

            dune_actor = create_static_mesh_actor(
                "/Engine/BasicShapes/Sphere",
                unreal.Vector(x, y, 0),
                unreal.Rotator(0, (i * 23) % 360, 0),
                f"Dune_{i}",
                scale=(500 + (i % 5) * 200, 500 + (i % 7) * 200, 100)
            )

            if dune_actor:
                self.track_actors.append(dune_actor)

        log_info("Desert props created")

    def create_stadium_props(self):
        """Create stadium/oval environment props"""
        # Create grandstands
        for i in range(8):
            angle = (i / 8) * 2 * math.pi
            radius = 25000
            x = radius * math.cos(angle)
            y = radius * math.sin(angle)

            stand_actor = create_static_mesh_actor(
                "/Engine/BasicShapes/Cube",
                unreal.Vector(x, y, 500),
                unreal.Rotator(0, math.degrees(angle), 0),
                f"Grandstand_{i}",
                scale=(500, 100, 1000)
            )

            if stand_actor:
                self.track_actors.append(stand_actor)

        log_info("Stadium props created")

    def create_default_props(self):
        """Create default environment props"""
        log_info("Using default environment props")
        self.create_city_props()

    def create_lighting(self):
        """Create track lighting"""
        log_info("Creating lighting...")

        lighting_config = self.config.get("lighting", {})

        # Create directional light (sun/moon)
        sun_location = unreal.Vector(0, 0, 10000)
        sun_actor = create_actor_from_class(
            unreal.DirectionalLight,
            sun_location,
            unreal.Rotator(-45, -45, 0),
            "DirectionalLight_Sun"
        )

        if sun_actor:
            # UE5.7 Python component property names vary; guard defensively
            if hasattr(sun_actor, 'directional_light_component'):
                sun_actor.directional_light_component.set_intensity(
                    lighting_config.get("sun_intensity", 10.0)
                )
            self.track_actors.append(sun_actor)

        # Create sky light
        sky_actor = create_actor_from_class(
            unreal.SkyLight,
            unreal.Vector(0, 0, 5000),
            unreal.Rotator(0, 0, 0),
            "SkyLight"
        )
        if sky_actor:
            self.track_actors.append(sky_actor)

        # Create fog
        fog_actor = create_actor_from_class(
            unreal.ExponentialHeightFog,
            unreal.Vector(0, 0, 0),
            unreal.Rotator(0, 0, 0),
            "HeightFog"
        )
        if fog_actor:
            self.track_actors.append(fog_actor)

        log_info("Lighting created")

    def create_nio_branding(self):
        """Create NIO branding elements"""
        log_info("Creating NIO branding...")

        road_points = self.config.get("road_points", self.generate_default_track_points())

        # Create NIO billboards along track
        for i in range(0, len(road_points), 5):
            point = road_points[i]

            billboard_actor = create_static_mesh_actor(
                "/Engine/BasicShapes/Plane",
                unreal.Vector(point["x"], point["y"], point["z"] + 300),
                unreal.Rotator(0, 0, 0),
                f"NIO_Billboard_{i}",
                scale=(5, 3, 1)
            )

            if billboard_actor:
                self.track_actors.append(billboard_actor)

        # Create NIO swap station
        if len(road_points) > 10:
            station_point = road_points[len(road_points) // 2]
            station_actor = create_static_mesh_actor(
                "/Engine/BasicShapes/Cube",
                unreal.Vector(station_point["x"], station_point["y"], station_point["z"] + 200),
                unreal.Rotator(0, 0, 0),
                "NIO_SwapStation",
                scale=(20, 10, 4)
            )

            if station_actor:
                self.track_actors.append(station_actor)

        log_info("NIO branding created")

# ============================================================================
# Main Track Creation Functions
# ============================================================================

def _create_track_level(track_id: str) -> bool:
    """Create or open a level for the given track ID."""
    level_path = f"/Game/Maps/{track_id}"
    try:
        if unreal.EditorAssetLibrary.does_asset_exist(level_path):
            unreal.EditorLevelLibrary.load_level(level_path)
        else:
            unreal.EditorLevelLibrary.new_level(level_path)
        return True
    except Exception as e:
        log_error(f"Failed to create/load level {level_path}: {e}")
        return False


def _save_track_level(track_id: str) -> None:
    """Save the current level for the given track."""
    level_path = f"/Game/Maps/{track_id}"
    try:
        unreal.EditorAssetLibrary.save_asset(level_path, only_if_is_dirty=True)
        log_info(f"Saved level: {level_path}")
    except Exception as e:
        log_error(f"Failed to save level {level_path}: {e}")


def create_nio_city_circuit():
    _create_track("NIOCityCircuit", "NIO City Circuit")

def _create_track(name_id: str, display_name: str):
    """Generic helper to create a track by name."""
    config = load_track_config()
    if not config:
        return
    track_config = next((t for t in config["tracks"] if t["name"] == display_name), None)
    if not track_config:
        log_error(f"Track config not found: {display_name}")
        return
    if not _create_track_level(name_id):
        return
    builder = TrackBuilder(name_id, track_config)
    builder.build_track()
    _save_track_level(name_id)

def create_shanghai_pudong():
    _create_track("ShanghaiPudong", "Shanghai Pudong")

def create_speedway_oval():
    _create_track("SpeedwayOval", "Speedway Oval")

def create_mountain_pass():
    _create_track("MountainPass", "Mountain Pass")

def create_desert_rally():
    _create_track("DesertRally", "Desert Rally")

def create_all_tracks():
    """Create all tracks"""
    log_info("Creating all tracks...")
    create_nio_city_circuit()
    create_speedway_oval()
    create_shanghai_pudong()
    create_mountain_pass()
    create_desert_rally()
    log_info("All tracks created!")

def create_main_menu_level():
    """Create main menu level"""
    log_info("Creating main menu level...")

    if not _create_track_level("MainMenu"):
        return

    # Create simple level with camera position
    menu_config = {
        "type": "menu",
        "road_points": [],
        "lighting": {
            "sun_intensity": 5.0
        }
    }

    builder = TrackBuilder("MainMenu", menu_config)
    builder.clear_level()
    builder.create_lighting()

    # Create camera actor for menu
    camera_actor = create_actor_from_class(
        unreal.CineCameraActor,
        unreal.Vector(0, -500, 300),
        unreal.Rotator(-15, 0, 0),
        "MenuCamera"
    )

    if camera_actor:
        builder.track_actors.append(camera_actor)

    _save_track_level("MainMenu")
    log_info("Main menu level created")

# ============================================================================
# Entry Point
# ============================================================================

if __name__ == "__main__":
    log_info("Track Builder — auto-running create_all_tracks()...")
    try:
        create_all_tracks()
    except Exception as e:
        log_error(f"Error: {e}")
        log_info("Run individual track creation functions manually")
