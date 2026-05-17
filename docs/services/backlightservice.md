# BacklightService

Screen brightness control via sysfs.

```python
backlight = None
try:
    backlight = helium.services.BacklightService.get_default()
except Exception:
    pass

if backlight:
    brightness = backlight.get_brightness()
    max_bright = backlight.get_max_brightness()
    pct = backlight.get_brightness_percent()
    backlight.set_brightness(500)
```

## Methods

| Method | What it does |
|---|---|
| `get_default()` | Returns the singleton |
| `get_brightness()` | Returns current brightness |
| `set_brightness(v)` | Sets brightness value |
| `get_max_brightness()` | Returns maximum brightness |
| `get_brightness_percent()` | Returns brightness as percentage |
