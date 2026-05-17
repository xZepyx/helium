# Config

Helium ships with a JSON-based config system. You define your config structure using plain Python classes — the nesting creates the JSON hierarchy automatically.

## Defining your config

Create classes for each section of your config. Nested classes become nested JSON objects, and attribute values become the defaults.

```python
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
        self.modules = BarModules()

class MyConfig:
    def __init__(self):
        self.bar = BarConfig()
        self.theme = "dark"
```

## Registering defaults

Use `merge_defaults()` with `helium.functions.config_from_class()` which recursively converts your class instance to a nested dict:

```python
config.merge_defaults(helium.functions.config_from_class(MyConfig()))
```

This sets your class structure as the config schema. Missing keys in the JSON file will be auto-filled with these defaults on every load.

## Path

Set the path to your config file:

```python
import os
config.set_path(os.path.expanduser("~/.config/helium/config.json"))
```

If the file doesn't exist it's created with your defaults. If it exists but is missing keys, the missing defaults are merged in and the file is rewritten automatically.

## Reading values

Use dot notation to traverse nested keys:

```python
theme = config.get("appearance.theme")
height = config.get("bar.height")
format = config.get("bar.modules.clock.format")
```

Provide a fallback for keys that may not exist:

```python
value = config.get("nonexistent.key", "fallback")
```

If no fallback is given, `None` is returned for missing keys.

## Writing values

```python
config.set("appearance.theme", "cream")
config.set("bar.height", 40)
config.set("bar.modules.clock.format", "%I:%M %p")
```

Writing a nested dict replaces the entire subtree:

```python
config.set("bar.modules", {
    "workspaces": {"enabled": True, "indicators": 6},
    "clock": {"enabled": True, "format": "%H:%M"},
})
```

Changes trigger an automatic save after a 200 ms debounce. You can force an immediate save:

```python
config.save()
```

## Other operations

Reload from disk (discards in-memory changes):

```python
config.reload()
```

List all top-level keys:

```python
keys = config.keys()
```

Check if the config file exists on disk:

```python
if config.exists():
    ...
```

View the path:

```python
path = config.path()
```

Access the full data as a Python dict:

```python
data = config.data
```

## Accessing config from other files

`helium.config` is a singleton — register defaults and set the path once (typically in your main file), then `import helium` from anywhere else to read or write:

```python
# bar.py — your bar module
import helium

class BarWidget:
    def __init__(self):
        self.height = helium.config.get("bar.height")
        self.position = helium.config.get("bar.position")
```

The config path and defaults only need to be set once. All subsequent imports of `helium.config` share the same state.

```python
# main.py — entry point
import helium
from bar import BarWidget

class BarConfig:
    def __init__(self):
        self.height = 34
        self.position = "top"

class MyConfig:
    def __init__(self):
        self.bar = BarConfig()

helium.config.merge_defaults(helium.functions.config_from_class(MyConfig()))
helium.config.set_path("~/.config/helium/config.json")

bar = BarWidget()
```

## Full example

```python
import os, json
import helium

class BarConfig:
    def __init__(self):
        self.height = 34
        self.position = "top"

class MyConfig:
    def __init__(self):
        self.bar = BarConfig()
        self.theme = "dark"

config = helium.config
config.merge_defaults(helium.functions.config_from_class(MyConfig()))

config.set_path(os.path.expanduser("~/.config/helium/config.json"))

config.set("bar.height", 40)
print(config.get("bar.height"))

config.save()
```

Run it with `PYTHONPATH=builddir python3 yourfile.py`.
