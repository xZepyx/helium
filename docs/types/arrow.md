# Arrow

An animated arrow icon that rotates with CSS transitions.

```python
from helium.types import Arrow, ArrowDirection

arrow = Arrow(direction=ArrowDirection.UP, pixel_size=24)

arrow.set_rotated(True)
arrow.toggle()
arrow.set_degree(180)
arrow.set_time(200)
```

## ArrowDirection

Enum with values: `UP`, `DOWN`, `LEFT`, `RIGHT`

```python
from helium.types import ArrowDirection
```

## Constructor

`Arrow(direction: ArrowDirection = ArrowDirection.UP, pixel_size: int = 24)`

## Methods

| Method | What it does |
|---|---|
| `set_rotated(v)` | Sets rotated state |
| `get_rotated()` | Returns rotated state |
| `set_degree(d)` | Sets rotation angle |
| `get_degree()` | Returns rotation angle |
| `set_time(ms)` | CSS transition time |
| `get_time()` | Returns transition time |
| `set_direction(d)` | Sets arrow direction |
| `get_direction()` | Returns direction |
| `set_counterclockwise(v)` | Sets rotation direction |
| `get_counterclockwise()` | Returns rotation direction |
| `toggle()` | Toggles rotated state |
