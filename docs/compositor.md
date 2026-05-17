# Compositor — Hyprland

Functions for communicating with the Hyprland compositor via its Unix socket IPC. These send commands and return raw JSON responses.

All functions live under `helium.compositor.hyprland`.

## hyprland_dispatch

Sends an arbitrary Hyprland dispatch command.

```python
from helium.compositor.hyprland import hyprland_dispatch

hyprland_dispatch("dispatch workspace 3")
hyprland_dispatch("dispatch exec firefox")
result = hyprland_dispatch("dispatch togglespecialworkspace")
```

| Argument | Type | What it is |
|---|---|---|
| `command` | str | The Hyprland IPC command |

Returns the response string (usually JSON or empty).

## get_workspaces

Returns a JSON string listing all workspaces.

```python
import json
from helium.compositor.hyprland import get_workspaces

data = json.loads(get_workspaces())
for ws in data:
    print(ws["id"], ws["name"], ws["monitor"])
```

Each workspace object contains: `id`, `name`, `monitor`, `windows`, `active`, `urgent`.

## get_active_workspace

Returns a JSON string for the currently focused workspace.

```python
active = json.loads(get_active_workspace())
ws_id = active["id"]
```

## get_clients

Returns a JSON string listing all open windows (clients).

```python
clients = json.loads(get_clients())
for c in clients:
    print(c["class"], c["title"], c["workspace"]["id"])
```

Each client object contains: `address`, `class`, `title`, `workspace` (object with `id`), `x`, `y`, `w`, `h`, `floating`, `pinned`, `focused`, `monitor`.

## get_monitors

Returns a JSON string listing all monitors.

```python
monitors = json.loads(get_monitors())
for m in monitors:
    print(m["id"], m["name"], m["width"], "x", m["height"])
```

Each monitor object contains: `id`, `name`, `width`, `height`, `x`, `y`, `scale`, `primary`, `activeWorkspace` (object with `id` and `name`).

## Quick Reference

| Function | Returns | What it does |
|---|---|---|
| `hyprland_dispatch(command)` | str | Sends an IPC command |
| `get_workspaces()` | str (JSON) | Lists all workspaces |
| `get_active_workspace()` | str (JSON) | Gets the focused workspace |
| `get_clients()` | str (JSON) | Lists all open windows |
| `get_monitors()` | str (JSON) | Lists all monitors |
