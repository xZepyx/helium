# Stack

Shows one child at a time (like a tab switcher).

```python
from helium.types import Stack, Label

stack = Stack()
stack.add_child(Label("Page 1"), title="first")
stack.add_child(Label("Page 2"), title="second")

stack.set_visible_child_name("second")
name = stack.get_visible_child_name()

stack.set_transition_type("slide_left_right")
stack.set_transition_duration(200)
```

Transition types: `"none"`, `"crossfade"`, `"slide_right"`, `"slide_left"`, `"slide_up"`, `"slide_down"`, `"slide_left_right"`, `"slide_up_down"`, `"over_up"`, `"over_down"`, `"over_up_down"`, `"under_up"`, `"under_down"`, `"under_left_right"`, `"rotate_left"`, `"rotate_right"`.

## Constructor

`Stack()`

## Methods

| Method | What it does |
|---|---|
| `add_child(child, title="")` | Adds a page |
| `remove(child)` | Removes a page |
| `set_visible_child(child)` | Shows a specific page |
| `set_visible_child_name(name)` | Shows by title |
| `get_visible_child_name()` | Returns visible title |
| `set_transition_type(type)` | Sets transition animation |
| `set_transition_duration(ms)` | Sets animation duration |
| `set_hhomogeneous(v)` | Same width for all pages |
| `set_vhomogeneous(v)` | Same height for all pages |
