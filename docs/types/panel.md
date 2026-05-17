# Panel

A convenience layer-shell panel with sensible defaults.

```python
from helium.types import Panel

panel = Panel(
    namespace="my_panel",
    anchor=["left", "top", "right"],
    exclusive=True,
    layer="top",
    width=-1,
    height=40,
)

panel.set_child(my_widget)
panel.set_size(500, 40)
panel.set_margin("top", 5)
```

## Constructor

`Panel(namespace: str = "panel", anchor: list = [], exclusive: bool = True, layer: str = "top", kb_mode: str = "none", popup: bool = False, width: int = -1, height: int = 40, margin_top: int = 0, margin_bottom: int = 0, margin_left: int = 0, margin_right: int = 0)`

## Methods

| Method | What it does |
|---|---|
| `set_width(v)` | Sets width |
| `set_height(v)` | Sets height |
| `set_size(w, h)` | Sets both dimensions |
| `set_child(child)` | Sets the child |
| `set_margin(side, value)` | Sets a margin |
