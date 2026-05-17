# Revealer

Shows and hides its child with an animation.

```python
from helium.types import Revealer, Label

rev = Revealer()
rev.set_child(Label("hidden content"))
rev.set_reveal_child(False)
rev.set_transition_type("slide_down")
rev.set_transition_duration(300)
rev.toggle()

visible = rev.get_reveal_child()
```

## Constructor

`Revealer()`

## Methods

| Method | What it does |
|---|---|
| `set_child(child)` | Sets the child |
| `set_reveal_child(v)` | Sets reveal state |
| `get_reveal_child()` | Returns reveal state |
| `set_transition_duration(ms)` | Animation length in ms |
| `set_transition_type(type)` | `"none"`, `"crossfade"`, `"slide_right"`, `"slide_left"`, `"slide_up"`, `"slide_down"` |
| `get_transition_type()` | Returns transition type |
| `toggle()` | Toggles reveal state |
