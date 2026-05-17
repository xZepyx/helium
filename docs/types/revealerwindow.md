# RevealerWindow

A layer-shell window that can be shown/hidden on demand. Good for notification popups.

```python
from helium.types import RevealerWindow

win = RevealerWindow(
    namespace="notif_popup",
    monitor=0,
    anchor=["right", "top"],
    layer="overlay",
)

win.set_child(my_content)
win.reveal()
win.hide_window()
win.toggle()
visible = win.is_visible()
```

## Constructor

`RevealerWindow(namespace: str, monitor: int = 0, anchor: list = [], layer: str = "overlay")`

## Methods

| Method | What it does |
|---|---|
| `set_child(child)` | Sets the child |
| `reveal()` | Shows the window |
| `hide_window()` | Hides the window |
| `toggle()` | Toggles visibility |
| `is_visible()` | Returns visibility |
| `get_namespace()` | Returns namespace |
