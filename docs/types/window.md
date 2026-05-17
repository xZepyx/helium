# Window

A layer-shell window for bars, dashboards, and overlays.

```python
from helium.types import Window

win = Window(
    namespace="my_window",
    monitor=0,
    anchor=["left", "top", "right"],
    exclusivity="exclusive",
    layer="top",
    kb_mode="none",
    popup=False,
    margin_top=0,
    margin_bottom=0,
    margin_left=0,
    margin_right=0,
    dynamic_input_region=False,
)

win.set_child(my_widget)
win.show()
```

**anchor**: `"left"`, `"right"`, `"top"`, `"bottom"` — combine to pin edges.  
**exclusivity**: `"exclusive"` reserves space, `"none"` allows overlap.  
**layer**: `"background"`, `"bottom"`, `"top"`, `"overlay"`.  
**kb_mode**: `"none"`, `"exclusive"`, `"on-demand"`.

## Constructor

`Window(namespace: str, monitor: int = 0, anchor: list = [], exclusivity: str = "normal", layer: str = "top", kb_mode: str = "none", popup: bool = False, margin_top: int = 0, margin_bottom: int = 0, margin_left: int = 0, margin_right: int = 0, dynamic_input_region: bool = False)`

## Methods

| Method | What it does |
|---|---|
| `set_child(child)` | Sets the child widget |
| `get_namespace()` | Returns namespace |
| `set_monitor(v)` | Moves to a monitor |
| `get_monitor()` | Returns monitor index |
| `set_layer(v)` | Changes layer |
| `get_layer()` | Returns layer |
| `set_kb_mode(v)` | Changes keyboard mode |
| `get_kb_mode()` | Returns keyboard mode |
| `set_exclusivity(v)` | Changes exclusivity |
| `get_exclusivity()` | Returns exclusivity |
| `set_popup(v)` | Sets popup flag |
| `get_popup()` | Returns popup flag |
| `set_anchor(anchors)` | Sets edge anchors |
| `get_anchor()` | Returns anchors |
| `set_margin(side, value)` | Sets margin per side |
| `set_input_width(v)` | Input region width |
| `set_input_height(v)` | Input region height |
| `get_input_width()` | Returns input width |
| `get_input_height()` | Returns input height |
| `set_dynamic_input_region(v)` | Enables dynamic input |
| `get_dynamic_input_region()` | Returns dynamic input flag |
