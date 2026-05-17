# Helium

Helium is a framework for building desktop shell widgets with GTK4 and layer-shell. You can create bars, dashboards, notification popups, control panels — anything that sits on your desktop.

It's structured into a few pieces:

- **helium.types** — widget classes: labels, buttons, boxes, windows, etc.
- **helium.services** — singletons that talk to your system: audio, network, notifications, media players, and more
- **helium.managers** — global registries for windows, icons, and CSS
- **helium.functions** — timers and file watchers (Poll, Timeout, watch_file)
- **helium.compositor.hyprland** — Hyprland IPC functions

## A tiny example

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

Run it with `PYTHONPATH=builddir python3 yourfile.py`.

## Getting started

- **introduction.md** — walks through building a real bar step by step
- **types/** — each widget documented in its own file
- **services/** — each service documented in its own file
- **compositor.md** — Hyprland IPC
- **managers.md** — WindowManager, CssManager, IconManager
- **utilities.md** — Poll, Timeout, watch_file
