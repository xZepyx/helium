# Image

A static image loaded from a file or icon theme. Use the factory methods — no direct constructor.

```python
from helium.types import Image

img = Image.from_icon("document-open-symbolic")
img = Image.from_file("/path/to/image.png")
```

## Methods

| Method | What it does |
|---|---|
| `Image.from_icon(name)` | Creates from theme icon |
| `Image.from_file(path)` | Creates from file |
