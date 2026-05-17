# SpinButton

A numeric entry with up/down arrows.

```python
from helium.types import SpinButton

spin = SpinButton(min=0, max=100, step=5, value=50)

def on_change(value):
    print(f"spin: {value}")

spin.on_change(on_change)
spin.set_value(75)
current = spin.get_value()
as_int = spin.get_value_as_int()

spin.set_wrap(True)
spin.set_digits(1)
```

## Constructor

`SpinButton(min: float = 0, max: float = 100, step: float = 1, value: float = 0)`

## Methods

| Method | What it does |
|---|---|
| `on_change(callback)` | Fires with new value |
| `set_value(v)` | Sets the value |
| `get_value()` | Returns the value |
| `get_value_as_int()` | Returns value as int |
| `set_min(v)` | Sets minimum |
| `get_min()` | Returns minimum |
| `set_max(v)` | Sets maximum |
| `get_max()` | Returns maximum |
| `set_step(v)` | Sets step increment |
| `get_step()` | Returns step |
| `set_digits(v)` | Decimal places |
| `set_wrap(v)` | Wrap from max to min |
