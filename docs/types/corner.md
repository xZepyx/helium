# Corner

Draws a rounded corner shape using Cairo. Useful for giving windows rounded corners.

```python
from helium.types import Corner

corner = Corner(
    size=20,
    radius=10,
    corner="top-left",
    color="rgba(30, 30, 30, 0.9)",
)

corner.set_size(30)
corner.set_radius(15)
corner.set_corner("bottom-right")
corner.set_color("black")
```

Corner positions: `"top-left"`, `"top-right"`, `"bottom-left"`, `"bottom-right"`.

## Constructor

`Corner(size: int = 20, radius: int = 10, corner: str = "top-left", color: str = "black")`

## Methods

| Method | What it does |
|---|---|
| `set_size(v)` | Sets corner size in pixels |
| `get_size()` | Returns size |
| `set_radius(v)` | Sets corner radius |
| `get_radius()` | Returns radius |
| `set_corner(v)` | Sets which corner to draw |
| `get_corner()` | Returns corner position |
| `set_color(v)` | Sets fill color |
| `get_color()` | Returns color |
