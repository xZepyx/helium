# HyprlandService

Connects to the Hyprland event socket for real-time compositor events.

```python
hypr = None
try:
    hypr = helium.services.HyprlandService.get_default()
    hypr.start_listening()
except Exception:
    pass

if hypr:
    active_ws = hypr.get_active_workspace_id()
    window_class = hypr.get_active_window_class()
    window_title = hypr.get_active_window_title()
    hypr.dispatch("workspace 3")
```

## Methods

| Method | What it does |
|---|---|
| `get_default()` | Returns the singleton |
| `start_listening()` | Connects to Hyprland event socket |
| `stop_listening()` | Stops event listening |
| `is_listening()` | Returns listening state |
| `connect_signal(signal, cb)` | Registers for Hyprland events |
| `dispatch(cmd)` | Sends a Hyprland dispatch command |
| `send_command(cmd)` | Sends raw IPC command |
| `get_active_workspace_id()` | Returns active workspace ID |
| `get_active_window_class()` | Returns focused window class |
| `get_active_window_title()` | Returns focused window title |
