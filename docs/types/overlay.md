# Overlay

Layers widgets on top of a main child.

```python
from helium.types import Overlay, Label

overlay = Overlay()
overlay.set_child(main_widget)
overlay.add_overlay(Label("badge"))
```

## Constructor

`Overlay()`

## Methods

| Method | What it does |
|---|---|
| `set_child(child)` | Sets the main child |
| `add_overlay(widget)` | Adds a floating overlay |
