"""
NIO Racing Plus - Auto Setup Test Level
Run in UE5 Editor Output Log: py setup_test_level.py
Or run from command line:
  UnrealEditor project.uproject -run=pythonscript -script="Content/Python/setup_test_level.py" -unattended -nullrhi
"""

import unreal
import math

MAP_PATH = "/Game/Maps/TestTrack"

# Number of checkpoints (must match RaceManager's CheckpointsPerLap default of 10)
NUM_CHECKPOINTS = 10
TRACK_RADIUS = 3000.0  # oval radius in cm


def setup_test_level():
    """Create a complete test level with all necessary actors."""
    unreal.log("=== NIO Racing Plus: Setting up Test Track ===")

    ell = unreal.EditorLevelLibrary
    eal = unreal.EditorAssetLibrary

    # 1. Create or open level
    if eal.does_asset_exist(MAP_PATH):
        unreal.log(f"Map {MAP_PATH} already exists, opening...")
        ell.load_level(MAP_PATH)
    else:
        unreal.log("Creating new empty level...")
        ell.new_level("/Game/Maps/TestTrack")

    # 2. Clear existing non-essential actors
    unreal.log("Clearing level...")
    for actor in ell.get_all_level_actors():
        keep = (unreal.WorldSettings,)
        if not isinstance(actor, keep):
            ell.destroy_actor(actor)

    # 3. Add ground plane
    unreal.log("Adding ground plane...")
    ground = ell.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, -50))
    if ground:
        smc = ground.static_mesh_component
        cube_mesh = unreal.load_asset("/Engine/BasicShapes/Cube")
        if cube_mesh:
            smc.set_static_mesh(cube_mesh)
        ground.set_actor_scale3d(unreal.Vector(100, 100, 0.1))
        ground.set_actor_label("Ground")

    # 4. Add road surface (circular track visual)
    unreal.log("Adding road surface...")
    cube_mesh = unreal.load_asset("/Engine/BasicShapes/Cube")
    for i in range(36):
        angle = (i / 36.0) * 2.0 * math.pi
        x = TRACK_RADIUS * math.cos(angle)
        y = TRACK_RADIUS * math.sin(angle)
        seg = ell.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(x, y, -40))
        if seg:
            seg.static_mesh_component.set_static_mesh(cube_mesh)
            seg.set_actor_scale3d(unreal.Vector(3, 1, 0.05))
            seg.set_actor_rotation(unreal.Rotator(0, math.degrees(angle), 0), False)
            seg.set_actor_label(f"Road_Segment_{i}")

    # 5. Add Player Start (facing +X toward checkpoint 0)
    unreal.log("Adding Player Start...")
    ps = ell.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(TRACK_RADIUS - 500, 0, 100))
    if ps:
        ps.set_actor_label("PlayerStart_Main")

    # 6. Add AI Spawn Points (behind player start)
    unreal.log("Adding AI spawn points...")
    spawn_positions = [
        unreal.Vector(TRACK_RADIUS - 500, -400, 100),
        unreal.Vector(TRACK_RADIUS - 500, -800, 100),
        unreal.Vector(TRACK_RADIUS - 500, -1200, 100),
        unreal.Vector(TRACK_RADIUS - 1200, 0, 100),
        unreal.Vector(TRACK_RADIUS - 1200, -400, 100),
        unreal.Vector(TRACK_RADIUS - 1200, -800, 100),
        unreal.Vector(TRACK_RADIUS - 1200, -1200, 100),
    ]
    for i, pos in enumerate(spawn_positions):
        sp = ell.spawn_actor_from_class(unreal.PlayerStart, pos)
        if sp:
            sp.set_actor_label(f"AI_Spawn_{i+1}")
            sp.tags = list(sp.tags) + ["AISpawn"]

    # 7. Add ACheckpoint actors in an oval around the track
    unreal.log("Adding checkpoints...")
    CheckpointClass = getattr(unreal, "Checkpoint", None)
    if CheckpointClass is None:
        unreal.log_warning("  ACheckpoint class not found in Python — checkpoints SKIPPED")
    else:
        for i in range(NUM_CHECKPOINTS):
            angle = (float(i) / NUM_CHECKPOINTS) * 2.0 * math.pi
            x = TRACK_RADIUS * math.cos(angle)
            y = TRACK_RADIUS * math.sin(angle)

            cp = ell.spawn_actor_from_class(CheckpointClass, unreal.Vector(x, y, 100))
            if cp:
                cp.set_actor_label(f"Checkpoint_{i}")
                cp.set_editor_property("CheckpointIndex", i)
                if i == 0:
                    cp_type = getattr(unreal.CheckpointType, "StartLine", None)
                elif i == NUM_CHECKPOINTS - 1:
                    cp_type = getattr(unreal.CheckpointType, "FinishLine", None)
                else:
                    cp_type = getattr(unreal.CheckpointType, "Regular", None)
                if cp_type:
                    cp.set_editor_property("CheckpointType", cp_type)
                unreal.log(f"  Placed checkpoint {i} at ({x:.0f}, {y:.0f}) type={cp_type}")

    # 8. Add track barriers
    unreal.log("Adding track barriers...")
    inner_radius = TRACK_RADIUS - 600
    outer_radius = TRACK_RADIUS + 600
    # Create 36 barrier segments on inner and outer edges
    for side_name, radius in [("Inner", inner_radius), ("Outer", outer_radius)]:
        for i in range(36):
            a1 = (i / 36.0) * 2.0 * math.pi
            a2 = ((i + 1) / 36.0) * 2.0 * math.pi
            mx = radius * (math.cos(a1) + math.cos(a2)) * 0.5
            my = radius * (math.sin(a1) + math.sin(a2)) * 0.5
            bv = ell.spawn_actor_from_class(unreal.BlockingVolume, unreal.Vector(mx, my, 150))
            if bv:
                bv.set_actor_scale3d(unreal.Vector(3, 1, 3))
                mid_angle = math.degrees((a1 + a2) * 0.5)
                bv.set_actor_rotation(unreal.Rotator(0, mid_angle, 0), False)
                bv.set_actor_label(f"Barrier_{side_name}_{i}")

    # 9. Add AI waypoints (visible in editor, hidden in game)
    unreal.log("Adding AI waypoints...")
    WaypointClass = getattr(unreal, "TargetPoint", None) or getattr(unreal, "Actor", None)
    num_wp = 20
    for i in range(num_wp):
        angle = (float(i) / num_wp) * 2.0 * math.pi
        x = TRACK_RADIUS * math.cos(angle)
        y = TRACK_RADIUS * math.sin(angle)
        wp = ell.spawn_actor_from_class(WaypointClass, unreal.Vector(x, y, 50))
        if wp:
            wp.set_actor_label(f"AI_Waypoint_{i}")
            wp.set_actor_hidden_in_game(True)

    # 10. Add lighting
    unreal.log("Adding lighting...")
    sun = ell.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 500))
    if sun:
        sun.set_actor_label("Sun")
        sun.set_actor_rotation(unreal.Rotator(-45, -45, 0), False)

    sky = ell.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 300))
    if sky:
        sky.set_actor_label("SkyLight")

    # 11. Save
    unreal.log("Saving level...")
    eal.save_asset(MAP_PATH, only_if_is_dirty=True)

    unreal.log("")
    unreal.log("========================================")
    unreal.log("  Test Track setup complete!")
    unreal.log(f"  Map: {MAP_PATH}")
    unreal.log("  - Oval road (36 segments)")
    unreal.log("  - 1 Player Start")
    unreal.log("  - 7 AI Spawn Points (tagged AISpawn)")
    unreal.log("  - 10 ACheckpoint actors (lap-wrapping)")
    unreal.log("  - 72 Barrier segments")
    unreal.log("  - 20 AI Waypoints")
    unreal.log("  - Sun + SkyLight")
    unreal.log("========================================")
    unreal.log("Press Play to test!")
    return True


if __name__ == "__main__":
    setup_test_level()
