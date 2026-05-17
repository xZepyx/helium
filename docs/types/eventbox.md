# EventBox

Wraps a child and adds mouse event handling.

```python
from helium.types import EventBox, Label

box = EventBox()
box.set_child(Label("interactive"))

box.on_click(lambda: print("left click"))
box.on_right_click(lambda: print("right click"))
box.on_middle_click(lambda: print("middle click"))
box.on_scroll_up(lambda: print("up"))
box.on_scroll_down(lambda: print("down"))
box.on_hover(lambda entering: print("enter" if entering else "leave"))
```

## Constructor

`EventBox()`

## Methods

| Method | What it does |
|---|---|
| `set_child(child)` | Sets the child |
| `on_click(callback)` | Left click |
| `on_right_click(callback)` | Right click |
| `on_middle_click(callback)` | Middle click |
| `on_scroll_up(callback)` | Scroll up |
| `on_scroll_down(callback)` | Scroll down |
| `on_hover(callback)` | Enter/leave (receives bool) |
