# CenterBox

Three-slot layout with start, center, and end positions. Perfect for bars.

```python
from helium.types import CenterBox, Label

layout = CenterBox()
layout.set_start(Label("left"))
layout.set_center(Label("center"))
layout.set_end(Label("right"))
```

## Constructor

`CenterBox()`

## Methods

| Method | What it does |
|---|---|
| `set_start(child)` | Sets the start widget |
| `set_center(child)` | Sets the center widget |
| `set_end(child)` | Sets the end widget |
