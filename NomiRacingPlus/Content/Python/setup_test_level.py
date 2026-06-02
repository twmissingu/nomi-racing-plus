"""
NIO Racing Plus - Auto Setup Test Level
Run in UE5 Editor Output Log: py setup_test_level.py
Or run from command line:
  UnrealEditor project.uproject -run=pythonscript -script="Content/Python/setup_test_level.py" -unattended -nullrhi
"""

import unreal
import os

MAP_PATH = "/Game/Maps/TestTrack"


def setup_test_level():
    """Create a complete test level with all necessary actors."""
    unreal.log("=== NIO Racing Plus: Setting up Test Track ===")

    editor_level_lib = unreal.EditorLevelLibrary
    editor_asset_lib = unreal.EditorAssetLibrary

    # 1. Create or open level
    if editor_asset_lib.does_asset_exist(MAP_PATH):
        unreal.log(f"Map {MAP_PATH} already exists, opening...")
        editor_level_lib.load_level(MAP_PATH)
    else:
        unreal.log("Creating new empty level...")
        editor_level_lib.new_level("/Game/Maps/TestTrack")

    # 2. Add ground plane
    unreal.log("Adding ground plane...")
    ground = editor_level_lib.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, -50))
    if ground:
        smc = ground.static_mesh_component
        cube_mesh = unreal.load_asset("/Engine/BasicShapes/Cube")
        if cube_mesh:
            smc.set_static_mesh(cube_mesh)
        ground.set_actor_scale3d(unreal.Vector(100, 100, 0.1))
        ground.set_actor_label("Ground")

    # 3. Add Player Start
    unreal.log("Adding Player Start...")
    ps = editor_level_lib.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0, 0, 200))
    if ps:
        ps.set_actor_label("PlayerStart_Main")

    # 4. Add AI Spawn Points
    unreal.log("Adding AI spawn points...")
    spawn_positions = [
        unreal.Vector(0, -500, 200),
        unreal.Vector(0, -1000, 200),
        unreal.Vector(0, -1500, 200),
        unreal.Vector(500, 0, 200),
        unreal.Vector(500, -500, 200),
        unreal.Vector(500, -1000, 200),
        unreal.Vector(500, -1500, 200),
    ]
    for i, pos in enumerate(spawn_positions):
        sp = editor_level_lib.spawn_actor_from_class(unreal.PlayerStart, pos)
        if sp:
            sp.set_actor_label(f"AI_Spawn_{i+1}")
            sp.tags = list(sp.tags) + ["AISpawn"]

    # 5. Add track barriers
    unreal.log("Adding track barriers...")
    barriers = [
        (unreal.Vector(0, 5000, 200),    unreal.Vector(100, 1, 5)),
        (unreal.Vector(0, -5000, 200),   unreal.Vector(100, 1, 5)),
        (unreal.Vector(5000, 0, 200),    unreal.Vector(1, 100, 5)),
        (unreal.Vector(-5000, 0, 200),   unreal.Vector(1, 100, 5)),
    ]
    for i, (loc, scale) in enumerate(barriers):
        bv = editor_level_lib.spawn_actor_from_class(unreal.BlockingVolume, loc)
        if bv:
            bv.set_actor_scale3d(scale)
            bv.set_actor_label(f"TrackBarrier_{i+1}")

    # 6. Add lighting
    unreal.log("Adding lighting...")
    sun = editor_level_lib.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 500))
    if sun:
        sun.set_actor_label("Sun")
        # set_actor_rotation requires teleport_physics in UE5.7
        sun.set_actor_rotation(unreal.Rotator(-45, -45, 0), False)

    sky = editor_level_lib.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 300))
    if sky:
        sky.set_actor_label("SkyLight")

    # 7. Save
    unreal.log("Saving level...")
    unreal.EditorAssetLibrary.save_asset(MAP_PATH, only_if_is_dirty=True)

    unreal.log("")
    unreal.log("========================================")
    unreal.log("  Test Track setup complete!")
    unreal.log(f"  Map: {MAP_PATH}")
    unreal.log("  - Ground plane (100x100)")
    unreal.log("  - 1 Player Start")
    unreal.log(f"  - {len(spawn_positions)} AI Spawn Points")
    unreal.log("  - 4 Track Barriers")
    unreal.log("  - Sun + SkyLight")
    unreal.log("========================================")
    unreal.log("Press Play to test!")
    return True


if __name__ == "__main__":
    setup_test_level()
