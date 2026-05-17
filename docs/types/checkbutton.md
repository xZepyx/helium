# CheckButton

Checkbox or radio button. Use `set_group()` for radio behavior.

```python
from helium.types import CheckButton

cb = CheckButton("Enable wifi", active=True)
cb.on_toggled(lambda: print(f"checked: {cb.get_active()}"))

cb.set_active(False)
state = cb.get_active()
```

Radio buttons:

```python
a = CheckButton("A", active=True)
b = CheckButton("B")
c = CheckButton("C")

b.set_group(a)
c.set_group(a)
```

## Constructor

`CheckButton(label: str = "", active: bool = False)`

## Methods

| Method | What it does |
|---|---|
| `on_toggled(callback)` | Fires when toggled |
| `set_active(v)` | Sets checked state |
| `get_active()` | Returns checked state |
| `set_label(v)` | Changes the label |
| `get_label()` | Returns the label |
| `set_group(group)` | Links to a radio group |
