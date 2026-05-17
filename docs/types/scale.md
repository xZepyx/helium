# Scale

A draggable slider for numeric input.

```python
from helium.types import Scale

slider = Scale(min=0, max=100, step=1, value=50, orientation="horizontal")

def on_change(value):
    print(f"slider: {value}")

slider.on_change(on_change)
slider.set_value(75)
current = slider.get_value()
```

## Constructor

`Scale(min: float = 0, max: float = 100, step: float = 1, value: float = 0, orientation: str = "horizontal")`

## Methods

| Method | What it does |
|---|---|
| `on_change(callback)` | Fires with the new value |
| `set_value(v)` | Sets the value |
| `get_value()` | Returns the value |
| `set_min(v)` | Sets minimum |
| `get_min()` | Returns minimum |
| `set_max(v)` | Sets maximum |
| `get_max()` | Returns maximum |
| `set_step(v)` | Sets step increment |
| `get_step()` | Returns step |
| `set_digits(v)` | Decimal places to show |
