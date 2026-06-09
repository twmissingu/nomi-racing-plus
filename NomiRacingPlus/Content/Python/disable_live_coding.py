# Run this in UE Editor's Python console (Window > Developer Tools > Python)
import unreal

# Disable Live Coding auto-compile on Play
settings = unreal.get_editor_settings()
# Toggle off Live Coding
unreal.SystemLibrary.execute_console_command(None, "LiveCoding.BlockOnRecompile 0")

print("Live Coding auto-recompile disabled. Restart editor for full effect.")
