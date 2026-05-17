# Switch

An on/off toggle.

```python
from helium.types import Switch

sw = Switch(active=True)

def on_change(state):
    print(f"switch is {'on' if state else 'off'}")

sw.on_change(on_change)
sw.set_active(False)
state = sw.get_active()
```

## Constructor

`Switch(active: bool = False)`

## Methods

| Method | What it does |
|---|---|
| `on_change(callback)` | Fires with the new state |
| `set_active(v)` | Sets the state |
| `get_active()` | Returns the state |
