# ToggleButton

A button that stays pressed in until clicked again.

```python
from helium.types import ToggleButton

btn = ToggleButton("Toggle", active=True)
btn.on_toggled(lambda: print(f"active: {btn.get_active()}"))

btn.set_active(False)
state = btn.get_active()
```

## Constructor

`ToggleButton(label: str = "", active: bool = False)`

## Methods

| Method | What it does |
|---|---|
| `on_toggled(callback)` | Fires when toggled |
| `on_click(callback)` | Fires on click |
| `set_active(v)` | Sets pressed state |
| `get_active()` | Returns pressed state |
| `set_label(v)` | Changes the label |
| `get_label()` | Returns the label |
