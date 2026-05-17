# Helium

Helium is a framework for building desktop shell widgets with GTK4 and layer-shell. You can create bars, dashboards, notification popups, control panels — anything that sits on your desktop.

## Hello World

> [!IMPORTANT]
> Defaut config file should be at ~/.config/helium named daemon.py
> This is the entry point of the config.

The smallest possible bar:

```python
import helium
from helium.types import Window, Label

helium.init()

win = Window(
    namespace="hello",
    anchor=["left", "top", "right"],
    exclusivity="exclusive",
)
win.set_child(Label("hello world"))
win.show()

helium.run()
```

---

## Building a Real Bar

Let's build something useful step by step. A bar with a workspace switcher on the left, a clock in the middle, and system info on the right.

### Step 1 — The window and layout

First we need a window that spans the top of the screen, and a `CenterBox` to split it into three sections:

```python
import helium
from helium.types import Window, CenterBox, Box, Label

helium.init()

win = Window(
    namespace="my_bar",
    anchor=["left", "top", "right"],
    exclusivity="exclusive",
)

layout = CenterBox()
layout.add_css_class("bar")

win.set_child(layout)
win.show()
```

### Step 2 — A live clock

A `Label` with a `Poll` timer that updates every second:

```python
import datetime
from helium.types import Label

clock = Label("")
clock.add_css_class("clock")

def tick():
    clock.set_label(datetime.datetime.now().strftime("%H:%M"))
    return True

helium.functions.Poll(1000, tick)
tick()
```

`Poll` takes an interval in milliseconds and a callback. Return `True` to keep running, `False` to stop. We call `tick()` once immediately so the clock shows the time right away instead of waiting one second.

### Step 3 — Workspace switcher

A simple workspace switcher that reads from Hyprland. We make a `Box` and fill it with buttons, one per workspace:

```python
import json
from helium.types import Box, Button
from helium.compositor.hyprland import (
    hyprland_dispatch,
    get_workspaces,
    get_active_workspace,
)

ws_box = Box(orientation="horizontal", spacing=5)
ws_box.add_css_class("workspaces")

def update_workspaces():
    try:
        raw_ws = get_workspaces()
        ws_list = json.loads(raw_ws)

        active_raw = json.loads(get_active_workspace())
        active_id = active_raw["id"]

        ws_box.clear()
        for ws in sorted(ws_list, key=lambda w: w["id"]):
            btn = Button(label=str(ws["id"]))
            btn.add_css_class("workspace")
            wid = ws["id"]
            btn.on_click(lambda idx=wid: hyprland_dispatch(f"dispatch workspace {idx}"))
            if ws["id"] == active_id:
                btn.add_css_class("active")
            ws_box.add(btn)
    except Exception:
        pass
    return True

helium.functions.Poll(500, update_workspaces)
update_workspaces()
```

We poll every 500ms so the buttons update quickly when you switch workspaces. Each button sends a Hyprland dispatch when clicked. The `clear()` call removes all old buttons before re-creating them, so the list stays in sync.

### Step 4 — Putting it all together

```python
import datetime
import json
import helium
from helium.types import Window, CenterBox, Box, Label, Button
from helium.compositor.hyprland import (
    hyprland_dispatch,
    get_workspaces,
    get_active_workspace,
)

helium.init()

# --- Workspaces ---

ws_box = Box(orientation="horizontal", spacing=5)
ws_box.add_css_class("workspaces")

def update_workspaces():
    try:
        raw_ws = get_workspaces()
        ws_list = json.loads(raw_ws)
        active_raw = json.loads(get_active_workspace())
        active_id = active_raw["id"]

        ws_box.clear()
        for ws in sorted(ws_list, key=lambda w: w["id"]):
            btn = Button(label=str(ws["id"]))
            btn.add_css_class("workspace")
            wid = ws["id"]
            btn.on_click(lambda idx=wid: hyprland_dispatch(f"dispatch workspace {idx}"))
            if ws["id"] == active_id:
                btn.add_css_class("active")
            ws_box.add(btn)
    except Exception:
        pass
    return True

helium.functions.Poll(500, update_workspaces)
update_workspaces()

# --- Clock ---

clock = Label("")
clock.add_css_class("clock")

def tick():
    clock.set_label(datetime.datetime.now().strftime("%H:%M"))
    return True

helium.functions.Poll(1000, tick)
tick()

# --- Right section ---

right_box = Box(orientation="horizontal", spacing=10)
right_box.add_css_class("right")
right_box.add(Label("vol: --"))

# --- Bar window ---

win = Window(
    namespace="my_bar",
    anchor=["left", "top", "right"],
    exclusivity="exclusive",
)

layout = CenterBox()
layout.add_css_class("bar")
layout.set_start(ws_box)
layout.set_center(clock)
layout.set_end(right_box)

win.set_child(layout)
win.show()

helium.run()
```

That's a working bar. You can style it with CSS, add more sections, wire up audio or network services, and build from there.
---

## What's in these docs

| File | What it covers |
|---|---|
| **types/** | Each widget in its own file — labels, buttons, boxes, windows, etc. |
| **services/** | Each service in its own file — audio, network, notifications, etc. |
| **config.md** | JSON config system (dot notation, defaults, auto-save) |
| **compositor.md** | Hyprland IPC functions |
| **managers.md** | WindowManager, CssManager, IconManager |
| **utilities.md** | Poll, Timeout, watch_file, unwatch_file |

