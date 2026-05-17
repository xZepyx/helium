# WallpaperService

Sets the desktop wallpaper. Auto-detects the best backend (swww, hyprctl, swaybg, feh).

```python
wp = None
try:
    wp = helium.services.WallpaperService.get_default()
except Exception:
    pass

if wp:
    wp.set_wallpaper("/path/to/wallpaper.jpg")
    current = wp.get_wallpaper()
```

## Methods

| Method | What it does |
|---|---|
| `get_default()` | Returns the singleton |
| `set_wallpaper(path)` | Sets the wallpaper |
| `get_wallpaper()` | Returns current path |
