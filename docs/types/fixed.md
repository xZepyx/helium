# Fixed

Absolute positioning by x/y coordinates.

```python
from helium.types import Fixed, Label

fixed = Fixed()
fixed.put(Label("at (10, 20)"), 10, 20)
fixed.put(some_label, 100, 50)
fixed.move(some_label, 200, 100)
fixed.remove(some_label)
```

## Constructor

`Fixed()`

## Methods

| Method | What it does |
|---|---|
| `put(child, x, y)` | Places at coordinates |
| `move(child, x, y)` | Moves to new coordinates |
| `remove(child)` | Removes a child |
