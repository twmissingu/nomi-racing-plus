"""
NIO Racing Plus - UE5 Editor Utility Scripts
=============================================
Collection of Python scripts that run inside the UE5 Editor to automate
repetitive asset pipeline tasks.

Modules:
    batch_import        - Batch import FBX/OBJ textures, audio, and HDR files
    lod_generation      - Auto-generate LOD levels for static and skeletal meshes
    material_setup      - Create PBR materials for vehicles, environment, and UI
    blueprint_compile   - Batch compile Blueprints with error reporting

Quick Start (UE5 Editor Output Log):
    # Import all modules
    exec(open(r"<PROJECT>/Scripts/Editor/batch_import.py").read())
    exec(open(r"<PROJECT>/Scripts/Editor/lod_generation.py").read())
    exec(open(r"<PROJECT>/Scripts/Editor/material_setup.py").read())
    exec(open(r"<PROJECT>/Scripts/Editor/blueprint_compile.py").read())

    # Or use the unified runner
    exec(open(r"<PROJECT>/Scripts/Editor/run_all.py").read())
"""
