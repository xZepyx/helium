# SystemTrayService

DBus system tray (SNI) support.

```python
tray = None
try:
    tray = helium.services.SystemTrayService.get_default()
except Exception:
    pass

if tray:
    if tray.is_available():
        items = tray.get_items()
        for item in items:
            print(item.id, item.title)
```

## Methods

| Method | What it does |
|---|---|
| `get_default()` | Returns the singleton |
| `connect_signal(signal, cb)` | Registers a signal handler |
| `is_available()` | Returns availability |
| `get_items()` | Returns tray items |
| `start_polling(ms=5000)` | Starts polling |
| `stop_polling()` | Stops polling |
