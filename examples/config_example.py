import os, json
import helium

config = helium.config

# Define your config structure with plain Python classes.
# Nesting creates the JSON hierarchy automatically.

class WorkspacesConfig:
    def __init__(self):
        self.enabled = True
        self.indicators = 8

class ClockConfig:
    def __init__(self):
        self.enabled = True
        self.format = "%H:%M"

class BarModules:
    def __init__(self):
        self.workspaces = WorkspacesConfig()
        self.clock = ClockConfig()

class BarConfig:
    def __init__(self):
        self.position = "top"
        self.enabled = True
        self.height = 34
        self.floating = False
        self.modules = BarModules()

class FontConfig:
    def __init__(self):
        self.scale = 1.0
        self.main = "JetBrains Mono"

class AppearanceConfig:
    def __init__(self):
        self.theme = "dark"
        self.font = FontConfig()

class MyConfig:
    def __init__(self):
        self.appearance = AppearanceConfig()
        self.bar = BarConfig()

# Convert the class hierarchy to a nested dict and register as defaults
config.merge_defaults(helium.functions.config_from_class(MyConfig()))

# Set the config file path (creates on first use, merges defaults on every load)
CONFIG_PATH = os.path.expanduser("~/.config/helium/config.json")
config.set_path(CONFIG_PATH)
print(f"Config path: {config.path()}")
print()

# Reading values (dot notation)
print(f"Theme: {config.get('appearance.theme')}")
print(f"Font scale: {config.get('appearance.font.scale')}")
print(f"Bar height: {config.get('bar.height')}")
print(f"Clock format: {config.get('bar.modules.clock.format')}")
print(f"Do not disturb: {config.get('notifications.dnd', False)}")
print()

# Writing values (also dot notation)
config.set("appearance.theme", "cream")
config.set("bar.height", 40)
config.set("bar.modules.clock.format", "%I:%M %p")
print("Values updated.")
print()

# Verify reads
print(f"Theme now: {config.get('appearance.theme')}")
print(f"Bar height now: {config.get('bar.height')}")
print()

# Force save to disk
config.save()
print(f"Config saved to {CONFIG_PATH}")
print()

# Reload from disk
config.set("bar.height", 999)
print(f"Before reload: {config.get('bar.height')}")
config.reload()
print(f"After reload:  {config.get('bar.height')}")
print()

# List top-level keys
print(f"Top-level keys: {config.keys()}")
print()

# Read the raw file
with open(CONFIG_PATH) as f:
    print("On-disk JSON:")
    print(json.dumps(json.load(f), indent=2))
