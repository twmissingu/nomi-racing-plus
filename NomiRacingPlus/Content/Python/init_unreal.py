"""
NIO Racing Plus - init_unreal.py
==================================
UE5.7 auto-executes this on editor startup if placed in Content/Python/.
"""

import unreal
unreal.log_warning("[NIOInit] init_unreal.py EXECUTED!")
unreal.log_warning("[NIOInit] Testing project-level Python startup execution.")

# Quick check: is AssetTools available?
try:
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    unreal.log_warning(f"[NIOInit] AssetTools available: {tools is not None}")
except Exception as e:
    unreal.log_warning(f"[NIOInit] AssetTools check: {e}")

unreal.log_warning("[NIOInit] init_unreal.py completed.")
