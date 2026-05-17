# StackSwitcher

A bar of buttons that control a Stack's visible page.

```python
from helium.types import StackSwitcher

switcher = StackSwitcher()
switcher.set_stack(my_stack)
```

## Constructor

`StackSwitcher()`

## Methods

| Method | What it does |
|---|---|
| `set_stack(stack)` | Links to a Stack |
| `get_stack()` | Returns the linked Stack |
