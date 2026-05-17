# ScrolledWindow

A scrollable viewport for a single child.

```python
from helium.types import ScrolledWindow

sw = ScrolledWindow()
sw.set_child(my_tall_box)
sw.set_policy("automatic", "automatic")  # h, v
```

Policy: `"always"`, `"never"`, `"automatic"`.

## Constructor

`ScrolledWindow()`

## Methods

| Method | What it does |
|---|---|
| `set_child(child)` | Sets the scrollable child |
| `set_policy(h, v)` | Sets scrollbar policy per axis |
