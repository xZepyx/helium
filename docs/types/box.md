# Box

Arranges children in a row or column.

```python
from helium.types import Box, Label

box = Box(orientation="horizontal", spacing=10)
box.add(Label("one"))
box.add(Label("two"))
box.prepend(Label("zero"))
```

With children at construction:

```python
box = Box(
    orientation="vertical",
    spacing=5,
    children=[Label("a"), Label("b")],
)
```

## Constructor

`Box(orientation: str = "horizontal", spacing: int = 0, children: list = [], halign: str = "fill", valign: str = "fill", hexpand: bool = False, vexpand: bool = False)`

## Methods

| Method | What it does |
|---|---|
| `add(child)` | Appends a child |
| `append(child)` | Same as add |
| `prepend(child)` | Inserts at front |
| `remove(child)` | Removes a child |
| `clear()` | Removes all children |
