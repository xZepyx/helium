# PopoverMenu

A popover that floats above the window when triggered. Can hold one child — use a Box inside for multiple items.

```python
from helium.types import PopoverMenu, Button, Box, Label

menu = PopoverMenu()

content = Box(orientation="vertical", spacing=5)
content.add(Button("Option 1"))
content.add(Button("Option 2"))
menu.add_child(content)

trigger = Button("menu")
trigger.on_click(lambda: menu.popup())
```

## Constructor

`PopoverMenu()`

## Methods

| Method | What it does |
|---|---|
| `add_child(child, id="")` | Sets the popover's child |
| `popup()` | Shows the popover |
| `popdown()` | Hides the popover |
| `set_pointing_to(x, y, w, h)` | Sets target rectangle |
| `set_parent(parent)` | Attaches to a parent |
